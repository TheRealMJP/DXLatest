#pragma once

#include <stdint.h>

#include "AgilitySDK/include/d3d12.h"
#include "AgilitySDK/include/d3dx12/d3dx12.h"

#define DXL_STRUCT_BOILERPLATE(DXLStruct, D3D12Struct)  \
    DXLStruct() = default;                              \
    DXLStruct(D3D12Struct d3d12Struct) { memcpy(this, &d3d12Struct, sizeof(DXLStruct)); }        \
    operator D3D12Struct() const { D3D12Struct d3d12Struct = { }; memcpy(&d3d12Struct, this, sizeof(DXLStruct)); return d3d12Struct; }


namespace dxl
{

struct DXL_HEAP_PROPERTIES
{
    D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_CPU_PAGE_PROPERTY CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    D3D12_MEMORY_POOL MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uint32_t CreationNodeMask = 0;
    uint32_t VisibleNodeMask = 0;

    DXL_STRUCT_BOILERPLATE(DXL_HEAP_PROPERTIES, D3D12_HEAP_PROPERTIES);
};

struct DXL_HEAP_DESC
{
    uint64_t SizeInBytes = 0;
    DXL_HEAP_PROPERTIES Properties = { };
    uint64_t Alignment = 0;
    D3D12_HEAP_FLAGS Flags = D3D12_HEAP_FLAG_NONE;

    DXL_STRUCT_BOILERPLATE(DXL_HEAP_DESC, D3D12_HEAP_DESC);
};

struct DXL_MIP_REGION
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t Depth = 0;
};

struct DXL_RESOURCE_DESC
{
    D3D12_RESOURCE_DIMENSION Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    uint64_t Alignment = 0;
    uint64_t Width = 0;
    uint32_t Height = 0;
    uint16_t DepthOrArraySize = 0;
    uint16_t MipLevels = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    DXGI_SAMPLE_DESC SampleDesc = { 1, 0 };
    D3D12_TEXTURE_LAYOUT Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
    DXL_MIP_REGION SamplerFeedbackMipRegion = { };

    DXL_STRUCT_BOILERPLATE(DXL_RESOURCE_DESC, D3D12_RESOURCE_DESC1);
};

class DXLBase : public IUnknown
{

public:

    HRESULT QueryInterface(REFIID riid, void** outObject) override;
    ULONG AddRef() override;
    ULONG Release() override;

protected:

    IUnknown* underyling = nullptr;
    ULONG refCount = 1;
};

class IDXLObject : public DXLBase
{

public:

    ID3D12Object* ToID3D12Object() const;

    HRESULT GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const;
    HRESULT SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data);
    HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* data);
    HRESULT SetName(const wchar_t* name);
    HRESULT SetName(const char* name);
};

class IDXLDeviceChild : public IDXLObject
{

public:

    ID3D12DeviceChild* ToID3D12DeviceChild() const;

    HRESULT GetDevice(REFIID riid, void** outDevice);
};

class IDXLRootSignature : public IDXLDeviceChild
{

public:

    ID3D12RootSignature* ToID3D12RootSignature() const;
};

class IDXLPageable : public IDXLDeviceChild
{

public:

    ID3D12Pageable* ToID3D12Pageable() const;
};

class IDXLHeap : public IDXLPageable
{

public:

    ID3D12Heap1* ToID3D12Heap() const;

    DXL_HEAP_DESC GetDesc() const;
    HRESULT GetProtectedResourceSession(REFIID riid, void** outProtectedSession) const;
};

class IDXLResource : public IDXLPageable
{

public:

    ID3D12Resource2* ToID3D12Resource() const;

    HRESULT Map(uint32_t subresource, const D3D12_RANGE* readRange, void** outData);
    void Unmap(uint32_t subresource, const D3D12_RANGE* writtenRange);

    void* Map(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);
    void Unmap(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);

    DXL_RESOURCE_DESC GetDesc() const;

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    HRESULT WriteToSubresource(uint32_t dstSubresource, const D3D12_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch);
    HRESULT ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const D3D12_BOX* srcBox) const;

    HRESULT GetHeapProperties(DXL_HEAP_PROPERTIES* outHeapProperties, D3D12_HEAP_FLAGS* outHeapFlags) const;

    HRESULT GetProtectedResourceSession(REFIID riid, void** protectedSession) const;
};

class IDXLCommandAllocator : public IDXLPageable
{
    ID3D12CommandAllocator* ToID3D12CommandAllocator() const;

    HRESULT Reset();
};

class IDXLFence : public IDXLPageable
{
    ID3D12Fence1* ToID3D12Fence() const;

    uint64_t GetCompletedValue() const;
    HRESULT SetEventOnCompletion(uint64_t value, HANDLE event);
    HRESULT Signal(uint64_t value);

    D3D12_FENCE_FLAGS GetCreationFlags() const;
};

} // namespace dxl