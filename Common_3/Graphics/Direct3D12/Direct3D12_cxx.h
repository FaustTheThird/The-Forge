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

#ifdef __cplusplus
extern "C"
{
#endif

    struct IDxcBlobUtf8;
    extern HRESULT IDxcBlobEncoding_QueryInterface(struct IDxcBlobEncoding* pEncoding, struct IDxcBlobUtf8** pOut);
    extern void    IDxcBlobEncoding_Release(struct IDxcBlobEncoding* pEncoding);
    extern LPVOID  IDxcBlobEncoding_GetBufferPointer(struct IDxcBlobEncoding* pEncoding);
    extern SIZE_T  IDxcBlobEncoding_GetBufferSize(struct IDxcBlobEncoding* pEncoding);
    extern LPCSTR  IDxcBlobUtf8_GetStringPointer(struct IDxcBlobUtf8* pBlob);
    extern SIZE_T  IDxcBlobUtf8_GetStringLength(struct IDxcBlobUtf8* pBlob);
    extern HRESULT IDxcUtils_CreateBlob(void* pByteCode, uint32_t byteCodeSize, struct IDxcBlobEncoding** ppEncoding);

    typedef struct D3D12MA_ALLOCATION_DESC
    {
        D3D12_RESOURCE_STATES    ResourceStates;
        D3D12_HEAP_TYPE          HeapType;
        D3D12_HEAP_FLAGS         ExtraHeapFlags;
        UINT                     CreationNodeMask;
        const D3D12_CLEAR_VALUE* pOptimizedClearValue;
        UINT                     VisibleNodeMask;
        bool                     mUseDedicatedAllocation;
    } D3D12MA_ALLOCATION_DESC;

#ifdef XBOX
    typedef IDXGIAdapter D3D12MA_IDXGIAdapter;
#else
typedef IDXGIAdapter4 D3D12MA_IDXGIAdapter;
#endif

    extern void    D3D12MA_BuildStatsString(struct D3D12MAAllocator_* pAllocator, BOOL detailedMap);
    extern void    D3D12MA_CalculateMemoryUse(struct D3D12MAAllocator_* pAllocator, uint64_t* usedBytes, uint64_t* totalAllocatedBytes);
    extern HRESULT D3D12MA_CreateAllocator(ID3D12Device* pDevice, D3D12MA_IDXGIAdapter* pGpu, struct D3D12MAAllocator_** ppOut);
    extern void    D3D12MA_ReleaseAllocator(struct D3D12MAAllocator_* pAllocator);
    extern HRESULT D3D12MA_CreateResource(struct D3D12MAAllocator_* pAllocator, const D3D12MA_ALLOCATION_DESC* pDesc,
                                          const D3D12_RESOURCE_DESC* pResDesc, struct D3D12MAAllocation_** ppAlloc,
                                          ID3D12Resource** ppResource);
    extern void    D3D12MA_ReleaseAllocation(struct D3D12MAAllocation_* pAlloc);

    extern void PIX_BeginEvent(ID3D12GraphicsCommandList1* context, float r, float g, float b, const char* pName);
    extern void PIX_EndEvent(ID3D12GraphicsCommandList1* context);
    extern void PIX_SetMarker(ID3D12GraphicsCommandList1* context, float r, float g, float b, const char* pName);

    // BloomEngine M6.6 B.6: mesh-shader PSO creation via
    // D3D12_PIPELINE_STATE_STREAM_DESC. Implemented in C++ to leverage
    // alignas(void*) struct layout guarantees that MSVC C-mode does
    // not honour (sizeof(struct) % alignof(struct) == 0 is a C++ rule,
    // not C). The C-side addMeshPipeline assembles inputs and calls
    // this; PSO ownership transfers to the caller via *ppOut.
    extern HRESULT Bloom_CreateMeshPipelineState(
        ID3D12Device*                   pDevice,
        ID3D12RootSignature*            pRootSignature,
        const D3D12_SHADER_BYTECODE*    pAS,        // optional, may be NULL
        const D3D12_SHADER_BYTECODE*    pMS,        // required
        const D3D12_SHADER_BYTECODE*    pPS,        // optional, may be NULL
        const D3D12_BLEND_DESC*         pBlend,
        const D3D12_RASTERIZER_DESC*    pRasterizer,
        const D3D12_DEPTH_STENCIL_DESC* pDepthStencil,
        DXGI_FORMAT                     dsvFormat,
        const DXGI_FORMAT*              pRtvFormats,
        UINT                            numRtvFormats,
        const DXGI_SAMPLE_DESC*         pSampleDesc,
        UINT                            sampleMask,
        UINT                            nodeMask,
        ID3D12PipelineState**           ppOut);

#ifdef __cplusplus
}
#endif
