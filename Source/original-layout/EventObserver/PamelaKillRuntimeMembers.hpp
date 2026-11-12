// Donor trance admission only. Existing retained stance/eligibility/native
// interaction paths still own the real cinematic and final death state.
    cfgsweater::KillArmResult ArmSweaterKill(cfgkill::Runtime& kill,const cfgkill::Identity& expected,
        std::uintptr_t wearer,std::uint64_t used){
        cfgkill::Identity id{};float squared=0,health=0;
        if(!Scope(id)||!(id==expected)||!KnownLiving(wearer)||!DistanceSquared(wearer,squared)||
           !PamelaHealthValue(health))return cfgsweater::KillArmResult::Deferred;
        if(!std::isfinite(squared)||squared<0||squared>250.f*250.f)return cfgsweater::KillArmResult::Rejected;
        if(!kill.policy.PamelaUsed(id,wearer,used,true,squared,health))return cfgsweater::KillArmResult::Rejected;
        kill.pamelaHealthAt=0;kill.pamelaFinisherCursor=0;kill.pamelaStock={};
        return cfgsweater::KillArmResult::Accepted;
    }
    bool PamelaHealthValue(float& health){
        return cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)&&
            r_.Read(bundleJason.pawn.object+cfghunter::disk::Health,health)&&std::isfinite(health)&&
            cfgescape::Same(r_,bundleJason.pawn.object+cfghunter::disk::Health,health);
    }
    bool PamelaHeld(std::uintptr_t pawn,std::uintptr_t& weapon){
        namespace d=cfgcompat::cfg35::pamelafinisher;
        weapon=0;
        // Same native getter's exact equipped field and SCWeapon/owner/slot
        // proof, without another allocating/dispatching getter inside selection.
        return cfgbundle::Code(r_,plan.module,d::HeldGetter,d::HeldGetterCode)&&
            r_.Read(pawn+d::EquippedItem,weapon)&&weapon&&HeldWeapon(pawn,weapon)&&
            cfgobserver::Same(r_,pawn+d::EquippedItem,weapon);
    }
    cfgkill::Preparation PreparePamelaContext(const cfgkill::Context& death){
        if(bundleKillClone&&bundleKillHookComponent==death.killComponent)return
            cfgobserver::Same(r_,death.killComponent,bundleKillClone+8)?cfgkill::Preparation::Ready:cfgkill::Preparation::Unavailable;
        if(!InstallKillEligibility(death))return cfgkill::Preparation::Unavailable;
        return cfgkill::Preparation::Pending; // let stock queries run before AI selection
    }
    bool PamelaCandidate(unsigned i,std::uintptr_t& pawn,std::uintptr_t& weapon,cfgkill::Weapon& kind){
        float squared=0;
        std::uintptr_t manager=0;
        const auto& stock=bundleKill.pamelaStock;
        if(i&&(!stock.Current(bundleKill.policy,GetTickCount64())||i>bundleReport.bots||
            bundleReport.roster[i-1].pawn!=stock.pawn||stock.component!=bundleKillComponentSlot.object))return false;
        if(!Known(i,pawn)||!KnownLiving(pawn)||!DistanceSquared(pawn,squared)||
            !std::isfinite(squared)||squared<0||squared>200.f*200.f||
            !r_.Read(pawn+cfghunter::disk::Manager,manager)||!manager||!PamelaHeld(pawn,weapon))return false;
        kind=cfgkill::Weapon::Other;
        if(!i)return true;
        cfgprobe::lifetime::SlotObservation slot{};
        return cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot)&&slot.index==stock.index&&slot.serial==stock.serial&&
            weapon==stock.weapon;
    }
    bool ObservePamelaStock(std::uintptr_t component,std::uintptr_t pawn,int result,std::uint64_t now){
        if(!result||!bundleKill.policy.PamelaAccepted()||!bundleKill.policy.Exclusive(now)||
           !bundleKill.policy.context||component!=bundleKillComponentSlot.object||
           !EligibilityCurrent(component,pawn))return false;
        std::uintptr_t weapon=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!PamelaHeld(pawn,weapon)||!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,slot)||!slot.HasAssignedSerial())return false;
        return bundleKill.pamelaStock.Remember(bundleKill.policy,bundleKill.policy.context,component,pawn,weapon,
            slot.index,slot.serial,now);
    }
    bool PamelaFinisher(std::uintptr_t& pawn,std::uintptr_t& weapon,cfgkill::Weapon& kind){
        return cfgkill::SelectPamelaFinisher(*this,bundleReport.bots,bundleKill.pamelaFinisherCursor,pawn,weapon,kind);
    }
    bool PamelaEligibilityCall(const cfgkill::Policy& policy,std::uintptr_t component,std::uintptr_t pawn,
        cfgkill::EligibilityOriginal original,const cfgkill::EligibilityVector* location,const cfgkill::EligibilityVector* direction,int& result,
        const cfgkill::EligibilityTicket* nativeTicket=nullptr){
        result=0;
        if(!policy.PamelaAccepted()||component!=bundleKillComponentSlot.object||pawn!=policy.finisher||
           !policy.weapon||!HeldWeapon(pawn,policy.weapon)||
           !cfgobserver::Same(r_,pawn+cfgcompat::cfg35::pamelafinisher::EquippedItem,policy.weapon))return false;
        // AI fallback must pass stock eligibility, without either override.
        if(nativeTicket?!cfgkill::PamelaNativeOverrideAllowed(policy,*nativeTicket,component,pawn,GetTickCount64()):
           !cfgkill::PamelaOverrideAllowed(policy,bundleKillTicket,component,pawn,GetTickCount64(),
            bundleHumanTarget.object?bundleHumanTarget.object:plan.pawn)){
            result=original(reinterpret_cast<void*>(component),reinterpret_cast<void*>(pawn),location,direction);
            return true;
        }
        namespace d=cfgcompat::cfg35::pamelafinisher;
        struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
            bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}} list;
        std::uintptr_t klass=0,first=0;cfgprobe::lifetime::SlotObservation type{},again{};
        std::uint8_t hunter=2;
        if(!r_.Read(component+d::WeaponRequirements,list)||list.count<0||list.count>16||list.capacity<list.count||list.capacity>32||
           !r_.Read(policy.weapon+cfgprobe::ObjectClass,klass)||!cfgspawn::ObjectSlot(r_,context.objectArray,klass,type)||
           !r_.Read(pawn+d::Hunter,hunter)||hunter>1||
           !cfgbundle::Code(r_,plan.module,cfgkill::disk::bIsHunterSetter,cfgkill::disk::bIsHunterSetterCode))return false;
        if(list.count&&(!cfgprobe::ObjectPointer(list.data)||!r_.Read(list.data,first)||!first))return false;
        const auto writable=[](std::uintptr_t at,std::size_t bytes){MEMORY_BASIC_INFORMATION info{};
            if(!at||VirtualQuery(reinterpret_cast<void*>(at),&info,sizeof(info))!=sizeof(info)||
               info.State!=MEM_COMMIT||info.Type!=MEM_PRIVATE||info.Protect!=PAGE_READWRITE)return false;
            const auto start=reinterpret_cast<std::uintptr_t>(info.BaseAddress);
            return at>=start&&at-start<=info.RegionSize&&bytes<=info.RegionSize-(at-start);};
        if(!writable(pawn+d::Hunter,1)||(list.count&&!writable(list.data,sizeof(std::uintptr_t)))||
           !cfgspawn::ObjectSlot(r_,context.objectArray,klass,again)||!(type==again)||
           !cfgobserver::Same(r_,policy.weapon+cfgprobe::ObjectClass,klass)||
           !cfgescape::Same(r_,component+d::WeaponRequirements,list)||
           !cfgescape::Same(r_,pawn+d::Hunter,hunter))return false;
        result=cfgkill::CallEligibilityWithHeldWeapon(original,reinterpret_cast<void*>(component),reinterpret_cast<void*>(pawn),
            location,direction,reinterpret_cast<volatile char*>(pawn+d::Hunter),list.count?reinterpret_cast<std::uintptr_t*>(list.data):nullptr,first,klass);
        return true;
    }
    bool PamelaDamageEvent(std::uintptr_t function){
        return cfgkill::pamela::Event(r_,context.objectArray,plan.module,bundleJason.pawn.object,function);
    }
#include "PamelaNativeContextRuntimeMembers.hpp"
#include "PamelaNativeEligibilityMembers.hpp"
