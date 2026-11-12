#pragma once
#include "DonorJasonTargeting.hpp"
#include "J21cFeatureFlags.hpp"
#include "J22FeatureFlags.hpp"
#include "J23GrabTiming.hpp"
#include "J24FeatureFlags.hpp"
#include <array>
#include <cstdint>
#include <cmath>
#include <type_traits>
#include <utility>
namespace cfgdonorgrab {
// Exact CFG4FC availability bound cmp edi,6 / unsigned ja: seven0..6 slots.
constexpr unsigned Slots=7,AllSlots=(1u<<Slots)-1;
#ifndef CFG_H244_EXTRACTION_WAIT_BOUND
#define CFG_H244_EXTRACTION_WAIT_BOUND 1
#endif
constexpr std::uint64_t ExtractionHoldWaitMs=3000;
// H2.76: native CanGrabKill turns true ~2.4 s into a hold; the old 2.5 s legacy handoff beat the slot probe and the walk.
constexpr std::uint64_t FallbackDeadlineMs=CFG_H276_HELD_WALK?9000ull:cfgj24::HeldFallbackMs,StaleReleaseMs=CFG_H276_HELD_WALK?3500ull:2500ull;
using Position=cfgdonorhunt::Position;
enum class Outcome {NotEntered,Returned,Uncertain};
enum class Stage {Idle,Waiting,Probing,Scanning,Walking,Committed,Uncertain};
struct Hold {
    std::uintptr_t world=0,jason=0,victim=0;std::int32_t jasonSerial=0,victimSerial=0;
    std::int32_t jasonIndex=0,victimIndex=0;
    bool Same(const Hold& b)const{return world==b.world&&jason==b.jason&&victim==b.victim&&jasonSerial==b.jasonSerial&&victimSerial==b.victimSerial&&jasonIndex==b.jasonIndex&&victimIndex==b.victimIndex;}
    bool Valid()const{return world&&jason&&victim&&jasonSerial>0&&victimSerial>0&&jasonIndex>=0&&victimIndex>=0;}
};
struct Observation {
    Hold hold{};bool dead=false,killing=false,allowed=false,seat=false,extracting=false,struggling=false,finalContext=false;
    Position location{};
    bool water=false,victimDead=false;
};
inline unsigned Count(unsigned value){unsigned result=0;for(value&=AllSlots;value;value&=value-1)++result;return result;}
template<class Random>bool UniformSlot(unsigned mask,int previous,Random&& random,unsigned& selected){
    mask&=AllSlots;
    if(previous>=0&&previous<static_cast<int>(Slots)&&Count(mask)>1)mask&=~(1u<<static_cast<unsigned>(previous));
    const auto count=Count(mask);if(!count)return false;
    // Reject the incomplete low interval rather than bias rank with modulo.
    const auto threshold=static_cast<std::uint32_t>(0u-count)%count;
    for(unsigned attempt=0;attempt<8;++attempt){const std::uint32_t value=random();if(value<threshold)continue;
        auto rank=value%count;for(unsigned i=0;i<Slots;++i)if(mask&(1u<<i)){if(!rank){selected=i;return true;}--rank;}}
    return false; // bounded RNG failure defers; it never invents a default slot
}
struct State {
    Hold hold{};Stage stage=Stage::Idle;
    std::uint64_t observedAt=0,nextProbeAt=0,committedAt=0,walkAt=0;
    unsigned queriedMask=0,availableMask=0,beforeMask=0,afterMask=0,cursor=0,direction=0,scanCount=0;
    std::array<float,8> freeDistances{};std::array<Position,8> points{};
    Position walkOrigin{},lastLocation{};float travelled=0;int lastUsed=-1;
    unsigned walks=0;int walkedDir=-1,excludeDir=-1; // H2.76: scan-and-walk rounds this hold; direction taken / direction to skip on the second walk
    unsigned attempts=0,returns=0,uncertain=0,probes=0,refusals=0,holds=0,cancels=0,grabFinishedCount=0;
    bool staleReleaseAttempted=false,scanned=false,motionOwned=false;
    cfgj21c::HeldDiagnostics diagnostics{};
    cfggrabtiming::Timing timing{};
    bool legacyFallback=false,freshFallbackSafe=false;
    bool grabStarted=false,grabFinished=false;
    // H2.47 (cumulative, not reset per hold): why a hold waited, and how often the scan was skipped for struggling.
    std::uint64_t seatWaitsSeat=0,seatWaitsExtracting=0,scanSkippedStruggling=0;
    std::uint64_t commitGuardWaits=0,postCommitReleases=0,postCommitKills=0,offlineF1Releases=0; // H2.80 (cumulative): kill-montage guard waits, stale releases and kills after a committed kill
    bool CanFallback(std::uint64_t now)const{return CFG_J21C_HELD_RECOVERY&&freshFallbackSafe&&hold.Valid()&&
        !committedAt&&stage!=Stage::Uncertain&&now>=observedAt&&now-observedAt>=FallbackDeadlineMs;}
    static float Distance(const Position& a,const Position& b){const auto x=a.X-b.X,y=a.Y-b.Y,z=a.Z-b.Z;return std::sqrt(x*x+y*y+z*z);}
    void Forget(){timing.End();hold={};stage=Stage::Idle;observedAt=nextProbeAt=committedAt=walkAt=0;
        queriedMask=availableMask=beforeMask=afterMask=cursor=direction=scanCount=0;freeDistances={};points={};
        walkOrigin=lastLocation={};travelled=0;staleReleaseAttempted=scanned=motionOwned=false;
        legacyFallback=freshFallbackSafe=false;grabStarted=grabFinished=false;
        walks=0;walkedDir=excludeDir=-1;}
    template<class Host>bool Drive(Host& host,std::uint64_t now){
        Observation current;freshFallbackSafe=legacyFallback=false;
        if(!host.Observe(current)){++refusals;return stage!=Stage::Idle;}
        if(!current.hold.Valid()){
            if(stage!=Stage::Idle){host.HoldReleased(hold,now);Forget();++cancels;}return false;
        }
        if(!hold.Same(current.hold)){
            if(stage!=Stage::Idle)host.HoldReleased(hold,now);
            Forget();hold=current.hold;stage=Stage::Waiting;observedAt=now;nextProbeAt=now+100;++holds;
            if(CFG_J23_GRAB_TIMING)timing.Begin(now);
            host.LogSlots(hold); // snapshot names/PickedGrabKills mapping, no assumed choke index
        }
        if(current.killing||stage==Stage::Committed)grabStarted=true;
        if(current.victimDead&&grabStarted&&!grabFinished){++grabFinishedCount;grabFinished=true;}
        freshFallbackSafe=!current.dead&&!current.finalContext&&!current.killing&&!current.water&&!current.seat&&!current.extracting;
        host.ClearSpeculativeCombatTimer(); // compatibility host preserves the old timer when recovery is enabled
        // Water forbids OUR walking destination, not the game's swim kill.
        // Native original Tick owns the water hold; D may only retire its path.
        if(current.water){Stop(host);return false;}
        if(stage==Stage::Uncertain)return true;
        // H2.44: a car extraction that ends in this hold completes only with
        // the kill, so an unbounded wait deadlocks (live H2.43: SeatWaits=333,
        // CarExtractionCommitted=1/Completed=0, no kill). Wait at most
        // ExtractionHoldWaitMs per hold, then let the normal held-kill stages
        // run; the native kill input still decides whether a kill is allowed.
        if((current.seat||current.extracting)&&
           !(CFG_H244_EXTRACTION_WAIT_BOUND&&now>=observedAt&&now-observedAt>=ExtractionHoldWaitMs)){
            seatWaitsSeat+=current.seat;seatWaitsExtracting+=current.extracting;
            ++diagnostics.seatWaits;host.PreserveExtraction(current,now);return true;}
        if(current.dead||current.finalContext||current.killing){
            diagnostics.deadWaits+=current.dead;diagnostics.finalWaits+=current.finalContext;diagnostics.killingWaits+=current.killing;
            nextProbeAt=now+200;return true;}
        if(CFG_J23_GRAB_TIMING&&current.allowed)timing.Allowed(now);
        if(CanFallback(now)&&Stop(host)){legacyFallback=true;++diagnostics.fallbacks;return false;}
        if(stage==Stage::Committed){
            if(now<committedAt||now-committedAt<5000)return true;
            // H2.80: a native kill montage can outlast the donor's five seconds (live H2.79: death at +6.7 s); our stale release then cancelled it.
            if(CFG_H280_KILL_MONTAGE_GUARD&&now-committedAt<static_cast<std::uint64_t>(CFG_H280_KILL_MONTAGE_MS)){++commitGuardWaits;return true;}
            stage=Stage::Waiting;nextProbeAt=now; // donor gives native montage five seconds
        }
        if(motionOwned){
            const auto moved=Distance(lastLocation,current.location);
            if(std::isfinite(moved)&&moved>=0)travelled+=moved;
            lastLocation=current.location;
        }
        if(now<nextProbeAt)return true;
        if(!current.allowed){
            ++diagnostics.allowedWaits;
            nextProbeAt=now+250;
            if(now>=observedAt&&now-observedAt>=StaleReleaseMs&&!staleReleaseAttempted){
                if(!Stop(host))return true;
                const auto released=host.ReleaseStale(hold,now);
                if(released==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
                if(released==Outcome::Returned){staleReleaseAttempted=true;nextProbeAt=now+1000;postCommitReleases+=committedAt!=0;}
            }
            return true;
        }
        if(stage!=Stage::Probing&&stage!=Stage::Scanning){queriedMask=availableMask=0;cursor=0;stage=Stage::Probing;}
        if(stage==Stage::Probing){
            unsigned turnProbes=0;
            while(cursor<Slots){bool available=false;
                const auto result=host.ProbeSlot(hold,cursor,available);
                if(result==Outcome::NotEntered){++diagnostics.notEntered[0];if(CFG_J21C_HELD_RECOVERY)nextProbeAt=now+100;return true;}
                if(result==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
                if(CFG_J23_GRAB_TIMING&&available)timing.Available(now,cursor);
                queriedMask|=1u<<cursor;if(available)availableMask|=1u<<cursor;++cursor;++probes;
                if(CFG_J21C_HELD_RECOVERY&&++turnProbes>=2&&cursor<Slots){nextProbeAt=now+25;return true;}
            }
            if(queriedMask!=AllSlots)return true; // no prefix/default decision
            if(!scanned&&!motionOwned)beforeMask=availableMask;afterMask=availableMask;
            // H2.76: struggling no longer ends the walk (it only makes an already-available slot kill at once).
            const bool cap=(!CFG_H276_HELD_WALK&&current.struggling)||(scanned&&
                ((now>=walkAt&&now-walkAt>=2500)||travelled>=400.f));
            if(Count(availableMask)>=2||((cap||(CFG_H276_HELD_WALK&&current.struggling))&&availableMask))return Kill(host,current,now);
            if(scanned){
                if(cap){
                    if(!Stop(host))return true;
                    if(CFG_H276_HELD_WALK&&walks<2){ // first walk found no slot: scan again from here, never straight back
                        excludeDir=walkedDir>=0?(walkedDir+4)%8:-1;walkedDir=-1;scanned=false;
                    }else{
                    if(!staleReleaseAttempted){const auto released=host.ReleaseStale(hold,now);
                        if(released==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
                        if(released==Outcome::Returned)staleReleaseAttempted=true;}
                    stage=Stage::Waiting;nextProbeAt=now+250;return true;
                    }
                }else{stage=Stage::Walking;nextProbeAt=now+250;return true;}
            }
            stage=Stage::Scanning;cursor=0;scanCount=1;++walks;travelled=0;walkAt=now;walkOrigin=current.location;
        }
        if(stage==Stage::Scanning){
            if((!CFG_H276_HELD_WALK&&current.struggling)||(now>=walkAt&&now-walkAt>=2500)){
                scanSkippedStruggling+=(!CFG_H276_HELD_WALK&&current.struggling);
                scanned=true;stage=Stage::Waiting;nextProbeAt=now;return true;
            }
            while(cursor<8){float free=0;Position point{};bool safe=false;
                const auto result=host.ProbeOpenDirection(hold,walkOrigin,cursor,400.f,free,point,safe);
                if(result==Outcome::NotEntered)return true;
                if(result==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
                freeDistances[cursor]=safe&&std::isfinite(free)&&free>=0&&free<=400?free:0.f;points[cursor]=point;++cursor;
            }
            scanned=true;direction=0;if(CFG_H276_HELD_WALK&&excludeDir>=0&&excludeDir<8)freeDistances[excludeDir]=0.f;
            if(CFG_H276_HELD_WALK)walkedDir=-1;
            for(unsigned i=1;i<8;++i){const auto neighbours=freeDistances[(i+7)%8]+freeDistances[(i+1)%8];
                const auto bestNeighbours=freeDistances[(direction+7)%8]+freeDistances[(direction+1)%8];
                if(freeDistances[i]>freeDistances[direction]||(freeDistances[i]==freeDistances[direction]&&neighbours>bestNeighbours))direction=i;
            }
            if(freeDistances[direction]<=0){walkAt=now;motionOwned=true;travelled=400;stage=Stage::Waiting;nextProbeAt=now;return true;}
            const auto result=host.MoveHolding(hold,points[direction]);
            if(result==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
            if(result==Outcome::NotEntered){nextProbeAt=now+25;return true;} // same completed scan, no new scan/path before entry
            walkOrigin=lastLocation=current.location;travelled=0;motionOwned=true;if(CFG_H276_HELD_WALK)walkedDir=static_cast<int>(direction);
            stage=Stage::Walking;nextProbeAt=now+250;
        }
        return true;
    }
    template<class Host>bool Stop(Host& host){
        if(!motionOwned)return true;const auto stopped=host.StopHolding(hold);
        if(stopped==Outcome::Uncertain){motionOwned=false;stage=Stage::Uncertain;++uncertain;return false;}
        if(stopped!=Outcome::Returned)return false;motionOwned=false;return true;
    }
    template<class Host,class=void>struct HasKillAllowed:std::false_type{};
    template<class Host>struct HasKillAllowed<Host,std::void_t<decltype(std::declval<Host&>().HeldKillAllowed())>>:std::true_type{};
    template<class Host>static bool KillAllowed(Host& host){if constexpr(HasKillAllowed<Host>::value)return host.HeldKillAllowed();else return true;}
    template<class Host>bool Kill(Host& host,const Observation& current,std::uint64_t now){
        if(!Stop(host))return true;
        if(CFG_H282_F1_OFFLINE_NO_HELD_KILL&&!KillAllowed(host)){ // H2.82: offline F1 never kills from a hold; release the victim alive (same as the no-slot cap path)
            if(!staleReleaseAttempted){const auto released=host.ReleaseStale(hold,now);
                if(released==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
                if(released==Outcome::Returned){staleReleaseAttempted=true;++offlineF1Releases;}}
            stage=Stage::Waiting;nextProbeAt=now+250;return true;}
        unsigned slot=0;if(!UniformSlot(availableMask,lastUsed,[&](){return host.Random();},slot)){
            stage=Stage::Waiting;nextProbeAt=now+250;return true;}
        // Reproving native gateway includes current hold + native availability;
        // stale mask is never permission to send an invalid/escaped index.
        const auto result=host.Kill(hold,slot);host.LogAttempt(hold,availableMask,slot,result);
        if(result==Outcome::NotEntered){stage=Stage::Waiting;nextProbeAt=now+250;return true;}
        ++attempts;lastUsed=static_cast<int>(slot);postCommitKills+=committedAt!=0;
        if(result==Outcome::Uncertain){stage=Stage::Uncertain;++uncertain;return true;}
        ++returns;committedAt=now;stage=Stage::Committed;nextProbeAt=now+350;
        grabStarted=true;
        host.CombatDelayed(current.hold.jason,now+700);return true;
    }
};
// H2.83 report-only read of the native kill-completion gates (CFG 3.5 image). GrabKillInput 0x435850 -> GrabActors[slot] vfunc 0x5E8
// -> USCContextKillComponent::ActivateGrabKill 0x34F840 -> SERVER/MULTICAST_ActivateContextKill. EndAction (component vtable 0x216CB58
// slot 0x4E0 = 0x0/*CFG3.5 address withheld*/) kills the victim only via ForceKillCharacter (counselor vtable 0x219DA70 slot 0xDF0 -> TakeDamage 0x40F060) when
// CDO(special move)+0x13F && GetOwnerRole()==ROLE_Authority(3) && victim valid. Special move: per-weapon map (+0x326 on, +0x328/+0x330,
// 0x20-byte {move,weaponClass} rows, matched by killer+0xF98 IsA) else +0x350 (resolver 0x3672A0). Pure reads; never a native call.
namespace h283 {
constexpr std::uintptr_t GrabActors=0x1200,KillComponent=0x368,ComponentOwner=0xE0,ActorRole=0x110,Killer=0x2A0,Victim=0x3D8,
    PerWeapon=0x326,MoveMap=0x328,MoveCount=0x330,DefaultMove=0x350,MoveRow=0x20,Weapon=0xF98,ObjectClass=0x10,SuperStruct=0x30,
    ClassDefault=0xF8,KillFlag=0x13F,Health=0xE58,ContextKill=0x1070,Invulnerable=0x1525;
template<class R,class Row>void ObserveKillGates(R& r,std::uintptr_t jason,std::uintptr_t victim,unsigned slot,Row& row){
    std::uintptr_t actor=0,component=0,owner=0,killer=0,held=0,weapon=0,type=0,data=0,move=0,cdo=0;
    std::int32_t count=-1;std::uint8_t value=0;float health=0;row.clause=1;
    if(slot<Slots&&r.Read(jason+GrabActors+8*slot,actor)&&actor&&r.Read(actor+KillComponent,component)&&component){row.clause=2;
        if(r.Read(component+ComponentOwner,owner)&&owner&&r.Read(owner+ActorRole,value))row.ownerRole=value;
        if(r.Read(component+Killer,killer))row.compKiller=killer==jason?1u:killer?2u:0u;
        if(r.Read(component+Victim,held))row.compVictim=held==victim?1u:held?2u:0u;
        if(r.Read(component+PerWeapon,value))row.perWeapon=value;
        if(r.Read(jason+Weapon,weapon))row.weapon=weapon?1u:0u;
        if(r.Read(component+DefaultMove,move)){row.clause=3;
            if(row.perWeapon&&row.perWeapon!=255&&weapon&&r.Read(weapon+ObjectClass,type)&&r.Read(component+MoveMap,data)&&
               r.Read(component+MoveCount,count)&&count>=0&&count<=8){row.mapEntries=static_cast<std::uint32_t>(count);row.matchedEntry=254;
                for(std::int32_t i=0;i<count&&row.matchedEntry==254;++i){std::uintptr_t candidate=0,weaponClass=0,at=type;
                    if(!r.Read(data+MoveRow*static_cast<unsigned>(i),candidate)||!r.Read(data+MoveRow*static_cast<unsigned>(i)+8,weaponClass))break;
                    for(unsigned depth=0;weaponClass&&at&&depth<12;++depth){if(at==weaponClass){move=candidate;row.matchedEntry=static_cast<std::uint32_t>(i);break;}
                        if(!r.Read(at+SuperStruct,at))break;}
                }
            }
            if(move&&r.Read(move+ClassDefault,cdo)&&cdo&&r.Read(cdo+KillFlag,value)){row.moveKillFlag=value;row.clause=4;}
        }
    }
    if(victim&&r.Read(victim+Health,health))row.victimHealth=health;
    if(victim&&r.Read(victim+ContextKill,value))row.victimContext=value;
    if(victim&&r.Read(victim+Invulnerable,value))row.victimInvulnerable=value;
}
}
}
