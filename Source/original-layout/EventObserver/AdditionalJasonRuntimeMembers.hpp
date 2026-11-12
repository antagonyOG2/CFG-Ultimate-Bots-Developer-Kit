// Included in BundleAccess, not a second injected backend. CFG bindings and
// current offline authority are reused; all actor/action state is per extra.
#ifdef CFG_FRONTEND_BUNDLE
    bool AdditionalConstructionReady(){return PrepareAdditionalConstructionTrace(plan.module);}
    bool AdditionalPawnSpawn(std::uintptr_t type,cfgspawn::Vec3 at,std::uintptr_t& output){
        auto& scope=bundleAdditionalConstructionScope;
        if(scope.active)return false;
        scope={plan.module,plan.world,plan.mode,spawnAssets.gameState,bundleAdditional.selected.pawnClass,
            bundleJason.pawn.object,plan.pawn,plan.threadId,true};
        scope.primaryState=bundleAdditional.journal.primary.state;
        scope.primaryController=bundleJason.killerController.object;
        scope.primarySlot=bundleJason.pawn;
        scope.primaryControllerSlot=bundleJason.killerController;
        struct Exit {cfgadditionalconstruction::Scope& scope;~Exit(){scope={};}}exit{scope};
        cfgclosedtrace::Mark("f1-selected-native-catalogue-class",scope.type,bundleAdditional.selectedIndex);
        // Donor actor-first birth: do not let SpawnAIFromClass possess this
        // killer with a generic AI controller before the typed handoff.
        // H2.56: donor hosted F1 spawns with AdjustIfPossibleButAlwaysSpawn (OfflineSetup.cpp:10456); offline keeps its proven value.
        const bool returned=AdditionalNativeSpawn(type,at,output,HostedRoutineActive()?2:1);
        if(!output){
            cfgprobe::lifetime::SlotObservation witness{};
            if(cfgadditionalconstruction::CleanupWitness(r_,scope,GetCurrentThreadId(),witness)){
                output=witness.object;
                cfgclosedtrace::Mark("f1-construction-cleanup-witness",output,witness.serial);
                return false; // restoration/rollback only; never activate an ambiguous birth
            }
        }
        return returned;
    }
#include "AdditionalJasonRegistryMembers.hpp"
#include "AdditionalJasonBirthMembers.hpp"
#include "AdditionalJasonLifecycleMembers.hpp"
#if CFG_H283_F1_KILLER_CLASS
    // H2.83: same verified consuming setter and packet rules as HostedPrepareAi. Source = the primary AI Jason's PlayerState
    // (journal pair captured at Project). A refusal leaves the F1 as before (the H2.82 release fallback stays in force).
    bool AdditionalKillerClass(std::uintptr_t state){
        namespace d=cfgmenu::birthdisk;auto& s=bundleAdditional;++bundleReport.h283ClassAttempts;
        const auto refuse=[&](std::uint32_t clause){bundleReport.h283ClassRefusal=clause;cfgclosedtrace::Mark("h283-f1-class-refused",state,clause);return false;};
        cfgmenu::promotion::Packet packet{},again{};const auto source=s.primaryState;
        if(!state||!source||state==source||calls_>=cfgbundle::CallCap||!cfgspawn::ObjectIs(r_,state,spawnAssets.resources.classes[cfgspawn::PlayerState]))return refuse(1);
        if(!cfgbundle::Code(r_,plan.module,d::ConsumeActive,d::ConsumeActiveCode))return refuse(2);
        if(!r_.Read(source+d::ActiveCharacterClass,packet)||!cfgending::EmptySubpath(packet.bytes)||packet.bytes==decltype(packet.bytes){}||
           !r_.Read(source+d::ActiveCharacterClass,again)||!cfgending::SamePath(packet.bytes,again.bytes))return refuse(3);
        const auto expected=packet.bytes;
        if(r_.Read(state+d::ActiveCharacterClass,again)&&cfgending::SamePath(again.bytes,expected)){++bundleReport.h283ClassAlready;return s.killerClass=true;}
        if(!NativeEnvironmentAllowed())return refuse(4);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{state,source})||!NativeEnvironmentAllowed()||!BeginBundleNative())return refuse(5);
#endif
        ++calls_; // ownership of the by-value packet transfers at call entry (empty subpath: no CFG-owned FString)
        const bool returned=cfgmenu::promotion::detail::Two(plan.module+d::ConsumeActive,state,&packet);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!returned){BundleRetentionFailed();return refuse(6);}
        if(!FinishBundleNative())return refuse(7);
#else
        if(!returned)return refuse(6);
#endif
        if(!AdditionalWorld()||!r_.Read(state+d::ActiveCharacterClass,again)||!cfgending::SamePath(again.bytes,expected))return refuse(8);
        ++bundleReport.h283ClassApplied;cfgclosedtrace::Mark("h283-f1-class-applied",state,source);return s.killerClass=true;
    }
#endif
    bool AdditionalPossessCall(std::uintptr_t controller,std::uintptr_t pawn,cfgnativejason::Fault& fault){
        if(!NativeEnvironmentAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(pawn)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{controller})||!NativeEnvironmentAllowed()||!BeginBundleNative())return false;
#endif
        const bool returned=cfgnativejason::detail::Possess(plan.module+cfgnativejason::disk::BasePossess,controller,pawn,fault);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!returned||fault.code){BundleRetentionFailed();return false;}
        return FinishBundleNative();
#else
        return returned;
#endif
    }








#include "AdditionalJasonSelectionMembers.hpp"
    // Restore only changes attributable to positively journaled extra objects.
    // This runs BEFORE Context() and before yielding after every native mutation.

    bool AdditionalNativeSpawn(std::uintptr_t type,cfgspawn::Vec3 at,std::uintptr_t& output,std::uint8_t collision=2){
        cfgnativejason::Fault fault;cfgspawn::Vec3 rotation{};
        if(!NativeCallAllowed()||!cfgnativejason::SpawnCode(r_,plan.module)||!AdditionalPrimary()||
           !AdditionalInProgress()||!AdditionalLifecycle()||r_.reads>cfgbundle::ReadCap-600)return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(type)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{
            bundleAdditional.actor.pawn.object,bundleAdditional.actor.killerController.object})||!NativeCallAllowed()||!BeginBundleNative(1))return false;
#endif
        ++calls_;
        bundleAdditional.allocationAttempted=true;
        const bool returned=cfgnativejason::detail::Spawn(plan.module+cfgmenu::populationdisk::SpawnParametersConstructor,
            plan.module+cfgmenu::populationdisk::SpawnActor,plan.world,type,&at,&rotation,output,fault,collision);
        if(fault.code){bundleAdditional.journal.uncertain=true;cfgclosedtrace::Mark("f1-native-spawn-fault",fault.address,fault.code);}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!returned||fault.code){BundleRetentionFailed();return false;}
        // H2.56: SpawnActor returned no actor (live hosted F1). Nothing was created, so close the window and refuse. Retaining a null
        // output poisoned the root bank, which stopped the AI Jason and ended the match. Same handling as a null ChangeRootMenu result.
        if(!output){cfgclosedtrace::Mark("f1-native-spawn-null",type,collision);(void)FinishBundleNative();return false;}
        if(!RetainBundleReturnedObject(output)){bundleAdditional.journal.uncertain=true;return false;}
#endif
        return returned;
    }
    bool AdditionalNavigation(){
        auto& e=bundleAdditional.actor;cfgcrowd::Observation before{},after{};
        if(!AdditionalLinked()||!cfgcrowd::Observe(r_,plan.module,context.objectArray,e.path,e.killerController.object,before)||!cfgcrowd::Idle(before))return false;
        if(cfgcrowd::OrdinaryReady(before))return true;
        if(calls_>=cfgbundle::CallCap||!cfgbundle::Code(r_,plan.module,cfgcrowd::Setter,cfgcrowd::SetterBytes)||
           !cfgbundle::Code(r_,plan.module,cfgcrowd::CrowdFollow,cfgcrowd::FollowBytes)||!Context()||!AdditionalLinked())return false;
        const bool returned=AdditionalCrowdCall(e.path.object);
        const bool restored=AdditionalRestorePrimary();
        return returned&&restored&&cfgcrowd::Observe(r_,plan.module,context.objectArray,e.path,e.killerController.object,after)&&
            cfgcrowd::Idle(after)&&cfgcrowd::OrdinaryReady(after)&&after.kind==before.kind&&AdditionalLinked();
    }
#include "AdditionalNativeCallRuntimeMembers.hpp"


    bool AdditionalRetireGeneric(){
        const auto& s=bundleAdditional;const auto& controller=s.actor.initialController;std::uintptr_t function=0,owner=0;
        if(!controller.object)return AdditionalLinked()&&AdditionalPrimary(); // direct donor birth has no generic controller
        using namespace cfgadditional::disk;
        if(!AdditionalLinked()||!AdditionalLifecycle()||!cfgjason::SameSlot(r_,context.objectArray,controller)||
           !cfgobserver::Same(r_,controller.object+cfgprobe::ObjectClass,s.selected.defaultAI)||
           !cfgobserver::Same(r_,controller.object+0x370,std::uintptr_t{})||
           !FeatureFunction(Destroy,function,owner)||!cfgspawn::ObjectIs(r_,controller.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+DestroyExecutor)||
           !cfgbundle::Code(r_,plan.module,DestroyExecutor,DestroyCode)||!AdditionalPrimary()||r_.reads>cfgbundle::ReadCap-400)return false;
        // Mark consumed before destruction; failure cleanup must not redispatch it.
        bundleAdditional.genericDestroyAttempted=true;
        const bool returned=AdditionalInvoke(controller.object,function,nullptr);
        const bool restored=AdditionalRestorePrimary();return returned&&restored;
    }

    bool AdditionalSoundPending(){return !bundleAdditional.sound.Attempted();}
    bool AdditionalSoundPhase(){
        auto& s=bundleAdditional;const auto pawn=s.actor.pawn.object;
        if(!Context()||!AdditionalLinked())return false;
        if(!s.sound.Attempted()){
            cfgsoundblip::Scope scope;std::uintptr_t human=0;
            if(!Known(0,human)||!r_.Read(pawn+cfgprobe::ObjectClass,scope.killerClass)||
               !r_.Read(s.actor.killerController.object+cfgprobe::ObjectClass,scope.controllerClass))return false;
            scope.active=scope.offlineCounselor=true;scope.module=plan.module;scope.viewport=plan.viewport;
            scope.world=plan.world;scope.mode=plan.mode;scope.humanController=plan.controller;scope.humanPawn=human;
            scope.killer=s.actor.pawn;scope.controller=s.actor.killerController;
            s.soundScope=scope;cfgsoundblip::NativeAccess<Reader,BundleAccess> access(r_,*this,scope,
                cfgprimary::LookupSafe(r_,plan.module),GetCurrentThreadId()==plan.threadId);
            s.sound.Apply(access);return true; // separate extra visibility/emitter transaction, no primary scope change
        }
        return false;
    }
#if defined(CFG_H1_JASON)&&defined(CFG_RETAINED_GAMEPLAY_BUNDLE)&&defined(CFG_FRONTEND_BUNDLE)&&CFG_H263_F1_ROSTER_TARGETS
    // H2.63: the registry below only knows the host and one native target online. 0 = not applicable, 1 = target set, 2 = try next turn.
    unsigned HostedF1RosterTarget(std::uint64_t now,const cfgspawn::Vec3& from,std::uintptr_t& target){
        auto& s=bundleAdditional;auto& shared=bundleDonorRoster;auto& roster=shared.roster;bool alive=false;
        if(!HostedRoutineActive()||!shared.censusComplete||!(shared.world==plan.objects[0])||roster.count<=0)return 0;
        const unsigned total=(std::min)(static_cast<unsigned>(roster.count),64u);
        if(now>=s.targets.next&&r_.reads<cfgbundle::ReadCap-350){
            if(s.targets.count!=total){s.targets={};s.targets.count=total;}
            const auto candidate=roster.actors[s.targets.cursor];cfgspawn::Vec3 point{};
            const bool valid=candidate&&!roster.DonorIsJasonAITargetBlocked(candidate,now)&&Alive(candidate,alive)&&alive&&Location(candidate,point);
            const float dx=point.x-from.x,dy=point.y-from.y,dz=point.z-from.z,flat=dx*dx+dy*dy;
            s.targets.Sample(total,valid,cfgj22::F1TargetScore(candidate==roster.LocalCounselorTarget,flat+dz*dz,flat),now);s.cursor=s.targets.selected;
        }
        if(s.cursor>=total)s.cursor=0;
        target=roster.actors[s.cursor];alive=false;
        if(!target||roster.DonorIsJasonAITargetBlocked(target,now)||!Alive(target,alive)||!alive){s.cursor=(s.cursor+1)%total;return 2;}
        if(target!=s.hostedLastTarget&&s.hostedTargetMarks<64){s.hostedLastTarget=target;++s.hostedTargetMarks;
            cfgclosedtrace::Mark("f1-h263-fallback-target",target,target==roster.LocalCounselorTarget?1u:0u);}
        return 1;
    }
#endif
    bool AdditionalDrive(std::uint64_t now,bool interactionsOnly=false){
        auto& s=bundleAdditional;const auto pawn=s.actor.pawn.object;
        if(!Context()||!AdditionalLinked())return false;
        if(AdditionalSoundPhase())return true;
        // Owed release before match/animation/target gates, like the donor.
        if(s.policy.pressed&&now>=s.policy.releaseAt){
            if(!cfgbundle::ReleaseBinding(r_,plan.module,pawn))return false;
            s.policy.pressed=false;return AdditionalInput(true);
        }
        if(!AdditionalInProgress()||bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady){
            if(!s.stopped){s.stopped=true;return CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,
                cfgtrap::disk::StopMovementCode,s.actor.killerController.object,nullptr);}return true;
        }
        s.stopped=false;bool living=false,stunned=false,attacking=false;std::uintptr_t held=0;
        if(!s.rage.attempted&&RageDifficultyPhase(s.rage,s.actor.pawn,true))return true;
        if(!cfgbundle::Alive(r_,pawn,living)||!living||!cfgbundle::NativeBindings(r_,plan.module,pawn)||
           !r_.Read(pawn+cfghunter::disk::Held,held)||
           !AdditionalBool(cfghunter::disk::Stunned,stunned)||!AdditionalBool(cfghunter::disk::Attacking,attacking))return false;
        if(stunned||attacking||s.policy.pressed)return true; // native paired/pocket-knife animation owns pawn
        s.grab.Observe(held,now);
        if(held){
            if(DonorHeldDecisionInstalled())return true;
            if(!s.grab.Due(now))return true;
            // Same native eligibility and four equipped slots as primary,
            // but no borrowed global hold/input state and no seated kill.
            std::uintptr_t seat=0;cfgescape::State state{};
            if(!r_.Read(held+cfgvehicleoccupancy::disk::CurrentSeat,seat)||seat||
               !cfgescape::Counselor(r_,context.objectArray,plan.module,held,bundleCounselorClass,state)||
               state!=cfgescape::State::Living)return true;
            std::uint8_t killing=0,allowed=0;
            using namespace cfggrab::disk;
            if(!CarCall(IsGrabKilling,IsGrabKillingExecutor,IsGrabKillingCode,pawn,&killing)||killing>1||
               !CarCall(CanGrabKill,CanGrabKillExecutor,CanGrabKillCode,pawn,&allowed)||allowed>1)return false;
            if(killing||!allowed)return true;
            struct Packet {std::int32_t slot;std::uint8_t result=0,pad[3]{};} packet{static_cast<std::int32_t>(s.grab.slot)};
            if(!CarCall(IsGrabKillAvailable,IsGrabKillAvailableExecutor,IsGrabKillAvailableCode,pawn,&packet)||packet.result>1)return false;
            if(!packet.result){s.grab.Refused(now);return true;}
            if(!AdditionalLinked()||!cfgobserver::Same(r_,pawn+cfghunter::disk::Held,held)||
               !cfgobserver::Same(r_,held+cfgvehicleoccupancy::disk::CurrentSeat,std::uintptr_t{})||
               !cfgbundle::Code(r_,plan.module,cfggrab::disk::GrabKillInput,GrabKillInputCode)||calls_>=cfgbundle::CallCap)return false;
            return AdditionalGrabInput(s.grab.slot);
        }
        if(interactionsOnly)return false; // native donor root owns all on-foot chase/combat
        // Same known native counselor registry as primary. One fresh candidate
        // per slice, independent target/path/cooldown, no primary-state swapping.
        std::uintptr_t target=0;cfgspawn::Vec3 from{},to{};
        if(!Location(pawn,from))return false;
        unsigned rosterRoute=0;
#if defined(CFG_H1_JASON)&&defined(CFG_RETAINED_GAMEPLAY_BUNDLE)&&defined(CFG_FRONTEND_BUNDLE)&&CFG_H263_F1_ROSTER_TARGETS
        rosterRoute=HostedF1RosterTarget(now,from,target);if(rosterRoute==2)return true;
#endif
        if(!rosterRoute&&now>=s.targets.next&&r_.reads<cfgbundle::ReadCap-350){
            // Restart before sampling when F3 changes the registry size.
            const unsigned targetCount=bundleReport.bots+2; // local + F3 bots + native roster target
            if(s.targets.count!=targetCount){s.targets={};s.targets.count=targetCount;}
            const unsigned logical=s.targets.cursor;
            const unsigned candidateIndex=logical>bundleReport.bots?BundleNativeTargetIndex:logical;
            std::uintptr_t candidate=0;cfgspawn::Vec3 point{};bool alive=false;
            const bool valid=Known(candidateIndex,candidate)&&Alive(candidate,alive)&&alive&&Location(candidate,point);
            const float dx=point.x-from.x,dy=point.y-from.y,dz=point.z-from.z;
            s.targets.Sample(targetCount,valid,dx*dx+dy*dy+dz*dz,now);s.cursor=s.targets.selected;
        }
        if(!rosterRoute){const unsigned currentIndex=s.cursor>bundleReport.bots?BundleNativeTargetIndex:s.cursor;
        if(!Known(currentIndex,target)||!Alive(target,living)||!living){s.cursor=(s.cursor+1)%(bundleReport.bots+2);return true;}}
        if(AdditionalRun(now))return true;
        if(!Location(target,to))return false;
        const float x=to.x-from.x,y=to.y-from.y,z=to.z-from.z,squared=x*x+y*y+z*z;
        if(!std::isfinite(squared))return false;
        if(squared<260.f*260.f){bool allowed=false;
            if(!s.policy.CombatDue(now))return true;
            if(!AdditionalBool(cfghunter::disk::CanAttack,allowed))return false;
            if(!allowed)return true;
            cfgspawn::Vec3 rotation{};rotation.y=std::atan2(y,x)*57.2957795f;
            if(!CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,
                cfgtrap::disk::SetControlRotationCode,s.actor.killerController.object,&rotation)||!AdditionalLinked())return false;
            s.policy.CombatDispatched(now);
            return AdditionalInput(false);
        }
        if(s.path.Preserve(now,target)||!s.move.Due(now))return true;
        cfgspawn::MoveParameters packet;packet.goal=target;cfgprobe::lifetime::SlotObservation function{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.functions[3],function)||!(function==spawnAssets.functionSlots[3])||
           !cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[3].cache,spawnAssets.functions[3])||
           !cfgobserver::Same(r_,spawnAssets.functions[3]+0x20,spawnAssets.resources.classes[cfgspawn::BaseAI])||
           !AdditionalLinked()||!Invoke(s.actor.killerController.object,spawnAssets.functions[3],&packet))return false;
        s.move.Completed(now,packet.result);s.path.Completed(now,target,packet.result);return true;
    }
#if CFG_H267_F1_BURST&&defined(CFG_RETAINED_GAMEPLAY_BUNDLE)&&defined(CFG_FRONTEND_BUNDLE)
    // H2.67: the primary's burst policy on the F1 pawn, with his own movement fields, cooldown and turn.
    bool AdditionalBurstMovement(){
        auto& s=bundleAdditional;const auto pawn=s.actor.pawn.object;const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        std::uintptr_t movement=0,type=0;cfgprobe::lifetime::SlotObservation slot{};
        return Context()&&AdditionalLinked()&&r_.Read(pawn+cfghook::disk::MovementComponent,movement)&&
            NativeClass(binding.reflection,binding.constructor,type)&&cfgspawn::ObjectIs(r_,movement,type)&&cfgobserver::Same(r_,movement+0x20,pawn)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,movement,slot)&&cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)&&
            ((!s.burst.Owned()&&(s.burstMovement=slot,true))||slot==s.burstMovement);
    }
    bool AdditionalBurstField(unsigned index,float expected,float desired,bool restoring){
        static constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        auto& s=bundleAdditional;float current=0;
        if(index>=4||!AdditionalBurstMovement()||!r_.Read(s.burstMovement.object+fields[index],current)||!std::isfinite(current))return false;
        if(restoring&&current!=expected)return true; // a later foreign value is not ours
        if(current!=expected||!cfgprimary::LookupSafe(r_,plan.module)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,s.burstMovement)||
           !cfgobserver::Same(r_,s.actor.pawn.object+cfghook::disk::MovementComponent,s.burstMovement.object))return false;
        return CarSpeedWrite(s.burstMovement.object+fields[index],expected,desired);
    }
    bool AdditionalBurstObserve(std::array<float,4>& values){
        static constexpr std::uintptr_t fields[]{cfghook::disk::Walk,cfghook::disk::Sprint,cfghook::disk::Run,cfghook::disk::SlowRun};
        auto& s=bundleAdditional;const auto pawn=s.actor.pawn.object;const auto target=bundleDonorAdditional.additional.Target;
        std::uintptr_t held=0,water=0,seat=0;bool alive=false,stunned=false,attacking=false;cfgspawn::Vec3 from{},to{};
        if(!AdditionalBurstMovement()||!target||!Alive(target,alive)||!alive||!r_.Read(pawn+cfghunter::disk::Held,held)||held||
           !r_.Read(pawn+cfgswim::disk::WaterVolume,water)||water||!r_.Read(target+cfgvehicleoccupancy::disk::CurrentSeat,seat)||seat||
           !Location(pawn,from)||!Location(target,to))return false;
        const float dx=to.x-from.x,dy=to.y-from.y,dz=to.z-from.z,squared=dx*dx+dy*dy+dz*dz;
        s.burstSquared=squared; // H2.85 chase recovery reads it after Observe
        if(!std::isfinite(squared)||squared<400.f*400.f||!AdditionalBool(cfghunter::disk::Stunned,stunned)||stunned||
           !AdditionalBool(cfghunter::disk::Attacking,attacking)||attacking)return false;
        for(unsigned i=0;i<4;++i)if(!r_.Read(s.burstMovement.object+fields[i],values[i]))return false;
        return AdditionalBurstMovement();
    }
    bool AdditionalBurstService(std::uint64_t now){
        struct Host {BundleAccess& a;
            bool Observe(std::array<float,4>& values){return a.AdditionalBurstObserve(values);}
            bool Exchange(unsigned i,float before,float after){return a.AdditionalBurstField(i,before,after,false);}
            bool Restore(unsigned i,float before,float after){return a.AdditionalBurstField(i,before,after,true);}
        }host{*this};
        auto& s=bundleAdditional;const bool stopped=bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady;
        if(s.burst.Owned()){if(!s.burst.Due(now,stopped))return false;s.burst.Step(host,now,stopped);return true;}
        const auto target=bundleDonorAdditional.additional.Target;
        if(stopped||!target||now<s.burst.readyAt||now<s.burstCheckAt||!AdditionalInProgress())return false;
        s.burstCheckAt=now+250;s.burstSquared=-1.f;const bool began=s.burst.Begin(host,now,target,true,&s.burstSquared);
        bundleReport.h285Starts[1]=s.burst.starts;bundleReport.h285ChaseStarts[1]=s.burst.chaseStarts;
        if(s.burst.Owned()&&s.burstMarks<32){++s.burstMarks;cfgclosedtrace::Mark("f1-h267-burst",target,s.burst.starts);}
        return began&&s.burst.Owned();
    }
#else
    bool AdditionalBurstService(std::uint64_t){return false;}
#endif
    bool AdditionalRun(std::uint64_t now){
        auto& s=bundleAdditional;
        using namespace cfgrun::disk;
        unsigned char running=0;std::uintptr_t water=0,function=0,owner=0;
        if(!r_.Read(s.rawPawn+Running,running)||!r_.Read(s.rawPawn+cfgswim::disk::WaterVolume,water)||water)return false;
        if(now>=s.run.stateAt&&!running){
            s.run.stateAt=now+250;
            if(!FeatureFunction(ServerSetRunning,function,owner)||!cfgspawn::ObjectIs(r_,s.rawPawn,owner)||
               !cfgobserver::Same(r_,function+0xB0,plan.module+Executor)||
               !cfgbundle::Code(r_,plan.module,Executor,ExecutorCode)||
               !cfgbundle::Code(r_,plan.module,Implementation,ImplementationCode)||
               !cfgbundle::Code(r_,plan.module,SetBit,SetBitCode)||!AdditionalLinked()||!AdditionalPrimary()||
               !cfgescape::Same(r_,s.rawPawn+Running,std::uint8_t{}))return false;
            std::uint8_t desired=1;const bool returned=AdditionalInvoke(s.rawPawn,function,&desired);
            const bool restored=AdditionalRestorePrimary();
            if(!returned||!restored)cfgclosedtrace::Mark("f1-run-refused",s.rawPawn,Reads());return true;
        }
        if(now<s.run.next)return false;s.run.next=now+1000;
        std::uintptr_t movement=0,type=0;cfgprobe::lifetime::SlotObservation slot{};float walk=0,run=0,desired=0;
        const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        if(!r_.Read(s.rawPawn+cfghook::disk::MovementComponent,movement)||!NativeClass(binding.reflection,binding.constructor,type)||
           !cfgspawn::ObjectIs(r_,movement,type)||!cfgobserver::Same(r_,movement+0x20,s.rawPawn)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,movement,slot)||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
           !r_.Read(movement+cfghook::disk::Walk,walk)||!r_.Read(movement+cfghook::disk::Run,run)||
           !std::isfinite(run)||run<25.f||run>2500.f||
           !cfgrun::Desired(walk,(std::max)(run,cfgrun::MinimumPursuitRun),desired))return false;
        if(!AdditionalLinked()||!AdditionalPrimary()||!cfgjason::OwnedComponentSlot(r_,context.objectArray,slot)||
           !cfgobserver::Same(r_,s.rawPawn+cfghook::disk::MovementComponent,movement)||
           !cfgescape::Same(r_,movement+cfghook::disk::Run,run))return false;
        for(const auto field:{cfghook::disk::Run,cfghook::disk::Sprint,cfghook::disk::SlowRun}){
            float current=0;if(!r_.Read(movement+field,current)||!std::isfinite(current)||current<25.f||current>2500.f)return false;
            if(current<desired){s.run.next=now+100;return CarSpeedWrite(movement+field,current,desired);}
        }
        return desired!=walk&&CarSpeedWrite(movement+cfghook::disk::Walk,walk,desired);
    }


    bool AdditionalGrabInput(unsigned slot)noexcept{
        if(slot>=4||!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        std::uintptr_t held=0;
        if(!RetainBundleActors(bundleAdditional.actor.pawn.object)||
           !r_.Read(bundleAdditional.actor.pawn.object+cfghunter::disk::Held,held)||
           !RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{held})||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;bundleAdditional.grab.Commit();
        __try{reinterpret_cast<void(__fastcall*)(void*,std::int32_t)>(plan.module+cfggrab::disk::GrabKillInput)(
            reinterpret_cast<void*>(bundleAdditional.actor.pawn.object),static_cast<std::int32_t>(slot));}
        __except(EXCEPTION_EXECUTE_HANDLER){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            bundleAdditional.journal.uncertain=true;BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
#else
    bool AdditionalLinked(){return false;}
    bool AdditionalPhase(std::uint64_t){return false;}
#endif
