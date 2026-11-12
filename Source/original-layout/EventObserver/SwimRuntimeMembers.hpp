// Included inside BundleAccess. Native underwater action owns victim selection,
// depth/range/eligibility and the actual cinematic; never forced drowning/HP.
    bool SwimDue(std::uintptr_t target,std::uint64_t now){
        std::uintptr_t water=0;
        return target&&now>=bundleSwimNext&&r_.Read(target+cfgswim::disk::WaterVolume,water)&&water;
    }
    bool SwimScope(std::uintptr_t target,std::uintptr_t& actor){
        using namespace cfgswim::disk;
        std::uintptr_t water=0,type=0,level=0,levelType=0,again=0;
        cfgprobe::lifetime::SlotObservation before{},after{};
        actor=0;
        bundleReport.swimGuard=1;if(!Context())return false;
        bundleReport.swimGuard=2;if(!Jason())return false;
        bundleReport.swimGuard=3;if(!KnownLiving(target)&&!HostedRosterLive(target))return false;
        bundleReport.swimGuard=4;if(!r_.Read(target+WaterVolume,water)||!cfgprobe::ObjectPointer(water))return false;
        // Native CanInteractUnderwater/OnInteractUnderwater enumerate swimmers
        // themselves (shared CFG body helper41E010). A kill-context actor is
        // NOT an admission prerequisite; requiring it blocked ordinary swims.
        // Retain the actual target water-volume identity across native calls.
        actor=water;(void)type;
        bundleReport.swimGuard=7;if(!cfgspawn::ObjectSlot(r_,context.objectArray,actor,before))return false;
        bundleReport.swimGuard=8;if(!r_.Read(actor+0x20,level)||!NativeClass(cfgkill::disk::LevelReflection,cfgkill::disk::LevelConstructor,levelType)||
            !cfgspawn::ObjectIs(r_,level,levelType)||!cfgobserver::Same(r_,level+cfgkill::disk::OwningWorld,plan.world))return false;
        bundleReport.swimGuard=9;if(!cfgspawn::ObjectSlot(r_,context.objectArray,actor,after)||!(before==after)||
            !r_.Read(target+WaterVolume,again)||again!=actor||
            !cfgobserver::Same(r_,actor+0x20,level)||!cfgobserver::Same(r_,target+WaterVolume,water))return false;
        bundleReport.swimGuard=10;if(!cfgprimary::LookupSafe(r_,plan.module))return false;
        bundleReport.swimGuard=0;return true;
    }
    template<class B,std::size_t N,std::size_t M>bool SwimFunction(const B& b,std::uintptr_t executor,
        const std::array<unsigned char,N>& code,std::uintptr_t body,const std::array<unsigned char,M>& bodyCode,
        std::uintptr_t& function){
        std::uintptr_t owner=0;
        return FeatureFunction(b,function,owner)&&cfgspawn::ObjectIs(r_,bundleJason.pawn.object,owner)&&
            cfgobserver::Same(r_,function+0xB0,plan.module+executor)&&
            cfgbundle::Code(r_,plan.module,executor,code)&&cfgbundle::Code(r_,plan.module,body,bodyCode);
    }
    bool SwimPhase(std::uintptr_t target,std::uint64_t now){
        using namespace cfgswim::disk;
        if(now<bundleSwimNext)return false;
        std::uintptr_t water=0;if(!r_.Read(target+WaterVolume,water)||!water){HostedLaneRefuse(cfgh1::Lane::Swim,25);return false;}
        bundleSwimNext=now+500;++bundleReport.swimChecks;bundleReport.swimFailure=1;
        const auto refused=[&](unsigned reason,bool consumed){HostedLaneRefuse(cfgh1::Lane::Swim,reason);return consumed;};
        std::uintptr_t actor=0,can=0,action=0;float squared=0;bool busy=false;
        if(!SwimScope(target,actor))return refused(bundleReport.swimGuard?bundleReport.swimGuard:1,false);
        bundleReport.swimGuard=11;if(!Distance(target,squared))return refused(11,false);
        bundleReport.swimGuard=12;if(squared>600.f*600.f){
            // This protected lane consumes the hunt callback. Range refusal
            // must not starve the existing chase request every700ms.
            // Native path/kill eligibility stays intact; no depth/kill bypass.
            if(!GrabBusy(busy)||busy)return refused(12,false);
            const bool moved=Move(target);if(!moved)HostedLaneRefuse(cfgh1::Lane::Swim,13);return moved;
        }
        bundleReport.swimGuard=13;if(!GrabBusy(busy))return refused(13,false);
        bundleReport.swimGuard=14;if(busy)return refused(14,false);
        bundleReport.swimGuard=0;
        bundleReport.swimFailure=2;
        bundleReport.swimGuard=15;
        if(!SwimFunction(CanInteractUnderwater,CanInteractUnderwaterExecutor,CanInteractUnderwaterCode,
           CanInteractUnderwaterBody,CanInteractUnderwaterBodyCode,can))return refused(15,true);
        bundleReport.swimGuard=16;
        if(!SwimFunction(OnInteractUnderwater,OnInteractUnderwaterExecutor,OnInteractUnderwaterCode,
           OnInteractUnderwaterBody,OnInteractUnderwaterBodyCode,action))return refused(16,true);
        struct Admission {std::uintptr_t character=0;cfgspawn::Vec3 location{},rotation{};std::int32_t result=0;std::uint32_t pad=0;} query;
        struct Action {std::uintptr_t interactor=0;std::uint8_t method=0,pad[7]{};} packet;
        static_assert(sizeof(Admission)==40&&offsetof(Admission,result)==32,"native36/storage40 underwater query");
        static_assert(sizeof(Action)==16&&offsetof(Action,method)==8,"native9/storage16 underwater action");
        query.character=bundleJason.pawn.object;
        bundleReport.swimGuard=17;
        if(!Location(bundleJason.pawn.object,query.location))return refused(17,true);
        // Query plus fresh busy(2),stun,attack and action(1): six native calls.
        // Do not enter a predicate whose positive result cannot be consumed.
        bundleReport.swimGuard=18;
        if(calls_+6>cfgbundle::CallCap)return refused(18,true);
        bundleReport.swimGuard=19;
        if(!Invoke(bundleJason.pawn.object,can,&query))return refused(19,true);
        bundleReport.swimNativeAdmission=query.result;bundleReport.swimFailure=3;
        bundleReport.swimGuard=0;
        if(query.result!=1)return refused(20,false); // exact native eligibility, not a bypass
        std::uintptr_t fresh=0;bundleReport.swimFailure=4;
        bool stunned=false,attacking=false;
        if(!SwimScope(target,fresh)||fresh!=actor)return refused(bundleReport.swimGuard?bundleReport.swimGuard:21,true);
        bundleReport.swimGuard=20;
        if(calls_+5>cfgbundle::CallCap)return refused(22,true);
        bundleReport.swimGuard=21;
        if(!GrabBusy(busy)||busy||!NativeBool(cfghunter::disk::Stunned,stunned)||stunned||
           !NativeBool(cfghunter::disk::Attacking,attacking)||attacking||
           !cfgobserver::Same(r_,plan.module+OnInteractUnderwater.cache,action))return refused(23,true);
        packet.interactor=bundleJason.pawn.object;
        // Consume cadence before dispatch, including exceptions. Native context
        // chooses the eligible swimmer; never replay during a grabbed/kill.
        bundleSwimNext=now+3000;++bundleReport.swimActions;
        if(!Invoke(bundleJason.pawn.object,action,&packet)){++bundleReport.swimRefused;return refused(24,true);}
        bundleReport.swimFailure=0;bundleReport.swimGuard=0;return true;
    }
