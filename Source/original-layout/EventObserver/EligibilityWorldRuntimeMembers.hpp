    bool EligibilityWorldCurrent(bool nativeInteractor){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            // Engine-origin predicate forwarding is observation, NOT a new
            // native dispatch. The exact bound listen-host World is still proved.
            return nativeInteractor?SummaryObservationWorld():GameplayWorld(context);
        }
#else
        (void)nativeInteractor;
#endif
        return cfgobserver::Same(r_,plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
    }
