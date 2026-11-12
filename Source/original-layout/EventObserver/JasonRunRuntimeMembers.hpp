    cfgprobe::lifetime::SlotObservation runMovement_{};
    float runNativeValue_=0;
    bool RunState(){
        using namespace cfgrun::disk;
        unsigned char running=0;std::uintptr_t water=0,function=0,owner=0;
        if(!r_.Read(bundleJason.pawn.object+Running,running)||running)return false;
        bool stunned=false,attacking=false,busy=false;std::uintptr_t held=0;
        ++bundleReport.runStateChecks;bundleReport.runStateFailure=1;
        if(!Context()||!Jason()||!Held(held)||!NativeBool(cfghunter::disk::Stunned,stunned)||
           !NativeBool(cfghunter::disk::Attacking,attacking)||!GrabBusy(busy)||held||stunned||attacking||busy||
           !r_.Read(bundleJason.pawn.object+cfgswim::disk::WaterVolume,water)||water)return false;
        bundleReport.runStateFailure=2;
        if(!FeatureFunction(ServerSetRunning,function,owner)||!cfgspawn::ObjectIs(r_,bundleJason.pawn.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+Executor)||
           !cfgbundle::Code(r_,plan.module,Executor,ExecutorCode)||
           !cfgbundle::Code(r_,plan.module,Implementation,ImplementationCode)||
           !cfgbundle::Code(r_,plan.module,Toggle,ToggleCode)||
           !cfgbundle::Code(r_,plan.module,SetBit,SetBitCode)||!Context()||!Jason()||
           !cfgescape::Same(r_,bundleJason.pawn.object+Running,std::uint8_t{}))return false;
        std::uint8_t desired=1;++bundleReport.runStateAttempts;
        if(!Invoke(bundleJason.pawn.object,function,&desired)){bundleReport.runStateFailure=3;return true;}
        if(!Context()||!Jason()||!cfgescape::Same(r_,bundleJason.pawn.object+Running,desired)){
            bundleReport.runStateFailure=4;return true;}
        ++bundleReport.runStateReturns;bundleReport.runStateFailure=0;return true;
    }
    bool RunObserve(float& walk,float& run){
        if(!Context()||!Jason())return false;
        std::uintptr_t movement=0,type=0;
        const auto& binding=cfghook::disk::Types[cfghook::disk::Movement];
        return r_.Read(bundleJason.pawn.object+cfghook::disk::MovementComponent,movement)&&movement&&
            NativeClass(binding.reflection,binding.constructor,type)&&cfgspawn::ObjectIs(r_,movement,type)&&
            cfgobserver::Same(r_,movement+0x20,bundleJason.pawn.object)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,movement,runMovement_)&&cfgjason::OwnedComponentSlot(r_,context.objectArray,runMovement_)&&
            r_.Read(movement+cfghook::disk::Walk,walk)&&r_.Read(movement+cfghook::disk::Run,run)&&
            std::isfinite(run)&&run>=25.f&&run<=2500.f&&
            ((runNativeValue_=run),run=(std::max)(run,cfgrun::MinimumPursuitRun),true);
    }
    bool RunAuxiliary(float desired){
        if(bundleCarSpeed.active||bundleFootBurst.Owned()||!Context()||!Jason()||
           !cfgjason::OwnedComponentSlot(r_,context.objectArray,runMovement_)||
           !cfgobserver::Same(r_,bundleJason.pawn.object+cfghook::disk::MovementComponent,runMovement_.object)||
           !cfgobserver::Same(r_,runMovement_.object+0x20,bundleJason.pawn.object)||
           !cfgescape::Same(r_,runMovement_.object+cfghook::disk::Run,runNativeValue_))return false;
        for(const auto field:{cfghook::disk::Run,cfghook::disk::Sprint,cfghook::disk::SlowRun}){
            float current=0;if(!r_.Read(runMovement_.object+field,current)||!std::isfinite(current)||current<25.f||current>2500.f)return false;
            if(current<desired){const bool written=CarSpeedWrite(runMovement_.object+field,current,desired);
                cfgclosedtrace::Mark(written?"pursuit-run-limit-repaired":"pursuit-run-limit-refused",bundleJason.pawn.object,field);return true;}
        }
        return false;
    }
    bool RunWrite(float expected,float desired){
        return !bundleCarSpeed.active&&Context()&&Jason()&&cfgjason::OwnedComponentSlot(r_,context.objectArray,runMovement_)&&
            cfgobserver::Same(r_,bundleJason.pawn.object+cfghook::disk::MovementComponent,runMovement_.object)&&
            cfgobserver::Same(r_,runMovement_.object+0x20,bundleJason.pawn.object)&&
            cfgescape::Same(r_,runMovement_.object+cfghook::disk::Run,runNativeValue_)&&
            CarSpeedWrite(runMovement_.object+cfghook::disk::Walk,expected,desired);
    }
