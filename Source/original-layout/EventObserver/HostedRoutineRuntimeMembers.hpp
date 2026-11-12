#include "GameplayWorldObservationMembers.hpp"
#ifdef CFG_H1_JASON
    void InvalidateHostedContextProof() noexcept { hostedContextProof_.Invalidate(); }
    bool HostedContext(bool human){
        if(!HostedRoutineActive()||GetCurrentThreadId()!=plan.threadId||!originalActorDispatcher)return false;
        // BundleAccess is callback-local. Reuse its complete hosted world and
        // adoption proof across J2.19 phases in this callback; native entry
        // invalidates the memo before dispatch so a changed owner is re-proved.
        if(!hostedContextProof_.Get([&](){
            return (hostedContextClause_=1,cfgprimary::LookupSafe(r_,plan.module))&&(hostedContextClause_=2,cfgprimary::CurrentHostedBinding().Matches(plan))&&
               (hostedContextClause_=3,cfgprimary::RetainedWorld(r_,plan,context))&&
               (hostedContextClause_=4,cfghosted::adoption::Current(r_,context.objectArray,hostedAdoptionReceipt,plan))&&
               (hostedContextClause_=5,cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))&&
               (hostedContextClause_=6,cfgobserver::Same(r_,spawnAssets.gameState+cfghosted::birth35::CurrentKiller,bundleJason.pawn.object))&&
               (hostedContextClause_=7,cfgobserver::Same(r_,spawnAssets.gameState+cfghosted::birth35::CurrentKillerState,hostedAdoptionReceipt.controllerState));
        }))return false;
        return !human||(cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])&&
            cfgjason::SameSlot(r_,context.objectArray,plan.objects[3])&&
            cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,plan.pawn)&&
            cfgobserver::Same(r_,plan.pawn+cfgescape::disk::Controller,plan.controller));
    }
    bool HostedRoutineRoster(){
        ++hostedRosterSamples;
        if(!HostedContext(false)){++hostedRosterRefused;hostedRosterReason=1;return /*recorded*/ false;}
        cfgspawn::PlayerArray list{},again{};
        if(!r_.Read(spawnAssets.gameState+cfghosted::birth35::PlayerArray,list)||list.count<1||list.count>32||list.capacity<list.count||list.capacity>64){++hostedRosterRefused;hostedRosterReason=2;return /*recorded*/ false;}
        std::array<cfgbundle::Bot,cfgbundle::MaximumBots> bots{};
        std::array<cfgprobe::lifetime::SlotObservation,cfgbundle::MaximumBots> pawns{},controllers{};unsigned count=0;
        for(int i=0;i<list.count;++i){std::uintptr_t state=0,controller=0,pawn=0;std::uint32_t flags=0;
            if(!r_.Read(list.data+8ULL*static_cast<unsigned>(i),state)||!state||!r_.Read(state+cfghosted::birth35::BotFlags,flags))return /*recorded*/ false;
            if(!(flags&cfghosted::birth35::BotMask)||state==hostedAdoptionReceipt.controllerState||(CFG_H256_HOSTED_F1_RESTART&&bundleAdditional.journal.killerState&&state==bundleAdditional.journal.killerState))continue; // F1 Jason is not a counselor bot
            if(count==bots.size()||!r_.Read(state+cfghosted::adoption::StateOwner,controller)||controller==plan.controller||
               !r_.Read(controller+cfgprobe::ControllerPawn,pawn)||pawn==plan.pawn||pawn==bundleJason.pawn.object||
               !cfgspawn::ObjectIs(r_,controller,spawnAssets.resources.classes[cfgspawn::CounselorAI])||!cfgspawn::ObjectIs(r_,pawn,bundleCounselorClass)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,pawn,pawns[count])||!pawns[count].HasAssignedSerial()||
               !cfgspawn::ObjectSlot(r_,context.objectArray,controller,controllers[count])||!controllers[count].HasAssignedSerial()||
               !cfghosted::native::WorldOwned(r_,pawn,plan.world)||!cfghosted::native::WorldOwned(r_,controller,plan.world)||
               !cfgobserver::Same(r_,pawn+cfgescape::disk::Controller,controller)||
               !cfgobserver::Same(r_,pawn+cfgescape::disk::PlayerState,state)||
               !cfgobserver::Same(r_,controller+cfgescape::disk::PlayerState,state))return /*recorded*/ false;
            for(unsigned j=0;j<count;++j)if(bots[j].pawn==pawn||bots[j].controller==controller)return /*recorded*/ false;
            bots[count]={pawn,controller,pawns[count].serial,controllers[count].serial};++count;
        }
        if(!r_.Read(spawnAssets.gameState+cfghosted::birth35::PlayerArray,again)||again.data!=list.data||again.count!=list.count||again.capacity!=list.capacity||!HostedContext(false))return /*recorded*/ false;
        // Observations of native-owned births; never write PlayerArray/NumBots or add another stock fill.
        for(unsigned i=0;i<hostedBots.size();++i)hostedBots[i].registered=0;
        for(unsigned i=0;i<count;++i){
            auto it=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){return d.pawn==bots[i].pawn;});
            if(it==hostedBots.end())it=std::find_if(hostedBots.begin(),hostedBots.end(),[](const auto& d){return !d.pawn;});
            if(it!=hostedBots.end()){it->pawn=bots[i].pawn;it->controller=bots[i].controller;it->registered=1;if(!it->source)it->source=2; /* native/stock baseline */}
        }
        hostedRosterReason=0;
        bundleReport.roster=bots;bundlePawnSlots=pawns;bundleControllerSlots=controllers;
        bundleReport.bots=bundleReport.spawned=count;bundleSchedule.requestedBots=bundleSchedule.spawned=count;bundleSchedule.helpersActive=false;
        return true;
    }
#include "HostedCounselorBrainMembers.hpp"

#endif

