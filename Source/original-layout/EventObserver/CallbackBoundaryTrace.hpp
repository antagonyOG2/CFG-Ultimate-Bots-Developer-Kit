#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
namespace cfgcallbacktrace {
constexpr unsigned Capacity=32;
struct Sample {std::uintptr_t tick=0;std::uint32_t gc=0;std::uint8_t serializing=0,valid=0;};
struct Event {
    std::uintptr_t object=0,function=0,returnPc=0;
    std::uint64_t elapsed=0;
    std::uint32_t thread=0,depth=0,returned=0,finished=0;
    Sample before{},after{};
};
struct Report {
    std::uint32_t bytes=sizeof(Report),version=2,published=0,capacity=Capacity;
    std::uintptr_t module=0,pawn=0;
    std::uint32_t thread=0;
    std::uint32_t entries=0,ownerEntries=0,pawnEntries=0,sampleAttempts=0;
    std::array<Event,Capacity> events{};
};
static_assert(std::is_trivially_copyable<Report>::value&&sizeof(Report)<8192);
class Capture {
    Report report_{};unsigned reserved_=0;std::uint64_t started_=0;
public:
    void Arm(std::uintptr_t module,std::uintptr_t pawn,std::uint32_t thread) noexcept {
        report_.module=module;report_.pawn=pawn;report_.thread=thread;
    }
    unsigned Begin(std::uintptr_t object,std::uintptr_t function,std::uintptr_t pc,
        std::uint32_t thread,unsigned depth,std::uint64_t now,const Sample& sample) noexcept {
        if(thread!=report_.thread||!report_.thread||reserved_==Capacity)return Capacity;
        // Observe owned actor callbacks without assuming the cached base-class
        // ReceiveTick is the function actually dispatched by a blueprint.
        // The sampled cache remains evidence for classification, not admission.
        if(!depth&&object!=report_.pawn)return Capacity;
        if(!reserved_)started_=now;
        if(now<started_||now-started_>5000)return Capacity;
        const auto index=reserved_++;
        auto& event=report_.events[index];event.object=object;event.function=function;
        event.returnPc=pc;event.thread=thread;event.depth=depth;event.elapsed=now-started_;event.before=sample;
        return index;
    }
    void Finish(unsigned index,bool returned,const Sample& sample) noexcept {
        if(index>=reserved_)return;
        auto& event=report_.events[index];event.after=sample;event.returned=returned?1u:0u;event.finished=1;
    }
    // Only publish complete trees after their outermost original has returned
    // or unwound. Published records are never changed again.
    unsigned CompletePrefix() const noexcept {
        unsigned n=report_.published;while(n<reserved_&&report_.events[n].finished)++n;return n;
    }
    Report& Data() noexcept{return report_;}
};
}
