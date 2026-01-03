#include "Window.h"

static const char* WindowClassName = "SimpleWindowClass";

Window::Window( HINSTANCE hinstance,
                const char* name,
                uint32_t style,
                uint32_t exStyle,
                uint32_t clientWidth,
                uint32_t clientHeight) :    style(style),
                                            exStyle(exStyle),
                                            hinstance(hinstance)
{
    if (hinstance == nullptr)
        this->hinstance = GetModuleHandle(nullptr);

    MakeWindow(name);
    SetClientArea(clientWidth, clientHeight);
}

Window::~Window()
{
    ::DestroyWindow(hwnd);
    ::UnregisterClass(WindowClassName, hinstance);
}

HWND Window::GetHwnd() const
{
    return hwnd;
}

HINSTANCE Window::GetHinstance() const
{
    return hinstance;
}

bool Window::IsAlive() const
{
    return ::IsWindow(hwnd) != 0;
}

bool Window::IsMinimized() const
{
    return ::IsIconic(hwnd) != 0;
}

bool Window::HasFocus() const
{
    return GetActiveWindow() == hwnd;
}

void Window::SetWindowStyle(uint32_t newStyle)
{
    ::SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

    style = newStyle;
}

void Window::SetExtendedStyle(uint32_t newExStyle)
{
    ::SetWindowLongPtr(hwnd, GWL_EXSTYLE, newExStyle);

    exStyle = newExStyle;
}

LONG_PTR Window::GetWindowStyle() const
{
    return GetWindowLongPtr(hwnd, GWL_STYLE);
}

LONG_PTR Window::GetExtendedStyle() const
{
    return GetWindowLongPtr(hwnd, GWL_EXSTYLE);
}

void Window::MakeWindow(const char* name)
{
    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof(WNDCLASSEX),
        CS_DBLCLKS,
        WndProc,
        0,
        0,
        hinstance,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "SimpleWindowClass",
        nullptr
    };

    ::RegisterClassExA(&wc);

    // Create the application's window
    hwnd = ::CreateWindowExA(exStyle, WindowClassName, name, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hinstance, this);
}

void Window::SetWindowPos(int32_t posX, int32_t posY)
{
    ::SetWindowPos(hwnd, HWND_NOTOPMOST, posX, posY, 0, 0, SWP_NOSIZE);
}

void Window::GetWindowPos(int32_t& posX, int32_t& posY) const
{
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);
    posX = windowRect.left;
    posY = windowRect.top;
}

void Window::ShowWindow(bool show)
{
    int32_t cmdShow = show ? SW_SHOW : SW_HIDE;

    ::ShowWindow(hwnd, cmdShow);
}

void Window::SetClientArea(int32_t clientX, int32_t clientY)
{
    RECT windowRect;
    ::SetRect( &windowRect, 0, 0, clientX, clientY );

    BOOL isMenu = (::GetMenu(hwnd) != nullptr);
    AdjustWindowRectEx(&windowRect, style, isMenu, exStyle);

    ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE);
}

void Window::GetClientArea(int32_t& clientX, int32_t& clientY) const
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    clientX = clientRect.right;
    clientY = clientRect.bottom;
}

int32_t Window::GetClientWidth() const
{
    int32_t width, height;
    GetClientArea(width, height);
    return width;
}

int32_t Window::GetClientHeight() const
{
    int32_t width, height;
    GetClientArea(width, height);
    return height;
}

void Window::SetWindowTitle(const char* title)
{
    SetWindowText(hwnd, title);
}

void Window::SetBorderless(bool borderless_)
{
    if (borderless == borderless_)
        return;

    if (borderless_)
    {
        // Get the desktop coordinates
        POINT point = { };
        HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
        if (monitor == 0)
            return;

        MONITORINFOEX info = { };
        info.cbSize = sizeof(MONITORINFOEX);
        if (GetMonitorInfo(monitor, &info) == 0)
            return;

        int32_t monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
        int32_t monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

        // Save off the current window size
        RECT wrect = { };
        GetWindowRect(hwnd, &wrect);
        nonFSWidth = uint32_t(wrect.right - wrect.left);
        nonFSHeight = uint32_t(wrect.bottom - wrect.top);
        nonFSPosX = wrect.left;
        nonFSPosY = wrect.top;
        nonFSStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

        SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        ::SetWindowPos(hwnd, nullptr, info.rcMonitor.left, info.rcMonitor.top, monitorWidth, monitorHeight, 0);
    }
    else
    {
        // Restore the previous window style
        SetWindowLongPtr(hwnd, GWL_STYLE, nonFSStyle);

        // Place the window to the saved position, and resize it
        ::SetWindowPos(hwnd, nullptr, nonFSPosX, nonFSPosY, nonFSWidth, nonFSHeight, 0);
    }

    borderless = borderless_;
}

bool Window::Borderless() const
{
    return borderless;
}

int32_t Window::CreateMessageBox(const char* message, const char* title, uint32_t type)
{
    return ::MessageBoxA(hwnd, message, title, type);
}

void Window::Maximize()
{
    ::ShowWindow( hwnd, SW_MAXIMIZE );
}

void Window::Destroy()
{
    ::DestroyWindow(hwnd);
    ::UnregisterClass(WindowClassName, hinstance);
}

LRESULT Window::MessageHandler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
    for (const Callback& callback : messageCallbacks)
        callback.Function(callback.Context, hWnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
        // Window is being destroyed
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;

        // Window is being closed
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            return 0;
        }
    }

    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT WINAPI Window::WndProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_NCCREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    Window* pObj = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pObj)
        return pObj->MessageHandler(hWnd, uMsg, wParam, lParam);
    else
        return ::DefWindowProcA(hWnd, uMsg, wParam, lParam);
}


void Window::MessageLoop()
{
    // Main message loop:
    MSG msg;

    while(PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageA(&msg);
    }
}

void Window::RegisterMessageCallback(MsgFunction msgFunction, void* context)
{
    Callback callback;
    callback.Function = msgFunction;
    callback.Context = context;
    messageCallbacks.push_back(callback);
}