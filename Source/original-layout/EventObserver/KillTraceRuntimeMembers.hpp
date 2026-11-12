// Included inside BundleAccess. Native event before/after observation ONLY.
    bool MeleeTraceEvent(std::uintptr_t source,std::uintptr_t function){
        using namespace cfgkill::contactdisk;
        std::uintptr_t cached=0,owner=0;std::uint16_t bytes=1;
        cfgprobe::lifetime::SlotObservation functionSlot{},ownerSlot{};
        const bool native=r_.Read(plan.module+AttackTrace.ownerReflection,owner)&&owner&&
            cfgobserver::Same(r_,plan.module+AttackTrace.ownerConstructor,owner)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,owner,ownerSlot)&&
            cfgspawn::ObjectIs(r_,source,owner)&&
            cfgbundle::Code(r_,plan.module,Executor,ExecutorCode)&&
            cfgbundle::Code(r_,plan.module,Implementation,ImplementationCode);
        if(!native)return false;
        if(!function)return true; // direct native Tick contact-list observation, no synthetic callback
        return r_.Read(plan.module+AttackTrace.cache,cached)&&cached==function&&
            cfgobserver::Same(r_,function+0x20,owner)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,function,functionSlot)&&
            r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)&&bytes==0&&
            cfgobserver::Same(r_,function+0xB0,plan.module+Executor)&&
            cfgbundle::Code(r_,plan.module,AttackTrace.factory,AttackTrace.code);
    }
    bool BeginMeleeTrace(std::uintptr_t source,std::uintptr_t function,cfgkillcontact::Proof& proof){
        proof={};r_.cap=cfgbundle::ReadCap-1;auto& p=bundleKill.policy; // one caller hint read
        if(p.stage!=cfgkill::Stage::NativeContext||p.interactionAttempted||p.finishingStrike||p.nativeActiveObserved||
           source!=p.finisher||!p.context||!p.weapon||
           !Scope(proof.identity)||!(proof.identity==p.identity)||!KnownLiving(source)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,source,proof.source)||!proof.source.HasAssignedSerial()||
           !MeleeTraceEvent(source,function)||
           !cfgkillcontact::Read(r_,source+cfgkill::contactdisk::Actors,proof.before))return false;
        proof.function=function;proof.context=p.context;proof.weapon=p.weapon;proof.usedAt=p.usedAt;proof.acceptedAt=p.acceptedAt;
        proof.beforeReads=r_.reads;proof.armed=true;return true;
    }
    bool MeleeContinuationBudget(const cfgkillcontact::Proof& proof){
        if(proof.beforeReads>=cfgbundle::ReadCap-1)return false;
        r_.cap=cfgbundle::ReadCap-proof.beforeReads-1; // before+after+hint share ONE finite callback budget
        return true;
    }
    bool FinishMeleeTrace(cfgkillcontact::Proof& proof){
        if(!MeleeContinuationBudget(proof))return false;
        return proof.armed&&Context()&&cfgjason::SameSlot(r_,context.objectArray,proof.source)&&
            MeleeTraceEvent(proof.source.object,proof.function)&&
            cfgkillcontact::Read(r_,proof.source.object+cfgkill::contactdisk::Actors,proof.after)&&
            cfgkillcontact::AddedTarget(proof.before,proof.after,proof.identity.jason,proof.context);
    }
    bool MeleeContactCurrent(const cfgkillcontact::Proof& proof){
        cfgkillcontact::Contacts current;
        return cfgjason::SameSlot(r_,context.objectArray,proof.source)&&
            MeleeTraceEvent(proof.source.object,proof.function)&&
            cfgkillcontact::Read(r_,proof.source.object+cfgkill::contactdisk::Actors,current)&&
            cfgkillcontact::Same(current,proof.after);
    }
