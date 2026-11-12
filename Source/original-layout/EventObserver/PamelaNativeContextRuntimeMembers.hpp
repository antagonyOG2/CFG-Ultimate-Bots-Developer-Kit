// Active donor FJB TryGetValidatedJasonDeathContext 8808-8962 and
// TryCompleteSimplifiedPamelaTranceKill 12497-12502. Addresses are the
// existing exact CFG3.5 DeathContext proofs, not donor structure offsets.
    bool PamelaNativeContextPhase(std::uint64_t now){
        auto& policy=bundleKill.policy;
        if(!policy.pamela||now<policy.usedAt||now>=policy.until||policy.interactionAttempted||
           policy.stage==cfgkill::Stage::Uncertain||policy.stage==cfgkill::Stage::Expired)return false;
        if(!pamelaContextRetry_.Due(policy.usedAt,now))return false;
        // Scheduling hint only. No ownership/effects from this raw pointer.
        std::uintptr_t hint=0;
        if(!r_.Read(bundleJason.pawn.object+cfgkill::disk::JasonKillObject,hint)){
            pamelaContextRetry_.Failed(now);return false;
        }
        if(!hint)return false; // donor's cheap per-callback context hint
        cfgkill::Identity id{};cfgkill::Context death{};
        if(!Scope(id)||!(id==policy.identity)||!DeathContext(death)||!death.actor){
            pamelaContextRetry_.Failed(now);return false;
        }
        const bool first=policy.context!=death.actor;
        if(!policy.ObservePamelaNativeContext(id,death.actor,now,death.active)){
            pamelaContextRetry_.Failed(now);return true;
        }
        // The H1 receipt is the only route that opts this native-only
        // reservation into post-timeout recovery. Offline policy is unchanged.
        if(HostedAdoptionOwnsDriver()&&policy.pamelaNativeContextOnly&&death.active){
            policy.hostedNativeContextRecovery=true;
            policy.hostedNativeContextUsedAt=policy.usedAt;
        }
        if(first)cfgclosedtrace::Mark("pamela-native-context-accepted",death.actor,bundleKill.accepted);
        if(!death.active){
            if(!InstallKillEligibility(death)){pamelaContextRetry_.Failed(now);return true;}
        }
        if((pamelaStoppedContext_!=death.actor||pamelaStoppedUseAt_!=policy.usedAt)&&!PamelaContextStop()){
            pamelaContextRetry_.Failed(now);return true;
        }
        pamelaContextRetry_.Succeeded();
        bundleReport.killStage=static_cast<unsigned>(policy.stage);
        return policy.pamelaNativeContextOnly; // existing accepted-hit driver is unchanged
    }
