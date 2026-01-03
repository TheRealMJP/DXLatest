#include "../../dxlatest.h"
#include "../Shared/Window.h"
#include "../Shared/ExampleHelpers.h"

using namespace dxl;

static constexpr uint32_t RenderLatency = 2;

static Window window(nullptr, "SingleTriangle");

static DXLDevice device;
static DXLCommandQueue commandQueue;
static DXLCommandAllocator commandAllocators[RenderLatency];
static DXLCommandList commandList;

static uint64_t cputFrame = 0;  // Total number of CPU frames completed (completed means all command buffers submitted to the GPU)
static uint64_t gpuFrame = 0;  // Total number of GPU frames completed (completed means that the GPU signals the fence)
extern uint64_t frameBufferIndex = 0;  // CPUFrame % RenderLatency

static DXLSwapChain swapChain;
static uint32_t swapChainWidth = 0;
static uint32_t swapChainHeight = 0;

static void Initialize()
{
    {
        const std::string agilitySDKPath = GetSampleAgilitySDKPath();

        CreateDeviceResult result = dxl::CreateDevice({ .AgilitySDKPath = agilitySDKPath.c_str(), .EnableDebugLayer = true });
        if (!result.Device)
        {
            window.CreateMessageBox(MakeString("Failed to create D3D12 device: %s (hr=0x%x)", result.FailureReason, result.Result).c_str(), "SingleTriangle");
            return;
        }
        device = result.Device;
    }

    {
        commandQueue = device->CreateCommandQueue({ .Type = D3D12_COMMAND_LIST_TYPE_DIRECT, .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE });
        commandQueue->SetName("Graphics Command Queue");

        for (uint32_t i = 0; i < RenderLatency; ++i)
        {
            commandAllocators[i] = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
            commandAllocators[i]->SetName(MakeString("Command Allocator %u", i).c_str());
        }

        commandList = device->CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE);
        commandList->SetName("Graphics Command List");
    }

    {
        swapChainWidth = uint32_t(window.GetClientWidth());
        swapChainHeight = uint32_t(window.GetClientHeight());

        const DXGI_SWAP_CHAIN_DESC swapChainDesc =
        {
            .BufferDesc = { .Width = swapChainWidth, .Height = swapChainHeight, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,
            .OutputWindow = window.GetHwnd(),
            .Windowed = true,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        };
        swapChain = DXLSwapChain::Create(swapChainDesc, commandQueue);
    }

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