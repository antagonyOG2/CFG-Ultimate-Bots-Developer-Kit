// Included after ServiceQueuedGameplay: exact native entry, own returned-call
// lifetime provider, then donor work. Does not reuse a prior ReceiveTick proof.
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
class NativeJasonControllerTickSession {
    cfgjasontick::Owner owner_{};bool callback_=false;
    GcRegistryAccess::NativeContextProof proof_{};
    std::uintptr_t savedActor_=0,savedFunction_=0,savedReturn_=0;
    const GcRegistryAccess::NativeContextProof* previous_=nullptr;
public:
    explicit NativeJasonControllerTickSession(const cfgjasontick::Owner& owner)noexcept:owner_(owner){
        // caller constructs ONLY after original or verified base Tick returns, on the
        // independently validated owner thread. No nested/foreign/root borrow.
        if(bundleNativeJasonTickProof||bundleRootLease||!bundleNativeJasonTickDepth||
           GetCurrentThreadId()!=owner.source.threadId)return;
        Reader r;r.cap=(owner_.adoption.valid||HostedAdoptionOwnsDriver())?4096:512;if(!Current(r))return;
        savedActor_=primaryRootTickActor;savedFunction_=primaryRootTickFunction;savedReturn_=primaryRootTickReturn;
        proof_={this,[](Reader& reader,void* value){return static_cast<NativeJasonControllerTickSession*>(value)->Current(reader);},
            [](Reader& reader,void* value,std::uintptr_t function,std::uintptr_t pc){
                auto& self=*static_cast<NativeJasonControllerTickSession*>(value);
                return self.callback_&&function==self.owner_.actor.killerController.object&&
                    pc==self.owner_.source.module+cfgcompat::cfg35::j1a::KillerControllerTick&&self.Current(reader);}};
        // Marker is the proved controller UObject, not code cast to UObject.
        // GcRegistryAccess uses proof_ rather than ReceiveTick decoding.
        primaryRootTickActor=owner_.actor.killerController.object;
        primaryRootTickFunction=owner_.actor.killerController.object;
        primaryRootTickReturn=owner_.source.module+cfgcompat::cfg35::j1a::KillerControllerTick;
        previous_=bundleNativeJasonTickProof;callback_=true;bundleNativeJasonTickProof=&proof_;
    }
    ~NativeJasonControllerTickSession(){
        if(callback_){bundleNativeJasonTickProof=previous_;primaryRootTickActor=savedActor_;
            primaryRootTickFunction=savedFunction_;primaryRootTickReturn=savedReturn_;}
    }
    bool Entered()const noexcept{return callback_;}
    template<class R>bool Current(R& r)noexcept{
        const auto& p=owner_.source;
        return bundleNativeJasonTickDepth==1&&GetCurrentThreadId()==p.threadId&&plan.module==p.module&&
            plan.world==p.world&&plan.mode==p.mode&&plan.controller==p.controller&&plan.pawn==p.pawn&&
            plan.objects==p.objects&&cfgjasontick::ObserveOwner(r,owner_,GetCurrentThreadId())&&
            BundleNativeJasonControllerTickCodeCurrent(p.module);
    }
};
bool ResolveNativeJasonTickOwner(std::uintptr_t controller,cfgjasontick::Owner& owner)noexcept{
    owner={};owner.source=plan;
    if(!bundleNativeJasonDonorService||!(bundleStarted||HostedAdoptionOwnsDriver()))return false;
    const bool primaryOwed=bundleDonorPrimary.bound&&bundleDonorPrimary.combat.AttackPressed;
    const bool additionalOwed=bundleDonorAdditional.bound&&bundleDonorAdditional.additional.AttackPressed;
    if(!bundleSchedule.active&&!HostedAdoptionOwnsDriver()&&!primaryOwed&&!additionalOwed&&!J21LeanOwedCleanup())return false;
    owner.killerBase=spawnAssets.killerBase;owner.killerAI=spawnAssets.resources.classes[cfgspawn::KillerAI];
    owner.pathClass=spawnAssets.classes[3];
    if(controller==bundleJason.killerController.object){
        const auto& model=JasonTemplate();owner.actor=bundleJason;owner.pawnClass=model.pawnClass;owner.defaults=model.defaults;if(HostedAdoptionOwnsDriver()){owner.adoption=hostedAdoptionReceipt;}
    }else if(controller==bundleAdditional.actor.killerController.object&&
             (bundleAdditional.policy.stage==cfgadditional::Stage::Active||additionalOwed)){
        owner.actor=bundleAdditional.actor;owner.pawnClass=bundleAdditional.selected.pawnClass;owner.defaults=bundleAdditional.selected.defaults;
    }else return false;
    return true; // routing hint only; ObserveOwner is still mandatory
}
void ServiceNativeJasonControllerTick(const cfgjasontick::Owner& owner)noexcept{
    // Called only AFTER this known actor's original or proved base Tick returned.
    if(!bundleNativeJasonDonorService)return;
    J21Timing nativeTiming(cfgj21::Native);J21CallbackMemoScope memo(owner.source.world);
    {const auto& d=bundleRootDiagnostics;auto& r=bundleReport; // H2.91 timing only: publish the callback root-window costs
     r.h291RootWindows=d.windows;r.h291RootWindowUs=d.windowUs;r.h291RootWindowMaxUs=d.windowMaxUs;r.h291RootSleeps=d.sleeps;r.h291RootSleepUs=d.sleepUs;
     r.h291RootBeginUs=d.beginUs;r.h291RootOperationUs=d.operationUs;r.h291RootEndUs=d.endUs;r.h291RootRetries=d.retries;r.h292RootProvenSkips=d.provenSkips;}
    J21Timing sessionTiming(cfgj21::NativeSessionEntry);
    NativeJasonControllerTickSession session(owner);sessionTiming.Stop();if(!session.Entered()){
#ifdef CFG_H1_JASON
        if(owner.adoption.valid){++hostedDriverTrace.ownerRefusals;hostedDriverTrace.stage=5;}
#endif
        DonorNativeRefused(owner.actor.killerController.object,cfgdonorhunt::DriverRefusal::Owner);return;}
    const auto current=[](Reader& r,void* value){return static_cast<NativeJasonControllerTickSession*>(value)->Current(r);};
    J21Timing lookupTiming(cfgj21::NativeLookupScope);
    cfgprimary::ScopedLookupEligibility<Reader> scope(plan.module,&session,current,true);lookupTiming.Stop();
    if(!scope.Entered()){DonorNativeRefused(owner.actor.killerController.object,cfgdonorhunt::DriverRefusal::Owner);return;}
    J21Timing objectiveTiming(cfgj21::NativeObjectiveScope);
    cfgprimary::ScopedLookupEligibility<cfgobjective::Budget<Reader>> objectiveScope(plan.module,&session,
        [](cfgobjective::Budget<Reader>& r,void* value){return static_cast<NativeJasonControllerTickSession*>(value)->Current(r);},true);
    objectiveTiming.Stop();
    if(!objectiveScope.Entered()){DonorNativeRefused(owner.actor.killerController.object,cfgdonorhunt::DriverRefusal::Owner);return;}
    J21Timing rootsTiming(cfgj21::NativeRootScope);
    BundleCallbackRootScope roots(plan.module);rootsTiming.Stop();
    J21Timing retirementTiming(cfgj21::NativeRetirement);
    const bool rootsReady=roots.Entered()&&DrainBundleDeferredRetirement();retirementTiming.Stop();if(!rootsReady){
        DonorNativeRefused(owner.actor.killerController.object,cfgdonorhunt::DriverRefusal::Retention);return;}
    if(!CFG_J21C_LAZY_INPUT_ROOTS){
        Reader retain;retain.cap=256;
        J21Timing retainTiming(cfgj21::NativeRetain);
        const bool retained=RetainPrimaryObjects(retain,std::array<std::uintptr_t,3>{owner.actor.pawn.object,
            owner.actor.killerController.object,owner.actor.path.object});retainTiming.Stop();if(!retained){
            DonorNativeRefused(owner.actor.killerController.object,cfgdonorhunt::DriverRefusal::Retention);return;}
    }
    J21Timing serviceTiming(cfgj21::NativeDonorService);
    #ifdef CFG_H1_JASON
    if(owner.adoption.valid){++hostedDriverTrace.services;hostedDriverTrace.stage=8;}
#endif
#ifdef CFG_H1_JASON
    if(owner.adoption.valid&&hostedClassRestore.pending&&hostedClassRestore.due){
        // Outro can stop the local pawn's ReceiveTick before travel. This is
        // the SAME exact returned controller lease, not a borrowed PE callback.
        BundleAccess restore;(void)restore.HostedRestoreClassAtEnd();return;
    }
#if CFG_H247_HOST_TAKEOVER&&CFG_H253_HOST_VIEW_REPAIR
    if(owner.adoption.valid&&hostedViewRepair.pending){
        const auto now=GetTickCount64();
        if(now>=(hostedViewRepair.done?hostedViewRepair.finalAt:hostedViewRepair.earlyAt)){
            BundleAccess view;view.HostedViewRepairService(now);return;
        }
    }
#endif
#endif
    bundleNativeJasonDonorService(owner); // exact3.5 host, not the old ProcessEvent hunt lane
}
struct J21NativeHints {bool water=true,known=false,useBase=false;};
J21NativeHints ObserveJ21NativeHints(const cfgjasontick::Owner& owner,bool owned,std::uintptr_t controller,float delta){
    if(!owned)return {};
    J21Timing timing(cfgj21::Hints);J21NativeHints result;
    const bool primary=owner.actor.pawn==bundleJason.pawn;
    const bool primaryAlive=primary?J21AdmittedAlive(owner.actor.pawn):J21OwnedAlive(bundleJason.pawn);
    const bool extraAlive=primary?(bundleAdditional.policy.stage==cfgadditional::Stage::Active&&J21OwnedAlive(bundleAdditional.actor.pawn)):
        J21AdmittedAlive(owner.actor.pawn);
    bundleJ21Budget.Tick(plan.world,primary?0u:1u,primaryAlive,delta);bundleJ21Budget.bothAlive=primaryAlive&&extraAlive;
    bundleJ21LastControllerAt=primaryAlive||extraAlive?GetTickCount64():0; // dead controller ticks must not block terminal PE fallback
    result.known=owned&&DonorHeldWaterHint(owner,result.water);
    result.useBase=result.known&&cfgdonorturn::UseBaseTick(DonorNativeDriverReady(controller),
        DonorHeldTickHint(owner,result.known,result.water),DonorHeldStopTickHint(owner),result.water);
    return result;
}
void __fastcall BundleNativeJasonControllerTick(void* controller,float deltaSeconds){
    const auto original=bundleNativeJasonControllerTickOriginal;
    if(!original)return; // never enabled without its resident trampoline
    // ACTIVE donor: owned controllers receive inherited/base Tick, THEN the
    // donor driver. Others retain complete original Tick. Never run both.
    const bool previousBase=bundleNativeJasonTickUsedBase,previousWaterKnown=bundleNativeJasonWaterKnown,previousWater=bundleNativeJasonWater;
    ++bundleNativeJasonTickDepth;
    __try {
        bool owned=false;cfgjasontick::Owner owner{};
        if(bundleNativeJasonTickDepth==1&&!bundleRootLease&&!bundleNativeJasonTickProof&&GetCurrentThreadId()==plan.threadId&&
           ResolveNativeJasonTickOwner(reinterpret_cast<std::uintptr_t>(controller),owner)){
            Reader proof;proof.cap=HostedAdoptionOwnsDriver()?4096:512;
#ifdef CFG_H1_JASON
            if(owner.adoption.valid){++hostedDriverTrace.ownerChecks;hostedDriverTrace.stage=2;}
#endif
            const bool ownerCurrent=cfgjasontick::ObserveOwner(proof,owner,GetCurrentThreadId());
            owned=ownerCurrent&&cfgbundle::Code(proof,plan.module,cfgcompat::cfg35::j1a::BaseAIControllerTick,
                cfgcompat::cfg35::j1a::BaseAIControllerTickCode)&&BundleNativeJasonControllerTickCodeCurrent(plan.module);
            #ifdef CFG_H1_JASON
            if(owner.adoption.valid){hostedDriverTrace.lastReads=proof.reads;hostedDriverTrace.readCap=proof.cap;
                if(!owned){++hostedDriverTrace.ownerRefusals;hostedDriverTrace.stage=3;}else hostedDriverTrace.stage=4;}
#endif
            if(!owned)DonorNativeRefused(reinterpret_cast<std::uintptr_t>(controller),ownerCurrent?
                cfgdonorhunt::DriverRefusal::Code:cfgdonorhunt::DriverRefusal::Owner);
        }
        const auto hints=ObserveJ21NativeHints(owner,owned,reinterpret_cast<std::uintptr_t>(controller),deltaSeconds);
        const bool water=hints.water,waterKnown=hints.known,useBase=hints.useBase;
        bundleNativeJasonWaterKnown=waterKnown;bundleNativeJasonWater=water;
        bundleJ21Memo.Clear(); // original/base Tick is a native call too
#ifdef CFG_H1_JASON
        if(owned&&owner.adoption.valid){++hostedDriverTrace.ticks;if(useBase)++hostedDriverTrace.baseTicks;hostedDriverTrace.stage=useBase?7:6;}
#endif
        if(owned&&useBase){
            DonorNativeBeginHandoff(reinterpret_cast<std::uintptr_t>(controller),GetTickCount64());
            bundleNativeJasonTickUsedBase=true;
            const auto base=reinterpret_cast<NativeJasonControllerTickOriginal>(plan.module+cfgcompat::cfg35::j1a::BaseAIControllerTick);
            base(controller,deltaSeconds);
            ServiceNativeJasonControllerTick(owner);
        }else{
            if(bundleNativeJasonTickDepth==1)bundleNativeJasonTickUsedBase=false;
            original(controller,deltaSeconds);
            if(owned)ServiceNativeJasonControllerTick(owner); // readonly/initialization handoff; never drive after full original Tick
        }
        if(owned)DonorRecordWaterRoute(owner,waterKnown,!useBase,water); // observed actual route; bounded first/128th/exit trace
    }__finally{--bundleNativeJasonTickDepth;bundleNativeJasonTickUsedBase=previousBase;
        bundleNativeJasonWaterKnown=previousWaterKnown;bundleNativeJasonWater=previousWater;}
}
#endif
