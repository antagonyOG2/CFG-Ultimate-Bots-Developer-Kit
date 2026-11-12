#pragma once
#include <array>
#include <cstdint>

namespace cfgport::callbacks {
// Observation only. Deliberately has no feature capabilities/native-call API.
constexpr std::size_t Capacity=64;
constexpr std::uint64_t MaximumElapsedMs=5000;
constexpr std::uint64_t MinimumSpacingMs=10;
enum class Stop : std::uint32_t { None, Budget, IdentityChanged, ClockChanged, Timeout };
struct Event {
    std::uint64_t function=0, parameters=0, elapsedMs=0;
    std::uint32_t threadId=0, sequence=0;
};
class Capture {
public:
    // Arm before publishing to callbacks. Pawn/thread stay immutable thereafter.
    // Mutable state and accessors belong to the owner thread (or quiescent tests).
    bool Arm(std::uint64_t pawn,std::uint32_t threadId) noexcept {
        if (started_ || !pawn || !threadId) return false;
        started_=armed_=true; pawn_=pawn; threadId_=threadId; return true;
    }
    bool Candidate(std::uint64_t object,std::uint32_t threadId,bool nested) const noexcept {
        return threadId==threadId_ && object==pawn_ && !nested && armed_;
    }
    bool Due(std::uint64_t now) const noexcept {
        return armed_ && (!count_ || now<lastAt_ || now-firstAt_>MaximumElapsedMs || now-lastAt_>=MinimumSpacingMs);
    }
    bool Record(std::uint64_t function,std::uint64_t parameters,std::uint32_t threadId,
        std::uint64_t now,bool identityMatches) noexcept {
        if (threadId!=threadId_ || !armed_) return false;
        if (!identityMatches) { Disarm(Stop::IdentityChanged); return false; }
        if (!count_) firstAt_=lastAt_=now;
        if (now<lastAt_) { Disarm(Stop::ClockChanged); return false; }
        if (now-firstAt_>MaximumElapsedMs) { Disarm(Stop::Timeout); return false; }
        if (count_ && now-lastAt_<MinimumSpacingMs) return false;
        lastAt_=now;
        events_[count_]={function,parameters,now-firstAt_,threadId,static_cast<std::uint32_t>(count_+1)};
        ++count_;
        if (count_==Capacity) Disarm(Stop::Budget);
        return true;
    }
    bool Armed() const noexcept { return armed_; }
    std::size_t Count() const noexcept { return count_; }
    Stop Reason() const noexcept { return stop_; }
    Event At(std::size_t index) const noexcept { return index<count_ ? events_[index] : Event{}; }
private:
    void Disarm(Stop reason) noexcept { armed_=false; stop_=reason; }
    std::array<Event,Capacity> events_{};
    std::uint64_t pawn_=0,firstAt_=0,lastAt_=0;
    std::uint32_t threadId_=0;
    std::size_t count_=0;
    bool started_=false,armed_=false;
    Stop stop_=Stop::None;
};
} // namespace cfgport::callbacks
