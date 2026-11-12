#pragma once
#include "../RuntimeProbe/CounselorPickerObservation.hpp"
#include <type_traits>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
namespace cfgmenu::counselorpicker {
enum class CastResult {Changed,Refused,Indeterminate};
enum class ConstructState {Idle,Forwarding,Returned,Refused,Indeterminate,RestoredAwaitValidation};
// Only the outermost borrowed Construct callback owns the temporary cast.
// Native wrapper->graph recursion forwards normally under the same lease.
// No replacement frame, manual bytecode stepping, possession/save edits,
// selected class fallback, mode switch or callback publication occurs here.
class ScopedConstruct {
    ConstructState state_=ConstructState::Idle;
public:
    ConstructState State()const noexcept{return state_;}
    template<class Host,class Forward> bool Run(Host& host,Forward&& forward){
        static_assert(std::is_nothrow_invocable_r_v<bool,Forward>,"Native forwarding must be noexcept and protect native failures");
        if(state_==ConstructState::Forwarding)return forward();
        if(state_!=ConstructState::Idle||!host.OwnedConstruct()||!host.Current(false))return forward();
        const bool widening=host.NeedsWidening();
        if(widening){const auto changed=host.Widen();
            if(changed!=CastResult::Changed){state_=changed==CastResult::Refused?ConstructState::Refused:ConstructState::Indeterminate;return forward();}}
        state_=ConstructState::Forwarding;const bool returned=forward();
        // Restore only our exactly retained operand under its original owner.
        // A foreign replacement or lost owner is not rollback permission.
        if(widening){
            if(!host.Current(true)||host.Restore()!=CastResult::Changed){
                state_=ConstructState::Indeterminate;return returned;}
            // Confirmed restoration is not an uncertain publication. No route
            // authority is granted: subsequent native input must independently
            // revalidate current source/receiver/executors. Never re-widen or
            // retry this completed scope, and preserve original native failure.
            if(!host.Current(false)){
                state_=returned?ConstructState::RestoredAwaitValidation:ConstructState::Refused;return returned;}}
        state_=returned?ConstructState::Returned:ConstructState::Refused;return returned;
    }
};
template<class Reader> class NativeCastHost {
    Reader& reader_;const Binding& binding_;std::uintptr_t receiver_=0,frame_=0;bool lease_=false;
    CastResult Exchange(std::uintptr_t expected,std::uintptr_t replacement)noexcept{
        const auto module=binding_.context.module;
        if(module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||GetCurrentThreadId()!=binding_.context.thread||
           (expected==binding_.nativeState?!lease_:!OwnedConstruct())||!Current(expected==binding_.nativeState))return CastResult::Refused;
        const auto at=binding_.graph.script.data+CastOperandOffset;std::uintptr_t current=0;
        if(!cfgprobe::lifetime::Extent(at,8)||!reader_.Read(at,current)||current!=expected)return CastResult::Refused;
        DWORD original=0;if(!VirtualProtect(reinterpret_cast<void*>(at),8,PAGE_READWRITE,&original))return CastResult::Refused;
        bool written=false,failed=false;
        __try {
            // This serialized pointer operand is unaligned. Do NOT use an
            // alignment-requiring Interlocked pointer operation on bytecode.
            std::memcpy(&current,reinterpret_cast<const void*>(at),8);
            if(current==expected){std::memcpy(reinterpret_cast<void*>(at),&replacement,8);written=true;}
        }__except(EXCEPTION_EXECUTE_HANDLER){failed=true;}
        DWORD ignored=0;const bool protectedAgain=VirtualProtect(reinterpret_cast<void*>(at),8,original,&ignored)!=FALSE;
        if(failed||!protectedAgain)return CastResult::Indeterminate;
        if(!written)return CastResult::Refused;
        if(!reader_.Read(at,current)||current!=replacement)return CastResult::Indeterminate;
        return CastResult::Changed;
    }
public:
    NativeCastHost(Reader& reader,const Binding& binding,std::uintptr_t receiver,std::uintptr_t frame):
        reader_(reader),binding_(binding),receiver_(receiver),frame_(frame){}
    bool OwnedConstruct(){Callback kind=Callback::Other;return Frame(reader_,binding_,receiver_,frame_,GetCurrentThreadId(),kind)&&kind==Callback::Construct;}
    bool Current(bool widened){return counselorpicker::Current(reader_,binding_,GetCurrentThreadId(),widened);}
    bool NeedsWidening(){return !cfgspawn::ObjectIs(reader_,binding_.state,binding_.lobbyClass);}
    CastResult Widen(){const auto result=Exchange(binding_.lobbyClass,binding_.nativeState);lease_=result==CastResult::Changed;return result;}
    // The original interpreter advances its borrowed FFrame.Code. Restore the
    // retained class-data lease, not an imagined unchanged input-frame cursor.
    CastResult Restore(){if(!lease_)return CastResult::Refused;const auto result=Exchange(binding_.nativeState,binding_.lobbyClass);if(result==CastResult::Changed)lease_=false;return result;}
};
}
