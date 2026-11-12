#include "../Core/DonorNativeMorphCompletion.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgdonorhunt;
int main(){unsigned checks=0;auto check=[&](bool b){++checks;assert(b);};
    for(auto purpose:{NavRingPurpose::Initial,NavRingPurpose::Distance,NavRingPurpose::SharedFifty,NavRingPurpose::Stuck,NavRingPurpose::Additional}){
        for(bool success:{false,true}){
            NavRingContinuation nav;ChaseState chase;AdditionalHuntState extra;
            nav.purpose=purpose;nav.stage=NavRingStage::Project;nav.identity.slots[3].object=99;
            chase.PathLocked=chase.HaveLastLocation=true;chase.LastAcceptedMoveAt=700;
            chase.ConsecutiveStuckChecks=2;extra.StalledSamples=3;extra.HasProgressSample=true;
            check(DonorConsumeNativeMorph(nav,chase,extra,1000)==MorphCompletion::Pending&&
                !chase.LastMorphTeleportAt&&!chase.NextDistanceTeleportAt&&!extra.NextMorphAt);
            nav.stage=success?NavRingStage::Succeeded:NavRingStage::Failed;
            check(DonorConsumeNativeMorph(nav,chase,extra,2000)==MorphCompletion::Completed&&nav.stage==NavRingStage::Idle);
            if(purpose==NavRingPurpose::Additional){
                check(extra.NextMorphAt==(success?22000:8000)&&!chase.LastMorphTeleportAt);
                check(success?extra.NextMoveAt==3000&&!extra.HasProgressSample&&!extra.StalledSamples:
                    !extra.NextMoveAt&&extra.HasProgressSample&&extra.StalledSamples==3);
            }else if(purpose==NavRingPurpose::Stuck){
                check(!chase.LastMorphTeleportAt&&!chase.NextDistanceTeleportAt);
                check(success?!chase.PathLocked&&!chase.HaveLastLocation&&!chase.ConsecutiveStuckChecks:
                    chase.NextStuckCheckAt==6750&&chase.ConsecutiveStuckChecks==1);
            }else if(success){
                check(chase.Target==99&&chase.LastMorphTeleportAt==2000&&chase.NextDistanceTeleportAt==22000&&!chase.PathLocked);
                check(purpose==NavRingPurpose::Initial?chase.LastAcceptedMoveAt==700&&chase.HaveLastLocation:
                    !chase.LastAcceptedMoveAt&&!chase.HaveLastLocation);
            }else check(chase.NextDistanceTeleportAt==(purpose==NavRingPurpose::Initial?0:5000)&&!chase.LastMorphTeleportAt);
            const auto ready=chase.NextDistanceTeleportAt,extraReady=extra.NextMorphAt;
            check(DonorConsumeNativeMorph(nav,chase,extra,3000)==MorphCompletion::Idle&&chase.NextDistanceTeleportAt==ready&&extra.NextMorphAt==extraReady);
        }
    }
    {NavRingContinuation nav;ChaseState chase;AdditionalHuntState extra;nav.stage=NavRingStage::Uncertain;
        check(DonorConsumeNativeMorph(nav,chase,extra,1000)==MorphCompletion::Quarantined&&nav.stage==NavRingStage::Uncertain&&!chase.LastMorphTeleportAt);
        nav.stage=NavRingStage::Invalid;check(DonorConsumeNativeMorph(nav,chase,extra,1000)==MorphCompletion::Cancelled&&nav.stage==NavRingStage::Idle);}
    for(bool success:{false,true}){NavRingContinuation nav;ChaseState chase;AdditionalHuntState extra;
        nav.purpose=NavRingPurpose::StartupFinish;nav.stage=NavRingStage::Project;nav.identity.slots[3].object=99;
        check(DonorConsumeNativeMorph(nav,chase,extra,1000)==MorphCompletion::Pending&&!chase.InitialTeleportAttempted);
        nav.stage=success?NavRingStage::Succeeded:NavRingStage::Failed;
        check(DonorConsumeNativeMorph(nav,chase,extra,2000)==MorphCompletion::Completed&&nav.stage==NavRingStage::Idle);
        check(chase.InitialTeleportAttempted==success);
        check(success?chase.LastMorphTeleportAt==2000&&chase.Target==99&&chase.NextDistanceTeleportAt==22000:
            chase.InitialTeleportAt==2500&&!chase.LastMorphTeleportAt&&!chase.NextDistanceTeleportAt);
    }
    std::printf("J1a actual native Morph completion: %u checks PASS; all6call sites,pending has no credit,returned exact cooldown/reset,uncertainty quarantines,no double completion.\n",checks);
}
