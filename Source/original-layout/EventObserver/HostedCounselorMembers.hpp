#include "HostedCounselorBirthBrainMembers.hpp"
    bool HostedCounselorPreflight(){
        auto& f=hostedCounselor;namespace b=cfghosted::birth35;namespace p=cfgmenu::populationdisk;
        f.stage=1;if(!HostedWorld())return false;
        f.stage=2;if(!cfghosted::native::PopulationBefore<Reader,cfghosted::listen35::Bindings,true>(r_,f.host.session,plan.threadId,f.before))return false;
        if(!cfghostedfill::CounselorRoom(f.before.list.count,HostedJasonReservationArmed(),HostedJasonReservationAdopted())){f.stage=14;return false;}
        const auto& ai=cfgcompat::cfg35::ai::Classes[2];std::uintptr_t table=0,restart=0;
        f.stage=3;bool warmed=false;
        if(!cfghosted::native::Class(r_,plan.module,ai.reflection,ai.constructor,f.ai)&&
           (!HostedWarmControllerClass(false,f.ai,warmed)||warmed))return false;
        f.stage=4;if(!cfgbundle::Code(r_,plan.module,p::SpawnParametersConstructor,p::SpawnParametersConstructorCode)||
           !cfgbundle::Code(r_,plan.module,p::SpawnActor,p::SpawnActorCode)||
           !cfgbundle::Code(r_,plan.module,b::Restart,b::RestartCode)||
           !cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::CopyConstruct,cfgmenu::birthdisk::CopyConstructCode)||
           !cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::ConsumeActive,cfgmenu::birthdisk::ConsumeActiveCode)||
           !(f.travel=cfgownednative::Resolve(plan.module,plan.module+p::HasFullyTraveled,p::HasFullyTraveledCode)))return false;
        f.stage=5;if(!r_.Read(plan.mode,table)||table!=plan.module+b::HuntVtable||!r_.Read(table+b::RestartSlot,restart)||restart!=plan.module+b::Restart)return false;
        f.stage=6;
        if(!cfghostedpacket::Choose(r_,plan.module,plan.mode,f.before.states,static_cast<unsigned>(f.before.list.count),
             static_cast<unsigned>(GetTickCount64()),f.counselorSource,f.packetDiagnostic))return false;
        if(!r_.Read(plan.controller+cfgescape::disk::PlayerState,f.sourceState)||!f.sourceState)return false;
        hostedTx.snap.world=plan.world;hostedTx.snap.mode=plan.mode;hostedTx.snap.gameState=f.before.context.State();
        hostedTx.snap.jason=f.before.killer.object;hostedTx.snap.human=plan.controller;hostedTx.snap.humanState=f.sourceState;
        hostedTx.gameState=f.before.context.State();hostedTx.restartFn=restart;return true;
    }
    bool HostedCounselorSpawn(){
        auto& f=hostedCounselor;f.stage=7;namespace p=cfgmenu::populationdisk;
        if(!HostedPrepare(HostedRoots(f.ai,plan.pawn),1))return false;
        ++calls_;f.entered=true;std::uintptr_t created=0;
        const bool returned=cfgmenu::population::detail::Spawn(plan.module+p::SpawnParametersConstructor,plan.module+p::SpawnActor,plan.world,f.ai,plan.mode,created);
        hostedTx.birth.controller=created;
        if(!returned){f.fault=true;BundleRetentionFailed();return false;}
        if(!created){f.fault=!FinishBundleNative();return false;}
        if(!RetainBundleReturnedObject(created)){f.fault=true;return false;}
        return true;
    }
    bool HostedCounselorNew(){
        auto& f=hostedCounselor;f.stage=8;
        if(!HostedWorld()||!cfghosted::native::ControllerBorn<Reader,cfghosted::listen35::Bindings,true>(r_,f.before,plan.threadId,hostedTx.birth.controller,f.born))return false;
        hostedTx.birth.state=f.born.state.object;hostedTx.birth.botFlag=true;return true;
    }
    bool HostedCounselorClass(){
        auto& f=hostedCounselor;f.stage=9;
        if(!HostedCopyCounselor(f.counselorSource,hostedCounselorPacket,f.packetDiagnostic))return false;
        f.copied=true;
        return HostedConsumeActive(f.born.state.object,hostedCounselorPacket,&f.consumed);
    }
    bool HostedCounselorRestart(){
        auto& f=hostedCounselor;f.stage=10;bool entered=false;
        if(!HostedRestart(plan.mode,f.born.controller.object,entered))return false;
        return HostedWorld()&&r_.Read(f.born.controller.object+cfgprobe::ControllerPawn,f.pawn)&&f.pawn&&f.pawn!=plan.pawn&&f.pawn!=f.before.killer.object;
    }
    bool HostedCounselorTraveled(){
        auto& f=hostedCounselor;f.stage=11;namespace p=cfgmenu::populationdisk;bool ready=false;
        if(!HostedPrepare(HostedRoots(f.pawn),0))return false;++calls_;
        if(!f.travel||cfgownednative::Resolve(plan.module,plan.module+p::HasFullyTraveled,p::HasFullyTraveledCode)!=f.travel||
           !cfgmenu::population::detail::Travel(f.travel,f.born.state.object,ready)){f.fault=true;BundleRetentionFailed();return false;}
        f.travelReturned=true;f.travelReady=ready;
        return FinishBundleNative(); // donor FullyTraveled is notification; ownership/roster checked by Post
    }
    bool HostedCounselorAccount(){
        auto& f=hostedCounselor;f.stage=12;std::int32_t bots=0;
        if(!HostedWorld()||!HostedReadBots(bots))return false;
        if(bots==f.before.bots+1)return true;
        if(bots!=f.before.bots||!HostedAccountBot(bots))return false;f.accounted=true;return true;
    }
    bool HostedPromoteCounselorState(){
        auto& f=hostedCounselor;auto& state=f.born.state;
        if(state.HasAssignedSerial())return true;
        // ControllerBorn intentionally admits a newly registered state before
        // weak serial assignment. Promote that same slot only after the full
        // birth tuple is independently corroborated at postconditions.
        cfgprobe::lifetime::SlotObservation current{},controller{};std::uintptr_t stateClass=0;
        std::uint32_t flags=0;std::uint8_t controllerRole=0,stateRole=0;
        std::int32_t count=0;bool contains=false;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,state.object,current)||
           !cfghostedfill::PromoteUnassignedSlot(state.object,state.index,state.serial,current.object,current.index,current.serial,true)||
           !cfgjason::SameSlot(r_,context.objectArray,f.born.controller)||
           !cfghosted::native::Class(r_,plan.module,cfgescape::disk::SCPlayerStateReflection,
                cfgescape::disk::SCPlayerStateConstructor,stateClass)||
           !cfgspawn::ObjectIs(r_,state.object,stateClass)||!HostedInWorld(state.object)||
           !r_.Read(f.born.controller.object+cfghosted::disk35::ActorRole,controllerRole)||controllerRole!=cfghosted::disk::AuthorityRole||
           !r_.Read(state.object+cfghosted::disk35::ActorRole,stateRole)||stateRole!=cfghosted::disk::AuthorityRole||
           !r_.Read(state.object+cfghosted::birth35::BotFlags,flags)||!(flags&cfghosted::birth35::BotMask)||
           !HostedReadRoster(state.object,count,contains)||!contains||count!=f.before.list.count+1||
           !cfgobserver::Same(r_,f.born.controller.object+cfgescape::disk::PlayerState,state.object)||
           !cfgobserver::Same(r_,f.born.controller.object+cfgprobe::ControllerPawn,f.pawn)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::Controller,f.born.controller.object)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::PlayerState,state.object)||
           !cfgobserver::Same(r_,state.object+0x88,f.born.controller.object)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,state.object,controller)||!(controller==current))return false;
        state=current;f.born.stateSerialAssigned=true;return true;
    }
    bool HostedCounselorPost(){
        auto& f=hostedCounselor;f.stage=13;std::uintptr_t type=0;std::int32_t bots=0,count=0;bool contains=false;
        if(!HostedWorld()||!HostedReadBots(bots)||bots!=f.before.bots+1||
           !HostedReadRoster(f.born.state.object,count,contains)||!contains||count!=f.before.list.count+1||
           !cfghosted::native::Class(r_,plan.module,cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,type)||
           !cfgspawn::ObjectIs(r_,f.pawn,type)||!HostedInWorld(f.pawn)||
           !cfgobserver::Same(r_,f.pawn+cfgescape::disk::Controller,f.born.controller.object)||
           !cfgobserver::Same(r_,f.born.controller.object+cfgprobe::ControllerPawn,f.pawn)||
           !cfgobserver::Same(r_,f.born.controller.object+cfgescape::disk::PlayerState,f.born.state.object)||
           !cfgobserver::Same(r_,f.born.state.object+0x88,f.born.controller.object)||
           !cfgobserver::Same(r_,hostedTx.gameState+cfghosted::birth35::CurrentKiller,f.before.killer.object)||
           !cfgobserver::Same(r_,hostedTx.gameState+cfghosted::birth35::CurrentKillerState,f.before.killerState.object))return false;
        for(int i=0;i<f.before.list.count;++i){std::int32_t observed=0;bool found=false;if(!HostedReadRoster(f.before.states[static_cast<unsigned>(i)],observed,found)||!found||observed!=count)return false;}
        f.pawnSlot={};(void)cfgspawn::ObjectSlot(r_,context.objectArray,f.pawn,f.pawnSlot);
        // Keep a valid counselor birth if the serial has not advanced yet. The
        // one-shot brain start remains independently guarded and may be deferred.
        (void)HostedPromoteCounselorState();
        auto trace=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){return d.pawn==f.pawn;});
        if(trace==hostedBots.end())trace=std::find_if(hostedBots.begin(),hostedBots.end(),[](const auto& d){return !d.pawn;});
        if(trace!=hostedBots.end()){trace->pawn=f.pawn;trace->controller=f.born.controller.object;trace->source=1;}
        return true;
    }
    bool HostedCounselorRollback(){
        auto& f=hostedCounselor;f.stage=14;if(f.fault||!HostedWorld()||!f.born.controller.object||f.born.controller.object!=hostedTx.birth.controller)return false;
        if(f.pawn&&(!HostedInWorld(f.pawn)||!HostedDestroy(f.pawn)))return false;
        const auto controller=hostedTx.birth.controller;
        if(controller&&(!HostedInWorld(controller)||!HostedDestroy(controller)))return false;
        std::int32_t bots=0,count=0;bool present=false;
        if(!HostedReadBots(bots))return false;
        if(bots==f.before.bots+1){if(!HostedUnaccountBot(bots))return false;}else if(bots!=f.before.bots)return false;
        if(!HostedReadRoster(hostedTx.birth.state,count,present)||present||count!=f.before.list.count)return false;
        for(int i=0;i<f.before.list.count;++i){bool found=false;if(!HostedReadRoster(f.before.states[static_cast<unsigned>(i)],count,found)||!found||count!=f.before.list.count)return false;}
        return !f.copied||f.consumed;
    }
