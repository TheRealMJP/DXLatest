#pragma once

#include <windows.h>
#include <string>

#define ARRAY_SIZE(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

inline std::string GetDirectoryFromFilePath(const char* filePath_)
{
    std::string filePath(filePath_);
    size_t idx = filePath.rfind('\\');
    if(idx != std::string::npos)
        return filePath.substr(0, idx + 1);
    else
        return std::string("");
}

inline std::string ResolveFilePath(const char* filePath)
{
    char resolvedPath[MAX_PATH] = { };
    GetFullPathNameA(filePath, ARRAY_SIZE(resolvedPath), resolvedPath, nullptr);
    return std::string(resolvedPath);
}

inline std::string GetSampleAgilitySDKPath()
{
    const char* sdkRelativePath = "..\\..\\..\\..\\..\\AgilitySDK\\bin\\x64";

    char exePath[MAX_PATH] = { };
    GetModuleFileNameA(nullptr, exePath, ARRAY_SIZE(exePath));
    std::string sdkPath = GetDirectoryFromFilePath(exePath) + sdkRelativePath;
    return ResolveFilePath(sdkPath.c_str());
}

inline std::string MakeString(const char* format, ...)
{
    char buffer[1024 * 16] = { 0 };
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, ARRAY_SIZE(buffer), format, args);
    return std::string(buffer);
}