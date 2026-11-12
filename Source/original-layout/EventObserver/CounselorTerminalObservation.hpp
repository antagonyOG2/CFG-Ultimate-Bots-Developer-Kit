#pragma once
#include "BundleObservations.hpp"
#include "../RuntimeProbe/EscapeBindings.generated.hpp"
#include "../Core/GameplayLimits.hpp"
#include <string_view>
namespace cfgescape {
template<class R,class T> bool Same(R& r,std::uintptr_t at,const T& expected){T actual{};return r.Read(at,actual)&&actual==expected;}
enum class State {Unknown,Living,Unpossessed,Dead,Escaped};
inline bool Terminal(State state){return state==State::Dead||state==State::Escaped;}
inline bool OfflineCompletionPhase(std::u16string_view phase){
    return phase==u"InProgress"||phase==u"WaitingPostMatchOutro";
}
// One-shot ending has already consumed native authority. Observe presentation
// at most once/second for75s, including the donor45s stalled-outro recovery.
// Never explicitly replay EndMatch or resume AI. Normal active
// matches have zero polls. The world/mode owner is revalidated by the caller.
struct PostMatchWatch {
    std::uint64_t until=0,next=0;bool active=false;
    void Begin(std::uint64_t now){active=now<=UINT64_MAX-75000;until=active?now+75000:0;next=now;}
    bool Due(std::uint64_t now){
        if(!active)return false;
        if(now>until||now>UINT64_MAX-1000){active=false;return false;}
        if(now<next)return false;
        next=now+1000;return true;
    }
    void Observe(std::u16string_view phase){
        if(phase!=u"InProgress"&&phase!=u"WaitingPostMatchOutro"&&phase!=u"PostMatchOutro")active=false;
    }
    void Lost(){active=false;}
};
// Read only. The caller supplies an exact current object-registry header; no
// class/name search, world census, getters, possession changes or bool writes.
template<class R> bool Counselor(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t module,std::uintptr_t pawn,std::uintptr_t counselorType,State& result){
    result=State::Unknown;cfgprobe::lifetime::SlotObservation slot{},fresh{},stateSlot{};
    std::uint8_t escaped=0,stateEscaped=0,dead=0;float health=0;
    std::uintptr_t state=0,stateType=0,controller=0;
    if(!cfgspawn::ObjectSlot(r,objects,pawn,slot)||!cfgspawn::ObjectIs(r,pawn,counselorType)||
       !r.Read(pawn+disk::bHasEscaped,escaped)||escaped>1||
       !r.Read(pawn+cfghunter::disk::Health,health)||!std::isfinite(health)||
       !r.Read(pawn+disk::PlayerState,state)||!r.Read(pawn+disk::Controller,controller))return false;
    if(state){
        if(!r.Read(module+disk::SCPlayerStateReflection,stateType)||!stateType||
           !cfgobserver::Same(r,module+disk::SCPlayerStateConstructor,stateType)||
           !cfgspawn::ObjectSlot(r,objects,state,stateSlot)||!cfgspawn::ObjectIs(r,state,stateType)||
           !r.Read(state+disk::bEscaped,stateEscaped)||stateEscaped>1||
           !r.Read(state+disk::bDead,dead)||dead>1||
           !Same(r,state+disk::bEscaped,stateEscaped)||!Same(r,state+disk::bDead,dead)||
           !cfgspawn::ObjectSlot(r,objects,state,fresh)||!(fresh==stateSlot))return false;
    }
    const auto value=(escaped||stateEscaped)?State::Escaped:
        ((dead||health<=cfghunter::disk::AliveThreshold)?State::Dead:
        (controller&&cfgobserver::Same(r,controller+disk::Pawn,pawn)?State::Living:State::Unpossessed));
    if(!Same(r,pawn+disk::PlayerState,state)||!Same(r,pawn+disk::bHasEscaped,escaped)||
       !Same(r,pawn+cfghunter::disk::Health,health)||!Same(r,pawn+disk::Controller,controller)||
       !cfgspawn::ObjectSlot(r,objects,pawn,fresh)||!(fresh==slot))return false;
    result=value;return true;
}
// Only for a driver freshly read from a registered native vehicle. Unlike a
// walking target, a healthy seated counselor need not own Controller.Pawn.
// Keep terminal/unknown evidence ineligible; this does not relax Known().
inline bool Driving(State state){return state==State::Living||state==State::Unpossessed;}
struct Completion {
    unsigned terminalMask=0;std::uint64_t candidateSince=0,nextCheck=0;bool dispatched=false;
    bool CheckDue(unsigned bots,bool helpersFinished,std::uint64_t now){
        // Cheap tracked flags only; ordinary gameplay pays NO roster scan.
        if(dispatched||bots>cfglimits::MaximumHelpers||!helpersFinished||terminalMask!=((1u<<(bots+1))-1)){
            candidateSince=nextCheck=0;return false;
        }
        if(now<nextCheck||now>UINT64_MAX-1000)return false;
        nextCheck=now+1000;return true;
    }
    void Observe(unsigned index,State state){
        if(index>cfglimits::MaximumHelpers)return;const unsigned bit=1u<<index;
        if(Terminal(state))terminalMask|=bit;
        else {terminalMask&=~bit;candidateSince=0;}
    }
    bool Ready(unsigned bots,bool helpersFinished,bool nativeTerminal,std::uint64_t now){
        if(dispatched||bots>cfglimits::MaximumHelpers||!helpersFinished||!nativeTerminal||terminalMask!=((1u<<(bots+1))-1)){
            candidateSince=0;return false;
        }
        if(!candidateSince){candidateSince=now;return false;}
        return now>=candidateSince&&now-candidateSince>=3000;
    }
};
struct TerminalWitness {unsigned failure=0,index=0;std::uintptr_t pawn=0;int count=0,hunter=0;std::uintptr_t pending=0;};
template<class R> bool NativeRosterTerminal(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t module,std::uintptr_t mode,std::uintptr_t gameState,std::uintptr_t counselorType,std::uintptr_t jason,TerminalWitness* witness=nullptr){
    struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
        bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}} list;
    std::int32_t hunter=0;std::uintptr_t pending=0;
    TerminalWitness proof;const auto refuse=[&](unsigned failure){proof.failure=failure;if(witness)*witness=proof;return false;};
    if(!r.Read(mode+disk::HunterToSpawn,pending))return refuse(1);proof.pending=pending;if(pending)return refuse(1);
    if(!r.Read(gameState+disk::HunterTimeRemaining,hunter))return refuse(2);proof.hunter=hunter;if(hunter>0)return refuse(2);
    if(!r.Read(gameState+disk::PlayerCharacterList,list)||list.count<0||list.count>16||
       list.capacity<list.count||list.capacity>32||(list.count&&!cfgprobe::ObjectPointer(list.data)))return refuse(3);
    proof.count=list.count;
    for(int i=0;i<list.count;++i){
        std::uintptr_t pawn=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!r.Read(list.data+8*static_cast<unsigned>(i),pawn)||!pawn||
           !cfgspawn::ObjectSlot(r,objects,pawn,slot)){proof.index=static_cast<unsigned>(i);proof.pawn=pawn;return refuse(4);}
        proof.index=static_cast<unsigned>(i);proof.pawn=pawn;
        if(pawn!=jason){State state;
            if(!Counselor(r,objects,module,pawn,counselorType,state))return refuse(5);
            if(!Terminal(state))return refuse(6);
        }
        if(!cfgobserver::Same(r,list.data+8*static_cast<unsigned>(i),pawn))return refuse(7);
    }
    if(!Same(r,gameState+disk::PlayerCharacterList,list)||
        !Same(r,mode+disk::HunterToSpawn,pending)||!Same(r,gameState+disk::HunterTimeRemaining,hunter))return refuse(7);
    if(witness)*witness=proof;return true;
}
}
