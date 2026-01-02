#pragma once

#include <stdint.h>

#include "AgilitySDK/include/d3d12.h"

#define DXL_STRUCT_BOILERPLATE(DXLStruct, D3D12Struct)  \
    DXLStruct() = default;                              \
    DXLStruct(D3D12Struct d3d12Struct) { memcpy(this, &d3d12Struct, sizeof(DXLStruct)); }        \
    operator D3D12Struct() const { D3D12Struct d3d12Struct = { }; memcpy(&d3d12Struct, this, sizeof(DXLStruct)); return d3d12Struct; }

#define DXL_ENABLE_DESCRIPTOR_TABLES() true
#define DXL_ENABLE_CLEAR_UAV() true
#define DXL_ENABLE_VIEW_INSTANCING() true
#define DXL_ENABLE_DEVELOPER_ONLY_FEATURES() true

#define DXL_ENABLE_EXTENSIONS() true

namespace dxl
{

#define DXL_INTERFACE_BOILERPLATE(DXLClass, D3D12Interface) \
    DXLClass() = default;   \
    DXLClass(D3D12Interface* d3d12Interface) { nativeInterface = d3d12Interface; }   \
    D3D12Interface* ToNative() const { return reinterpret_cast<D3D12Interface*>(nativeInterface); }     \
    D3D12Interface** AddressOfNative() { return reinterpret_cast<D3D12Interface**>(&nativeInterface); }     \
    static IID InterfaceID() { return __uuidof(D3D12Interface); }   \
    DXLClass* operator->() { return this; }     \
    const DXLClass* operator->() const { return this; }     \
    operator D3D12Interface*() const { return ToNative(); } \
    bool operator==(D3D12Interface* other) const { return nativeInterface == other; }    \
    bool operator==(DXLClass other) const { return nativeInterface == other.nativeInterface; }    \
    bool operator!=(D3D12Interface* other) const { return nativeInterface != other; }    \
    bool operator!=(DXLClass other) const { return nativeInterface != other.nativeInterface; }

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
        return nativeInterface != nullptr;
    }

protected:

    IUnknown* nativeInterface = nullptr;
};

class DXLObject : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLObject, ID3D12Object);

    HRESULT GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const;
    HRESULT SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data);
    HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* data);
    HRESULT SetName(const wchar_t* name);

#if DXL_ENABLE_EXTENSIONS()
    HRESULT SetName(const char* name);
#endif
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

    D3D12_HEAP_DESC GetDesc() const;
};

class DXLResource : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLResource, ID3D12Resource2);

    HRESULT Map(uint32_t subresource, const D3D12_RANGE* readRange, void** outData);
    void Unmap(uint32_t subresource, const D3D12_RANGE* writtenRange);

#if DXL_ENABLE_EXTENSIONS()
    void* Map(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);
    void Unmap(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);
#endif

    D3D12_RESOURCE_DESC1 GetDesc1() const;

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    HRESULT WriteToSubresource(uint32_t dstSubresource, const D3D12_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch);
    HRESULT ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const D3D12_BOX* srcBox) const;

    HRESULT GetHeapProperties(D3D12_HEAP_PROPERTIES* outHeapProperties, D3D12_HEAP_FLAGS* outHeapFlags) const;
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

    HRESULT GetRootSignature(REFIID riid, void** outRootSignature) const;
#if DXL_ENABLE_EXTENSIONS()
    DXLRootSignature GetRootSignature() const;
#endif
};

class DXLStateObject : public DXLPageable
{
public:

    DXL_INTERFACE_BOILERPLATE(DXLStateObject, ID3D12StateObject);
};

class DXLStateObjectProperties : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLStateObjectProperties, ID3D12StateObjectProperties2);

    void* GetShaderIdentifier(const wchar_t* exportName);
    uint64_t GetShaderStackSize(const wchar_t* exportName);
    D3D12_PROGRAM_IDENTIFIER GetProgramIdentifier(const wchar_t* programName);
    HRESULT GetGlobalRootSignatureForProgram(const wchar_t* programName, REFIID riid, void** outRootSignature);
    HRESULT GetGlobalRootSignatureForShader(const wchar_t* exportName, REFIID riid, void** outRootSignature);

#if DXL_ENABLE_EXTENSIONS()
    void* GetShaderIdentifier(const char* exportName);
    uint64_t GetShaderStackSize(const char* exportName);
    D3D12_PROGRAM_IDENTIFIER GetProgramIdentifier(const char* programName);
    DXLRootSignature GetGlobalRootSignatureForProgram(const char* programName);
    DXLRootSignature GetGlobalRootSignatureForShader(const char* exportName);
#endif

    uint64_t GetPipelineStackSize();
    void SetPipelineStackSize(uint64_t pipelineStackSizeInBytes);
};

class DXLWorkGraphProperties : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLWorkGraphProperties, ID3D12WorkGraphProperties);

    uint32_t GetNumWorkGraphs();
    const wchar_t* GetProgramName(uint32_t workGraphIndex);
    uint32_t GetWorkGraphIndex(const wchar_t* programName);

    uint32_t GetNumNodes(uint32_t workGraphIndex);
    D3D12_NODE_ID GetNodeID(uint32_t workGraphIndex, uint32_t nodeIndex);
    uint32_t GetNodeIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID);
    uint32_t GetNodeLocalRootArgumentsTableIndex(uint32_t workGraphIndex, uint32_t nodeIndex);

    uint32_t GetNumEntrypoints(uint32_t workGraphIndex);
    D3D12_NODE_ID GetEntrypointID(uint32_t workGraphIndex, uint32_t entrypointIndex);
    uint32_t GetEntrypointIndex(uint32_t workGraphIndex, D3D12_NODE_ID nodeID);
    uint32_t GetEntrypointRecordSizeInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex);

    void GetWorkGraphMemoryRequirements(uint32_t workGraphIndex, D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS* outWorkGraphMemoryRequirements);

    uint32_t GetEntrypointRecordAlignmentInBytes(uint32_t workGraphIndex, uint32_t entrypointIndex);
};

class DXLDescriptorHeap : public DXLPageable
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDescriptorHeap, ID3D12DescriptorHeap);

    D3D12_DESCRIPTOR_HEAP_DESC GetDesc();
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
    void DispatchRays(const D3D12_DISPATCH_RAYS_DESC* desc);
    void DispatchMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);
    void DispatchGraph(const D3D12_DISPATCH_GRAPH_DESC* desc);

    void CopyBufferRegion(DXLResource dstBuffer, uint64_t dstOffset, DXLResource srcBuffer, uint64_t srcOffset, uint64_t numBytes);
    void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* dst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* srcBox);
    void CopyResource(DXLResource dstResource, DXLResource srcResource);
    void CopyTiles(DXLResource tiledResource, const D3D12_TILED_RESOURCE_COORDINATE* tileRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* tileRegionSize, DXLResource buffer, uint64_t bufferStartOffsetInBytes, D3D12_TILE_COPY_FLAGS flags);

    void Barrier(uint32_t numBarrierGroups, const D3D12_BARRIER_GROUP* barrierGroups);

    void ResolveSubresource(DXLResource dstResource, uint32_t dstSubresource, DXLResource srcResource, uint32_t srcSubresource, DXGI_FORMAT format);

    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* view);
#if DXL_ENABLE_EXTENSIONS()
    void IASetIndexBuffer(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, uint32_t sizeInBytes, DXGI_FORMAT format = DXGI_FORMAT_R16_UINT);
#endif
    void IASetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibStripCutValue);

    void RSSetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports);
    void RSSetScissorRects(uint32_t numRects, const D3D12_RECT* rects);
    void RSSetDepthBias(float depthBias, float depthBiasClamp, float slopeScaledDepthBias);

    void RSSetShadingRate(D3D12_SHADING_RATE baseShadingRate, const D3D12_SHADING_RATE_COMBINER* combiners);
    void RSSetShadingRateImage(DXLResource shadingRateImage);

    void OMSetBlendFactor(const float blendFactor[4]);
    void OMSetStencilRef(uint32_t stencilRef);
    void OMSetFrontAndBackStencilRef(uint32_t frontStencilRef, uint32_t backStencilRef);

    void SetPipelineState(DXLPipelineState pipelineState);
    void SetPipelineState1(DXLStateObject stateObject);
    void SetProgram(const D3D12_SET_PROGRAM_DESC* desc);

    void SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap*const* descriptorHeaps);
#if DXL_ENABLE_EXTENSIONS()
    void SetDescriptorHeaps(DXLDescriptorHeap srvUavCbvHeap, DXLDescriptorHeap samplerHeap = DXLDescriptorHeap());
#endif

    void SetComputeRootSignature(DXLRootSignature rootSignature);
    void SetGraphicsRootSignature(DXLRootSignature rootSignature);

#if DXL_ENABLE_DESCRIPTOR_TABLES()
    void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
    void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);
#endif

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

    void OMSetRenderTargets(uint32_t numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetDescriptors, bool rtIsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, D3D12_CLEAR_FLAGS clearFlags,float depth, uint8_t stencil, uint32_t numRects, const D3D12_RECT* rects);
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float colorRGBA[4], uint32_t numRects, const D3D12_RECT* rects);
    void DiscardResource(DXLResource resource, const D3D12_DISCARD_REGION* region);

    void BeginRenderPass(uint32_t numRenderTargets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* renderTargets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depthStencil, D3D12_RENDER_PASS_FLAGS flags);
    void EndRenderPass();

#if DXL_ENABLE_CLEAR_UAV()
    void ClearUnorderedAccessViewUint(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const uint32_t values[4], uint32_t numRects, const D3D12_RECT* rects);
    void ClearUnorderedAccessViewFloat(D3D12_GPU_DESCRIPTOR_HANDLE viewGPUHandleInCurrentHeap, D3D12_CPU_DESCRIPTOR_HANDLE viewCPUHandle, DXLResource resource, const float values[4], uint32_t numRects, const D3D12_RECT* rects);
#endif

    void BeginQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index);
    void EndQuery(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t index);
    void ResolveQueryData(DXLQueryHeap queryHeap, D3D12_QUERY_TYPE type, uint32_t startIndex, uint32_t numQueries, DXLResource destinationBuffer, uint64_t alignedDestinationBufferOffset);
    void SetPredication(DXLResource buffer, uint64_t alignedBufferOffset, D3D12_PREDICATION_OP operation);

    void ExecuteIndirect(DXLCommandSignature commandSignature, uint32_t maxCommandCount, DXLResource argumentBuffer, uint64_t argumentBufferOffset, DXLResource countBuffer, uint64_t countBufferOffset);

    void AtomicCopyBufferUINT(
        ID3D12Resource* dstBuffer,
        uint64_t dstOffset,
        ID3D12Resource* srcBuffer,
        uint64_t srcOffset,
        uint32_t dependencies, // 0 Dependencies means only the dst buffer offset is synchronized
        ID3D12Resource*const* dependentResources,
        const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges
    );

    // UINT64 is only valid on UMA architectures
    void AtomicCopyBufferUINT64(
        DXLResource dstBuffer,
        uint64_t dstOffset,
        DXLResource srcBuffer,
        uint64_t srcOffset,
        uint32_t dependencies, // 0 Dependencies means only the dst buffer offset is synchronized
        ID3D12Resource*const* dependentResources,
        const D3D12_SUBRESOURCE_RANGE_UINT64* dependentSubresourceRanges
    );

    void OMSetDepthBounds(float min, float max);

    void SetSamplePositions(uint32_t numSamplesPerPixel, uint32_t numPixels, D3D12_SAMPLE_POSITION* samplePositions);

    void ResolveSubresourceRegion(
        DXLResource dstResource,
        uint32_t dstSubresource,
        uint32_t dstX,
        uint32_t dstY,
        DXLResource srcResource,
        uint32_t srcSubresource,
        D3D12_RECT* srcRect,
        DXGI_FORMAT format,
        D3D12_RESOLVE_MODE resolveMode
    );

    void WriteBufferImmediate(uint32_t count, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER* params, const D3D12_WRITEBUFFERIMMEDIATE_MODE* modes);

#if DXL_ENABLE_VIEW_INSTANCING()
    void SetViewInstanceMask(uint32_t mask);
#endif

    void BuildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* desc, uint32_t numPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* postbuildInfoDescs);
    void EmitRaytracingAccelerationStructurePostbuildInfo(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* desc, uint32_t numSourceAccelerationStructures, const D3D12_GPU_VIRTUAL_ADDRESS* sourceAccelerationStructureData);
    void CopyRaytracingAccelerationStructure(D3D12_GPU_VIRTUAL_ADDRESS destAccelerationStructureData, D3D12_GPU_VIRTUAL_ADDRESS sourceAccelerationStructureData, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE mode);
};

class DXLCommandQueue : public DXLPageable
{
public:

    DXL_INTERFACE_BOILERPLATE(DXLCommandQueue, ID3D12CommandQueue1);

    void UpdateTileMappings(
        DXLResource resource,
        uint32_t numResourceRegions,
        const D3D12_TILED_RESOURCE_COORDINATE* resourceRegionStartCoordinates,
        const D3D12_TILE_REGION_SIZE* resourceRegionSizes,
        ID3D12Heap* pHeap,
        uint32_t numRanges,
        const D3D12_TILE_RANGE_FLAGS* rangeFlags,
        const uint32_t* heapRangeStartOffsets,
        const uint32_t* rangeTileCounts,
        D3D12_TILE_MAPPING_FLAGS flags
    );

    void CopyTileMappings(
        DXLResource dstResource,
        const D3D12_TILED_RESOURCE_COORDINATE* dstRegionStartCoordinate,
        DXLResource srcResource,
        const D3D12_TILED_RESOURCE_COORDINATE* srcRegionStartCoordinate,
        const D3D12_TILE_REGION_SIZE* regionSize,
        D3D12_TILE_MAPPING_FLAGS flags
    );

    void ExecuteCommandLists(uint32_t numCommandLists, ID3D12CommandList*const* commandLists);

    HRESULT Signal(DXLFence fence, uint64_t value);
    HRESULT Wait(DXLFence fence, uint64_t value);

    HRESULT GetTimestampFrequency(uint64_t* outFrequency) const;

    HRESULT GetClockCalibration(uint64_t* outGpuTimestamp, uint64_t* outCpuTimestamp) const;

    D3D12_COMMAND_QUEUE_DESC GetDesc() const;

    HRESULT SetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY priority);
    HRESULT GetProcessPriority(D3D12_COMMAND_QUEUE_PROCESS_PRIORITY* outValue);

    HRESULT SetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY priority);
    HRESULT GetGlobalPriority(D3D12_COMMAND_QUEUE_GLOBAL_PRIORITY* outValue);
};

class DXLDevice : public DXLObject
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDevice, ID3D12Device14);

    uint32_t GetNodeCount();
    LUID GetAdapterLuid();

    HRESULT CheckFeatureSupport(D3D12_FEATURE feature, void* featureSupportData, uint32_t featureSupportDataSize);

    HRESULT CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* desc, REFIID riid, void** outCommandQueue);
    HRESULT CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** outCommandAllocator);
    HRESULT CreateCommandList1(uint32_t nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void** outCommandList);

    HRESULT CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc, REFIID riid, void** outPipelineState);
    HRESULT CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc, REFIID riid, void** outOipelineState);
    HRESULT CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC* desc, REFIID riid, void** outPipelineState);
    HRESULT CreateStateObject(const D3D12_STATE_OBJECT_DESC* desc, REFIID riid, void** outStateObject);
    HRESULT AddToStateObject(const D3D12_STATE_OBJECT_DESC* addition, DXLStateObject stateObjectToGrowFrom, REFIID riid, void** outNewStateObject);

    HRESULT CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* descriptorHeapDesc, REFIID riid, void** outHeap);
    uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType);

    HRESULT CreateRootSignature(uint32_t nodeMask, const void* blobWithRootSignature, size_t blobLengthInBytes, REFIID riid, void** outRootSignature);
    HRESULT CreateRootSignatureFromSubobjectInLibrary(uint32_t nodeMask, const void* libraryBlob, size_t blobLengthInBytes, const wchar_t* subobjectName, REFIID riid, void** rootSignature);

    HRESULT CreateQueryHeap(const D3D12_QUERY_HEAP_DESC* desc, REFIID riid, void** outHeap);

    HRESULT CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC* desc, DXLRootSignature rootSignature, REFIID riid, void** outCommandSignature);

    void CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateShaderResourceView(DXLResource resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateUnorderedAccessView(DXLResource resource, DXLResource counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateSamplerFeedbackUnorderedAccessView(DXLResource targetedResource, DXLResource feedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateRenderTargetView(DXLResource resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateDepthStencilView(DXLResource resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);
    void CreateSampler2(const D3D12_SAMPLER_DESC2* desc, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor);

    D3D12_RESOURCE_ALLOCATION_INFO GetResourceAllocationInfo3(
        uint32_t visibleMask,
        uint32_t numResourceDescs,
        const D3D12_RESOURCE_DESC1* resourceDescs,
        const uint32_t* numCastableFormats,
        const DXGI_FORMAT*const* castableFormats,
        D3D12_RESOURCE_ALLOCATION_INFO1* resourceAllocationInfo
    );

    HRESULT CreateHeap(const D3D12_HEAP_DESC* desc, REFIID riid, void** outHeap);
    HRESULT OpenExistingHeapFromAddress1(const void* address, size_t size, REFIID riid, void** outHeap);
    HRESULT OpenExistingHeapFromFileMapping(HANDLE fileMapping, REFIID riid, void** outHeap);

    D3D12_HEAP_PROPERTIES GetCustomHeapProperties(uint32_t nodeMask, D3D12_HEAP_TYPE heapType);

    void GetCopyableFootprints1(
        const D3D12_RESOURCE_DESC1* resourceDesc,
        uint32_t firstSubresource,
        uint32_t numSubresources,
        uint64_t baseOffset,
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts,
        uint32_t* numRows,
        uint64_t* rowSizeInBytes,
        uint64_t* totalBytes
    );

    HRESULT CreateCommittedResource3(
        const D3D12_HEAP_PROPERTIES* heapProperties,
        D3D12_HEAP_FLAGS heapFlags,
        const D3D12_RESOURCE_DESC1* desc,
        D3D12_BARRIER_LAYOUT initialLayout,
        const D3D12_CLEAR_VALUE* optimizedClearValue,
        ID3D12ProtectedResourceSession* protectedSession,
        uint32_t numCastableFormats,
        const DXGI_FORMAT* castableFormats,
        REFIID riid,
        void** outResource
    );

    HRESULT CreatePlacedResource2(
        ID3D12Heap* heap,
        uint64_t heapOffset,
        const D3D12_RESOURCE_DESC1* desc,
        D3D12_BARRIER_LAYOUT initialLayout,
        const D3D12_CLEAR_VALUE* optimizedClearValue,
        uint32_t numCastableFormats,
        const DXGI_FORMAT* castableFormats,
        REFIID riid,
        void** outResource
    );

    HRESULT CreateReservedResource2(
        const D3D12_RESOURCE_DESC* desc,
        D3D12_BARRIER_LAYOUT initialLayout,
        const D3D12_CLEAR_VALUE* optimizedClearValue,
        ID3D12ProtectedResourceSession* protectedSession,
        uint32_t numCastableFormats,
        const DXGI_FORMAT* castableFormats,
        REFIID riid,
        void** outResource
    );

    void GetResourceTiling(
        DXLResource tiledResource,
        uint32_t* outNumTilesForEntireResource,
        D3D12_PACKED_MIP_INFO* outPackedMipDesc,
        D3D12_TILE_SHAPE* outStandardTileShapeForNonPackedMips,
        uint32_t* numSubresourceTilings,
        uint32_t firstSubresourceTilingToGet,
        D3D12_SUBRESOURCE_TILING* outSubresourceTilingsForNonPackedMips
    );

    HRESULT CreateSharedHandle(DXLDeviceChild object, const SECURITY_ATTRIBUTES* attributes, uint32_t access, const wchar_t* name, HANDLE* outHandle);
    HRESULT OpenSharedHandle(HANDLE ntHandle, REFIID riid, void** outObj);
    HRESULT OpenSharedHandleByName(const wchar_t* name, uint32_t access, HANDLE* outHandle);

    HRESULT MakeResident(uint32_t numObjects, ID3D12Pageable*const* objects);
    HRESULT EnqueueMakeResident(D3D12_RESIDENCY_FLAGS flags, uint32_t numObjects, ID3D12Pageable*const* objects, ID3D12Fence* fenceToSignal, UINT64 fenceValueToSignal);
    HRESULT Evict(uint32_t numObjects, ID3D12Pageable*const* objects);
    HRESULT SetResidencyPriority(uint32_t numObjects, ID3D12Pageable*const* objects, const D3D12_RESIDENCY_PRIORITY* priorities);

    HRESULT CreateFence(uint64_t initialValue, D3D12_FENCE_FLAGS flags, REFIID riid, void** outFence);
    HRESULT SetEventOnMultipleFenceCompletion(ID3D12Fence*const* fences, const uint64_t* fenceValues, uint32_t numFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS flags, HANDLE event);

    void GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* desc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO* outInfo);

    void RemoveDevice();
    HRESULT GetDeviceRemovedReason();

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES()
    HRESULT SetStablePowerState(bool enable);
    HRESULT SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE mode, D3D12_MEASUREMENTS_ACTION measurementsAction, HANDLE eventToSignalUponCompletion, BOOL* outFurtherMeasurementsDesired);
#endif
};

#if DXL_ENABLE_DEVELOPER_ONLY_FEATURES()

class DXLDebug : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDebug, ID3D12Debug6);

    void EnableDebugLayer();
    void DisableDebugLayer();

    void SetEnableGPUBasedValidation(bool enable);
    void SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS flags);

    void SetEnableSynchronizedCommandQueueValidation(bool enable);

    void SetEnableAutoName(bool enable);
};

class DXLDebugDevice : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDebugDevice, ID3D12DebugDevice2);

#if DXL_ENABLE_EXTENSIONS()
    static DXLDebugDevice FromDevice(DXLDevice device);
#endif

    HRESULT SetFeatureMask(D3D12_DEBUG_FEATURE mask);
    D3D12_DEBUG_FEATURE GetFeatureMask();
    HRESULT ReportLiveDeviceObjects(D3D12_RLDO_FLAGS flags);

    HRESULT SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, const void* data, uint32_t dataSize);
    HRESULT GetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_TYPE type, void* data, uint32_t dataSize);
};

class DXLDebugCommandQueue : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDebugCommandQueue, ID3D12DebugCommandQueue1);

#if DXL_ENABLE_EXTENSIONS()
    static DXLDebugCommandQueue FromCommandQueue(DXLCommandQueue commandQueue);
#endif

    void AssertResourceAccess(DXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access);
    void AssertTextureLayout(DXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout);
};

class DXLDebugCommandList : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDebugCommandList, ID3D12DebugCommandList3);

#if DXL_ENABLE_EXTENSIONS()
    static DXLDebugCommandList FromCommandList(DXLCommandList commandList);
#endif

    HRESULT SetFeatureMask(D3D12_DEBUG_FEATURE Mask);
    D3D12_DEBUG_FEATURE GetFeatureMask();

    HRESULT SetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, const void* data, uint32_t dataSize);
    HRESULT GetDebugParameter(D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE type, void* data, uint32_t dataSize);

    void AssertResourceAccess(DXLResource resource, uint32_t subresource, D3D12_BARRIER_ACCESS access);
    void AssertTextureLayout(DXLResource resource, uint32_t subresource, D3D12_BARRIER_LAYOUT layout);
};

class DXLDebugInfoQueue : public DXLBase
{

public:

    DXL_INTERFACE_BOILERPLATE(DXLDebugInfoQueue, ID3D12InfoQueue1);

#if DXL_ENABLE_EXTENSIONS()
    static DXLDebugInfoQueue FromDevice(DXLDevice device);
#endif

    HRESULT RegisterMessageCallback(D3D12MessageFunc callbackFunc, D3D12_MESSAGE_CALLBACK_FLAGS callbackFilterFlags, void* context, DWORD* outCallbackCookie);
    HRESULT UnregisterMessageCallback(DWORD callbackCookie);

    void SetMuteDebugOutput(bool mute);
    bool GetMuteDebugOutput();
};

#endif

#if DXL_ENABLE_EXTENSIONS()

struct CreateDeviceParams
{
    const char* AgilitySDKPath = "";
    bool EnableDebugLayer = false;
    bool EnableGPUBasedValidation = true;
    D3D12MessageFunc DebugLayerCallbackFunction = nullptr;
};

DXLDevice CreateDevice(CreateDeviceParams params);

void Release(IUnknown*& unknown);
void Release(DXLBase& base);

template<typename TInterface> IID GetIID(TInterface** ptrToInterface) { return __uuidof(**ptrToInterface); }
template<typename TDXLInterface> IID GetIID([[maybe_unused]] TDXLInterface* ptrToInterface) { return TDXLInterface::InterfaceID(); }

template<typename TInterface> void** GetPPVArg(TInterface** ptrToInterface) { return reinterpret_cast<void**>(ptrToInterface); }
template<typename TDXLInterface> void** GetPPVArg(TDXLInterface* ptrToInterface) { return reinterpret_cast<void**>(ptrToInterface->AddressOfNative()); }

#define DXL_PPV_ARGS(ptrToInterface)    GetIID(ptrToInterface), GetPPVArg(ptrToInterface)

#endif  // DXL_ENABLE_EXTENSIONS()

} // namespace dxl