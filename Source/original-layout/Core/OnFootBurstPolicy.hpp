#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#ifndef CFG_H267_BURST_ANY_RANGE
#define CFG_H267_BURST_ANY_RANGE 1 // burst at any range over 4 m while in pursuit; same 3 s and 7 s cooldown
#endif
#ifndef CFG_H285_CHASE_BURST
#define CFG_H285_CHASE_BURST 1 // target within 30 m when the burst starts: 3 s recovery instead of 7 s (a fleeing counselor is not left behind)
#endif
namespace cfgfootburst {
// Initial human-test tuning:3s burst,7s recovery AFTER it finishes. One total5x
// multiplier, never layered over the accepted car lease or native Shift.
constexpr std::uint64_t DurationMs=3000,CooldownMs=7000;
constexpr std::uint64_t ChaseCooldownMs=3000;constexpr float ChaseRangeCm=3000.f;
enum class Stage:unsigned {Idle,Apply,Hold,Restore};
struct Policy {
    Stage stage=Stage::Idle;unsigned cursor=0,mask=0,starts=0,restores=0,refused=0,leaseStarts=0;
    bool leaseMode=false; // H2.46: begun while the donor lease is published; the donor driver is not stopped for it
    std::uint64_t until=0,readyAt=0,cooldown=CooldownMs;std::uintptr_t target=0;unsigned chaseStarts=0;
    std::array<float,4> original{},boosted{};
    bool Owned()const{return stage!=Stage::Idle;}
    // External-reason bit 9 for the donor driver: a lease-mode burst never stops it.
    bool ExternalOwned()const{return Owned()&&!leaseMode;}
    bool Due(std::uint64_t now,bool interrupted)const{return Owned()&&(stage!=Stage::Hold||interrupted||now>=until);}
    // chaseSquared: target distance squared, read AFTER Observe (the host may refresh it there); null = not a chase.
    template<class H>bool Begin(H& h,std::uint64_t now,std::uintptr_t selected,bool underLease=false,const float* chaseSquared=nullptr){
        if(Owned()||!selected||now<readyAt||now>UINT64_MAX-DurationMs-CooldownMs)return false;
        std::array<float,4> values{};
        if(!h.Observe(values)){++refused;readyAt=now+1000;return true;}
        for(const auto value:values)if(!std::isfinite(value)||value<25.f||value>2500.f){++refused;readyAt=now+1000;return true;}
        original=values;for(unsigned i=0;i<4;++i)boosted[i]=values[i]*5.f;
        const bool chase=CFG_H285_CHASE_BURST&&chaseSquared&&std::isfinite(*chaseSquared)&&*chaseSquared>=0.f&&*chaseSquared<=ChaseRangeCm*ChaseRangeCm; // -1 = unknown, never near
        cooldown=chase?ChaseCooldownMs:CooldownMs;if(chase)++chaseStarts;
        target=selected;cursor=mask=0;until=now+DurationMs;readyAt=until+cooldown;
        stage=Stage::Apply;++starts;leaseMode=underLease;if(underLease)++leaseStarts;return true;
    }
    template<class H>bool Step(H& h,std::uint64_t now,bool interrupted){
        if(!Due(now,interrupted))return false;
        if(interrupted||now>=until){if(stage!=Stage::Restore){stage=Stage::Restore;cursor=0;}}
        if(stage==Stage::Apply){
            const auto index=cursor;mask|=1u<<index; // own ambiguous attempted CAS before entering
            if(!h.Exchange(index,original[index],boosted[index])){++refused;stage=Stage::Restore;cursor=0;return true;}
            if(++cursor==4){stage=Stage::Hold;cursor=0;}return true;
        }
        if(stage==Stage::Restore){
            while(cursor<4&&!(mask&(1u<<cursor)))++cursor;
            if(cursor<4){
                // Host performs a current-identity, compare-only restoration.
                // A later foreign value is preserved, never overwritten.
                if(!h.Restore(cursor,boosted[cursor],original[cursor])){++refused;return true;}
                mask&=~(1u<<cursor);++cursor;
            }
            if(!mask){stage=Stage::Idle;cursor=0;target=0;leaseMode=false;++restores;
                // Interrupted bursts also get a real cooldown after cleanup.
                const auto after=now>UINT64_MAX-cooldown?UINT64_MAX:now+cooldown;
                if(after>readyAt)readyAt=after;}
            return true;
        }
        return false;
    }
};
}
