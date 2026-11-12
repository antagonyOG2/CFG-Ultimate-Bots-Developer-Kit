#pragma once
#include "CounselorConvergencePolicy.hpp"
#ifndef CFG_H247_MOB
#define CFG_H247_MOB 1
#endif
namespace cfglean {
// H2.47 mob decision, transcribed from donor DriveCounselorSweaterConvergence /
// TickStockCounselorConvergence (FrozenJasonBridge.cpp): an ARMED bot gets the fight
// goal (Jason target, fight/armed/melee true, flee/seek false); an UNARMED bot is
// given the melee class first and, while it stays unarmed, is left on the stock tree
// (donor "continue"). Lean used to publish Clear (ShouldFlee=true, SeekWeapon=true)
// onto an unarmed bot that was never fighting, which is the run-away signal.
// Clear is only owed to a bot previously published as fighting (it lost its weapon).
enum class MobStep { Grant, Engage, Hold, Release };
inline MobStep MobDecide(bool armed,bool onDesiredClass,bool haveDesired,bool grantBlocked,bool wasFighting){
    if(armed&&(onDesiredClass||!haveDesired))return MobStep::Engage;
    if(haveDesired&&!grantBlocked)return MobStep::Grant;
    if(armed)return MobStep::Engage; // grant refused: keep the weapon it already holds
    return wasFighting?MobStep::Release:MobStep::Hold;
}
// Donor preconvergence: one registered helper candidate every 211 ms.
// The callback performs the caller's typed, current-world/serial/live checks;
// this scheduler owns only the bounded cursor and cadence, never native access.
struct SweaterScan {
    unsigned cursor=0;std::uint64_t next=0;
    template<class Sample> bool Step(std::uint64_t now,unsigned count,Sample&& sample){
        if(!count||now<next)return false;
        const unsigned index=cursor%count;cursor=(index+1)%count;
        next=now>UINT64_MAX-211?UINT64_MAX:now+211;
        return sample(index);
    }
};
inline bool SweaterLatched(bool humanEarned,bool enabled,bool sameWorld,bool positiveBotOwner){
    return humanEarned||(enabled&&sameWorld&&positiveBotOwner);
}
struct Counters {
    std::uint64_t passes=0,calls=0,publications=0,skipped=0,grants=0,moves=0,fearWrites=0,refused=0,
        uncertain=0,missingWeaponClass=0,zeroSerialCache=0,warmSlices=0,cleanup=0,readBudget=0,peakReads=0;
};
// Confirmed values suppress redundant writes only on a freshly re-proved
// positive blackboard identity. They are never a native-call capability.
struct Cache {
    std::array<std::uintptr_t,cfgconvergence::KeyCount> value{};
    std::array<std::uint64_t,cfgconvergence::KeyCount> at{};
    unsigned confirmed=0,quarantined=0;
    bool Needs(unsigned key,std::uintptr_t wanted,std::uint64_t now,bool positive)const{
        if(key>=cfgconvergence::KeyCount||(quarantined&(1u<<key)))return false;
        return !positive||!(confirmed&(1u<<key))||value[key]!=wanted||now>=at[key]+3000;
    }
    void Confirm(unsigned key,std::uintptr_t wanted,std::uint64_t now){value[key]=wanted;at[key]=now;confirmed|=1u<<key;}
    void Quarantine(unsigned key){quarantined|=1u<<key;}
};
inline bool Driving(bool latch,bool trance,bool committed,bool complete){return latch&&!trance&&!committed&&!complete;}
struct Schedule {
    std::uint64_t next=0;unsigned cursor=0,current=cfgconvergence::MaximumBots,phase=0,key=0;
    bool Due(std::uint64_t now,bool primary,bool latch,bool blocked)const{return primary&&latch&&!blocked&&now>=next;}
    void Continue(std::uint64_t now){next=now+25;}
    void Finish(std::uint64_t now){current=cfgconvergence::MaximumBots;phase=key=0;next=now+333;}
};
}
