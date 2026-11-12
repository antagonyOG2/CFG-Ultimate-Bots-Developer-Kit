// Include inside BundleAccess. Core/EndingMetadataPolicy.hpp and generated
// bindings must be included globally; owner provides bundleEnding transaction.
    bool EndingScope(){
        cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(r_,plan.module);
        return Context()&&Jason()&&r_.Read(plan.mode+cfgescape::disk::MatchState,phase)&&
            names.Decode(phase,text)&&text.View()==u"InProgress"&&cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase);
    }
    bool EndingFresh(){
        const auto& e=bundleEnding;
        cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t state=0,type=0,human=0,defaults=0;
        cfgending::Packet selected{},weapon{};
        return EndingScope()&&
            r_.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,state)&&state==e.playerState&&
            cfgobserver::Same(r_,bundleJason.killerController.object+cfgescape::disk::PlayerState,state)&&
            cfgobserver::Same(r_,state+0x88,bundleJason.killerController.object)&&
            r_.Read(plan.controller+cfgescape::disk::PlayerState,human)&&state!=human&&
            cfgspawn::ObjectSlot(r_,context.objectArray,state,slot)&&slot.serial==e.playerStateSerial&&slot.index==e.playerStateIndex&&
            cfgobserver::Same(r_,state+cfgprobe::ObjectClass,e.playerStateType)&&
            r_.Read(e.playerStateType+0xF8,defaults)&&cfgprobe::ObjectPointer(defaults)&&defaults!=state&&
            cfgobserver::Same(r_,defaults+cfgprobe::ObjectClass,e.playerStateType)&&
            cfgescape::Same(r_,state+0x110,std::uint8_t{3})&&
            r_.Read(plan.module+cfgending::disk::SCPlayerStateClass,type)&&
            cfgobserver::Same(r_,plan.module+cfgending::disk::SCPlayerStateConstructor,type)&&cfgspawn::ObjectIs(r_,state,type)&&
            cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,bundleConfiguration.jasonCatalogueIndex)&&
            r_.Read(e.selectedSource,selected)&&selected==e.selected&&
            r_.Read(e.weaponSource,weapon)&&weapon==e.weapon&&
            cfgobserver::Same(r_,JasonTemplate().pawnClass+cfgspawn::ClassDefaultObject,JasonTemplate().defaults);
    }
    bool EndingNativeInvoke(std::uintptr_t entry,void* argument){
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(plan.module+entry)(reinterpret_cast<void*>(bundleEnding.playerState),argument);return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
    }
    bool EndingNative(cfgending::NativeSetter setter,void* argument){
        if(!argument||!NativeCallAllowed()||!EndingFresh())return false;
        std::uintptr_t entry=0,skin=0;
        switch(setter){
        case cfgending::NativeSetter::Class:
            if(!cfgending::disk::NativeSettersVerified||!cfgending::EmptySubpath(*static_cast<cfgending::Packet*>(argument)))return false;
            entry=cfgending::disk::SetClass;break;
        case cfgending::NativeSetter::Active:
            if(!cfgending::EmptySubpath(*static_cast<cfgending::Packet*>(argument)))return false;
            entry=cfgmenu::birthdisk::ConsumeActive;break;
        case cfgending::NativeSetter::Skin:
            skin=reinterpret_cast<std::uintptr_t>(argument);
            if(!cfgending::disk::NativeSettersVerified||!cfgending::skin::Concrete(r_,plan.module,context.objectArray,skin))return false;
            entry=cfgending::disk::SetSkin;break;
        case cfgending::NativeSetter::Weapon:
            if(!cfgending::disk::NativeSettersVerified||!cfgending::EmptySubpath(*static_cast<cfgending::Packet*>(argument)))return false;
            entry=cfgending::disk::SetWeapon;break;
        default:return false;
        }
        struct Host {
            BundleAccess& a;std::uintptr_t entry,skin;void* argument;
            bool Prepare(){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                Reader retention;retention.cap=512;
                if(!a.RetainBundleActors(bundleEnding.playerState)||
                   !RetainPrimaryObjects(retention,std::array<std::uintptr_t,7>{bundleEnding.playerState,
                       bundleEnding.playerStateType,plan.controller,JasonTemplate().pawnClass,
                       JasonTemplate().defaults,skin,spawnAssets.gameState}))return false;
#endif
                return a.NativeCallAllowed()&&a.EndingFresh();
            }
            bool Begin(){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                return a.BeginBundleNative();
#else
                return a.NativeCallAllowed();
#endif
            }
            bool Invoke(){return a.EndingNativeInvoke(entry,argument);}
            bool Finish(){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                return a.FinishBundleNative();
#else
                return true;
#endif
            }
        }host{*this,entry,skin,argument};
        return cfgending::RetainedSetter(bundleEnding,setter,host);
    }
    bool EndingMetadataPhase(){
        using cfgending::Stage;auto& e=bundleEnding;
        if(e.nativeBusy)return false; // nested callback may observe, never replay the setter
        if(e.Finished())return true;
        if(e.stage==Stage::Prepare){
            if(!EndingScope())return e.Refuse(1);
            std::uintptr_t state=0,humanState=0,type=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!r_.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,state)||!state||
               !r_.Read(plan.controller+cfgescape::disk::PlayerState,humanState)||state==humanState||
               !cfgobserver::Same(r_,bundleJason.killerController.object+cfgescape::disk::PlayerState,state)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,state,slot))return e.Refuse(2);
            if(!r_.Read(plan.module+cfgending::disk::SCPlayerStateClass,type)||
               !cfgobserver::Same(r_,plan.module+cfgending::disk::SCPlayerStateConstructor,type)||!cfgspawn::ObjectIs(r_,state,type))return e.Refuse(3);
            e.playerState=state;e.playerStateSerial=slot.serial;e.playerStateIndex=slot.index;
            if(!r_.Read(state+cfgprobe::ObjectClass,e.playerStateType))return e.Refuse(3);
            // Donor BindNativeCurrentKiller publishes the exact pawn/controller
            // PlayerState independently of presentation metadata. CFG may leave
            // this strongly owned state at serial zero; never allocate a serial
            // or infer counselor/killer roles from cosmetic soft-class paths.
            std::uintptr_t previous=0,defaults=0;cfgprobe::lifetime::SlotObservation fresh{};
            if(!cfgobserver::Same(r_,state+0x88,bundleJason.killerController.object)||
               !cfgescape::Same(r_,state+0x110,std::uint8_t{3})||
               !r_.Read(e.playerStateType+cfgspawn::ClassDefaultObject,defaults)||!cfgprobe::ObjectPointer(defaults)||defaults==state||
               !cfgobserver::Same(r_,defaults+cfgprobe::ObjectClass,e.playerStateType)||
               !cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKiller,bundleJason.pawn.object)||
               !r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,previous)||
               !cfgending::CanRegisterState(state,humanState,previous)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,state,fresh)||!(fresh==slot)||
               !cfgobserver::Same(r_,bundleJason.pawn.object+cfgescape::disk::PlayerState,state)||
               !cfgobserver::Same(r_,bundleJason.killerController.object+cfgescape::disk::PlayerState,state)||
               !cfgobserver::Same(r_,state+cfgprobe::ObjectClass,e.playerStateType))return e.Refuse(18);
            if(previous!=state&&InterlockedCompareExchangePointer(
                reinterpret_cast<void* volatile*>(spawnAssets.gameState+cfgprimary::CurrentKillerState),
                reinterpret_cast<void*>(state),nullptr)!=nullptr)return e.Refuse(19);
            if(!cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKillerState,state))return e.Refuse(19);
            const auto index=bundleConfiguration.jasonCatalogueIndex;
            if(!cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,index))return e.Refuse(4);
            e.selectedSource=spawnAssets.primary.catalogue.data+static_cast<std::uintptr_t>(index)*40;
            e.weaponSource=JasonTemplate().defaults+cfgending::disk::DefaultWeapon;
            if(!r_.Read(e.selectedSource,e.selected)||!cfgending::EmptySubpath(e.selected)||
               !cfgmenu::classchoice::MatchesPath(r_,plan.module,JasonTemplate().pawnClass,spawnAssets.killerBase,e.selected)||
               !r_.Read(e.weaponSource,e.weapon)||!cfgending::EmptySubpath(e.weapon))return e.Refuse(5);
            e.stage=Stage::Class;return false;
        }
        if(!EndingFresh())return e.Refuse(6);
        cfgending::Packet result{};
        if(e.stage==Stage::Class){
            if(!cfgending::disk::NativeSettersVerified)return e.Refuse(27);
            if(!cfgbundle::Code(r_,plan.module,cfgending::disk::SetClass,cfgending::disk::SetClassCode))return e.Refuse(7);
            alignas(8) auto owned=e.selected; // empty subpath proven; destructor cannot free game-owned data
            if(!EndingNative(cfgending::NativeSetter::Class,&owned)||!EndingFresh()||
               !r_.Read(e.playerState+cfgending::disk::PickedClass,result)||!cfgending::SamePath(result,e.selected))return e.Refuse(8);
            e.stage=Stage::Active;return false;
        }
        if(e.stage==Stage::Active){
            namespace d=cfgmenu::birthdisk;
            if(!cfgbundle::Code(r_,plan.module,d::ConsumeActive,d::ConsumeActiveCode)||
               !cfgbundle::Code(r_,plan.module,d::PathCopy,d::PathCopyCode)||
               !cfgbundle::Code(r_,plan.module,d::ActiveNotify,d::ActiveNotifyCode)||
               !cfgbundle::Code(r_,plan.module,d::SettingsNotify,d::SettingsNotifyCode)||
               !r_.Read(e.playerState+d::ActiveCharacterClass,result)||!e.BeginActive(result))return e.Refuse(16);
            // The exact consuming setter owns this local by-value packet. Its
            // empty subpath contains no FString allocation borrowed from CFG.
            // No pawn class, role/traitor/death flag or cinematic hook is changed.
            alignas(8) auto owned=e.selected;
            const bool returned=EndingNative(cfgending::NativeSetter::Active,&owned);
            const bool fresh=returned&&EndingFresh()&&r_.Read(e.playerState+d::ActiveCharacterClass,result);
            if(!e.CompleteActive(fresh,result))return false;
            return false;
        }
        if(e.stage==Stage::Skin||e.stage==Stage::SkinInFlight){
            // CFG PlayOutro does NOT accept a null skin: it requests settings
            // and returns without starting ClientsPlayOutro. Follow donor's
            // native skin-before-weapon order, using only the current human's
            // matching selected loadout. A captured empty offline profile may
            // use the donor's exact native no-override class; never a guessed asset.
            struct SkinHost {
                BundleAccess& a;
                bool Fresh(){return a.bundleSkinInFlight()&&a.EndingFresh();}
                bool Target(std::uintptr_t& out){
                    const bool okay=a.r_.Read(bundleEnding.playerState+cfgending::disk::PickedSkin,out);
                    if(bundleEndingSkin.phase==1)cfgclosedtrace::Mark(okay?"ending-skin-target":"ending-skin-target-unreadable",bundleEnding.playerState,okay?out:a.r_.reads);
                    return okay;
                }
                bool Concrete(std::uintptr_t value){return cfgending::skin::Concrete(a.r_,plan.module,a.context.objectArray,value);}
                bool Source(std::uintptr_t& out){
#ifdef CFG_FRONTEND_BUNDLE
                    // Native intro borrowed ActiveCharacterClass/GameState
                    // KillerClass, NOT human PickedKillerClass. Use only its
                    // captured witness plus fresh unchanged strong source.
                    if(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Ready&&
                       nativeOfflineStartup.Status().world==plan.world&&
                       nativeOfflineStartup.Presentation().Ready()){
                        return cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[2])&&
                            nativeOfflineStartup.Presentation().Current(a.r_,a.context.objectArray,
                                plan.world,plan.controller,bundleEnding.selected,out)&&
                            cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[2]);
                    }
#endif
                    std::uintptr_t human=0;
                    return cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[2])&&
                        a.r_.Read(plan.controller+cfgescape::disk::PlayerState,human)&&human!=bundleEnding.playerState&&
                        cfgending::skin::Source(a.r_,plan.module,a.context.objectArray,human,plan.controller,bundleEnding.selected,out)&&
                        cfgobserver::Same(a.r_,plan.controller+cfgescape::disk::PlayerState,human)&&
                        cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[2]);
                }
                bool Set(std::uintptr_t value){
                    if(!cfgending::disk::NativeSettersVerified)return false;
#ifdef CFG_FRONTEND_BUNDLE
                    if(NativeOfflineWorld()){
                        std::uintptr_t human=0;
                        if(!a.r_.Read(plan.controller+cfgescape::disk::PlayerState,human)||
                           !cfgbundle::Code(a.r_,plan.module,cfgending::disk::SetSkin,cfgending::disk::SetSkinCode))return false;
                        const auto written=cfgending::skin::PublishOfflinePresentation(a.r_,plan.module,a.context.objectArray,
                            plan.world,bundleEnding.playerState,bundleJason.killerController.object,bundleJason.pawn.object,human,value,
                            [&](){return NativeOfflineWorld()&&a.EndingFresh();},
                            [&](std::uintptr_t field,std::uintptr_t skin){
                                return InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(field),
                                    reinterpret_cast<void*>(skin),nullptr)==nullptr;
                            });
                        cfgclosedtrace::Mark(written?"ending-offline-skin-published":"ending-offline-skin-refused",bundleEnding.playerState,value);
                        return written;
                    }
#endif
                    return cfgbundle::Code(a.r_,plan.module,cfgending::disk::SetSkin,cfgending::disk::SetSkinCode)&&
                        a.EndingNative(cfgending::NativeSetter::Skin,reinterpret_cast<void*>(value));
                }
            }host{*this};
            e.stage=Stage::SkinInFlight;
            const auto before=bundleEndingSkin.phase;
            const bool complete=bundleEndingSkin.Step(host);
            cfgclosedtrace::Mark(before?"ending-skin-verify":"ending-skin-prepare",e.playerState,
                (static_cast<std::uint64_t>(bundleEndingSkin.failure)<<32)|r_.reads);
            if(bundleEndingSkin.failure||e.stage!=Stage::SkinInFlight)return e.Refuse(20+bundleEndingSkin.failure);
            if(!complete)return false;
            e.skin=bundleEndingSkin.expected;
            e.stage=Stage::Weapon;return false;
        }
        if(e.stage==Stage::Weapon){
            if(!cfgending::disk::NativeSettersVerified)return e.Refuse(28);
            if(!cfgbundle::Code(r_,plan.module,cfgending::disk::SetWeapon,cfgending::disk::SetWeaponCode))return e.Refuse(11);
            alignas(8) auto weapon=e.weapon;
            if(!EndingNative(cfgending::NativeSetter::Weapon,&weapon)||!EndingFresh()||
               !r_.Read(e.playerState+cfgending::disk::PickedWeapon,result)||!cfgending::SamePath(result,e.weapon))return e.Refuse(12);
            e.stage=Stage::Publish;return false;
        }
        if(e.stage==Stage::Publish){
            std::uintptr_t previous=0;
            if(!cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKiller,bundleJason.pawn.object)||
               !r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,previous)||
               (previous&&previous!=e.playerState)||
               !r_.Read(e.playerState+cfgending::disk::PickedClass,result)||!cfgending::SamePath(result,e.selected)||
               !r_.Read(e.playerState+cfgmenu::birthdisk::ActiveCharacterClass,result)||!cfgending::SamePath(result,e.selected)||
               !r_.Read(e.playerState+cfgending::disk::PickedWeapon,result)||!cfgending::SamePath(result,e.weapon)||
               !cfgobserver::Same(r_,e.playerState+cfgending::disk::PickedSkin,e.skin))return e.Refuse(13);
            if(previous!=e.playerState&&InterlockedCompareExchangePointer(
                reinterpret_cast<void* volatile*>(spawnAssets.gameState+cfgprimary::CurrentKillerState),
                reinterpret_cast<void*>(e.playerState),reinterpret_cast<void*>(previous))!=reinterpret_cast<void*>(previous))return e.Refuse(14);
            e.stage=Stage::Ready;return true;
        }
        return e.Refuse(15);
    }
    bool bundleSkinInFlight()const{return bundleEnding.stage==cfgending::Stage::SkinInFlight;}
