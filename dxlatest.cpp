#include "dxlatest.h"

#include "AgilitySDK/include/d3dx12/d3dx12.h"

#include <wrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "DXGI.lib")
#endif

#define DXL_ARRAY_SIZE(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace DXL
{

using Microsoft::WRL::ComPtr;

static void PrintMessageBuffer(const char* msg)
{
    printf("%s", msg);
    OutputDebugStringA(msg);
}

static void PrintMessage(const char* msg, ...)
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

static void ShowMessageBox(const char* msg, ...)
{
    char messageBuffer[1024] = { };

    va_list args;
    va_start(args, msg);
    vsnprintf_s(messageBuffer, 1024 - 1, 1024 - 1, msg, args);

    MessageBoxA(nullptr, messageBuffer, "DXL", MB_OK | MB_ICONERROR);
}

static void PrintAssertMessage(const char* condition, const char* msg, const char* file, const int32_t line)
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

static void AssertHandler(const char* condition, const char* file, const int32_t line, const char* msg, ...)
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

#define DXL_ASSERT(cond, msg, ...) \
    do \
    { \
        if (!(cond)) \
        { \
            AssertHandler(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__); \
            __debugbreak(); \
        } \
    } while(0)

#if DXL_ENABLE_EXTENSIONS

struct WideStringConverter
{
    wchar_t* wideString = nullptr;
    wchar_t fixedStorage[512] = { };
    wchar_t* allocatedString = nullptr;

    WideStringConverter(const char* str)
    {
        if (str != nullptr)
        {
            wideString = fixedStorage;

            if (str[0] != 0)
            {
                const int32_t numChars = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
                if (numChars > (DXL_ARRAY_SIZE(fixedStorage)))

                {
                    allocatedString = new wchar_t[numChars];
                    wideString = allocatedString;
                }

                MultiByteToWideChar(CP_UTF8, 0, str, -1, wideString, numChars);
            }
        }
    }

    ~WideStringConverter()
    {
        if (allocatedString)
        {
            delete[] allocatedString;
            allocatedString = nullptr;
        }
    }
};

static void DefaultErrorCallback(const char* function, HRESULT hr, const char* message)
{
    char hrString[256] = { };
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), hrString, DXL_ARRAY_SIZE(hrString), nullptr);
    size_t hrStringLen = strlen(hrString);
    if (hrString[hrStringLen - 1] == '\n')
        hrString[hrStringLen - 1] = 0;
    if (hrString[hrStringLen - 2] == '\r')
        hrString[hrStringLen - 2] = 0;

    if (message && message[0])
    {
        PrintMessage("Function '%s' failed with HRESULT 0x%x (%s): %s", function, hr, hrString, message);
        if (IsDebuggerPresent() == false)
            ShowMessageBox("Function '%s' failed with HRESULT 0x%x (%s): %s", function, hr, hrString, message);
    }
    else
    {
        PrintMessage("Function '%s' failed with HRESULT 0x%x (%s)", function, hr, hrString, message);
        if (IsDebuggerPresent() == false)
            ShowMessageBox("Function '%s' failed with HRESULT 0x%x (%s)", function, hr, hrString, message);
    }

    __debugbreak();
}

static ErrorCallbackFunction errorCallback = DefaultErrorCallback;

void SetErrorCallback(ErrorCallbackFunction callback)
{
    errorCallback = callback;
}

#define DXL_ERROR(function, hr, msg) do { if (errorCallback) errorCallback(function, hr, msg); } while(0)
#define DXL_HANDLE_HRESULT(hr) do { if (FAILED(hr) && errorCallback) errorCallback(__FUNCTION__, hr, ""); } while(0)
#define DXL_HANDLE_HRESULT_MSG(hr, msg) do { if (FAILED(hr) && errorCallback) errorCallback(__FUNCTION__, hr, msg); } while(0)

namespace Helpers
{

static D3D12_BLEND_DESC blendStateDescs[] =
{
    // BlendState::Opaque
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = false,
                .SrcBlend = D3D12_BLEND_SRC_ALPHA,
                .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
            }
        }
    },

    // BlendState::Additive
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = true,
                .SrcBlend = D3D12_BLEND_ONE,
                .DestBlend = D3D12_BLEND_ONE,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
            }
        }
    },

    // BlendState::AlphaBlend
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = true,
                .SrcBlend = D3D12_BLEND_SRC_ALPHA,
                .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
            }
        }
    },

    // BlendState::PreMultiplied
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = false,
                .SrcBlend = D3D12_BLEND_ONE,
                .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
            }
        }
    },

    // BlendState::NoColorWrites
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = false,
                .SrcBlend = D3D12_BLEND_SRC_ALPHA,
                .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = 0,
            }
        }
    },

    // BlendState::PreMultipliedRGB
    {
        .IndependentBlendEnable = false,
        .RenderTarget =
        {
            {
                .BlendEnable = true,
                .SrcBlend = D3D12_BLEND_ONE,
                .DestBlend = D3D12_BLEND_INV_SRC1_COLOR,
                .BlendOp = D3D12_BLEND_OP_ADD,
                .SrcBlendAlpha = D3D12_BLEND_ONE,
                .DestBlendAlpha = D3D12_BLEND_ONE,
                .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
            }
        }
    }
};
static_assert(DXL_ARRAY_SIZE(blendStateDescs) == uint32_t(BlendState::NumValues));

static D3D12_RASTERIZER_DESC2 rasterizerStateDescs[] =
{
    // RasterizerState::NoCull
    {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_NONE,
        .DepthClipEnable = true,
    },

    // RasterizerState::NoCullNoZClip
    {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_NONE,
        .DepthClipEnable = false,
    },

    // RasterizerState::FrontFaceCull
    {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_FRONT,
        .DepthClipEnable = true,
    },

    // RasterizerState::BackFaceCull
    {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_BACK,
        .DepthClipEnable = true,
    },

    // RasterizerState::BackFaceCullNoZClip
    {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_BACK,
        .DepthClipEnable = false,
    },

    // RasterizerState::Wireframe
    {
        .FillMode = D3D12_FILL_MODE_WIREFRAME,
        .CullMode = D3D12_CULL_MODE_NONE,
        .DepthClipEnable = true,
    },
};
static_assert(DXL_ARRAY_SIZE(rasterizerStateDescs) == uint32_t(RasterizerState::NumValues));

static D3D12_DEPTH_STENCIL_DESC2 depthStateDescs[] =
{
    // DepthState::Disabled
    {
        .DepthEnable = false,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    },

    // DepthState::Enabled
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    },

    // DepthState::Reversed
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
    },

    // DepthState::WritesEnabled
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    },

    // DepthState::ReversedWritesEnabled
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
    },

    // DepthState::Equal
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_EQUAL,
    },

    // DepthState::DepthFail
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_GREATER,
    },

    // DepthState::DepthFailReversed
    {
        .DepthEnable = true,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
    },
};
static_assert(DXL_ARRAY_SIZE(depthStateDescs) == uint32_t(DepthState::NumValues));

D3D12_BLEND_DESC BlendStateDesc(BlendState blendState)
{
    DXL_ASSERT(uint32_t(blendState) < uint32_t(BlendState::NumValues), "Invalid BlendState %u", uint32_t(blendState));
    return blendStateDescs[uint32_t(blendState)];
}

D3D12_RASTERIZER_DESC2 RasterizerStateDesc(RasterizerState rasterizerState)
{
    DXL_ASSERT(uint32_t(rasterizerState) < uint32_t(RasterizerState::NumValues), "Invalid RasterizerState %u", uint32_t(rasterizerState));
    return rasterizerStateDescs[uint32_t(rasterizerState)];
}

D3D12_DEPTH_STENCIL_DESC2 DepthStateDesc(DepthState depthState)
{
    DXL_ASSERT(uint32_t(depthState) < uint32_t(DepthState::NumValues), "Invalid DepthState %u", uint32_t(depthState));
    return depthStateDescs[uint32_t(depthState)];
}

} // namespace Helpers

#endif // DXL_ENABLE_EXTENSIONS

// == IDXLBase ======================================================

HRESULT IDXLBase::QueryInterface(REFIID riid, void** outObject)
{
    return ToNative()->QueryInterface(riid, outObject);
}

uint32_t IDXLBase::AddRef()
{
    return ToNative()->AddRef();
}

uint32_t IDXLBase::Release()
{
    return ToNative()->Release();
}

// == IDXLObject ======================================================

HRESULT IDXLObject::GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const
{
    return ToNative()->GetPrivateData(guid, outDataSize, outData);
}

HRESULT IDXLObject::SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data)
{
    return ToNative()->SetPrivateData(guid, dataSize, data);
}

HRESULT IDXLObject::SetPrivateDataInterface(REFGUID guid, const IUnknown* data)
{
    return ToNative()->SetPrivateDataInterface(guid, data);
}

HRESULT IDXLObject::SetName(const wchar_t* name)
{
    return ToNative()->SetName(name);
}

#if DXL_ENABLE_EXTENSIONS

HRESULT IDXLObject::SetName(const char* name)
{
    return ToNative()->SetName(WideStringConverter(name).wideString);
}

#endif

// == IDXLDeviceChild ======================================================

HRESULT IDXLDeviceChild::GetDevice(REFIID riid, void** outDevice)
{
    return ToNative()->GetDevice(riid, outDevice);
}

// == IDXLHeap ======================================================

D3D12_HEAP_DESC IDXLHeap::GetDesc() const
{
    return ToNative()->GetDesc();
}

// == IDXLResource ======================================================

HRESULT IDXLResource::Map(uint32_t subresource, const D3D12_RANGE* readRange, void** outData)
{
    return ToNative()->Map(subresource, readRange, outData);
}

void IDXLResource::Unmap(uint32_t subresource, const D3D12_RANGE* writtenRange)
{
    ToNative()->Unmap(subresource, writtenRange);
}

#if DXL_ENABLE_EXTENSIONS

void* IDXLResource::Map(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToNative()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);

    void* data = nullptr;
    DXL_HANDLE_HRESULT(ToNative()->Map(subresourceIndex, nullptr, &data));
    return data;
}

void IDXLResource::Unmap(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToNative()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);
    ToNative()->Unmap(subresourceIndex, nullptr);
}

#endif

D3D12_RESOURCE_DESC1 IDXLResource::GetDesc1() const
{
    return ToNative()->GetDesc1();
}

D3D12_GPU_VIRTUAL_ADDRESS IDXLResource::GetGPUVirtualAddress() const
{
    return ToNative()->GetGPUVirtualAddress();
}

HRESULT IDXLResource::WriteToSubresource(uint32_t dstSubresource, const D3D12_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
{
    return ToNative()->WriteToSubresource(dstSubresource, dstBox, srcData, srcRowPitch, srcDepthPitch);
}

HRESULT IDXLResource::ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const D3D12_BOX* srcBox) const
{
    return ToNative()->ReadFromSubresource(dstData, dstRowPitch, dstDepthPitch, srcSubresource, srcBox);
}

HRESULT IDXLResource::GetHeapProperties(D3D12_HEAP_PROPERTIES* outHeapProperties, D3D12_HEAP_FLAGS* outHeapFlags) const
{
    return ToNative()->GetHeapProperties(outHeapProperties, outHeapFlags);
}

// == IDXLCommandAllocator ======================================================

HRESULT IDXLCommandAllocator::Reset()
{
    return ToNative()->Reset();
}

// == IDXLFence ======================================================

uint64_t IDXLFence::GetCompletedValue() const
{
    return ToNative()->GetCompletedValue();
}

HRESULT IDXLFence::SetEventOnCompletion(uint64_t value, HANDLE event)
{
    return ToNative()->SetEventOnCompletion(value, event);
}

HRESULT IDXLFence::Signal(uint64_t value)
{
    return ToNative()->Signal(value);
}

D3D12_FENCE_FLAGS IDXLFence::GetCreationFlags() const
{
    return ToNative()->GetCreationFlags();
}

#if DXL_ENABLE_EXTENSIONS

bool IDXLFence::WaitWithEvent(uint64_t value, HANDLE event, uint32_t timeout)
{
    if (ToNative()->GetCompletedValue() < value)
    {
        DXL_HANDLE_HRESULT(ToNative()->SetEventOnCompletion(value, event));
        return WaitForSingleObject(event, timeout) == WAIT_OBJECT_0;
    }
    else
    {
        return true;
    }
}

#endif

// == IDXLPipelineState ======================================================

/*HRESULT IDXLPipelineState::GetRootSignature(REFIID riid, void** outRootSignature) const
{
    return ToNative()->GetRootSignature(riid, outRootSignature);
}

#if DXL_ENABLE_EXTENSIONS

DXLRootSignature IDXLPipelineState::GetRootSignature() const
{
    ID3D12RootSignature* rootSig = nullptr;
    DXL_HANDLE_HRESULT_MSG(GetRootSignature(IID_PPV_ARGS(&rootSig)));
    return DXLRootSignature(rootSig);
}

#endif*/

// == IDXLStateObjectProperties =====================================================

void* IDXLStateObjectProperties::GetShaderIdentifier(const wchar_t* exportName)
{
    return ToNative()->GetShaderIdentifier(exportName);
}

uint64_t IDXLStateObjectProperties::GetShaderStackSize(const wchar_t* exportName)
{
    return ToNative()->GetShaderStackSize(exportName);
}

HRESULT IDXLStateObjectProperties::GetGlobalRootSignatureForProgram(const wchar_t* programName, REFIID riid, void** outRootSignature)
{
    return ToNative()->GetGlobalRootSignatureForProgram(programName, riid, outRootSignature);
}
HRESULT IDXLStateObjectProperties::GetGlobalRootSignatureForShader(const wchar_t* exportName, REFIID riid, void** outRootSignature)
{
    return ToNative()->GetGlobalRootSignatureForShader(exportName, riid, outRootSignature);
}

#if DXL_ENABLE_EXTENSIONS

D3D12_PROGRAM_IDENTIFIER IDXLStateObjectProperties::GetProgramIdentifier(const char* programName)
{
    return ToNative()->GetProgramIdentifier(WideStringConverter(programName).wideString);
}

void* IDXLStateObjectProperties::GetShaderIdentifier(const char* exportName)
{
    return ToNative()->GetShaderIdentifier(WideStringConverter(exportName).wideString);
}

uint64_t IDXLStateObjectProperties::GetShaderStackSize(const char* exportName)
{
    return ToNative()->GetShaderStackSize(WideStringConverter(exportName).wideString);
}

IDXLRootSignature IDXLStateObjectProperties::GetGlobalRootSignatureForProgram(const char* programName)
{
    ID3D12RootSignature* rootSig = nullptr;
    DXL_HANDLE_HRESULT(ToNative()->GetGlobalRootSignatureForProgram(WideStringConverter(programName).wideString, IID_PPV_ARGS(&rootSig)));
    return IDXLRootSignature(rootSig);
}

IDXLRootSignature IDXLStateObjectProperties::GetGlobalRootSignatureForShader(const char* exportName)
{
    ID3D12RootSignature* rootSig = nullptr;
    DXL_HANDLE_HRESULT(ToNative()->GetGlobalRootSignatureForShader(WideStringConverter(exportName).wideString, IID_PPV_ARGS(&rootSig)));
    return IDXLRootSignature(rootSig);
}

#endif  // DXL_ENABLE_EXTENSIONS

uint64_t IDXLStateObjectProperties::GetPipelineStackSize()
{
    return ToNative()->GetPipelineStackSize();
}

void IDXLStateObjectProperties::SetPipelineStackSize(uint64_t pipelineStackSizeInBytes)
{
    return ToNative()->SetPipelineStackSize(pipelineStackSizeInBytes);
}

// == IDXLWorkGraphProperties =====================================================

uint32_t IDXLWorkGraphProperties::GetNumWorkGraphs()
{
    return ToNative()->GetNumWorkGraphs();
}

const wchar_t* IDXLWorkGraphProperties::GetProgramName(uint32_t workGraphIndex)
{
    return ToNative()->GetProgramName(workGraphIndex);
}

uint32_t IDXLWorkGraphProperties::GetWorkGraphIndex(const wchar_t* programName)
{
    return ToNative()->GetWorkGraphIndex(programName);
}

uint32_t IDXLWorkGraphProperties::GetNumNodes(uint32_t workGraphIndex)
{
    return ToNative()->GetNumNodes(workGraphIndex);
}

D3D12_NODE_ID IDXLWorkGraphProperties::GetNodeID(uint32_t workGraphIndex, uint32_t nodeIndex)
{
    return ToNative()->GetNodeID(workGraphIndex, nodeIndex);
}

uint32_t IDXLWorkGraphProperties::GetNodeIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID)
{
    return ToNative()->GetNodeIndex(workGraphIndex, nodeID);
}

uint32_t IDXLWorkGraphProperties::GetNodeLocalRootArgumentsTableIndex(uint32_t workGraphIndex, uint32_t nodeIndex)
{
    return ToNative()->GetNodeLocalRootArgumentsTableIndex(workGraphIndex, nodeIndex);
}

uint32_t IDXLWorkGraphProperties::GetNumEntrypoints(uint32_t workGraphIndex)
{
    return ToNative()->GetNumEntrypoints(workGraphIndex);
}

D3D12_NODE_ID IDXLWorkGraphProperties::GetEntrypointID(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointID(workGraphIndex, entrypointIndex);
}

uint32_t IDXLWorkGraphProperties::GetEntrypointIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID)
{
    return ToNative()->GetEntrypointIndex(workGraphIndex, nodeID);
}

uint32_t IDXLWorkGraphProperties::GetEntrypointRecordSizeInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointRecordSizeInBytes(workGraphIndex, entrypointIndex);
}

void IDXLWorkGraphProperties::GetWorkGraphMemoryRequirements(uint32_t workGraphIndex, D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS* outWorkGraphMemoryRequirements)
{
    ToNative()->GetWorkGraphMemoryRequirements(workGraphIndex, outWorkGraphMemoryRequirements);
}

uint32_t IDXLWorkGraphProperties::GetEntrypointRecordAlignmentInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointRecordAlignmentInBytes(workGraphIndex, entrypointIndex);
}

// == IDXLDescriptorHeap =====================================================

D3D12_DESCRIPTOR_HEAP_DESC IDXLDescriptorHeap::GetDesc()
{
    return ToNative()->GetDesc();
}

D3D12_CPU_DESCRIPTOR_HANDLE IDXLDescriptorHeap::GetCPUDescriptorHandleForHeapStart()
{
    return ToNative()->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE IDXLDescriptorHeap::GetGPUDescriptorHandleForHeapStart()
{
    return ToNative()->GetGPUDescriptorHandleForHeapStart();
}

// == IDXLCommandList =====================================================

D3D12_COMMAND_LIST_TYPE IDXLCommandList::GetType() const
{
    return ToNative()->GetType();
}

HRESULT IDXLCommandList::Close()
{
    return ToNative()->Close();
}

HRESULT IDXLCommandList::Reset(IDXLCommandAllocator allocator, IDXLPipelineState pipelineState)
{
    return ToNative()->Reset(allocator, pipelineState);
}

void IDXLCommandList::ClearState(IDXLPipelineState pipelineState)
{
    ToNative()->ClearState(pipelineState);
}

void IDXLCommandList::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
    ToNative()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void IDXLCommandList::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
    ToNative()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void IDXLCommandList::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    ToNative()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void IDXLCommandList::DispatchRays(const D3D12_DISPATCH_RAYS_DESC* desc)
{
    ToNative()->DispatchRays(desc);
}

void IDXLCommandList::DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    ToNative()->DispatchMesh(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void IDXLCommandList::DispatchGraph(const D3D12_DISPATCH_GRAPH_DESC* desc)
{
    ToNative()->DispatchGraph(desc);
}

void IDXLCommandList::CopyBufferRegion(IDXLResource dstBuffer, uint64_t dstOffset, IDXLResource srcBuffer, uint64_t srcOffset, uint64_t numBytes)
{
    ToNative()->CopyBufferRegion(dstBuffer, dstOffset, srcBuffer, srcOffset, numBytes);
}

void IDXLCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* dst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* srcBox)
{
    ToNative()->CopyTextureRegion(dst, dstX, dstY, dstZ, src, srcBox);
}

void IDXLCommandList::CopyResource(IDXLResource dstResource, IDXLResource srcResource)
{
    ToNative()->CopyResource(dstResource, srcResource);
}

void IDXLCommandList::CopyTiles(IDXLResource tiledResource, const D3D12_TILED_RESOURCE_COORDINATE* tileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* tileRegionSize, IDXLResource buffer, uint64_t bufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS flags)
{
    ToNative()->CopyTiles(tiledResource, tileRegionStartCoordinate, tileRegionSize, buffer, bufferStartOffsetInBytes, flags);
}

void IDXLCommandList::Barrier(uint32_t numBarrierGroups, const D3D12_BARRIER_GROUP* barrierGroups)
{
    ToNative()->Barrier(numBarrierGroups, barrierGroups);
}

#if DXL_ENABLE_EXTENSIONS

void IDXLCommandList::Barrier(D3D12_GLOBAL_BARRIER barrier)
{
    const D3D12_BARRIER_GROUP group =
    {
        .Type = D3D12_BARRIER_TYPE_GLOBAL,
        .NumBarriers = 1,
        .pGlobalBarriers = &barrier,
    };
    ToNative()->Barrier(1, &group);
}

void IDXLCommandList::Barrier(D3D12_BUFFER_BARRIER barrier)
{
    const D3D12_BARRIER_GROUP group =
    {
        .Type = D3D12_BARRIER_TYPE_BUFFER,
        .NumBarriers = 1,
        .pBufferBarriers = &barrier,
    };
    ToNative()->Barrier(1, &group);
}

void IDXLCommandList::Barrier(D3D12_TEXTURE_BARRIER barrier)
{
    const D3D12_BARRIER_GROUP group =
    {
        .Type = D3D12_BARRIER_TYPE_TEXTURE,
        .NumBarriers = 1,
        .pTextureBarriers = &barrier,
    };
    ToNative()->Barrier(1, &group);
}

#endif // DXL_ENABLE_EXTENSIONS

void IDXLCommandList::ResolveSubresource(IDXLResource dstResource, uint32_t dstSubresource, IDXLResource srcResource, uint32_t srcSubresource, DXGI_FORMAT format)
{
    ToNative()->ResolveSubresource(dstResource, dstSubresource, srcResource, srcSubresource, format);
}

void IDXLCommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    ToNative()->IASetPrimitiveTopology(primitiveTopology);
}

void IDXLCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view)
{
    ToNative()->IASetIndexBuffer(view);
}

#if DXL_ENABLE_EXTENSIONS

void IDXLCommandList::IASetIndexBuffer(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, uint32_t sizeInBytes, DXGI_FORMAT format)
{
    D3D12_INDEX_BUFFER_VIEW d3d12View = { .BufferLocation = bufferLocation, .SizeInBytes = sizeInBytes, .Format = format };
    ToNative()->IASetIndexBuffer(&d3d12View);
}

#endif

void IDXLCommandList::IASetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibStripCutValue)
{
    ToNative()->IASetIndexBufferStripCutValue(ibStripCutValue);
}

void IDXLCommandList::RSSetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports)
{
    ToNative()->RSSetViewports(numViewports, viewports);
}

void IDXLCommandList::RSSetScissorRects(uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->RSSetScissorRects(numRects, rects);
}

void IDXLCommandList::RSSetDepthBias(float depthBias, float depthBiasClamp, float slopeScaledDepthBias)
{
    ToNative()->RSSetDepthBias(depthBias, depthBiasClamp, slopeScaledDepthBias);
}

#if DXL_ENABLE_EXTENSIONS

void IDXLCommandList::RSSetViewportAndScissor(uint32_t width, uint32_t height)
{
    D3D12_VIEWPORT viewport =
    {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = float(width),
        .Height = float(height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    D3D12_RECT scissorRect =
    {
        scissorRect.left = 0,
        scissorRect.top = 0,
        scissorRect.right = uint32_t(width),
        scissorRect.bottom = uint32_t(height),
    };

    ToNative()->RSSetViewports(1, &viewport);
    ToNative()->RSSetScissorRects(1, &scissorRect);
}

#endif

void IDXLCommandList::RSSetShadingRate(D3D12_SHADING_RATE baseShadingRate, const D3D12_SHADING_RATE_COMBINER* combiners)
{
    ToNative()->RSSetShadingRate(baseShadingRate, combiners);
}

void IDXLCommandList::RSSetShadingRateImage(IDXLResource shadingRateImage)
{
    ToNative()->RSSetShadingRateImage(shadingRateImage);
}

void IDXLCommandList::OMSetBlendFactor(const float blendFactor[4])
{
    ToNative()->OMSetBlendFactor(blendFactor);
}

void IDXLCommandList::OMSetStencilRef(uint32_t stencilRef)
{
    ToNative()->OMSetStencilRef(stencilRef);
}

void IDXLCommandList::OMSetFrontAndBackStencilRef(uint32_t frontStencilRef, uint32_t backStencilRef)
{
    ToNative()->OMSetFrontAndBackStencilRef(frontStencilRef, backStencilRef);
}

void IDXLCommandList::SetPipelineState(IDXLPipelineState pipelineState)
{
    ToNative()->SetPipelineState(pipelineState);
}

void IDXLCommandList::SetPipelineState1(IDXLStateObject stateObject)
{
    ToNative()->SetPipelineState1(stateObject);
}

void IDXLCommandList::SetProgram(const D3D12_SET_PROGRAM_DESC* desc)
{
    ToNative()->SetProgram(desc);
}

void IDXLCommandList::SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap*const* descriptorHeaps)
{
    ToNative()->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}

#if DXL_ENABLE_EXTENSIONS

void IDXLCommandList::SetDescriptorHeaps(IDXLDescriptorHeap srvUavCbvHeap, IDXLDescriptorHeap samplerHeap)
{
    ID3D12DescriptorHeap* heaps[] = { srvUavCbvHeap, samplerHeap };
    ToNative()->SetDescriptorHeaps(samplerHeap ? 2 : 1, heaps);
}

#endif

void IDXLCommandList::SetComputeRootSignature(IDXLRootSignature rootSignature)
{
    ToNative()->SetComputeRootSignature(rootSignature);
}

void IDXLCommandList::SetGraphicsRootSignature(IDXLRootSignature rootSignature)
{
    ToNative()->SetGraphicsRootSignature(rootSignature);
}

#if DXL_ENABLE_DESCRIPTOR_TABLES

void IDXLCommandList::SetComputeRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToNative()->SetComputeRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

void IDXLCommandList::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToNative()->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

#endif

void IDXLCommandList::SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetComputeRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void IDXLCommandList::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetGraphicsRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void IDXLCommandList::SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void IDXLCommandList::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void IDXLCommandList::SetComputeRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::SetComputeRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void IDXLCommandList::OMSetRenderTargets(uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, bool rtIsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
{
    ToNative()->OMSetRenderTargets(numRenderTargetDescriptors, renderTargetDescriptors, rtIsSingleHandleToDescriptorRange, depthStencilDescriptor);
}

void IDXLCommandList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags,float depth, uint8_t stencil, uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, rects);
}

void IDXLCommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, rects);
}

void IDXLCommandList::DiscardResource(IDXLResource resource, const D3D12_DISCARD_REGION* region)
{
    ToNative()->DiscardResource(resource, region);
}

void IDXLCommandList::BeginRenderPass(uint32_t numRenderTargets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* renderTargets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depthStencil, D3D12_RENDER_PASS_FLAGS flags)
{
    ToNative()->BeginRenderPass(numRenderTargets, renderTargets, depthStencil, flags);
}

void IDXLCommandList::EndRenderPass()
{
    ToNative()->EndRenderPass();
}

#if DXL_ENABLE_CLEAR_UAV

void IDXLCommandList::ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, IDXLResource resource, const uint32_t values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearUnorderedAccessViewUint(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

void IDXLCommandList::ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, IDXLResource resource, const float values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearUnorderedAccessViewFloat(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

#endif

void IDXLCommandList::BeginQuery(IDXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToNative()->BeginQuery(queryHeap, type, index);
}

void IDXLCommandList::EndQuery(IDXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToNative()->EndQuery(queryHeap, type, index);
}

void IDXLCommandList::ResolveQueryData(IDXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t startIndex, uint32_t numQueries, IDXLResource destinationBuffer, uint64_t alignedDestinationBufferOffset)
{
    ToNative()->ResolveQueryData(queryHeap, type, startIndex, numQueries, destinationBuffer, alignedDestinationBufferOffset);
}

void IDXLCommandList::SetPredication(IDXLResource buffer, uint64_t alignedBufferOffset, D3D12_PREDICATION_OP operation)
{
    ToNative()->SetPredication(buffer, alignedBufferOffset, operation);
}

void IDXLCommandList::ExecuteIndirect(IDXLCommandSignature commandSignature, uint32_t maxCommandCount, IDXLResource argumentBuffer, uint64_t argumentBufferOffset, IDXLResource countBuffer, uint64_t countBufferOffset)
{
    ToNative()->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

void IDXLCommandList::AtomicCopyBufferUINT(ID3D12Resource* dstBuffer, uint64_t dstOffset, ID3D12Resource* srcBuffer, uint64_t srcOffset, uint32_t dependencies, ID3D12Resource*const* dependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges)
{
    ToNative()->AtomicCopyBufferUINT(dstBuffer, dstOffset, srcBuffer, srcOffset, dependencies, dependentResources, dependentSubresourceRanges);
}

// UINT64 is only valid on UMA architectures
void IDXLCommandList::AtomicCopyBufferUINT64(IDXLResource dstBuffer, uint64_t dstOffset, IDXLResource srcBuffer, uint64_t srcOffset, uint32_t dependencies, ID3D12Resource*const* dependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges)
{
    ToNative()->AtomicCopyBufferUINT64(dstBuffer, dstOffset, srcBuffer, srcOffset, dependencies, dependentResources, dependentSubresourceRanges);
}

void IDXLCommandList::OMSetDepthBounds(float min, float max)
{
    ToNative()->OMSetDepthBounds(min, max);
}

void IDXLCommandList::SetSamplePositions(uint32_t numSamplesPerPixel, uint32_t numPixels, D3D12_SAMPLE_POSITION* samplePositions)
{
    ToNative()->SetSamplePositions(numSamplesPerPixel, numPixels, samplePositions);
}

void IDXLCommandList::ResolveSubresourceRegion(IDXLResource dstResource, uint32_t dstSubresource, uint32_t dstX, uint32_t dstY, IDXLResource srcResource, uint32_t srcSubresource, D3D12_RECT* srcRect, DXGI_FORMAT format, D3D12_RESOLVE_MODE resolveMode)
{
    ToNative()->ResolveSubresourceRegion(dstResource, dstSubresource, dstX, dstY, srcResource, srcSubresource, srcRect, format, resolveMode);
}

#if DXL_ENABLE_VIEW_INSTANCING
void IDXLCommandList::SetViewInstanceMask(uint32_t mask)
{
    ToNative()->SetViewInstanceMask(mask);
}
#endif

void IDXLCommandList::WriteBufferImmediate(uint32_t count, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER* params, const D3D12_WRITEBUFFERIMMEDIATE_MODE* modes)
{
    ToNative()->WriteBufferImmediate(count, params, modes);
}

void IDXLCommandList::BuildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* desc, uint32_t numPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* postbuildInfoDescs)
{
    ToNative()->BuildRaytracingAccelerationStructure(desc, numPostbuildInfoDescs, postbuildInfoDescs);
}

void IDXLCommandList::EmitRaytracingAccelerationStructurePostbuildInfo(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* desc, uint32_t numSourceAccelerationStructures, const D3D12_GPU_VIRTUAL_ADDRESS* sourceAccelerationStructureData)
{
    ToNative()->EmitRaytracingAccelerationStructurePostbuildInfo(desc, numSourceAccelerationStructures, sourceAccelerationStructureData);
}

void IDXLCommandList::CopyRaytracingAccelerationStructure(D3D12_GPU_VIRTUAL_ADDRESS destAccelerationStructureData, D3D12_GPU_VIRTUAL_ADDRESS sourceAccelerationStructureData, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE mode)
{
    ToNative()->CopyRaytracingAccelerationStructure(destAccelerationStructureData, sourceAccelerationStructureData, mode);
}

// == IDXLCommandQueue ======================================================

void IDXLCommandQueue::UpdateTileMappings(IDXLResource resource, uint32_t numResourceRegions, const D3D12_TILED_RESOURCE_COORDINATE* resourceRegionStartCoordinates, const D3D12_TILE_REGION_SIZE* resourceRegionSizes, ID3D12Heap* pHeap, uint32_t numRanges, const D3D12_TILE_RANGE_FLAGS* rangeFlags, const uint32_t* heapRangeStartOffsets, const uint32_t* rangeTileCounts, D3D12_TILE_MAPPING_FLAGS flags)
{
    ToNative()->UpdateTileMappings(resource, numResourceRegions, resourceRegionStartCoordinates, resourceRegionSizes, pHeap, numRanges, rangeFlags, heapRangeStartOffsets, rangeTileCounts, flags);
}

void IDXLCommandQueue::CopyTileMappings(IDXLResource dstResource, const D3D12_TILED_RESOURCE_COORDINATE* dstRegionStartCoordinate, IDXLResource srcResource, const D3D12_TILED_RESOURCE_COORDINATE* srcRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* regionSize, D3D12_TILE_MAPPING_FLAGS flags)
{
    ToNative()->CopyTileMappings(dstResource, dstRegionStartCoordinate, srcResource, srcRegionStartCoordinate, regionSize, flags);
}

void IDXLCommandQueue::ExecuteCommandLists(uint32_t numCommandLists, ID3D12CommandList*const* commandLists)
{
    ToNative()->ExecuteCommandLists(numCommandLists, commandLists);
}

HRESULT IDXLCommandQueue::Signal(IDXLFence fence, uint64_t value)
{
    return ToNative()->Signal(fence, value);
}

HRESULT IDXLCommandQueue::Wait(IDXLFence fence, uint64_t value)
{
    return ToNative()->Wait(fence, value);
}

HRESULT IDXLCommandQueue::GetTimestampFrequency(uint64_t* outFrequency) const
{
    return ToNative()->GetTimestampFrequency(outFrequency);
}

HRESULT IDXLCommandQueue::GetClockCalibration(uint64_t* outGpuTimestamp, uint64_t* outCpuTimestamp) const
{
    return ToNative()->GetClockCalibration(outGpuTimestamp, outCpuTimestamp);
}

D3D12_COMMAND_QUEUE_DESC IDXLCommandQueue::GetDesc() const
{
    return ToNative()->GetDesc();
}

/*HRESULT IDXLCommandQueue::SetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY priority)
{
    return ToNative()->SetProcessPriority(priority);
}

HRESULT IDXLCommandQueue::GetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY* outValue)
{
    return ToNative()->GetProcessPriority(outValue);
}

HRESULT IDXLCommandQueue::SetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY priority)
{
    return ToNative()->SetGlobalPriority(priority);
}

HRESULT IDXLCommandQueue::GetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY* outValue)
{
    return ToNative()->GetGlobalPriority(outValue);
}*/

// == IDXLDevice ======================================================

uint32_t IDXLDevice::GetNodeCount()
{
    return ToNative()->GetNodeCount();
}

LUID IDXLDevice::GetAdapterLuid()
{
    return ToNative()->GetAdapterLuid();
}

HRESULT IDXLDevice::CheckFeatureSupport(D3D12_FEATURE feature, void* featureSupportData, uint32_t featureSupportDataSize)
{
    return ToNative()->CheckFeatureSupport(feature, featureSupportData, featureSupportDataSize);
}

HRESULT IDXLDevice::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* desc, REFIID riid, void** outCommandQueue)
{
    return ToNative()->CreateCommandQueue(desc, riid, outCommandQueue);
}

HRESULT IDXLDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** outCommandAllocator)
{
    return ToNative()->CreateCommandAllocator(type, riid, outCommandAllocator);
}

HRESULT IDXLDevice::CreateCommandList1(uint32_t nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void** outCommandList)
{
    return ToNative()->CreateCommandList1(nodeMask, type, flags, riid, outCommandList);
}

#if DXL_ENABLE_EXTENSIONS

IDXLCommandQueue IDXLDevice::CreateCommandQueue(D3D12_COMMAND_QUEUE_DESC desc)
{
    IDXLCommandQueue commandQueue;
    DXL_HANDLE_HRESULT(ToNative()->CreateCommandQueue(&desc, DXL_PPV_ARGS(&commandQueue)));
    return commandQueue;
}

IDXLCommandAllocator IDXLDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    IDXLCommandAllocator commandAllocator;
    DXL_HANDLE_HRESULT(ToNative()->CreateCommandAllocator(type, DXL_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

IDXLCommandList IDXLDevice::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags)
{
    IDXLCommandList commandList;
    DXL_HANDLE_HRESULT(ToNative()->CreateCommandList1(0, type, flags, DXL_PPV_ARGS(&commandList)));
    return commandList;
}

#endif // DXL_ENABLE_EXTENSIONS

HRESULT IDXLDevice::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc, REFIID riid, void** outPipelineState)
{
    return ToNative()->CreateComputePipelineState(desc, riid, outPipelineState);
}

HRESULT IDXLDevice::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* desc, REFIID riid, void** outPipelineState)
{
    return ToNative()->CreatePipelineState(desc, riid, outPipelineState);
}

HRESULT IDXLDevice::CreateStateObject(const D3D12_STATE_OBJECT_DESC* desc, REFIID riid, void** outStateObject)
{
    return ToNative()->CreateStateObject(desc, riid, outStateObject);
}

HRESULT IDXLDevice::AddToStateObject(const D3D12_STATE_OBJECT_DESC* addition, IDXLStateObject stateObjectToGrowFrom, REFIID riid, void** outNewStateObject)
{
    return ToNative()->AddToStateObject(addition, stateObjectToGrowFrom, riid, outNewStateObject);
}

#if DXL_ENABLE_EXTENSIONS

IDXLPipelineState IDXLDevice::CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC desc)
{
    IDXLPipelineState pso;
    DXL_HANDLE_HRESULT(ToNative()->CreateComputePipelineState(&desc, DXL_PPV_ARGS(&pso)));
    return pso;
}

IDXLPipelineState IDXLDevice::CreateComputePSO(IDXLRootSignature rootSignature, const void* byteCode, size_t byteCodeLength)
{
    return CreateComputePSO({ .pRootSignature = rootSignature, .CS = { byteCode, byteCodeLength } });
}

IDXLPipelineState IDXLDevice::CreateGraphicsPSO(D3D12_PIPELINE_STATE_STREAM_DESC desc)
{
    IDXLPipelineState pso;
    DXL_HANDLE_HRESULT(ToNative()->CreatePipelineState(&desc, DXL_PPV_ARGS(&pso)));
    return pso;
}

IDXLPipelineState IDXLDevice::CreateGraphicsPSO(DXL_SIMPLE_GRAPHICS_PSO_DESC desc)
{
    CD3DX12_PIPELINE_STATE_STREAM6 stateStream;
    stateStream.pRootSignature = desc.RootSignature;
    stateStream.PrimitiveTopologyType = desc.PrimitiveTopologyType;
    stateStream.VS = desc.VertexShaderByteCode;
    stateStream.PS = desc.PixelShaderByteCode;
    stateStream.BlendState = CD3DX12_BLEND_DESC(desc.BlendState);
    stateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(desc.DepthStencilState);
    stateStream.DSVFormat = desc.DepthStencilFormat;
    stateStream.RasterizerState = CD3DX12_RASTERIZER_DESC2(desc.RasterizerState);
    stateStream.RTVFormats = desc.RenderTargetFormats;

    return CreateGraphicsPSO({ .SizeInBytes = sizeof(stateStream), .pPipelineStateSubobjectStream = &stateStream, });
}

IDXLPipelineState IDXLDevice::CreateGraphicsPSO(DXL_MESH_SHADER_GRAPHICS_PSO_DESC desc)
{
    CD3DX12_PIPELINE_STATE_STREAM6 stateStream;
    stateStream.pRootSignature = desc.RootSignature;
    stateStream.PrimitiveTopologyType = desc.PrimitiveTopologyType;
    stateStream.AS = desc.AmplificationShaderByteCode;
    stateStream.MS = desc.MeshShaderByteCode;
    stateStream.PS = desc.PixelShaderByteCode;
    stateStream.BlendState = CD3DX12_BLEND_DESC(desc.BlendState);
    stateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(desc.DepthStencilState);
    stateStream.DSVFormat = desc.DepthStencilFormat;
    stateStream.RasterizerState = CD3DX12_RASTERIZER_DESC2(desc.RasterizerState);
    stateStream.RTVFormats = desc.RenderTargetFormats;

    return CreateGraphicsPSO({ .SizeInBytes = sizeof(stateStream), .pPipelineStateSubobjectStream = &stateStream, });
}

IDXLStateObject IDXLDevice::CreateStateObject(D3D12_STATE_OBJECT_DESC desc)
{
    IDXLStateObject  stateObject;
    DXL_HANDLE_HRESULT(ToNative()->CreateStateObject(&desc, DXL_PPV_ARGS(&stateObject)));
    return stateObject;
}

IDXLStateObject  IDXLDevice::AddToStateObject(D3D12_STATE_OBJECT_DESC addition, IDXLStateObject stateObjectToGrowFrom)
{
    IDXLStateObject  stateObject;
    DXL_HANDLE_HRESULT(ToNative()->AddToStateObject(&addition, stateObjectToGrowFrom.ToNative(), DXL_PPV_ARGS(&stateObjectToGrowFrom)));
    return stateObject;
}

#endif // DXL_ENABLE_EXTENSIONS

HRESULT IDXLDevice::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* descriptorHeapDesc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateDescriptorHeap(descriptorHeapDesc, riid, outHeap);
}

uint32_t IDXLDevice::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType)
{
    return ToNative()->GetDescriptorHandleIncrementSize(descriptorHeapType);
}

HRESULT IDXLDevice::CreateRootSignature(uint32_t nodeMask, const void* blobWithRootSignature, size_t blobLengthInBytes, REFIID riid, void** outRootSignature)
{
    return ToNative()->CreateRootSignature(nodeMask, blobWithRootSignature, blobLengthInBytes, riid, outRootSignature);
}

HRESULT IDXLDevice::CreateRootSignatureFromSubobjectInLibrary(uint32_t nodeMask, const void* libraryBlob, size_t blobLengthInBytes, const wchar_t* subobjectName, REFIID riid, void** rootSignature)
{
    return ToNative()->CreateRootSignatureFromSubobjectInLibrary(nodeMask, libraryBlob, blobLengthInBytes, subobjectName, riid, rootSignature);
}

HRESULT IDXLDevice::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* desc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateQueryHeap(desc, riid, outHeap);
}

HRESULT IDXLDevice::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* desc, IDXLRootSignature rootSignature, REFIID riid, void** outCommandSignature)
{
    return ToNative()->CreateCommandSignature(desc, rootSignature, riid, outCommandSignature);
}

#if DXL_ENABLE_EXTENSIONS

IDXLDescriptorHeap IDXLDevice::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc)
{
    IDXLDescriptorHeap descriptorHeap;
    DXL_HANDLE_HRESULT(ToNative()->CreateDescriptorHeap(&descriptorHeapDesc, DXL_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

IDXLRootSignature IDXLDevice::CreateRootSignature(D3D12_ROOT_SIGNATURE_DESC2 rootSignatureDesc)
{
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc =
    {
        .Version = D3D_ROOT_SIGNATURE_VERSION_1_2,
        .Desc_1_2 = rootSignatureDesc,
    };

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signature, &error);
    if (FAILED(hr))
    {
        const char* errString = error ? reinterpret_cast<const char*>(error->GetBufferPointer()) : "";
        DXL_HANDLE_HRESULT_MSG(hr, errString);
        return IDXLRootSignature();
    }

    IDXLRootSignature rootSig;
    DXL_HANDLE_HRESULT(ToNative()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), DXL_PPV_ARGS(&rootSig)));

    return rootSig;
}

IDXLQueryHeap IDXLDevice::CreateQueryHeap(D3D12_QUERY_HEAP_DESC desc)
{
    IDXLQueryHeap queryHeap;
    DXL_HANDLE_HRESULT(ToNative()->CreateQueryHeap(&desc, DXL_PPV_ARGS(&queryHeap)));
    return queryHeap;
}

IDXLCommandSignature IDXLDevice::CreateCommandSignature(D3D12_COMMAND_SIGNATURE_DESC desc, IDXLRootSignature rootSignature)
{
    IDXLCommandSignature commandSignature;
    DXL_HANDLE_HRESULT(ToNative()->CreateCommandSignature(&desc, rootSignature, DXL_PPV_ARGS(&commandSignature)));
    return commandSignature;
}

#endif // DXL_ENABLE_EXTENSIONS

void IDXLDevice::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateConstantBufferView(desc, destDescriptor);
}

void IDXLDevice::CreateShaderResourceView(IDXLResource resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateShaderResourceView(resource, desc, destDescriptor);
}

void IDXLDevice::CreateUnorderedAccessView(IDXLResource resource, IDXLResource counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateUnorderedAccessView(resource, counterResource, desc, destDescriptor);
}

void IDXLDevice::CreateSamplerFeedbackUnorderedAccessView(IDXLResource targetedResource, IDXLResource feedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateSamplerFeedbackUnorderedAccessView(targetedResource, feedbackResource, destDescriptor);
}

void IDXLDevice::CreateRenderTargetView(IDXLResource resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateRenderTargetView(resource, desc, destDescriptor);
}

void IDXLDevice::CreateDepthStencilView(IDXLResource resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateDepthStencilView(resource, desc, destDescriptor);
}

void IDXLDevice::CreateSampler2(const D3D12_SAMPLER_DESC2* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateSampler2(desc, destDescriptor);
}

D3D12_RESOURCE_ALLOCATION_INFO IDXLDevice::GetResourceAllocationInfo3(uint32_t visibleMask, uint32_t numResourceDescs, const D3D12_RESOURCE_DESC1* resourceDescs, const uint32_t* numCastableFormats, const DXGI_FORMAT*const* castableFormats, D3D12_RESOURCE_ALLOCATION_INFO1* resourceAllocationInfo)
{
    return ToNative()->GetResourceAllocationInfo3(visibleMask, numResourceDescs, resourceDescs, numCastableFormats, castableFormats, resourceAllocationInfo);
}

HRESULT IDXLDevice::CreateHeap(const D3D12_HEAP_DESC* desc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateHeap(desc, riid, outHeap);
}

HRESULT IDXLDevice::OpenExistingHeapFromAddress1(const void* address, size_t size, REFIID riid, void** outHeap)
{
    return ToNative()->OpenExistingHeapFromAddress1(address, size, riid, outHeap);
}

HRESULT IDXLDevice::OpenExistingHeapFromFileMapping(HANDLE fileMapping, REFIID riid, void** outHeap)
{
    return ToNative()->OpenExistingHeapFromFileMapping(fileMapping, riid, outHeap);
}

D3D12_HEAP_PROPERTIES IDXLDevice::GetCustomHeapProperties(uint32_t nodeMask, D3D12_HEAP_TYPE heapType)
{
    return ToNative()->GetCustomHeapProperties(nodeMask, heapType);
}

HRESULT IDXLDevice::CreateCommittedResource3(const D3D12_HEAP_PROPERTIES* heapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC1* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ID3D12ProtectedResourceSession* protectedSession, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreateCommittedResource3(heapProperties, heapFlags, desc, initialLayout, optimizedClearValue, protectedSession, numCastableFormats, castableFormats, riid, outResource);
}

HRESULT IDXLDevice::CreatePlacedResource2(IDXLHeap heap, uint64_t heapOffset, const D3D12_RESOURCE_DESC1* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreatePlacedResource2(heap, heapOffset, desc, initialLayout, optimizedClearValue, numCastableFormats, castableFormats, riid, outResource);
}

HRESULT IDXLDevice::CreateReservedResource2(const D3D12_RESOURCE_DESC* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ID3D12ProtectedResourceSession* protectedSession, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreateReservedResource2(desc, initialLayout, optimizedClearValue, protectedSession, numCastableFormats, castableFormats, riid, outResource);
}

#if DXL_ENABLE_EXTENSIONS

IDXLHeap IDXLDevice::CreateHeap(D3D12_HEAP_DESC desc)
{
    IDXLHeap heap;
    DXL_HANDLE_HRESULT(ToNative()->CreateHeap(&desc, DXL_PPV_ARGS(&heap)));
    return heap;
}

IDXLResource IDXLDevice::CreateCommittedResource(D3D12_HEAP_PROPERTIES heapProperties, D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_DESC1 desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, Span<const DXGI_FORMAT> castableFormats)
{
    IDXLResource resource;
    DXL_HANDLE_HRESULT(ToNative()->CreateCommittedResource3(&heapProperties, heapFlags, &desc, initialLayout, optimizedClearValue, nullptr, castableFormats.Count, castableFormats.Items, DXL_PPV_ARGS(&resource)));
    return resource;
}

IDXLResource IDXLDevice::CreatePlacedResource(IDXLHeap heap, uint64_t heapOffset, D3D12_RESOURCE_DESC1 desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, Span<const DXGI_FORMAT> castableFormats)
{
    IDXLResource resource;
    DXL_HANDLE_HRESULT(ToNative()->CreatePlacedResource2(heap, heapOffset, &desc, initialLayout, optimizedClearValue, castableFormats.Count, castableFormats.Items, DXL_PPV_ARGS(&resource)));
    return resource;
}

IDXLResource IDXLDevice::CreateTiledResource(D3D12_RESOURCE_DESC desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, Span<const DXGI_FORMAT> castableFormats)
{
    IDXLResource resource;
    DXL_HANDLE_HRESULT(ToNative()->CreateReservedResource2(&desc, initialLayout, optimizedClearValue, nullptr, castableFormats.Count, castableFormats.Items, DXL_PPV_ARGS(&resource)));
    return resource;
}

#endif // #if DXL_ENABLE_EXTENSIONS

void IDXLDevice::GetResourceTiling(IDXLResource tiledResource, uint32_t* outNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* outPackedMipDesc, D3D12_TILE_SHAPE* outStandardTileShapeForNonPackedMips, uint32_t* numSubresourceTilings, uint32_t firstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* outSubresourceTilingsForNonPackedMips)
{
    return ToNative()->GetResourceTiling(tiledResource, outNumTilesForEntireResource, outPackedMipDesc, outStandardTileShapeForNonPackedMips, numSubresourceTilings, firstSubresourceTilingToGet, outSubresourceTilingsForNonPackedMips);
}

HRESULT IDXLDevice::CreateSharedHandle(IDXLDeviceChild object, const SECURITY_ATTRIBUTES* attributes, uint32_t access, const wchar_t* name, HANDLE* outHandle)
{
    return ToNative()->CreateSharedHandle(object, attributes, access, name, outHandle);
}

HRESULT IDXLDevice::OpenSharedHandle(HANDLE ntHandle, REFIID riid, void** outObj)
{
    return ToNative()->OpenSharedHandle(ntHandle, riid, outObj);
}

HRESULT IDXLDevice::OpenSharedHandleByName(const wchar_t* name, uint32_t access, HANDLE* outHandle)
{
    return ToNative()->OpenSharedHandleByName(name, access, outHandle);
}

HRESULT IDXLDevice::MakeResident(uint32_t numObjects, ID3D12Pageable*const* objects)
{
    return ToNative()->MakeResident(numObjects, objects);
}

HRESULT IDXLDevice::EnqueueMakeResident(D3D12_RESIDENCY_FLAGS flags, uint32_t numObjects, ID3D12Pageable*const* objects, ID3D12Fence* fenceToSignal, UINT64 fenceValueToSignal)
{
    return ToNative()->EnqueueMakeResident(flags, numObjects, objects, fenceToSignal, fenceValueToSignal);
}

HRESULT IDXLDevice::Evict(uint32_t numObjects, ID3D12Pageable*const* objects)
{
    return ToNative()->Evict(numObjects, objects);
}

HRESULT IDXLDevice::SetResidencyPriority(uint32_t numObjects, ID3D12Pageable*const* objects, const D3D12_RESIDENCY_PRIORITY* priorities)
{
    return ToNative()->SetResidencyPriority(numObjects, objects, priorities);
}

HRESULT IDXLDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS flags, REFIID riid, void** outFence)
{
    return ToNative()->CreateFence(initialValue, flags, riid, outFence);
}

HRESULT IDXLDevice::SetEventOnMultipleFenceCompletion(ID3D12Fence*const* fences, const uint64_t* fenceValues, uint32_t numFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS flags, HANDLE event)
{
    return ToNative()->SetEventOnMultipleFenceCompletion(fences, fenceValues, numFences, flags, event);
}

#if DXL_ENABLE_EXTENSIONS

IDXLFence IDXLDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS flags)
{
    IDXLFence fence;
    ToNative()->CreateFence(initialValue, flags, DXL_PPV_ARGS(&fence));
    return fence;
}

#endif // DXL_ENABLE_EXTENSIONS

void IDXLDevice::GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* desc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* outInfo)
{
    ToNative()->GetRaytracingAccelerationStructurePrebuildInfo(desc, outInfo);
}

void IDXLDevice::RemoveDevice()
{
    ToNative()->RemoveDevice();
}

HRESULT IDXLDevice::GetDeviceRemovedReason()
{
    return ToNative()->GetDeviceRemovedReason();
}

void IDXLDevice::GetCopyableFootprints1(const D3D12_RESOURCE_DESC1* resourceDesc, uint32_t firstSubresource, uint32_t numSubresources, uint64_t baseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts, uint32_t* numRows, uint64_t* rowSizeInBytes, uint64_t* totalBytes)
{
    ToNative()->GetCopyableFootprints1(resourceDesc, firstSubresource, numSubresources, baseOffset, layouts, numRows, rowSizeInBytes, totalBytes);
}

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES

HRESULT IDXLDevice::SetStablePowerState(bool enable)
{
    return ToNative()->SetStablePowerState(enable);
}

HRESULT IDXLDevice::SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE mode, D3D12_MEASUREMENTS_ACTION measurementsAction, HANDLE eventToSignalUponCompletion, BOOL* outFurtherMeasurementsDesired)
{
    return ToNative()->SetBackgroundProcessingMode(mode, measurementsAction, eventToSignalUponCompletion, outFurtherMeasurementsDesired);
}

#endif // DXL_ENABLE_DEVELOPER_ONLY_FEATURES

// == IDXLSwapChain ======================================================

#if DXL_ENABLE_EXTENSIONS

IDXLSwapChain IDXLSwapChain::Create(DXGI_SWAP_CHAIN_DESC desc, IDXLCommandQueue presentQueue)
{
    ComPtr<IDXGIFactory7> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    DXL_HANDLE_HRESULT_MSG(hr, "failed to create DXGI factory");
    if (FAILED(hr))
        return IDXLSwapChain();

    ComPtr<IDXGISwapChain> tempSwapChain;
    hr = factory->CreateSwapChain(presentQueue, &desc, &tempSwapChain);
    DXL_HANDLE_HRESULT_MSG(hr, "failed to create swap chain");
    if (FAILED(hr))
        return IDXLSwapChain();

    IDXLSwapChain swapChain;
    hr = tempSwapChain->QueryInterface(DXL_PPV_ARGS(&swapChain));
    DXL_HANDLE_HRESULT_MSG(hr, "failed to query swap chain interface");

    return swapChain;
}

#endif

HRESULT IDXLSwapChain::Present(uint32_t syncInterval, uint32_t presentFlags)
{
    return ToNative()->Present(syncInterval, presentFlags);
}

HRESULT IDXLSwapChain::ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{
    return ToNative()->ResizeBuffers(bufferCount, width, height, newFormat, swapChainFlags);
}

uint32_t IDXLSwapChain::GetCurrentBackBufferIndex() const
{
    return ToNative()->GetCurrentBackBufferIndex();
}

HRESULT IDXLSwapChain::GetBuffer(uint32_t buffer, REFIID riid, void** outSurface) const
{
    return ToNative()->GetBuffer(buffer, riid, outSurface);
}

HRESULT IDXLSwapChain::GetDesc1(DXGI_SWAP_CHAIN_DESC1* outDesc)
{
    return ToNative()->GetDesc1(outDesc);
}

HRESULT IDXLSwapChain::GetHwnd(HWND* outHwnd)
{
    return ToNative()->GetHwnd(outHwnd);
}

HRESULT IDXLSwapChain::SetMaximumFrameLatency(uint32_t maxLatency)
{
    return ToNative()->SetMaximumFrameLatency(maxLatency);
}

HRESULT IDXLSwapChain::GetMaximumFrameLatency(uint32_t* maxLatency)
{
    return ToNative()->GetMaximumFrameLatency(maxLatency);
}

HANDLE IDXLSwapChain::GetFrameLatencyWaitableObject()
{
    return ToNative()->GetFrameLatencyWaitableObject();
}

HRESULT IDXLSwapChain::CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE colorSpace, uint32_t* colorSpaceSupport)
{
    return ToNative()->CheckColorSpaceSupport(colorSpace, colorSpaceSupport);
}

HRESULT IDXLSwapChain::SetColorSpace1(DXGI_COLOR_SPACE_TYPE colorSpace)
{
    return ToNative()->SetColorSpace1(colorSpace);
}

HRESULT IDXLSwapChain::SetHDRMetaData(DXGI_HDR_METADATA_TYPE type, uint32_t size, void* metaData)
{
    return ToNative()->SetHDRMetaData(type, size, metaData);
}

HRESULT IDXLSwapChain::SetRotation(DXGI_MODE_ROTATION rotation)
{
    return ToNative()->SetRotation(rotation);
}

HRESULT IDXLSwapChain::GetRotation(DXGI_MODE_ROTATION* outRotation)
{
    return ToNative()->GetRotation(outRotation);
}

HRESULT IDXLSwapChain::GetContainingOutput(IDXGIOutput** outOutput)
{
    return ToNative()->GetContainingOutput(outOutput);
}

HRESULT IDXLSwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS* outStats)
{
    return ToNative()->GetFrameStatistics(outStats);
}

HRESULT IDXLSwapChain::GetLastPresentCount(uint32_t* outLastPresentCount)
{
    return ToNative()->GetLastPresentCount(outLastPresentCount);
}

#if DXL_ENABLE_EXTENSIONS

IDXLResource IDXLSwapChain::GetBuffer(uint32_t bufferIndex) const
{
    IDXLResource resource;
    ToNative()->GetBuffer(bufferIndex, DXL_PPV_ARGS(&resource));
    return resource;
}

DXGI_SWAP_CHAIN_DESC1 IDXLSwapChain::GetDesc() const
{
    DXGI_SWAP_CHAIN_DESC1 desc = { };
    ToNative()->GetDesc1(&desc);
    return desc;
}

HWND IDXLSwapChain::GetHwnd() const
{
    HWND hwnd = nullptr;
    ToNative()->GetHwnd(&hwnd);
    return hwnd;
}

uint32_t IDXLSwapChain::GetMaximumFrameLatency()
{
    uint32_t latency = 0;
    ToNative()->GetMaximumFrameLatency(&latency);
    return latency;
}

DXGI_FRAME_STATISTICS IDXLSwapChain::GetFrameStatistics()
{
    DXGI_FRAME_STATISTICS stats = { };
    ToNative()->GetFrameStatistics(&stats);
    return stats;
}

uint32_t IDXLSwapChain::GetLastPresentCount()
{
    uint32_t presentCount = 0;
    ToNative()->GetLastPresentCount(&presentCount);
    return presentCount;
}

DXGI_MODE_ROTATION IDXLSwapChain::GetRotation()
{
    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
    ToNative()->GetRotation(&rotation);
    return rotation;
}

#endif // DXL_ENABLE_EXTENSIONS

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES

// == IDXLDebug ======================================================

void IDXLDebug::EnableDebugLayer()
{
    ToNative()->EnableDebugLayer();
}

void IDXLDebug::DisableDebugLayer()
{
    ToNative()->DisableDebugLayer();
}

void IDXLDebug::SetEnableGPUBasedValidation(bool enable)
{
    ToNative()->SetEnableGPUBasedValidation(enable);
}

void IDXLDebug::SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS flags)
{
    ToNative()->SetGPUBasedValidationFlags(flags);
}

void IDXLDebug::SetEnableSynchronizedCommandQueueValidation(bool enable)
{
    ToNative()->SetEnableSynchronizedCommandQueueValidation(enable);
}

void IDXLDebug::SetEnableAutoName(bool enable)
{
    ToNative()->SetEnableAutoName(enable);
}

// == IDXLDebugDevice ======================================================

#if DXL_ENABLE_EXTENSIONS

IDXLDebugDevice IDXLDebugDevice::FromDevice(IDXLDevice device)
{
    ID3D12DebugDevice2* debugDevice = nullptr;
    DXL_HANDLE_HRESULT_MSG(device->QueryInterface(IID_PPV_ARGS(&debugDevice)), "Failed to QueryInterface debug device interface from IDXLDevice. Did you enable the debug layer?");
    return IDXLDebugDevice(debugDevice);
}

#endif

HRESULT IDXLDebugDevice::SetFeatureMask(D3D12_DEBUG_FEATURE mask)
{
    return ToNative()->SetFeatureMask(mask);
}

D3D12_DEBUG_FEATURE IDXLDebugDevice::GetFeatureMask()
{
    return ToNative()->GetFeatureMask();
}

HRESULT IDXLDebugDevice::ReportLiveDeviceObjects(D3D12_RLDO_FLAGS flags)
{
    return ToNative()->ReportLiveDeviceObjects(flags);
}

HRESULT IDXLDebugDevice::SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, const void*data, uint32_t dataSize)
{
    return ToNative()->SetDebugParameter(type, data, dataSize);
}

HRESULT IDXLDebugDevice::GetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, void* data, uint32_t dataSize)
{
    return ToNative()->GetDebugParameter(type, data, dataSize);
}

// == IDXLDebugCommandQueue ======================================================

#if DXL_ENABLE_EXTENSIONS

IDXLDebugCommandQueue IDXLDebugCommandQueue::FromCommandQueue(IDXLCommandQueue commandQueue)
{
    ID3D12DebugCommandQueue1* debugQueue = nullptr;    
    DXL_HANDLE_HRESULT_MSG(commandQueue->QueryInterface(IID_PPV_ARGS(&debugQueue)), "Failed to QueryInterface debug command queue interface from IDXLCommandQueue. Did you enable the debug layer?");
    return IDXLDebugCommandQueue(debugQueue);
}

#endif

void IDXLDebugCommandQueue::AssertResourceAccess(IDXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access)
{
    ToNative()->AssertResourceAccess(resource, subresource, access);
}

void IDXLDebugCommandQueue::AssertTextureLayout(IDXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout)
{
    ToNative()->AssertTextureLayout(resource, subresource, layout);
}

// == IDXLDebugCommandList ======================================================

#if DXL_ENABLE_EXTENSIONS

IDXLDebugCommandList IDXLDebugCommandList::FromCommandList(IDXLCommandList commandList)
{
    ID3D12DebugCommandList3* debugList = nullptr;
    DXL_HANDLE_HRESULT_MSG(commandList->QueryInterface(IID_PPV_ARGS(&debugList)), "Failed to QueryInterface debug command list interface from IDXLCommandList. Did you enable the debug layer?");
    return IDXLDebugCommandList(debugList);
}

#endif

HRESULT IDXLDebugCommandList::SetFeatureMask(D3D12_DEBUG_FEATURE mask)
{
    return ToNative()->SetFeatureMask(mask);
}

D3D12_DEBUG_FEATURE IDXLDebugCommandList::GetFeatureMask()
{
    return ToNative()->GetFeatureMask();
}

HRESULT IDXLDebugCommandList::SetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, const void*data, uint32_t dataSize)
{
    return ToNative()->SetDebugParameter(type, data, dataSize);
}

HRESULT IDXLDebugCommandList::GetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, void* data, uint32_t dataSize)
{
    return ToNative()->GetDebugParameter(type, data, dataSize);
}

void IDXLDebugCommandList::AssertResourceAccess(IDXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access)
{
    ToNative()->AssertResourceAccess(resource, subresource, access);
}

void IDXLDebugCommandList::AssertTextureLayout(IDXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout)
{
    ToNative()->AssertTextureLayout(resource, subresource, layout);
}

// == IDXLDebugInfoQueue ======================================================

#if DXL_ENABLE_EXTENSIONS

IDXLDebugInfoQueue IDXLDebugInfoQueue::FromDevice(IDXLDevice device)
{
    ID3D12InfoQueue1* infoQueue = nullptr;    
    DXL_HANDLE_HRESULT_MSG(device->QueryInterface(IID_PPV_ARGS(&infoQueue)), "Failed to QueryInterface debug command info queue interface from IDXLDevice. Did you enable the debug layer?");
    return IDXLDebugInfoQueue(infoQueue);
}

#endif

HRESULT IDXLDebugInfoQueue::RegisterMessageCallback(D3D12MessageFunc callbackFunc, D3D12_MESSAGE_CALLBACK_FLAGS callbackFilterFlags, void* context, DWORD* outCallbackCookie)
{
    return ToNative()->RegisterMessageCallback(callbackFunc, callbackFilterFlags, context, outCallbackCookie);
}

HRESULT IDXLDebugInfoQueue::UnregisterMessageCallback(DWORD callbackCookie)
{
    return ToNative()->UnregisterMessageCallback(callbackCookie);
}

void IDXLDebugInfoQueue::SetMuteDebugOutput(bool mute)
{
    ToNative()->SetMuteDebugOutput(mute);
}

bool IDXLDebugInfoQueue::GetMuteDebugOutput()
{
    return ToNative()->GetMuteDebugOutput();
}

#endif // DXL_ENABLE_DEVELOPER_ONLY_FEATURES

#if DXL_ENABLE_EXTENSIONS

static void DefaultDebugLayerCallback([[maybe_unused]] D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, [[maybe_unused]] D3D12_MESSAGE_ID ID, const char* description, [[maybe_unused]] void* context)
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

CreateDeviceResult CreateDevice(CreateDeviceParams params)
{
    ComPtr<IDXGIFactory7> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return { IDXLDevice(), hr, "Failed to create DXGI factory" };

    ComPtr<IDXGIAdapter4> adapter;
    hr = factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
    if (FAILED(hr))
        return { IDXLDevice(), hr, "Failed to enumerate a DXGI adapter" };

    DXGI_ADAPTER_DESC1 desc = { };
    adapter->GetDesc1(&desc);
    PrintMessage("Creating D3D12 device on adapter '%ls'", desc.Description);

    ComPtr<ID3D12SDKConfiguration1> sdkConfig;
    hr = D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdkConfig));
    if (FAILED(hr))
        return { IDXLDevice(), hr, "Failed to aquire the D3D12 SDK configuration interface" };

    ComPtr<ID3D12DeviceFactory> deviceFactory;
    hr = sdkConfig->CreateDeviceFactory(D3D12_SDK_VERSION, params.AgilitySDKPath, IID_PPV_ARGS(&deviceFactory));
    if (FAILED(hr))
        return { IDXLDevice(), hr, "Failed to create a D3D12 device factory. Did you pass the wrong AgilitySDKPath?" };

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES
    if (params.EnableDebugLayer)
    {
        ComPtr<ID3D12Debug1> d3d12debug;
        if (SUCCEEDED(deviceFactory->GetConfigurationInterface(CLSID_D3D12Debug, DXL_PPV_ARGS(d3d12debug.GetAddressOf()))))
        {
            d3d12debug->EnableDebugLayer();
            PrintMessage("Enabled D3D12 debug layer");
        }

        if (params.EnableGPUBasedValidation)
            d3d12debug->SetEnableGPUBasedValidation(true);
    }
#endif

    ComPtr<ID3D12Device14> device;
    hr = deviceFactory->CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr))
        return { IDXLDevice(), hr, "Failed to create D3D12 device from the device factory" };

    D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = { };
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12));
    if(options12.EnhancedBarriersSupported == false)
        return { IDXLDevice(), E_FAIL, "The selected GPU does not support enhanced barriers, which is required for DXLatest" };

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES
    {
        ComPtr<ID3D12InfoQueue1> infoQueue;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            D3D12MessageFunc callback = params.DebugLayerCallbackFunction ? params.DebugLayerCallbackFunction : DefaultDebugLayerCallback;

            DWORD callbackCookie = 0;
            infoQueue->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &callbackCookie);
        }
    }
#endif

    device->AddRef();
    return { device.Get(), S_OK, nullptr };
}

void Release(IUnknown*& unknown)
{
    if (unknown)
    {
        unknown->Release();
        unknown = nullptr;
    }
}

void Release(IDXLBase& base)
{
    if (base)
    {
        base->Release();
        base = IDXLBase();
    }
}

#endif // DXL_ENABLE_EXTENSIONS

} // namespace DXL