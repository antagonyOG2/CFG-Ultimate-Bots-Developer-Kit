#pragma once
#include "DonorCounselorContextObservation.hpp"
#include "KillNativeResource.hpp"
#include "CounselorTerminalObservation.hpp"
namespace cfgdonorhunt {
struct CounselorObservation {
    cfgprobe::lifetime::SlotObservation pawn{},controller{},playerState{};
    float health=0;
    unsigned failure=0; // diagnostic only; failed snapshots never carry identity
    // H2.48 report-only: first failing sub-clause of failure==1 (see ObserveDonorCounselor) and the raw
    // context-kill / pawn bHasEscaped bytes last read (0xFF = not read). Never identity authority.
    unsigned clause=0;std::uint8_t rawContext=0xFF,rawEscaped=0xFF;
    // H2.48 Tommy slot, report-only: why clause 4 (SameSlot) refused. 1=expected unassigned 2=slot unreadable
    // 3=flags unreadable 4=PendingKill(0x20000000) 5=index changed 6=serial changed (0 = not refused here).
    std::uint8_t slotWhy=0;std::uint32_t slotFlags=0;std::int32_t slotFreshSerial=-1,slotExpectedSerial=-1;
    bool contextKill=false,escaped=false,possessed=false;
    bool Live()const {return health>0&&!contextKill&&!escaped&&possessed&&controller.HasAssignedSerial();}
};
// Current callback-local readonly value snapshot of FJB5838/5991. No native
// getters, class/name lookup, mutation or persistent UObject authority. Keep
// terminal/unpossessed observations distinct from failure to prove identity.
template<class R>bool ObserveDonorCounselor(R& r,
    const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,std::uintptr_t world,
    const cfgprobe::lifetime::SlotObservation& expected,std::uintptr_t counselorClass,CounselorObservation& out){
    out={};CounselorObservation value;value.pawn=expected;
    std::uintptr_t native=0,constructor=0,controller=0,state=0,possessed=0,levelType=0;
    cfgprobe::lifetime::SlotObservation level{};
    std::uint8_t pawnEscaped=0,stateEscaped=0;float healthAgain=0;
    out.failure=1; // pawn class/cache/slot/world/vitals/context/ownership fields
    // H2.48 clause: 1 module/world/slot, 2 class cache, 3 ObjectIs, 4 SameSlot, 5 ActorWorldCurrent, 6 health,
    // 7 context-kill (raw byte in rawContext; 0xFF = failed before the read), 8 bHasEscaped (raw in rawEscaped),
    // 9 PlayerState, 10 Controller. The wrapper only records: evaluation order and read counts are unchanged.
#if CFG_H248_TOMMY_SLOT
    // Same predicate as cfgjason::SameSlot(expected) (assigned serial, ObjectSlot incl. PendingKill veto, exact
    // equality) evaluated once, with the reason recorded. Never weakens identity.
    const auto sameSlot=[&]()->bool{
        cfgprobe::lifetime::SlotObservation fresh{};std::uint32_t flags=0;
        out.slotExpectedSerial=expected.serial;
        if(!expected.HasAssignedSerial()){out.slotWhy=1;return false;}
        if(!cfgprobe::lifetime::ReadSlot(r,objects,expected.object,fresh)){out.slotWhy=2;return false;}
        out.slotFreshSerial=fresh.serial;
        if(!r.Read(objects.data+static_cast<std::uintptr_t>(fresh.index)*24+8,flags)){out.slotWhy=3;return false;}
        out.slotFlags=flags;
        if(flags&0x20000000){out.slotWhy=4;return false;}
        if(fresh.index!=expected.index){out.slotWhy=5;return false;}
        if(fresh.serial!=expected.serial){out.slotWhy=6;return false;}
        return true;};
#else
    const auto sameSlot=[&]()->bool{return cfgjason::SameSlot(r,objects,expected);};
#endif
    const auto step=[&out](unsigned c,bool ok){
#if CFG_H248_SWEATER_TARGET
        if(!ok&&!out.clause)out.clause=c;
#else
        (void)c;
#endif
        return ok;};
    if(!step(1,module&&world&&expected.HasAssignedSerial())||
       !step(2,r.Read(module+cfgescape::disk::SCCounselorCharacterReflection,native)&&native==counselorClass&&
           r.Read(module+cfgescape::disk::SCCounselorCharacterConstructor,constructor)&&constructor==native)||
       !step(3,cfgspawn::ObjectIs(r,expected.object,counselorClass))||
       !step(4,sameSlot())||
       !step(5,cfgkill::ActorWorldCurrent(r,objects,module,expected.object,world,level,levelType))||
       !step(6,r.Read(expected.object+cfghunter::disk::Health,value.health)&&std::isfinite(value.health))||
       !step(7,ObserveCounselorContextKill(r,objects,module,expected,counselorClass,value.contextKill,&out.rawContext))||
       !step(8,r.Read(expected.object+cfgescape::disk::bHasEscaped,pawnEscaped)&&((out.rawEscaped=pawnEscaped),pawnEscaped<=1))||
       !step(9,r.Read(expected.object+cfgescape::disk::PlayerState,state))||
       !step(10,r.Read(expected.object+cfgescape::disk::Controller,controller)))return false;
    if(state){
        out.failure=2; // PlayerState cache/type/serial/escaped
        if(!r.Read(module+cfgescape::disk::SCPlayerStateReflection,native)||
           !r.Read(module+cfgescape::disk::SCPlayerStateConstructor,constructor)||!cfgprobe::MatchingClassCache(native,constructor)||
           // UE assigns weak serials lazily. PlayerState is used only inside
           // this readonly snapshot, never as retained native-call identity.
           // Keep exact slot/index/object/flags, type and final field resamples.
           !cfgspawn::ObjectSlot(r,objects,state,value.playerState)||
           !cfgspawn::ObjectIs(r,state,native)||!r.Read(state+cfgescape::disk::bEscaped,stateEscaped)||stateEscaped>1||
           !cfgescape::Same(r,state+cfgescape::disk::bEscaped,stateEscaped))return false;
        // SameSlot is a persistent-identity helper and demands a positive
        // weak serial. This is only a same-snapshot reread: compare the complete
        // observed slot (including serial0), index, object and flags directly.
        cfgprobe::lifetime::SlotObservation stateAgain{};
        if(!cfgspawn::ObjectSlot(r,objects,state,stateAgain)||!(stateAgain==value.playerState))return false;
    }
    if(controller){
        out.failure=3; // controller cache/type/slot
        if(!r.Read(module+cfgescape::disk::ControllerReflection,native)||
           !r.Read(module+cfgescape::disk::ControllerConstructor,constructor)||!cfgprobe::MatchingClassCache(native,constructor)||
           !cfgspawn::ObjectSlot(r,objects,controller,value.controller)||!value.controller.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,controller,native))return false;
        out.failure=4; // controller's owning Level/World
        if(!cfgkill::ActorWorldCurrent(r,objects,module,controller,world,level,levelType))return false;
        out.failure=5; // stable reciprocal possession read
        if(
           !r.Read(controller+cfgescape::disk::Pawn,possessed)||
           !cfgescape::Same(r,controller+cfgescape::disk::Pawn,possessed)||!cfgjason::SameSlot(r,objects,value.controller))return false;
        value.possessed=possessed==expected.object;
    }
    value.escaped=pawnEscaped||stateEscaped;
    out.failure=6; // final same-slot/field resample
    if(!cfgescape::Same(r,expected.object+cfgescape::disk::Controller,controller)||
       !cfgescape::Same(r,expected.object+cfgescape::disk::PlayerState,state)||
       !cfgescape::Same(r,expected.object+cfgescape::disk::bHasEscaped,pawnEscaped)||
       !r.Read(expected.object+cfghunter::disk::Health,healthAgain)||healthAgain!=value.health||
       !cfgjason::SameSlot(r,objects,expected))return false;
    value.rawContext=out.rawContext;value.rawEscaped=out.rawEscaped;
    out=value;return true;
}
}
