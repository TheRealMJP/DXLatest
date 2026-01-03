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
static DXLFence frameFence;
HANDLE frameFenceEvent = INVALID_HANDLE_VALUE;

static uint64_t cpuFrame = 0;  // Total number of CPU frames completed (completed means all command buffers submitted to the GPU)
static uint64_t gpuFrame = 0;  // Total number of GPU frames completed (completed means that the GPU signals the fence)
static uint64_t frameRecordingBufferIndex = 0;  // cpuFrame % RenderLatency

static constexpr uint32_t NumSwapChainBuffers = 2;
static DXLSwapChain swapChain;
static uint32_t swapChainWidth = 0;
static uint32_t swapChainHeight = 0;
static DXLResource swapChainBuffers[NumSwapChainBuffers];

static DXLDescriptorHeap rtvDescriptorHeap;
static D3D12_CPU_DESCRIPTOR_HANDLE swapChainRTVs[NumSwapChainBuffers] = { };

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

        frameFence = device->CreateFence(0);
        frameFence->SetName("Frame Fence");
        frameFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
    }

    {
        swapChainWidth = uint32_t(window.GetClientWidth());
        swapChainHeight = uint32_t(window.GetClientHeight());

        const DXGI_SWAP_CHAIN_DESC swapChainDesc =
        {
            .BufferDesc = { .Width = swapChainWidth, .Height = swapChainHeight, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = NumSwapChainBuffers,
            .OutputWindow = window.GetHwnd(),
            .Windowed = true,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        };
        swapChain = DXLSwapChain::Create(swapChainDesc, commandQueue);

        for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
            swapChainBuffers[i] = swapChain->GetBuffer(i);
    }

    {
        rtvDescriptorHeap = device->CreateDescriptorHeap({ .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
        for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
        {
            swapChainRTVs[i] = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); 
            swapChainRTVs[i].ptr += (i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
            device->CreateRenderTargetView(swapChainBuffers[i], nullptr, swapChainRTVs[i]);
        }
    }

    window.ShowWindow();
}

static void Shutdown()
{
    dxl::Release(frameFence);
    dxl::Release(commandList);
    dxl::Release(commandQueue);
    for (DXLCommandAllocator& commandAllocator: commandAllocators)
        dxl::Release(commandAllocator);
    for (DXLResource& buffer : swapChainBuffers)
        dxl::Release(buffer);
    dxl::Release(swapChain);
    dxl::Release(device);
}

static void Render()
{
    {
        // Prepare the command buffers to be used for the next frame
        commandAllocators[frameRecordingBufferIndex]->Reset();
        commandList->Reset(commandAllocators[frameRecordingBufferIndex]);
    }

    {
        // Transition the current swap chain back buffer to a render target layout
        const uint32_t backBufferIndex = swapChain->GetCurrentBackBufferIndex();
        const DXLResource backBuffer = swapChainBuffers[backBufferIndex];
        const D3D12_TEXTURE_BARRIER backBufferRTBarrier =
        {
            .SyncBefore = D3D12_BARRIER_SYNC_NONE,
            .SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET,
            .AccessBefore = D3D12_BARRIER_ACCESS_NO_ACCESS,
            .AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET,
            .LayoutBefore = D3D12_BARRIER_LAYOUT_UNDEFINED,
            .LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
            .pResource = backBuffer,
            .Subresources = { },
            .Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE,
        };
        commandList->Barrier(backBufferRTBarrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1] = { swapChainRTVs[backBufferIndex] };
        commandList->OMSetRenderTargets(1, rtvHandles, false, nullptr);

        const float clearColor[4] = { 0.1f, 0.9f, 0.1f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);

        const D3D12_TEXTURE_BARRIER backBufferPresentBarrier =
        {
            .SyncBefore = D3D12_BARRIER_SYNC_RENDER_TARGET,
            .SyncAfter = D3D12_BARRIER_SYNC_NONE,
            .AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET,
            .AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS,
            .LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
            .LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT,
            .pResource = backBuffer,
            .Subresources = { },
            .Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE,
        };
        commandList->Barrier(backBufferPresentBarrier);
    }

    {
        // Close and submit the command list
        commandList->Close();

        ID3D12CommandList* commandLists[] = { commandList };
        commandQueue->ExecuteCommandLists(ARRAY_SIZE(commandLists), commandLists);

        // Present the renderered swap chain buffer
        swapChain->Present(1, 0);

        cpuFrame += 1;

        // Signal the fence with the current frame number, so that we can check back on it
        commandQueue->Signal(frameFence, cpuFrame);

        // Wait for the GPU to catch up before we stomp an executing command buffer
        const uint64_t gpuLag = cpuFrame - cpuFrame;
        if (gpuLag >= RenderLatency)
        {
            // Make sure that the previous frame is finished
            frameFence->WaitWithEvent(gpuFrame + 1, frameFenceEvent);
            gpuFrame += 1;
        }

        frameRecordingBufferIndex = cpuFrame % RenderLatency;
    }
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