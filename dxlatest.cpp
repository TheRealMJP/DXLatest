#include "dxlatest.h"

#include "AgilitySDK/include/d3dx12/d3dx12.h"

#include <wrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "DXGI.lib")
#endif

namespace dxl
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

static void PrintAssertMessage(const char* condition, const char* msg, const char* file, const int32_t line)
{
    const uint64_t BufferSize = 2048;
    char buffer[BufferSize] = { };
    sprintf_s(buffer, BufferSize, "%s(%d): Assert Failure: ", file, line);

    if(condition != nullptr)
        sprintf_s(buffer, BufferSize, "%s'%s' ", buffer, condition);

    if(msg != nullptr)
        sprintf_s(buffer, BufferSize, "%s%s", buffer, msg);

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
                if (numChars > (sizeof(fixedStorage) / sizeof(wchar_t)))
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

// == DXLBase ======================================================

HRESULT DXLBase::QueryInterface(REFIID riid, void** outObject)
{
    return ToNative()->QueryInterface(riid, outObject);
}

uint32_t DXLBase::AddRef()
{
    return ToNative()->AddRef();
}

uint32_t DXLBase::Release()
{
    return ToNative()->Release();
}

// == DXLObject ======================================================

HRESULT DXLObject::GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const
{
    return ToNative()->GetPrivateData(guid, outDataSize, outData);
}

HRESULT DXLObject::SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data)
{
    return ToNative()->SetPrivateData(guid, dataSize, data);
}

HRESULT DXLObject::SetPrivateDataInterface(REFGUID guid, const IUnknown* data)
{
    return ToNative()->SetPrivateDataInterface(guid, data);
}

HRESULT DXLObject::SetName(const wchar_t* name)
{
    return ToNative()->SetName(name);
}

#if DXL_ENABLE_EXTENSIONS()

HRESULT DXLObject::SetName(const char* name)
{
    return ToNative()->SetName(WideStringConverter(name).wideString);
}

#endif

// == DXLDeviceChild ======================================================

HRESULT DXLDeviceChild::GetDevice(REFIID riid, void** outDevice)
{
    return ToNative()->GetDevice(riid, outDevice);
}

// == DXLPageable ======================================================

D3D12_HEAP_DESC DXLHeap::GetDesc() const
{
    return ToNative()->GetDesc();
}

// == DXLResource ======================================================

HRESULT DXLResource::Map(uint32_t subresource, const D3D12_RANGE* readRange, void** outData)
{
    return ToNative()->Map(subresource, readRange, outData);
}

void DXLResource::Unmap(uint32_t subresource, const D3D12_RANGE* writtenRange)
{
    ToNative()->Unmap(subresource, writtenRange);
}

#if DXL_ENABLE_EXTENSIONS()

void* DXLResource::Map(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToNative()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);

    void* data = nullptr;
    ToNative()->Map(subresourceIndex, nullptr, &data);
    return data;
}

void DXLResource::Unmap(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToNative()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);
    ToNative()->Unmap(subresourceIndex, nullptr);
}

#endif

D3D12_RESOURCE_DESC1 DXLResource::GetDesc1() const
{
    return ToNative()->GetDesc1();
}

D3D12_GPU_VIRTUAL_ADDRESS DXLResource::GetGPUVirtualAddress() const
{
    return ToNative()->GetGPUVirtualAddress();
}

HRESULT DXLResource::WriteToSubresource(uint32_t dstSubresource, const D3D12_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
{
    return ToNative()->WriteToSubresource(dstSubresource, dstBox, srcData, srcRowPitch, srcDepthPitch);
}

HRESULT DXLResource::ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const D3D12_BOX* srcBox) const
{
    return ToNative()->ReadFromSubresource(dstData, dstRowPitch, dstDepthPitch, srcSubresource, srcBox);
}

HRESULT DXLResource::GetHeapProperties(D3D12_HEAP_PROPERTIES* outHeapProperties, D3D12_HEAP_FLAGS* outHeapFlags) const
{
    return ToNative()->GetHeapProperties(outHeapProperties, outHeapFlags);
}

// == DXLCommandAllocator ======================================================

HRESULT DXLCommandAllocator::Reset()
{
    return ToNative()->Reset();
}

// == DXLFence ======================================================

uint64_t DXLFence::GetCompletedValue() const
{
    return ToNative()->GetCompletedValue();
}

HRESULT DXLFence::SetEventOnCompletion(uint64_t value, HANDLE event)
{
    return ToNative()->SetEventOnCompletion(value, event);
}

HRESULT DXLFence::Signal(uint64_t value)
{
    return ToNative()->Signal(value);
}

D3D12_FENCE_FLAGS DXLFence::GetCreationFlags() const
{
    return ToNative()->GetCreationFlags();
}

// == DXLPipelineState ======================================================

HRESULT DXLPipelineState::GetRootSignature(REFIID riid, void** outRootSignature) const
{
    return ToNative()->GetRootSignature(riid, outRootSignature);
}

#if DXL_ENABLE_EXTENSIONS()

DXLRootSignature DXLPipelineState::GetRootSignature() const
{
    ID3D12RootSignature* rootSig = nullptr;
    GetRootSignature(IID_PPV_ARGS(&rootSig));
    return DXLRootSignature(rootSig);
}

#endif

// == DXLStateObjectProperties =====================================================

void* DXLStateObjectProperties::GetShaderIdentifier(const wchar_t* exportName)
{
    return ToNative()->GetShaderIdentifier(exportName);
}

uint64_t DXLStateObjectProperties::GetShaderStackSize(const wchar_t* exportName)
{
    return ToNative()->GetShaderStackSize(exportName);
}

#if DXL_ENABLE_EXTENSIONS()

void* DXLStateObjectProperties::GetShaderIdentifier(const char* exportName)
{
    return ToNative()->GetShaderIdentifier(WideStringConverter(exportName).wideString);
}

uint64_t DXLStateObjectProperties::GetShaderStackSize(const char* exportName)
{
    return ToNative()->GetShaderStackSize(WideStringConverter(exportName).wideString);
}

D3D12_PROGRAM_IDENTIFIER DXLStateObjectProperties::GetProgramIdentifier(const char* programName)
{
    return ToNative()->GetProgramIdentifier(WideStringConverter(programName).wideString);
}

HRESULT DXLStateObjectProperties::GetGlobalRootSignatureForProgram(const wchar_t* programName, REFIID riid, void** outRootSignature)
{
    return ToNative()->GetGlobalRootSignatureForProgram(programName, riid, outRootSignature);
}
HRESULT DXLStateObjectProperties::GetGlobalRootSignatureForShader(const wchar_t* exportName, REFIID riid, void** outRootSignature)
{
    return ToNative()->GetGlobalRootSignatureForShader(exportName, riid, outRootSignature);
}

DXLRootSignature DXLStateObjectProperties::GetGlobalRootSignatureForProgram(const char* programName)
{
    ID3D12RootSignature* rootSig = nullptr;
    ToNative()->GetGlobalRootSignatureForProgram(WideStringConverter(programName).wideString, IID_PPV_ARGS(&rootSig));
    return DXLRootSignature(rootSig);
}

DXLRootSignature DXLStateObjectProperties::GetGlobalRootSignatureForShader(const char* exportName)
{
    ID3D12RootSignature* rootSig = nullptr;
    ToNative()->GetGlobalRootSignatureForShader(WideStringConverter(exportName).wideString, IID_PPV_ARGS(&rootSig));
    return DXLRootSignature(rootSig);
}

#endif  // DXL_ENABLE_EXTENSIONS()

uint64_t DXLStateObjectProperties::GetPipelineStackSize()
{
    return ToNative()->GetPipelineStackSize();
}

void DXLStateObjectProperties::SetPipelineStackSize(uint64_t pipelineStackSizeInBytes)
{
    return ToNative()->SetPipelineStackSize(pipelineStackSizeInBytes);
}

// == DXLWorkGraphProperties =====================================================

uint32_t DXLWorkGraphProperties::GetNumWorkGraphs()
{
    return ToNative()->GetNumWorkGraphs();
}

const wchar_t* DXLWorkGraphProperties::GetProgramName(uint32_t workGraphIndex)
{
    return ToNative()->GetProgramName(workGraphIndex);
}

uint32_t DXLWorkGraphProperties::GetWorkGraphIndex(const wchar_t* programName)
{
    return ToNative()->GetWorkGraphIndex(programName);
}

uint32_t DXLWorkGraphProperties::GetNumNodes(uint32_t workGraphIndex)
{
    return ToNative()->GetNumNodes(workGraphIndex);
}

D3D12_NODE_ID DXLWorkGraphProperties::GetNodeID(uint32_t workGraphIndex, uint32_t nodeIndex)
{
    return ToNative()->GetNodeID(workGraphIndex, nodeIndex);
}

uint32_t DXLWorkGraphProperties::GetNodeIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID)
{
    return ToNative()->GetNodeIndex(workGraphIndex, nodeID);
}

uint32_t DXLWorkGraphProperties::GetNodeLocalRootArgumentsTableIndex(uint32_t workGraphIndex, uint32_t nodeIndex)
{
    return ToNative()->GetNodeLocalRootArgumentsTableIndex(workGraphIndex, nodeIndex);
}

uint32_t DXLWorkGraphProperties::GetNumEntrypoints(uint32_t workGraphIndex)
{
    return ToNative()->GetNumEntrypoints(workGraphIndex);
}

D3D12_NODE_ID DXLWorkGraphProperties::GetEntrypointID(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointID(workGraphIndex, entrypointIndex);
}

uint32_t DXLWorkGraphProperties::GetEntrypointIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID)
{
    return ToNative()->GetEntrypointIndex(workGraphIndex, nodeID);
}

uint32_t DXLWorkGraphProperties::GetEntrypointRecordSizeInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointRecordSizeInBytes(workGraphIndex, entrypointIndex);
}

void DXLWorkGraphProperties::GetWorkGraphMemoryRequirements(uint32_t workGraphIndex, D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS* outWorkGraphMemoryRequirements)
{
    ToNative()->GetWorkGraphMemoryRequirements(workGraphIndex, outWorkGraphMemoryRequirements);
}

uint32_t DXLWorkGraphProperties::GetEntrypointRecordAlignmentInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex)
{
    return ToNative()->GetEntrypointRecordAlignmentInBytes(workGraphIndex, entrypointIndex);
}

// == DXLDescriptorHeap =====================================================

D3D12_DESCRIPTOR_HEAP_DESC DXLDescriptorHeap::GetDesc()
{
    return ToNative()->GetDesc();
}

D3D12_CPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetCPUDescriptorHandleForHeapStart()
{
    return ToNative()->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetGPUDescriptorHandleForHeapStart()
{
    return ToNative()->GetGPUDescriptorHandleForHeapStart();
}

// == DXLCommandList =====================================================

D3D12_COMMAND_LIST_TYPE DXLCommandList::GetType() const
{
    return ToNative()->GetType();
}

HRESULT DXLCommandList::Close()
{
    return ToNative()->Close();
}

HRESULT DXLCommandList::Reset(DXLCommandAllocator allocator, DXLPipelineState pipelineState)
{
    return ToNative()->Reset(allocator, pipelineState);
}

void DXLCommandList::ClearState(DXLPipelineState pipelineState)
{
    ToNative()->ClearState(pipelineState);
}

void DXLCommandList::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
    ToNative()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void DXLCommandList::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
    ToNative()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void DXLCommandList::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    ToNative()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void DXLCommandList::DispatchRays(const D3D12_DISPATCH_RAYS_DESC* desc)
{
    ToNative()->DispatchRays(desc);
}

void DXLCommandList::DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    ToNative()->DispatchMesh(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void DXLCommandList::DispatchGraph(const D3D12_DISPATCH_GRAPH_DESC* desc)
{
    ToNative()->DispatchGraph(desc);
}

void DXLCommandList::CopyBufferRegion(DXLResource dstBuffer, uint64_t dstOffset, DXLResource srcBuffer, uint64_t srcOffset, uint64_t numBytes)
{
    ToNative()->CopyBufferRegion(dstBuffer, dstOffset, srcBuffer, srcOffset, numBytes);
}

void DXLCommandList::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* dst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* srcBox)
{
    ToNative()->CopyTextureRegion(dst, dstX, dstY, dstZ, src, srcBox);
}

void DXLCommandList::CopyResource(DXLResource dstResource, DXLResource srcResource)
{
    ToNative()->CopyResource(dstResource, srcResource);
}

void DXLCommandList::CopyTiles(DXLResource tiledResource, const D3D12_TILED_RESOURCE_COORDINATE* tileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* tileRegionSize, DXLResource buffer, uint64_t bufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS flags)
{
    ToNative()->CopyTiles(tiledResource, tileRegionStartCoordinate, tileRegionSize, buffer, bufferStartOffsetInBytes, flags);
}

void DXLCommandList::Barrier(uint32_t numBarrierGroups, const D3D12_BARRIER_GROUP* barrierGroups)
{
    ToNative()->Barrier(numBarrierGroups, barrierGroups);
}

void DXLCommandList::ResolveSubresource(DXLResource dstResource, uint32_t dstSubresource, DXLResource srcResource, uint32_t srcSubresource, DXGI_FORMAT format)
{
    ToNative()->ResolveSubresource(dstResource, dstSubresource, srcResource, srcSubresource, format);
}

void DXLCommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    ToNative()->IASetPrimitiveTopology(primitiveTopology);
}

void DXLCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view)
{
    ToNative()->IASetIndexBuffer(view);
}

#if DXL_ENABLE_EXTENSIONS()

void DXLCommandList::IASetIndexBuffer(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, uint32_t sizeInBytes, DXGI_FORMAT format)
{
    D3D12_INDEX_BUFFER_VIEW d3d12View = { .BufferLocation = bufferLocation, .SizeInBytes = sizeInBytes, .Format = format };
    ToNative()->IASetIndexBuffer(&d3d12View);
}

#endif

void DXLCommandList::IASetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibStripCutValue)
{
    ToNative()->IASetIndexBufferStripCutValue(ibStripCutValue);
}

void DXLCommandList::RSSetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports)
{
    ToNative()->RSSetViewports(numViewports, viewports);
}

void DXLCommandList::RSSetScissorRects(uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->RSSetScissorRects(numRects, rects);
}

void DXLCommandList::RSSetDepthBias(float depthBias, float depthBiasClamp, float slopeScaledDepthBias)
{
    ToNative()->RSSetDepthBias(depthBias, depthBiasClamp, slopeScaledDepthBias);
}

void DXLCommandList::RSSetShadingRate(D3D12_SHADING_RATE baseShadingRate, const D3D12_SHADING_RATE_COMBINER* combiners)
{
    ToNative()->RSSetShadingRate(baseShadingRate, combiners);
}

void DXLCommandList::RSSetShadingRateImage(DXLResource shadingRateImage)
{
    ToNative()->RSSetShadingRateImage(shadingRateImage);
}

void DXLCommandList::OMSetBlendFactor(const float blendFactor[4])
{
    ToNative()->OMSetBlendFactor(blendFactor);
}

void DXLCommandList::OMSetStencilRef(uint32_t stencilRef)
{
    ToNative()->OMSetStencilRef(stencilRef);
}

void DXLCommandList::OMSetFrontAndBackStencilRef(uint32_t frontStencilRef, uint32_t backStencilRef)
{
    ToNative()->OMSetFrontAndBackStencilRef(frontStencilRef, backStencilRef);
}

void DXLCommandList::SetPipelineState(DXLPipelineState pipelineState)
{
    ToNative()->SetPipelineState(pipelineState);
}

void DXLCommandList::SetPipelineState1(DXLStateObject stateObject)
{
    ToNative()->SetPipelineState1(stateObject);
}

void DXLCommandList::SetProgram(const D3D12_SET_PROGRAM_DESC* desc)
{
    ToNative()->SetProgram(desc);
}

void DXLCommandList::SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap*const* descriptorHeaps)
{
    ToNative()->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}

#if DXL_ENABLE_EXTENSIONS()

void DXLCommandList::SetDescriptorHeaps(DXLDescriptorHeap srvUavCbvHeap, DXLDescriptorHeap samplerHeap)
{
    ID3D12DescriptorHeap* heaps[] = { srvUavCbvHeap, samplerHeap };
    ToNative()->SetDescriptorHeaps(samplerHeap ? 2 : 1, heaps);
}

#endif

void DXLCommandList::SetComputeRootSignature(DXLRootSignature rootSignature)
{
    ToNative()->SetComputeRootSignature(rootSignature);
}

void DXLCommandList::SetGraphicsRootSignature(DXLRootSignature rootSignature)
{
    ToNative()->SetGraphicsRootSignature(rootSignature);
}

#if DXL_ENABLE_DESCRIPTOR_TABLES()

void DXLCommandList::SetComputeRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToNative()->SetComputeRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

void DXLCommandList::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
    ToNative()->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptor);
}

#endif

void DXLCommandList::SetComputeRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetComputeRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetGraphicsRoot32BitConstant(uint32_t rootParameterIndex, uint32_t srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetGraphicsRoot32BitConstant(rootParameterIndex, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetComputeRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void* srcData, uint32_t destOffsetIn32BitValues)
{
    ToNative()->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, srcData, destOffsetIn32BitValues);
}

void DXLCommandList::SetComputeRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootConstantBufferView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetComputeRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootShaderResourceView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetComputeRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
    ToNative()->SetGraphicsRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void DXLCommandList::OMSetRenderTargets(uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, bool rtIsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
{
    ToNative()->OMSetRenderTargets(numRenderTargetDescriptors, renderTargetDescriptors, rtIsSingleHandleToDescriptorRange, depthStencilDescriptor);
}

void DXLCommandList::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags,float depth, uint8_t stencil, uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil, numRects, rects);
}

void DXLCommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, rects);
}

void DXLCommandList::DiscardResource(DXLResource resource, const D3D12_DISCARD_REGION* region)
{
    ToNative()->DiscardResource(resource, region);
}

void DXLCommandList::BeginRenderPass(uint32_t numRenderTargets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* renderTargets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depthStencil, D3D12_RENDER_PASS_FLAGS flags)
{
    ToNative()->BeginRenderPass(numRenderTargets, renderTargets, depthStencil, flags);
}

void DXLCommandList::EndRenderPass()
{
    ToNative()->EndRenderPass();
}

#if DXL_ENABLE_CLEAR_UAV()

void DXLCommandList::ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const uint32_t values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearUnorderedAccessViewUint(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

void DXLCommandList::ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const float values[4], uint32_t numRects, const D3D12_RECT* rects)
{
    ToNative()->ClearUnorderedAccessViewFloat(viewGPUHandleInCurrentHeap, viewCPUHandle, resource, values, numRects, rects);
}

#endif

void DXLCommandList::BeginQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToNative()->BeginQuery(queryHeap, type, index);
}

void DXLCommandList::EndQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index)
{
    ToNative()->EndQuery(queryHeap, type, index);
}

void DXLCommandList::ResolveQueryData(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t startIndex, uint32_t numQueries, DXLResource destinationBuffer, uint64_t alignedDestinationBufferOffset)
{
    ToNative()->ResolveQueryData(queryHeap, type, startIndex, numQueries, destinationBuffer, alignedDestinationBufferOffset);
}

void DXLCommandList::SetPredication(DXLResource buffer, uint64_t alignedBufferOffset, D3D12_PREDICATION_OP operation)
{
    ToNative()->SetPredication(buffer, alignedBufferOffset, operation);
}

void DXLCommandList::ExecuteIndirect(DXLCommandSignature commandSignature, uint32_t maxCommandCount, DXLResource argumentBuffer, uint64_t argumentBufferOffset, DXLResource countBuffer, uint64_t countBufferOffset)
{
    ToNative()->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

void DXLCommandList::AtomicCopyBufferUINT(ID3D12Resource* dstBuffer, uint64_t dstOffset, ID3D12Resource* srcBuffer, uint64_t srcOffset, uint32_t dependencies, ID3D12Resource*const* dependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges)
{
    ToNative()->AtomicCopyBufferUINT(dstBuffer, dstOffset, srcBuffer, srcOffset, dependencies, dependentResources, dependentSubresourceRanges);
}

// UINT64 is only valid on UMA architectures
void DXLCommandList::AtomicCopyBufferUINT64(DXLResource dstBuffer, uint64_t dstOffset, DXLResource srcBuffer, uint64_t srcOffset, uint32_t dependencies, ID3D12Resource*const* dependentResources, const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges)
{
    ToNative()->AtomicCopyBufferUINT64(dstBuffer, dstOffset, srcBuffer, srcOffset, dependencies, dependentResources, dependentSubresourceRanges);
}

void DXLCommandList::OMSetDepthBounds(float min, float max)
{
    ToNative()->OMSetDepthBounds(min, max);
}

void DXLCommandList::SetSamplePositions(uint32_t numSamplesPerPixel, uint32_t numPixels, D3D12_SAMPLE_POSITION* samplePositions)
{
    ToNative()->SetSamplePositions(numSamplesPerPixel, numPixels, samplePositions);
}

void DXLCommandList::ResolveSubresourceRegion(DXLResource dstResource, uint32_t dstSubresource, uint32_t dstX, uint32_t dstY, DXLResource srcResource, uint32_t srcSubresource, D3D12_RECT* srcRect, DXGI_FORMAT format, D3D12_RESOLVE_MODE resolveMode)
{
    ToNative()->ResolveSubresourceRegion(dstResource, dstSubresource, dstX, dstY, srcResource, srcSubresource, srcRect, format, resolveMode);
}

#if DXL_ENABLE_VIEW_INSTANCING()
void DXLCommandList::SetViewInstanceMask(uint32_t mask)
{
    ToNative()->SetViewInstanceMask(mask);
}
#endif

void DXLCommandList::WriteBufferImmediate(uint32_t count, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER* params, const D3D12_WRITEBUFFERIMMEDIATE_MODE* modes)
{
    ToNative()->WriteBufferImmediate(count, params, modes);
}

void DXLCommandList::BuildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* desc, uint32_t numPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* postbuildInfoDescs)
{
    ToNative()->BuildRaytracingAccelerationStructure(desc, numPostbuildInfoDescs, postbuildInfoDescs);
}

void DXLCommandList::EmitRaytracingAccelerationStructurePostbuildInfo(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* desc, uint32_t numSourceAccelerationStructures, const D3D12_GPU_VIRTUAL_ADDRESS* sourceAccelerationStructureData)
{
    ToNative()->EmitRaytracingAccelerationStructurePostbuildInfo(desc, numSourceAccelerationStructures, sourceAccelerationStructureData);
}

void DXLCommandList::CopyRaytracingAccelerationStructure(D3D12_GPU_VIRTUAL_ADDRESS destAccelerationStructureData, D3D12_GPU_VIRTUAL_ADDRESS sourceAccelerationStructureData, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE mode)
{
    ToNative()->CopyRaytracingAccelerationStructure(destAccelerationStructureData, sourceAccelerationStructureData, mode);
}

// == DXLCommandQueue ======================================================

void DXLCommandQueue::UpdateTileMappings(DXLResource resource, uint32_t numResourceRegions, const D3D12_TILED_RESOURCE_COORDINATE* resourceRegionStartCoordinates, const D3D12_TILE_REGION_SIZE* resourceRegionSizes, ID3D12Heap* pHeap, uint32_t numRanges, const D3D12_TILE_RANGE_FLAGS* rangeFlags, const uint32_t* heapRangeStartOffsets, const uint32_t* rangeTileCounts, D3D12_TILE_MAPPING_FLAGS flags)
{
    ToNative()->UpdateTileMappings(resource, numResourceRegions, resourceRegionStartCoordinates, resourceRegionSizes, pHeap, numRanges, rangeFlags, heapRangeStartOffsets, rangeTileCounts, flags);
}

void DXLCommandQueue::CopyTileMappings(DXLResource dstResource, const D3D12_TILED_RESOURCE_COORDINATE* dstRegionStartCoordinate, DXLResource srcResource, const D3D12_TILED_RESOURCE_COORDINATE* srcRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* regionSize, D3D12_TILE_MAPPING_FLAGS flags)
{
    ToNative()->CopyTileMappings(dstResource, dstRegionStartCoordinate, srcResource, srcRegionStartCoordinate, regionSize, flags);
}

void DXLCommandQueue::ExecuteCommandLists(uint32_t numCommandLists, ID3D12CommandList*const* commandLists)
{
    ToNative()->ExecuteCommandLists(numCommandLists, commandLists);
}

HRESULT DXLCommandQueue::Signal(DXLFence fence, uint64_t value)
{
    return ToNative()->Signal(fence, value);
}

HRESULT DXLCommandQueue::Wait(DXLFence fence, uint64_t value)
{
    return ToNative()->Wait(fence, value);
}

HRESULT DXLCommandQueue::GetTimestampFrequency(uint64_t* outFrequency) const
{
    return ToNative()->GetTimestampFrequency(outFrequency);
}

HRESULT DXLCommandQueue::GetClockCalibration(uint64_t* outGpuTimestamp, uint64_t* outCpuTimestamp) const
{
    return ToNative()->GetClockCalibration(outGpuTimestamp, outCpuTimestamp);
}

D3D12_COMMAND_QUEUE_DESC DXLCommandQueue::GetDesc() const
{
    return ToNative()->GetDesc();
}

HRESULT DXLCommandQueue::SetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY priority)
{
    return ToNative()->SetProcessPriority(priority);
}

HRESULT DXLCommandQueue::GetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY* outValue)
{
    return ToNative()->GetProcessPriority(outValue);
}

HRESULT DXLCommandQueue::SetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY priority)
{
    return ToNative()->SetGlobalPriority(priority);
}

HRESULT DXLCommandQueue::GetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY* outValue)
{
    return ToNative()->GetGlobalPriority(outValue);
}

// == DXLDevice ======================================================

uint32_t DXLDevice::GetNodeCount()
{
    return ToNative()->GetNodeCount();
}

LUID DXLDevice::GetAdapterLuid()
{
    return ToNative()->GetAdapterLuid();
}

HRESULT DXLDevice::CheckFeatureSupport(D3D12_FEATURE feature, void* featureSupportData, uint32_t featureSupportDataSize)
{
    return ToNative()->CheckFeatureSupport(feature, featureSupportData, featureSupportDataSize);
}

HRESULT DXLDevice::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* desc, REFIID riid, void** outCommandQueue)
{
    return ToNative()->CreateCommandQueue(desc, riid, outCommandQueue);
}

HRESULT DXLDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** outCommandAllocator)
{
    return ToNative()->CreateCommandAllocator(type, riid, outCommandAllocator);
}

HRESULT DXLDevice::CreateCommandList1(uint32_t nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void** outCommandList)
{
    return ToNative()->CreateCommandList1(nodeMask, type, flags, riid, outCommandList);
}

#if DXL_ENABLE_EXTENSIONS()

DXLCommandQueue DXLDevice::CreateCommandQueue(D3D12_COMMAND_QUEUE_DESC desc)
{
    DXLCommandQueue commandQueue;
    ToNative()->CreateCommandQueue(&desc, DXL_PPV_ARGS(&commandQueue));
    return commandQueue;
}

DXLCommandAllocator DXLDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    DXLCommandAllocator commandAllocator;
    ToNative()->CreateCommandAllocator(type, DXL_PPV_ARGS(&commandAllocator));
    return commandAllocator;
}

DXLCommandList DXLDevice::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags)
{
    DXLCommandList commandList;
    ToNative()->CreateCommandList1(0, type, flags, DXL_PPV_ARGS(&commandList));
    return commandList;
}

#endif // DXL_ENABLE_EXTENSIONS()

HRESULT DXLDevice::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc, REFIID riid, void** outPipelineState)
{
    return ToNative()->CreateComputePipelineState(desc, riid, outPipelineState);
}

HRESULT DXLDevice::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* desc, REFIID riid, void** outPipelineState)
{
    return ToNative()->CreatePipelineState(desc, riid, outPipelineState);
}

HRESULT DXLDevice::CreateStateObject(const D3D12_STATE_OBJECT_DESC* desc, REFIID riid, void** outStateObject)
{
    return ToNative()->CreateStateObject(desc, riid, outStateObject);
}

HRESULT DXLDevice::AddToStateObject(const D3D12_STATE_OBJECT_DESC* addition, DXLStateObject stateObjectToGrowFrom, REFIID riid, void** outNewStateObject)
{
    return ToNative()->AddToStateObject(addition, stateObjectToGrowFrom, riid, outNewStateObject);
}

#if DXL_ENABLE_EXTENSIONS()

DXLPipelineState DXLDevice::CreateComputePSO(D3D12_COMPUTE_PIPELINE_STATE_DESC desc)
{
    DXLPipelineState pso;
    ToNative()->CreateComputePipelineState(&desc, DXL_PPV_ARGS(&pso));
    return pso;
}

DXLPipelineState DXLDevice::CreateComputePSO(DXLRootSignature rootSignature, const void* byteCode, size_t byteCodeLength)
{
    return CreateComputePSO({ .pRootSignature = rootSignature, .CS = { byteCode, byteCodeLength } });
}

DXLPipelineState DXLDevice::CreateGraphicsPSO(D3D12_PIPELINE_STATE_STREAM_DESC desc)
{
    DXLPipelineState pso;
    ToNative()->CreatePipelineState(&desc, DXL_PPV_ARGS(&pso));
    return pso;
}

DXLPipelineState DXLDevice::CreateGraphicsPSO(DXL_SIMPLE_GRAPHICS_PSO_DESC desc)
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

DXLPipelineState DXLDevice::CreateGraphicsPSO(DXL_MESH_SHADER_GRAPHICS_PSO_DESC desc)
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

DXLStateObject DXLDevice::CreateStateObject(D3D12_STATE_OBJECT_DESC desc)
{
    DXLStateObject stateObject;
    ToNative()->CreateStateObject(&desc, DXL_PPV_ARGS(&stateObject));
    return stateObject;
}

DXLStateObject DXLDevice::AddToStateObject(D3D12_STATE_OBJECT_DESC addition, DXLStateObject stateObjectToGrowFrom)
{
    DXLStateObject stateObject;
    ToNative()->AddToStateObject(&addition, stateObjectToGrowFrom.ToNative(), DXL_PPV_ARGS(&stateObjectToGrowFrom));
    return stateObject;
}

#endif // DXL_ENABLE_EXTENSIONS()

HRESULT DXLDevice::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* descriptorHeapDesc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateDescriptorHeap(descriptorHeapDesc, riid, outHeap);
}

uint32_t DXLDevice::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType)
{
    return ToNative()->GetDescriptorHandleIncrementSize(descriptorHeapType);
}

HRESULT DXLDevice::CreateRootSignature(uint32_t nodeMask, const void* blobWithRootSignature, size_t blobLengthInBytes, REFIID riid, void** outRootSignature)
{
    return ToNative()->CreateRootSignature(nodeMask, blobWithRootSignature, blobLengthInBytes, riid, outRootSignature);
}

HRESULT DXLDevice::CreateRootSignatureFromSubobjectInLibrary(uint32_t nodeMask, const void* libraryBlob, size_t blobLengthInBytes, const wchar_t* subobjectName, REFIID riid, void** rootSignature)
{
    return ToNative()->CreateRootSignatureFromSubobjectInLibrary(nodeMask, libraryBlob, blobLengthInBytes, subobjectName, riid, rootSignature);
}

HRESULT DXLDevice::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* desc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateQueryHeap(desc, riid, outHeap);
}

HRESULT DXLDevice::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* desc, DXLRootSignature rootSignature, REFIID riid, void** outCommandSignature)
{
    return ToNative()->CreateCommandSignature(desc, rootSignature, riid, outCommandSignature);
}

void DXLDevice::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateConstantBufferView(desc, destDescriptor);
}

void DXLDevice::CreateShaderResourceView(DXLResource resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateShaderResourceView(resource, desc, destDescriptor);
}

void DXLDevice::CreateUnorderedAccessView(DXLResource resource, DXLResource counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateUnorderedAccessView(resource, counterResource, desc, destDescriptor);
}

void DXLDevice::CreateSamplerFeedbackUnorderedAccessView(DXLResource targetedResource, DXLResource feedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateSamplerFeedbackUnorderedAccessView(targetedResource, feedbackResource, destDescriptor);
}

void DXLDevice::CreateRenderTargetView(DXLResource resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateRenderTargetView(resource, desc, destDescriptor);
}

void DXLDevice::CreateDepthStencilView(DXLResource resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateDepthStencilView(resource, desc, destDescriptor);
}

void DXLDevice::CreateSampler2(const D3D12_SAMPLER_DESC2* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor)
{
    ToNative()->CreateSampler2(desc, destDescriptor);
}

D3D12_RESOURCE_ALLOCATION_INFO DXLDevice::GetResourceAllocationInfo3(uint32_t visibleMask, uint32_t numResourceDescs, const D3D12_RESOURCE_DESC1* resourceDescs, const uint32_t* numCastableFormats, const DXGI_FORMAT*const* castableFormats, D3D12_RESOURCE_ALLOCATION_INFO1* resourceAllocationInfo)
{
    return ToNative()->GetResourceAllocationInfo3(visibleMask, numResourceDescs, resourceDescs, numCastableFormats, castableFormats, resourceAllocationInfo);
}

HRESULT DXLDevice::CreateHeap(const D3D12_HEAP_DESC* desc, REFIID riid, void** outHeap)
{
    return ToNative()->CreateHeap(desc, riid, outHeap);
}

HRESULT DXLDevice::OpenExistingHeapFromAddress1(const void* address, size_t size, REFIID riid, void** outHeap)
{
    return ToNative()->OpenExistingHeapFromAddress1(address, size, riid, outHeap);
}

HRESULT DXLDevice::OpenExistingHeapFromFileMapping(HANDLE fileMapping, REFIID riid, void** outHeap)
{
    return ToNative()->OpenExistingHeapFromFileMapping(fileMapping, riid, outHeap);
}

D3D12_HEAP_PROPERTIES DXLDevice::GetCustomHeapProperties(uint32_t nodeMask, D3D12_HEAP_TYPE heapType)
{
    return ToNative()->GetCustomHeapProperties(nodeMask, heapType);
}

HRESULT DXLDevice::CreateCommittedResource3(const D3D12_HEAP_PROPERTIES* heapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC1* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ID3D12ProtectedResourceSession* protectedSession, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreateCommittedResource3(heapProperties, heapFlags, desc, initialLayout, optimizedClearValue, protectedSession, numCastableFormats, castableFormats, riid, outResource);
}

HRESULT DXLDevice::CreatePlacedResource2(ID3D12Heap* heap, uint64_t heapOffset, const D3D12_RESOURCE_DESC1* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreatePlacedResource2(heap, heapOffset, desc, initialLayout, optimizedClearValue, numCastableFormats, castableFormats, riid, outResource);
}

HRESULT DXLDevice::CreateReservedResource2(const D3D12_RESOURCE_DESC* desc, D3D12_BARRIER_LAYOUT initialLayout, const D3D12_CLEAR_VALUE* optimizedClearValue, ID3D12ProtectedResourceSession* protectedSession, uint32_t numCastableFormats, const DXGI_FORMAT* castableFormats, REFIID riid, void** outResource)
{
    return ToNative()->CreateReservedResource2(desc, initialLayout, optimizedClearValue, protectedSession, numCastableFormats, castableFormats, riid, outResource);
}

void DXLDevice::GetResourceTiling(DXLResource tiledResource, uint32_t* outNumTilesForEntireResource, D3D12_PACKED_MIP_INFO* outPackedMipDesc, D3D12_TILE_SHAPE* outStandardTileShapeForNonPackedMips, uint32_t* numSubresourceTilings, uint32_t firstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING* outSubresourceTilingsForNonPackedMips)
{
    return ToNative()->GetResourceTiling(tiledResource, outNumTilesForEntireResource, outPackedMipDesc, outStandardTileShapeForNonPackedMips, numSubresourceTilings, firstSubresourceTilingToGet, outSubresourceTilingsForNonPackedMips);
}

HRESULT DXLDevice::CreateSharedHandle(DXLDeviceChild object, const SECURITY_ATTRIBUTES* attributes, uint32_t access, const wchar_t* name, HANDLE* outHandle)
{
    return ToNative()->CreateSharedHandle(object, attributes, access, name, outHandle);
}

HRESULT DXLDevice::OpenSharedHandle(HANDLE ntHandle, REFIID riid, void** outObj)
{
    return ToNative()->OpenSharedHandle(ntHandle, riid, outObj);
}

HRESULT DXLDevice::OpenSharedHandleByName(const wchar_t* name, uint32_t access, HANDLE* outHandle)
{
    return ToNative()->OpenSharedHandleByName(name, access, outHandle);
}

HRESULT DXLDevice::MakeResident(uint32_t numObjects, ID3D12Pageable*const* objects)
{
    return ToNative()->MakeResident(numObjects, objects);
}

HRESULT DXLDevice::EnqueueMakeResident(D3D12_RESIDENCY_FLAGS flags, uint32_t numObjects, ID3D12Pageable*const* objects, ID3D12Fence* fenceToSignal, UINT64 fenceValueToSignal)
{
    return ToNative()->EnqueueMakeResident(flags, numObjects, objects, fenceToSignal, fenceValueToSignal);
}

HRESULT DXLDevice::Evict(uint32_t numObjects, ID3D12Pageable*const* objects)
{
    return ToNative()->Evict(numObjects, objects);
}

HRESULT DXLDevice::SetResidencyPriority(uint32_t numObjects, ID3D12Pageable*const* objects, const D3D12_RESIDENCY_PRIORITY* priorities)
{
    return ToNative()->SetResidencyPriority(numObjects, objects, priorities);
}

HRESULT DXLDevice::CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS flags, REFIID riid, void** outFence)
{
    return ToNative()->CreateFence(initialValue, flags, riid, outFence);
}

HRESULT DXLDevice::SetEventOnMultipleFenceCompletion(ID3D12Fence*const* fences, const uint64_t* fenceValues, uint32_t numFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS flags, HANDLE event)
{
    return ToNative()->SetEventOnMultipleFenceCompletion(fences, fenceValues, numFences, flags, event);
}

void DXLDevice::GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* desc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* outInfo)
{
    ToNative()->GetRaytracingAccelerationStructurePrebuildInfo(desc, outInfo);
}

void DXLDevice::RemoveDevice()
{
    ToNative()->RemoveDevice();
}

HRESULT DXLDevice::GetDeviceRemovedReason()
{
    return ToNative()->GetDeviceRemovedReason();
}

void DXLDevice::GetCopyableFootprints1(const D3D12_RESOURCE_DESC1* resourceDesc, uint32_t firstSubresource, uint32_t numSubresources, uint64_t baseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts, uint32_t* numRows, uint64_t* rowSizeInBytes, uint64_t* totalBytes)
{
    ToNative()->GetCopyableFootprints1(resourceDesc, firstSubresource, numSubresources, baseOffset, layouts, numRows, rowSizeInBytes, totalBytes);
}

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES()

HRESULT DXLDevice::SetStablePowerState(bool enable)
{
    return ToNative()->SetStablePowerState(enable);
}

HRESULT DXLDevice::SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE mode, D3D12_MEASUREMENTS_ACTION measurementsAction, HANDLE eventToSignalUponCompletion, BOOL* outFurtherMeasurementsDesired)
{
    return ToNative()->SetBackgroundProcessingMode(mode, measurementsAction, eventToSignalUponCompletion, outFurtherMeasurementsDesired);
}

#endif // DXL_ENABLE_DEVELOPER_ONLY_FEATURES()

// == DXLDebug ======================================================

#if DXL_ENABLE_EXTENSIONS()

DXLSwapChain DXLSwapChain::Create(DXGI_SWAP_CHAIN_DESC desc, DXLCommandQueue presentQueue)
{
    ComPtr<IDXGIFactory7> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return DXLSwapChain();

    ComPtr<IDXGISwapChain> tempSwapChain;
    hr = factory->CreateSwapChain(presentQueue, &desc, &tempSwapChain);
    if (FAILED(hr))
        return DXLSwapChain();

    DXLSwapChain swapChain;
    tempSwapChain->QueryInterface(DXL_PPV_ARGS(&swapChain));

    return swapChain;
}

#endif

HRESULT DXLSwapChain::Present1(uint32_t syncInterval, uint32_t presentFlags, const DXGI_PRESENT_PARAMETERS* presentParameters)
{
    return ToNative()->Present1(syncInterval, presentFlags, presentParameters);
}

HRESULT DXLSwapChain::ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{
    return ToNative()->ResizeBuffers(bufferCount, width, height, newFormat, swapChainFlags);
}

uint32_t DXLSwapChain::GetCurrentBackBufferIndex() const
{
    return ToNative()->GetCurrentBackBufferIndex();
}

HRESULT DXLSwapChain::GetBuffer(uint32_t buffer, REFIID riid, void** outSurface) const
{
    return ToNative()->GetBuffer(buffer, riid, outSurface);
}

HRESULT DXLSwapChain::GetDesc1(DXGI_SWAP_CHAIN_DESC1* outDesc)
{
    return ToNative()->GetDesc1(outDesc);
}

HRESULT DXLSwapChain::GetHwnd(HWND* outHwnd)
{
    return ToNative()->GetHwnd(outHwnd);
}

HRESULT DXLSwapChain::SetMaximumFrameLatency(uint32_t maxLatency)
{
    return ToNative()->SetMaximumFrameLatency(maxLatency);
}

HRESULT DXLSwapChain::GetMaximumFrameLatency(uint32_t* maxLatency)
{
    return ToNative()->GetMaximumFrameLatency(maxLatency);
}

HANDLE DXLSwapChain::GetFrameLatencyWaitableObject()
{
    return ToNative()->GetFrameLatencyWaitableObject();
}

HRESULT DXLSwapChain::CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE colorSpace, uint32_t* colorSpaceSupport)
{
    return ToNative()->CheckColorSpaceSupport(colorSpace, colorSpaceSupport);
}

HRESULT DXLSwapChain::SetColorSpace1(DXGI_COLOR_SPACE_TYPE colorSpace)
{
    return ToNative()->SetColorSpace1(colorSpace);
}

HRESULT DXLSwapChain::SetHDRMetaData(DXGI_HDR_METADATA_TYPE type, uint32_t size, void* metaData)
{
    return ToNative()->SetHDRMetaData(type, size, metaData);
}

HRESULT DXLSwapChain::SetRotation(DXGI_MODE_ROTATION rotation)
{
    return ToNative()->SetRotation(rotation);
}

HRESULT DXLSwapChain::GetRotation(DXGI_MODE_ROTATION* outRotation)
{
    return ToNative()->GetRotation(outRotation);
}

HRESULT DXLSwapChain::GetContainingOutput(IDXGIOutput** outOutput)
{
    return ToNative()->GetContainingOutput(outOutput);
}

HRESULT DXLSwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS* outStats)
{
    return ToNative()->GetFrameStatistics(outStats);
}

HRESULT DXLSwapChain::GetLastPresentCount(uint32_t* outLastPresentCount)
{
    return ToNative()->GetLastPresentCount(outLastPresentCount);
}

#if DXL_ENABLE_EXTENSIONS()

DXLResource DXLSwapChain::GetBuffer(uint32_t bufferIndex) const
{
    DXLResource resource;
    ToNative()->GetBuffer(bufferIndex, DXL_PPV_ARGS(&resource));
    return resource;
}

DXGI_SWAP_CHAIN_DESC1 DXLSwapChain::GetDesc() const
{
    DXGI_SWAP_CHAIN_DESC1 desc = { };
    ToNative()->GetDesc1(&desc);
    return desc;
}

HWND DXLSwapChain::GetHwnd() const
{
    HWND hwnd = nullptr;
    ToNative()->GetHwnd(&hwnd);
    return hwnd;
}

uint32_t DXLSwapChain::GetMaximumFrameLatency()
{
    uint32_t latency = 0;
    ToNative()->GetMaximumFrameLatency(&latency);
    return latency;
}

DXGI_FRAME_STATISTICS DXLSwapChain::GetFrameStatistics()
{
    DXGI_FRAME_STATISTICS stats = { };
    ToNative()->GetFrameStatistics(&stats);
    return stats;
}

uint32_t DXLSwapChain::GetLastPresentCount()
{
    uint32_t presentCount = 0;
    ToNative()->GetLastPresentCount(&presentCount);
    return presentCount;
}

DXGI_MODE_ROTATION DXLSwapChain::GetRotation()
{
    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
    ToNative()->GetRotation(&rotation);
    return rotation;
}

#endif // DXL_ENABLE_EXTENSIONS()

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES()

// == DXLDebug ======================================================

void DXLDebug::EnableDebugLayer()
{
    ToNative()->EnableDebugLayer();
}

void DXLDebug::DisableDebugLayer()
{
    ToNative()->DisableDebugLayer();
}

void DXLDebug::SetEnableGPUBasedValidation(bool enable)
{
    ToNative()->SetEnableGPUBasedValidation(enable);
}

void DXLDebug::SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS flags)
{
    ToNative()->SetGPUBasedValidationFlags(flags);
}

void DXLDebug::SetEnableSynchronizedCommandQueueValidation(bool enable)
{
    ToNative()->SetEnableSynchronizedCommandQueueValidation(enable);
}

void DXLDebug::SetEnableAutoName(bool enable)
{
    ToNative()->SetEnableAutoName(enable);
}

// == DXLDebugDevice ======================================================

#if DXL_ENABLE_EXTENSIONS()

DXLDebugDevice DXLDebugDevice::FromDevice(DXLDevice device)
{
    ID3D12DebugDevice2* debugDevice = nullptr;
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&debugDevice))))
        DXL_ASSERT(false, "Failed to QueryInterface debug device interface from DXLDevice. Did you enable the debug layer?");

    return DXLDebugDevice(debugDevice);
}

#endif

HRESULT DXLDebugDevice::SetFeatureMask(D3D12_DEBUG_FEATURE mask)
{
    return ToNative()->SetFeatureMask(mask);
}

D3D12_DEBUG_FEATURE DXLDebugDevice::GetFeatureMask()
{
    return ToNative()->GetFeatureMask();
}

HRESULT DXLDebugDevice::ReportLiveDeviceObjects(D3D12_RLDO_FLAGS flags)
{
    return ToNative()->ReportLiveDeviceObjects(flags);
}

HRESULT DXLDebugDevice::SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, const void*data, uint32_t dataSize)
{
    return ToNative()->SetDebugParameter(type, data, dataSize);
}

HRESULT DXLDebugDevice::GetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, void* data, uint32_t dataSize)
{
    return ToNative()->GetDebugParameter(type, data, dataSize);
}

// == DXLDebugCommandQueue ======================================================

#if DXL_ENABLE_EXTENSIONS()

DXLDebugCommandQueue DXLDebugCommandQueue::FromCommandQueue(DXLCommandQueue commandQueue)
{
    ID3D12DebugCommandQueue1* debugQueue = nullptr;    
    if (FAILED(commandQueue->QueryInterface(IID_PPV_ARGS(&debugQueue))))
        DXL_ASSERT(false, "Failed to QueryInterface debug command queue interface from DXLCommandQueue. Did you enable the debug layer?");

    return DXLDebugCommandQueue(debugQueue);
}

#endif

void DXLDebugCommandQueue::AssertResourceAccess(DXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access)
{
    ToNative()->AssertResourceAccess(resource, subresource, access);
}

void DXLDebugCommandQueue::AssertTextureLayout(DXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout)
{
    ToNative()->AssertTextureLayout(resource, subresource, layout);
}

// == DXLDebugCommandList ======================================================

#if DXL_ENABLE_EXTENSIONS()

DXLDebugCommandList DXLDebugCommandList::FromCommandList(DXLCommandList commandList)
{
    ID3D12DebugCommandList3* debugList = nullptr;
    if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&debugList))))
        DXL_ASSERT(false, "Failed to QueryInterface debug command list interface from DXLCommandList. Did you enable the debug layer?");

    return DXLDebugCommandList(debugList);
}

#endif

HRESULT DXLDebugCommandList::SetFeatureMask(D3D12_DEBUG_FEATURE mask)
{
    return ToNative()->SetFeatureMask(mask);
}

D3D12_DEBUG_FEATURE DXLDebugCommandList::GetFeatureMask()
{
    return ToNative()->GetFeatureMask();
}

HRESULT DXLDebugCommandList::SetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, const void*data, uint32_t dataSize)
{
    return ToNative()->SetDebugParameter(type, data, dataSize);
}

HRESULT DXLDebugCommandList::GetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, void* data, uint32_t dataSize)
{
    return ToNative()->GetDebugParameter(type, data, dataSize);
}

void DXLDebugCommandList::AssertResourceAccess(DXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access)
{
    ToNative()->AssertResourceAccess(resource, subresource, access);
}

void DXLDebugCommandList::AssertTextureLayout(DXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout)
{
    ToNative()->AssertTextureLayout(resource, subresource, layout);
}

// == DXLDebugInfoQueue ======================================================

#if DXL_ENABLE_EXTENSIONS()

DXLDebugInfoQueue DXLDebugInfoQueue::FromDevice(DXLDevice device)
{
    ID3D12InfoQueue1* infoQueue = nullptr;    
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        DXL_ASSERT(false, "Failed to QueryInterface debug command info queue interface from DXLDevice. Did you enable the debug layer?");

    return DXLDebugInfoQueue(infoQueue);
}

#endif

HRESULT DXLDebugInfoQueue::RegisterMessageCallback(D3D12MessageFunc callbackFunc, D3D12_MESSAGE_CALLBACK_FLAGS callbackFilterFlags, void* context, DWORD* outCallbackCookie)
{
    return ToNative()->RegisterMessageCallback(callbackFunc, callbackFilterFlags, context, outCallbackCookie);
}

HRESULT DXLDebugInfoQueue::UnregisterMessageCallback(DWORD callbackCookie)
{
    return ToNative()->UnregisterMessageCallback(callbackCookie);
}

void DXLDebugInfoQueue::SetMuteDebugOutput(bool mute)
{
    ToNative()->SetMuteDebugOutput(mute);
}

bool DXLDebugInfoQueue::GetMuteDebugOutput()
{
    return ToNative()->GetMuteDebugOutput();
}

#endif // DXL_ENABLE_DEVELOPER_ONLY_FEATURES()

#if DXL_ENABLE_EXTENSIONS()

CreateDeviceResult CreateDevice(CreateDeviceParams params)
{
    ComPtr<IDXGIFactory7> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return { DXLDevice(), hr, "Failed to create DXGI factory" };

    ComPtr<IDXGIAdapter4> adapter;
    hr = factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
    if (FAILED(hr))
        return { DXLDevice(), hr, "Failed to enumerage a DXGI adapter" };

    DXGI_ADAPTER_DESC1 desc = { };
    adapter->GetDesc1(&desc);
    PrintMessage("Creating D3D12 device on adapter '%ls'", desc.Description);

    ComPtr<ID3D12SDKConfiguration1> sdkConfig;
    hr = D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&sdkConfig));
    if (FAILED(hr))
        return { DXLDevice(), hr, "Failed to aquire the D3D12 SDK configuration interface" };

    ComPtr<ID3D12DeviceFactory> deviceFactory;
    hr = sdkConfig->CreateDeviceFactory(D3D12_SDK_VERSION, params.AgilitySDKPath, IID_PPV_ARGS(&deviceFactory));
    if (FAILED(hr))
        return { DXLDevice(), hr, "Failed to create a D3D12 device factory. Did you pass the wrong AgilitySDKPath?" };

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES()
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
        return { DXLDevice(), hr, "Failed to create D3D12 device from the device factory" };

    D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = { };
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12));
    if(options12.EnhancedBarriersSupported == false)
        return { DXLDevice(), E_FAIL, "The selected GPU does not support enhanced barriers, which is required for DXLatest" };

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

void Release(DXLBase& base)
{
    if (base)
    {
        base->Release();
        base = DXLBase();
    }
}

#endif // DXL_ENABLE_EXTENSIONS()

} // namespace dxl