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

DXL_CPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetCPUDescriptorHandleForHeapStart()
{
    return DXL_CPU_DESCRIPTOR_HANDLE(ToID3D12DescriptorHeap()->GetCPUDescriptorHandleForHeapStart());
}

DXL_GPU_DESCRIPTOR_HANDLE DXLDescriptorHeap::GetGPUDescriptorHandleForHeapStart()
{
    return DXL_GPU_DESCRIPTOR_HANDLE(ToID3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
}

} // namespace dxl