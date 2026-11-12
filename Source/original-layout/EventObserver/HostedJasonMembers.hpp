// H1 hosted Jason takeover: native effects. Included INSIDE BundleAccess (after AdditionalJasonRuntimeMembers).
// Every effect (a) re-proves the hosted world through the RetainedWorld dispatcher, (b) uses the SAME
// retained-call gateway as the F1/F3 births (RetainPrimaryObjects + BeginBundleNative/FinishBundleNative inside a
// verified returned-callback lease) and (c) never replays a dispatched call: a failed/ambiguous dispatch marks the
// lease uncertain (BundleRetentionFailed) so no further native call is admitted. Disabled unless CFG_H1_JASON and
// armed by the operator; offline sessions never reach this code (hostedTx stays empty).
#ifdef CFG_H1_JASON
#if CFG_H22_HOTKEYS
#include "HostedCounselorMembers.hpp"
#endif
    // ---- admission --------------------------------------------------------------------------------------------
    bool HostedWorld(){
        r_.reads=0; // each hosted op is its own separately bounded proof (a one-shot transaction, not recurring AI work)
        return GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&cfgprimary::LookupSafe(r_,plan.module)&&
            cfgprimary::CurrentHostedBinding().Matches(plan)&&cfgprimary::RetainedWorld(r_,plan,context);
    }
    // H2.43 evidence only: the adoption receipt is still current (same check as DonorNativeOwner clause 8).
    bool HostedAdoptionCurrent(){return HostedWorld()&&cfghosted::adoption::Current(r_,context.objectArray,hostedAdoptionReceipt,plan);}
    bool HostedSelectedKillerClass(std::uintptr_t& selectedClass){
        selectedClass=0;const auto pawn=hostedTx.snap.jason;
        cfgprobe::lifetime::SlotObservation before{},after{};
        if(!HostedWorld()||!pawn||!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,before)||!before.HasAssignedSerial()||
           !cfghostedpresentation::SelectedKillerClass(r_,pawn,hostedTx.killerBase,selectedClass)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,pawn,after)||!(before==after))return false;
        return true;
    }
    // Donor OfflineSetup.cpp:8828, hosted adoption at 10331 and F1 at 10452.
    // Primary takeover targets the transaction's newborn PlayerState before
    // possession; extra F1 targets its journaled newborn. Ambiguous native
    // setter entry is never retried.
    bool HostedCopyKillerPresentation(std::uintptr_t targetState,std::uintptr_t sourceState,
        std::uintptr_t sourceController,std::uintptr_t selectedClass,bool extra,bool takeoverBirth,
        std::uintptr_t& skinOut,cfgending::Packet& weaponOut,
        std::uintptr_t& skinBeforeOut,cfgending::Packet& weaponBeforeOut) noexcept {
        skinOut=skinBeforeOut=0;weaponOut=weaponBeforeOut={};
        bundleNativeStage_=0;
        hostedPresentationCopyRefusal_=hostedPresentationCopyBundleNativeStage_=0;
        const auto refuse=[&](unsigned clause)noexcept{hostedPresentationCopyRefusal_=clause;
            hostedPresentationCopyBundleNativeStage_=bundleNativeStage_;return false;};
        if(!HostedWorld())return refuse(1);
        if(!targetState)return refuse(2);
        if(!sourceState)return refuse(3);
        if(!sourceController)return refuse(4);
        if(!selectedClass)return refuse(5);
        if(!cfgending::disk::NativeSettersVerified)return refuse(6);
        if(!cfgbundle::Code(r_,plan.module,cfgending::disk::SetSkin,cfgending::disk::SetSkinCode))return refuse(7);
        if(!cfgbundle::Code(r_,plan.module,cfgending::disk::SetWeapon,cfgending::disk::SetWeaponCode))return refuse(8);
        cfgprobe::lifetime::SlotObservation targetSlot{},sourceSlot{};std::uintptr_t sourceOwner=0,skin=0,skinClass=0;
        std::uintptr_t playerStateClass=0;
        cfgending::Packet weapon{};
        const auto targetCurrent=[&](){return extra?bundleAdditional.journal.killerState==targetState&&AdditionalLinked()&&AdditionalPrimary():
            (takeoverBirth&&hostedTx.birth.state==targetState&&hostedTx.birth.controller&&
             cfgobserver::Same(r_,hostedTx.birth.controller+cfghosted::adoption::ControllerState,targetState)&&
             cfgobserver::Same(r_,targetState+cfghosted::adoption::StateOwner,hostedTx.birth.controller))||
            (!takeoverBirth&&
            cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKillerState,targetState)&&
            cfgobserver::Same(r_,bundleJason.pawn.object+cfgescape::disk::PlayerState,targetState)&&
            cfgobserver::Same(r_,bundleJason.killerController.object+cfgescape::disk::PlayerState,targetState));};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,targetState,targetSlot))return refuse(9);
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,sourceState,sourceSlot))return refuse(10);
        if(!targetCurrent())return refuse(11);
        if(!r_.Read(plan.module+cfgending::disk::SCPlayerStateClass,playerStateClass))return refuse(12);
        if(!cfgobserver::Same(r_,plan.module+cfgending::disk::SCPlayerStateConstructor,playerStateClass))return refuse(13);
        if(!cfgspawn::ObjectIs(r_,targetState,playerStateClass))return refuse(14);
        if(!cfgspawn::ObjectIs(r_,sourceState,playerStateClass))return refuse(15);
        if(!cfghosted::native::WorldOwned(r_,sourceController,plan.world))return refuse(16);
        if(!r_.Read(targetState+cfgending::disk::PickedSkin,skinBeforeOut))return refuse(17);
        if(!r_.Read(targetState+cfgending::disk::PickedWeapon,weaponBeforeOut))return refuse(18);
        if(!r_.Read(sourceState+0x88,sourceOwner))return refuse(19);
        if(sourceOwner!=sourceController)return refuse(20);
        if(!cfgobserver::Same(r_,sourceController+0x388,sourceState))return refuse(21);
        const bool useSelectedDefaults=extra;
        std::uintptr_t sourceSkinField=0;
        cfgending::Packet sourceWeapon{};
        if(useSelectedDefaults){
            // Donor OfflineSetup.cpp:10452 passes useSelectedDefaults=true for
            // F1. Keep the randomly selected extra Jason's own class and use
            // the stock skin plus that class's authored default weapon.
            if(!r_.Read(plan.module+cfgending::disk::SCJasonSkinClass,skin))return refuse(22);
            if(!skin)return refuse(23);
            if(!cfgobserver::Same(r_,plan.module+cfgending::disk::SCJasonSkinConstructor,skin))return refuse(24);
        }else{
            if(!r_.Read(sourceState+cfgending::disk::PickedSkin,skin))return refuse(25);
            sourceSkinField=skin;
            if(!skin){
                if(!r_.Read(plan.module+cfgending::disk::SCJasonSkinClass,skin))return refuse(26);
                if(!skin)return refuse(27);
                if(!cfgobserver::Same(r_,plan.module+cfgending::disk::SCJasonSkinConstructor,skin))return refuse(28);
            }
            if(!r_.Read(sourceState+cfgending::disk::PickedWeapon,sourceWeapon))return refuse(29);
            weapon=sourceWeapon;
        }
        if(!cfgending::skin::Concrete(r_,plan.module,context.objectArray,skin))return refuse(30);
        if(useSelectedDefaults||cfgending::EmptyPath(weapon)){
            if(!r_.Read(selectedClass+cfgspawn::ClassDefaultObject,skinClass))return refuse(31);
            if(!skinClass)return refuse(32);
            if(!r_.Read(skinClass+cfgending::disk::DefaultWeapon,weapon))return refuse(33);
        }
        if(!cfgending::EmptySubpath(weapon))return refuse(34);
        // Current receiver/source proof and object retention are repeated just
        // before each stock setter; no field writes or profile access.
        // H2.43 (CFG_H243_PRESENTATION): roots and slot identity are chosen in
        // Core/HostedPresentationCopy.hpp; the ordered clause sequence is unchanged.
        const auto roots=cfghostedpresent::Roots(targetState,sourceState,sourceController,skin,skinClass,selectedClass,
            cfgending::disk::SetSkin+plan.module,cfgending::disk::SetWeapon+plan.module);
        struct CopyHost {
            BundleAccess& a;const decltype(roots)& roots;const decltype(targetCurrent)& current;
            std::uintptr_t targetState,sourceState,sourceController,skin,sourceSkinField;
            const cfgprobe::lifetime::SlotObservation &targetSlot,&sourceSlot;
            const cfgending::Packet& sourceWeapon;cfgending::Packet& weapon;
            bool SlotOk(const cfgprobe::lifetime::SlotObservation& slot){
#if CFG_H243_PRESENTATION
                return cfgjason::OwnedComponentSlot(a.r_,a.context.objectArray,slot); // serial 0 allowed, never allocated
#else
                return cfgjason::SameSlot(a.r_,a.context.objectArray,slot);
#endif
            }
            bool Retain(){return RetainPrimaryObjects(a.r_,roots);}
            bool World(){return a.HostedWorld();}
            bool TargetSlot(){return SlotOk(targetSlot);}
            bool SourceSlot(){return SlotOk(sourceSlot);}
            bool SourceOwner(){return cfgobserver::Same(a.r_,sourceState+0x88,sourceController);}
            bool SourceController(){return cfgobserver::Same(a.r_,sourceController+0x388,sourceState);}
            bool TargetCurrent(){return current();}
            bool SourceSkinUnchanged(){
                std::uintptr_t live=0;
                return a.r_.Read(sourceState+cfgending::disk::PickedSkin,live)&&live==sourceSkinField&&
                    cfgobserver::Same(a.r_,sourceState+cfgending::disk::PickedSkin,sourceSkinField);
            }
            bool SourceWeaponUnchanged(bool){
                cfgending::Packet live{};
                if(!a.r_.Read(sourceState+cfgending::disk::PickedWeapon,live))return false;
                return cfgending::EmptyPath(sourceWeapon)?cfgending::EmptyPath(live):cfgending::SamePath(live,sourceWeapon);
            }
            bool NativeAllowed(){return a.NativeCallAllowed();}
            bool Begin(){return a.BeginBundleNative();}
            bool Finish(){return a.FinishBundleNative();}
            int CallSkin(){
                ++a.calls_;
                __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(plan.module+cfgending::disk::SetSkin)(reinterpret_cast<void*>(targetState),reinterpret_cast<void*>(skin));}
                __except(EXCEPTION_EXECUTE_HANDLER){a.BundleRetentionFailed();return 2;}
#if CFG_H244_SKIN_BRIDGE
                SkinBridge();
#endif
                return 0;
            }
#if CFG_H244_SKIN_BRIDGE
            // Uses the offline J2.19 publish itself (cfgending::skin::PublishPresentation),
            // only with the offline-world gate replaced by the hosted scope proof.
            void SkinBridge(){
                std::uintptr_t now=1;
                if(!a.r_.Read(targetState+cfgending::disk::PickedSkin,now)||now)return;
                cfgending::skin::PublishPresentation(a.r_,plan.module,a.context.objectArray,plan.world,targetState,
                    bundleJason.killerController.object,bundleJason.pawn.object,sourceState,skin,
                    [&](){return a.HostedWorld();},
                    [&](std::uintptr_t field,std::uintptr_t value){
                        return InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(field),
                            reinterpret_cast<void*>(value),nullptr)==nullptr;
                    },false);
            }
#endif
            bool SkinApplied(){std::uintptr_t after=0;return a.r_.Read(targetState+cfgending::disk::PickedSkin,after)&&after==skin;}
            int CallWeapon(){
                ++a.calls_;
                __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(plan.module+cfgending::disk::SetWeapon)(reinterpret_cast<void*>(targetState),&weapon);return 0;}
                __except(EXCEPTION_EXECUTE_HANDLER){a.BundleRetentionFailed();return 2;}
            }
            bool WeaponApplied(){cfgending::Packet verify{};return a.r_.Read(targetState+cfgending::disk::PickedWeapon,verify)&&cfgending::SamePath(verify,weapon);}
        } copyHost{*this,roots,targetCurrent,targetState,sourceState,sourceController,skin,sourceSkinField,targetSlot,sourceSlot,sourceWeapon,weapon};
        if(const unsigned clause=cfghostedpresent::Sequence(copyHost,useSelectedDefaults))return refuse(clause);
        hostedPresentationCopyRefusal_=0;
        hostedPresentationCopyBundleNativeStage_=bundleNativeStage_;
        skinOut=skin;weaponOut=weapon;return true;
    }
#if CFG_H22_HOTKEYS
    bool HostedRetryAllowed(){return HostedWorld()&&NativeCallAllowed();}
#endif
    template<std::size_t N>bool HostedPrepare(const std::array<std::uintptr_t,N>& inputs,unsigned outputs){
        return HostedWorld()&&NativeCallAllowed()&&RetainPrimaryObjects(r_,inputs)&&NativeCallAllowed()&&BeginBundleNative(outputs);
    }
    std::array<std::uintptr_t,10> HostedRoots(std::uintptr_t extraA=0,std::uintptr_t extraB=0)const{
        const auto& s=hostedTx.snap;
        return {plan.world,plan.mode,hostedTx.gameState,s.jason,s.human,s.humanState,hostedTx.birth.controller,hostedTx.birth.state,extraA,extraB};
    }
    bool HostedInModule(std::uintptr_t address)const{return address>=plan.module&&address-plan.module<cfgprobe::ExecutableImageBytes;}
    // ---- read-only helpers -----------------------------------------------------------------------------------
    bool HostedReadRoster(std::uintptr_t state,std::int32_t& count,bool& contains){
        cfghosted::listen35::LocalPlayers list{};count=-1;contains=false;
        if(!r_.Read(hostedTx.gameState+cfghosted::birth35::PlayerArray,list)||list.count<0||list.count>32||list.capacity<list.count||
           (list.count&&!cfgprobe::ObjectPointer(list.data)))return false;
        for(std::int32_t i=0;i<list.count;++i){std::uintptr_t entry=0;
            if(!r_.Read(list.data+8*static_cast<std::uintptr_t>(i),entry))return false;
            if(state&&entry==state)contains=true;}
        count=list.count;return true;
    }
    bool HostedReadBots(std::int32_t& bots){return r_.Read(plan.mode+cfghosted::birth35::NumBots,bots)&&bots>=0&&bots<=31;}
    bool HostedBotsMemory(std::uintptr_t& address)const{
        address=plan.mode+cfghosted::birth35::NumBots;MEMORY_BASIC_INFORMATION m{};
        return !(address&3)&&VirtualQuery(reinterpret_cast<const void*>(address),&m,sizeof(m))==sizeof(m)&&m.State==MEM_COMMIT&&
            m.Type==MEM_PRIVATE&&m.Protect==PAGE_READWRITE;
    }
    // ---- Ops (cfghostedtx::Takeover) --------------------------------------------------------------------------
    bool HostedHasFullyTraveled(std::uintptr_t humanState){
        namespace d=cfgmenu::populationdisk;
        const auto function=cfgownednative::Resolve(plan.module,plan.module+d::HasFullyTraveled,d::HasFullyTraveledCode);
        if(!function||!HostedPrepare(HostedRoots(plan.controller,plan.pawn),0))return false;
        bool ready=false; // result-only getter: not charged to CallCap so the rollback keeps its 4 calls (forward 8 + rollback 4 <= 12; empty original packet needs no allocating Copy)
        if(!cfgmenu::population::detail::Travel(function,humanState,ready)){BundleRetentionFailed();return false;}
        return FinishBundleNative()&&ready;
    }
    // Donor StaticClass path, through the existing retained callback gateway.
    // Warm in a separate opportunity: no actor birth shares its call budget.
    bool HostedWarmControllerClass(bool killer,std::uintptr_t& klass,bool& warmed){
        warmed=false;const auto& ai=cfgcompat::cfg35::ai::Classes[killer?3:2];namespace d=cfgnativejason::disk;namespace p=cfgmenu::populationdisk;
        auto& tx=hostedTx;tx.classStage=2;
        if(!r_.Read(plan.module+ai.reflection,tx.classReflection)||!r_.Read(plan.module+ai.constructor,tx.classConstructor))return false;
        if(cfgprobe::MatchingClassCache(tx.classReflection,tx.classConstructor)){klass=tx.classConstructor;return true;}
        if((tx.classReflection&&!cfgprobe::ObjectPointer(tx.classReflection))||(tx.classConstructor&&!cfgprobe::ObjectPointer(tx.classConstructor))||
           (tx.classReflection&&tx.classConstructor))return false;
        const auto getter=killer?d::KillerControllerClassGetter:p::CounselorControllerClassGetter;
        const auto factory=killer?d::KillerControllerClassFactory:p::CounselorControllerClassFactory;
        const auto& getterCode=killer?d::KillerControllerClassGetterCode:p::CounselorControllerClassGetterCode;
        const auto& factoryCode=killer?d::KillerControllerClassFactoryCode:p::CounselorControllerClassFactoryCode;
        if(!cfgbundle::Code(r_,plan.module,getter,getterCode)||!cfgbundle::Code(r_,plan.module,factory,factoryCode))return false;
        const auto invoke=[&](std::uintptr_t function,std::uintptr_t input){
            if(!HostedPrepare(HostedRoots(input),1))return false;
            ++calls_;++tx.classWarmCalls;std::uintptr_t result=0;
            if(!cfgmenu::population::detail::GetClass(plan.module+function,result)){BundleRetentionFailed();return false;}
            if(!result){(void)FinishBundleNative();return false;}
            return RetainBundleReturnedObject(result)&&HostedWorld();
        };
        if(!tx.classConstructor){if(!invoke(getter,tx.classReflection))return false;warmed=true;}
        if(!r_.Read(plan.module+ai.constructor,tx.classConstructor)||!cfgprobe::ObjectPointer(tx.classConstructor))return false;
        if(!tx.classReflection){if(!invoke(factory,tx.classConstructor))return false;warmed=true;}
        return cfghosted::native::Class(r_,plan.module,ai.reflection,ai.constructor,klass);
    }
    bool HostedResolveClasses(){
        const auto& s=hostedTx.snap;namespace b=cfghosted::birth35;auto& tx=hostedTx;
        std::uintptr_t vtable=0,restart=0,getter=0,state=0;
        tx.classStage=1;if(!HostedWorld())return false;
        bool warmed=false;if(!HostedWarmControllerClass(true,tx.killerAIClass,warmed))return false;
        if(warmed)return false; // clean NotReady; retry with fresh budget, never replay an ambiguous call
        tx.classStage=3;if(!cfghosted::native::Class(r_,plan.module,cfgmenu::birthdisk::SCKillerCharacterClass,cfgmenu::birthdisk::SCKillerCharacterConstructor,tx.killerBase))return false;
        tx.classStage=4;if(!cfghosted::native::Class(r_,plan.module,cfgmenu::birthdisk::SCPlayerStateClass,cfgmenu::birthdisk::SCPlayerStateConstructor,tx.playerStateClass))return false;
        tx.classStage=5;if(!cfgnativejason::SpawnCode(r_,plan.module))return false;
        tx.classStage=6;if(!cfgnativejason::PossessCode(r_,plan.module))return false;
        tx.classStage=7;if(!cfgbundle::Code(r_,plan.module,b::Restart,b::RestartCode))return false;
        tx.classStage=8;if(!cfgbundle::Code(r_,plan.module,b::Getter,b::GetterCode))return false;
        tx.classStage=9;if(!cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::CopyConstruct,cfgmenu::birthdisk::CopyConstructCode))return false;
        tx.classStage=10;if(!cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::ConsumeActive,cfgmenu::birthdisk::ConsumeActiveCode))return false;
        tx.classStage=11;if(!cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::ActiveNotify,cfgmenu::birthdisk::ActiveNotifyCode))return false;
        tx.classStage=12;if(!cfgbundle::Code(r_,plan.module,cfgadditional::disk::DestroyExecutor,cfgadditional::disk::DestroyCode))return false;
        tx.classStage=13;if(!r_.Read(s.mode,vtable)||!r_.Read(vtable+b::RestartSlot,restart)||restart!=plan.module+b::Restart||
            !r_.Read(vtable+b::GetterSlot,getter)||getter!=plan.module+b::Getter||vtable!=plan.module+b::HuntVtable)return false;
        tx.classStage=14;
        const std::array<std::uintptr_t,1> choices{s.humanState};
        const bool preferred=cfghostedpacket::Preferred(r_,plan.module,s.humanState,tx.counselorSource,tx.packetDiagnostic,s.mode);
        tx.counselorPreferencePreflight=preferred?1u:0u;tx.counselorPreferencePreflightReason=preferred?0u:tx.packetDiagnostic.reason;
        if(!preferred&&!cfghostedpacket::Choose(r_,plan.module,s.mode,choices,1,static_cast<unsigned>(GetTickCount64()),tx.counselorSource,tx.packetDiagnostic))return false;
        tx.counselorPreferencePreflightKind=tx.counselorSource.kind;tx.counselorPreferencePreflightIndex=tx.counselorSource.index;
        tx.classStage=15;if(!r_.Read(s.gameState,state))return false;
        tx.restartFn=restart;tx.classStage=0;return true;
    }
    bool HostedCopyPacket(std::uintptr_t source,cfgmenu::promotion::Packet& destination){
        if(!HostedPrepare(HostedRoots(),0))return false;
        ++calls_;
        if(!cfgmenu::promotion::detail::Copy(plan.module+cfgmenu::birthdisk::CopyConstruct,&destination,reinterpret_cast<const void*>(source))){
            BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
    bool HostedCopyCounselor(const cfghostedpacket::Source& source,cfgmenu::promotion::Packet& destination,cfghostedpacket::Diagnostic& diagnostic){
        if(!HostedWorld()||!cfghostedpacket::Current(r_,plan.module,source,diagnostic)||
           !HostedPrepare(HostedRoots(source.owner),0)||!cfghostedpacket::Current(r_,plan.module,source,diagnostic))return false;
        // Local observed packet, no borrowed array/FString pointer survives an
        // allocating copy. Native copy/consumer preserve their own ownership ABI.
        const auto packet=source.packet;++calls_;
        if(!cfgmenu::promotion::detail::Copy(plan.module+cfgmenu::birthdisk::CopyConstruct,&destination,&packet)){BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
    bool HostedConsumeActive(std::uintptr_t playerState,cfgmenu::promotion::Packet& packet,bool* entered=nullptr){
        if(entered)*entered=false;
        MEMORY_BASIC_INFORMATION m{};const auto at=playerState+cfgmenu::birthdisk::ActiveCharacterClass;
        if(VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||
           (m.Protect!=PAGE_READWRITE&&m.Protect!=PAGE_WRITECOPY)||at-reinterpret_cast<std::uintptr_t>(m.BaseAddress)>m.RegionSize||
           m.RegionSize-(at-reinterpret_cast<std::uintptr_t>(m.BaseAddress))<sizeof(packet))return false;
        if(!HostedPrepare(HostedRoots(),0))return false;
        if(entered)*entered=true;
        ++calls_;
        // Ownership transfers at call entry: never Destroy this packet afterwards.
        if(!cfgmenu::promotion::detail::Two(plan.module+cfgmenu::birthdisk::ConsumeActive,playerState,&packet)){BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
    bool HostedSpawnAi(cfghostedtx::Birth& birth){
        birth={};const auto ai=hostedTx.killerAIClass;
        if(!ai||!cfgnativejason::SpawnCode(r_,plan.module)||!HostedPrepare(HostedRoots(ai),1))return false;
        std::uintptr_t created=0;cfgnativejason::Fault fault;cfgspawn::Vec3 at{},rotation{};
        ++calls_;birth.attempted=true;
        const bool returned=cfgnativejason::detail::Spawn(plan.module+cfgmenu::populationdisk::SpawnParametersConstructor,
            plan.module+cfgmenu::populationdisk::SpawnActor,plan.world,ai,&at,&rotation,created,fault,2);
        birth.controller=created;
        if(!returned||fault.code){BundleRetentionFailed();return false;}
        if(!created||!RetainBundleReturnedObject(created))return false;
        std::uint32_t flags=0;std::uintptr_t state=0;
        if(!HostedWorld()||!cfgspawn::ObjectIs(r_,created,ai)||!r_.Read(created+cfghosted::adoption::ControllerState,state)||
           !cfgprobe::ObjectPointer(state)||!r_.Read(state+cfghosted::birth35::BotFlags,flags))return false;
        birth.state=state;birth.botFlag=(flags&cfghosted::birth35::BotMask)!=0;
        hostedTx.birth=birth;
        return cfghosted::native::WorldOwned(r_,created,plan.world)&&cfgobserver::Same(r_,state+cfghosted::adoption::StateOwner,created);
    }
    bool HostedPathReady(const cfghostedtx::Birth& birth){
        // Spawn must have produced the exact owned PathFollowingComponent that
        // adoption will later retain. This flow has no verified native path
        // constructor/initializer to call, so a missing component is a clean,
        // early rollback condition rather than a guessed initialization call.
        const auto& binding=cfgjason::disk::Classes[3];std::uintptr_t path=0,type=0;
        cfgprobe::lifetime::SlotObservation slot{},again{};
        if(!HostedWorld()||!birth.controller||!r_.Read(birth.controller+cfgjason::disk::PathFollowing,path))return false;
        hostedTx.aiPathAtSpawn=path;cfgclosedtrace::Mark("h223-path-after-spawn",birth.controller,path);
        return cfgprobe::ObjectPointer(path)&&
            cfghosted::native::Class(r_,plan.module,binding.reflection,binding.constructor,type)&&
            cfgspawn::ObjectIs(r_,path,type)&&cfgobserver::Same(r_,path+0x20,birth.controller)&&
            cfgobserver::Same(r_,path+cfgjason::disk::ComponentOwner,birth.controller)&&
            r_.Read(birth.controller+cfgjason::disk::PathFollowing,hostedTx.aiPath)&&hostedTx.aiPath==path&&
            cfgspawn::ObjectSlot(r_,context.objectArray,path,slot)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,path,again)&&slot==again&&
            cfgobserver::Same(r_,path+0x20,birth.controller)&&
            cfgobserver::Same(r_,path+cfgjason::disk::ComponentOwner,birth.controller);
    }
    bool HostedLocalCallbackCurrent(const cfgprobe::lifetime::SlotObservation& controller,
        const cfgprobe::lifetime::SlotObservation& pawn){
        return HostedWorld()&&plan.controller==controller.object&&plan.pawn==pawn.object&&
            cfgjason::SameSlot(r_,context.objectArray,controller)&&cfgjason::SameSlot(r_,context.objectArray,pawn)&&
            cfgobserver::Same(r_,controller.object+cfgprobe::ControllerPawn,pawn.object)&&
            cfgobserver::Same(r_,pawn.object+cfgescape::disk::Controller,controller.object)&&
            cfghosted::native::WorldOwned(r_,pawn.object,plan.world);
    }
    bool HostedAccountBot(std::int32_t before){
        std::uintptr_t address=0;std::int32_t now=-1;
        if(!HostedBotsMemory(address)||!HostedReadBots(now)||now!=before)return false;
        return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(address),before+1,before)==before;
    }
    bool HostedUnaccountBot(std::int32_t current){
        std::uintptr_t address=0;std::int32_t now=-1;
        if(!HostedBotsMemory(address)||!HostedReadBots(now)||now!=current||current<1)return false;
        return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(address),current-1,current)==current;
    }
    // Donor AI SetActive(killer) uses the existing verified native consumer;
    // HostedOps performs the donor presentation copy immediately afterward.
    bool HostedPrepareAi(const cfghostedtx::Birth& birth){
        std::uintptr_t owner=0;cfgmenu::promotion::Packet packet{},again{};
        if(!HostedWorld()||!birth.controller||!birth.state||!birth.botFlag||!cfgspawn::ObjectIs(r_,birth.state,hostedTx.playerStateClass)||
           !r_.Read(birth.state+cfghosted::adoption::StateOwner,owner)||owner!=birth.controller||
           !r_.Read(hostedTx.snap.humanState+cfgmenu::birthdisk::ActiveCharacterClass,packet)||!cfgending::EmptySubpath(packet.bytes)||
           !r_.Read(hostedTx.snap.humanState+cfgmenu::birthdisk::ActiveCharacterClass,again)||!cfgending::SamePath(packet.bytes,again.bytes))return false;
        const auto expected=packet.bytes;
        return HostedConsumeActive(birth.state,packet)&&r_.Read(birth.state+cfgmenu::birthdisk::ActiveCharacterClass,again)&&
            cfgending::SamePath(again.bytes,expected);
    }
    bool HostedAiBirthReady(const cfghostedtx::Birth& birth,std::uintptr_t jason){
        std::int32_t count=-1;bool contains=false;std::uintptr_t value=0;std::uint32_t flags=0;
        if(!HostedWorld()||!birth.controller||!birth.state||!HostedReadRoster(birth.state,count,contains)||!contains||
           !cfgspawn::ObjectIs(r_,birth.controller,hostedTx.killerAIClass)||
           !r_.Read(birth.controller+cfghosted::adoption::ControllerState,value)||value!=birth.state)return false;
        if(!r_.Read(birth.state+cfghosted::birth35::BotFlags,flags)||!(flags&cfghosted::birth35::BotMask))return false;
        if(!jason)return true;
        return cfgobserver::Same(r_,jason+cfghosted::adoption::PawnController,birth.controller)&&
            cfgobserver::Same(r_,birth.controller+cfghosted::adoption::ControllerPawn,jason);
    }
    bool HostedPrepareHuman(std::uintptr_t humanState){
        namespace d=cfgmenu::birthdisk;
        // Ordinary soft-class packets have no owned FString subpath. Save that
        // verified by-value ABI directly (same native consumer as ending metadata),
        // reserving the allocating-copy budget for the selected counselor packet.
        if(!HostedWorld()||!r_.Read(humanState+d::ActiveCharacterClass,hostedTx.savedActive)||
           !cfgending::EmptySubpath(hostedTx.savedActive.bytes))return false;
        hostedTx.haveSaved=true;
        // Refresh the displaced player's own donor PickedCounselorClass at the
        // handoff. If that observation is unavailable, keep the already-proven
        // preflight packet and report the refusal without blocking the takeover.
        cfghostedpacket::Source restartSource=hostedTx.counselorSource,preferred{};
        cfghostedpacket::Diagnostic preferenceDiagnostic{};
        const bool preferredCurrent=cfghostedpacket::Preferred(r_,plan.module,humanState,preferred,preferenceDiagnostic,plan.mode);
        hostedTx.counselorPreferenceHandoff=preferredCurrent?1u:0u;
        hostedTx.counselorPreferenceHandoffReason=preferredCurrent?0u:preferenceDiagnostic.reason;
        if(preferredCurrent)restartSource=preferred;
        hostedTx.counselorPreferenceHandoffKind=restartSource.kind;hostedTx.counselorPreferenceHandoffIndex=restartSource.index;
        if(!HostedCopyCounselor(restartSource,hostedTx.counselorPacket,hostedTx.packetDiagnostic))return false;
        if(preferredCurrent)hostedTx.counselorSource=preferred;
        else hostedTx.packetDiagnostic=preferenceDiagnostic;
        hostedTx.haveCounselor=true;hostedTx.counselorConsumed=true;
        return HostedConsumeActive(humanState,hostedTx.counselorPacket)&&
            r_.Read(humanState+d::ActiveCharacterClass,hostedTx.expectedActive)&&
            cfgending::EmptySubpath(hostedTx.expectedActive.bytes)&&
            cfgending::SamePath(hostedTx.expectedActive.bytes,hostedTx.counselorPacket.bytes);
    }
    bool HostedNotifyAiTraveled(const cfghosted::adoption::Receipt& receipt){
        namespace p=cfgmenu::populationdisk;bool ready=false;
        const auto fn=cfgownednative::Resolve(plan.module,plan.module+p::HasFullyTraveled,p::HasFullyTraveledCode);
        if(!fn||!HostedPrepare(HostedRoots(receipt.controller.object,receipt.controllerState),0))return false;
        ++calls_;
        if(!cfgmenu::population::detail::Travel(fn,receipt.controllerState,ready)){BundleRetentionFailed();return false;}
        return FinishBundleNative(); // notification, not a boolean gate (same donor F3 path)
    }
    bool HostedQueueClassRestore(){
        auto& ticket=hostedClassRestore;
        if(ticket.pending||!hostedTx.haveSaved||!cfgending::EmptySubpath(hostedTx.savedActive.bytes)||
           !cfgending::EmptySubpath(hostedTx.expectedActive.bytes)||!HostedWorld())return false;
        cfgprobe::lifetime::SlotObservation controller{},state{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,hostedTx.snap.human,controller)||!controller.HasAssignedSerial()||
           !cfghosted::takeover35::ObservedTyped(r_,context.objectArray,hostedTx.snap.humanState,hostedTx.playerStateClass,state)||
           !cfgobserver::Same(r_,controller.object+cfghosted::adoption::ControllerState,state.object)||
           !cfgobserver::Same(r_,state.object+cfghosted::adoption::StateOwner,controller.object))return false;
        ticket={};ticket.pending=true;ticket.world=plan.world;ticket.mode=plan.mode;ticket.gameState=hostedTx.gameState;
        ticket.state=state.object;ticket.controller=controller;ticket.stateSlot=state;
        // On a native Jason host ActiveCharacterClass can still name Jason.
        // The donor restores the human's separately selected counselor from
        // PickedCounselorClass after the AI takeover; the transaction's saved
        // ActiveCharacterClass remains reserved for verified rollback.
        ticket.original=hostedTx.counselorPacket;ticket.expected=hostedTx.expectedActive;
        // Transfer ownership of the native empty-subpath copy, exactly once.
        hostedTx.haveSaved=false;hostedTx.savedActive={};return true;
    }
    bool HostedRestoreClassAtEnd(){
        auto& t=hostedClassRestore;
        if(!t.pending||!t.due||t.world!=plan.world||t.mode!=plan.mode||!HostedWorld())return false;
        const auto now=GetTickCount64();if(now<t.next)return false;t.next=now+250;
        ++t.attempts;std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation state{};
        cfgnames::Words word{};cfgnames::Text phase;cfgnames::Observation<Reader> names(r_,plan.module);
        cfgmenu::promotion::Packet current{};std::int32_t count=0;bool present=false;
        const auto refuse=[&](unsigned reason){t.lastRefusal=reason;++t.refused;return false;};
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,word)||!names.Decode(word,phase)||phase.View()==u"InProgress")return refuse(1);
        if(!cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,t.gameState))return refuse(2);
        if(!cfgjason::SameSlot(r_,context.objectArray,t.controller))return refuse(3);
        if(!cfghosted::native::WorldOwned(r_,t.controller.object,t.world))return refuse(4);
        if(!cfghosted::native::Class(r_,plan.module,cfgmenu::birthdisk::SCPlayerStateClass,cfgmenu::birthdisk::SCPlayerStateConstructor,type))return refuse(5);
        if(!cfghosted::takeover35::ObservedTyped(r_,context.objectArray,t.state,type,state)||!(state==t.stateSlot))return refuse(6);
        if(!cfgobserver::Same(r_,t.controller.object+cfghosted::adoption::ControllerState,t.state))return refuse(7);
        if(!cfgobserver::Same(r_,t.state+cfghosted::adoption::StateOwner,t.controller.object))return refuse(8);
        if(!HostedReadRoster(t.state,count,present)||!present)return refuse(9);
        if(!r_.Read(t.state+cfgmenu::birthdisk::ActiveCharacterClass,current))return refuse(10);
        if(!cfgending::SamePath(current.bytes,t.expected.bytes)){
            ++t.changed;t.pending=false;return true; // native/user replacement wins
        }
        if(!cfgbundle::Code(r_,plan.module,cfgmenu::birthdisk::ConsumeActive,cfgmenu::birthdisk::ConsumeActiveCode))return refuse(11);
        if(!NativeCallAllowed())return refuse(12);
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,5>{t.world,t.mode,t.gameState,t.controller.object,t.state}))return refuse(13);
        if(!r_.Read(t.state+cfgmenu::birthdisk::ActiveCharacterClass,current)||!cfgending::SamePath(current.bytes,t.expected.bytes))return refuse(14);
        if(!BeginBundleNative())return refuse(15);
        const auto original=t.original;t.pending=false;t.original={}; // entered: no replay
        auto packet=original;++calls_;
        if(!cfgmenu::promotion::detail::Two(plan.module+cfgmenu::birthdisk::ConsumeActive,t.state,&packet)){
            BundleRetentionFailed();return refuse(16);}
        if(!FinishBundleNative()||!r_.Read(t.state+cfgmenu::birthdisk::ActiveCharacterClass,current)||
           !cfgending::SamePath(current.bytes,original.bytes))return refuse(16);
        ++t.restored;return true;
    }
    bool HostedRestoreHumanClass(std::uintptr_t humanState){
        if(!hostedTx.haveSaved)return true;       // nothing was changed
        hostedTx.haveSaved=false;                  // single consumption, never replayed
        return HostedConsumeActive(humanState,hostedTx.savedActive);
    }
    bool HostedPossessAi(const cfghostedtx::Birth& birth,std::uintptr_t jason,bool& entered){
        entered=false;cfgnativejason::Fault fault;
        if(!cfgnativejason::PossessCode(r_,plan.module)||!HostedPrepare(HostedRoots(),0))return false;
        ++calls_;entered=true;
        if(!cfgnativejason::detail::Possess(plan.module+cfgnativejason::disk::BasePossess,birth.controller,jason,fault)||fault.code){
            BundleRetentionFailed();return false;}
        if(!FinishBundleNative())return false;
        return HostedAiPathAfterPossess(birth);
    }
    // H2.55b: shared by the takeover's Possess and the preborn host's RestartPlayer birth. Live H2.55: without the crowd setter the
    // AI Jason's moves were accepted (Result=2) but he never walked, while teleports, slashes and knives worked.
    bool HostedAiPathAfterPossess(const cfghostedtx::Birth& birth){
        std::uintptr_t path=0;cfgprobe::lifetime::SlotObservation pathSlot{};
        if(!r_.Read(birth.controller+cfgjason::disk::PathFollowing,path))return false;
        hostedTx.aiPathAfterPossess=path;cfgclosedtrace::Mark("h223-path-after-possess",birth.controller,path);
        const auto& pathBinding=cfgjason::disk::Classes[3];std::uintptr_t pathType=0;
        if(!cfgprobe::ObjectPointer(path)||!cfghosted::native::Class(r_,plan.module,pathBinding.reflection,pathBinding.constructor,pathType)||
           !cfgspawn::ObjectIs(r_,path,pathType)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,path,pathSlot)||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,pathSlot)||
           !cfgobserver::Same(r_,path+cfgjason::disk::ComponentOwner,birth.controller)||
           !cfgobserver::Same(r_,path+0x20,birth.controller))return false;
        hostedTx.aiPath=path; // retain the donor's dynamic post-possession path component
        // This setter is virtual only on the verified crowd receiver. Ordinary
        // PathFollowing components are already ready and must skip the optional call.
        cfgcrowd::Observation before{},after{};
        const auto observeCurrentPath=[&](cfgcrowd::Observation& observation){
            const auto backlink=birth.controller+cfgjason::disk::PathFollowing;
            return HostedWorld()&&cfgobserver::Same(r_,backlink,path)&&
                cfgcrowd::Observe(r_,plan.module,context.objectArray,pathSlot,birth.controller,observation)&&
                cfgobserver::Same(r_,backlink,path);
        };
        if(observeCurrentPath(before)){
            const auto action=cfgcrowd::OptionalSetter(before);
            if(action==cfgcrowd::OptionalSetterAction::Skip)hostedTx.crowd=1;
            else if(action==cfgcrowd::OptionalSetterAction::Apply&&
                    cfgbundle::Code(r_,plan.module,cfgcrowd::Setter,cfgcrowd::SetterBytes)&&
                    cfgbundle::Code(r_,plan.module,cfgcrowd::CrowdFollow,cfgcrowd::FollowBytes)&&
                    HostedPrepare(HostedRoots(path),0)){
                cfgcrowd::Observation retained{};
                if(!observeCurrentPath(retained)||retained.kind!=before.kind||retained.state!=before.state||
                   retained.registered!=before.registered||cfgcrowd::OptionalSetter(retained)!=action){
                    (void)FinishBundleNative();
                    return false;
                }
                ++calls_; // consume before entry; an uncertain setter is never replayed
                if(!HostedLeafCrowdSetter(plan.module+cfgcrowd::Setter,path,cfgcrowd::DisabledState)){
                    BundleRetentionFailed();hostedTx.crowd=2;return false;
                }
                if(!FinishBundleNative()||!observeCurrentPath(after)||!cfgcrowd::SetterPostcondition(before,after)){
                    hostedTx.crowd=2;return false;
                }
                hostedTx.crowd=1;
            }
        }
        return true;
    }
    // Rollback re-possession of the human PlayerController by virtual dispatch: find the base Possess slot in the
    // AI controller's vtable and take the same slot from the human's vtable; else the base entry itself.
    bool HostedPossessHuman(std::uintptr_t human,std::uintptr_t jason,bool& entered){
        entered=false;cfgnativejason::Fault fault;std::uintptr_t aiTable=0,humanTable=0,entry=plan.module+cfgnativejason::disk::BasePossess;
        if(hostedTx.birth.controller&&r_.Read(hostedTx.birth.controller,aiTable)&&r_.Read(human,humanTable)&&cfgprobe::ObjectPointer(aiTable)&&
           cfgprobe::ObjectPointer(humanTable)){
            for(unsigned slot=0;slot<320;++slot){std::uintptr_t value=0;
                if(!r_.Read(aiTable+8ULL*slot,value))break;
                if(value==entry){std::uintptr_t candidate=0;
                    if(r_.Read(humanTable+8ULL*slot,candidate)&&HostedInModule(candidate))entry=candidate;
                    break;}}
        }
        if(!cfgnativejason::PossessCode(r_,plan.module)||!HostedInModule(entry)||!HostedPrepare(HostedRoots(),0))return false;
        ++calls_;entered=true;
        if(!cfgnativejason::detail::Possess(entry,human,jason,fault)||fault.code){BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
    bool HostedRestart(std::uintptr_t mode,std::uintptr_t human,bool& entered){
        entered=false;
        if(!hostedTx.restartFn||!HostedPrepare(HostedRoots(plan.controller,plan.pawn),1))return false;
        ++calls_;entered=true;
        if(!cfgmenu::promotion::detail::Two(hostedTx.restartFn,mode,reinterpret_cast<void*>(human))){BundleRetentionFailed();return false;}
        std::uintptr_t pawn=0;
        if(r_.Read(human+cfghosted::adoption::ControllerPawn,pawn)&&pawn&&pawn!=hostedTx.snap.jason)return RetainBundleReturnedObject(pawn);
        return FinishBundleNative();
    }
    bool HostedReadPost(const cfghostedtx::Snapshot& s,const cfghostedtx::Birth& birth,cfghostedtx::Post& post){
        post={};std::uintptr_t counselorClass=0;
        cfgmenu::promotion::Packet activeCounselor{};
        if(!HostedWorld()||!r_.Read(s.jason+cfghosted::adoption::PawnController,post.jasonController)||
           !r_.Read(s.jason+cfghosted::adoption::PawnState,post.jasonPawnState)||
           !r_.Read(s.human+cfghosted::adoption::ControllerPawn,post.counselor)||
           !r_.Read(s.human+cfghosted::adoption::ControllerState,post.humanControllerState)||
           !r_.Read(hostedTx.gameState+cfghosted::birth35::CurrentKiller,post.currentKiller)||
           !r_.Read(hostedTx.gameState+cfghosted::birth35::CurrentKillerState,post.currentKillerState)||
           !HostedReadBots(post.bots))return false;
        // RestartPlayer can reset a remote controller's active class. Record
        // that discrepancy, but never roll back an otherwise valid Jason
        // takeover for a cosmetic counselor-class mismatch.
        if(hostedTx.haveSaved&&r_.Read(s.humanState+cfgmenu::birthdisk::ActiveCharacterClass,activeCounselor)&&
           !cfgending::SamePath(activeCounselor.bytes,hostedTx.expectedActive.bytes))hostedTx.packetDiagnostic.reason=11;
        if(birth.controller&&!r_.Read(birth.controller+cfghosted::adoption::ControllerPawn,post.aiPawn))return false;
        bool contains=false;if(!HostedReadRoster(birth.state,post.players,contains))return false;post.aiRegistered=birth.state&&contains;
        if(post.counselor&&post.counselor!=s.jason){
            post.counselorInWorld=cfghosted::native::WorldOwned(r_,post.counselor,plan.world);
            if(!r_.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselorClass))return false;
            post.counselorIsCounselor=post.counselorInWorld&&cfgspawn::ObjectIs(r_,post.counselor,counselorClass);
            if(!r_.Read(post.counselor+cfghosted::adoption::PawnController,post.counselorController))return false;
        }else if(post.counselor==s.jason)post.counselorController=post.jasonController;
        // LocalCounselor is resolved AFTER RestartPlayer for both host-picked
        // and remote-picked paths. No cached pre-restart pawn is accepted.
        std::uintptr_t local=0;cfgprobe::lifetime::SlotObservation localSlot;
        if(r_.Read(s.localController+cfgprobe::ControllerPawn,local)&&local&&local!=s.jason&&
           cfgspawn::ObjectSlot(r_,context.objectArray,local,localSlot)&&localSlot.HasAssignedSerial()&&
           counselorClass&&cfgspawn::ObjectIs(r_,local,counselorClass)&&
           cfghosted::native::WorldOwned(r_,local,s.world)&&
           cfgobserver::Same(r_,local+cfgescape::disk::Controller,s.localController)&&
           cfgjason::SameSlot(r_,context.objectArray,localSlot)){
            post.localCounselor=local;post.localCounselorProven=true;
        }
        return true;
    }
    bool HostedInWorld(std::uintptr_t actor){return actor&&cfghosted::native::WorldOwned(r_,actor,plan.world);}
    // Destroy via the actor's virtual K2_DestroyActor (the pinned exec thunk's own tail call: vtable+0x2D0).
    bool HostedDestroy(std::uintptr_t actor){
        std::uintptr_t table=0,entry=0;
        if(!actor||!cfgbundle::Code(r_,plan.module,cfgadditional::disk::DestroyExecutor,cfgadditional::disk::DestroyCode)||
           !r_.Read(actor,table)||!cfgprobe::ObjectPointer(table)||!r_.Read(table+0x2D0,entry)||!HostedInModule(entry)||
           !HostedPrepare(HostedRoots(actor),0))return false;
        ++calls_;
        if(!HostedLeafVirtualDestroy(entry,actor)){BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
#if CFG_H255_HOST_AI_BIRTH
    bool HostedPrepareAiPreborn(const cfghostedtx::Birth& birth){
        std::uintptr_t owner=0;cfgmenu::promotion::Packet again{};
        if(!hostedPreborn.saved||!HostedWorld()||!birth.controller||!birth.state||!birth.botFlag||!cfgspawn::ObjectIs(r_,birth.state,hostedTx.playerStateClass)||
           !r_.Read(birth.state+cfghosted::adoption::StateOwner,owner)||owner!=birth.controller||!cfgending::EmptySubpath(hostedPreborn.killer.bytes))return false;
        auto packet=hostedPreborn.killer;const auto expected=packet.bytes;
        return HostedConsumeActive(birth.state,packet)&&r_.Read(birth.state+cfgmenu::birthdisk::ActiveCharacterClass,again)&&cfgending::SamePath(again.bytes,expected);
    }
    bool HostedControllerPawn(std::uintptr_t controller,std::uintptr_t& pawn){pawn=0;return HostedWorld()&&r_.Read(controller+cfghosted::adoption::ControllerPawn,pawn);}
    bool HostedPrebornKillerReady(const cfghostedtx::Birth& birth,std::uintptr_t jason){
        return HostedWorld()&&cfgspawn::ObjectIs(r_,jason,hostedTx.killerBase)&&cfghosted::native::WorldOwned(r_,jason,plan.world)&&HostedAiBirthReady(birth,jason);
    }
    // Publishes the AI as the killer once it owns a real Jason. Compare-exchange only, and read back.
    bool HostedPublishPrebornKiller(std::uintptr_t humanState,std::uintptr_t aiState,std::uintptr_t jason){
        namespace b=cfghosted::birth35;std::uintptr_t pawn=0,state=0;const auto gameState=hostedTx.gameState;
        if(!HostedWorld()||!r_.Read(gameState+b::CurrentKiller,pawn)||!r_.Read(gameState+b::CurrentKillerState,state)||(state!=humanState&&state!=aiState))return false;
        if(pawn!=jason&&!cfgadditionalwrite::Exchange(gameState+b::CurrentKiller,pawn,jason))return false;
        if(state!=aiState&&!cfgadditionalwrite::Exchange(gameState+b::CurrentKillerState,state,aiState))return false;
        return cfgobserver::Same(r_,gameState+b::CurrentKiller,jason)&&cfgobserver::Same(r_,gameState+b::CurrentKillerState,aiState);
    }
#endif
    bool HostedWriteKillerOwner(std::uintptr_t expected,std::uintptr_t desired){
        return HostedWorld()&&cfgadditionalwrite::Exchange(hostedTx.gameState+cfghosted::birth35::CurrentKillerState,expected,desired);
    }
    bool HostedSlot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){
        return cfgspawn::ObjectSlot(r_,context.objectArray,object,slot)&&slot.HasAssignedSerial();
    }
#endif
