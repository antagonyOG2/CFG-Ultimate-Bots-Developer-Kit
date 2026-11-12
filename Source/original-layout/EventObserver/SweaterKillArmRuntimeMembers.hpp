    bool SweaterKillArmPhase(std::uint64_t now,bool sweaterOpportunity=false){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::KillPrompt);
#endif

        // A newly witnessed activation owns its own fresh-budget admission,
        // before the preceding unconsumed Pamela health observation.
        if(bundleSweaterPolicy.ServiceArm(*this,bundleKill,bundleSweaterOwner.object,now)){
            bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
            bundleReport.killFailure=static_cast<unsigned>(bundleKill.failure);
            return true;
        }
        // A slow Convergence callback may always arrive later than healthAt.
        // Owe its NEXT sweater opportunity after an unchanged-health probe;
        // other/global health callbacks never consume this owed phase.
        if(sweaterOpportunity&&bundleSweaterPolicy.healthOpportunityOwed){
            bundleSweaterPolicy.healthOpportunityOwed=false;return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::KillPrompt,1)
#else
 false
#endif
;
        }
        if(bundleKill.policy.pamela){
            bundleKill.policy.Expire(now);
            if(bundleKill.policy.stage==cfgkill::Stage::AwaitHit&&now>=bundleKill.pamelaHealthAt){
                bundleKill.pamelaHealthAt=now+250;
                bundleKill.ObservePamelaHealth(*this,now);
                if(sweaterOpportunity&&bundleKill.policy.stage==cfgkill::Stage::AwaitHit)
                    bundleSweaterPolicy.healthOpportunityOwed=true;
                bundleReport.killAcceptedHits=bundleKill.accepted;bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
                return true; // own fresh-budget kill observation phase
            }
        }
        return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::KillPrompt,2)
#else
 false
#endif
;
    }
