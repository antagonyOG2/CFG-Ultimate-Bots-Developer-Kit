#pragma once
#include "NativeOfflineOwnerObservation.hpp"
#include "../RuntimeProbe/NativePopulationBindings.generated.hpp"
namespace cfgmenu::offlinepopulation {
// Only for a newly spawned bot inside the synchronous owner-thread birth.
// UE may not have assigned its PlayerState a weak-reference serial yet.
// The positive controller identity and native strong ownership edges anchor
// this observation; a zero state serial is never a retained identity/pin.
inline bool SameNewbornSlot(const cfgprobe::lifetime::SlotObservation& before,
    const cfgprobe::lifetime::SlotObservation& after){
    return cfgprobe::ObjectPointer(before.object)&&before.index>=0&&before.serial>=0&&after.serial>=0&&
        before.object==after.object&&before.index==after.index&&(!before.serial||before.serial==after.serial);
}
template<class Reader> bool ControllerOwnedState(Reader& r,const promotion::Context& owner,
    const cfgprobe::lifetime::SlotObservation& controller,cfgprobe::lifetime::SlotObservation& state){
    if(!controller.HasAssignedSerial()||state.object==controller.object||state.object==owner.playerState)return false;
    using cfgprobe::lifetime::ReadSlot;using cfgprobe::lifetime::SlotObservation;
    SlotObservation anchor{},first{},last{};std::uintptr_t controllerType=0,ctor=0,stateBase=0,stateType=0;
    std::uint32_t flags=0;
    if(!ReadSlot(r,owner.registry,controller.object,anchor)||!(anchor==controller)||
       !r.Read(owner.module+populationdisk::CounselorControllerReflection,controllerType)||
       !r.Read(owner.module+populationdisk::CounselorControllerClassCache,ctor)||!cfgprobe::MatchingClassCache(controllerType,ctor)||
       !offlineowner::Same(r,controller.object+cfgprobe::ObjectClass,controllerType)||
       !offlineowner::Same(r,controller.object+0x388,state.object)||!offlineowner::Same(r,state.object+0x88,controller.object)||
       !ReadSlot(r,owner.registry,state.object,first)||!SameNewbornSlot(state,first)||
       !r.Read(owner.module+cfgspawn::Classes[cfgspawn::PlayerState].reflection,stateBase)||
       !offlineowner::Same(r,owner.module+cfgspawn::Classes[cfgspawn::PlayerState].constructor,stateBase)||
       !r.Read(state.object+cfgprobe::ObjectClass,stateType)||!cfgspawn::ObjectIs(r,state.object,stateBase)||
       !r.Read(state.object+0x394,flags)||!(flags&4))return false;
    // Recheck sampled ownership/type and registry slots before publishing a
    // serial transition. Once positive, it can never change or return to zero.
    if(!offlineowner::Same(r,controller.object+0x388,state.object)||!offlineowner::Same(r,state.object+0x88,controller.object)||
       !offlineowner::Same(r,controller.object+cfgprobe::ObjectClass,controllerType)||
       !offlineowner::Same(r,state.object+cfgprobe::ObjectClass,stateType)||
       !r.Read(state.object+0x394,flags)||!(flags&4)||
       !ReadSlot(r,owner.registry,state.object,last)||!SameNewbornSlot(first,last)||
       !ReadSlot(r,owner.registry,controller.object,anchor)||!(anchor==controller))return false;
    state=last;return true;
}
}
