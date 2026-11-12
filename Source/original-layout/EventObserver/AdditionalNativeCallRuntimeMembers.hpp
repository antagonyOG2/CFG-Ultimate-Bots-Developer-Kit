// Included inside BundleAccess. All these calls share its last-moment veto.
    bool AdditionalCrowdCall(std::uintptr_t path)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(path)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,std::uint8_t)>(plan.module+cfgcrowd::Setter)(
            reinterpret_cast<void*>(path),cfgcrowd::DisabledState);}
        __except(EXCEPTION_EXECUTE_HANDLER){
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
    bool AdditionalBool(std::uintptr_t entry,bool& result)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(bundleAdditional.actor.pawn.object)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{result=reinterpret_cast<bool(__fastcall*)(void*)>(plan.module+entry)(
            reinterpret_cast<void*>(bundleAdditional.actor.pawn.object));}
        __except(EXCEPTION_EXECUTE_HANDLER){
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
    bool AdditionalInput(bool release)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(bundleAdditional.actor.pawn.object)||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*)>(plan.module+(release?cfghunter::disk::Release:cfghunter::disk::Press))(
            reinterpret_cast<void*>(bundleAdditional.actor.pawn.object));}
        __except(EXCEPTION_EXECUTE_HANDLER){
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
    bool AdditionalInvoke(std::uintptr_t object,std::uintptr_t function,void* packet,bool actor=true)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleInvocation(object,function,packet)||!NativeCallAllowed()||!EnterBundleInvocation())return false;
#endif
        ++calls_;
        __try{const auto fn=actor?originalActorDispatcher:reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva);
            fn(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),packet);}
        __except(EXCEPTION_EXECUTE_HANDLER){bundleAdditional.journal.uncertain=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const bool retained=RetainBundleResults(function,packet)&&NativeEnvironmentAllowed();
        if(!retained){bundleAdditional.journal.uncertain=true;BundleRetentionFailed();}
        return retained;
#else
        return true;
#endif
    }
