#include "../../dxlatest.h"
#include <windows.h>

class UnknownX
{

public:

    uint32_t Release()
    {
        return internal->Release();
    }

    operator IUnknown* () const
    {
        return internal;
    }

    operator bool() const
    {
        return internal != nullptr;
    }

    bool operator==(IUnknown* other) const { return internal == other; }
    bool operator==(UnknownX other) const { return internal == other.internal; }

    bool operator!=(IUnknown* other) const { return internal != other; }
    bool operator!=(UnknownX other) const { return internal != other.internal; }


    UnknownX* operator->() { return this; }
    const UnknownX* operator->() const { return this; }

    IUnknown* ToNative() const
    {
        return internal;
    }

protected:

    IUnknown* internal = nullptr;
};

class BaseX : public UnknownX
{
public:

    operator ID3D12Object* () const
    {
        return (ID3D12Object*)internal;
    }

    BaseX* operator->() { return this; }
    const BaseX* operator->() const { return this; }

    bool operator==(ID3D12Object* other) const { return internal == other; }
    bool operator==(BaseX other) const { return internal == other.internal; }

    bool operator!=(ID3D12Object* other) const { return internal != other; }
    bool operator!=(BaseX other) const { return internal != other.internal; }

    ID3D12Object* ToNative() const
    {
        return (ID3D12Object*)internal;
    }

    uint32_t GetCount() const { return 0; }

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

        const BaseX constX;
        num += constX.GetCount();
        num += constX->GetCount();

        if (constX != nullptr)
            num += 1;

        if (constX == nullptr)
            num += 1;

        if (constX != obj)
            num += 1;

        if (constX == obj)
            num += 1;

        if (constX == otherX)
            num += 2;

        if (constX != otherX)
            num += 2;
    }

    {
        dxl::DXLObject x;
        [[maybe_unused]] uint32_t num = x ? 2 : 1;
    }
}