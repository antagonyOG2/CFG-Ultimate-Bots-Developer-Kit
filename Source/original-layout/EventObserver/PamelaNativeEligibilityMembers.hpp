// Included inside BundleAccess; mandatory exact native/identity/SEH seams.
    bool PamelaContextStop(){
        cfgkill::Identity id{};std::uintptr_t function=0,owner=0;
        if(!Scope(id)||!(id==bundleKill.policy.identity)||
           !FeatureFunction(cfgtrap::disk::StopMovement,function,owner)||
           !cfgspawn::ObjectIs(r_,bundleJason.killerController.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::StopMovementExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode))return false;
        if(calls_>=cfgbundle::CallCap)return false;
        // Consume ONLY after the bounded preflight, BEFORE possible native entry.
        pamelaStoppedContext_=bundleKill.policy.context;
        pamelaStoppedUseAt_=bundleKill.policy.usedAt;
        const bool stopped=Invoke(bundleJason.killerController.object,function,nullptr);
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();
        bundleMoveAt=0;bundleTargetLockUntil=0;
        cfgclosedtrace::Mark(stopped?"pamela-native-context-stop":"pamela-native-context-stop-refused",bundleJason.killerController.object,bundleKill.policy.context);
        return stopped;
    }
    bool NativePamelaCounselor(std::uintptr_t pawn){
        std::uintptr_t klass=0,levelType=0;cfgprobe::lifetime::SlotObservation slot{},again{},level{};
        bool alive=false;
        return NativeClass(cfgkill::disk::SCCounselorCharacterReflection,cfgkill::disk::SCCounselorCharacterConstructor,klass)&&
            cfgspawn::ObjectIs(r_,pawn,klass)&&cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot)&&
            cfgkill::ActorWorldCurrent(r_,context.objectArray,plan.module,pawn,plan.world,level,levelType)&&
            Alive(pawn,alive)&&alive&&cfgspawn::ObjectSlot(r_,context.objectArray,pawn,again)&&slot==again;
    }
#include "NativePamelaEligibilityObservationMembers.hpp"
    void TracePamelaDamageFunction(std::uintptr_t function,std::uint64_t now){
        const auto& p=bundleKill.policy;
        if(!p.pamela||now<p.usedAt||now>=p.until)return;
        if(pamelaDamageTraceUse_!=p.usedAt){pamelaDamageTraceUse_=p.usedAt;pamelaDamageTraceCount_=0;}
        if(pamelaDamageTraceCount_>=32)return;
        cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text name;
        if(!names.Object(function,name)||name.View().find(u"Damage")==std::u16string_view::npos)return;
        std::array<char,128> label{};constexpr char prefix[]="pamela-damage-event:";
        std::copy(prefix,prefix+sizeof(prefix)-1,label.begin());
        unsigned cursor=sizeof(prefix)-1;
        for(const auto c:name.View()){if(cursor+1>=label.size())break;label[cursor++]=c<128?static_cast<char>(c):'_';}
        cfgclosedtrace::Mark(label.data(),function,++pamelaDamageTraceCount_);
    }
