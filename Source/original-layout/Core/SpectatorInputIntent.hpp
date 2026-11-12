#pragma once
#include <atomic>
#include <cstdint>
namespace cfginput {
// Input only. Sender cannot supply a pawn, world, native callback or camera.
// Current gameplay authority decides whether this request can be consumed.
class SpectatorIntent {
    std::atomic<std::uint64_t> at_{0},sequence_{0};std::uint64_t seen_=0;
public:
    void Queue(std::uint64_t now)noexcept{at_.store(now,std::memory_order_relaxed);sequence_.fetch_add(1,std::memory_order_release);}
    void Clear()noexcept{seen_=sequence_.load(std::memory_order_acquire);}
    bool Take(std::uint64_t now,bool allowed)noexcept{
        const auto sequence=sequence_.load(std::memory_order_acquire);
        if(sequence==seen_)return false;
        const auto at=at_.load(std::memory_order_relaxed);
        if(sequence_.load(std::memory_order_acquire)!=sequence)return false;
        seen_=sequence;return allowed&&at<=now&&now-at<=15000;
    }
};
}
