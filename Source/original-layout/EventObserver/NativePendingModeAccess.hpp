#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "PendingSandboxStart.hpp"
#include "../RuntimeProbe/PendingModeMemberObservation.hpp"
#include "OwnedPointerCell.hpp"
namespace cfgmenu::sandboxstart {
// Concrete current-process writer. No page-protection change, code patch,
// remote write or retry. The caller still owns the class-local Start frame
// and must run Route's shipping Sandbox/map/alias validation around this.
template<class Reader> class NativePendingModeAccess {
    Reader& reader_;pendingmember::Binding binding_{};bool ready_=false;
public:
    explicit NativePendingModeAccess(Reader& reader):reader_(reader){}
    bool Prepare(const active::Context& context,bool exactImageVerified){
        if(ready_||GetCurrentThreadId()!=context.thread)return false;
        ready_=pendingmember::Observe(reader_,context,GetCurrentThreadId(),exactImageVerified,binding_);return ready_;
    }
    bool PrepareWithAuthoredMode(const active::Context& context,bool exactImageVerified,const cfgreflection::Property& mode){
        if(ready_||GetCurrentThreadId()!=context.thread)return false;
        ready_=pendingmember::ObserveWithAuthoredMode(reader_,context,GetCurrentThreadId(),exactImageVerified,mode,binding_);return ready_;
    }
    WriteResult ChangePendingMode(const active::Context& context,std::uintptr_t previous,std::uintptr_t replacement)noexcept{
        if(!ready_||!(context==binding_.context)||!cfgprobe::ObjectPointer(previous)||!cfgprobe::ObjectPointer(replacement)||
            !pendingmember::Current(reader_,binding_,GetCurrentThreadId()))return WriteResult::Refused;
        const auto address=context.mode+selectiondisk::PendingOfflineMode;
        // Atomic comparison is the final old-value check. Replacement is never
        // adopted if another native route has already changed this pointer.
        const auto result=ExchangeOwnedPointer(address,previous,replacement);
        if(result!=WriteResult::Changed)return result;
        std::uintptr_t observed=0;
        if(!reader_.Read(address,observed)||observed!=replacement||
            !pendingmember::Current(reader_,binding_,GetCurrentThreadId()))return WriteResult::Indeterminate;
        return WriteResult::Changed;
    }
};
}
