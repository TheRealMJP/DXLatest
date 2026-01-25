#pragma once

#include <string>

#define ARRAY_SIZE(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace DXLExampleHelpers
{

std::string GetDirectoryFromFilePath(const char* filePath_);
std::string ResolveFilePath(const char* filePath);
std::string GetSampleAgilitySDKPath();

std::string MakeString(const char* format, ...);

void PrintMessageBuffer(const char* msg);
void PrintMessage(const char* msg, ...);
void ShowMessageBox(const char* msg, ...);
void PrintAssertMessage(const char* condition, const char* msg, const char* file, const int32_t line);
void AssertHandler(const char* condition, const char* file, const int32_t line, const char* msg, ...);

} // namespace DXLExampleHelpers

#define ASSERT(cond, msg, ...) \
    do \
    { \
        if (!(cond)) \
        { \
            AssertHandler(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__); \
            __debugbreak(); \
        } \
    } while(0)

#define ASSERT_HR(cond, msg, ...) ASSERT(SUCCEEDED(cond), msg, __VA_ARGS__)