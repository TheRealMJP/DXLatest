#pragma once

#include <stdint.h>

#define COM_NO_WINDOWS_H
#include <Unknwn.h>

// Forward declarations
struct ID3D12Object;
struct ID3D12DeviceChild;
struct ID3D12RootSignature;
struct ID3D12Pageable;
struct ID3D12Heap1;
struct ID3D12Resource2;
struct ID3D12CommandAllocator;
struct ID3D12Fence1;

namespace dxl
{

using DXL_GPU_VIRTUAL_ADDRESS = uint64_t;

struct DXL_RANGE
{
    size_t Begin = 0;
    size_t End = 0;
};

struct DXL_BOX
{
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t front = 0;
    uint32_t right = 0;
    uint32_t bottom = 0;
    uint32_t back = 0;
};

enum DXL_HEAP_TYPE : uint32_t
{
    DXL_HEAP_TYPE_DEFAULT = 1,
    DXL_HEAP_TYPE_UPLOAD = 2,
    DXL_HEAP_TYPE_READBACK = 3,
    DXL_HEAP_TYPE_CUSTOM = 4,
    DXL_HEAP_TYPE_GPU_UPLOAD = 5
};

enum DXL_CPU_PAGE_PROPERTY : uint32_t
{
    DXL_CPU_PAGE_PROPERTY_UNKNOWN = 0,
    DXL_CPU_PAGE_PROPERTY_NOT_AVAILABLE = 1,
    DXL_CPU_PAGE_PROPERTY_WRITE_COMBINE = 2,
    DXL_CPU_PAGE_PROPERTY_WRITE_BACK = 3
};

enum DXL_MEMORY_POOL : uint32_t
{
    DXL_MEMORY_POOL_UNKNOWN = 0,
    DXL_MEMORY_POOL_L0 = 1,
    DXL_MEMORY_POOL_L1 = 2
};

struct DXL_HEAP_PROPERTIES
{
    DXL_HEAP_TYPE Type = DXL_HEAP_TYPE_DEFAULT;
    DXL_CPU_PAGE_PROPERTY CPUPageProperty = DXL_CPU_PAGE_PROPERTY_UNKNOWN;
    DXL_MEMORY_POOL MemoryPoolPreference = DXL_MEMORY_POOL_UNKNOWN;
    uint32_t CreationNodeMask = 0;
    uint32_t VisibleNodeMask = 0;
};

enum DXL_HEAP_FLAGS : uint32_t
{
    DXL_HEAP_FLAG_NONE    = 0,
    DXL_HEAP_FLAG_SHARED  = 0x1,
    DXL_HEAP_FLAG_DENY_BUFFERS    = 0x4,
    DXL_HEAP_FLAG_ALLOW_DISPLAY   = 0x8,
    DXL_HEAP_FLAG_SHARED_CROSS_ADAPTER    = 0x20,
    DXL_HEAP_FLAG_DENY_RT_DS_TEXTURES = 0x40,
    DXL_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES = 0x80,
    DXL_HEAP_FLAG_HARDWARE_PROTECTED  = 0x100,
    DXL_HEAP_FLAG_ALLOW_WRITE_WATCH   = 0x200,
    DXL_HEAP_FLAG_ALLOW_SHADER_ATOMICS    = 0x400,
    DXL_HEAP_FLAG_CREATE_NOT_RESIDENT = 0x800,
    DXL_HEAP_FLAG_CREATE_NOT_ZEROED   = 0x1000,
    DXL_HEAP_FLAG_TOOLS_USE_MANUAL_WRITE_TRACKING = 0x2000,
    DXL_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES  = 0,
    DXL_HEAP_FLAG_ALLOW_ONLY_BUFFERS  = 0xc0,
    DXL_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES   = 0x44,
    DXL_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES   = 0x84
};

struct DXL_HEAP_DESC
{
    uint64_t SizeInBytes = 0;
    DXL_HEAP_PROPERTIES Properties = { };
    uint64_t Alignment = 0;
    DXL_HEAP_FLAGS Flags = DXL_HEAP_FLAG_NONE;
};

enum DXL_FORMAT : uint32_t
{
    DXL_FORMAT_UNKNOWN                                 = 0,
    DXL_FORMAT_R32G32B32A32_TYPELESS                   = 1,
    DXL_FORMAT_R32G32B32A32_FLOAT                      = 2,
    DXL_FORMAT_R32G32B32A32_UINT                       = 3,
    DXL_FORMAT_R32G32B32A32_SINT                       = 4,
    DXL_FORMAT_R32G32B32_TYPELESS                      = 5,
    DXL_FORMAT_R32G32B32_FLOAT                         = 6,
    DXL_FORMAT_R32G32B32_UINT                          = 7,
    DXL_FORMAT_R32G32B32_SINT                          = 8,
    DXL_FORMAT_R16G16B16A16_TYPELESS                   = 9,
    DXL_FORMAT_R16G16B16A16_FLOAT                      = 10,
    DXL_FORMAT_R16G16B16A16_UNORM                      = 11,
    DXL_FORMAT_R16G16B16A16_UINT                       = 12,
    DXL_FORMAT_R16G16B16A16_SNORM                      = 13,
    DXL_FORMAT_R16G16B16A16_SINT                       = 14,
    DXL_FORMAT_R32G32_TYPELESS                         = 15,
    DXL_FORMAT_R32G32_FLOAT                            = 16,
    DXL_FORMAT_R32G32_UINT                             = 17,
    DXL_FORMAT_R32G32_SINT                             = 18,
    DXL_FORMAT_R32G8X24_TYPELESS                       = 19,
    DXL_FORMAT_D32_FLOAT_S8X24_UINT                    = 20,
    DXL_FORMAT_R32_FLOAT_X8X24_TYPELESS                = 21,
    DXL_FORMAT_X32_TYPELESS_G8X24_UINT                 = 22,
    DXL_FORMAT_R10G10B10A2_TYPELESS                    = 23,
    DXL_FORMAT_R10G10B10A2_UNORM                       = 24,
    DXL_FORMAT_R10G10B10A2_UINT                        = 25,
    DXL_FORMAT_R11G11B10_FLOAT                         = 26,
    DXL_FORMAT_R8G8B8A8_TYPELESS                       = 27,
    DXL_FORMAT_R8G8B8A8_UNORM                          = 28,
    DXL_FORMAT_R8G8B8A8_UNORM_SRGB                     = 29,
    DXL_FORMAT_R8G8B8A8_UINT                           = 30,
    DXL_FORMAT_R8G8B8A8_SNORM                          = 31,
    DXL_FORMAT_R8G8B8A8_SINT                           = 32,
    DXL_FORMAT_R16G16_TYPELESS                         = 33,
    DXL_FORMAT_R16G16_FLOAT                            = 34,
    DXL_FORMAT_R16G16_UNORM                            = 35,
    DXL_FORMAT_R16G16_UINT                             = 36,
    DXL_FORMAT_R16G16_SNORM                            = 37,
    DXL_FORMAT_R16G16_SINT                             = 38,
    DXL_FORMAT_R32_TYPELESS                            = 39,
    DXL_FORMAT_D32_FLOAT                               = 40,
    DXL_FORMAT_R32_FLOAT                               = 41,
    DXL_FORMAT_R32_UINT                                = 42,
    DXL_FORMAT_R32_SINT                                = 43,
    DXL_FORMAT_R24G8_TYPELESS                          = 44,
    DXL_FORMAT_D24_UNORM_S8_UINT                       = 45,
    DXL_FORMAT_R24_UNORM_X8_TYPELESS                   = 46,
    DXL_FORMAT_X24_TYPELESS_G8_UINT                    = 47,
    DXL_FORMAT_R8G8_TYPELESS                           = 48,
    DXL_FORMAT_R8G8_UNORM                              = 49,
    DXL_FORMAT_R8G8_UINT                               = 50,
    DXL_FORMAT_R8G8_SNORM                              = 51,
    DXL_FORMAT_R8G8_SINT                               = 52,
    DXL_FORMAT_R16_TYPELESS                            = 53,
    DXL_FORMAT_R16_FLOAT                               = 54,
    DXL_FORMAT_D16_UNORM                               = 55,
    DXL_FORMAT_R16_UNORM                               = 56,
    DXL_FORMAT_R16_UINT                                = 57,
    DXL_FORMAT_R16_SNORM                               = 58,
    DXL_FORMAT_R16_SINT                                = 59,
    DXL_FORMAT_R8_TYPELESS                             = 60,
    DXL_FORMAT_R8_UNORM                                = 61,
    DXL_FORMAT_R8_UINT                                 = 62,
    DXL_FORMAT_R8_SNORM                                = 63,
    DXL_FORMAT_R8_SINT                                 = 64,
    DXL_FORMAT_A8_UNORM                                = 65,
    DXL_FORMAT_R1_UNORM                                = 66,
    DXL_FORMAT_R9G9B9E5_SHAREDEXP                      = 67,
    DXL_FORMAT_R8G8_B8G8_UNORM                         = 68,
    DXL_FORMAT_G8R8_G8B8_UNORM                         = 69,
    DXL_FORMAT_BC1_TYPELESS                            = 70,
    DXL_FORMAT_BC1_UNORM                               = 71,
    DXL_FORMAT_BC1_UNORM_SRGB                          = 72,
    DXL_FORMAT_BC2_TYPELESS                            = 73,
    DXL_FORMAT_BC2_UNORM                               = 74,
    DXL_FORMAT_BC2_UNORM_SRGB                          = 75,
    DXL_FORMAT_BC3_TYPELESS                            = 76,
    DXL_FORMAT_BC3_UNORM                               = 77,
    DXL_FORMAT_BC3_UNORM_SRGB                          = 78,
    DXL_FORMAT_BC4_TYPELESS                            = 79,
    DXL_FORMAT_BC4_UNORM                               = 80,
    DXL_FORMAT_BC4_SNORM                               = 81,
    DXL_FORMAT_BC5_TYPELESS                            = 82,
    DXL_FORMAT_BC5_UNORM                               = 83,
    DXL_FORMAT_BC5_SNORM                               = 84,
    DXL_FORMAT_B5G6R5_UNORM                            = 85,
    DXL_FORMAT_B5G5R5A1_UNORM                          = 86,
    DXL_FORMAT_B8G8R8A8_UNORM                          = 87,
    DXL_FORMAT_B8G8R8X8_UNORM                          = 88,
    DXL_FORMAT_R10G10B10_XR_BIAS_A2_UNORM              = 89,
    DXL_FORMAT_B8G8R8A8_TYPELESS                       = 90,
    DXL_FORMAT_B8G8R8A8_UNORM_SRGB                     = 91,
    DXL_FORMAT_B8G8R8X8_TYPELESS                       = 92,
    DXL_FORMAT_B8G8R8X8_UNORM_SRGB                     = 93,
    DXL_FORMAT_BC6H_TYPELESS                           = 94,
    DXL_FORMAT_BC6H_UF16                               = 95,
    DXL_FORMAT_BC6H_SF16                               = 96,
    DXL_FORMAT_BC7_TYPELESS                            = 97,
    DXL_FORMAT_BC7_UNORM                               = 98,
    DXL_FORMAT_BC7_UNORM_SRGB                          = 99,
    DXL_FORMAT_AYUV                                    = 100,
    DXL_FORMAT_Y410                                    = 101,
    DXL_FORMAT_Y416                                    = 102,
    DXL_FORMAT_NV12                                    = 103,
    DXL_FORMAT_P010                                    = 104,
    DXL_FORMAT_P016                                    = 105,
    DXL_FORMAT_420_OPAQUE                              = 106,
    DXL_FORMAT_YUY2                                    = 107,
    DXL_FORMAT_Y210                                    = 108,
    DXL_FORMAT_Y216                                    = 109,
    DXL_FORMAT_NV11                                    = 110,
    DXL_FORMAT_AI44                                    = 111,
    DXL_FORMAT_IA44                                    = 112,
    DXL_FORMAT_P8                                      = 113,
    DXL_FORMAT_A8P8                                    = 114,
    DXL_FORMAT_B4G4R4A4_UNORM                          = 115,

    DXL_FORMAT_P208                                    = 130,
    DXL_FORMAT_V208                                    = 131,
    DXL_FORMAT_V408                                    = 132,

    DXL_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         = 189,
    DXL_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE = 190,

    DXL_FORMAT_A4B4G4R4_UNORM                          = 191,
};

enum DXL_RESOURCE_DIMENSION : uint32_t
{
    DXL_RESOURCE_DIMENSION_UNKNOWN = 0,
    DXL_RESOURCE_DIMENSION_BUFFER = 1,
    DXL_RESOURCE_DIMENSION_TEXTURE1D = 2,
    DXL_RESOURCE_DIMENSION_TEXTURE2D = 3,
    DXL_RESOURCE_DIMENSION_TEXTURE3D = 4
};

enum DXL_TEXTURE_LAYOUT : uint32_t
{
    DXL_TEXTURE_LAYOUT_UNKNOWN = 0,
    DXL_TEXTURE_LAYOUT_ROW_MAJOR = 1,
    DXL_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE = 2,
    DXL_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE = 3
};

enum DXL_RESOURCE_FLAGS : uint32_t
{
    DXL_RESOURCE_FLAG_NONE = 0,
    DXL_RESOURCE_FLAG_ALLOW_RENDER_TARGET = 0x1,
    DXL_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL = 0x2,
    DXL_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS = 0x4,
    DXL_RESOURCE_FLAG_DENY_SHADER_RESOURCE = 0x8,
    DXL_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER = 0x10,
    DXL_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS = 0x20,
    DXL_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY = 0x40,
    DXL_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY = 0x80,
    DXL_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE = 0x100,
    DXL_RESOURCE_FLAG_USE_TIGHT_ALIGNMENT = 0x400
};

struct DXL_SAMPLE_DESC
{
    uint32_t Count = 1;
    uint32_t Quality = 0;
};

struct DXL_MIP_REGION
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t Depth = 0;
};

struct DXL_RESOURCE_DESC
{
    DXL_RESOURCE_DIMENSION Dimension;
    uint64_t Alignment = 0;
    uint64_t Width = 0;
    uint32_t Height = 0;
    uint16_t DepthOrArraySize = 0;
    uint16_t MipLevels = 0;
    DXL_FORMAT Format = DXL_FORMAT_UNKNOWN;
    DXL_SAMPLE_DESC SampleDesc = { };
    DXL_TEXTURE_LAYOUT Layout = DXL_TEXTURE_LAYOUT_UNKNOWN;
    DXL_RESOURCE_FLAGS Flags = DXL_RESOURCE_FLAG_NONE;
    DXL_MIP_REGION SamplerFeedbackMipRegion = { };
};

enum DXL_FENCE_FLAGS : uint32_t
{
    DXL_FENCE_FLAG_NONE = 0,
    DXL_FENCE_FLAG_SHARED = 0x1,
    DXL_FENCE_FLAG_SHARED_CROSS_ADAPTER = 0x2,
    DXL_FENCE_FLAG_NON_MONITORED = 0x4
};

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

    ID3D12Object* ToID3D12Object() const;

    HRESULT GetPrivateData(REFGUID guid, uint32_t* outDataSize, void* outData) const;
    HRESULT SetPrivateData(REFGUID guid, uint32_t dataSize, const void *data);
    HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* data);
    HRESULT SetName(const wchar_t* name);
    HRESULT SetName(const char* name);
};

class IDXLDeviceChild : public IDXLObject
{

public:

    ID3D12DeviceChild* ToID3D12DeviceChild() const;

    HRESULT GetDevice(REFIID riid, void** outDevice);
};

class IDXLRootSignature : public IDXLDeviceChild
{

public:

    ID3D12RootSignature* ToID3D12RootSignature() const;
};

class IDXLPageable : public IDXLDeviceChild
{

public:

    ID3D12Pageable* ToID3D12Pageable() const;
};

class IDXLHeap : public IDXLPageable
{

public:

    ID3D12Heap1* ToID3D12Heap() const;

    DXL_HEAP_DESC GetDesc() const;
    HRESULT GetProtectedResourceSession(REFIID riid, void** outProtectedSession) const;
};

class IDXLResource : public IDXLPageable
{

public:

    ID3D12Resource2* ToID3D12Resource() const;

    HRESULT Map(uint32_t subresource, const DXL_RANGE* readRange, void** outData);
    void Unmap(uint32_t subresource, const DXL_RANGE* writtenRange);

    void* Map(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);
    void Unmap(uint32_t mipLevel, uint32_t arrayIndex = 0, uint32_t planeIndex = 0);

    DXL_RESOURCE_DESC GetDesc() const;

    DXL_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    HRESULT WriteToSubresource(uint32_t dstSubresource, const DXL_BOX* dstBox, const void* srcData, uint32_t srcRowPitch, uint32_t srcDepthPitch);
    HRESULT ReadFromSubresource(void* dstData, uint32_t dstRowPitch, uint32_t dstDepthPitch, uint32_t srcSubresource, const DXL_BOX* srcBox) const;

    HRESULT GetHeapProperties(DXL_HEAP_PROPERTIES* outHeapProperties, DXL_HEAP_FLAGS* outHeapFlags) const;

    HRESULT GetProtectedResourceSession(REFIID riid, void** protectedSession) const;
};

class IDXLCommandAllocator : public IDXLPageable
{
    ID3D12CommandAllocator* ToID3D12CommandAllocator() const;

    HRESULT Reset();
};

class IDXLFence : public IDXLPageable
{
    ID3D12Fence1* ToID3D12Fence() const;

    uint64_t GetCompletedValue() const;
    HRESULT SetEventOnCompletion(uint64_t value, HANDLE event);
    HRESULT Signal(uint64_t value);

    DXL_FENCE_FLAGS GetCreationFlags() const;
};

} // namespace dxl