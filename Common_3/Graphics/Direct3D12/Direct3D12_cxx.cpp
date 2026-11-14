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

// ---------------------------------------------------------------------------------------------------------------------
// Pipeline states created on worker threads.
//
// The driver compiles a pipeline's shaders into the GPU's code when its state is created, which takes from under a
// millisecond to seconds for a large kernel, and it can do several at once. addPipeline therefore hands the creation to
// a worker and returns the pipeline at once; the first thread that needs the state (binding or removing the pipeline,
// or a caller asking whether it failed) waits for that one pipeline, and runs the job itself when no worker has started
// it, so a pipeline needed now never queues behind others. Creation is free-threaded in Direct3D 12; each job owns
// everything it reads.
// ---------------------------------------------------------------------------------------------------------------------

#include "../Interfaces/IGraphics.h"
#include "../../Utilities/Interfaces/ILog.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct D3D12PipelineJob
{
    void*                   pData = nullptr;
    D3D12PipelineJobRun     pRun = nullptr;
    D3D12PipelineJobRelease pRelease = nullptr;
};

struct D3D12PipelineQueue
{
    std::mutex mLock;
    // Signalled when a job is queued (for the workers) and when one finishes (for the threads waiting on it).
    std::condition_variable                         mQueued;
    std::condition_variable                         mFinished;
    std::deque<Pipeline*>                           mOrder;
    std::unordered_map<Pipeline*, D3D12PipelineJob> mJobs;
    std::unordered_set<Pipeline*>                   mRunning;
    std::vector<std::thread>                        mWorkers;
    bool                                            mStopping = false;
    std::atomic<uint32_t>                           mInFlight{ 0 };
    std::atomic<uint32_t>                           mRequested{ 0 };
    std::atomic<uint32_t>                           mCompleted{ 0 };
    std::atomic<uint64_t>                           mWaitedUs{ 0 };
};

static D3D12PipelineQueue gPipelineQueue;

// Several compiles at once, leaving cores for the frame and the rest of the process.
static uint32_t d3d12PipelineWorkerCount()
{
    const uint32_t cores = std::thread::hardware_concurrency();
    const uint32_t workers = cores > 4u ? cores / 2u : 1u;
    return workers > 8u ? 8u : workers;
}

// Runs a job taken out of the queue (pPipeline is in mRunning) and publishes its result.
static void d3d12RunPipelineJob(Pipeline* pPipeline, const D3D12PipelineJob& job)
{
    const BOOL created = job.pRun(job.pData);
    job.pRelease(job.pData);
    std::lock_guard<std::mutex> guard(gPipelineQueue.mLock);
    pPipeline->mDx.mCreationFailed = created ? 0u : 1u;
    gPipelineQueue.mRunning.erase(pPipeline);
    gPipelineQueue.mCompleted.fetch_add(1u, std::memory_order_relaxed);
    gPipelineQueue.mInFlight.fetch_sub(1u, std::memory_order_release);
    gPipelineQueue.mFinished.notify_all();
}

// Takes the next queued job, or returns false once the queue is stopping and empty.
static bool d3d12TakePipelineJob(Pipeline** ppPipeline, D3D12PipelineJob* pJob)
{
    std::unique_lock<std::mutex> lock(gPipelineQueue.mLock);
    for (;;)
    {
        gPipelineQueue.mQueued.wait(lock, [] { return gPipelineQueue.mStopping || !gPipelineQueue.mOrder.empty(); });
        if (gPipelineQueue.mOrder.empty())
        {
            return false;
        }
        Pipeline* pPipeline = gPipelineQueue.mOrder.front();
        gPipelineQueue.mOrder.pop_front();
        const auto found = gPipelineQueue.mJobs.find(pPipeline);
        if (found == gPipelineQueue.mJobs.end())
        {
            // A thread that needed it ran it already.
            continue;
        }
        *pJob = found->second;
        gPipelineQueue.mJobs.erase(found);
        gPipelineQueue.mRunning.insert(pPipeline);
        *ppPipeline = pPipeline;
        return true;
    }
}

static void d3d12PipelineWorker()
{
    Pipeline*        pPipeline = nullptr;
    D3D12PipelineJob job;
    while (d3d12TakePipelineJob(&pPipeline, &job))
    {
        d3d12RunPipelineJob(pPipeline, job);
    }
}

extern "C" void d3d12CreatePipelineAsync(Pipeline* pPipeline, void* pJob, D3D12PipelineJobRun run,
                                         D3D12PipelineJobRelease release)
{
    std::lock_guard<std::mutex> guard(gPipelineQueue.mLock);
    if (gPipelineQueue.mWorkers.empty())
    {
        gPipelineQueue.mStopping = false;
        const uint32_t count = d3d12PipelineWorkerCount();
        for (uint32_t i = 0; i < count; ++i)
        {
            gPipelineQueue.mWorkers.emplace_back(d3d12PipelineWorker);
        }
        LOGF(eINFO, "D3D12: pipelines are created on up to %u worker threads", count);
    }
    gPipelineQueue.mJobs[pPipeline] = D3D12PipelineJob{ pJob, run, release };
    gPipelineQueue.mOrder.push_back(pPipeline);
    gPipelineQueue.mRequested.fetch_add(1u, std::memory_order_relaxed);
    gPipelineQueue.mInFlight.fetch_add(1u, std::memory_order_relaxed);
    gPipelineQueue.mQueued.notify_one();
}

// Takes pPipeline's job out of the queue for the calling thread to run (true), or waits while a worker runs it.
static bool d3d12ClaimOrAwaitPipeline(Pipeline* pPipeline, D3D12PipelineJob* pJob, bool* pWaited)
{
    std::unique_lock<std::mutex> lock(gPipelineQueue.mLock);
    const auto                   found = gPipelineQueue.mJobs.find(pPipeline);
    if (found != gPipelineQueue.mJobs.end())
    {
        *pJob = found->second;
        gPipelineQueue.mJobs.erase(found);
        gPipelineQueue.mRunning.insert(pPipeline);
        return true;
    }
    if (gPipelineQueue.mRunning.count(pPipeline) != 0u)
    {
        *pWaited = true;
        gPipelineQueue.mFinished.wait(lock, [pPipeline] { return gPipelineQueue.mRunning.count(pPipeline) == 0u; });
    }
    return false;
}

extern "C" void d3d12WaitForPipeline(Pipeline* pPipeline)
{
    if (gPipelineQueue.mInFlight.load(std::memory_order_acquire) == 0u)
    {
        return;
    }
    const auto       start = std::chrono::steady_clock::now();
    bool             waited = false;
    D3D12PipelineJob job;
    if (d3d12ClaimOrAwaitPipeline(pPipeline, &job, &waited))
    {
        waited = true;
        d3d12RunPipelineJob(pPipeline, job);
    }
    if (waited)
    {
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
        gPipelineQueue.mWaitedUs.fetch_add(static_cast<uint64_t>(us.count()), std::memory_order_relaxed);
    }
}

extern "C" void d3d12GetPipelineProgress(uint32_t* pCompleted, uint32_t* pRequested)
{
    if (pCompleted)
    {
        *pCompleted = gPipelineQueue.mCompleted.load(std::memory_order_relaxed);
    }
    if (pRequested)
    {
        *pRequested = gPipelineQueue.mRequested.load(std::memory_order_relaxed);
    }
}

extern "C" bool d3d12WaitForPipelines(uint32_t timeoutMs)
{
    if (gPipelineQueue.mInFlight.load(std::memory_order_acquire) == 0u)
    {
        return true;
    }
    std::unique_lock<std::mutex> lock(gPipelineQueue.mLock);
    const auto                   done = [] { return gPipelineQueue.mInFlight.load(std::memory_order_acquire) == 0u; };
    if (timeoutMs == UINT32_MAX)
    {
        gPipelineQueue.mFinished.wait(lock, done);
        return true;
    }
    return gPipelineQueue.mFinished.wait_for(lock, std::chrono::milliseconds(timeoutMs), done);
}

extern "C" bool d3d12PipelineFailed(Pipeline* pPipeline)
{
    if (!pPipeline)
    {
        return true;
    }
    d3d12WaitForPipeline(pPipeline);
    return pPipeline->mDx.mCreationFailed != 0u || pPipeline->mDx.pPipelineState == nullptr;
}

extern "C" bool d3d12PipelineReady(Pipeline* pPipeline)
{
    if (gPipelineQueue.mInFlight.load(std::memory_order_acquire) == 0u)
    {
        return true;
    }
    std::lock_guard<std::mutex> guard(gPipelineQueue.mLock);
    return gPipelineQueue.mJobs.count(pPipeline) == 0u && gPipelineQueue.mRunning.count(pPipeline) == 0u;
}

extern "C" double d3d12TakePipelineWaitMs(void)
{
    return static_cast<double>(gPipelineQueue.mWaitedUs.exchange(0u, std::memory_order_relaxed)) / 1000.0;
}

extern "C" void d3d12ExitPipelineCreation(void)
{
    d3d12WaitForPipelines(UINT32_MAX);
    std::vector<std::thread> workers;
    {
        std::lock_guard<std::mutex> guard(gPipelineQueue.mLock);
        gPipelineQueue.mStopping = true;
        workers.swap(gPipelineQueue.mWorkers);
        gPipelineQueue.mQueued.notify_all();
    }
    for (std::thread& worker : workers)
    {
        worker.join();
    }
}
#endif
