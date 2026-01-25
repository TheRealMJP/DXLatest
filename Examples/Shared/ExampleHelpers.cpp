#include "ExampleHelpers.h"

#include <windows.h>

namespace DXLExampleHelpers
{

std::string GetDirectoryFromFilePath(const char* filePath_)
{
    std::string filePath(filePath_);
    size_t idx = filePath.rfind('\\');
    if(idx != std::string::npos)
        return filePath.substr(0, idx + 1);
    else
        return std::string("");
}

std::string ResolveFilePath(const char* filePath)
{
    char resolvedPath[MAX_PATH] = { };
    GetFullPathNameA(filePath, ARRAY_SIZE(resolvedPath), resolvedPath, nullptr);
    return std::string(resolvedPath);
}

std::string GetSampleAgilitySDKPath()
{
    const char* sdkRelativePath = "..\\..\\..\\..\\..\\AgilitySDK\\bin\\x64";

    char exePath[MAX_PATH] = { };
    GetModuleFileNameA(nullptr, exePath, ARRAY_SIZE(exePath));
    std::string sdkPath = GetDirectoryFromFilePath(exePath) + sdkRelativePath;
    return ResolveFilePath(sdkPath.c_str());
}

std::string MakeString(const char* format, ...)
{
    char buffer[1024 * 16] = { 0 };
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, ARRAY_SIZE(buffer), format, args);
    return std::string(buffer);
}

void PrintMessageBuffer(const char* msg)
{
    printf("%s", msg);
    OutputDebugStringA(msg);
}

void PrintMessage(const char* msg, ...)
{
    char messageBuffer[1024] = { };

    va_list args;
    va_start(args, msg);
    int32_t len = vsnprintf_s(messageBuffer, 1024 - 1, 1024 - 1, msg, args);
    va_end(args);

    messageBuffer[len] = '\n';
    messageBuffer[len + 1] = 0;

    PrintMessageBuffer(messageBuffer);
}

void ShowMessageBox(const char* msg, ...)
{
    char messageBuffer[1024] = { };

    va_list args;
    va_start(args, msg);
    vsnprintf_s(messageBuffer, 1024 - 1, 1024 - 1, msg, args);

    MessageBoxA(nullptr, messageBuffer, "DXL", MB_OK | MB_ICONERROR);
}

void PrintAssertMessage(const char* condition, const char* msg, const char* file, const int32_t line)
{
    const uint64_t BufferSize = 2048;
    char buffer[BufferSize] = { };
    sprintf_s(buffer, BufferSize, "%s(%d): Assert Failure: ", file, line);

    if (condition != nullptr)
        sprintf_s(buffer, BufferSize, "%s'%s' ", buffer, condition);

    if (msg != nullptr)
        sprintf_s(buffer, BufferSize, "%s%s", buffer, msg);

    if (IsDebuggerPresent() == false)
        ShowMessageBox(buffer);

    sprintf_s(buffer, BufferSize, "%s\n", buffer);

    PrintMessageBuffer(buffer);
}

void AssertHandler(const char* condition, const char* file, const int32_t line, const char* msg, ...)
{
    const char* message = nullptr;
    if (msg != nullptr)
    {
        char messageBuffer[1024] = { };
        {
            va_list args;
            va_start(args, msg);
            vsnprintf_s(messageBuffer, 1024, 1024, msg, args);
            va_end(args);
        }

        message = messageBuffer;
    }

    return PrintAssertMessage(condition, message, file, line);
}

} // DXLExampleHelpers