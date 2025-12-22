#include "dxlatest.h"

#include "AgilitySDK/include/d3d12.h"
#include "AgilitySDK/include/d3dx12/d3dx12.h"

#define NOMINMAX
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "D3D12.lib")
#endif

namespace dxl
{

static_assert(sizeof(DXL_HEAP_PROPERTIES) == sizeof(D3D12_HEAP_PROPERTIES));
static_assert(sizeof(DXL_HEAP_DESC) == sizeof(D3D12_HEAP_DESC));
static_assert(sizeof(DXL_RANGE) == sizeof(D3D12_RANGE));
static_assert(sizeof(DXL_BOX) == sizeof(D3D12_BOX));
static_assert(sizeof(DXL_SAMPLE_DESC) == sizeof(DXGI_SAMPLE_DESC));
static_assert(sizeof(DXL_MIP_REGION) == sizeof(D3D12_MIP_REGION));
static_assert(sizeof(DXL_RESOURCE_DESC) == sizeof(D3D12_RESOURCE_DESC1));

template<typename ToStruct, typename FromStruct> ToStruct ConvertStruct(FromStruct fromStruct)
{
    static_assert(sizeof(ToStruct) == sizeof(FromStruct));
    ToStruct toStruct = { };;
    memcpy(&toStruct, &fromStruct, sizeof(ToStruct));
    return toStruct;
}

// == DXLBase ======================================================

HRESULT DXLBase::QueryInterface(REFIID riid, void** outObject)
{
    return underyling->QueryInterface(riid, outObject);
}

ULONG DXLBase::AddRef()
{
    return ++refCount;
}

ULONG DXLBase::Release()
{
    if (refCount == 1)
    {
        underyling->Release();
        return 0;
    }
    else
    {
        return --refCount;
    }
}

// == IDXLObject ======================================================

ID3D12Object* IDXLObject::ToID3D12Object() const
{
    return reinterpret_cast<ID3D12Object*>(underyling);
}

HRESULT IDXLObject::GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const
{
    return ToID3D12Object()->GetPrivateData(guid, outDataSize, outData);
}

HRESULT IDXLObject::SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data)
{
    return ToID3D12Object()->SetPrivateData(guid, dataSize, data);
}

HRESULT IDXLObject::SetPrivateDataInterface(REFGUID guid, const IUnknown* data)
{
    return ToID3D12Object()->SetPrivateDataInterface(guid, data);
}

HRESULT IDXLObject::SetName(const wchar_t* name)
{
    return ToID3D12Object()->SetName(name);
}

HRESULT IDXLObject::SetName(const char* name)
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

// == IDXLDeviceChild ======================================================

ID3D12DeviceChild* IDXLDeviceChild::ToID3D12DeviceChild() const
{
    return reinterpret_cast<ID3D12DeviceChild*>(underyling);
}

HRESULT IDXLDeviceChild::GetDevice(REFIID riid, void** outDevice)
{
    return ToID3D12DeviceChild()->GetDevice(riid, outDevice);
}

// == IDXLRootSignature ======================================================

ID3D12RootSignature* IDXLRootSignature::ToID3D12RootSignature() const
{
    return reinterpret_cast<ID3D12RootSignature*>(underyling);
}

// == IDXLPageable ======================================================

ID3D12Pageable* IDXLPageable::ToID3D12Pageable() const
{
    return reinterpret_cast<ID3D12Pageable*>(underyling);
}

// == IDXLPageable ======================================================

ID3D12Heap1* IDXLHeap::ToID3D12Heap() const
{
    return reinterpret_cast<ID3D12Heap1*>(underyling);
}

DXL_HEAP_DESC IDXLHeap::GetDesc() const
{
    return ConvertStruct<DXL_HEAP_DESC>(ToID3D12Heap()->GetDesc());
}

HRESULT IDXLHeap::GetProtectedResourceSession(REFIID riid, void** outProtectedSession) const
{
    return ToID3D12Heap()->GetProtectedResourceSession(riid, outProtectedSession);
}

// == IDXLResource ======================================================

ID3D12Resource2* IDXLResource::ToID3D12Resource() const
{
    return reinterpret_cast<ID3D12Resource2*>(underyling);
}

HRESULT IDXLResource::Map(uint32_t subresource, const DXL_RANGE* readRange, void** outData)
{
    if (readRange != nullptr)
    {
        D3D12_RANGE d3d12Range = ConvertStruct<D3D12_RANGE>(*readRange);
        return ToID3D12Resource()->Map(subresource, &d3d12Range, outData);
    }
    else
    {
        return ToID3D12Resource()->Map(subresource, nullptr, outData);
    }
}

void IDXLResource::Unmap(uint32_t subresource, const DXL_RANGE* writtenRange)
{
    if (writtenRange != nullptr)
    {
        D3D12_RANGE d3d12Range = ConvertStruct<D3D12_RANGE>(*writtenRange);
        ToID3D12Resource()->Unmap(subresource, &d3d12Range);
    }
    else
    {
        ToID3D12Resource()->Unmap(subresource, nullptr);
    }
}

void* IDXLResource::Map(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToID3D12Resource()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);

    void* data = nullptr;
    ToID3D12Resource()->Map(subresourceIndex, nullptr, &data);
    return data;
}

void IDXLResource::Unmap(uint32_t mipLevel, uint32_t arrayIndex, uint32_t planeIndex)
{
    const D3D12_RESOURCE_DESC1 desc = ToID3D12Resource()->GetDesc1();
    const uint32_t subresourceIndex = D3D12CalcSubresource(mipLevel, arrayIndex, planeIndex, desc.MipLevels, desc.DepthOrArraySize);
    ToID3D12Resource()->Unmap(subresourceIndex, nullptr);
}

DXL_RESOURCE_DESC IDXLResource::GetDesc() const
{
    return ConvertStruct<DXL_RESOURCE_DESC>(ToID3D12Resource()->GetDesc1());
}

DXL_GPU_VIRTUAL_ADDRESS IDXLResource::GetGPUVirtualAddress() const
{
    return ToID3D12Resource()->GetGPUVirtualAddress();
}

HRESULT IDXLResource::WriteToSubresource(uint32_t dstSubresource, const DXL_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
{
    const D3D12_BOX d3d12Box = ConvertStruct<D3D12_BOX>(*dstBox);
    return ToID3D12Resource()->WriteToSubresource(dstSubresource, &d3d12Box, srcData, srcRowPitch, srcDepthPitch);
}

HRESULT IDXLResource::ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const DXL_BOX* srcBox) const
{
    const D3D12_BOX d3d12Box = ConvertStruct<D3D12_BOX>(*srcBox);
    return ToID3D12Resource()->ReadFromSubresource(dstData, dstRowPitch, dstDepthPitch, srcSubresource, &d3d12Box);
}

HRESULT IDXLResource::GetHeapProperties(DXL_HEAP_PROPERTIES* outHeapProperties, DXL_HEAP_FLAGS* outHeapFlags) const
{
    D3D12_HEAP_PROPERTIES d3d12HeapProperties = { };
    D3D12_HEAP_FLAGS d3d12HeapFlags = D3D12_HEAP_FLAG_NONE;
    HRESULT hr = ToID3D12Resource()->GetHeapProperties(&d3d12HeapProperties, &d3d12HeapFlags);
    *outHeapProperties = ConvertStruct<DXL_HEAP_PROPERTIES>(d3d12HeapProperties);
    *outHeapFlags = DXL_HEAP_FLAGS(d3d12HeapFlags);
    return hr;
}

HRESULT IDXLResource::GetProtectedResourceSession(REFIID riid, void** protectedSession) const
{
    return ToID3D12Resource()->GetProtectedResourceSession(riid, protectedSession);
}

// == IDXLCommandAllocator ======================================================

ID3D12CommandAllocator* IDXLCommandAllocator::ToID3D12CommandAllocator() const
{
    return reinterpret_cast<ID3D12CommandAllocator*>(underyling);;
}

HRESULT IDXLCommandAllocator::Reset()
{
    return ToID3D12CommandAllocator()->Reset();
}

// == IDXLFence ======================================================

ID3D12Fence1* IDXLFence::ToID3D12Fence() const
{
    return reinterpret_cast<ID3D12Fence1*>(underyling);
}

uint64_t IDXLFence::GetCompletedValue() const
{
    return ToID3D12Fence()->GetCompletedValue();
}

HRESULT IDXLFence::SetEventOnCompletion(uint64_t value, HANDLE event)
{
    return ToID3D12Fence()->SetEventOnCompletion(value, event);
}

HRESULT IDXLFence::Signal(uint64_t value)
{
    return ToID3D12Fence()->Signal(value);
}

DXL_FENCE_FLAGS IDXLFence::GetCreationFlags() const
{
    return DXL_FENCE_FLAGS(ToID3D12Fence()->GetCreationFlags());
}

} // namespace dxl