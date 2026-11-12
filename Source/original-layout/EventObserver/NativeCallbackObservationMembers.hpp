// Shared read-only offline callback proof; no tick/native-call authority.
    bool SummaryObservationWorld(){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()||HostedSummaryWindow())return cfgprimary::CurrentHostedBinding().Matches(plan)&&cfgprimary::RetainedWorld(r_,plan,context);
#endif
        return cfgprimary::RetainedOfflineWorld(r_,plan,context);
    }
    bool SummaryObservationScope(){
        cfgprobe::lifetime::SlotObservation state{};
        // Presentation must survive killer death/unpossession. Do not require
        // the gameplay singleton's live pawn/controller pair for a UI query.
        return GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&
            cfgprimary::LookupVetoClearForObservation(r_,plan.module)&&SummaryObservationWorld()&&
            cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])&&
            cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.gameState,state)&&state==spawnAssets.primary.stateSlot&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&SpectatorLocalOwner();
    }
    bool SpectatorCycleObservationScope(std::uintptr_t controller,std::int32_t direction){
        return controller==plan.controller&&(direction==1||direction==-1)&&SummaryObservationScope();
    }
