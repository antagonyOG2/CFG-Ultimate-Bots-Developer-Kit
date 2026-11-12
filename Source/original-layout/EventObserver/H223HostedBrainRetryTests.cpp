#include "../Core/HostedCounselorBirth.hpp"
#include <cstdio>

namespace {
struct Ticket {
    cfghostedfill::BrainRetryLocalIdentity local{};
    std::uintptr_t world=0,bornController=0,bornState=0,bornPawn=0;
    unsigned retries=0,dispatches=0;bool pending=true;
};
bool Enter(Ticket& t,std::uintptr_t actor,const cfghostedfill::BrainRetryLocalIdentity& current,
    std::uintptr_t world,std::uintptr_t controller,std::uintptr_t state,std::uintptr_t pawn){
    return t.pending&&cfghostedfill::MatchBrainRetryLocal(t.local,world,actor,current.controller,
        current.controllerIndex,current.controllerSerial,current.pawn,current.pawnIndex,current.pawnSerial)&&
        cfghostedfill::MatchBrainRetryBirth(t.world,t.bornController,t.bornState,t.bornPawn,
            world,controller,state,pawn);
}
void Window(Ticket& t,bool tupleCurrent,bool slotsReady,bool dispatchEntered){
    const auto decision=cfghostedfill::DecideBrainRetry(tupleCurrent,slotsReady,t.dispatches!=0,t.retries);
    if(decision==cfghostedfill::BrainRetry::Dispatch){++t.dispatches;if(dispatchEntered)t.pending=false;}
    else if(decision==cfghostedfill::BrainRetry::Cancel)t.pending=false;
    else if(++t.retries>=3)t.pending=false;
}
void Leave(Ticket& t,bool serviced){if(t.pending&&!serviced&&++t.retries>=3)t.pending=false;}
}
int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* text){++checks;if(!okay){++failures;std::printf("FAIL %s\n",text);}};
    constexpr std::uintptr_t world=0x700000,localController=0x300000,localPawn=0x301000;
    constexpr std::uintptr_t bornController=0x500000,bornState=0x501000,bornPawn=0x901000;
    Ticket queue[8]{};auto& ticket=queue[0];ticket.world=world;ticket.bornController=bornController;
    ticket.bornState=bornState;ticket.bornPawn=bornPawn;
    ticket.local={world,localController,localPawn,3,7,4,11};
    // F3 ticket owns a bot birth, while callbacks arrive on the distinct local human pawn.
    auto localZero=ticket.local;localZero.pawnSerial=11;
    check(Enter(ticket,localPawn,localZero,world,bornController,bornState,bornPawn),"local callback enters distinct born-pawn window");
    Window(ticket,true,false,false);Leave(ticket,true);
    check(ticket.pending&&ticket.retries==1&&ticket.dispatches==0,"window waits for delayed positive birth slots");
    // A second F3 updates the mutable last-birth state; copied queue ticket stays intact.
    std::uintptr_t mutableLastBirthPawn=ticket.bornPawn;const auto savedBirth=ticket.bornPawn;
    mutableLastBirthPawn=0x902000;check(ticket.bornPawn==savedBirth&&mutableLastBirthPawn!=ticket.bornPawn,
        "second F3 cannot overwrite pending copied birth evidence");
    auto localReady=ticket.local;
    check(Enter(ticket,localPawn,localReady,world,bornController,bornState,savedBirth),"same local host callback resumes copied ticket");
    Window(ticket,true,true,true);Leave(ticket,true);
    check(!ticket.pending&&ticket.dispatches==1,"window dispatches once and closes ticket");
    check(!Enter(ticket,localPawn,localReady,world,bornController,bornState,bornPawn),"completed native dispatch is not replayed");
    auto changedLocal=localReady;changedLocal.pawn=0x302000;changedLocal.pawnIndex=5;
    check(!Enter(queue[1],localPawn,localReady,world,bornController,bornState,bornPawn),"empty queue has no accidental ticket");
    Ticket changed{};changed.world=world;changed.bornController=bornController;changed.bornState=bornState;changed.bornPawn=bornPawn;
    changed.local=ticket.local;changed.pending=true;
    check(!Enter(changed,localPawn,changedLocal,world,bornController,bornState,bornPawn),"changed local pawn rejects retry");
    check(!Enter(changed,localPawn,localReady,world,bornController,bornState,0x902000),"changed birth pawn rejects retry");
    check(!Enter(changed,localPawn,localReady,world,bornController,0x502000,bornPawn),"changed birth state rejects retry");
    Ticket observed{};observed.pending=true;observed.world=world;
    check(!cfghostedfill::RetireBrainTicket(false,false,false)&&observed.pending,
        "invalid takeover observation retains ticket in adopted-host scenario");
    check(!cfghostedfill::RetireBrainTicket(false,true,true)&&observed.pending,
        "successful same-local-plan proof retains matching ticket");
    check(cfghostedfill::RetireBrainTicket(false,true,false),"successful local-plan proof retires changed local identity");
    check(!cfghostedfill::RetireBrainTicket(false,false,false)&&observed.pending,
        "failed local-plan proof does not retire same-world ticket");
    check(cfghostedfill::RetireBrainTicket(true,false,false),"independently proved changed world retires ticket");
    Ticket denied{};denied.world=world;denied.bornController=bornController;denied.bornState=bornState;denied.bornPawn=bornPawn;
    denied.local=ticket.local;denied.pending=true;const bool otherActor=Enter(denied,0x401000,localReady,world,bornController,bornState,bornPawn);
    check(!otherActor&&denied.pending,"unrelated callback leaves retry ticket and other work available");
    std::printf("H223 hosted F3 delayed brain retry route: %u checks, %u failures; local pawn distinct from born pawn, bounded predispatch retries, one dispatch-policy entry.\n",checks,failures);
    return failures?1:0;
}
