#include "IncludeExpander.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static constexpr int MAX_INCLUDE_DEPTH = 32;

static std::string NormalizePath(const std::string& path)
{
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    // collapse redundant slashes
    size_t pos;
    while ((pos = result.find("//")) != std::string::npos)
        result.erase(pos, 1);
    // resolve ./
    while ((pos = result.find("/./")) != std::string::npos)
        result.erase(pos, 2);
    return result;
}

static std::string GetDirectory(const std::string& filePath)
{
    size_t slash = filePath.find_last_of("/\\");
    if (slash == std::string::npos)
        return ".";
    return filePath.substr(0, slash);
}

static bool FileExists(const std::string& path)
{
    DWORD attrs = GetFileAttributesA(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static std::string ResolveInclude(const std::string& includePath,
                                   const std::string& currentDir,
                                   const std::vector<std::string>& searchPaths)
{
    // 1. Try relative to current file's directory
    std::string candidate = NormalizePath(currentDir + "/" + includePath);
    if (FileExists(candidate))
        return candidate;

    // 2. Try each search path
    for (const auto& sp : searchPaths)
    {
        candidate = NormalizePath(sp + "/" + includePath);
        if (FileExists(candidate))
            return candidate;
    }

    // 3. Not found — build error message
    std::ostringstream oss;
    oss << "#include \"" << includePath << "\": file not found. Searched: "
        << NormalizePath(currentDir + "/" + includePath);
    for (const auto& sp : searchPaths)
        oss << ", " << NormalizePath(sp + "/" + includePath);
    throw std::runtime_error(oss.str());
}

std::string ReadSourceFile(const std::string& filePath)
{
    std::ifstream f(filePath, std::ios::binary);
    if (!f.is_open())
        throw std::runtime_error("cannot open file: " + filePath);

    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    // Strip UTF-8 BOM
    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        content.erase(0, 3);
    }

    // Normalize line endings: \r\n → \n
    size_t pos = 0;
    while ((pos = content.find("\r\n", pos)) != std::string::npos)
    {
        content.erase(pos, 1);
    }

    return content;
}

static std::string ExpandIncludesImpl(const std::string& source,
                                       const std::string& currentDir,
                                       const std::vector<std::string>& searchPaths,
                                       std::unordered_set<std::string>& visited,
                                       int depth)
{
    if (depth > MAX_INCLUDE_DEPTH)
        throw std::runtime_error("#include nested too deeply (max " +
            std::to_string(MAX_INCLUDE_DEPTH) + ")");

    static const std::regex includeRe(R"(^\s*#include\s+\"([^\"]+)\"\s*$)");

    std::istringstream input(source);
    std::ostringstream output;
    std::string line;

    while (std::getline(input, line))
    {
        // Also strip trailing \r in case of mixed line endings
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::smatch match;
        if (std::regex_match(line, match, includeRe))
        {
            std::string includePath = match[1].str();
            std::string fullPath = ResolveInclude(includePath, currentDir, searchPaths);
            std::string absPath = NormalizePath(fullPath);

            if (visited.find(absPath) != visited.end())
                throw std::runtime_error("circular #include: " + absPath);

            visited.insert(absPath);
            std::string includedContent = ReadSourceFile(fullPath);
            std::string includedDir = GetDirectory(fullPath);
            std::string expanded = ExpandIncludesImpl(includedContent, includedDir,
                                                       searchPaths, visited, depth + 1);
            visited.erase(absPath);

            output << expanded << '\n';
        }
        else
        {
            output << line << '\n';
        }
    }

    return output.str();
}

std::string ExpandIncludes(const std::string& source,
                           const std::string& currentDir,
                           const std::vector<std::string>& searchPaths)
{
    std::unordered_set<std::string> visited;
    return ExpandIncludesImpl(source, currentDir, searchPaths, visited, 0);
}
