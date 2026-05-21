/*
 * Copyright (c) 2017-2025 The Forge Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "../GraphicsConfig.h"

#ifdef DIRECT3D12
#include "Direct3D12_Cxx.h"

#define AMD_AGS_HELPER_IMPL
#include "../../../Common_3/Graphics/ThirdParty/OpenSource/ags/AgsHelper.h"

#include "../../../Common_3/Graphics/ThirdParty/OpenSource/DirectXShaderCompiler/inc/dxcapi.h"

#define D3D12MA_IMPLEMENTATION
#include "../../Utilities/ThirdParty/OpenSource/Nothings/stb_ds.h"
#include "../../Utilities/ThirdParty/OpenSource/bstrlib/bstrlib.h"
#include "../ThirdParty/OpenSource/D3D12MemoryAllocator/Direct3D12MemoryAllocator.h"

#if defined(FORGE_PROFILE)
#define PROFILE_BUILD // This turns on USE_PIX so that we can set markers even in release build
#endif
#if defined(XBOX)
#include <pix3.h>
#else
#include "../../../Common_3/Graphics/ThirdParty/OpenSource/winpixeventruntime/Include/WinPixEventRuntime/pix3.h"
#endif
#if defined(FORGE_PROFILE)
#undef PROFILE_BUILD
#endif

extern "C" void PIX_BeginEvent(ID3D12GraphicsCommandList1* context, float r, float g, float b, const char* pName)
{
    // note: USE_PIX isn't the ideal test because we might be doing a debug build where pix
    // is not installed, or a variety of other reasons. It should be a separate #ifdef flag?
#ifdef USE_PIX
    // color is in B8G8R8X8 format where X is padding
    PIXBeginEvent(context, PIX_COLOR((BYTE)(r * 255), (BYTE)(g * 255), (BYTE)(b * 255)), pName);
#else
    UNREF_PARAM(context);
    UNREF_PARAM(r);
    UNREF_PARAM(g);
    UNREF_PARAM(b);
    UNREF_PARAM(pName);
#endif
}

extern "C" void PIX_EndEvent(ID3D12GraphicsCommandList1* context)
{
#ifdef USE_PIX
    PIXEndEvent(context);
#else
    UNREF_PARAM(context);
#endif
}

extern "C" void PIX_SetMarker(ID3D12GraphicsCommandList1* context, float r, float g, float b, const char* pName)
{
#ifdef USE_PIX
    // color is in B8G8R8X8 format where X is padding
    PIXSetMarker(context, PIX_COLOR((BYTE)(r * 255), (BYTE)(g * 255), (BYTE)(b * 255)), pName);
#else
    UNREF_PARAM(context);
    UNREF_PARAM(r);
    UNREF_PARAM(g);
    UNREF_PARAM(b);
    UNREF_PARAM(pName);
#endif
}

extern "C" HRESULT IDxcBlobEncoding_QueryInterface(struct IDxcBlobEncoding* pEncoding, struct IDxcBlobUtf8** pOut)
{
    return pEncoding->QueryInterface(pOut);
}

extern "C" void IDxcBlobEncoding_Release(struct IDxcBlobEncoding* pEncoding) { pEncoding->Release(); }

extern "C" LPVOID IDxcBlobEncoding_GetBufferPointer(struct IDxcBlobEncoding* pEncoding) { return pEncoding->GetBufferPointer(); }

extern "C" SIZE_T IDxcBlobEncoding_GetBufferSize(struct IDxcBlobEncoding* pEncoding) { return pEncoding->GetBufferSize(); }

extern "C" LPCSTR IDxcBlobUtf8_GetStringPointer(struct IDxcBlobUtf8* pBlob) { return pBlob->GetStringPointer(); }

extern "C" SIZE_T IDxcBlobUtf8_GetStringLength(struct IDxcBlobUtf8* pBlob) { return pBlob->GetStringLength(); }

extern "C" HRESULT IDxcUtils_CreateBlob(void* pByteCode, uint32_t byteCodeSize, struct IDxcBlobEncoding** ppEncoding)
{
    IDxcUtils* pUtils;
    HRESULT    res = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    if (!SUCCEEDED(res))
    {
        return res;
    }
    pUtils->CreateBlob(pByteCode, byteCodeSize, DXC_CP_ACP, ppEncoding); //-V522
    pUtils->Release();
    return 0;
}

extern "C" HRESULT D3D12MA_CreateAllocator(ID3D12Device* pDevice, D3D12MA_IDXGIAdapter* pGpu, struct D3D12MAAllocator_** ppOut)
{
    D3D12MA::ALLOCATOR_DESC desc = {};
    desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
    desc.pDevice = pDevice;
    desc.pAdapter = pGpu;

    D3D12MA::ALLOCATION_CALLBACKS allocationCallbacks = {};
    allocationCallbacks.pAllocate = [](size_t size, size_t alignment, void*) { return tf_memalign(alignment, size); };
    allocationCallbacks.pFree = [](void* ptr, void*) { tf_free(ptr); };
    desc.pAllocationCallbacks = &allocationCallbacks;
    return D3D12MA::CreateAllocator(&desc, (D3D12MAAllocator**)ppOut);
}

extern "C" void D3D12MA_ReleaseAllocator(struct D3D12MAAllocator_* pAllocator)
{
    if (pAllocator)
    {
        ((D3D12MAAllocator*)pAllocator)->Release();
    }
}

extern "C" HRESULT D3D12MA_CreateResource(struct D3D12MAAllocator_* pAllocator, const D3D12MA_ALLOCATION_DESC* pDesc,
                                          const D3D12_RESOURCE_DESC* pResDesc, struct D3D12MAAllocation_** ppAlloc,
                                          ID3D12Resource** ppResource)
{
    D3D12MA::ALLOCATION_DESC alloc_desc = {};
    alloc_desc.HeapType = pDesc->HeapType;
    alloc_desc.ExtraHeapFlags = pDesc->ExtraHeapFlags;
    alloc_desc.CreationNodeMask = pDesc->CreationNodeMask;
    alloc_desc.VisibleNodeMask = pDesc->VisibleNodeMask;
    if (pDesc->mUseDedicatedAllocation)
    {
        alloc_desc.Flags |= D3D12MA::ALLOCATION_FLAG_COMMITTED;
    }
    return ((D3D12MAAllocator*)pAllocator)
        ->CreateResource(&alloc_desc, pResDesc, pDesc->ResourceStates, pDesc->pOptimizedClearValue, (D3D12MAAllocation**)ppAlloc,
                         IID_ID3D12Resource, (void**)ppResource);
}

extern "C" void D3D12MA_ReleaseAllocation(struct D3D12MAAllocation_* pAlloc)
{
    if (pAlloc)
    {
        ((D3D12MAAllocation*)pAlloc)->Release();
    }
}

extern "C" void D3D12MA_CalculateMemoryUse(struct D3D12MAAllocator_* pAllocator, uint64_t* usedBytes, uint64_t* totalAllocatedBytes)
{
    D3D12MA::TotalStatistics stats;
    ((D3D12MAAllocator*)pAllocator)->CalculateStatistics(&stats);
    *usedBytes = stats.Total.Stats.BlockBytes;
    *totalAllocatedBytes = stats.Total.Stats.AllocationBytes;
}

extern "C" void D3D12MA_BuildStatsString(struct D3D12MAAllocator_* pAllocator, BOOL detailedMap)
{
    ((D3D12MAAllocator*)pAllocator)->BuildStatsString(detailedMap);
}

// =====================================================================
// BloomEngine M6.6 B.6: mesh-shader PSO creation via stream-desc.
//
// Mirrors d3dx12_pipeline_state_stream.h's `CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT`
// pattern -- one templated wrapper that pairs a subobject-type tag with
// the payload, then `alignas(void*)` on the class lets C++'s sizeof%
// alignof==0 rule do the trailing-pad work the D3D12 runtime expects
// when walking the stream. The whole reason this lives here and not
// in Direct3D12.c is that MSVC C-mode does NOT enforce
// sizeof(struct) % alignof(struct) == 0 for typedef'd alignas-d
// structs, which corrupts the cursor walk inside CreatePipelineState
// (manifesting as "Duplicate Subobject Type detected: ROOT_SIGNATURE").
// =====================================================================

// File-scope helpers. Anonymous namespaces are banned by Bloom style
// (no internal-linkage namespacing), so names get a BloomPss_ prefix
// to avoid collisions inside unity-style builds.
template <typename InnerStructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE TypeTag>
class alignas(void*) BloomPss_Subobject
{
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_type;
    InnerStructType                     m_inner;
public:
    BloomPss_Subobject() noexcept : m_type(TypeTag), m_inner{} {}
    BloomPss_Subobject(const InnerStructType& v) noexcept : m_type(TypeTag), m_inner(v) {}
    InnerStructType& operator=(const InnerStructType& v) noexcept { m_inner = v; return m_inner; }
};

using BloomPss_RootSig    = BloomPss_Subobject<ID3D12RootSignature*,    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>;
using BloomPss_AS         = BloomPss_Subobject<D3D12_SHADER_BYTECODE,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>;
using BloomPss_MS         = BloomPss_Subobject<D3D12_SHADER_BYTECODE,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>;
using BloomPss_PS         = BloomPss_Subobject<D3D12_SHADER_BYTECODE,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>;
using BloomPss_Blend      = BloomPss_Subobject<D3D12_BLEND_DESC,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>;
using BloomPss_Rast       = BloomPss_Subobject<D3D12_RASTERIZER_DESC,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>;
using BloomPss_Depth      = BloomPss_Subobject<D3D12_DEPTH_STENCIL_DESC,D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>;
using BloomPss_DsvFormat  = BloomPss_Subobject<DXGI_FORMAT,             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>;
using BloomPss_RtFormats  = BloomPss_Subobject<D3D12_RT_FORMAT_ARRAY,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
using BloomPss_SampleDesc = BloomPss_Subobject<DXGI_SAMPLE_DESC,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>;
using BloomPss_SampleMask = BloomPss_Subobject<UINT,                    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>;
using BloomPss_NodeMask   = BloomPss_Subobject<UINT,                    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>;

// The stream itself. Subobject order doesn't matter to the runtime --
// only that each appears exactly once.
struct alignas(void*) BloomMeshPipelineStream
{
    BloomPss_RootSig    RootSig;
    BloomPss_AS         AS;
    BloomPss_MS         MS;
    BloomPss_PS         PS;
    BloomPss_Blend      Blend;
    BloomPss_Rast       Rasterizer;
    BloomPss_Depth      DepthStencil;
    BloomPss_DsvFormat  DsvFormat;
    BloomPss_RtFormats  RtFormats;
    BloomPss_SampleDesc SampleDesc;
    BloomPss_SampleMask SampleMask;
    BloomPss_NodeMask   NodeMask;
};

extern "C" HRESULT Bloom_CreateMeshPipelineState(
    ID3D12Device*                   pDevice,
    ID3D12RootSignature*            pRootSignature,
    const D3D12_SHADER_BYTECODE*    pAS,
    const D3D12_SHADER_BYTECODE*    pMS,
    const D3D12_SHADER_BYTECODE*    pPS,
    const D3D12_BLEND_DESC*         pBlend,
    const D3D12_RASTERIZER_DESC*    pRasterizer,
    const D3D12_DEPTH_STENCIL_DESC* pDepthStencil,
    DXGI_FORMAT                     dsvFormat,
    const DXGI_FORMAT*              pRtvFormats,
    UINT                            numRtvFormats,
    const DXGI_SAMPLE_DESC*         pSampleDesc,
    UINT                            sampleMask,
    UINT                            nodeMask,
    ID3D12PipelineState**           ppOut)
{
    if (!pDevice || !pRootSignature || !pMS || !ppOut) { return E_INVALIDARG; }

    BloomMeshPipelineStream s{};
    s.RootSig      = pRootSignature;
    if (pAS) { s.AS = *pAS; }
    s.MS           = *pMS;
    if (pPS) { s.PS = *pPS; }
    s.Blend        = *pBlend;
    s.Rasterizer   = *pRasterizer;
    s.DepthStencil = *pDepthStencil;
    s.DsvFormat    = dsvFormat;

    D3D12_RT_FORMAT_ARRAY rtArr{};
    rtArr.NumRenderTargets = numRtvFormats;
    for (UINT i = 0; i < numRtvFormats && i < 8; ++i) { rtArr.RTFormats[i] = pRtvFormats[i]; }
    s.RtFormats = rtArr;

    s.SampleDesc = *pSampleDesc;
    s.SampleMask = sampleMask;
    s.NodeMask   = nodeMask;

    // CreatePipelineState is on ID3D12Device2+. Cast follows the same
    // pattern Forge uses for ID3D12Device5 in hook_CreateStateObject --
    // the runtime device is always the highest-version interface.
    ID3D12Device2*                   pDevice2 = reinterpret_cast<ID3D12Device2*>(pDevice);
    D3D12_PIPELINE_STATE_STREAM_DESC desc{};
    desc.SizeInBytes                    = sizeof(s);
    desc.pPipelineStateSubobjectStream = &s;
    return pDevice2->CreatePipelineState(&desc, IID_PPV_ARGS(ppOut));
}
#endif
