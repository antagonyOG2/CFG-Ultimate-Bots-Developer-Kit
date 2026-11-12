// Native overlap observation queues intent only. Real tick owns all AI actions.
    bool TrapEvent(std::uintptr_t victim,std::uintptr_t trap,std::uint64_t now){
        unsigned index=UINT32_MAX;
        if(victim==bundleReport.target)index=bundleTargetIndex;
        else if(victim==bundleHumanTarget.object||victim==plan.pawn)index=0;
        else for(unsigned i=0;i<bundleReport.bots;++i)if(victim==bundleReport.roster[i].pawn){index=i+1;break;}
        if(index==UINT32_MAX)return false;
        std::uintptr_t current=0;cfgtrappriority::Slot pawnSlot{},trapSlot{};
        if(!SummaryObservationScope())return false;
        std::uintptr_t killer=bundleJason.pawn.object;
#if defined(CFG_H1_JASON) && !defined(CFG_SPECTATOR_MEMBER_FIXTURE_ONLY)
        if(HostedRoutineActive()){
            JasonIdentity identity{};
            if(!JasonAdmission(identity)||!identity.hosted)return false;
            killer=identity.pawn.object;
        }else
#endif
        if(!Jason())return false;
        if(!Known(index,current)||current!=victim||
           !cfgtrappriority::Capture(r_,context.objectArray,plan.module,killer,victim,trap,pawnSlot,trapSlot)||
           !SummaryObservationScope()||!bundleTrapPriority.Queue(victim,index,now))return false;
        bundleTrapVictimSlot=pawnSlot;bundleTriggeredTrapSlot=trapSlot;bundleSchedule.huntAt=now;return true;
    }
