#include "dxlatest.h"

#include "AgilitySDK/include/d3d12.h"

#ifdef _MSC_VER
#pragma comment(lib, "D3D12.lib")
#endif

namespace dxl
{

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

HRESULT IDXLObject::GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData)
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

/*HRESULT IDXLObject::SetName(const char* name)
{
    return ToID3D12Object()->SetName(name);
}*/

ID3D12Object* IDXLObject::ToID3D12Object() const
{
    return reinterpret_cast<ID3D12Object*>(underyling);
}

} // namespace dxl