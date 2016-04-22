
local lfs = require "lfs"

local ipairs    = ipairs
local table     = table

local includePaths = {"src/common/"}

for path in lfs.dir("src/common/") do
    if path ~= "." and path ~= ".." then
        path = "src/common/" .. path .. "/"
        if lfs.attributes(path, "mode") == "directory" then
            table.insert(includePaths, path)
        end
    end
end

local included = {}

local function getFileContents(name)
    for _, path in ipairs(includePaths) do
        local file = io.open(path .. name, "r")
        if file then
            local str = file:read("*a")
            file:close()
            return str
        end
    end
    
    error("Could not find file '" .. name .. "'")
end

local function includeFile(name)
    if included[name] then return "" end
    
    included[name] = true
    
    local str = getFileContents(name)
    
    str = str:gsub('#%s*include%s*"([^"]+)"[^\n]*\n', includeFile)
    
    return str
end

local function buildFile(name, extraInclude)
    table.insert(includePaths, extraInclude)
    
    local file = assert(io.open("amalg/amalg_".. name .. ".cpp", "w+"))
    
    for _, dir in ipairs(includePaths) do
        for filename in lfs.dir(dir) do
            if filename:find("%.cpp$") then
                file:write(includeFile(filename))
            end
        end
    end

    file:close()
end

buildFile(arg[1], arg[2])
