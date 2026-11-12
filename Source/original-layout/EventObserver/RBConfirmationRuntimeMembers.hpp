// Inside the existing BundleAccess. All widget/profile/native queries retain
// exact inputs through the established callback lease, never a UI observation.
    bool RBConfirmationCurrent(const cfgrbemote::Owner& owner,const cfgrbemote::ConfirmationEvidence& evidence){
        cfgemotemirror::Slot pawn{};
        return NativeCallAllowed()&&Context()&&owner.world==plan.world&&owner.controller==plan.controller&&
            cfgjason::SameSlot(r_,context.objectArray,owner.controllerSlot)&&MirrorHuman(pawn)&&pawn==owner.pawn&&
            cfgjason::OwnedComponentSlot(r_,context.objectArray,evidence.hud)&&cfgjason::OwnedComponentSlot(r_,context.objectArray,evidence.widget)&&
            cfgobserver::Same(r_,plan.controller+cfgsweater::huddisk::ControllerHUD,evidence.hud.object)&&
            cfgobserver::Same(r_,evidence.hud.object+cfgrbemote::confirmdisk::PlayerOwner,plan.controller)&&
            cfgobserver::Same(r_,evidence.hud.object+cfgrbemote::confirmdisk::EmoteWidgetInstance,evidence.widget.object)&&
            cfgescape::Same(r_,evidence.widget.object+cfgrbemote::confirmdisk::CurrentHighlightSection,evidence.index);
    }
    bool RBConfirmationRoots(const cfgrbemote::Owner& owner,const cfgrbemote::ConfirmationEvidence& evidence){
        if(!RBConfirmationCurrent(owner,evidence))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        Reader retention;retention.cap=512;
        if(!RetainPrimaryObjects(retention,std::array<std::uintptr_t,7>{plan.world,plan.mode,spawnAssets.gameState,
            owner.controller,owner.pawn.object,evidence.hud.object,evidence.widget.object}))return false;
#endif
        return RBConfirmationCurrent(owner,evidence);
    }
    bool RBEligible(const cfgrbemote::Owner& owner,const cfgrbemote::ConfirmationEvidence& evidence){
        namespace d=cfgrbemote::confirmdisk;
        if(!cfgbundle::Code(r_,plan.module,d::Eligible,d::EligibleCode)||!RBConfirmationRoots(owner,evidence))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!BeginBundleNative())return false;
#endif
        ++calls_;bool eligible=false;
        __try{eligible=reinterpret_cast<bool(__fastcall*)(void*)>(plan.module+d::Eligible)(reinterpret_cast<void*>(owner.pawn.object));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!FinishBundleNative())return false;
#endif
        return eligible;
    }
    bool RBInvoke(const cfgrbemote::Owner& owner,const cfgrbemote::ConfirmationEvidence& evidence,std::uintptr_t function,void* packet){
        return RBConfirmationRoots(owner,evidence)&&Invoke(evidence.widget.object,function,packet,false);
    }
    void RBConfirmationFailed(){BundleRBConfirmationFailed();}
    #if !CFG_J21_EMOTE_MIRROR
bool RBCancelCurrent(const cfgrbemote::Owner& owner){
        cfgemotemirror::Slot pawn{};
        return NativeCallAllowed()&&Context()&&owner.world==plan.world&&owner.controller==plan.controller&&
            MirrorHuman(pawn)&&pawn==owner.pawn&&BundleRBCancelCurrent(pawn.object);
    }
#else
bool RBCancelCurrent(const cfgrbemote::Owner& owner){
        cfgemotemirror::Slot pawn{};
#ifdef CFG35_URINATION_NATIVE_PORT
        if(!NativeCallAllowed()||!Context()||owner.world!=plan.world||owner.controller!=plan.controller||!MirrorHuman(pawn)||!(pawn==owner.pawn))return false;
        cfgemotemirror::NativeAccess<Reader,BundleAccess> access(r_,*this,MirrorInputs());
        return bundleEmotes.CancelLocalForSelection(access,pawn);
#else
        return NativeCallAllowed()&&Context()&&owner.world==plan.world&&owner.controller==plan.controller&&
            MirrorHuman(pawn)&&pawn==owner.pawn&&BundleRBCancelCurrent(pawn.object);
#endif
    }
#endif
