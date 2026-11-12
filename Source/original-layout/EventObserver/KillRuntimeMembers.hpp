// Included inside BundleAccess. Native calls and reads share the normal budget.
    bool RequiresFinishingStrike()const noexcept{return true;}
#include "KillMeleeRuntimeMembers.hpp"
#include "KillTraceRuntimeMembers.hpp"
    std::uintptr_t observedHeldPawn_=0,observedHeldWeapon_=0;
    bool HeldWeapon(std::uintptr_t pawn,std::uintptr_t weapon){
        std::uintptr_t native=0;
        return NativeClass(cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,native)&&
            cfgkill::OwnedHeldWeapon(r_,context.objectArray,pawn,weapon,native);
    }
    bool Scope(cfgkill::Identity& identity){
        identity={};if(!Context()||!Jason())return false;
        identity={plan.world,bundleJason.pawn.object,bundleJason.pawn.serial};return identity.Valid();
    }
    bool KnownLiving(std::uintptr_t pawn){
        for(unsigned i=0;i<=bundleReport.bots;++i){std::uintptr_t candidate=0;bool alive=false;
            if(!i&&pawn!=(bundleHumanTarget.object?bundleHumanTarget.object:plan.pawn))continue;
            if(i&&bundleReport.roster[i-1].pawn!=pawn)continue;
            if(Known(i,candidate)&&candidate==pawn&&Alive(candidate,alive)&&alive)return true;}
        return false;
    }
    bool DamageCounselor(std::uintptr_t controller,std::uintptr_t& pawn){
        pawn=0;
        for(unsigned i=0;i<=bundleReport.bots;++i){
            if((i?bundleReport.roster[i-1].controller:plan.controller)!=controller)continue;
            return Known(i,pawn)&&cfgobserver::Same(r_,pawn+0x3A0,controller)&&cfgobserver::Same(r_,controller+0x370,pawn);
        }return false;
    }
    bool MaskOff(bool& off){
        off=false;unsigned char value=2;
        if(!cfgbundle::Code(r_,plan.module,cfgkill::disk::bMaskOnSetter,cfgkill::disk::bMaskOnSetterCode)||
            !r_.Read(bundleJason.pawn.object+cfgkill::disk::bMaskOn,value)||value>1)return false;
        off=value==0;return true;
    }
    bool InStun(bool& stunned){
        stunned=false;unsigned char value=2;
        if(!cfgbundle::Code(r_,plan.module,cfgkill::disk::bInStunSetter,cfgkill::disk::bInStunSetterCode)||
            !r_.Read(bundleJason.pawn.object+cfgkill::disk::bInStun,value)||value>1)return false;
        stunned=value==1;return true;
    }
    bool KillRecovery(std::uint64_t now){
        auto& policy=bundleKill.policy;bundleReport.killRecoveryFailure=1;
        cfgkill::Identity identity{};cfgkill::Context death{};bool dying=false,stunned=false;
        if(!policy.RecoveryPending(now)||!Scope(identity)||!DeathScope(dying)||!DeathContext(death))return false;
        if(death.active){policy.nativeActiveObserved=true;bundleReport.killRecoveryFailure=2;return false;}
        if(!policy.RecoveryRuntimeProof(now,identity,HostedAdoptionOwnsDriver(),dying,death.actor,death.active,
            pamelaStoppedContext_,pamelaStoppedUseAt_))return false;
        // An observed native cinematic owns this pawn permanently for this
        // transaction unless the hosted native-only timeout path has the full
        // fresh identity/owner/death/context proof above.
        if(!InStun(stunned))return false;
        if(policy.recoveryAttempted){
            if(!policy.recoveryReturned||stunned)return false;
            const bool recovered=policy.ConfirmRecovery(now,true,true);bundleReport.killRecoveryFailure=recovered?0u:2u;return recovered;
        }
        if(!stunned){
            if(!policy.RequestRecovery(now))return false;
            policy.recoveryReturned=true;policy.ConfirmRecovery(now,true,true);bundleReport.killRecoveryFailure=0;return true;
        }
        using namespace cfgofflinerecovery::disk;
        std::uintptr_t function=0,owner=0,table=0;
        bundleReport.killRecoveryFailure=3;
        if(!FeatureFunction(EndStun,function,owner)||!cfgspawn::ObjectIs(r_,identity.jason,owner)||
            !cfgobserver::Same(r_,function+0xB0,plan.module+EndStunExecutor)||
            !cfgbundle::Code(r_,plan.module,EndStunExecutor,EndStunExecutorCode)||
            !cfgbundle::Code(r_,plan.module,EndStunBody,EndStunBodyCode)||
            !r_.Read(identity.jason,table)||!cfgobserver::Same(r_,table+EndStunSlot,plan.module+EndStunBody)||
            !Scope(identity)||!(identity==policy.identity)||!DeathScope(dying)||dying||
            !DeathContext(death)||death.active||(death.actor&&death.actor!=policy.context))return false;
        if(!policy.RequestRecovery(now))return false;
        ++bundleReport.killRecoveryAttempts;
        const bool returned=Invoke(identity.jason,function,nullptr);
        if(returned){++bundleReport.killRecoveryReturns;policy.recoveryReturned=true;}
        bundleReport.killRecoveryFailure=returned?0u:4u;return returned;
    }
    bool CurrentWeapon(std::uintptr_t pawn,std::uintptr_t& weapon,cfgkill::Weapon& kind){
        observedHeldPawn_=observedHeldWeapon_=0;
        weapon=0;kind=cfgkill::Weapon::Other;std::uintptr_t function=0,owner=0;
        if(!KnownLiving(pawn)||!FeatureFunction(cfgconvergence::disk::GetCurrentWeapon,function,owner)||
            !cfgspawn::ObjectIs(r_,pawn,owner)||
            !cfgobserver::Same(r_,function+0xB0,plan.module+cfgconvergence::disk::GetCurrentWeaponExecutor)||
            !cfgbundle::Code(r_,plan.module,cfgconvergence::disk::GetCurrentWeaponExecutor,cfgconvergence::disk::GetCurrentWeaponExecutorCode)||
            !Invoke(pawn,function,&weapon)||!HeldWeapon(pawn,weapon))return false;
        observedHeldPawn_=pawn;observedHeldWeapon_=weapon;
        std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation typeSlot{};
        if(!r_.Read(weapon+cfgprobe::ObjectClass,type)||!cfgspawn::ObjectSlot(r_,context.objectArray,type,typeSlot))return false;
        if(typeSlot==bundleKillWeaponClass&&bundleKillWeaponKind!=cfgkill::Weapon::Other){kind=bundleKillWeaponKind;return true;}
        cfgprobe::Ancestry chain;
        if(!cfgprobe::CaptureAncestry(r_,weapon,chain)||!cfgprobe::StableAncestry(r_,weapon,chain))return false;
        cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text name;
        for(unsigned i=0;i<chain.count;++i){
            if(!names.Object(chain.classes[i],name))return false;
            if(name.View()==u"CounselorTwoHandedAxe_C")kind=cfgkill::Weapon::Axe;
            else if(name.View()==u"CounselorMachete_C")kind=cfgkill::Weapon::Machete;
        }
        if(kind==cfgkill::Weapon::Other)return bundleKill.policy.PamelaAccepted();
        bundleKillWeaponClass=typeSlot;bundleKillWeaponKind=kind;return true;
    }
    bool DamageTypeMatches(std::uintptr_t weapon,std::uintptr_t damage){
        std::uintptr_t normal=0,stun=0,type=0;cfgprobe::lifetime::SlotObservation slot{};
        return damage&&weapon==observedHeldWeapon_&&observedHeldPawn_&&HeldWeapon(observedHeldPawn_,weapon)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,damage,slot)&&
            r_.Read(damage+cfgprobe::ObjectClass,type)&&cfgspawn::ObjectSlot(r_,context.objectArray,type,slot)&&
            r_.Read(weapon+cfgkill::disk::WeaponNormalDamageType,normal)&&
            r_.Read(weapon+cfgkill::disk::WeaponStunDamageType,stun)&&
            ((normal&&normal==type)||(stun&&stun==type))&&
            cfgobserver::Same(r_,damage+cfgprobe::ObjectClass,type);
    }
    bool DistanceSquared(std::uintptr_t pawn,float& squared){return Distance(pawn,squared);}
    bool ReadDamage(std::uintptr_t address,cfgkill::DamagePacket& packet){return address&&r_.Read(address,packet);}
    bool WarmDamageGate(){
        const auto state=bundleDamageGate.Status();
        bundleReport.killDamageGate=static_cast<unsigned>(state);
        if(state==cfgkill::DamageGate::State::Ready||state==cfgkill::DamageGate::State::Unavailable)return false;
        std::uintptr_t type=0;
        if(!r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,type))bundleDamageGate.Refuse();
        else if(state==cfgkill::DamageGate::State::Empty){
            std::uintptr_t base=0,owner=0;
            if(!FeatureFunction(cfgkill::disk::ReceiveAnyDamage,base,owner)||
                !bundleDamageGate.Begin(r_,context.objectArray,type,base,owner))bundleDamageGate.Refuse();
        }else bundleDamageGate.Advance(r_,context.objectArray);
        // Even failure consumed this cold phase. Never stack sweater/blackboard
        // work onto the remaining budget or retry failed native discovery.
        bundleReport.killDamageGate=static_cast<unsigned>(bundleDamageGate.Status());
        bundleReport.killGateFailedPhase=static_cast<unsigned>(bundleDamageGate.FailedPhase());
        bundleReport.killGateFields=bundleDamageGate.ObservedFields();bundleReport.killGateOwners=bundleDamageGate.ObservedOwners();return true;
    }
    bool DamageEvent(std::uintptr_t function){
        std::uintptr_t type=0;
        return bundleDamageGate.Matches(function)&&r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,type)&&
            bundleDamageGate.Validate(r_,context.objectArray,type,function);
    }
    bool KillCheck(bool okay,unsigned failure){
        if(!okay)bundleReport.killResourceFailure=failure;return okay;
    }
    bool DeathContext(cfgkill::Context& out){
        out={};std::uintptr_t actor=0,type=0,expected=0,native=0,interact=0,kill=0,victim=0,levelType=0;unsigned char active=2;
        if(!KillCheck(Jason()&&r_.Read(bundleJason.pawn.object+cfgkill::disk::JasonKillObject,actor),8))return false;
        if(!actor)return true;
        cfgprobe::lifetime::SlotObservation a{},k{},i{},l{};
        if(!KillCheck(r_.Read(bundleJason.pawn.object+cfgkill::disk::JasonKillClass,expected)&&expected&&
            r_.Read(actor+cfgprobe::ObjectClass,type)&&type==expected,9)||
            !KillCheck(NativeClass(cfgkill::disk::SCContextKillActorReflection,cfgkill::disk::SCContextKillActorConstructor,native)&&
            cfgspawn::ObjectIs(r_,actor,native)&&cfgspawn::ObjectSlot(r_,context.objectArray,actor,a)&&a.HasAssignedSerial(),10)||
            !KillCheck(cfgkill::ActorWorldCurrent(r_,context.objectArray,plan.module,actor,plan.world,l,levelType),11)||
            !KillCheck(r_.Read(actor+cfgkill::disk::InteractComponent,interact)&&
            r_.Read(actor+cfgkill::disk::ContextKillComponent,kill)&&interact&&kill&&
            NativeClass(cfgkill::disk::SCInteractComponentReflection,cfgkill::disk::SCInteractComponentConstructor,native)&&
            cfgspawn::ObjectIs(r_,interact,native)&&cfgspawn::ObjectSlot(r_,context.objectArray,interact,i)&&
            NativeClass(cfgkill::disk::SCContextKillComponentReflection,cfgkill::disk::SCContextKillComponentConstructor,native)&&
            cfgspawn::ObjectIs(r_,kill,native)&&cfgspawn::ObjectSlot(r_,context.objectArray,kill,k),12)||
            !KillCheck(cfgobserver::Same(r_,interact+0x20,actor)&&cfgobserver::Same(r_,interact+0xE0,actor)&&
            cfgobserver::Same(r_,kill+0x20,actor)&&cfgobserver::Same(r_,kill+0xE0,actor),13)||
            !KillCheck(r_.Read(kill+cfgkill::disk::Victim,victim)&&(!victim||victim==bundleJason.pawn.object)&&
            r_.Read(kill+cfgkill::disk::bIsKillActive,active)&&active<=1&&
            cfgobserver::Same(r_,bundleJason.pawn.object+cfgkill::disk::JasonKillObject,actor),14))return false;
        const bool newUse=killContextUseAt_!=bundleKill.policy.usedAt;
        if(!KillCheck(newUse||!bundleKillActorSlot.object||((a==bundleKillActorSlot)&&
            cfgkill::StrongChildIdentity(bundleKillComponentSlot,k)&&
            cfgkill::StrongChildIdentity(bundleKillInteractSlot,i)&&cfgkill::StrongChildIdentity(killContextLevelSlot_,l)),15))return false;
        if(newUse){
            // A newly witnessed sweater use owns a NEW context transaction.
            // Full native Jason->actor/class/world/components proof above is
            // mandatory before retiring old local preparation state.
            killInteractPhase_=0;killInteractActor_=killInteractPawn_=0;
            killContextUseAt_=bundleKill.policy.usedAt;
        }
        bundleKillActorSlot=a;bundleKillComponentSlot=k;bundleKillInteractSlot=i;
        killContextLevelSlot_=l;killContextLevelClass_=levelType;
        out={actor,interact,kill,active!=0};return true;
    }
    bool BridgeStun(){
        cfgkill::Identity id{};bool off=false;
        if(!Scope(id)||!(id==bundleKill.policy.identity)||!MaskOff(off)||!off||
            !cfgbundle::Code(r_,plan.module,cfgkill::disk::bInStunSetter,cfgkill::disk::bInStunSetterCode))return false;
        const auto value=ExchangeByte(bundleJason.pawn.object+cfgkill::disk::bInStun,0,1);
        return value==cfgofflinesettings::Write::Changed&&cfgescape::Same(r_,bundleJason.pawn.object+cfgkill::disk::bInStun,std::uint8_t{1});
    }
    inline static std::uintptr_t killStanceOwner_=0,killInteractFunction_=0,killInteractOwner_=0;
    inline static cfgprobe::lifetime::SlotObservation killStanceOwnerSlot_{},killInteractFunctionSlot_{},killInteractOwnerSlot_{};
    inline static unsigned killInteractPhase_=0;
    inline static std::uintptr_t killInteractActor_=0,killInteractPawn_=0;
    inline static cfgprobe::lifetime::SlotObservation killContextLevelSlot_{};
    inline static std::uintptr_t killContextLevelClass_=0;
    inline static std::uint64_t killContextUseAt_=0;
    inline static std::uintptr_t pamelaStoppedContext_=0;
    inline static std::uint64_t pamelaStoppedUseAt_=0;
    inline static cfgpamela::ContextRetry pamelaContextRetry_{};
    inline static std::uint64_t pamelaDamageTraceUse_=0;
    inline static unsigned pamelaDamageTraceCount_=0;
    static void ResetKillEpoch()noexcept{
        killStanceOwner_=killInteractFunction_=killInteractOwner_=0;
        killStanceOwnerSlot_=killInteractFunctionSlot_=killInteractOwnerSlot_={};
        killInteractPhase_=0;killInteractActor_=killInteractPawn_=0;
        killContextLevelSlot_={};killContextLevelClass_=0;killContextUseAt_=0;
        pamelaStoppedContext_=0;
        pamelaStoppedUseAt_=0;
        pamelaContextRetry_={};
        pamelaDamageTraceUse_=0;pamelaDamageTraceCount_=0;
    }
    bool StanceResource(){
        return KillCheck(cfgkill::ResourceCurrent(r_,context.objectArray,plan.module,cfgkill::disk::EnterKillStance,
            bundleKillStanceFunction,killStanceOwner_,bundleKillStanceSlot,killStanceOwnerSlot_),7)&&
            KillCheck(cfgobserver::Same(r_,bundleKillStanceFunction+0xB0,plan.module+cfgkill::disk::EnterKillStanceExecutor),2)&&
            KillCheck(cfgbundle::Code(r_,plan.module,cfgkill::disk::EnterKillStanceExecutor,cfgkill::disk::EnterKillStanceExecutorCode),3)&&
            KillCheck(cfgbundle::Code(r_,plan.module,cfgkill::disk::EnterKillStanceImplementation,cfgkill::disk::EnterKillStanceImplementationCode),4);
    }
    template<class Mark> bool DispatchStance(Mark mark){
        cfgkill::Identity id{};bool off=false,stunned=false;
        // Nothing below the byte write may discover metadata or perform a full
        // context traversal. Reserve its sole readback BEFORE changing bInStun.
        if(!Scope(id)||!(id==bundleKill.policy.identity)||!StanceResource()||!MaskOff(off)||!off||!InStun(stunned)||
            calls_>=cfgbundle::CallCap||r_.reads>=r_.cap)return false;
        if(!stunned){
            const auto value=ExchangeByte(bundleJason.pawn.object+cfgkill::disk::bInStun,0,1);
            if(value!=cfgofflinesettings::Write::Changed||
                !cfgescape::Same(r_,bundleJason.pawn.object+cfgkill::disk::bInStun,std::uint8_t{1}))return false;
        }
        return mark()&&Invoke(bundleJason.pawn.object,bundleKillStanceFunction,nullptr);
    }
    cfgkill::Preparation PrepareStance(){
        if(bundleKillStanceFunction)return StanceResource()?cfgkill::Preparation::Ready:cfgkill::Preparation::Unavailable;
        std::uintptr_t function=0,owner=0;
        if(!KillCheck(FeatureFunction(cfgkill::disk::EnterKillStance,function,owner),1)||
            !KillCheck(cfgobserver::Same(r_,function+0xB0,plan.module+cfgkill::disk::EnterKillStanceExecutor),2)||
            !KillCheck(cfgbundle::Code(r_,plan.module,cfgkill::disk::EnterKillStanceExecutor,cfgkill::disk::EnterKillStanceExecutorCode),3)||
            !KillCheck(cfgbundle::Code(r_,plan.module,cfgkill::disk::EnterKillStanceImplementation,cfgkill::disk::EnterKillStanceImplementationCode),4)||
            !KillCheck(cfgspawn::ObjectSlot(r_,context.objectArray,function,bundleKillStanceSlot),5)||
            !KillCheck(cfgspawn::ObjectSlot(r_,context.objectArray,owner,killStanceOwnerSlot_),6))return cfgkill::Preparation::Unavailable;
        killStanceOwner_=owner;bundleKillStanceFunction=function;return cfgkill::Preparation::Pending;
    }
    bool InstallKillEligibility(const cfgkill::Context& death){
        if(bundleKillClone){
            if(bundleKillHookComponent==death.killComponent)return cfgobserver::Same(r_,death.killComponent,bundleKillClone+8);
            if(!bundleKill.policy.PamelaAccepted())return false;
            // A fresh, fully proved context after a recharged use gets its own
            // table. The old table stays pinned; retired callbacks pass through
            // the unchanged original predicate and are never dereferenced here.
            bundleKillClone=bundleKillHookComponent=0;
        }
        if(death.active||!death.killComponent||!cfgobserver::Same(r_,death.killComponent,plan.module+cfgkill::disk::EligibilityVtable)||
            !cfgbundle::Code(r_,plan.module,cfgkill::disk::Eligibility,cfgkill::disk::EligibilityCode))return false;
        cfgkill::EligibilityTable table{};
        if(!cfgkill::BuildEligibilityTable(r_,plan.module,plan.module+cfgkill::disk::EligibilityVtable,
            reinterpret_cast<std::uintptr_t>(&BundleKillEligibility),table))return false;
        auto* clone=static_cast<std::uintptr_t*>(VirtualAlloc(nullptr,sizeof(table),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
        if(!clone)return false;
        std::memcpy(clone,table.data(),sizeof(table));DWORD old=0;
        if(!VirtualProtect(clone,sizeof(table),PAGE_READONLY,&old)){VirtualFree(clone,0,MEM_RELEASE);return false;}
        // Clone remains pinned for process lifetime. Never free an in-use table.
        const auto prior=InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(death.killComponent),clone+1,
            reinterpret_cast<void*>(plan.module+cfgkill::disk::EligibilityVtable));
        if(prior!=reinterpret_cast<void*>(plan.module+cfgkill::disk::EligibilityVtable)){VirtualFree(clone,0,MEM_RELEASE);return false;}
        bundleKillClone=reinterpret_cast<std::uintptr_t>(clone);bundleKillHookComponent=death.killComponent;++bundleReport.killEligibilityInstalled;
        return true;
    }
    bool InteractionResource(){
        return cfgkill::ResourceCurrent(r_,context.objectArray,plan.module,cfgdoor::disk::Interact,
            killInteractFunction_,killInteractOwner_,killInteractFunctionSlot_,killInteractOwnerSlot_)&&
            cfgobserver::Same(r_,killInteractFunction_+0xB0,plan.module+cfgdoor::disk::Executor)&&
            cfgbundle::Code(r_,plan.module,cfgdoor::disk::Executor,cfgdoor::disk::ExecutorCode);
    }
    cfgkill::Preparation PrepareInteraction(std::uintptr_t actor,std::uintptr_t pawn){
        using P=cfgkill::Preparation;
        if(killInteractPhase_==4)return P::Unavailable;
        if(bundleKill.policy.PamelaAccepted()&&!bundleKill.policy.interactionAttempted&&
           !bundleKill.policy.pamelaDispatchPending&&actor==killInteractActor_&&pawn!=killInteractPawn_){
            // No native call is pending/started: a newly eligible human can
            // replace an AI selection without inheriting its native-frame state.
            killInteractPhase_=0;killInteractActor_=killInteractPawn_=0;
        }
        if(!actor||actor!=bundleKill.policy.context||pawn!=bundleKill.policy.finisher||
            (killInteractPhase_&&(actor!=killInteractActor_||pawn!=killInteractPawn_))){killInteractPhase_=4;return P::Unavailable;}
        if(!killInteractPhase_){killInteractActor_=actor;killInteractPawn_=pawn;killInteractPhase_=1;return P::Pending;}
        if(killInteractPhase_==1){
            if(!FeatureFunction(cfgdoor::disk::Interact,killInteractFunction_,killInteractOwner_)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,killInteractFunction_,killInteractFunctionSlot_)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,killInteractOwner_,killInteractOwnerSlot_)||
                !InteractionResource()){killInteractPhase_=4;return P::Unavailable;}
            killInteractPhase_=2;return P::Pending;
        }
        if(killInteractPhase_==2){
            cfgkill::Identity id{};cfgkill::Context death{};std::uintptr_t weapon=0;cfgkill::Weapon kind=cfgkill::Weapon::Other;
            if(!Scope(id)||!(id==bundleKill.policy.identity)||!DeathContext(death)||death.actor!=actor){killInteractPhase_=4;return P::Unavailable;}
            if(death.active)return P::Ready; // Runtime observes native ownership; no clone/call needed.
            const bool held=bundleKill.policy.PamelaAccepted()?PamelaHeld(pawn,weapon):CurrentWeapon(pawn,weapon,kind);
            if(!KnownLiving(pawn)||!held||weapon!=bundleKill.policy.weapon||
                (!bundleKill.policy.PamelaAccepted()&&kind!=cfgkill::Weapon::Axe&&kind!=cfgkill::Weapon::Machete)||
                !cfgbundle::Code(r_,plan.module,cfgkill::disk::bIsHunterSetter,cfgkill::disk::bIsHunterSetterCode)||
                !InstallKillEligibility(death)){killInteractPhase_=4;return P::Unavailable;}
            killInteractPhase_=3;return P::Pending;
        }
        if(!InteractionResource()){killInteractPhase_=4;return P::Unavailable;}
        return P::Ready;
    }
    template<class Mark> bool FinalInteraction(const cfgkill::Context& death,std::uintptr_t pawn,Mark mark){
        float squared=0;unsigned char active=2;
        const auto admission=[&](unsigned failure){bundleReport.killInteractionAdmission=failure;
            bundleReport.killInteractionReads=r_.reads;bundleReport.killInteractionCalls=calls_;return false;};
        if(killInteractPhase_!=3||death.actor!=killInteractActor_||pawn!=killInteractPawn_||
            death.actor!=bundleKillActorSlot.object||death.interactable!=bundleKillInteractSlot.object||
            death.killComponent!=bundleKillComponentSlot.object||death.active)return admission(1);
        if(bundleKill.policy.PamelaAccepted()){
            std::uintptr_t held=0;
            if(!PamelaHeld(pawn,held)||held!=bundleKill.policy.weapon)return admission(1);
        }
        if(!InteractionResource())return admission(2);
        if(!DistanceSquared(pawn,squared)||!std::isfinite(squared)||squared<0)return admission(3);
        bundleReport.killInteractionDistanceSquared=squared;
        // The accepted melee still requires200cm. Once native kneeling has
        // moved either pawn, use the donor's600cm context acquisition envelope;
        // the unchanged native eligibility predicate owns final weapon/range.
        if(squared>600.f*600.f)return admission(3);
        if(!cfgobserver::Same(r_,death.killComponent,bundleKillClone+8)||
            !cfgjason::OwnedComponentSlot(r_,context.objectArray,bundleKillInteractSlot)||
            !r_.Read(death.killComponent+cfgkill::disk::bIsKillActive,active)||active||
            !cfgbundle::Code(r_,plan.module,cfgkill::disk::bIsHunterSetter,cfgkill::disk::bIsHunterSetterCode))return admission(4);
        struct Packet{std::uintptr_t component=0;std::uint8_t hold=0,padding[7]{};}packet;
        packet.component=death.interactable;packet.hold=1; // native Press method bit, NOT a hold boolean
        bundleKillTicket={{plan.world,bundleJason.pawn.object,bundleJason.pawn.serial},death.actor,death.killComponent,pawn,true,true,true,true,true};
        // Predicate can run more than once within native Interact. Keep a shared
        // read reserve for two worst-case known-roster/world validations (167 each).
        if(!EligibilityCurrent(death.killComponent,pawn))return admission(5);
        if(calls_>=cfgbundle::CallCap)return admission(6);
        if(r_.reads>r_.cap||r_.cap-r_.reads<(bundleKill.policy.PamelaAccepted()?768u:334u))return admission(7);
        if(!mark())return admission(8);
        bundleKillEligibilityAccess=this;
        const auto dispatched=Invoke(pawn,killInteractFunction_,&packet);
        bundleKillEligibilityAccess=nullptr; // Invoke catches native failure; no callback outlives this call
        admission(dispatched?0u:9u);return dispatched; // native eligibility still owns actual kill
    }
    bool EligibilityTicket(std::uintptr_t component,std::uintptr_t pawn,cfgkill::EligibilityTicket& ticket){
        ticket={};cfgkill::Identity id{};cfgkill::Context death{};std::uintptr_t weapon=0;cfgkill::Weapon kind=cfgkill::Weapon::Other;
        if(!Scope(id)||!DeathContext(death)||death.killComponent!=component||!KnownLiving(pawn)||
            !CurrentWeapon(pawn,weapon,kind)||weapon!=bundleKill.policy.weapon||
            !cfgbundle::Code(r_,plan.module,cfgkill::disk::Eligibility,cfgkill::disk::EligibilityCode)||
            !cfgbundle::Code(r_,plan.module,cfgkill::disk::bIsHunterSetter,cfgkill::disk::bIsHunterSetterCode))return false;
        ticket={id,death.actor,component,pawn,true,true,true,true,true};return true;
    }
#include "EligibilityWorldRuntimeMembers.hpp"
    bool EligibilityCurrent(std::uintptr_t component,std::uintptr_t pawn,bool nativeInteractor=false){
        // Prepared synchronously just before native Interact. No nested weapon
        // getter, full context sweep or extra reader inside the predicate.
        bool alive=false;
        return GetCurrentThreadId()==plan.threadId&&
            (nativeInteractor?cfgprimary::LookupVetoClearForObservation(r_,plan.module):cfgprimary::LookupSafe(r_,plan.module))&&
            cfgobserver::Same(r_,plan.viewport+cfgprobe::ViewportWorld,plan.world)&&
            EligibilityWorldCurrent(nativeInteractor)&&
            cfgjason::SameSlot(r_,context.objectArray,bundleKillActorSlot)&&
            cfgjason::OwnedComponentSlot(r_,context.objectArray,bundleKillComponentSlot)&&
            component==bundleKillComponentSlot.object&&
            cfgjason::OwnedComponentSlot(r_,context.objectArray,killContextLevelSlot_)&&
            cfgobserver::Same(r_,bundleKillActorSlot.object+0x20,killContextLevelSlot_.object)&&
            cfgobserver::Same(r_,killContextLevelSlot_.object+cfgprobe::ObjectClass,killContextLevelClass_)&&
            cfgobserver::Same(r_,killContextLevelSlot_.object+cfgkill::disk::OwningWorld,plan.world)&&
            cfgobserver::Same(r_,component+0x20,bundleKillActorSlot.object)&&
            cfgobserver::Same(r_,component+0xE0,bundleKillActorSlot.object)&&
            cfgobserver::Same(r_,bundleJason.pawn.object+cfgkill::disk::JasonKillObject,bundleKillActorSlot.object)&&
            (nativeInteractor?NativePamelaCounselor(pawn):(KnownLiving(pawn)&&Alive(pawn,alive)&&alive))&&
            cfgbundle::Code(r_,plan.module,cfgkill::disk::Eligibility,cfgkill::disk::EligibilityCode);
    }
