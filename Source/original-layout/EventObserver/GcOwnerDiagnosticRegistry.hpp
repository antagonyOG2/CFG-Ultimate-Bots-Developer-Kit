#pragma once
#include <array>
#include <atomic>
#include <cstdint>
namespace cfggcdiagnostic {
// Addresses of process-resident C++ referencers, NOT roots/UObjects. This list
// does not register, retain, mutate slots, remove owners or authorize native work.
// Track BEFORE native registration, preserving evidence of ambiguous returns.
struct Registry {
    static constexpr unsigned Capacity=256;
    std::array<std::atomic<std::uintptr_t>,Capacity> owners{};
    std::atomic<unsigned> overflow{0};
    void Track(std::uintptr_t owner)noexcept {
        if(!owner)return;
        for(auto& entry:owners){auto current=entry.load(std::memory_order_acquire);
            if(current==owner)return;
            if(!current&&entry.compare_exchange_strong(current,owner,std::memory_order_acq_rel))return;
            if(current==owner)return;
        }
        overflow.fetch_add(1,std::memory_order_relaxed);
    }
    void Snapshot(std::array<std::uintptr_t,Capacity>& out,unsigned& count)const noexcept {
        count=0;for(const auto& entry:owners){const auto owner=entry.load(std::memory_order_acquire);if(owner)out[count++]=owner;}
    }
};
inline Registry registeredOwners;
}
