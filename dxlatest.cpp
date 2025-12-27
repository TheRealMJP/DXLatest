#include "dxlatest.h"

#include "AgilitySDK/include/d3dx12/d3dx12.h"

// #define NOMINMAX
// #include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "D3D12.lib")
#endif

namespace dxl
{

static_assert(sizeof(DXL_HEAP_PROPERTIES) == sizeof(D3D12_HEAP_PROPERTIES));
static_assert(sizeof(DXL_HEAP_DESC) == sizeof(D3D12_HEAP_DESC));
static_assert(sizeof(DXL_MIP_REGION) == sizeof(D3D12_MIP_REGION));
static_assert(sizeof(DXL_RESOURCE_DESC) == sizeof(D3D12_RESOURCE_DESC1));

// == DXLBase ======================================================

HRESULT DXLBase::QueryInterface(REFIID riid, void** outObject)
{
    return ToIUnknown()->QueryInterface(riid, outObject);
}

uint32_t DXLBase::AddRef()
{
    return ToIUnknown()->AddRef();
}

uint32_t DXLBase::Release()
{
    return ToIUnknown()->Release();
}

// == DXLObject ======================================================

HRESULT DXLObject::GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const
{
    return ToID3D12Object()->GetPrivateData(guid, outDataSize, outData);
}

HRESULT DXLObject::SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data)
{
    return ToID3D12Object()->SetPrivateData(guid, dataSize, data);
}

HRESULT DXLObject::SetPrivateDataInterface(REFGUID guid, const IUnknown* data)
{
    return ToID3D12Object()->SetPrivateDataInterface(guid, data);
}

HRESULT DXLObject::SetName(const wchar_t* name)
{
    return ToID3D12Object()->SetName(name);
}

HRESULT DXLObject::SetName(const char* name)
{
    const int32_t numChars = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
    if (numChars <= 0)
        return E_FAIL;

    wchar_t* wideChars = new wchar_t[numChars];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wideChars, numChars);
    HRESULT hr = ToID3D12Object()->SetName(wideChars);
    delete[] wideChars;

    return hr;
}

// == DXLDeviceChild ======================================================

HRESULT DXLDeviceChild::GetDevice(REFIID riid, void** outDevice)
{
    return ToID3D12DeviceChild()->GetDevice(riid, outDevice);
}

// == DXLRootSignature ======================================================

// == DXLPageable ======================================================

// == DXLPageable ======================================================

DXL_HEAP_DESC DXLHeap::GetDesc() const
{
    return ToID3D12Heap()->GetDesc();
}

HRESULT DXLHeap::GetProtectedResourceSession(REFIID riid, void** outProtectedSession) const
{
    return ToID3D12Heap()->GetProtectedResourceSession(riid, outProtectedSession);
}

// == DXLResource ======================================================

HRESULT DXLResource::Map(uint32_t subresource, const D3D12_RANGE* readRange, void** outData)
{
    return ToID3D12Resource()->Map(subresource, readRange, outData);
}

void DXLResource::Unmap(uint32_t subresource, const D3D12_RANGE* writtenRange)
{
    ToID3D12Resource()->Unmap(subresource, writtenRange);
}

void* DXLResource::Map(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToID3D12Resource()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);

    void* data = nullptr;
    ToID3D12Resource()->Map(subresourceIndex, nullptr, &data);
    return data;
}

void DXLResource::Unmap(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToID3D12Resource()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);
    ToID3D12Resource()->Unmap(subresourceIndex, nullptr);
}

DXL_RESOURCE_DESC DXLResource::GetDesc() const
{
    return ToID3D12Resource()->GetDesc1();
}

D3D12_GPU_VIRTUAL_ADDRESS DXLResource::GetGPUVirtualAddress() const
{
    return ToID3D12Resource()->GetGPUVirtualAddress();
}

HRESULT DXLResource::WriteToSubresource(uint32_t dstSubresource, const D3D12_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
{
    return ToID3D12Resource()->WriteToSubresource(dstSubresource, dstBox, srcData, srcRowPitch, srcDepthPitch);
}

HRESULT DXLResource::ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const D3D12_BOX* srcBox) const
{
    return ToID3D12Resource()->ReadFromSubresource(dstData, dstRowPitch, dstDepthPitch, srcSubresource, srcBox);
}

HRESULT DXLResource::GetHeapProperties(DXL_HEAP_PROPERTIES* outHeapProperties, D3D12_HEAP_FLAGS* outHeapFlags) const
{
    D3D12_HEAP_PROPERTIES d3d12HeapProperties = { };
    HRESULT hr = ToID3D12Resource()->GetHeapProperties(&d3d12HeapProperties, outHeapFlags);
    *outHeapProperties = d3d12HeapProperties;
    return hr;
}

HRESULT DXLResource::GetProtectedResourceSession(REFIID riid, void** protectedSession) const
{
    return ToID3D12Resource()->GetProtectedResourceSession(riid, protectedSession);
}

// == DXLCommandAllocator ======================================================

HRESULT DXLCommandAllocator::Reset()
{
    return ToID3D12CommandAllocator()->Reset();
}

// == DXLFence ======================================================

uint64_t DXLFence::GetCompletedValue() const
{
    return ToID3D12Fence()->GetCompletedValue();
}

HRESULT DXLFence::SetEventOnCompletion(uint64_t value, HANDLE event)
{
    return ToID3D12Fence()->SetEventOnCompletion(value, event);
}

HRESULT DXLFence::Signal(uint64_t value)
{
    return ToID3D12Fence()->Signal(value);
}

D3D12_FENCE_FLAGS DXLFence::GetCreationFlags() const
{
    return ToID3D12Fence()->GetCreationFlags();
}

// == DXLPipelineState ======================================================

DXLRootSignature DXLPipelineState::GetRootSignature() const
{
    ID3D12RootSignature* rootSig = nullptr;
    GetRootSignature(IID_PPV_ARGS(&rootSig));
    return DXLRootSignature(rootSig);
}

HRESULT DXLPipelineState::GetRootSignature(REFIID riid, void** outRootSignature) const
{
    return ToID3D12PipelineState()->GetRootSignature(riid, outRootSignature);
}

// == DXLDescriptorHeap =====================================================

DXL_DESCRIPTOR_HEAP_DESC DXLDescriptorHeap::GetDesc()
{
    return DXL_DESCRIPTOR_HEAP_DESC(ToID3D12DescriptorHeap()->GetDesc());
}

D3D12_CPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetCPUDescriptorHandleForHeapStart()
{
    return ToID3D12DescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetGPUDescriptorHandleForHeapStart()
{
    return ToID3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
}

// == DXLCommandList =====================================================

D3D12_COMMAND_LIST_TYPE DXLCommandList::GetType() const
{
    return ToID3D12CommandList()->GetType();
}

HRESULT DXLCommandList::Close()
{
    return ToID3D12CommandList()->Close();
}

HRESULT DXLCommandList::Reset(DXLCommandAllocator allocator, DXLPipelineState pipelineState)
{
    return ToID3D12CommandList()->Reset(allocator, pipelineState);
}

void DXLCommandList::ClearState(DXLPipelineState pipelineState)
{
    ToID3D12CommandList()->ClearState(pipelineState);
}

void DXLCommandList::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
    ToID3D12CommandList()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void DXLCommandList::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
    ToID3D12CommandList()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void DXLCommandList::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    ToID3D12CommandList()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void DXLCommandList::CopyBufferRegion(DXLResource dstBuffer, uint64_t dstOffset, DXLResource srcBuffer, uint64_t srcOffset, uint64_t numBytes)
{
    ToID3D12CommandList()->CopyBufferRegion(dstBuffer, dstOffset, srcBuffer, srcOffset, numBytes);
}

void DXLCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* dst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* srcBox)
{
    ToID3D12CommandList()->CopyTextureRegion(dst, dstX, dstY, dstZ, src, srcBox);
}

void DXLCommandList::CopyResource(DXLResource dstResource, DXLResource srcResource)
{
    ToID3D12CommandList()->CopyResource(dstResource, srcResource);
}

void DXLCommandList::CopyTiles(DXLResource tiledResource, const D3D12_TILED_RESOURCE_COORDINATE* tileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* tileRegionSize, DXLResource buffer, uint64_t bufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS flags)
{
    ToID3D12CommandList()->CopyTiles(tiledResource, tileRegionStartCoordinate, tileRegionSize, buffer, bufferStartOffsetInBytes, flags);
}

void DXLCommandList::ResolveSubresource(DXLResource dstResource, uint32_t dstSubresource, DXLResource srcResource, uint32_t srcSubresource, DXGI_FORMAT format)
{
    ToID3D12CommandList()->ResolveSubresource(dstResource, dstSubresource, srcResource, srcSubresource, format);
}
void DXLCommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    ToID3D12CommandList()->IASetPrimitiveTopology(primitiveTopology);
}

void DXLCommandList::RSSetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports)
{
    ToID3D12CommandList()->RSSetViewports(numViewports, viewports);
}

void DXLCommandList::RSSetScissorRects(uint32_t numRects, const D3D12_RECT* rects)
{
    ToID3D12CommandList()->RSSetScissorRects(numRects, rects);
}

void DXLCommandList::OMSetBlendFactor(const float blendFactor[4])
{
    ToID3D12CommandList()->OMSetBlendFactor(blendFactor);
}

void DXLCommandList::OMSetStencilRef(uint32_t stencilRef)
{
    ToID3D12CommandList()->OMSetStencilRef(stencilRef);
}

void DXLCommandList::SetPipelineState(DXLPipelineState pipelineState)
{
    ToID3D12CommandList()->SetPipelineState(pipelineState);
}

void DXLCommandList::SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap*const* descriptorHeaps)
{
    ToID3D12CommandList()->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}

void DXLCommandList::SetDescriptorHeaps(DXLDescriptorHeap srvUavCbvHeap, DXLDescriptorHeap samplerHeap)
{
    ID3D12DescriptorHeap* heaps[] = { srvUavCbvHeap, samplerHeap };
    ToID3D12CommandList()->SetDescriptorHeaps(samplerHeap ? 2 : 1, heaps);
}

void DXLCommandList::SetComputeRootSignature(DXLRootSignature rootSignature)
{
    ToID3D12CommandList()->SetComputeRootSignature(rootSignature);
}

void DXLCommandList::SetGraphicsRootSignature(DXLRootSignature rootSignature)
{
    ToID3D12CommandList()->SetGraphicsRootSignature(rootSignature);
}

void DXLCommandList::SetComputeRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToID3D12CommandList()->SetComputeRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

void DXLCommandList::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToID3D12CommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

void DXLCommandList::SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToID3D12CommandList()->SetComputeRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToID3D12CommandList()->SetGraphicsRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToID3D12CommandList()->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToID3D12CommandList()->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetComputeRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetComputeRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetComputeRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetComputeRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetGraphicsRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetComputeRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToID3D12CommandList()->SetGraphicsRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view)
{
    ToID3D12CommandList()->IASetIndexBuffer(view);
}

void DXLCommandList::IASetIndexBuffer(DXL_INDEX_BUFFER_VIEW view)
{
    D3D12_INDEX_BUFFER_VIEW d3d12View = view;
    ToID3D12CommandList()->IASetIndexBuffer(&d3d12View);
}

void DXLCommandList::OMSetRenderTargets(uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, bool rtIsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
{
    ToID3D12CommandList()->OMSetRenderTargets(numRenderTargetDescriptors, renderTargetDescriptors, rtIsSingleHandleToDescriptorRange, depthStencilDescriptor);
}

void DXLCommandList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags,float depth, uint8_t stencil, uint32_t numRects, const D3D12_RECT* rects)
{
    ToID3D12CommandList()->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, rects);
}

void DXLCommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToID3D12CommandList()->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, rects);
}

void DXLCommandList::ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const UINT values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToID3D12CommandList()->ClearUnorderedAccessViewUint(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

void DXLCommandList::ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const FLOAT values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToID3D12CommandList()->ClearUnorderedAccessViewFloat(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

void DXLCommandList::DiscardResource(DXLResource resource, const D3D12_DISCARD_REGION* region)
{
    ToID3D12CommandList()->DiscardResource(resource, region);
}

void DXLCommandList::BeginQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToID3D12CommandList()->BeginQuery(queryHeap, type, index);
}

void DXLCommandList::EndQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToID3D12CommandList()->EndQuery(queryHeap, type, index);
}

void DXLCommandList::ResolveQueryData(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t startIndex, uint32_t numQueries, DXLResource destinationBuffer, uint64_t alignedDestinationBufferOffset)
{
    ToID3D12CommandList()->ResolveQueryData(queryHeap, type, startIndex, numQueries, destinationBuffer, alignedDestinationBufferOffset);
}

void DXLCommandList::SetPredication(DXLResource buffer, uint64_t alignedBufferOffset, D3D12_PREDICATION_OP operation)
{
    ToID3D12CommandList()->SetPredication(buffer, alignedBufferOffset, operation);
}

void DXLCommandList::ExecuteIndirect(DXLCommandSignature commandSignature, uint32_t maxCommandCount, DXLResource argumentBuffer, uint64_t argumentBufferOffset, DXLResource countBuffer, uint64_t countBufferOffset)
{
    ToID3D12CommandList()->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

} // namespace dxl