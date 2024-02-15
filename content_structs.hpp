#pragma once
#include <cmath>
#include <string>

struct ContentVersion
{
    std::string CreateName()
    {
        return std::to_string(Id) + "_" + ForkId;
    }
    ContentVersion(uint32_t id, void* hash, std::string forkId, std::string forkVersion)
    {
        Id = id;
        Hash = hash;
        ForkId = forkId;
        ForkVersion = forkVersion;
    }
    uint32_t Id;
    void* Hash;
    std::string ForkId;
    std::string ForkVersion;
};

struct ContentData
{
    ContentData(uint32_t compressedSize, uint32_t decompressedSize, uint32_t compression, void* data)
    {
        CompressedSize = compressedSize;
        DecompressedSize = decompressedSize;
        Compression = compression;
        Data = data;
    }
    uint32_t CompressedSize;
    uint32_t DecompressedSize;
    uint32_t Compression;
    void* Data;
};

struct PathInfo
{
    PathInfo(uint32_t contentId, uint32_t versionId, std::string path)
    {
        ContentId = contentId;
        VersionId = VersionId;
        Path = path;
    }
    uint32_t ContentId;
    uint32_t VersionId;
    std::string Path;
};