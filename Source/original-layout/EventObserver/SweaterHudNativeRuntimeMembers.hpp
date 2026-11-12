// Inside BundleAccess. The existing HUD event may allocate/reenter Blueprint;
// retain its exact controller/HUD/widget/ability inputs before native entry.
    bool SweaterHudNative(const cfgsweater::HudEvidence& hud)noexcept{
        if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,8>{plan.world,plan.mode,
            spawnAssets.gameState,plan.controller,bundleSweaterOwner.object,
            bundleSweaterAbility.object,hud.hud.object,hud.widget.object})||
            !NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        bundleSweaterPolicy.BeginPublish();++bundleSweaterPolicy.hudAttempts;++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,bool)>(plan.module+cfgsweater::huddisk::Available)(
            reinterpret_cast<void*>(hud.hud.object),true);}
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
