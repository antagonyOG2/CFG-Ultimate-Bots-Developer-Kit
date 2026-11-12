#pragma once
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/DispatchObservation.hpp"
#include "../RuntimeProbe/OwnedGenericDispatcher.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <intrin.h>
#endif

namespace cfgchallengeescape::dispatch {
inline constexpr unsigned CallLimit = 512, ReadLimit = 524288;
inline constexpr std::uintptr_t ImageBytes = cfgprobe::ExecutableImageBytes;
template<class Reader,class Platform,class Lease> class Engine;
namespace detail {
template<unsigned N> struct Priority : Priority<N-1> {};
template<> struct Priority<0> {};
template<class R> auto SourceRemaining(const R& r,Priority<3>) -> decltype(r.Remaining(),unsigned{}) { return r.Remaining(); }
template<class R> auto SourceRemaining(const R& r,Priority<2>) -> decltype(r.cap,r.count,unsigned{}) {
    return r.count < r.cap ? r.cap-r.count : 0;
}
template<class R> auto SourceRemaining(const R& r,Priority<1>) -> decltype(r.cap,r.reads,unsigned{}) {
    return r.reads < r.cap ? r.cap-r.reads : 0;
}
inline unsigned SourceRemaining(...) { return ReadLimit; }
template<class Lease,class R> auto Current(Lease& lease,R& r,int) -> decltype(lease(r),bool{}) { return lease(r); }
template<class Lease,class R> auto Current(Lease& lease,R&,long) -> decltype(lease(),bool{}) { return lease(); }
struct State {
    unsigned calls = 0, reservedEnd = 0;
    bool bound = false, exactPinned = false, faulted = false, guarding = false, native = false, reserved = false;
    std::uintptr_t module = 0;
    std::uint32_t thread = 0;
};
inline thread_local const State* activeState = nullptr;
struct Active {
    const State* previous;
    explicit Active(const State& state) : previous(activeState) { activeState=&state; }
    ~Active() { activeState=previous; }
    Active(const Active&) = delete;
    Active& operator=(const Active&) = delete;
};
struct Flag {
    bool& value;
    explicit Flag(bool& v) : value(v) { value = true; }
    ~Flag() { value = false; }
    Flag(const Flag&) = delete;
    Flag& operator=(const Flag&) = delete;
};
}
// One callback owns ONE reader. Every observation, guard, proof and native
// engine borrowing this object spends its same fixed counters. No reset API.
// Source is a byte reader; an existing stricter source cap is also respected.
template<class Source> class CallbackReader {
    Source& source_;
    unsigned reads_ = 0;
    detail::State state_{};
    template<class R,class P,class L> friend class Engine;
    detail::State& DispatchState() { return state_; }
public:
    explicit CallbackReader(Source& source) : source_(source) {}
    CallbackReader(const CallbackReader&) = delete;
    CallbackReader& operator=(const CallbackReader&) = delete;
    CallbackReader(CallbackReader&&) = delete;
    CallbackReader& operator=(CallbackReader&&) = delete;
    unsigned Reads() const noexcept { return reads_; }
    bool IsActiveCallback() const noexcept { return detail::activeState==&state_; }
    unsigned Remaining() const {
        return (std::min)(ReadLimit-reads_,detail::SourceRemaining(source_,detail::Priority<3>{}));
    }
    bool ReadBytes(std::uintptr_t at,void* output,std::size_t bytes) {
        if (!output || !cfgprobe::lifetime::Extent(at,bytes) || !Remaining()) return false;
        ++reads_;
        return source_.ReadBytes(at,output,bytes);
    }
    template<class T> bool Read(std::uintptr_t at,T& output) {
        T candidate{};
        if (!ReadBytes(at,&candidate,sizeof(candidate))) return false;
        output = candidate; return true;
    }
};

// The caller supplies its immutable exact-image pin and a read-only scope lease
// that freshly proves the full mission/generation/authority scope using THIS
// reader. A dispatcher guard alone is never gameplay call authority.
// Platform exposes Thread(), Module(), Invoke(address,object,function,packet),
// Factory(address,out); false from a native edge means a fault and poisons the
// entire callback. Portable tests use owned byte fixtures and a fake platform.
template<class Reader,class Platform,class Lease> class Engine {
    Reader& reader_;
    Platform& platform_;
    const std::uintptr_t module_;
    const std::uint32_t thread_;
    const bool exactPinned_;
    Lease lease_;
    detail::State& state_;
    bool bindingValid_ = true;
    bool Bindings() {
        std::uint32_t observed = 0; std::uint8_t flag = 0;
        return reader_.Read(module_+cfgprobe::dispatch::ThreadIdRva,observed) && observed==thread_ &&
            reader_.Read(module_+cfgprobe::dispatch::ThreadFlagRva,flag) && flag==1 && cfgdispatch::Proof(reader_,module_);
    }
    bool Guard() {
        if (!bindingValid_ || !exactPinned_ || !state_.exactPinned || state_.faulted || state_.guarding || state_.native ||
            (detail::activeState && detail::activeState!=&state_) ||
            state_.calls >= CallLimit || !cfgprobe::lifetime::Extent(module_,ImageBytes) ||
            !thread_ || platform_.Thread()!=thread_ || platform_.Module()!=module_) return false;
        detail::Flag guarding(state_.guarding);
        detail::Active active(state_);
        return Bindings() && detail::Current(lease_,reader_,0) && Bindings() &&
            platform_.Thread()==thread_ && platform_.Module()==module_;
    }
    bool CallAvailable() const {
        return state_.calls < CallLimit && (!state_.reserved || state_.calls < state_.reservedEnd);
    }
public:
    Engine(Reader& reader,Platform& platform,std::uintptr_t module,std::uint32_t thread,bool exactImagePinned,Lease lease) :
        reader_(reader),platform_(platform),module_(module),thread_(thread),exactPinned_(exactImagePinned),
        lease_(std::move(lease)),state_(reader.DispatchState()) {
        if (!state_.bound) {
            state_.bound=true; state_.module=module_; state_.thread=thread_; state_.exactPinned=exactPinned_;
        } else bindingValid_=state_.module==module_ && state_.thread==thread_ && state_.exactPinned==exactPinned_;
        // A fresh reader created inside another native/guard callback is never
        // a fresh callback budget. Only borrowing the existing reader is valid.
        if (detail::activeState && detail::activeState!=&state_) bindingValid_=false;
    }
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;
    std::uint32_t Thread() const noexcept { return platform_.Thread(); }
    unsigned Calls() const noexcept { return state_.calls; }
    unsigned RemainingCalls() const noexcept { return CallLimit-state_.calls; }
    bool Faulted() const noexcept { return state_.faulted; }
    bool Busy() const noexcept { return state_.native || state_.guarding; }
    bool Reserved() const noexcept { return state_.reserved; }
    // Admission never resets counters, enlarges caps, or replaces a previous
    // transaction. Reservation remains latched until the callback object dies,
    // even if actions fail or all reserved calls have been spent.
    bool Preflight(unsigned nativeCalls,unsigned readReserve) {
        if (!nativeCalls || !readReserve || state_.reserved || !CallAvailable() || !Guard() ||
            nativeCalls > CallLimit-state_.calls || readReserve > reader_.Remaining()) return false;
        state_.reservedEnd=state_.calls+nativeCalls; state_.reserved=true;
        return true;
    }
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* packet) {
        if (!cfgprobe::ObjectPointer(object) || !cfgprobe::ObjectPointer(function) || !packet || !CallAvailable() || !Guard()) return false;
        detail::Flag native(state_.native); ++state_.calls;
        detail::Active active(state_);
        const bool okay=platform_.Invoke(module_+cfgspawn::GenericDispatcherRva,object,function,packet);
        if (!okay) state_.faulted=true;
        return okay;
    }
    // Factory construction is only allowed before a recovery reservation.
    // Resource metadata must separately pin the exact factory and native owner.
    bool Factory(std::uintptr_t address,std::uintptr_t& output) {
        output=0;
        if (state_.reserved || address<module_ || address-module_>=ImageBytes || !CallAvailable() || !Guard()) return false;
        detail::Flag native(state_.native); ++state_.calls;
        detail::Active active(state_);
        std::uintptr_t candidate=0;
        const bool okay=platform_.Factory(address,candidate);
        if (!okay) { state_.faulted=true; return false; }
        if (!cfgprobe::ObjectPointer(candidate)) return false;
        output=candidate; return true;
    }
};

#if defined(_WIN32)
class WindowsSource {
public:
    bool ReadBytes(std::uintptr_t at,void* output,std::size_t bytes) noexcept {
        // This adapter exists ONLY inside the admitted local game callback.
        // Preserve all bounded identity/thread/GC/source proofs, but avoid a
        // kernel round trip for every tiny in-process field. Invalid memory is
        // a refused observation, not an unchecked pointer dereference.
        if(!output||!cfgprobe::lifetime::Extent(at,bytes))return false;
        __try {
            _ReadWriteBarrier();
            std::memcpy(output,reinterpret_cast<const void*>(at),bytes);
            _ReadWriteBarrier();return true;
        } __except(GetExceptionCode()==EXCEPTION_ACCESS_VIOLATION||GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR?
                   EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) {return false;}
    }
};
class WindowsPlatform {
    // Keep SEH inside primitive-only helpers, separate from template guards
    // and C++ scope owners (MSVC C2712). No hook installation occurs here.
    static bool InvokeNative(std::uintptr_t address,std::uintptr_t object,std::uintptr_t function,void* packet) noexcept {
        __try {
            reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(address)(
                reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),packet);
            return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
    static bool FactoryNative(std::uintptr_t address,std::uintptr_t* output) noexcept {
        __try { *output=reinterpret_cast<std::uintptr_t(__fastcall*)()>(address)(); return true; }
        __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
public:
    std::uint32_t Thread() const noexcept { return GetCurrentThreadId(); }
    std::uintptr_t Module() const noexcept { return reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)); }
    bool Invoke(std::uintptr_t address,std::uintptr_t object,std::uintptr_t function,void* packet) noexcept {
        return InvokeNative(address,object,function,packet);
    }
    bool Factory(std::uintptr_t address,std::uintptr_t& output) noexcept { return FactoryNative(address,&output); }
};
template<class Reader,class Lease> class NativeEngine {
    WindowsPlatform platform_;
    Engine<Reader,WindowsPlatform,Lease> engine_;
public:
    NativeEngine(Reader& reader,std::uintptr_t module,std::uint32_t thread,bool exactImagePinned,Lease lease) :
        engine_(reader,platform_,module,thread,exactImagePinned,std::move(lease)) {}
    NativeEngine(const NativeEngine&) = delete;
    NativeEngine& operator=(const NativeEngine&) = delete;
    NativeEngine(NativeEngine&&) = delete;
    NativeEngine& operator=(NativeEngine&&) = delete;
    std::uint32_t Thread() const noexcept { return engine_.Thread(); }
    unsigned Calls() const noexcept { return engine_.Calls(); }
    unsigned RemainingCalls() const noexcept { return engine_.RemainingCalls(); }
    bool Faulted() const noexcept { return engine_.Faulted(); }
    bool Busy() const noexcept { return engine_.Busy(); }
    bool Reserved() const noexcept { return engine_.Reserved(); }
    bool Preflight(unsigned calls,unsigned reads) { return engine_.Preflight(calls,reads); }
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* packet) { return engine_.Invoke(object,function,packet); }
    bool Factory(std::uintptr_t address,std::uintptr_t& output) { return engine_.Factory(address,output); }
};
#endif
}
