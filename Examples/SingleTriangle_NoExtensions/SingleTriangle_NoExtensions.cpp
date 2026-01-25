#include "../../dxlatest.h"
#include "../../AgilitySDK/include/d3dx12/d3dx12.h"
#include "../Shared/Window.h"
#include "../Shared/ExampleHelpers.h"

#include "SingleTriangleVS.h"
#include "SingleTrianglePS.h"

using namespace dxl;
using namespace DXLExampleHelpers;

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

static IDXGIFactory7* dxgiFactory = nullptr;
static constexpr uint32_t NumSwapChainBuffers = 2;
static DXLSwapChain swapChain;
static uint32_t swapChainWidth = 0;
static uint32_t swapChainHeight = 0;
static DXLResource swapChainBuffers[NumSwapChainBuffers];

static DXLDescriptorHeap rtvDescriptorHeap;
static D3D12_CPU_DESCRIPTOR_HANDLE swapChainRTVs[NumSwapChainBuffers] = { };

static DXLRootSignature rootSignature;
static DXLPipelineState pso;

#define RELEASE_INTERFACE(x) do { if (x) { x->Release(); x = nullptr; } } while (0)

static void DebugLayerCallback([[maybe_unused]] D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, [[maybe_unused]] D3D12_MESSAGE_ID ID, const char* description, [[maybe_unused]] void* context)
{
    if (severity == D3D12_MESSAGE_SEVERITY_MESSAGE || severity == D3D12_MESSAGE_SEVERITY_MESSAGE)
    {
        PrintMessage("D3D Debug Layer: %s", description);
        return;
    }

    PrintMessage("D3D Debug Layer Error: %s", description);
    if (IsDebuggerPresent() == false)
        ShowMessageBox("D3D Debug Layer Error: %s", description);
    __debugbreak();
}

static void Initialize()
{
    {
        const std::string agilitySDKPath = GetSampleAgilitySDKPath();

        ASSERT_HR(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)), "Failed to create DXGI factory");

        IDXGIAdapter4* adapter = nullptr;
        ASSERT_HR(dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)), "Failed to enumerate a DXGI adapter");

        DXGI_ADAPTER_DESC1 desc = { };
        adapter->GetDesc1(&desc);
        PrintMessage("Creating D3D12 device on adapter '%ls'", desc.Description);

        ID3D12SDKConfiguration1* sdkConfig = nullptr;
        ASSERT_HR(D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdkConfig)), "Failed to aquire the D3D12 SDK configuration interface");

        ID3D12DeviceFactory* deviceFactory = nullptr;
        ASSERT_HR(sdkConfig->CreateDeviceFactory(D3D12_SDK_VERSION, agilitySDKPath.c_str(), IID_PPV_ARGS(&deviceFactory)), "Failed to create a D3D12 device factory");
        RELEASE_INTERFACE(sdkConfig);

        {
            DXLDebug d3d12debug;
            if (SUCCEEDED(deviceFactory->GetConfigurationInterface(CLSID_D3D12Debug, IID_PPV_ARGS(d3d12debug.AddressOfNative()))))
            {
                d3d12debug->EnableDebugLayer();
                PrintMessage("Enabled D3D12 debug layer");
            }

            d3d12debug->SetEnableGPUBasedValidation(true);
            RELEASE_INTERFACE(d3d12debug);
        }

        ASSERT_HR(deviceFactory->CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.AddressOfNative())), "Failed to create D3D12 device from device factory");

        {
            DXLDebugInfoQueue infoQueue;
            if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(infoQueue.AddressOfNative()))))
            {
                DWORD callbackCookie = 0;
                infoQueue->RegisterMessageCallback(DebugLayerCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &callbackCookie);
            }
            RELEASE_INTERFACE(infoQueue);
        }

        RELEASE_INTERFACE(deviceFactory);
        RELEASE_INTERFACE(adapter);
    }

    {
        const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { .Type = D3D12_COMMAND_LIST_TYPE_DIRECT, .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE };
        ASSERT_HR(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue.AddressOfNative())), "Failed to create command queue");
        commandQueue->SetName(L"Graphics Command Queue");

        for (uint32_t i = 0; i < RenderLatency; ++i)
        {
            ASSERT_HR(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocators[i].AddressOfNative())), "Failed to create command allocator");
            commandAllocators[i]->SetName(L"Command Allocator");
        }

        ASSERT_HR(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(commandList.AddressOfNative())), "Failed to create graphics command list");
        commandList->SetName(L"Graphics Command List");        

        ASSERT_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(frameFence.AddressOfNative())), "Failed to create frame fence");
        frameFence->SetName(L"Frame Fence");
        frameFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
    }

    {
        swapChainWidth = uint32_t(window.GetClientWidth());
        swapChainHeight = uint32_t(window.GetClientHeight());

        DXGI_SWAP_CHAIN_DESC swapChainDesc =
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

        IDXGISwapChain* tempSwapChain = nullptr;
        ASSERT_HR(dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain), "Failed to create DXGI swap chain");

        ASSERT_HR(tempSwapChain->QueryInterface(IID_PPV_ARGS(swapChain.AddressOfNative())), "failed to query swap chain interface");
        RELEASE_INTERFACE(tempSwapChain);

        for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
            swapChain->GetBuffer(i, IID_PPV_ARGS(swapChainBuffers[i].AddressOfNative()));
    }

    {
        const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, .NumDescriptors = 2, .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE };
        ASSERT_HR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvDescriptorHeap.AddressOfNative())), "Failed to create RTV descriptor heap");
        for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
        {
            swapChainRTVs[i] = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(); 
            swapChainRTVs[i].ptr += (i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
            device->CreateRenderTargetView(swapChainBuffers[i], nullptr, swapChainRTVs[i]);
        }
    }

    {
        D3D12_ROOT_SIGNATURE_DESC2 rootSignatureDesc = { };

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc =
        {
            .Version = D3D_ROOT_SIGNATURE_VERSION_1_2,
            .Desc_1_2 = rootSignatureDesc,
        };

        ID3DBlob* signature = nullptr;
        ID3DBlob* error = nullptr;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signature, &error);
        if (FAILED(hr))
        {
            const char* errString = error ? reinterpret_cast<const char*>(error->GetBufferPointer()) : "";
            ASSERT(false, "Failed to serialize root signature: %s", errString);
        }

        ASSERT_HR(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.AddressOfNative())), "Failed to create root signature");

        RELEASE_INTERFACE(signature);
        RELEASE_INTERFACE(error);
    }

    {
        D3D12_RT_FORMAT_ARRAY rtvFormats = { };
        rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvFormats.NumRenderTargets = 1;

        CD3DX12_DEPTH_STENCIL_DESC2 depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC2(CD3DX12_DEFAULT());
        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

        CD3DX12_PIPELINE_STATE_STREAM6 stateStream;
        stateStream.pRootSignature = rootSignature;
        stateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        stateStream.VS = { SingleTriangleVS, sizeof(SingleTriangleVS) };
        stateStream.PS = { SingleTrianglePS, sizeof(SingleTrianglePS) };
        stateStream.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
        stateStream.DepthStencilState = depthStencilDesc;
        stateStream.RasterizerState = CD3DX12_RASTERIZER_DESC2(CD3DX12_DEFAULT());
        stateStream.DSVFormat = DXGI_FORMAT_UNKNOWN;
        stateStream.RTVFormats = rtvFormats;

        const D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = { .SizeInBytes = sizeof(stateStream), .pPipelineStateSubobjectStream = &stateStream };

        ASSERT_HR(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(pso.AddressOfNative())), "Failed to create PSO");

        pso->SetName(L"Main PSO");
    }

    window.ShowWindow();
}

static void Shutdown()
{
    RELEASE_INTERFACE(dxgiFactory);
    RELEASE_INTERFACE(rootSignature);
    RELEASE_INTERFACE(pso);
    RELEASE_INTERFACE(rtvDescriptorHeap);
    RELEASE_INTERFACE(frameFence);
    RELEASE_INTERFACE(commandList);
    RELEASE_INTERFACE(commandQueue);
    for (DXLCommandAllocator& commandAllocator: commandAllocators)
        RELEASE_INTERFACE(commandAllocator);
    for (DXLResource& buffer : swapChainBuffers)
        RELEASE_INTERFACE(buffer);
    RELEASE_INTERFACE(swapChain);
    RELEASE_INTERFACE(device);
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

        {
            const D3D12_TEXTURE_BARRIER barrier =
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

            const D3D12_BARRIER_GROUP group =
            {
                .Type = D3D12_BARRIER_TYPE_TEXTURE,
                .NumBarriers = 1,
                .pTextureBarriers = &barrier,
            };
            commandList->Barrier(1, &group);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1] = { swapChainRTVs[backBufferIndex] };
        commandList->OMSetRenderTargets(1, rtvHandles, false, nullptr);

        const float clearColor[4] = { 0.2f, 0.3f, 0.7f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);

        {
            D3D12_VIEWPORT viewport =
            {
                .TopLeftX = 0.0f,
                .TopLeftY = 0.0f,
                .Width = float(swapChainWidth),
                .Height = float(swapChainHeight),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };

            D3D12_RECT scissorRect =
            {
                scissorRect.left = 0,
                scissorRect.top = 0,
                scissorRect.right = uint32_t(swapChainWidth),
                scissorRect.bottom = uint32_t(swapChainHeight),
            };

            commandList->RSSetViewports(1, &viewport);
            commandList->RSSetScissorRects(1, &scissorRect);
        }

        commandList->SetPipelineState(pso);
        commandList->SetGraphicsRootSignature(rootSignature);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(3, 1, 0, 0);

        {
            const D3D12_TEXTURE_BARRIER barrier =
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

            const D3D12_BARRIER_GROUP group =
            {
                .Type = D3D12_BARRIER_TYPE_TEXTURE,
                .NumBarriers = 1,
                .pTextureBarriers = &barrier,
            };
            commandList->Barrier(1, &group);
        }
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
        const uint64_t gpuLag = cpuFrame - gpuFrame;
        if (gpuLag >= RenderLatency)
        {
            // Make sure that the previous frame is finished
            const uint64_t waitValue = gpuFrame + 1;
            if (frameFence->GetCompletedValue() < waitValue)
            {
                frameFence->SetEventOnCompletion(waitValue, frameFenceEvent);
                WaitForSingleObject(frameFenceEvent, INFINITE);
            }

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