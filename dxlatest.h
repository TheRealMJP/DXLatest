#pragma once

#include <stdint.h>

#include "AgilitySDK/include/d3d12.h"

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

struct DXL_DESCRIPTOR_HEAP_DESC
{
    D3D12_DESCRIPTOR_HEAP_TYPE Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uint32_t NumDescriptors = 0;
    D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    uint32_t NodeMask = 0;

    DXL_STRUCT_BOILERPLATE(DXL_DESCRIPTOR_HEAP_DESC, D3D12_DESCRIPTOR_HEAP_DESC);
};

struct DXL_INDEX_BUFFER_VIEW
{
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation = 0;
    uint32_t SizeInBytes = 0;
    DXGI_FORMAT Format = DXGI_FORMAT_R16_UINT;

    DXL_STRUCT_BOILERPLATE(DXL_INDEX_BUFFER_VIEW, D3D12_INDEX_BUFFER_VIEW);
};

#define DXL_INTERFACE_BOILERPLATE(DXLClass, D3D12Interface) \
    DXLClass() = default;   \
    DXLClass(D3D12Interface* d3d12Interface) { underlying = d3d12Interface; }   \
    D3D12Interface* ToNative() const { return reinterpret_cast<D3D12Interface*>(underlying); }     \
    DXLClass* operator->() { return this; }     \
    operator D3D12Interface*() const { return ToNative(); }

class DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLBase, IUnknown);

    HRESULT QueryInterface(REFIID riid, void** outObject);
    template<typename T> T* QueryInterface()
    {
        T* queried = nullptr;
        QueryInterface(IID_PPV_ARGS(&queried));
        return queried;
    }

    uint32_t AddRef();
    uint32_t Release();

    operator bool() const
    {
        return underlying != nullptr;
    }

protected:

    IUnknown* underlying = nullptr;
};

class DXLObject : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLObject, ID3D12Object);

    HRESULT GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const;
    HRESULT SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data);
    HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* data);
    HRESULT SetName(const wchar_t* name);
    HRESULT SetName(const char* name);
};

class DXLDeviceChild : public DXLObject
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDeviceChild, ID3D12DeviceChild);

    HRESULT GetDevice(REFIID riid, void** outDevice);
};

class DXLRootSignature : public DXLDeviceChild
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLRootSignature, ID3D12RootSignature);
};

class DXLPageable : public DXLDeviceChild
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLPageable, ID3D12Pageable);
};

class DXLHeap : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLHeap, ID3D12Heap1);

    DXL_HEAP_DESC GetDesc() const;
    HRESULT GetProtectedResourceSession(REFIID riid, void** outProtectedSession) const;
};

class DXLResource : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLResource, ID3D12Resource2);

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

class DXLCommandAllocator : public DXLPageable
{
public:

    DXL_INTERFACE_BOILERPLATE(DXLCommandAllocator, ID3D12CommandAllocator);

    HRESULT Reset();
};

class DXLFence : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLFence, ID3D12Fence1);

    uint64_t GetCompletedValue() const;
    HRESULT SetEventOnCompletion(uint64_t value, HANDLE event);
    HRESULT Signal(uint64_t value);

    D3D12_FENCE_FLAGS GetCreationFlags() const;
};

class DXLPipelineState : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLPipelineState, ID3D12PipelineState1);

    DXLRootSignature GetRootSignature() const;
    HRESULT GetRootSignature(REFIID riid, void** outRootSignature) const;
};

class DXLDescriptorHeap : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDescriptorHeap, ID3D12DescriptorHeap);

    DXL_DESCRIPTOR_HEAP_DESC GetDesc();
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart();
};

class DXLQueryHeap : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLQueryHeap, ID3D12QueryHeap);
};

class DXLCommandSignature : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLCommandSignature, ID3D12CommandSignature);
};

class DXLCommandList : public DXLDeviceChild
{
public:

    DXL_INTERFACE_BOILERPLATE(DXLCommandList, ID3D12GraphicsCommandList10);

    D3D12_COMMAND_LIST_TYPE GetType() const;

    HRESULT Close();
    HRESULT Reset(DXLCommandAllocator allocator, DXLPipelineState pipelineState = DXLPipelineState());

    void ClearState(DXLPipelineState pipelineState = DXLPipelineState());

    void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation);
    void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation);

    void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

    void CopyBufferRegion(DXLResource dstBuffer, uint64_t dstOffset, DXLResource srcBuffer, uint64_t srcOffset, uint64_t numBytes);
    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* dst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* srcBox);
    void CopyResource(DXLResource dstResource, DXLResource srcResource);
    void CopyTiles(DXLResource tiledResource, const D3D12_TILED_RESOURCE_COORDINATE* tileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* tileRegionSize, DXLResource buffer, uint64_t bufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS flags);

    void ResolveSubresource(DXLResource dstResource, uint32_t dstSubresource, DXLResource srcResource, uint32_t srcSubresource, DXGI_FORMAT format);

    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);

    void RSSetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports);
    void RSSetScissorRects(uint32_t numRects, const D3D12_RECT* rects);

    void OMSetBlendFactor(const float blendFactor[4]);
    void OMSetStencilRef(uint32_t stencilRef);

    void SetPipelineState(DXLPipelineState pipelineState);

    void SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap*const* descriptorHeaps);
    void SetDescriptorHeaps(DXLDescriptorHeap srvUavCbvHeap, DXLDescriptorHeap samplerHeap = DXLDescriptorHeap());

    void SetComputeRootSignature(DXLRootSignature rootSignature);
    void SetGraphicsRootSignature(DXLRootSignature rootSignature);

    void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
    void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);

    void SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues);
    void SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues);

    void SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues);
    void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues);

    void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
    void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view);
    void IASetIndexBuffer(DXL_INDEX_BUFFER_VIEW view);

    void OMSetRenderTargets(uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, bool rtIsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags,float depth, uint8_t stencil, uint32_t numRects, const D3D12_RECT* rects);
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], uint32_t numRects, const D3D12_RECT* rects);
    void ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const UINT values[4], uint32_t numRects, const D3D12_RECT* rects);
    void ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const FLOAT values[4], uint32_t numRects, const D3D12_RECT* rects);
    void DiscardResource(DXLResource resource, const D3D12_DISCARD_REGION* region);

    void BeginQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index);
    void EndQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index);
    void ResolveQueryData(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t startIndex, uint32_t numQueries, DXLResource destinationBuffer, uint64_t alignedDestinationBufferOffset);
    void SetPredication(DXLResource buffer, uint64_t alignedBufferOffset, D3D12_PREDICATION_OP operation);

    void ExecuteIndirect(DXLCommandSignature commandSignature, uint32_t maxCommandCount, DXLResource argumentBuffer, uint64_t argumentBufferOffset, DXLResource countBuffer, uint64_t countBufferOffset);
};

} // namespace dxl