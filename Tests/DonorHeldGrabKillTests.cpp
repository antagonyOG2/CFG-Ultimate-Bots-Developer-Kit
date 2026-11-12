#ifndef CFG_J21C_HELD_RECOVERY
#define CFG_J21C_HELD_RECOVERY 0
#endif
#include "../Core/DonorHeldGrabKill.hpp"
#include <cstdio>
#include <map>
using namespace cfgdonorgrab;
struct H283Memory {std::map<std::uintptr_t,std::uint8_t> bytes;unsigned reads=0; // H2.83 fake image for the EndAction gate reads
    template<class T>void Put(std::uintptr_t at,T value){for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=reinterpret_cast<const std::uint8_t*>(&value)[i];}
    template<class T>bool Read(std::uintptr_t at,T& out){++reads;T value{};for(unsigned i=0;i<sizeof(T);++i){const auto f=bytes.find(at+i);if(f==bytes.end())return false;
        reinterpret_cast<std::uint8_t*>(&value)[i]=f->second;}out=value;return true;}};
struct H283Row {float victimHealth=-1;std::uint32_t clause=0,ownerRole=255,compKiller=255,compVictim=255,perWeapon=255,weapon=255,mapEntries=255,
    matchedEntry=255,moveKillFlag=255,victimContext=255,victimInvulnerable=255;};
struct Host {
    Observation sample{{0x100000,0x200000,0x300000,2,3},false,false,true,false,false,false,false,{}};
    unsigned mask=3,probeLimit=7,probed=0,scans=0,moves=0,stops=0,kills=0,releases=0,clears=0,quarantines=0;
    bool killAllowed=true;bool HeldKillAllowed()const{return killAllowed;}
    unsigned slot=99,scanLimit=8;std::uint32_t rng=1234;bool observed=true,safe=true;
    std::array<Position,8> origins{};
    Outcome input=Outcome::Returned,stopOutcome=Outcome::Returned;std::array<float,8> free{{200,300,400,300,200,100,100,100}};
    bool Observe(Observation& out){out=sample;return observed;}
    void HoldReleased(const Hold&,std::uint64_t){++quarantines;}
    void ClearSpeculativeCombatTimer(){++clears;}
    void LogSlots(const Hold&){}void PreserveExtraction(const Observation&,std::uint64_t){}
    Outcome ReleaseStale(const Hold&,std::uint64_t){++releases;return Outcome::Returned;}
    Outcome ProbeSlot(const Hold&,unsigned i,bool& value){if(probed>=probeLimit)return Outcome::NotEntered;
        ++probed;value=(mask&(1u<<i))!=0;return Outcome::Returned;}
    Outcome ProbeOpenDirection(const Hold&,const Position& origin,unsigned i,float distance,float& amount,Position& point,bool& provedSafe){
        if(scans>=scanLimit)return Outcome::NotEntered;origins[i]=origin;
        ++scans;amount=free[i];point={origin.X+distance,origin.Y,origin.Z};provedSafe=safe;return Outcome::Returned;}
    Outcome MoveHolding(const Hold&,const Position&){++moves;return Outcome::Returned;}
    Outcome StopHolding(const Hold&){++stops;return stopOutcome;}
    Outcome Kill(const Hold&,unsigned picked){++kills;slot=picked;return input;}
    void LogAttempt(const Hold&,unsigned,unsigned,Outcome){}void CombatDelayed(std::uintptr_t,std::uint64_t){}
    std::uint32_t Random(){rng=rng*1664525u+1013904223u;return rng;}
    void ResetBudget(unsigned maximum=7){probed=0;probeLimit=maximum;}
};
int main(){unsigned failures=0,checks=0;auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    {
        std::uint32_t random=42;std::array<unsigned,Slots> counts{};
        for(unsigned trial=0;trial<70000;++trial){unsigned slot=99;
            check(UniformSlot(AllSlots,-1,[&](){random=random*1664525u+1013904223u;return random;},slot)&&slot<7,"seven-slot uniform choice bounded");++counts[slot];}
        for(auto n:counts)check(n>9500&&n<10500,"all7slots covered without directional bias");
        for(int last=0;last<7;++last){unsigned slot=99;
            check(UniformSlot(AllSlots,last,[&](){random=random*1664525u+1013904223u;return random;},slot)&&slot!=static_cast<unsigned>(last),"exclude last-used with alternatives");}
        unsigned slot=99;check(UniformSlot(1u<<6,6,[](){return 0u;},slot)&&slot==6,"sole slot6 allowed even if previous");
        check(!UniformSlot(1u<<7,-1,[](){return 0u;},slot),"slot7 never accepted");
        check(!UniformSlot(AllSlots,-1,[](){return 0u;},slot),"bounded RNG reject interval never biases fallback");
    }
    {
        State s;Host h;s.Drive(h,1000);s.Drive(h,1099);check(!h.probed&&!h.kills,"new hold100ms settle");
        h.ResetBudget(3);s.Drive(h,1100);check(s.queriedMask==7&&!h.kills,"partial mask never becomes complete decision");
        h.ResetBudget(4);s.Drive(h,1110);check(s.queriedMask==AllSlots&&h.kills==1&&h.slot<2&&!h.scans,"all7queries then immediate random if2available");
        h.ResetBudget();s.Drive(h,1460);check(h.kills==1,"committed input not replayed while montage may start");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);
        check(h.scans==8&&h.moves==1&&!h.kills&&s.direction==2,"single slot one8direction scan chooses most open");
        h.ResetBudget();s.Drive(h,1349);check(!h.probed,"walk probes no faster250ms");
        h.mask=(1u<<4)|(1u<<6);h.ResetBudget();s.Drive(h,1350);
        check(h.kills==1&&(h.slot==4||h.slot==6)&&h.stops==1&&h.scans==8,"new available customization slots stop then random kill without rescan");
        check(s.beforeMask==1&&s.afterMask==h.mask&&s.scanCount==1,"capture mask before/after and one scan");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);
        for(std::uint64_t now=1350;now<3600;now+=250){h.ResetBudget();s.Drive(h,now);}
        check(!h.kills&&h.scans==8,"no early fallback or repeated scan");
        h.ResetBudget();s.Drive(h,3600);check(h.kills==1&&h.slot==0&&h.stops==1,"2500ms cap accepts sole available fallback");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);h.sample.location={401,0,0};h.ResetBudget();s.Drive(h,1350);
        check(h.kills==1&&s.travelled>=400,"400cm cap stops walking");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);h.sample.hold.victim=0;s.Drive(h,1200);
        check(s.stage==Stage::Idle&&h.quarantines==1&&!h.kills,"lost hold cancels and enters post-grab quarantine");
    }
    {
        State s;Host h;h.sample.seat=true;s.Drive(h,1000);s.Drive(h,1100);
        check(!h.probed&&!h.scans&&!h.moves&&!h.kills,"seat/extraction retains paired native owner");
        h.sample.seat=false;h.sample.finalContext=true;s.Drive(h,1400);check(!h.probed,"Pamela final context cannot be interrupted");
    }
    {
        State s;Host h;h.sample.extracting=true;s.Drive(h,1000);s.Drive(h,3900);
        check(!h.probed&&!h.kills,"H2.44 extraction wait holds below the bound");
        s.Drive(h,4000);s.Drive(h,4200);
        check(h.probed,"H2.44 extraction wait is bounded: held-kill stages resume after 3000 ms");
    }
    {
        State s;Host h;h.mask=1;h.safe=false;s.Drive(h,1000);s.Drive(h,1100);h.ResetBudget();s.Drive(h,1110);
        check(!h.moves&&h.kills==1&&h.slot==0,"no proved safe ground never moves into unsafe area");
    }
    {
        State s;Host h;h.input=Outcome::Uncertain;s.Drive(h,1000);s.Drive(h,1100);h.ResetBudget();s.Drive(h,1500);
        check(s.stage==Stage::Uncertain&&s.uncertain==1&&h.kills==1,"ambiguous entered kill quarantines, no retry");
    }
    {
        State s;Host h;h.sample.killing=true;s.Drive(h,1000);h.sample.killing=false;h.sample.dead=true;h.sample.victimDead=true;
        s.Drive(h,1100);check(s.grabFinishedCount==1,"native killing plus confirmed victim death counts one completed grab");
        s.Drive(h,1200);check(s.grabFinishedCount==1,"one held victim cannot count twice");
        h.sample.hold.victim=0;s.Drive(h,1300);check(s.grabFinishedCount==1,"completed count survives hold retirement");
    }
    {
        State s;Host h;h.mask=1;h.scanLimit=3;s.Drive(h,1000);s.Drive(h,1100);
        check(s.stage==Stage::Scanning&&h.scans==3,"partial openness scan continues without movement");
        h.sample.location={100,0,0};h.scanLimit=8;h.ResetBudget();s.Drive(h,1125);
        check(h.scans==8&&h.moves==1&&h.origins[7].X==0,"all directions share captured scan origin across ticks");
        h.sample.water=true;h.ResetBudget();const bool owns=s.Drive(h,1200);
        check(!owns&&h.stops==1&&!h.kills&&!s.motionOwned,"water yields ownership to original swim kill after one path stop");
        check(!s.Drive(h,1250)&&h.stops==1&&!h.kills,"water never owns or repeats a returned path stop");
    }
    {
        State s;Host h;h.mask=1;h.scanLimit=2;s.Drive(h,1000);s.Drive(h,1100);
        h.scanLimit=8;s.Drive(h,3700);h.ResetBudget();s.Drive(h,3701);
        check(!h.moves&&h.scans==2&&h.kills==1,"deferred scan respects2500ms cap; no late path starts");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);
        h.sample.allowed=false;s.Drive(h,1000+StaleReleaseMs+100);
        check(h.stops==1&&h.releases==1,"stale hold stops owned path before native cancellation");
    }
    {
        State s;Host h;h.mask=1;s.Drive(h,1000);s.Drive(h,1100);h.sample.water=true;h.stopOutcome=Outcome::Uncertain;
        check(!s.Drive(h,1200)&&h.stops==1&&s.stage==Stage::Uncertain,"uncertain water path stop yields native ownership");
        check(!s.Drive(h,1250)&&h.stops==1&&!h.kills,"uncertain water stop never replayed or replaced by D kill");
    }
    // H2.76: held walk. Driven every 25 ms with a fresh probe budget so it holds with or without bounded-probe recovery.
    const auto run=[&](State& s,Host& h,std::uint64_t from,std::uint64_t to){for(std::uint64_t t=from;t<to;t+=25){h.ResetBudget();s.Drive(h,t);}};
    check(FallbackDeadlineMs==(CFG_H276_HELD_WALK?9000u:cfgj24::HeldFallbackMs)&&StaleReleaseMs==(CFG_H276_HELD_WALK?3500u:2500u),"H2.76 deadlines follow the flag; flag 0 keeps 2500/2500");
#if CFG_H276_HELD_WALK
    {
        State s;Host h;h.mask=0;h.sample.struggling=true;s.Drive(h,1000);run(s,h,1100,1500);
        check(h.moves==1&&!h.kills&&s.scanSkippedStruggling==0&&s.motionOwned,"H2.76 struggling with no slot still scans and walks");
    }
    {
        State s;Host h;h.mask=1;h.sample.struggling=true;s.Drive(h,1000);run(s,h,1100,1500);
        check(h.kills==1&&!h.scans&&!h.moves,"H2.76 struggling with an available slot kills immediately, no walk");
    }
    {
        State s;Host h;h.mask=0;h.scanLimit=16;h.free={{0,0,400,0,0,0,300,0}};s.Drive(h,1000);run(s,h,1100,3500);
        check(h.moves==1&&s.direction==2&&s.walks==1&&!h.releases,"H2.76 first walk toward the most open direction");
        h.free={{0,0,0,0,0,0,400,100}};run(s,h,3500,4200);
        check(h.moves==2&&s.walks==2&&s.direction==7&&!h.releases&&!h.kills,"H2.76 second walk excludes the direction opposite the first (6), takes 7");
        run(s,h,4200,7000);
        check(h.releases==1&&h.stops==2&&!h.kills&&!s.motionOwned&&h.moves==2&&h.scans==16,"H2.76 release only after the second walk finds nothing; no third walk");
        s.Drive(h,7025);h.sample.hold.victim=0;s.Drive(h,7050);check(s.walks==0&&s.excludeDir==-1,"H2.76 walks counter reset by Forget");
    }
    {
        State s;Host h;h.mask=0;h.scanLimit=16;s.Drive(h,1000);run(s,h,1100,3500);h.mask=1<<3;run(s,h,3500,3800);
        check(h.kills==1&&h.slot==3&&h.moves==1,"H2.76 slot opening during the walk kills at the walk cap, no second walk");
    }
#endif
#if CFG_H280_KILL_MONTAGE_GUARD
    {
        State s;Host h;h.mask=3;s.Drive(h,1000);h.ResetBudget();s.Drive(h,1110);h.ResetBudget();s.Drive(h,1120);
        check(h.kills==1&&s.stage==Stage::Committed,"H2.80 fixture commits one kill");
        h.mask=0;h.sample.allowed=false;
        for(std::uint64_t now=1500;now<1110+12000;now+=250){h.ResetBudget();s.Drive(h,now);}
        check(h.kills==1&&!h.releases&&!h.moves&&s.stage==Stage::Committed&&s.commitGuardWaits>0,"H2.80 no release/walk/kill while the montage guard runs (victim held, alive)");
        h.ResetBudget();s.Drive(h,1110+12000);s.Drive(h,1110+12250);
        check(h.releases==1&&s.postCommitReleases==1,"H2.80 stale release resumes after the guard window and is counted");
    }
#endif
#if CFG_H282_F1_OFFLINE_NO_HELD_KILL
    {
        State s;Host h;h.mask=3;h.killAllowed=false;s.Drive(h,1000);h.ResetBudget();s.Drive(h,1110);h.ResetBudget();s.Drive(h,1120);
        check(!h.kills&&h.releases==1&&s.offlineF1Releases==1&&s.stage==Stage::Waiting&&s.committedAt==0,"H2.82 offline F1: no kill sent, one alive release, back to Waiting");
    }
#endif
#if CFG_H283_F1_KILL_DIAG
    {   // H2.83 fixture: fake image of the native EndAction inputs (grab actor -> component -> owner role, killer/victim, weapon-matched special move CDO flag).
        using Memory=H283Memory;using Row=H283Row;namespace g=cfgdonorgrab::h283;
        const std::uintptr_t jason=0x10000,victim=0x20000,actor=0x30000,component=0x40000,owner=0x50000,weapon=0x60000,weaponType=0x61000,weaponBase=0x62000,
            map=0x70000,lethal=0x80000,stun=0x81000,lethalCdo=0x90000,stunCdo=0x91000;
        Memory m;m.Put(jason+g::GrabActors+8,actor);m.Put(actor+g::KillComponent,component);m.Put(component+g::ComponentOwner,owner);m.Put(owner+g::ActorRole,std::uint8_t{3});
        m.Put(component+g::Killer,jason);m.Put(component+g::Victim,victim);m.Put(component+g::PerWeapon,std::uint8_t{1});m.Put(component+g::DefaultMove,stun);
        m.Put(component+g::MoveMap,map);m.Put(component+g::MoveCount,std::int32_t{2});
        m.Put(map,stun);m.Put(map+8,std::uintptr_t{0x99000});m.Put(map+g::MoveRow,lethal);m.Put(map+g::MoveRow+8,weaponBase);
        m.Put(jason+g::Weapon,weapon);m.Put(weapon+g::ObjectClass,weaponType);m.Put(weaponType+g::SuperStruct,weaponBase);
        m.Put(lethal+g::ClassDefault,lethalCdo);m.Put(lethalCdo+g::KillFlag,std::uint8_t{1});m.Put(stun+g::ClassDefault,stunCdo);m.Put(stunCdo+g::KillFlag,std::uint8_t{0});
        m.Put(victim+g::Health,100.f);m.Put(victim+g::ContextKill,std::uint8_t{1});m.Put(victim+g::Invulnerable,std::uint8_t{0});
        Row row;g::ObserveKillGates(m,jason,victim,1,row);
        check(row.clause==4&&row.ownerRole==3&&row.compKiller==1&&row.compVictim==1&&row.perWeapon==1&&row.weapon==1&&row.mapEntries==2&&
            row.matchedEntry==1&&row.moveKillFlag==1&&row.victimHealth==100.f&&row.victimContext==1&&row.victimInvulnerable==0,
            "H2.83 gates: weapon base class picks the lethal row, owner role and bound killer/victim read");
        m.Put(weaponType+g::SuperStruct,std::uintptr_t{0});Row fallback;g::ObserveKillGates(m,jason,victim,1,fallback);
        check(fallback.matchedEntry==254&&fallback.moveKillFlag==0&&fallback.clause==4,"H2.83 gates: no weapon row falls back to the default move and reads its flag");
        Row empty;g::ObserveKillGates(m,jason,victim,0,empty);
        check(empty.clause==1&&empty.ownerRole==255&&empty.moveKillFlag==255&&empty.victimHealth==100.f,"H2.83 gates: missing slot actor reads only the victim, nothing invented");
        Row bad;g::ObserveKillGates(m,jason,victim,Slots,bad);check(bad.clause==1&&bad.compKiller==255,"H2.83 gates: slot 7 never read");
    }
#endif
    std::printf("Donor held kill + approved249 positioning: %u checks,%u failures;7slots/complete mask/random/no-repeat,one scan,250ms probes,2500ms/400cm cap,held/seat/water-original/Pamela/refusal/uncertainty. Core behavior only.\n",checks,failures);
    return failures?1:0;
}
