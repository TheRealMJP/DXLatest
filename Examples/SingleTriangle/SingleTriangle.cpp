#include "../../dxlatest.h"
#include "../Shared/Window.h"
#include "../Shared/ExampleHelpers.h"

using namespace dxl;

static Window window(nullptr, "SingleTriangle");
static DXLDevice device;
static IDXGISwapChain4* swapChain = nullptr;

static void Initialize()
{
    const std::string agilitySDKPath = GetSampleAgilitySDKPath();

    CreateDeviceResult result = dxl::CreateDevice({ .AgilitySDKPath = agilitySDKPath.c_str(), .EnableDebugLayer = true });
    if (!result.Device)
    {
        window.CreateMessageBox(MakeString("Failed to create D3D12 device: %s (hr=0x%x)", result.FailureReason, result.Result).c_str(), "SingleTriangle");
        return;
    }
    device = result.Device;

    window.ShowWindow();
}

static void Shutdown()
{

}

static void Render()
{

}

int32_t APIENTRY WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] char* lpCmdLine, [[maybe_unused]] int32_t nCmdShow)
{
    Initialize();

    while (window.IsAlive())
    {
        if (!window.IsMinimized())
            Render();

        window.MessageLoop();
    }

    Shutdown();

    return 0;
}