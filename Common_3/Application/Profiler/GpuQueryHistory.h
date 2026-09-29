#pragma once
#include <cstdint>

// Query positions belong to a submitted query-pool slot. A later frame can
// add, remove or reorder scopes before that slot is resolved.
struct GpuQueryHistory
{
    static constexpr uint32_t FrameCount = 3;
    uint32_t Indices[FrameCount] = {};
    bool Pending[FrameCount] = {};
    void Record(uint32_t slot, uint32_t index) { Indices[slot] = index; Pending[slot] = true; }
    bool Consume(uint32_t slot, uint32_t& index)
    {
        if (!Pending[slot]) return false;
        index = Indices[slot];
        Pending[slot] = false;
        return true;
    }
};
inline bool GpuQueryElapsed(uint64_t begin, uint64_t end, uint64_t& ticks)
{
    ticks = 0;
    if (begin == 0 || end < begin) return false;
    ticks = end - begin; // equal timestamps are valid zero-work scopes
    return true;
}
inline uint64_t GpuExclusiveTicks(uint64_t inclusive, uint64_t children)
{
    return children < inclusive ? inclusive - children : 0;
}
