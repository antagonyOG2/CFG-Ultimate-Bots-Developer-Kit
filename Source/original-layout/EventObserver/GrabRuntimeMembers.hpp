// Included inside BundleAccess. Exact native bindings, same bounded reader/calls.
    template<std::size_t N> bool GrabFunction(const cfgvehicle::disk::Binding& binding,std::uintptr_t executor,
        const std::array<unsigned char,N>& code,std::uintptr_t& function){
        std::uintptr_t owner=0;
        return FeatureFunction(binding,function,owner)&&cfgspawn::ObjectIs(r_,bundleJason.pawn.object,owner)&&
            cfgobserver::Same(r_,function+0xB0,plan.module+executor)&&cfgbundle::Code(r_,plan.module,executor,code);
    }
    bool GrabWarm(){
        using namespace cfggrab::disk;std::uintptr_t function=0;bool ok=false;
        switch(bundleGrabWarm){
        case 0:ok=GrabFunction(cfggrab::disk::BeginGrab,BeginGrabExecutor,BeginGrabCode,function);break;
        case 1:ok=GrabFunction(CanGrabKill,CanGrabKillExecutor,CanGrabKillCode,function);break;
        case 2:ok=GrabFunction(IsGrabKilling,IsGrabKillingExecutor,IsGrabKillingCode,function);break;
        case 3:ok=GrabFunction(IsAttemptingGrab,IsAttemptingGrabExecutor,IsAttemptingGrabCode,function);break;
        case 4:ok=GrabFunction(IsCharacterInGrabRange,IsCharacterInGrabRangeExecutor,IsCharacterInGrabRangeCode,function);break;
        case 5:ok=GrabFunction(IsGrabKillAvailable,IsGrabKillAvailableExecutor,IsGrabKillAvailableCode,function);break;
        default:return false;
        }
        if(ok)++bundleGrabWarm;else{bundleGrabUnavailable=true;++bundleReport.grabUnavailable;}
        return true;
    }
    template<std::size_t N> bool GrabBool(const cfgvehicle::disk::Binding& binding,std::uintptr_t executor,
        const std::array<unsigned char,N>& code,bool& value){
        value=false;std::uintptr_t function=0;std::uint8_t result=0;
        if(!GrabFunction(binding,executor,code,function)||!Invoke(bundleJason.pawn.object,function,&result)||result>1)return false;
        value=result!=0;return true;
    }
    bool GrabBusy(bool& busy){
        using namespace cfggrab::disk;busy=false;bool attempting=false,killing=false;
        if(bundleGrabUnavailable||bundleGrabWarm<6)return true;
        if(!GrabBool(IsAttemptingGrab,IsAttemptingGrabExecutor,IsAttemptingGrabCode,attempting)||
           !GrabBool(IsGrabKilling,IsGrabKillingExecutor,IsGrabKillingCode,killing))return false;
        busy=attempting||killing;return true;
    }
    bool BeginGrab(){
        using namespace cfggrab::disk;bool range=false;std::uintptr_t function=0,held=0;
        if(bundleGrabUnavailable||bundleGrabWarm<6||
           !GrabBool(IsCharacterInGrabRange,IsCharacterInGrabRangeExecutor,IsCharacterInGrabRangeCode,range)||!range||
           !GrabFunction(cfggrab::disk::BeginGrab,BeginGrabExecutor,BeginGrabCode,function)||
           !Held(held)||held)return false;
        ++bundleReport.grabAttempts;
        const bool returned=Invoke(bundleJason.pawn.object,function,nullptr);
        if(returned)++bundleReport.grabReturns;return returned;
    }
    bool GrabKillInput(unsigned slot)noexcept{
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Grab);
#endif

        if(slot>=4||!NativeCallAllowed()||
           !cfgbundle::Code(r_,plan.module,cfggrab::disk::GrabKillInput,cfggrab::disk::GrabKillInputCode))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Grab,1)
#else
 false
#endif
;
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,std::int32_t)>(plan.module+cfggrab::disk::GrabKillInput)(
            reinterpret_cast<void*>(bundleJason.pawn.object),static_cast<std::int32_t>(slot));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Grab,2)
#else
 false
#endif
;}
    }
    void HeldGrab(std::uintptr_t held,std::uint64_t now){
        if(DonorHeldDecisionInstalled()){bundleGrab.Observe(held,now);return;}
        using namespace cfggrab::disk;
        const bool newHold=held&&held!=bundleGrab.held;bundleGrab.Observe(held,now);
        if(newHold)++bundleReport.grabHolds;
        if(!held||bundleGrabUnavailable||bundleGrabWarm<6||!bundleGrab.Due(now))return;
        std::uintptr_t seat=0,extracting=0;
        if(!r_.Read(held+cfgvehicleoccupancy::disk::CurrentSeat,seat)||seat||
           !r_.Read(bundleJason.pawn.object+cfgaction::disk::ExtractionComponent,extracting)||extracting)return;
        bool killing=false,allowed=false;
        if(!GrabBool(IsGrabKilling,IsGrabKillingExecutor,IsGrabKillingCode,killing)||
           !GrabBool(CanGrabKill,CanGrabKillExecutor,CanGrabKillCode,allowed)){
            ++bundleReport.grabUnavailable;bundleGrab.Refused(now);return;
        }
        if(killing){bundleGrab.Commit();return;}
        if(!allowed){bundleGrab.nextProbe=now+250;return;}
        struct SlotPacket{std::int32_t slot;std::uint8_t result=0,padding[3]{};} packet{static_cast<std::int32_t>(bundleGrab.slot)};
        static_assert(sizeof(SlotPacket)==8&&offsetof(SlotPacket,result)==4);
        std::uintptr_t function=0;
        if(!GrabFunction(IsGrabKillAvailable,IsGrabKillAvailableExecutor,IsGrabKillAvailableCode,function)||
           !Invoke(bundleJason.pawn.object,function,&packet)||packet.result>1){
            ++bundleReport.grabUnavailable;bundleGrab.Refused(now);return;
        }
        if(!packet.result){bundleGrab.Refused(now);return;}
        std::uintptr_t fresh=0;cfgescape::State state;
        if(!Held(fresh)||fresh!=held||!cfgescape::Counselor(r_,context.objectArray,plan.module,held,bundleCounselorClass,state)||
           cfgescape::Terminal(state)||state==cfgescape::State::Unknown||!Jason())return;
        bundleGrab.Commit();++bundleReport.grabKillAttempts;
        bundleHeldRecovery.Input(held,now);
        if(GrabKillInput(bundleGrab.slot))++bundleReport.grabKillReturns;
    }
