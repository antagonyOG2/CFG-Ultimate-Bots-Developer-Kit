// Included inside BundleAccess. Native-thread offline AI knife lane only.
    bool KnifeGameplayActive()const{
#ifdef CFG_H1_JASON
        return BundleGameplayStarted();
#else
        return bundleStarted;
#endif
    }
    unsigned knifeFailure=UINT32_MAX;
    unsigned KnifeFailureCode()const{return knifeFailure;}
    unsigned KnifeReadCount()const{return r_.reads;}
    unsigned KnifeCallCount()const{return Calls();}
    // Offline primary knife: the first abort in the H2.76 offline capture was Windup, KnifeFailure=1 (owner proved, observe read failed) with
    // reads==ReadCap, so the callback's shared reader was already spent by the lane prelude. Hosted already gives the knife its own budget.
    void KnifeTick(std::uint64_t now){
#if CFG_H277_OFFLINE_KNIFE
        if(!HostedRoutineActive()){
            BundleAccess knife(CFG_H278_OFFLINE_KNIFE_CAP?4096u:cfgbundle::ReadCap,calls_);knife.context=context;ChildAccounting accounting(*this,knife);
            bundleKnife.Tick(knife,now);
            knifeFailure=knife.knifeFailure;exception_=exception_||knife.exception_;
            return;
        }
#endif
        bundleKnife.Tick(*this,now);
    }
    void KnifeLifecycleSample(std::uintptr_t driver,unsigned phase){
        cfgknife::Identity id{};std::uintptr_t current=0,movement=0,owner=0,updated=0,table=0,thrownBy=0;
        std::uint8_t collision=0;
        if(GetCurrentThreadId()!=plan.threadId||!driver||!KnifeIdentity(driver,id)||
           !r_.Read(bundleJason.pawn.object+cfgknife::disk::Driver,current)||current!=driver||
           !r_.Read(driver+0x378,movement)||!movement||!cfgobserver::Same(r_,movement+0x20,driver)||
           !r_.Read(movement+0xF0,updated)||!r_.Read(driver+0x86,collision)||!r_.Read(driver,table)||
           !r_.Read(driver+cfgknife::disk::ThrownBy,thrownBy)||
           !r_.Read(driver+0x20,owner)||!KnifeIdentity(driver,id)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfgknife::disk::Driver,driver))return;
        (void)owner; // diagnostic only; no mutation authority is inferred
        const unsigned value=(collision&2)?2u:1u;
        if(phase==0)bundleReport.knifeAcquiredCollision=value;
        if(phase==1)bundleReport.knifeReleaseCollisionBefore=value;
        if(phase==2)bundleReport.knifeReleaseCollisionAfter=value;
        if(phase==3){++bundleReport.knifeAllOwnedStops;bundleReport.knifeStopStage=static_cast<unsigned>(bundleKnife.stage);}
        bundleReport.knifeLifecycleUpdated=updated;bundleReport.knifeLifecycleThrownBy=thrownBy;
        std::uintptr_t stop=0;if(r_.Read(table+0x608,stop))bundleReport.knifeActualStopTarget=stop;
    }
    bool KnifeNotifyBindings(){
        return bundleKnifeNotifyEnabled&&bundleKnifeNotifyHook.Current(r_)&&bundleKnifeMulticastHook.Current(r_)&&
            bundleKnifeLaunchHook.Current(r_)&&bundleKnifeStopLease.Current(r_);
    }
    bool KnifeCompletionPermission(std::uintptr_t driver){
        std::uint8_t collision=0;const auto* window=bundleKnifeNotifyWindow;
        return window&&window->armed&&window->consumed&&!window->stopped&&!window->launched&&!window->multicast&&
            window->driver.object==driver&&r_.Read(driver+0x86,collision)&&(collision&2)!=0;
    }
    bool KnifeNotifyBefore(std::uintptr_t killer,cfgknife::NotifyWindow& window){
        window={};cfgknife::Snapshot snapshot;std::uintptr_t driver=0,thrownBy=1,table=0;
        std::uint8_t collision=0;
        if(!KnifeGameplayActive()||!bundleSchedule.active||GetCurrentThreadId()!=plan.threadId||
           bundleKill.policy.Exclusive(GetTickCount64())||bundleDeathEnd.confirmedAt||
           killer!=bundleJason.pawn.object||bundleKnife.stage!=cfgknife::Stage::Notify||
           !bundleKnife.nativeCompletionRequired||!KnifeNotifyBindings()||
           !KnifeObserve(bundleKnife.targetIndex,snapshot)||snapshot.jason!=bundleKnife.initial.jason||
           snapshot.target!=bundleKnife.initial.target||!snapshot.targetLiving||
           snapshot.driver!=bundleKnife.release.driver||snapshot.count!=bundleKnife.release.count||
           !KnifeDriver(snapshot.driver,driver,true)||!r_.Read(driver,table)||
           !cfgobserver::Same(r_,table+0x608,plan.module+cfgknife::disk::OnStop)||
           !r_.Read(driver+cfgknife::disk::ThrownBy,thrownBy)||thrownBy||
           !r_.Read(driver+0x86,collision)||(collision&2)==0||!KnifeOwnerLinks())return false;
        cfgspawn::Vec3 source{},target{};float speed=0;
        if(!Location(snapshot.jason.object,source)||!Location(snapshot.target.object,target)||
           !r_.Read(snapshot.jason.object+cfgknife::disk::Speed,speed)||
           !cfgknife::WorldLaunch({source.x,source.y,source.z},{target.x,target.y,target.z},speed,window.geometry)||
           !KnifeOwnerLinks())return false;
        window.geometryReady=true;
        window.jason=snapshot.jason;window.driver=snapshot.driver;window.target=snapshot.target;
        window.count=snapshot.count;window.armed=true;return true;
    }
    bool KnifeNotifyAfter(cfgknife::NotifyWindow& window){
        cfgknife::Snapshot snapshot;std::uintptr_t driver=0,thrownBy=0,table=0;
        std::uint8_t collision=0;
        if(!window.armed||bundleKnife.stage!=cfgknife::Stage::Notify||
           bundleKnife.release.driver!=window.driver||!KnifeNotifyBindings()||
           !KnifeObserve(bundleKnife.targetIndex,snapshot)||!KnifeDriver(window.driver,driver,true)||
           !r_.Read(driver,table)||!cfgobserver::Same(r_,table+0x608,plan.module+cfgknife::disk::OnStop)||
           !r_.Read(driver+cfgknife::disk::ThrownBy,thrownBy)||!r_.Read(driver+0x86,collision)||
           !KnifeOwnerLinks())return false;
        if(!window.Missing(snapshot,thrownBy,(collision&2)!=0))return false;
        window.consumed=true; // consume permission BEFORE corrective native calls
        if(!KnifeLaunch(bundleKnife.targetIndex,window.target,window.driver))return false;
        return bundleKnife.CompleteNativeNotify(window.driver,GetTickCount64());
    }
    bool KnifeCandidate(unsigned index,bool& living,float& squared){
        living=false;squared=0;std::uintptr_t target=0;
        return Context()&&Jason()&&Known(index,target)&&HidingTargetAllowed(target,GetTickCount64())&&
            Alive(target,living)&&(!living||Distance(target,squared));
    }
    bool KnifeHeldStop(std::uintptr_t driver,std::uint64_t now){
        if(!KnifeGameplayActive()||!bundleSchedule.active||!bundleKnifeStopEnabled||
           bundleKnifeStopTarget!=plan.module+cfgknife::disk::OnStop||!bundleKnife.HeldStopCandidate(now)||
           !bundleKnifeStopLease.Current(r_))return false;
        cfgknife::Snapshot fresh{};cfgknife::Identity identity{};std::uintptr_t actual=0,thrownBy=1,table=0;
        std::uint8_t collision=0;
        if(!KnifeObserve(bundleKnife.targetIndex,fresh)||!KnifeIdentity(driver,identity)||!(identity==fresh.driver)||
           !KnifeDriver(identity,actual,true)||actual!=driver||!r_.Read(driver,table)||
           !cfgobserver::Same(r_,table+0x608,plan.module+cfgknife::disk::OnStop)||
           !r_.Read(driver+cfgknife::disk::ThrownBy,thrownBy)||!r_.Read(driver+0x86,collision)||
           !bundleKnife.MaySuppressHeldStop(fresh,driver,thrownBy,(collision&2)!=0,now)||!KnifeOwnerLinks())return false;
        // This is not a launched projectile: no count consumption, release,
        // ThrownBy or collision retirement occurred. Ignore only the premature
        // held-driver stop callback. Real release/flight/impact ALWAYS forwards.
        return cfgobserver::Same(r_,bundleJason.pawn.object+cfgknife::disk::Driver,driver)&&
           cfgobserver::Same(r_,driver+cfgknife::disk::ThrownBy,std::uintptr_t{})&&
           cfgescape::Same(r_,bundleJason.pawn.object+cfgknife::disk::Pressed,std::uint8_t{1})&&
           cfgescape::Same(r_,bundleJason.pawn.object+cfgknife::disk::Count,bundleKnife.initial.count)&&
           bundleKnife.HeldStopCandidate(GetTickCount64());
    }
    bool knifeOwnerProved_=false;
    std::array<std::uintptr_t,3> knifeOwnerClasses_{};
    cfgprobe::lifetime::SlotObservation knifeState_{};
    std::uintptr_t knifeStateClass_=0;
    bool KnifeOwnerLinks(unsigned* detail=nullptr){
        // Credit is confined to THIS BundleAccess callback. Recheck lifetime,
        // offline world, GC/thread, classes and reciprocal controller/state
        // links around calls; do not repeat the full cold inheritance/registry
        // walk three times inside one aim transaction.
        const auto fail=[&](cfgbundle::KnifeOwnerFailure why){
            if(detail)*detail=static_cast<unsigned>(why);return false;};
        cfgobserver::Plan fresh;
        if(!knifeOwnerProved_||GetCurrentThreadId()!=plan.threadId)return fail(cfgbundle::KnifeOwnerFailure::ThreadOrOwnerState);
        if(!cfgprimary::LookupSafe(r_,plan.module))return fail(cfgbundle::KnifeOwnerFailure::LookupSafe);
#ifdef CFG_H1_JASON
        if(!GameplayWorld(fresh))return fail(cfgbundle::KnifeOwnerFailure::GameplayWorld);
#else
        if(!cfgprimary::RetainedOfflineWorld(r_,plan,fresh))return fail(cfgbundle::KnifeOwnerFailure::GameplayWorld);
#endif
        if(!cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))
            return fail(cfgbundle::KnifeOwnerFailure::GameState);
        if(!cfgknife::FreshOwnerLinks(r_,fresh.objectArray,bundleJason,knifeOwnerClasses_,knifeState_,knifeStateClass_))
            return fail(cfgbundle::KnifeOwnerFailure::FreshOwnerLinks);
        return true;
    }
    bool KnifeIdentity(std::uintptr_t object,cfgknife::Identity& id){
        cfgprobe::lifetime::SlotObservation s{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,object,s)||!s.HasAssignedSerial())return false;
        id={s.object,s.serial};return true;
    }
    bool KnifeOwner(bool* terminal=nullptr){
        knifeFailure=1;unsigned char authority=0;bool living=false;std::uintptr_t table=0;
        if(terminal)*terminal=false;
        if(knifeOwnerProved_){unsigned detail=0;if(!KnifeOwnerLinks(&detail)){knifeFailure=detail;return false;}}
        else {
            std::uintptr_t state=0;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::Context);if(!Context())return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::Jason);if(!Jason())return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::PlayerState);
            if(!r_.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,state))return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::PlayerStateSlot);
            if(!cfgspawn::ObjectSlot(r_,context.objectArray,state,knifeState_))return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::PlayerStateClass);
            if(!r_.Read(state+0x10,knifeStateClass_))return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::PawnClass);
            if(!r_.Read(bundleJason.pawn.object+0x10,knifeOwnerClasses_[0]))return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::ControllerClass);
            if(!r_.Read(bundleJason.killerController.object+0x10,knifeOwnerClasses_[1]))return false;
            knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::PathClass);
            if(!r_.Read(bundleJason.path.object+0x10,knifeOwnerClasses_[2]))return false;
            knifeOwnerProved_=true;unsigned detail=0;
            if(!KnifeOwnerLinks(&detail)){knifeFailure=detail;return false;}
        }
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::AliveProof);
        if(!Alive(bundleJason.pawn.object,living))return false;
        if(!living){knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::NotLiving);if(terminal)*terminal=true;return false;}
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::AuthorityRole);
        if(!r_.Read(bundleJason.pawn.object+cfgknife::disk::Role,authority)||authority!=3)return false;
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::VTableRead);
        if(!r_.Read(bundleJason.pawn.object,table))return false;
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::ReleaseBinding);
        if(!cfgobserver::Same(r_,table+0xF60,plan.module+cfgknife::disk::Release))return false;
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::RefreshBinding);
        if(!cfgobserver::Same(r_,table+0xF50,plan.module+cfgknife::disk::RefreshDriver))return false;
        knifeFailure=static_cast<unsigned>(cfgbundle::KnifeOwnerFailure::FinalizeBinding);
        if(!cfgobserver::Same(r_,table+0x1050,plan.module+cfgknife::disk::Finalize))return false;
        knifeFailure=1;return true;
    }
    bool KnifeDriver(cfgknife::Identity expected,std::uintptr_t& driver,bool requireAuthority=false){
        using namespace cfgknife::disk;knifeFailure=2;
        std::uintptr_t type=0,ctor=0,table=0,owner=0;unsigned char authority=0;cfgknife::Identity id{};
        if(!r_.Read(bundleJason.pawn.object+Driver,driver)||!driver||!KnifeIdentity(driver,id)||!(id==expected)||
           !r_.Read(plan.module+ThrowableReflection,type)||!r_.Read(plan.module+ThrowableConstructor,ctor)||!type||type!=ctor||
           !cfgspawn::ObjectIs(r_,driver,type)||!r_.Read(driver+Role,authority)||authority>3||(requireAuthority&&authority!=3)||
           !r_.Read(driver+ThrownBy,owner)||(owner&&owner!=bundleJason.pawn.object)||
           !r_.Read(driver,table)||!cfgobserver::Same(r_,table+0x5F0,plan.module+MulticastUse)||
           !cfgobserver::Same(r_,table+0x5E8,plan.module+Launch)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+Driver,driver))return false;
        return true;
    }
    bool KnifeObserve(unsigned index,cfgknife::Snapshot& out,bool entry=false){
        using namespace cfgknife::disk;out={};out.hosted=HostedRoutineActive();
        if(!KnifeOwner(&out.ownerTerminal)||!KnifeIdentity(bundleJason.pawn.object,out.jason))return false;
        unsigned char pressed=0;std::uintptr_t driver=0,target=0,held=0;bool stunned=false,attacking=false;
        if(!r_.Read(bundleJason.pawn.object+Count,out.count)||out.count<0||out.count>1000||
           !r_.Read(bundleJason.pawn.object+Pressed,pressed)||pressed>1||!r_.Read(bundleJason.pawn.object+Driver,driver))return false;
        out.pressed=pressed!=0;
        if(driver&&(!KnifeIdentity(driver,out.driver)||!KnifeDriver(out.driver,driver)))return false;
        if(driver&&!bundleKnife.ownedDriver.object)KnifeLifecycleSample(driver,0);
        if(Known(index,target)&&KnifeIdentity(target,out.target)&&Alive(target,out.targetLiving)&&out.targetLiving){
            if(entry&&!Distance(target,out.squared))return false;
        }
        if(!entry){knifeFailure=0;return true;}
        // Stun/attack are only an entry gate. Release/cleanup must stay available
        // during the montage that this lane started.
        if(!Held(held)||!cfgbundle::Code(r_,plan.module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes)||
           !cfgbundle::Code(r_,plan.module,cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes)||
           !NativeBool(cfghunter::disk::Stunned,stunned)||!NativeBool(cfghunter::disk::Attacking,attacking))return false;
        out.ready=!held&&!stunned&&!attacking;knifeFailure=0;return true;
    }
    bool KnifeRefill(const cfgknife::Snapshot& expected){
        using namespace cfgknife::disk;
        if(expected.count!=0&&expected.count<=20)return false;
        cfgknife::Identity fresh{};std::uintptr_t driver=0;unsigned char pressed=0;
        int count=-1;
        if(!KnifeOwner()||!KnifeIdentity(bundleJason.pawn.object,fresh)||!(fresh==expected.jason)||
           !cfgbundle::Code(r_,plan.module,Release,ReleaseCode)||
           !r_.Read(fresh.object+Driver,driver)||driver||
           !r_.Read(fresh.object+Pressed,pressed)||pressed||
           !r_.Read(fresh.object+Count,count)||count!=expected.count||
           GetCurrentThreadId()!=plan.threadId)return false;
        // Count layout is proved by the pinned native Release consumer.
        // Only our living authority AI can receive this donor replenishment.
        __try{return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(fresh.object+Count),6,count)==count;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool KnifeTarget(unsigned index,cfgknife::Identity expected,std::uintptr_t& target){
        cfgknife::Identity fresh{};bool living=false;
        return Known(index,target)&&KnifeIdentity(target,fresh)&&fresh==expected&&Alive(target,living)&&living;
    }
    bool KnifeViewWrite(const cfgspawn::Vec3& before,const cfgspawn::Vec3& rotation)noexcept{
        if(GetCurrentThreadId()!=plan.threadId)return false;
        __try{auto* view=reinterpret_cast<void*>(bundleJason.pawn.object+0x7DC);
            if(std::memcmp(view,&before,sizeof(before))!=0)return false;
            std::memcpy(view,&rotation,sizeof(rotation));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool KnifeAim(unsigned index,cfgknife::Identity expected){
#ifdef CFG_H1_JASON
        if(cfgh226::UseHostedAimSibling(HostedRoutineActive(),hostedKnifeAimSibling_)){
            // Observation and aiming can each need most of the regular read
            // cap. Aim is safe to move as one unit into a fresh bounded reader
            // because no aim-side native call has entered on this path yet.
            // Inherit this callback's call debt; never retry a partial aim.
            BundleAccess sibling(cfgbundle::ReadCap,calls_);sibling.context=context;
            sibling.hostedKnifeAimSibling_=true;ChildAccounting accounting(*this,sibling);
            const bool aimed=sibling.KnifeAim(index,expected);
            knifeFailure=sibling.knifeFailure;exception_=exception_||sibling.exception_;
            return aimed;
        }
#endif
        knifeFailure=3;std::uintptr_t target=0,function=0,owner=0;cfgspawn::Vec3 j{},t{},rotation{};
        if(!KnifeTarget(index,expected,target)||!Location(bundleJason.pawn.object,j)||!Location(target,t))return false;
        // Diagnostic only: normal native throws previously left this blank,
        // making their origin impossible to compare with corrective launches.
        bundleReport.knifeAimTarget=target;bundleReport.knifeAimPoint={t.x,t.y,t.z+cfgknife::TorsoHeight};
        // Match corrective launch's existing75cm hand-height convention.
        // A root-origin pitch puts that modeled ray75cm above the torso.
        // Exact native socket height and live accuracy remain to be measured.
        const float x=t.x-j.x,y=t.y-j.y;float pitch=0;
        if(!cfgknife::AimPitch(j.z,t.z,std::sqrt(x*x+y*y),pitch))return false;
        rotation={pitch,std::atan2(y,x)*57.2957795f,0};
        if(!cfgspawn::Finite(rotation)||!FeatureFunction(cfgtrap::disk::SetControlRotation,function,owner)||
           !cfgspawn::ObjectIs(r_,bundleJason.killerController.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::SetControlRotationExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode)||
           !KnifeOwner()||!KnifeTarget(index,expected,target)||!Invoke(bundleJason.killerController.object,function,&rotation))return false;
        cfgtrap::RotationParameters body;body.rotation={0,rotation.y,0};
        if(!FeatureFunction(cfgtrap::disk::K2_SetActorRotation,function,owner)||!cfgspawn::ObjectIs(r_,bundleJason.pawn.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::K2_SetActorRotationExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode)||
           !KnifeOwner()||!KnifeTarget(index,expected,target)||!Invoke(bundleJason.pawn.object,function,&body)||body.result!=1)return false;
        cfgspawn::Vec3 previous{};
        if(!cfgbundle::Code(r_,plan.module,cfgknife::disk::ViewTransform,cfgknife::disk::ViewTransformCode)||
           !cfgbundle::Code(r_,plan.module,cfgknife::disk::Velocity,cfgknife::disk::VelocityCode)||
           !KnifeOwner()||!KnifeTarget(index,expected,target)||!r_.Read(bundleJason.pawn.object+0x7DC,previous)||
           !cfgspawn::Finite(previous)||!KnifeViewWrite(previous,rotation))return false;
        knifeFailure=0;return true;
    }
    bool KnifeSimpleCall(std::uintptr_t entry)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        std::uintptr_t driver=0;
        if(!RetainBundleActors(bundleJason.pawn.object)||
           !r_.Read(bundleJason.pawn.object+cfgknife::disk::Driver,driver)||
           !RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{driver})||!BeginBundleNative())return false;
#endif
        ++calls_;
        const auto savedReleaseDepth=bundleKnifeReleaseDepth;
        if(entry==cfgknife::disk::Release)++bundleKnifeReleaseDepth;
        __try{reinterpret_cast<void(__fastcall*)(void*)>(plan.module+entry)(reinterpret_cast<void*>(bundleJason.pawn.object));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;bundleKnifeReleaseDepth=savedReleaseDepth;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
        bundleKnifeReleaseDepth=savedReleaseDepth;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
    bool KnifePress(){
        knifeFailure=4;std::uintptr_t driver=0;unsigned char pressed=0;
        if(!cfgbundle::Code(r_,plan.module,cfgknife::disk::Press,cfgknife::disk::PressCode)||!KnifeOwner()||
           !r_.Read(bundleJason.pawn.object+cfgknife::disk::Driver,driver)||driver||
           !r_.Read(bundleJason.pawn.object+cfgknife::disk::Pressed,pressed)||pressed)return false;
        // Donor Features.cpp deliberately preserves the accepted chase path:
        // canceling it before native Press leaves stale locomotion at notify.
        // Windup owns aim updates; it must not also cancel path following.
        bundleKnife.nativeCompletionRequired=bundleKnifeNotifyEnabled;
        const bool ok=KnifeSimpleCall(cfgknife::disk::Press);if(ok)knifeFailure=0;return ok;
    }
    bool KnifeRelease(cfgknife::Identity expected){
        knifeFailure=5;std::uintptr_t driver=0;
        if(!cfgbundle::Code(r_,plan.module,cfgknife::disk::Release,cfgknife::disk::ReleaseCode)||!KnifeOwner()||!KnifeDriver(expected,driver))return false;
        KnifeLifecycleSample(driver,1);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(bundleKnifeReleaseAdapterEnabled&&bundleKnife.stage==cfgknife::Stage::Notify&&bundleKnife.nativeCompletionRequired)
            return KnifeRetainedRelease(expected);
#endif
        const bool ok=KnifeSimpleCall(cfgknife::disk::Release);KnifeLifecycleSample(driver,2);if(ok)knifeFailure=0;return ok;
    }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    bool KnifeReleaseEntryProof(const cfgknife::ReleaseFrame& frame){
        cfgknife::Snapshot snapshot{};std::uintptr_t driver=0,thrownBy=1;std::uint8_t collision=0;
        // Read-only renewal on the supplied reader. No getters, retention,
        // publication or nested native calls while Release is entered.
        return KnifeObserve(bundleKnife.targetIndex,snapshot)&&snapshot.jason==frame.window.jason&&
            snapshot.target==frame.window.target&&snapshot.targetLiving&&snapshot.driver==frame.window.driver&&
            snapshot.count==frame.window.count&&KnifeDriver(frame.window.driver,driver,true)&&
            r_.Read(driver+cfgknife::disk::ThrownBy,thrownBy)&&!thrownBy&&
            r_.Read(driver+0x86,collision)&&(collision&2)!=0&&KnifeOwnerLinks();
    }
    bool KnifeReleaseEntryCurrent(const cfgknife::ReleaseFrame& frame){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()&&HostedAdoptionOwnsDriver()){
            // ReleaseFrame::Enter already matched the exact ThrowKnife return
            // PC, owner-thread/depth and this lexical lease before calling us.
            // Recheck the stack frame and original callback witness, then give
            // ONLY the nested read-only renewal a fresh bounded reader. Inherit
            // call debt; it performs no getter, retention or native call.
            if(frame.access!=this||frame.lease!=bundleRootLease||!bundleRootLease||
               !bundleRootLease->Current()||!bundleRootLease->NativeEntered()||
               frame.thread!=plan.threadId||GetCurrentThreadId()!=plan.threadId||
               frame.depth!=bundleKnifeReleaseDepth||
               frame.returnPc!=plan.module+cfgcompat::cfg35::constructBoundary::ReturnRva||
               frame.entries!=1||frame.inside||frame.ambiguous||!frame.window.armed||
               bundleKnifeReleaseFrame!=&frame||bundleRootWitness.lease!=bundleRootLease||
               bundleRootWitness.actor!=primaryRootTickActor||bundleRootWitness.function!=primaryRootTickFunction||
               bundleRootWitness.pc!=primaryRootTickReturn)return false;
            BundleAccess entry(cfgbundle::ReadCap,calls_);entry.context=context;
            ChildAccounting accounting(*this,entry);
            const bool current=entry.KnifeReleaseEntryProof(frame);
            bundleKnifeReleaseDiagnostics.entryReads=entry.r_.reads;
            return current;
        }
#endif
        const auto before=r_.reads;
        const bool current=KnifeReleaseEntryProof(frame);
        bundleKnifeReleaseDiagnostics.entryReads=r_.reads-before;return current;
    }
#include "KnifeReleaseAdapterMembers.hpp"
#endif
    bool KnifeFinalize(cfgknife::Identity expected){
        knifeFailure=6;std::uintptr_t driver=0;
        if(!cfgbundle::Code(r_,plan.module,cfgknife::disk::Finalize,cfgknife::disk::FinalizeCode)||!KnifeOwner()||!KnifeDriver(expected,driver))return false;
        const bool ok=KnifeSimpleCall(cfgknife::disk::Finalize);if(ok)knifeFailure=0;return ok;
    }
    bool KnifeVelocityCall(std::uintptr_t driver,cfgspawn::Vec3& output)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(driver)||!BeginBundleNative())return false;
#endif
        ++calls_;
        bool valid=false;
        __try{const auto returned=reinterpret_cast<cfgspawn::Vec3*(__fastcall*)(void*,cfgspawn::Vec3*,void*,bool)>(plan.module+cfgknife::disk::Velocity)(
            reinterpret_cast<void*>(driver),&output,reinterpret_cast<void*>(bundleJason.pawn.object),true);valid=returned==&output&&cfgspawn::Finite(output);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative()&&valid;
#else
        return valid;
#endif
    }
    bool KnifeLaunchCall(std::uintptr_t driver,cfgspawn::Vec3& velocity,cfgspawn::Vec3& location)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(driver)||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,void*,const cfgspawn::Vec3*,const cfgspawn::Vec3*)>(plan.module+cfgknife::disk::MulticastUse)(
            reinterpret_cast<void*>(driver),reinterpret_cast<void*>(bundleJason.pawn.object),&velocity,&location);}
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
    bool KnifeFlight(cfgknife::Identity expected,cfgknife::FlightSample& sample){
        sample={};cfgknife::Identity identity{};std::uintptr_t root=0,movement=0,owner=0,table=0;cfgspawn::Vec3 position{},velocity{};unsigned char active=0;
        // Read-only retained projectile, independent of Jason.Driver (stock
        // clears that pointer while the thrown actor is still in flight).
        if(!KnifeIdentity(expected.object,identity)||!(identity==expected)||
           !cfgobserver::Same(r_,expected.object+cfgknife::disk::ThrownBy,bundleJason.pawn.object)||
           !r_.Read(expected.object+0x160,root)||!root||!r_.Read(root+0x20,owner)||owner!=expected.object||
           !r_.Read(root+0x190,position)||!cfgspawn::Finite(position)||
           !r_.Read(expected.object+0x378,movement)||!movement||!r_.Read(movement,table)||table!=plan.module+cfgknife::disk::ProjectileMovementTable||
           !r_.Read(movement+0x20,owner)||owner!=expected.object||!r_.Read(movement+0x104,velocity)||!cfgspawn::Finite(velocity)||
           !r_.Read(movement+0xF0,sample.updated)||!r_.Read(movement+0xAA,active)||
           !KnifeIdentity(expected.object,identity)||!(identity==expected))return false;
        sample.x=position.x;sample.y=position.y;sample.z=position.z;sample.active=(active&1)!=0;
        const auto tick=GetTickCount64();
        if(bundleReport.knifeTrailCount<8&&bundleKnife.launchedAt&&tick>=bundleKnife.launchedAt&&tick-bundleKnife.launchedAt<=200){
            // Diagnostic-only known target/root readback; no getter/RPC,
            // collision edits, launch replay, steering or global scanning.
            std::uintptr_t target=0,targetRoot=0;cfgspawn::Vec3 targetPosition{};
            if(KnifeTarget(bundleKnife.targetIndex,bundleKnife.initial.target,target)&&
               r_.Read(target+0x160,targetRoot)&&targetRoot&&cfgobserver::Same(r_,targetRoot+0x20,target)&&
               r_.Read(targetRoot+0x190,targetPosition)&&cfgspawn::Finite(targetPosition)&&
               cfgobserver::Same(r_,target+0x160,targetRoot)&&cfgobserver::Same(r_,targetRoot+0x20,target)){
                const auto i=bundleReport.knifeTrailCount++;
                bundleReport.knifeTrailMs[i]=static_cast<std::uint32_t>(tick-bundleKnife.launchedAt);
                bundleReport.knifeTrailPosition[i]=position;bundleReport.knifeTrailVelocity[i]=velocity;
                bundleReport.knifeTrailTarget[i]=targetPosition;
            }
        }
        sample.speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y+velocity.z*velocity.z);return std::isfinite(sample.speed);
    }
    bool KnifePrepareMovement(std::uintptr_t driver,std::uintptr_t movement){
        using namespace cfgknife::disk;
        std::uintptr_t root=0,updated=0,table=0,outer=0;cfgprobe::lifetime::SlotObservation rootSlot{};cfgknife::Identity movementId{};
        if(!r_.Read(driver+0x160,root)||!root||!cfgspawn::ObjectSlot(r_,context.objectArray,root,rootSlot)||!KnifeIdentity(movement,movementId)||
           !r_.Read(root+0x20,outer)||outer!=driver||!r_.Read(movement,table)||table!=plan.module+ProjectileMovementTable||
           !r_.Read(movement+0xF0,updated)||(updated&&updated!=root)||
           !cfgbundle::Code(r_,plan.module,SetUpdatedComponent,SetUpdatedComponentCode)||
           !cfgbundle::Code(r_,plan.module,ActivateMovement,ActivateMovementCode)||calls_>=cfgbundle::CallCap||!NativeEnvironmentAllowed())return false;
        // StopSimulating may clear UpdatedComponent before the AI notify arrives.
        // Restore only this projectile's own root, through the stock setter.
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(driver)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{movement,root}))return false;
#endif
        __try{
            if(!updated){if(!NativeCallAllowed())return false;++calls_;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                if(!BeginBundleNative())return false;
#endif
                reinterpret_cast<void(__fastcall*)(void*,void*)>(plan.module+SetUpdatedComponent)(reinterpret_cast<void*>(movement),reinterpret_cast<void*>(root));
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                if(!FinishBundleNative())return false;
#endif
            }
        }__except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
        return cfgjason::OwnedComponentSlot(r_,context.objectArray,rootSlot)&&cfgobserver::Same(r_,driver+0x160,root)&&
            cfgobserver::Same(r_,root+0x20,driver)&&cfgobserver::Same(r_,movement+0xF0,root);
    }
    bool KnifeActivateAfterLaunch(std::uintptr_t driver,std::uintptr_t movement){
        using namespace cfgknife::disk;std::uintptr_t root=0,outer=0,table=0;
        cfgknife::Identity movementId{};cfgprobe::lifetime::SlotObservation rootSlot{};
        if(calls_>=cfgbundle::CallCap||!NativeEnvironmentAllowed()||
           !KnifeIdentity(movement,movementId)||!r_.Read(movement,table)||table!=plan.module+ProjectileMovementTable||
           !r_.Read(movement+0x20,outer)||outer!=driver||!r_.Read(driver+0x160,root)||!root||
           !cfgspawn::ObjectSlot(r_,context.objectArray,root,rootSlot)||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,rootSlot)||
           !cfgobserver::Same(r_,root+0x20,driver)||!cfgobserver::Same(r_,movement+0xF0,root)||
           !cfgbundle::Code(r_,plan.module,ActivateMovement,ActivateMovementCode))return false;
        // Exact donor order: repair UpdatedComponent, native multicast, THEN
        // Activate(reset). Never reactivate a later real-impact flight tick.
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(driver)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{movement,root}))return false;
#endif
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,bool)>(plan.module+ActivateMovement)(reinterpret_cast<void*>(movement),true);}
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
    bool KnifeLaunch(unsigned index,cfgknife::Identity targetId,cfgknife::Identity driverId){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Knife);
#endif

        using namespace cfgknife::disk;knifeFailure=7;std::uintptr_t driver=0,target=0,function=0,movement=0,back=0;
        cfgspawn::Vec3 j{},t{},direction{},velocity{},scratch{};float speed=0;cfgprobe::lifetime::SlotObservation moveSlot{};
        if(!KnifeOwner()||!KnifeDriver(driverId,driver,true)||!KnifeTarget(index,targetId,target)||
           !Location(bundleJason.pawn.object,j)||!Location(target,t)||
           !r_.Read(bundleJason.pawn.object+Speed,speed)||!std::isfinite(speed)||speed<100.f||speed>20000.f||
           !r_.Read(driver+0x378,movement)||!movement||!cfgspawn::ObjectSlot(r_,context.objectArray,movement,moveSlot)||!moveSlot.HasAssignedSerial()||
           !r_.Read(movement+0x20,back)||back!=driver||
           !cfgbundle::Code(r_,plan.module,Velocity,VelocityCode)||!KnifeNotifyBindings()||!TeleportFunction(function))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Knife,1)
#else
 false
#endif
;
        j.z+=cfgknife::HandHeight;t.z+=cfgknife::TorsoHeight;direction={t.x-j.x,t.y-j.y,t.z-j.z};
        float length=std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);
        if(!std::isfinite(length)||length<1)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Knife,2)
#else
 false
#endif
;
        direction={direction.x/length,direction.y/length,direction.z/length};
        cfgpursuit::TeleportPacket packet{};packet.location={j.x+direction.x*110.f,j.y+direction.y*110.f,j.z+direction.z*110.f};
        packet.rotation={std::atan2(direction.z,std::sqrt(direction.x*direction.x+direction.y*direction.y))*57.2957795f,std::atan2(direction.y,direction.x)*57.2957795f,0};
        direction={t.x-packet.location.x,t.y-packet.location.y,t.z-packet.location.z};
        length=std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);
        if(!std::isfinite(length)||length<1)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Knife,3)
#else
 false
#endif
;
        velocity={direction.x*speed/length,direction.y*speed/length,direction.z*speed/length};
        bundleReport.knifeAimTarget=target;bundleReport.knifeAimPoint=t;
        bundleReport.knifeLaunchPoint=packet.location;bundleReport.knifeLaunchVelocity=velocity;
        bundleReport.knifeLaunchObserved=0;
        // Read-only evidence for CFG's stopped-driver recovery. Native OnStop
        // clears actor collision as well as UpdatedComponent; never assume a
        // moving relaunch proves collision/hit eligibility or re-enable an
        // already impacted projectile merely to compensate for a miss.
        std::uint8_t collision=0;bundleReport.knifeUpdatedBefore=0;
        bundleReport.knifeCollisionBefore=0;bundleReport.knifeCollisionAfter=0;
        if(r_.Read(driver+0x86,collision)&&r_.Read(movement+0xF0,bundleReport.knifeUpdatedBefore))
            bundleReport.knifeCollisionBefore=(collision&2)?2u:1u;
        // Do not revive an already stopped/impacted noncolliding projectile.
        if(bundleReport.knifeCollisionBefore!=2||!KnifeCompletionPermission(driver))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Knife,4)
#else
 false
#endif
;
        bundleReport.knifeTrailCount=0;bundleReport.knifeTrailMs={};
        bundleReport.knifeTrailPosition={};bundleReport.knifeTrailTarget={};bundleReport.knifeTrailVelocity={};
        // Native setter/activation restores this driver's movement component;
        // visibility/collision and velocity stay in the real multicast body.
        // No actor Role patch and no second ThrowKnife call.
        if(calls_+5>cfgbundle::CallCap||!KnifeDriver(driverId,driver,true)||!KnifeTarget(index,targetId,target)||
           !Invoke(driver,function,&packet)||packet.result!=1||!KnifeDriver(driverId,driver,true)||
           !KnifeCompletionPermission(driver)||
           !KnifeVelocityCall(driver,scratch)||!KnifeDriver(driverId,driver,true)||
           !cfgobserver::Same(r_,driver+ThrownBy,bundleJason.pawn.object)||
           !cfgjason::SameSlot(r_,context.objectArray,moveSlot)||!cfgobserver::Same(r_,driver+0x378,movement)||
           !KnifePrepareMovement(driver,movement)||!KnifeCompletionPermission(driver)||!KnifeLaunchCall(driver,velocity,packet.location)||
           !KnifeActivateAfterLaunch(driver,movement))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Knife,5)
#else
 false
#endif
;
        // Diagnostic only, bounded known component reads, no second launch or
        // projectile steering. Distinguish target choice from native transform
        // or velocity changes instead of compensating with an arbitrary offset.
        std::uintptr_t root=0;cfgspawn::Vec3 actual{},actualVelocity{};
        if(r_.Read(driver+0x160,root)&&root&&cfgobserver::Same(r_,root+0x20,driver)&&
           r_.Read(root+0x190,actual)&&cfgspawn::Finite(actual)&&
           r_.Read(movement+0x104,actualVelocity)&&cfgspawn::Finite(actualVelocity)){
            bundleReport.knifeActualLaunch=actual;bundleReport.knifeActualVelocity=actualVelocity;
            bundleReport.knifeLaunchObserved=1;
            if(r_.Read(driver+0x86,collision))bundleReport.knifeCollisionAfter=(collision&2)?2u:1u;
        }
        knifeFailure=0;return true;
    }
