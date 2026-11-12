#pragma once
#include "NativeCounselorConstruct.hpp"
#include "../RuntimeProbe/PickerPrecreationObservation.hpp"
namespace cfgmenu::counselorpicker {
struct PreparedConstructTrace {unsigned stage=0,currentChecks=0,ownedChecks=0,widenResult=3,restoreResult=3,error=0;
    pickerprepare::ConstructorDiagnostic state{};};
template<class Reader> bool PreparedConstructFrame(Reader& reader,const active::Context& actual,const pickerprepare::Prepared& prepared,
    const pickerprepare::Executors& executors,const pickerprepare::Receiver& receiver,std::uintptr_t borrowedFrame,std::uint32_t thread){
    if(!receiver.owningController.HasAssignedSerial()||!(receiver.owningController==actual.identities[1]))return false;
    pickerprepare::Receiver structural;std::uintptr_t table=0,getter=0;
    // FirstConstruct already validates the entire freshly retained source and
    // prepared metadata. Do not repeat that full walk to compare the latched
    // controller or its actual receiver's exact native getter virtual slot.
    return pickerprepare::FirstConstruct(reader,actual,prepared,executors,receiver.widget,borrowedFrame,thread,structural)&&
        structural.widget==receiver.widget&&structural.type==receiver.type&&structural.identity==receiver.identity&&
        reader.Read(receiver.widget,table)&&cfgprobe::ObjectPointer(table)&&reader.Read(table+widgetowner::VirtualSlot,getter)&&getter==actual.module+widgetowner::VirtualGetter;
}
// Native class-data exchange only for the positively owner-admitted receiver
// being forwarded through its original borrowed Construct. This adapter never
// creates a widget, getter call, profile/class request, frame or active context.
template<class Reader> class NativePreparedCastHost {
    Reader& reader_;const pickerprepare::Prepared& prepared_;const pickerprepare::Executors& executors_;
    const pickerprepare::Receiver& receiver_;const pickerprepare::ConstructorState& state_;
    std::uintptr_t frame_=0;bool widened_=false,ownedProof_=false,currentProof_=false;
    PreparedConstructTrace trace_{};
    bool Process()const noexcept{return prepared_.source.source.module==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))&&
        GetCurrentThreadId()==prepared_.source.source.thread;}
    CastResult Exchange(std::uintptr_t expected,std::uintptr_t replacement)noexcept{
        trace_.stage=5;
        // ScopedConstruct just proved these exact predicates. There is no
        // native call between that proof and this exchange. Consume the proof
        // instead of repeating multiple complete metadata/owner walks.
        if(!Process()||!currentProof_||(!widened_&&!ownedProof_)||
           (widened_?(expected!=prepared_.nativeState||replacement!=prepared_.lobbyClass):(expected!=prepared_.lobbyClass||replacement!=prepared_.nativeState)))return CastResult::Refused;
        currentProof_=false;
        const auto at=prepared_.graph.script.data+CastOperandOffset;std::uintptr_t value=0;
        trace_.stage=6;
        if(!cfgprobe::lifetime::Extent(at,8)||!reader_.Read(at,value)||value!=expected)return CastResult::Refused;
        trace_.stage=7;DWORD old=0;if(!VirtualProtect(reinterpret_cast<void*>(at),8,PAGE_READWRITE,&old)){trace_.error=GetLastError();return CastResult::Refused;}
        bool written=false,failed=false;
        __try {
            // Serialized graph pointer is unaligned. Compare and copy exactly
            // eight bytes; Interlocked pointer exchange is not applicable.
            std::memcpy(&value,reinterpret_cast<const void*>(at),8);
            if(value==expected){std::memcpy(reinterpret_cast<void*>(at),&replacement,8);written=true;}
        }__except(EXCEPTION_EXECUTE_HANDLER){failed=true;}
        DWORD ignored=0;const bool restoredPage=VirtualProtect(reinterpret_cast<void*>(at),8,old,&ignored)!=FALSE;
        trace_.stage=8;if(failed||!restoredPage){trace_.error=GetLastError();return CastResult::Indeterminate;}
        if(!written)return CastResult::Refused;
        trace_.stage=9;if(!reader_.Read(at,value)||value!=replacement)return CastResult::Indeterminate;
        trace_.stage=0;
        return CastResult::Changed;
    }
public:
    NativePreparedCastHost(Reader& reader,const pickerprepare::Prepared& prepared,const pickerprepare::Executors& executors,
        const pickerprepare::Receiver& receiver,const pickerprepare::ConstructorState& state,std::uintptr_t borrowedFrame):
        reader_(reader),prepared_(prepared),executors_(executors),receiver_(receiver),state_(state),frame_(borrowedFrame){}
    bool OwnedConstruct()noexcept{
        ++trace_.ownedChecks;ownedProof_=false;trace_.stage=1;
        try {if(!Process()||widened_)return false;active::Context actual;
            trace_.stage=2;if(!active::Observe(reader_,prepared_.source.source.module,actual))return false;
            trace_.stage=3;ownedProof_=PreparedConstructFrame(reader_,actual,prepared_,executors_,receiver_,frame_,GetCurrentThreadId());
            if(ownedProof_)trace_.stage=0;return ownedProof_;
        }catch(...){return false;}
    }
    bool Current(bool widened)noexcept{
        ++trace_.currentChecks;currentProof_=false;trace_.stage=1;
        try {if(!Process()||widened!=widened_)return false;active::Context actual;
            trace_.stage=2;if(!active::Observe(reader_,prepared_.source.source.module,actual))return false;
            trace_.stage=4;currentProof_=pickerprepare::ConstructorStateCurrent(reader_,actual,prepared_,executors_,receiver_,state_,GetCurrentThreadId(),widened,&trace_.state);
            if(currentProof_)trace_.stage=0;return currentProof_;
        }catch(...){return false;}
    }
    bool NeedsWidening()const noexcept{return state_.needsWidening;}
    CastResult Widen(){if(!state_.needsWidening||widened_)return CastResult::Refused;
        const auto result=Exchange(prepared_.lobbyClass,prepared_.nativeState);trace_.widenResult=static_cast<unsigned>(result);widened_=result==CastResult::Changed;return result;}
    CastResult Restore(){if(!widened_)return CastResult::Refused;
        // Original interpreter advances its borrowed frame. Its cursor is not
        // rollback authority; intact retained source/receiver/state/operand is.
        const auto result=Exchange(prepared_.nativeState,prepared_.lobbyClass);trace_.restoreResult=static_cast<unsigned>(result);if(result==CastResult::Changed)widened_=false;return result;}
    const PreparedConstructTrace& Trace()const noexcept{return trace_;}
};
}
