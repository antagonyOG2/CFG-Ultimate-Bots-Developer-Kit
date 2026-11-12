// Included inside BundleAccess and the focused native-boundary fixture.
// Recheck immediately before every call in these common gateways. This veto
// check is necessary, but does not establish retention or authorize execution.
    bool NativeEnvironmentAllowed() noexcept {
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return BundleRetentionEnvironment();
#else
        return GetCurrentThreadId()==plan.threadId && originalActorDispatcher &&
            cfgprimary::LookupSafe(r_,plan.module);
#endif
    }
    bool NativeCallAllowed() noexcept {
        bundleJ21Memo.Clear(); // even legacy native-entry attempts invalidate callback facts
#ifdef CFG_H1_JASON
        InvalidateHostedContextProof(); // never reuse an adoption proof across a native callback
#endif
        return calls_<cfgbundle::CallCap && NativeEnvironmentAllowed();
    }
    unsigned bundleInvocationStage_=0;
#if CFG_H277_HELPER_SPAWN_SAFE
    bool h277HelperSpawn_=false; // set only around the helper SpawnActor dispatch
    inline static unsigned h277VetoCount_=0,h277VetoTolerated_=0,h277VetoStage_=0,h277VetoSerializing_=0,h277VetoCollecting_=0,h277VetoScoped_=0;
    inline static std::uintptr_t h277VetoFunction_=0; // persists across the per-callback BundleAccess instances
#endif
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* parameters,bool actor=true)noexcept{
        bundleInvocationStage_=1;
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        bundleInvocationStage_=2;if(!RetainBundleInvocation(object,function,parameters))return false;
        bundleInvocationStage_=3;if(!NativeCallAllowed())return false;
        bundleInvocationStage_=4;if(!EnterBundleInvocation())return false;
#endif
        bundleInvocationStage_=5;
        ++calls_;
        __try{const auto fn=actor?originalActorDispatcher:reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva);
            fn(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),parameters);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;bundleInvocationStage_=9;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        bundleInvocationStage_=6;
        bool retained=RetainBundleResults(function,parameters);
#if CFG_H277_HELPER_SPAWN_SAFE
        const bool resultsPublished=retained; // CompleteCall succeeded: flags clear, outputs rooted under the window proof
#endif
        if(retained){bundleInvocationStage_=7;retained=NativeEnvironmentAllowed();}
#if CFG_H277_HELPER_SPAWN_SAFE
        if(resultsPublished&&!retained){ // frozen capture: first poison at this call with NativeFlags=0 => only the post-call environment veto failed
            Reader veto;veto.cap=64;std::uint8_t serializing=0xFF;std::uint32_t collecting=0xFFFFFFFFu;
            h277VetoStage_=bundleEnvironmentStage_;h277VetoFunction_=function;++h277VetoCount_;
            h277VetoSerializing_=veto.Read(plan.module+cfgprimary::SerializationFlag,serializing)?serializing:0xFFu;
            h277VetoCollecting_=veto.Read(plan.module+cfgprimary::GarbageCollectionCounter,collecting)?collecting:0xFFFFFFFFu;
            h277VetoScoped_=cfgprimary::ScopedLookupEligibility<Reader>::Current(veto,plan.module)?1u:0u;
            if(h277HelperSpawn_&&!bundleRetentionUncertain_){++h277VetoTolerated_;retained=true;} // the spawn is real and rooted; a lookup veto is not a lifetime fault
        }
#endif
        if(!retained)BundleRetentionFailed(); // dispatch happened: never replay an uncertain result
        if(retained)bundleInvocationStage_=8;
        return retained;
#else
        bundleInvocationStage_=8;
        return true;
#endif
    }
    bool NativeBool(std::uintptr_t entry,bool& out)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(bundleJason.pawn.object)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{out=reinterpret_cast<bool(__fastcall*)(void*)>(plan.module+entry)(reinterpret_cast<void*>(bundleJason.pawn.object));}
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
    bool Input(bool release)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(bundleJason.pawn.object)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*)>(plan.module+(release?cfghunter::disk::Release:cfghunter::disk::Press))(
            reinterpret_cast<void*>(bundleJason.pawn.object));}
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
