#pragma once
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
inline void PrintJ21cHeldDiagnostics(const cfgbundle::Report& report){
    std::printf("J23NearFollow Moves=%llu DisabledBrain=%llu Refused=%llu (within600cm, goal shift>150cm, minimum1000ms; native brain ON, melee assist OFF)\n",report.j23NearFollowMoves,report.j23NearFollowDisabled,report.j23NearFollowRefused);
    for(unsigned i=0;i<report.j23Arrival.size();++i){const auto& d=report.j23Arrival[i];if(!d.pawn)continue;
        std::printf("J23Arrival Bot=%u At=%llu Pawn=0x%llX Controller=0x%llX DistanceCm=%.1f BrainKnown=%u Enabled=%u BrainAt=%llu WeaponKnown=%u Held=%u Weapon=0x%llX Class=0x%llX WeaponAt=%llu NearMoves=%llu BTTask=UNVERIFIED\n",
            i,d.at,static_cast<unsigned long long>(d.pawn),static_cast<unsigned long long>(d.controller),d.distance,d.brainKnown,d.brainEnabled,d.brainAt,
            d.weaponKnown,d.held,static_cast<unsigned long long>(d.weapon),static_cast<unsigned long long>(d.weaponClass),d.weaponAt,d.nearMoves);
        for(unsigned key=0;key<cfgconvergence::KeyCount;++key)if(d.knownKeys&(1u<<key))
            std::printf("J23ArrivalKey Bot=%u Key=%u ReadAt=%llu Value=0x%llX (last native readback, not a current authority)\n",i,key,d.readAt[key],static_cast<unsigned long long>(d.values[key]));
    }
    std::printf("J23EmoteReadBudget Deferrals=%llu Dropped=%llu (pre-dispatch proof only; no native retries)\n",report.j23EmoteReadDeferrals,report.j23EmoteBudgetDropped);
    std::printf("J21cFlags SharedConverter=%u HeldRecovery=%u ReaderCounters=%u LazyInputRoots=%u\n",
        CFG_J21C_SHARED_CONVERTER,CFG_J21C_HELD_RECOVERY,CFG_J21C_READER_COUNTERS,CFG_J21C_LAZY_INPUT_ROOTS);
    for(unsigned role=0;role<report.j21cHeld.size();++role){const auto& d=report.j21cHeld[role];
        std::printf("J21cHeld Role=%u ObserveStage=%u QueryStage=%u InputStage=%u AllowedWaits=%llu KillingWaits=%llu FinalWaits=%llu SeatWaits=%llu DeadWaits=%llu StaleReleases=%llu Fallbacks=%llu QueryReads=%llu ReadBudget=%llu CallBudget=%llu GrabFinished=%u\n",
            role,d.observeStage,d.queryStage,d.inputStage,d.allowedWaits,d.killingWaits,d.finalWaits,d.seatWaits,d.deadWaits,
            d.staleReleases,d.fallbacks,d.queryReads,d.readBudget,d.callBudget,report.grabFinished[role]);
        std::printf("J21cHeldObserveFailures Role=%u",role);for(auto count:d.observeFailures)std::printf(" %llu",count);std::puts("");
        std::printf("J21cHeldNotEntered Role=%u",role);for(auto count:d.notEntered)std::printf(" %llu",count);std::puts("");
        {const auto& h=report.h247Held[role];
         std::printf("H247Held Role=%u SeatWaitsSeat=%llu SeatWaitsExtracting=%llu StaleSeatIgnored=%llu SeatProofFailures=%llu ScanSkippedStruggling=%llu (SeatWaitsSeat+Extracting split J21cHeld SeatWaits; stale = CurrentSeat not a vehicle seat listing the victim)\n",
            role,h.seatWaitsSeat,h.seatWaitsExtracting,h.staleSeatIgnored,h.seatProofFailures,h.scanSkippedStruggling);
         std::printf("H280Held Role=%u CommitGuardWaits=%llu PostCommitReleases=%llu PostCommitKills=%llu OfflineF1Releases=%llu\n",role,h.commitGuardWaits,h.postCommitReleases,h.postCommitKills,h.offlineF1Releases);}
        if(role<report.h281Unstick.size()){const auto& s=report.h281Unstick[role];
         std::printf("H281Unstick Role=%u WaterEscapes=%u WaterRefused=%u FrozenEscapes=%u FrozenRefused=%u LastReason=%u FailedRings=%u OtherTarget=%u\n",role,s.waterEscapes,s.waterRefused,s.frozenEscapes,s.frozenRefused,s.lastReason,
             role<report.h288FailedRings.size()?report.h288FailedRings[role]:0u,role<report.h288OtherTargets.size()?report.h288OtherTargets[role]:0u);}
        if(role<report.h283Kill.size()){const auto& k=report.h283Kill[role];
         std::printf("H283KillDamage Role=%u Hits=%llu HitsZero=%llu Kills=%llu KillsApplied=%llu KillsZero=%llu LastKillIncoming=%.1f LastKillResult=%.1f LastKillHealth=%.1f->%.1f LastKillOwnInstigator=%u ReleasedAlive=%llu ReleasedDead=%llu ReleaseHealth=%.1f ReleaseContext=%u\n",
            role,k.hits,k.hitsZero,k.kills,k.killsApplied,k.killsZero,k.lastKillIncoming,k.lastKillResult,k.lastKillHealthBefore,k.lastKillHealthAfter,k.lastKillOwnInstigator,
            k.releasedAlive,k.releasedDead,k.releaseHealth,k.releaseContext);
         std::printf("H283KillGates Role=%u Snapshots=%llu Clause=%u OwnerRole=%u CompKiller=%u CompVictim=%u PerWeapon=%u Weapon=%u MapEntries=%u MatchedEntry=%u MoveKillFlag=%u VictimHealth=%.1f VictimContext=%u VictimInvulnerable=%u\n",
            role,k.snapshots,k.clause,k.ownerRole,k.compKiller,k.compVictim,k.perWeapon,k.weapon,k.mapEntries,k.matchedEntry,k.moveKillFlag,k.victimHealth,k.victimContext,k.victimInvulnerable);}
    }
    std::puts("H283 native EndAction kills only via ForceKillCharacter->TakeDamage when MoveKillFlag=1 and OwnerRole=3; Kills=kill-sized TakeDamage (>=1000) with this Jason as causer. Comp/Victim 1=this Jason/victim 2=other 0=null; MatchedEntry 254=no weapon row (default move); 255=unread. Clause 1=slot actor 2=component 3=move 4=flag read.");
    std::printf("H285ChaseBurst PrimaryRequests=%llu PrimaryStarts=%u PrimaryChaseStarts=%u F1Starts=%u F1ChaseStarts=%u (chase = target within 30 m at start: 3 s recovery instead of 7 s)\n",
        report.h285BurstRequests,report.h285Starts[0],report.h285ChaseStarts[0],report.h285Starts[1],report.h285ChaseStarts[1]);
    std::printf("H285BurstVeto BeginBlocked=%llu ProbeAllowed=%llu Car=%llu CarSpeed=%llu Traps=%llu Hiding=%llu Door=%llu KnifeThrow=%llu KnifeProbeFar=%llu (primary; ticks the wanted burst was vetoed)\n",
        report.h285BeginBlocked,report.h285Veto[0],report.h285Veto[1],report.h285Veto[2],report.h285Veto[3],report.h285Veto[4],report.h285Veto[5],report.h285Veto[6],report.h285Veto[7]);
    {const auto avg=[](std::uint64_t total,std::uint64_t n){return n?total/n:0ull;};
     std::printf("H291RootWindow Windows=%llu AvgUs=%llu MaxUs=%llu Retries=%llu Sleeps=%llu SleepUs=%llu BeginUs=%llu OperationUs=%llu EndUs=%llu TotalUs=%llu (callback GC root windows; admission = Total-Sleep-Begin-Operation-End)\n",
        report.h291RootWindows,avg(report.h291RootWindowUs,report.h291RootWindows),report.h291RootWindowMaxUs,report.h291RootRetries,report.h291RootSleeps,
        report.h291RootSleepUs,report.h291RootBeginUs,report.h291RootOperationUs,report.h291RootEndUs,report.h291RootWindowUs);
     std::printf("H292RootProvenSkips=%llu (retains satisfied by claims already proved after the last native entry; no window opened)\n",report.h292RootProvenSkips);
     for(const auto& [name,c]:{std::pair<const char*,const cfgbundle::Report::H291Cost&>{"HeldQuery",report.h291HeldQuery},{"GrabKillInput",report.h291HeldInput}})
        std::printf("H291Native %s Count=%llu AvgUs=%llu MaxUs=%llu OverFrame=%llu TotalUs=%llu (native call time only)\n",name,c.count,avg(c.us,c.count),c.maxUs,c.overFrame,c.us);}
    std::printf("H286Nearest Chosen=%llu Empty[NotReady,CountMismatch,OrderMismatch,NoneEligible]=%llu,%llu,%llu,%llu Skips[Blocked,NotLive,NoController,NoPosition,Excluded]=%llu,%llu,%llu,%llu,%llu (both Jasons, per decision)\n",
        report.h286NearestChosen,report.h286NearestEmpty[0],report.h286NearestEmpty[1],report.h286NearestEmpty[2],report.h286NearestEmpty[3],
        report.h286NearestSkips[0],report.h286NearestSkips[1],report.h286NearestSkips[2],report.h286NearestSkips[3],report.h286NearestSkips[4]);
    std::printf("H286F1NewMatchResets=%llu (previous match's F1 cleared at the first F1 press of a new match)\n",report.h286F1NewMatchResets);
    std::printf("H286BurstLead PrimaryLed=%llu PrimaryUnled=%llu F1Led=%llu F1Unled=%llu (burst moves aimed 2 s ahead of the counselor, re-aimed every 500 ms; unled = counselor not running or no heading)\n",
        report.h286Lead[0].led,report.h286Lead[0].unled,report.h286Lead[1].led,report.h286Lead[1].unled);
    for(unsigned role=0;role<report.h284Cutoff.size();++role){const auto& c=report.h284Cutoff[role];
        std::printf("H284MorphCutoff Role=%u PursuitMorphs=%llu Led=%llu NoHeading=%llu LandedClose=%llu LandedRing=%llu (Led=counselor running, landings tried ahead first; Close=7-18 m ahead)\n",
            role,c.pursuitMorphs,c.led,c.noHeading,c.landedClose,c.landedRing);}
    std::printf("H283F1Class Attempts=%llu Applied=%llu Already=%llu LastRefusal=%u (offline F1 PlayerState Jason class; Applied/Already>0 = F1 grab kills enabled)\n",report.h283ClassAttempts,report.h283ClassApplied,report.h283ClassAlready,report.h283ClassRefusal);
    for(unsigned role=0;role<report.j23GrabTiming.size();++role){const auto& t=report.j23GrabTiming[role];
        std::printf("J23GrabTiming Role=%u Holds=%llu FirstAllowedSamples=%llu AllowedTotalMs=%llu AllowedMaxMs=%llu FirstAvailableSamples=%llu AvailableTotalMs=%llu AvailableMaxMs=%llu\n",
            role,t.holds,t.allowed,t.allowedTotalMs,t.allowedMaxMs,t.available,t.availableTotalMs,t.availableMaxMs);
        for(unsigned i=0;i<t.count;++i){const auto& s=t.samples[i];
            std::printf("J23GrabSample Role=%u Cell=%u KnownMask=%u FirstAllowedMs=%llu FirstAvailableMs=%llu FirstAvailableSlot=%u\n",
                role,i,s.known,s.allowedMs,s.availableMs,s.slot);}
        for(unsigned i=0;i<4;++i)std::printf("J23FrozenGrabInput Role=%u Direction=%u Entered=%llu Returned=%llu Uncertain=%llu\n",
            role,i,t.frozenEntered[i],t.frozenReturned[i],t.frozenUncertain[i]);
    }
    std::puts("J23 timing is FIRST OBSERVED hold, not native grab-animation start. KnownMask:1=allowed,2=available; absent bit=UNOBSERVED, not0ms. Frozen input return is NOT a kill confirmation. Directions Top/Bottom/Left/Right reach native inputs3/1/2/0; Bottom has an additional native guard.");
    std::puts("J21cObserveStages=1ownerHeld,2victimSlotType,3stateFlags,4position,5IsGrabKilling,6CanGrabKill,7finalCurrent,8waterCurrent; QueryStages=1currentMetadata,2retention,3invoke; NotEntered=0slot,1-3query,8+inputStage,15RNG; InputStages=1slot,2CanGrabKill,3seatExtractionCode,4nativeAdmission,5retention,6begin");
}
