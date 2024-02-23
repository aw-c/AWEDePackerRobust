local lfs = require("lfs")
local searchFrom = {
    "SS220", "MIXNikita", "EULA/CLA", "Alan Wake", "Meower", "Whitly", "spo9k", 
    "hosting restriction","DarkReaper", "Dark Reaper", "Жнец", "Тёмный жнец", 
    "Темный жнец", "тёмный жнец", "темный жнец", "CQC", "MissableComponent"
}
local whitelist = {"checkSS220_foundCorrupted", "Photocopier", "ButtScan"}

local function hasConfigElements(str)
    str = str:lower()
    for i=1, #whitelist do
        if string.find(str, whitelist[i]:lower()) then
            return false
        end
    end
    for i=1, #searchFrom do
        if string.find(str, searchFrom[i]:lower()) then
            return true
        end
    end
end

local function find_files(abs_path)
    local files = {}

    function walk_directory(path)
        for file in lfs.dir(path) do
            if file ~= "." and file ~= ".." then
                local full_path = path .. "/" .. file
                local attr = lfs.attributes(full_path)
                if attr.mode == "directory" then
                    walk_directory(full_path)
                else
                    if hasConfigElements(file) then
                        table.insert(files, string.sub(full_path, string.len(abs_path) + 2))
                    else
                        local file_desc = io.open(full_path, "r")
                        local file_contents = file_desc:read("*a")
                        if hasConfigElements(file_contents) then
                            table.insert(files, string.sub(full_path, string.len(abs_path) + 2))
                        end
                        file_desc:close()
                    end
                end
            end
        end
    end

    walk_directory(abs_path)

    return files
end

io.write("Fork folder: ")
local searchFolder = io.read()
local abs_path = os.getenv("LOCALAPPDATA").."\\AWEDePackerRobust\\"..searchFolder
local relative_paths = find_files(abs_path)

if #relative_paths > 0 then
    local file = io.open(abs_path.."\\checkSS220_foundCorrupted.txt","w+")

    for _, path in ipairs(relative_paths) do
        file:write(path.."\n");
    end
    return print("Exists: ", #relative_paths)
end

print("Not exists")