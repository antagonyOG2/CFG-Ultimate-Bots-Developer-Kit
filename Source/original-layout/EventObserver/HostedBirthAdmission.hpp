#pragma once
#include "HostedRegisteredRoster.hpp"
#include "../RuntimeProbe/HostedBirthBindings.hpp"
#include "../RuntimeProbe/HostedAuthority35.generated.hpp"
#include <type_traits>
namespace cfghosted::native {
struct PlayerList {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const PlayerList& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
struct Population {
    Context context{};PlayerList list{};std::array<std::uintptr_t,8> states{};
    Slot killer{},killerState{};std::int32_t bots=0;
};
template<class Bindings,class R>bool CurrentListen(R& r,const Context& c){
    if constexpr(std::is_same_v<Bindings,HistoricalBindings>)return true; // historical owned fixtures remain disabled at runtime
    else return Same(r,c.Driver()+disk35::ServerConnection,std::uintptr_t{})&&Same(r,c.Driver()+disk35::DriverWorld,c.World());
}
// A roster UPROPERTY/CurrentKillerState may have no assigned weak serial.
// This observes membership only; callers still retain GameState and exact
// native inputs in their independent callback lease BEFORE allocating.
template<class R>bool ObservedState(R& r,const Table& table,std::uintptr_t object,std::uintptr_t type,Slot& out){
    cfgprobe::Ancestry ancestry;Slot again;std::uint32_t flags=0,slotFlags=0;
    return cfgprobe::CaptureAncestry(r,object,ancestry)&&ancestry.Contains(type)&&
        cfgprobe::lifetime::ReadSlot(r,table,object,out)&&r.Read(object+8,flags)&&!(flags&0x30u)&&
        r.Read(table.data+24ULL*static_cast<unsigned>(out.index)+8,slotFlags)&&!(slotFlags&0x30000000u)&&
        cfgprobe::StableAncestry(r,object,ancestry)&&cfgprobe::lifetime::ReadSlot(r,table,object,again)&&out==again;
}
// Controller-first birth preflight. Native roster, NumBots and primary killer
// are sampled without altering any human actor/CDO. This is not spawn approval.
template<class R,class Bindings=HistoricalBindings,bool ObserveUnassignedStates=false> bool PopulationBefore(R& source,const Context& owner,std::uint32_t thread,Population& output){
    output={};Budget<R,std::is_same_v<Bindings,HistoricalBindings>?ReadLimit:8192> r(source);Population p;
    if(!Capture<decltype(r),Bindings>(r,owner.admission,thread,p.context)||!SameSession(owner,p.context)||!CurrentListen<Bindings>(r,p.context)||
       !r.Read(owner.Mode()+cfgmenu::populationdisk::NumBots,p.bots)||p.bots<0||p.bots>=7||
       !r.Read(owner.State()+birthdisk::PlayerArray,p.list)||p.list.count<1||p.list.count>=8||
       p.list.capacity<p.list.count||p.list.capacity>32||
       !cfgprobe::lifetime::Extent(p.list.data,8ULL*static_cast<unsigned>(p.list.count)))return false;
    std::uintptr_t stateClass=0,killerClass=0,killer=0,killerState=0;
    if(!Class(r,owner.admission.module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateClass)||
       !Class(r,owner.admission.module,cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,
        cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,killerClass)||
       !r.Read(owner.State()+birthdisk::CurrentKiller,killer)||!r.Read(owner.State()+birthdisk::CurrentKillerState,killerState)||
       !Typed(r,p.context.table,killer,killerClass,p.killer)||
       !(ObserveUnassignedStates?ObservedState(r,p.context.table,killerState,stateClass,p.killerState):Typed(r,p.context.table,killerState,stateClass,p.killerState))||
       !WorldOwned(r,killer,owner.World())||!WorldOwned(r,killerState,owner.World()))return false;
    unsigned killerMatches=0;
    for(int i=0;i<p.list.count;++i){auto& state=p.states[static_cast<unsigned>(i)];Slot slot;
        if(!r.Read(p.list.data+8ULL*static_cast<unsigned>(i),state)||
           !(ObserveUnassignedStates?ObservedState(r,p.context.table,state,stateClass,slot):Typed(r,p.context.table,state,stateClass,slot))||
           !WorldOwned(r,state,owner.World()))return false;
        for(int j=0;j<i;++j)if(p.states[static_cast<unsigned>(j)]==state)return false;
        if(state==killerState)++killerMatches;
    }
    if(killerMatches!=1||!Same(r,owner.State()+birthdisk::PlayerArray,p.list)||
       !Same(r,owner.Mode()+cfgmenu::populationdisk::NumBots,p.bots)||
       !Same(r,owner.State()+birthdisk::CurrentKiller,killer)||!Same(r,owner.State()+birthdisk::CurrentKillerState,killerState))return false;
    for(int i=0;i<p.list.count;++i)if(!Same(r,p.list.data+8ULL*static_cast<unsigned>(i),p.states[static_cast<unsigned>(i)]))return false;
    Context after;if(!Capture<decltype(r),Bindings>(r,owner.admission,thread,after)||!SameSession(p.context,after)||!CurrentListen<Bindings>(r,after))return false;
    output=p;return true;
}
struct NewControllerState {Slot controller{},state{};bool stateSerialAssigned=false;std::int32_t botDelta=0;};
// Used AFTER a separately guarded one-shot controller spawn. Requires no pawn:
// prevents applying ActiveCharacterClass/RestartPlayer to an existing human or
// already possessed AI. A zero serial is accepted ONLY for controller-owned PS,
// labelled unassigned, never admitted as a durable registered roster identity.
template<class R,class Bindings=HistoricalBindings,bool ObserveUnassignedStates=false> bool ControllerBorn(R& source,const Population& before,std::uint32_t thread,
    std::uintptr_t created,NewControllerState& output){
    output={};Budget<R,std::is_same_v<Bindings,HistoricalBindings>?ReadLimit:8192> r(source);Context current;NewControllerState found;
    if(before.list.count<1||before.list.count>=8||before.bots<0||before.bots>=7||
       !before.killer.HasAssignedSerial()||(!ObserveUnassignedStates&&!before.killerState.HasAssignedSerial())||
       !Capture<decltype(r),Bindings>(r,before.context.admission,thread,current)||!SameSession(before.context,current)||!CurrentListen<Bindings>(r,current))return false;
    const auto module=current.admission.module;std::uintptr_t ai=0,stateClass=0,state=0;
    if(!Class(r,module,cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,ai)||
       !Typed(r,current.table,created,ai,found.controller)||!WorldOwned(r,created,current.World())||
       !Same(r,created+cfgprobe::ControllerPawn,std::uintptr_t{})||
       !r.Read(created+cfgescape::disk::PlayerState,state)||state==created||
       !Class(r,module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateClass))return false;
    for(int i=0;i<before.list.count;++i)if(before.states[static_cast<unsigned>(i)]==state)return false;
    cfgprobe::Ancestry ancestry;std::uint32_t flags=0;Slot fresh;
    if(!cfgprobe::CaptureAncestry(r,state,ancestry)||!ancestry.Contains(stateClass)||
       !cfgprobe::lifetime::ReadSlot(r,current.table,state,found.state)||!WorldOwned(r,state,current.World())||
       !Same(r,state+0x88,created)||!Same(r,created+disk::ActorRole,disk::AuthorityRole)||
       !Same(r,state+disk::ActorRole,disk::AuthorityRole)||!r.Read(state+birthdisk::BotFlags,flags)||!(flags&birthdisk::BotMask))return false;
    PlayerList list;std::array<std::uintptr_t,8> states{};std::int32_t bots=0;
    if(before.list.count<1||before.list.count>=8||before.bots<0||before.bots>=7||
       !r.Read(current.State()+birthdisk::PlayerArray,list)||list.count!=before.list.count+1||list.capacity<list.count||list.capacity>32||
       !cfgprobe::lifetime::Extent(list.data,8ULL*static_cast<unsigned>(list.count))||
       !r.Read(current.Mode()+cfgmenu::populationdisk::NumBots,bots)||bots<before.bots||bots>before.bots+1)return false;
    unsigned own=0;
    for(int i=0;i<list.count;++i){auto& value=states[static_cast<unsigned>(i)];if(!r.Read(list.data+8ULL*static_cast<unsigned>(i),value))return false;
        if(value==state)++own;for(int j=0;j<i;++j)if(states[static_cast<unsigned>(j)]==value)return false;}
    if(own!=1)return false;
    for(int i=0;i<before.list.count;++i){bool exists=false;for(int j=0;j<list.count;++j)exists|=before.states[static_cast<unsigned>(i)]==states[static_cast<unsigned>(j)];if(!exists)return false;}
    if(!Same(r,current.State()+birthdisk::CurrentKiller,before.killer.object)||
       !Same(r,current.State()+birthdisk::CurrentKillerState,before.killerState.object)||
       !cfgprobe::lifetime::ReadSlot(r,current.table,before.killer.object,fresh)||!(fresh==before.killer)||
       !cfgprobe::lifetime::ReadSlot(r,current.table,before.killerState.object,fresh)||!(fresh==before.killerState)||
       !Same(r,current.State()+birthdisk::PlayerArray,list)||!Same(r,current.Mode()+cfgmenu::populationdisk::NumBots,bots))return false;
    for(int i=0;i<list.count;++i)if(!Same(r,list.data+8ULL*static_cast<unsigned>(i),states[static_cast<unsigned>(i)]))return false;
    Context after;if(!Capture<decltype(r),Bindings>(r,current.admission,thread,after)||!SameSession(current,after)||!CurrentListen<Bindings>(r,after)||
       !cfgprobe::lifetime::ReadSlot(r,after.table,created,fresh)||!(fresh==found.controller)||
       !cfgprobe::lifetime::ReadSlot(r,after.table,state,fresh)||!(fresh==found.state)||
       !cfgprobe::StableAncestry(r,state,ancestry)||!Same(r,created+cfgprobe::ControllerPawn,std::uintptr_t{})||
       !Same(r,created+cfgescape::disk::PlayerState,state)||!Same(r,state+0x88,created)||
       !Same(r,state+birthdisk::BotFlags,flags))return false;
    found.stateSerialAssigned=found.state.HasAssignedSerial();found.botDelta=bots-before.bots;output=found;return true;
}
}
