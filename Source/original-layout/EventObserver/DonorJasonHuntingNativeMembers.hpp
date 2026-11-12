// Included inside existing BundleAccess. One native gateway for BOTH owned
// Jasons; the primary singleton/death-state owner is never swapped to F1.
    // H2.74: offline, both Jasons prove their owner on a separate read budget; live H2.73 starved 297 primary teleport probes on the callback's reader.
    bool DonorNativeOwner(const cfgjasontick::Owner& owner){
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            const auto& receipt=hostedAdoptionReceipt;
            auto& trace=hostedDriverTrace;++trace.adoptedReadOwnerChecks;trace.adoptedReadOwnerClause=0;
            const auto refuse=[&](unsigned clause){++trace.adoptedReadOwnerRefusals;trace.adoptedReadOwnerClause=clause;return false;};
            if(!bundleNativeJasonTickProof)return refuse(1);
            if(primaryRootTickActor!=owner.actor.killerController.object)return refuse(2);
            if(owner.source.module!=plan.module||owner.source.world!=plan.world||owner.source.mode!=plan.mode||
               owner.source.threadId!=plan.threadId)return refuse(3);
            if(!cfgprimary::CurrentHostedBinding().Matches(plan)||!cfgprimary::CurrentHostedBinding().Matches(owner.source))return refuse(4);
#if CFG_H263_F1_DONOR_HUNT
            // H2.63: the F1 Jason is not an adopted human, so it takes the same linked-owner proof offline uses (ObserveOwner). The primary keeps its adoption proof below.
            if(!owner.adoption.valid&&owner.actor.pawn.HasAssignedSerial()&&owner.actor.pawn==bundleAdditional.actor.pawn&&
               owner.actor.killerController==bundleAdditional.actor.killerController){
                const auto extra=[&](unsigned clause){auto& s=bundleAdditional;if(s.hostedOwnerMarks<8){++s.hostedOwnerMarks;
                    cfgclosedtrace::Mark("f1-h263-owner-refused",owner.actor.pawn.object,(static_cast<std::uint64_t>(clause)<<32)|Reads());}return false;};
                if(!receipt.Complete())return extra(10);
                if(!NativeEnvironmentAllowed())return extra(11);
                // H2.68: live, 12 of 12 F1 teleport rings and every locked-door press died here or on the probes after it, while the primary's proof has its own budget.
                Reader proof;proof.cap=4096;const bool observed=cfgjasontick::ObserveOwner(proof,owner,GetCurrentThreadId());bundleReport.reads+=proof.reads;
                if(!observed)return extra(12);
                return true;
            }
#endif
            if(!owner.adoption.Complete()||!receipt.Complete())return refuse(5);
            if(!(owner.adoption.pawn==receipt.pawn)||!(owner.adoption.controller==receipt.controller)||
               !(owner.adoption.controllerState==receipt.controllerState)||!(owner.adoption.displacedState==receipt.displacedState)||
               owner.adoption.world!=receipt.world||owner.adoption.mode!=receipt.mode)return refuse(6);
            if(!(owner.actor.pawn==receipt.pawn)||!(owner.actor.killerController==receipt.controller)||
               !(bundleJason.pawn==receipt.pawn)||!(bundleJason.killerController==receipt.controller))return refuse(7);
            if(!cfghosted::adoption::Current(r_,context.objectArray,receipt,plan)){
                const auto why=cfghosted::adoption::CurrentClause(r_,context.objectArray,receipt,plan); // diagnostics only
                trace.h248CurrentClause=why;if(why<trace.h248CurrentClauses.size())++trace.h248CurrentClauses[why];
                if(!trace.h248CurrentFirstFailAt){trace.h248CurrentFirstFailAt=GetTickCount64();trace.h248CurrentFirstClause=why;}
                if(why==7){ // H2.50 diagnostics: what happened to the adopted controller's object slot
                    cfgprobe::lifetime::SlotObservation fresh{};std::uint32_t flags=0;std::uintptr_t pawnController=0;
                    trace.h250CtlReceiptIndex=receipt.controller.index;trace.h250CtlReceiptSerial=receipt.controller.serial;
                    trace.h250ReceiptController=receipt.controller.object;
                    trace.h250CtlReadOk=cfgprobe::lifetime::ReadSlot(r_,context.objectArray,receipt.controller.object,fresh)?1u:0u;
                    trace.h250CtlFreshIndex=fresh.index;trace.h250CtlFreshSerial=fresh.serial;
                    if(trace.h250CtlReadOk&&fresh.index>=0&&r_.Read(context.objectArray.data+static_cast<std::uintptr_t>(fresh.index)*24+8,flags))
                        trace.h250CtlFlags=flags;
                    if(r_.Read(receipt.pawn.object+cfghosted::adoption::PawnController,pawnController))trace.h250PawnController=pawnController;
                }
                return refuse(8);
            }
            trace.h248CurrentLastPassAt=GetTickCount64();
            if(!NativeEnvironmentAllowed())return refuse(9);
            return cfgdonorhunt::SelectNativeOwnerRoute(true,true)==cfgdonorhunt::NativeOwnerRoute::HostedAdoption;
        }
#endif
        if(CFG_H274_OFFLINE_OWNER_BUDGET){Reader proof;proof.cap=4096;const bool observed=bundleNativeJasonTickProof&&primaryRootTickActor==owner.actor.killerController.object&&owner.source.module==plan.module&&owner.source.world==plan.world&&NativeEnvironmentAllowed()&&cfgprimary::RetainedWorld(proof,owner.source,context)&&cfgjasontick::ObserveOwner(proof,owner,GetCurrentThreadId());bundleReport.reads+=proof.reads;return observed;}
        return bundleNativeJasonTickProof&&primaryRootTickActor==owner.actor.killerController.object&&
            owner.source.module==plan.module&&owner.source.world==plan.world&&
            NativeEnvironmentAllowed()&&cfgprimary::RetainedWorld(r_,owner.source,context)&&
            cfgjasontick::ObserveOwner(r_,owner,GetCurrentThreadId());
#else
        (void)owner;return false;
#endif
    }
    bool DonorNativeInput(const cfgjasontick::Owner& owner,bool release,
        cfgdonorhunt::NativeOutcome* outcome=nullptr)noexcept{
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::NotEntered;
        const auto pawn=owner.actor.pawn.object;
        if(!DonorNativeOwner(owner)||!NativeCallAllowed()||
           !(release?cfgbundle::ReleaseBinding(r_,plan.module,pawn):cfgbundle::NativeBindings(r_,plan.module,pawn)))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(pawn)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;if(outcome)*outcome=cfgdonorhunt::NativeOutcome::Uncertain;
        __try{reinterpret_cast<void(__fastcall*)(void*)>(plan.module+(release?cfghunter::disk::Release:cfghunter::disk::Press))(
            reinterpret_cast<void*>(pawn));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const bool finished=FinishBundleNative();
        if(finished&&outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        return finished;
#else
        if(outcome)*outcome=cfgdonorhunt::NativeOutcome::ReturnedTrue;
        return true;
#endif
    }
    bool DonorNativeDriver(const cfgjasontick::Owner& owner)noexcept{
        using namespace cfgcompat::cfg35::j1a;std::uintptr_t table=0,target=0;
        const auto pawn=owner.actor.pawn.object;
        if(!DonorNativeOwner(owner)||!NativeCallAllowed()||!r_.Read(pawn,table)||
           !r_.Read(table+DriverSlot,target)||target!=plan.module+Driver||
           !cfgbundle::Code(r_,plan.module,Driver,DriverCode))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(pawn)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,bool)>(target)(reinterpret_cast<void*>(pawn),true);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
    template<class AttackState>cfgdonorhunt::ReleaseDrain DonorNativeDrainOwedRelease(
        const cfgjasontick::Owner& owner,AttackState& state,std::uint64_t now){
        struct Host {BundleAccess& access;const cfgjasontick::Owner& owner;
            cfgdonorhunt::NativeOutcome ReleaseAttackOutcome(){
                auto outcome=cfgdonorhunt::NativeOutcome::NotEntered;
                access.DonorNativeInput(owner,true,&outcome);return outcome;
            }
        } host{*this,owner};
        return cfgdonorhunt::DonorDrainOwedAttackRelease(host,state,now);
    }
    bool DonorNativeBusyFlags(const cfgjasontick::Owner& owner,std::array<std::uint8_t,4>& flags){
        DonorNativePredicateBegin(owner,6);
        using namespace cfgcompat::cfg35::j1a;
        if(!DonorNativeOwner(owner))return DonorNativePredicateRefuse(owner,1);
        if(!cfgbundle::Code(r_,plan.module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes))return DonorNativePredicateRefuse(owner,2);
        if(!cfgbundle::Code(r_,plan.module,StunSetter,StunSetterCode))return DonorNativePredicateRefuse(owner,3);
        if(!cfgbundle::Code(r_,plan.module,MorphSetter,MorphSetterCode))return DonorNativePredicateRefuse(owner,4);
        if(!cfgbundle::Code(r_,plan.module,ShiftSetter,ShiftSetterCode))return DonorNativePredicateRefuse(owner,5);
        const auto pawn=owner.actor.pawn.object;
        if(!r_.Read(pawn+DriverState0,flags[0]))return DonorNativePredicateRefuse(owner,6);
        if(!r_.Read(pawn+DriverState1,flags[1]))return DonorNativePredicateRefuse(owner,7);
        if(!r_.Read(pawn+MorphActive,flags[2]))return DonorNativePredicateRefuse(owner,8);
        if(!r_.Read(pawn+ShiftActive,flags[3]))return DonorNativePredicateRefuse(owner,9);
        if(!cfgjason::SameSlot(r_,context.objectArray,owner.actor.pawn))return DonorNativePredicateRefuse(owner,10);
        return true;
    }
    bool DonorNativePosition(const cfgprobe::lifetime::SlotObservation& pawn,cfgspawn::Vec3& point){
#if CFG_H284_MORPH_CUTOFF
        const bool proven=DonorNativePositionProof(pawn,point);
        if(proven)bundleCutoff.Note(pawn.object,point.x,point.y,GetTickCount64()); // heading sample for the cut-off Morph; no extra reads
        return proven;
    }
    bool DonorNativePositionProof(const cfgprobe::lifetime::SlotObservation& pawn,cfgspawn::Vec3& point){
#endif
        std::uintptr_t root=0,scene=0;cfgprobe::lifetime::SlotObservation rootSlot{},rootAgain{};cfgspawn::Vec3 again{};
        return NativeEnvironmentAllowed()&&pawn.HasAssignedSerial()&&cfgjason::SameSlot(r_,context.objectArray,pawn)&&
            r_.Read(pawn.object+cfgposition::disk::ActorRootComponent,root)&&
            NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,scene)&&
            cfgspawn::ObjectIs(r_,root,scene)&&cfgspawn::ObjectSlot(r_,context.objectArray,root,rootSlot)&&
            cfgobserver::Same(r_,root+0x20,pawn.object)&&cfgobserver::Same(r_,root+0xE0,pawn.object)&&
            r_.Read(root+cfgposition::disk::SceneWorldTranslation,point)&&cfgspawn::Finite(point)&&
            r_.Read(root+cfgposition::disk::SceneWorldTranslation,again)&&
            point.x==again.x&&point.y==again.y&&point.z==again.z&&
            cfgspawn::ObjectSlot(r_,context.objectArray,root,rootAgain)&&rootAgain==rootSlot&&
            cfgobserver::Same(r_,pawn.object+cfgposition::disk::ActorRootComponent,root)&&
            cfgjason::SameSlot(r_,context.objectArray,pawn);
    }
    bool DonorNativeCounselorContextKill(const cfgprobe::lifetime::SlotObservation& pawn,bool& active){
        active=false;
        return NativeEnvironmentAllowed()&&cfgdonorhunt::ObserveCounselorContextKill(
            r_,context.objectArray,plan.module,pawn,bundleCounselorClass,active);
    }
    bool DonorNativeCounselor(const cfgjasontick::Owner& owner,
        const cfgprobe::lifetime::SlotObservation& pawn,cfgdonorhunt::CounselorObservation& value){
        value={};
        if(!DonorNativeOwner(owner))return false;
        const bool observed=cfgdonorhunt::ObserveDonorCounselor(
            r_,context.objectArray,plan.module,plan.world,pawn,bundleCounselorClass,value);
        DonorRecordRosterObservation(pawn.object,value,observed,GetTickCount64());return observed;
    }
    bool DonorNativeStallObservation(const cfgjasontick::Owner& owner,
        const cfgprobe::lifetime::SlotObservation& target,cfgdonorhunt::Position& jasonPosition,
        cfgdonorhunt::Position& targetPosition,bool& targetLive){
        targetLive=false;cfgdonorhunt::MatchPhase phase=cfgdonorhunt::MatchPhase::Unknown;
        JasonIdentity identity{};
        if(!DonorNativeOwner(owner)||!owner.adoption.valid||!(owner.actor.pawn==bundleJason.pawn)||
           !JasonAdmission(identity)||!identity.hosted||
           !DonorNativeMatchPhase(phase)||phase!=cfgdonorhunt::MatchPhase::InProgress||!target.HasAssignedSerial())return false;
        cfgdonorhunt::CounselorObservation observed{};
        if(!cfgdonorhunt::ObserveDonorCounselor(r_,context.objectArray,plan.module,plan.world,target,
            bundleCounselorClass,observed)||!observed.Live())return false;
        cfgspawn::Vec3 jason{},counselor{};
        if(!DonorNativePosition(owner.actor.pawn,jason)||!DonorNativePosition(target,counselor))return false;
        jasonPosition={jason.x,jason.y,jason.z};targetPosition={counselor.x,counselor.y,counselor.z};
        targetLive=true;return true;
    }
    cfgdonorframe::State DonorNativeDecisionFrame(const cfgdonorframe::Source& source,cfgdonorframe::Frame& frame){
        struct Host {
            BundleAccess& a;
            bool Current(const cfgjasontick::Owner& owner){return a.DonorNativeOwner(owner);}
            bool BudgetExhausted(){return a.r_.reads>=a.r_.cap;}
            bool TargetBudgetAvailable(){return a.r_.reads+256<=a.r_.cap;}
            bool Observe(const cfgprobe::lifetime::SlotObservation& slot,cfgdonorhunt::CounselorObservation& value,
                cfgdonorhunt::Position& point){
                // Current() admitted this owner. These reads perform no native
                // work; do not spend a second whole-owner proof per candidate.
                cfgspawn::Vec3 location{};
                const bool observed=cfgdonorhunt::ObserveDonorCounselor(a.r_,a.context.objectArray,plan.module,plan.world,slot,bundleCounselorClass,value);
                a.DonorRecordRosterObservation(slot.object,value,observed,GetTickCount64());
                if(!observed||!a.DonorNativePosition(slot,location))return false;
                point={location.x,location.y,location.z};return true;
            }
            bool Origin(const cfgprobe::lifetime::SlotObservation& slot,cfgdonorhunt::Position& point){
                cfgspawn::Vec3 value{};if(!a.DonorNativePosition(slot,value))return false;
                point={value.x,value.y,value.z};return true;
            }
        } host{*this};
        return frame.Step(host,source);
    }
