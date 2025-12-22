#pragma once

#include <stdint.h>

#define COM_NO_WINDOWS_H
#include <Unknwn.h>

// Forward declarations
struct ID3D12Object;

namespace dxl
{

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

    HRESULT GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData);
    HRESULT SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data);
    HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* data);
    HRESULT SetName(const wchar_t* name);
    // HRESULT SetName(const char* name);
    ID3D12Object* ToID3D12Object() const;
};

} // namespace dxl