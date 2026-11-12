    bool ChaseRecoveryPhase(std::uint64_t now){
        auto& s=bundleChaseRecovery;if(!s.pending)return false;
        if(now>=s.until||bundleCarPursuit.Active(now)||bundleReport.target!=s.target){s.pending=false;return false;}
        if(!Context()||!Jason())return true;
        const auto index=s.cursor++;std::uintptr_t candidate=0;bool living=false;
        if(index>bundleReport.bots){s.pending=false;return true;}
        // The current target remains selectable if it is the only usable one.
        // This is an alternative proof, not HidingAlternative's pointer hint.
        if(!Known(index,candidate)||candidate==s.target||!Alive(candidate,living)||!living)return true;
        float distance=0;if(!Distance(candidate,distance)||!std::isfinite(distance))return true;
        std::uintptr_t fresh=0;if(!Known(index,fresh)||fresh!=candidate||!Alive(fresh,living)||!living)return true;
        bundleReport.target=candidate;bundleTargetIndex=index;bundleTargetLockUntil=now+10000;
        bundleFollowPath.Clear();bundlePathCooldown.Clear();bundlePursuit.Clear();bundleMoveAt=now;
        s.pending=false;s.repeated=0;
        cfgclosedtrace::Mark("chase-already-at-goal-alternative",s.target,candidate);return true;
    }
