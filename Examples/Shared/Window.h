#pragma once

#include <windows.h>
#include <vector>

class Window
{

public:

    typedef void (*MsgFunction)(void*, HWND, uint32_t, WPARAM, LPARAM);

    Window(HINSTANCE hinstance,
           const char* name = "Window",
           uint32_t style = WS_CAPTION|WS_OVERLAPPED|WS_SYSMENU,
           uint32_t exStyle = WS_EX_APPWINDOW,
           uint32_t clientWidth = 1280,
           uint32_t clientHeight = 720);
    ~Window();

    HWND GetHwnd() const;
    HINSTANCE GetHinstance() const;
    void MessageLoop();

    bool IsAlive() const;
    bool IsMinimized() const;
    bool HasFocus() const;
    LONG_PTR GetWindowStyle() const;
    LONG_PTR GetExtendedStyle() const;
    void SetWindowStyle(uint32_t newStyle);
    void SetExtendedStyle(uint32_t newExStyle);
    void Maximize();
    void SetWindowPos(int32_t posX, int32_t posY);
    void GetWindowPos(int32_t& posX, int32_t& posY) const;
    void ShowWindow(bool show = true);
    void SetClientArea(int32_t clientX, int32_t clientY);
    void GetClientArea(int32_t& clientX, int32_t& clientY) const;
    void SetWindowTitle(const char* title);
    void SetBorderless(bool borderless);
    bool Borderless() const;
    void Destroy();

    int32_t CreateMessageBox(const char* message, const char* title, uint32_t type = MB_OK);

    void RegisterMessageCallback(MsgFunction msgFunction, void* context);

    operator HWND() { return hwnd; }

private:

    void MakeWindow(const char* name);

    LRESULT MessageHandler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT WINAPI WndProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

    HWND hwnd = nullptr;
    HINSTANCE hinstance = nullptr;
    uint32_t style = 0;
    uint32_t exStyle = 0;
    uint32_t nonFSWidth = 0;
    uint32_t nonFSHeight = 0;
    int32_t nonFSPosX = 0;
    int32_t nonFSPosY = 0;
    uint64_t nonFSStyle = 0;
    bool borderless = false;

    struct Callback
    {
        MsgFunction Function;
        void* Context;
    };

    std::vector<Callback> messageCallbacks;
};
