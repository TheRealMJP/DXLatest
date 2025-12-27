#include "../../dxlatest.h"
#include <windows.h>

class UnknownX
{

public:

    uint32_t Release()
    {
        return internal->Release();
    }

    operator IUnknown* ()
    {
        return internal;
    }

    operator bool()
    {
        return internal != nullptr;
    }

    bool operator==(IUnknown* other) { return internal == other; }
    bool operator==(UnknownX other) { return internal == other.internal; }

    bool operator!=(IUnknown* other) { return internal != other; }
    bool operator!=(UnknownX other) { return internal != other.internal; }


    UnknownX* operator->() { return this; }

    IUnknown* ToNative()
    {
        return internal;
    }

protected:

    IUnknown* internal = nullptr;
};

class BaseX : public UnknownX
{
public:

    operator ID3D12Object* ()
    {
        return (ID3D12Object*)internal;
    }

    BaseX* operator->() { return this; }

    bool operator==(ID3D12Object* other) { return internal == other; }
    bool operator==(BaseX other) { return internal == other.internal; }

    bool operator!=(ID3D12Object* other) { return internal != other; }
    bool operator!=(BaseX other) { return internal != other.internal; }

    ID3D12Object* ToNative()
    {
        return (ID3D12Object*)internal;
    }

private:

    // using UnknownX::operator IUnknown*;
};

int32_t APIENTRY WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] char* lpCmdLine, [[maybe_unused]] int32_t nCmdShow)
{
    {
        BaseX x;
        if (x)
            x.Release();
        x->Release();
        [[maybe_unused]] uint32_t num = x ? 2 : 1;
        [[maybe_unused]] ID3D12Object* obj = x;
        obj = x.ToNative();

        if (x != nullptr)
            num += 1;

        if (x == nullptr)
            num += 1;

        if (x != obj)
            num += 1;

        if (x == obj)
            num += 1;

        BaseX otherX;
        if (x == otherX)
            num += 2;

        if (x != otherX)
            num += 2;
    }

    {
        dxl::DXLObject x;
        [[maybe_unused]] uint32_t num = x ? 2 : 1;
    }
}