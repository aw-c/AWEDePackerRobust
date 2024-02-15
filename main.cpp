#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include "content_structs.hpp"
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <mutex>

#include <zstd.h>

using namespace SQLite;
namespace fs = std::filesystem;

// Console Pretty Print Start

#define COLOR_RED 4
#define COLOR_BLUE 3

HANDLE CONSOLE = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
WORD saved_attributes;

template <typename T>
void colored_print(T str, WORD color = COLOR_BLUE)
{
    SetConsoleTextAttribute(CONSOLE, color);
    std::cout << str;
    SetConsoleTextAttribute(CONSOLE, saved_attributes);
}

// Console Pretty Print End

//Some data Start

#define HASH_SIZE 32
#define ABSOLUTE_DB_PATH

Database contentDB("C:\\Users\\gordy\\AppData\\Local\\Space Station 14\\launcher\\content.db");
std::unordered_map<uint32_t, PathInfo> Manifest;
std::vector<ContentVersion> Forks;
std::unordered_map<uint32_t, ContentData> Content;
bool isThreadStopped = false;
std::string LastTrackedSymbol;
uint32_t CurrentIndexFile = 0;

std::mutex mtx;

//Some data End

std::vector<ContentVersion> FetchForks()
{
    Statement data(contentDB, "SELECT * FROM ContentVersion");

    std::vector<ContentVersion> content;
    while (data.executeStep())
    {
        uint32_t Id = data.getColumn(0).getInt();
        void* Hash = (void*)data.getColumn(1).getBlob();
        std::string ForkId = data.getColumn(2).getString();
        std::string ForkVersion = data.getColumn(3).getString();

        content.push_back({ Id, Hash, ForkId, ForkVersion });
    }

    return content;
}

bool isHashEquals(void* hash1, void* hash2)
{
    /*void* alloced_addr = malloc(HASH_SIZE + 1);
    memcpy(alloced_addr, hash1, HASH_SIZE);
    char* s_hash1 = (char*)alloced_addr;
    s_hash1[HASH_SIZE] = '0';

    alloced_addr = malloc(HASH_SIZE + 1);
    memcpy(alloced_addr, hash2, HASH_SIZE);
    char* s_hash2 = (char*)alloced_addr;
    s_hash2[HASH_SIZE] = '0';

    bool bCan = std::strcmp(s_hash1, s_hash2) ? false : true;

    free(s_hash1);
    free(s_hash2);

    return bCan;*/

    return memcmp(hash1, hash2, HASH_SIZE) == 0;
}

std::unordered_map<uint32_t, PathInfo> FetchManifest(uint32_t versionId)
{
    Statement data(contentDB, "SELECT * FROM ContentManifest WHERE (VersionId = ?)");
    data.bind(1, versionId);

    std::unordered_map<uint32_t, PathInfo> manifest;
    while (data.executeStep())
    {
        versionId = data.getColumn(1).getInt();
        std::string path = data.getColumn(2).getString();
        uint32_t contentId = data.getColumn(3).getInt();

        manifest.insert({ contentId, { contentId, versionId, path } });
    }

    return manifest;
}

ContentData GetContent(uint32_t id)
{
    Statement data(contentDB, "SELECT * FROM Content WHERE (Id = ?)");
    data.bind(1, id);
    data.executeStep();

    void* blob = malloc(data.getColumn(4).getBytes());
    memcpy(blob, data.getColumn(4).getBlob(), data.getColumn(4).getBytes());

    return ContentData(data.getColumn(4).getBytes(), data.getColumn(2).getUInt(), data.getColumn(3).getUInt(), blob);
}

ContentVersion* GetFork(int forkId)
{
    for (auto& v : Forks)
    {
        if (v.Id == forkId)
            return &v;
    }

    return nullptr;
}

bool IsForkExisting(int forkId)
{
    if (GetFork(forkId) != nullptr)
        return true;
    return false;
}

void RemoveLastConsoleLine(uint32_t completeXRRsymb)
{
    std::cout << '\r' << std::string(completeXRRsymb, '\xff');
}

void UnpackFiles(std::unordered_map<uint32_t, PathInfo>& manifest, const std::string output_dir)
{
    for (auto path : manifest)
    {
        auto content = GetContent(path.first);

        //printf("%s", (std::string(abstract_path) + '/' + std::string(path.second.Path)).c_str());
        std::string fullpath = output_dir + path.second.Path;
        fs::path pathObj(fullpath);
        fs::create_directories(pathObj.parent_path());
        FILE* file = fopen(fullpath.c_str(), "wb");
        //printf("Proccessing %p\n", file);
        if (file == nullptr)
        {
            printf("Error path: %s\n", fullpath.c_str());
            return;
        }
        void* data = content.Data;
        void* decompressedData;
        bool proccessAsCompressed = false;
        uint32_t size = content.CompressedSize;
        if (content.Compression == 2)
        {
            decompressedData = malloc(content.DecompressedSize);
            ZSTD_decompress(decompressedData, content.DecompressedSize, data, content.CompressedSize);
            fwrite(decompressedData, 1, content.DecompressedSize, file);
            free(decompressedData);
        }
        else
            fwrite(data, 1, size, file);
        fclose(file);

        delete data;

        mtx.lock();
        CurrentIndexFile++;
        LastTrackedSymbol = path.second.Path;
        mtx.unlock();
    }

    isThreadStopped = true;
}

int main(int argc, char** argv)
{
    GetConsoleScreenBufferInfo(CONSOLE, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;

    try
    {
        Forks = FetchForks();
    }
    catch (const std::exception&)
    {
        colored_print("Unavailable to fetch forks. Abort!\n", COLOR_RED);
        return 1;
    }

    if (argc < 2)
    {
        colored_print("[ - ] AWEDePackerRobust\n", COLOR_RED);
        colored_print("[ - ] Developed by Alan Wake 2/14/2024\n");
        colored_print("[ - ] Select build id to unpack:\n");
        for (auto& t : Forks)
        {
            printf("-\tId: %d\tHash: %s %d %s\tForkId: %s\tForkVersion: %s\nOutput folder: %s\n",
                t.Id, "BLOB", HASH_SIZE, "Bytes",
                t.ForkId.c_str(),
                t.ForkVersion.c_str(),
                t.CreateName().c_str());
        }

        return 0;
    }
    if (argc == 2)
    {
        int selectedFork = std::atoi(argv[1]);

        printf("Processing forkId %d...\n", selectedFork);

        if (!IsForkExisting(selectedFork))
        {
            colored_print("[ - ] There are no fork with id \"", COLOR_RED);
            colored_print(selectedFork, COLOR_RED);
            colored_print("\"\n", COLOR_RED);
            return 1;
        }

        colored_print("[ - ] Found forkId \"");
        colored_print(selectedFork);
        colored_print("\"\n");

        printf("Fetching manifest...\n");
        try
        {
            Manifest = FetchManifest(selectedFork);
        }
        catch (const std::exception&)
        {
            colored_print("There was an exception while fetching the manifest. Abort!\n", COLOR_RED);
            return 1;
        }

        colored_print("[ - ] Fetch manifest has been finished\n");
        colored_print("[ - ] Start unpacking build files\n");

        ContentVersion* currentFork = GetFork(selectedFork);

        const std::string result_path(std::string(getenv("LOCALAPPDATA"))
            + '\\'
            + "AWEDePackerRobust\\"
            + currentFork->CreateName()
            + "\\");

        std::thread unpack([result_path]() {UnpackFiles(Manifest, result_path); });
        unpack.detach();

        CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
        GetConsoleScreenBufferInfo(CONSOLE, &screenBufferInfo);

        std::string tracked;
        std::string _tracked;
        std::string completeString;
        uint32_t elements = Manifest.size();
        char c[256];

        while (!isThreadStopped)
        {
            Sleep(50);
            mtx.lock();
            _tracked = LastTrackedSymbol;
            mtx.unlock();
            if (tracked != _tracked)
            {
                RemoveLastConsoleLine(completeString.length());
                sprintf(c, "\r[%d / %d]: %s", CurrentIndexFile, elements, tracked.c_str());
                completeString = c;
                tracked = _tracked;
                printf(c);
            }
        }

        colored_print("\n\n[ - ] End unpacking build files\n");
        colored_print("[ - ] Check directory: ");
        printf(result_path.c_str());
        colored_print("\n");

        return 0;
    }

    //delete contentDB;
}