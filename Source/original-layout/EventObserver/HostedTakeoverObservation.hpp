#pragma once
#include "HostedListenAuthorityObservation.hpp"
#include "../RuntimeProbe/HostedBirth35.generated.hpp"

namespace cfghosted::takeover35 {
enum class Eligibility : unsigned {None,NoKiller,HostSelected,LocalCounselorNotReady,RemoteOwnership,Ready,HostPreborn};
struct Frame {
    listen35::Context host{};
    native::Slot killer{},human{},humanState{},localPawn{},connection{};
    std::array<native::Slot,8> players{};unsigned count=0;
    listen35::LocalPlayers list{};
    std::uintptr_t explicitPick=0,counselorTree=0;
    std::int32_t bots=0;std::uint8_t botFill=0;
    Eligibility eligibility=Eligibility::None;
};
// PlayerState/connection may have an unassigned weak serial. Observed slot
// membership is not retention. The native transaction must root every input
// before allocating/dispatching, then revalidate these same slots.
template<class R>bool ObservedTyped(R& r,const native::Table& table,std::uintptr_t object,std::uintptr_t type,native::Slot& output){
    cfgprobe::Ancestry ancestry;native::Slot before{},after{};
    if(!cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(type)||
       !cfgprobe::lifetime::ReadSlot(r,table,object,before)||!cfgprobe::StableAncestry(r,object,ancestry)||
       !cfgprobe::lifetime::ReadSlot(r,table,object,after)||!(after==before))return false;
    output=before;return true;
}
template<class R>bool Capture(R& source,const native::Admission& admission,std::uint32_t thread,Frame& output,unsigned* reads=nullptr){
    output={};native::Budget<R,8192> r(source);Frame frame;
    struct Finish{native::Budget<R,8192>& r;unsigned* result;~Finish(){if(result)*result=r.Used();}}finish{r,reads};
    if(!listen35::Capture(r,admission,thread,frame.host))return false;
    const auto& host=frame.host;const auto& session=host.session;const auto module=admission.module;
    std::uintptr_t stateType=0,controllerType=0,killerType=0,counselorType=0,connectionType=0;
    const auto& pc=cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass];
    const auto& killer=cfgprobe::NativeClasses[cfgprobe::Killer];const auto& counselor=cfgprobe::NativeClasses[cfgprobe::Counselor];
    if(!native::Class(r,module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateType)||
       !native::Class(r,module,pc.reflectionCacheRva,pc.constructorCacheRva,controllerType)||
       !native::Class(r,module,killer.reflectionCacheRva,killer.constructorCacheRva,killerType)||
       !native::Class(r,module,counselor.reflectionCacheRva,counselor.constructorCacheRva,counselorType)||
       !native::Class(r,module,birth35::ConnectionReflection,birth35::ConnectionConstructor,connectionType))return false;
    if(!r.Read(session.Mode()+birth35::NumBots,frame.bots)||frame.bots<0||frame.bots>7||
       !r.Read(session.Mode()+birth35::JasonSelectPlayer,frame.explicitPick)||
       !r.Read(session.Mode()+birth35::CounselorBehaviorTree,frame.counselorTree)||
       !r.Read(session.Mode()+birth35::BotFill,frame.botFill)||frame.botFill>1||
       !r.Read(session.State()+birth35::PlayerArray,frame.list)||frame.list.count<1||frame.list.count>8||
       frame.list.capacity<frame.list.count||frame.list.capacity>64||!cfgprobe::ObjectPointer(frame.list.data))return false;
    frame.count=static_cast<unsigned>(frame.list.count);
    for(unsigned i=0;i<frame.count;++i){std::uintptr_t player=0;
        if(!r.Read(frame.list.data+8*i,player)||!ObservedTyped(r,session.table,player,stateType,frame.players[i])||
           !native::WorldOwned(r,player,session.World()))return false;
        for(unsigned j=0;j<i;++j)if(frame.players[j].object==player)return false;
    }
    std::uintptr_t pawn=0,human=0,ps=0,localPawn=0,connection=0;
    if(!r.Read(session.State()+birth35::CurrentKiller,pawn)||!r.Read(session.State()+birth35::CurrentKillerState,ps))return false;
    if(!pawn&&!ps)frame.eligibility=Eligibility::NoKiller;
    else if(!pawn){
        // H2.55: the host won the lottery but was born a counselor (H2.54), so the killer state is the host's and no Jason pawn exists.
        bool present=false;for(unsigned i=0;i<frame.count;++i)if(frame.players[i].object==ps)present=true;
        std::uintptr_t localState=0;
        if(!present||!ObservedTyped(r,session.table,ps,stateType,frame.humanState)||
           !r.Read(host.local[2].object+cfgescape::disk::PlayerState,localState)||localState!=ps||
           !r.Read(host.local[2].object+cfgprobe::ControllerPawn,localPawn)||
           !native::Typed(r,session.table,localPawn,counselorType,frame.localPawn)||
           !native::Same(r,localPawn+cfgescape::disk::Controller,host.local[2].object)||
           !native::WorldOwned(r,localPawn,session.World()))return false;
        frame.human=host.local[2];frame.eligibility=Eligibility::HostPreborn;
    }
    else {
        bool present=false;for(unsigned i=0;i<frame.count;++i)if(frame.players[i].object==ps)present=true;
        if(!present||!native::Typed(r,session.table,pawn,killerType,frame.killer)||
           !ObservedTyped(r,session.table,ps,stateType,frame.humanState)||
           !r.Read(pawn+cfgescape::disk::Controller,human)||!native::Typed(r,session.table,human,controllerType,frame.human)||
           !native::Same(r,human+cfgprobe::ControllerPawn,pawn)||!native::Same(r,human+cfgescape::disk::PlayerState,ps)||
           !native::WorldOwned(r,pawn,session.World())||!native::WorldOwned(r,human,session.World()))return false;
        if(human==host.local[2].object){
            // Donor takeover accepts the local selected Jason too. This is still
            // the OLD pawn; the local counselor must be resolved after Restart.
            localPawn=pawn;frame.localPawn=frame.killer;
            std::uint32_t flags=0;
            if(!r.Read(ps+birth35::BotFlags,flags)||(flags&birth35::BotMask))return false;
            frame.eligibility=Eligibility::HostSelected;
        }
        else if(!r.Read(host.local[2].object+cfgprobe::ControllerPawn,localPawn)||
                !native::Typed(r,session.table,localPawn,counselorType,frame.localPawn)||
                !native::Same(r,localPawn+cfgescape::disk::Controller,host.local[2].object)||
                !native::WorldOwned(r,localPawn,session.World()))frame.eligibility=Eligibility::LocalCounselorNotReady;
        else {
            std::uint32_t botFlags=0;
            if(!r.Read(human+disk35::ControllerPlayer,connection)||
               !ObservedTyped(r,session.table,connection,connectionType,frame.connection)||
               !native::Same(r,human+birth35::ControllerConnection,connection)||
               !native::Same(r,connection+disk35::PlayerController,human)||
               !native::Same(r,connection+birth35::ConnectionActor,human)||
               !native::Same(r,connection+birth35::ConnectionDriver,session.Driver())||
               !r.Read(ps+birth35::BotFlags,botFlags)||(botFlags&birth35::BotMask))frame.eligibility=Eligibility::RemoteOwnership;
            else frame.eligibility=Eligibility::Ready;
        }
    }
    // Fresh tuple and roster endpoints; no adoption of a different world or
    // changed player membership while the census was in progress.
    listen35::Context again;
    if(!listen35::Capture(r,admission,thread,again)||!native::SameSession(host.session,again.session)||host.local!=again.local||
       !native::Same(r,session.State()+birth35::PlayerArray,frame.list)||
       !native::Same(r,session.State()+birth35::CurrentKiller,pawn)||!native::Same(r,session.State()+birth35::CurrentKillerState,ps)||
       !native::Same(r,session.Mode()+birth35::NumBots,frame.bots)||
       !native::Same(r,session.Mode()+birth35::JasonSelectPlayer,frame.explicitPick))return false;
    for(unsigned i=0;i<frame.count;++i){native::Slot slot;
        if(!native::Same(r,frame.list.data+8*i,frame.players[i].object)||
           !ObservedTyped(r,again.session.table,frame.players[i].object,stateType,slot)||!(slot==frame.players[i]))return false;
    }
    const std::array<native::Slot,5> selected{{frame.killer,frame.human,frame.humanState,frame.localPawn,frame.connection}};
    for(const auto& expected:selected){native::Slot actual;
        if(expected.object&&(!cfgprobe::lifetime::ReadSlot(r,again.session.table,expected.object,actual)||!(actual==expected)))return false;}
    if(frame.killer.object&&(!native::Same(r,pawn+cfgescape::disk::Controller,human)||
       !native::Same(r,human+cfgprobe::ControllerPawn,pawn)||!native::Same(r,human+cfgescape::disk::PlayerState,ps)))return false;
    if(frame.eligibility==Eligibility::Ready){std::uint32_t flags=0;
        if(!native::Same(r,host.local[2].object+cfgprobe::ControllerPawn,localPawn)||
           !native::Same(r,localPawn+cfgescape::disk::Controller,host.local[2].object)||
           !native::Same(r,human+disk35::ControllerPlayer,connection)||
           !native::Same(r,human+birth35::ControllerConnection,connection)||
           !native::Same(r,connection+disk35::PlayerController,human)||
           !native::Same(r,connection+birth35::ConnectionDriver,session.Driver())||
           !native::Same(r,connection+birth35::ConnectionActor,human)||
           !r.Read(ps+birth35::BotFlags,flags)||(flags&birth35::BotMask))return false;
    }
    output=frame;return true;
}
}
