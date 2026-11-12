#pragma once
#include "DonorJasonMorph.hpp"
#include "H246BurstPolicy.hpp"
#include "J22FeatureFlags.hpp"
#include <algorithm>
#include <array>
// H2.84 counselor heading from two position samples (>=250 ms apart, newest <=1.5 s old). Pure state, no reads.
namespace cfgcutoff {
struct Sample {std::uintptr_t object=0;float x=0,y=0,px=0,py=0;std::uint64_t at=0,pat=0;float lvx=0,lvy=0;std::uint64_t lat=0;};
struct Tracker {
    std::array<Sample,8> samples{};unsigned next=0;
    void Note(std::uintptr_t object,float x,float y,std::uint64_t now){
        if(!object||!std::isfinite(x)||!std::isfinite(y))return;
        for(auto& s:samples)if(s.object==object){if(now>=s.at+250){s.px=s.x;s.py=s.y;s.pat=s.at;s.x=x;s.y=y;s.at=now;
                // H2.95: remember the last RUNNING heading (>=150 cm/s) for the fallback below.
                if(s.pat&&now-s.pat<=2000){const float dt=static_cast<float>(now-s.pat)/1000.f,vx=(x-s.px)/dt,vy=(y-s.py)/dt;
                    if(std::isfinite(vx)&&std::isfinite(vy)&&vx*vx+vy*vy>=150.f*150.f){s.lvx=vx;s.lvy=vy;s.lat=now;}}}
            return;}
        samples[next++%samples.size()]={object,x,y,x,y,now,0,0,0,0};
    }
    bool Heading(std::uintptr_t object,std::uint64_t now,float& vx,float& vy)const{
        for(const auto& s:samples)if(s.object==object){
            if(s.pat&&s.at>s.pat&&now>=s.at&&now-s.at<=1500&&s.at-s.pat<=2000){
                const float dt=static_cast<float>(s.at-s.pat)/1000.f;vx=(s.x-s.px)/dt;vy=(s.y-s.py)/dt;
                if(std::isfinite(vx)&&std::isfinite(vy)&&(!CFG_H295_HEADING_FALLBACK||vx*vx+vy*vy>=150.f*150.f))return true;}
            // H2.95: no fresh running heading (stale samples, or a brief stop/turn) -> last running heading if at most 2 s old.
            if(CFG_H295_HEADING_FALLBACK&&s.lat&&now>=s.lat&&now-s.lat<=2000){vx=s.lvx;vy=s.lvy;return true;}
            return false;}
        return false;
    }
};
}
#include "../RuntimeProbe/ObjectLifetime.hpp"

namespace cfgdonorhunt {
// Native admission is separate from a returned UE boolean. In particular,
// an unentered call is neither a failed nav candidate nor a used Morph.
enum class NativeOutcome {NotEntered,ReturnedFalse,ReturnedTrue,Uncertain};
enum class NavProof {Unavailable,Changed,Current};
struct NavRingIdentity {
    std::uintptr_t module=0;
    std::uint32_t thread=0;
    // Current World, owned Jason, owned controller, target. Weak observations
    // only; the host must repeat typed/world/possession/lifetime/root proofs.
    std::array<cfgprobe::lifetime::SlotObservation,4> slots{};
    bool Valid()const {
        if(!module||!thread)return false;
        for(const auto& s:slots)if(!s.HasAssignedSerial())return false;
        return true;
    }
    bool operator==(const NavRingIdentity& other)const {
        return module==other.module&&thread==other.thread&&slots==other.slots;
    }
};
enum class NavRingPurpose {Initial,Distance,SharedFifty,Stuck,Additional,StartupFinish};
enum class NavRingStage {Idle,Project,Teleport,Succeeded,Failed,Invalid,Uncertain};
struct NavRingContinuation {
    NavRingIdentity identity{};
    NavRingPurpose purpose=NavRingPurpose::Initial;
    NavRingStage stage=NavRingStage::Idle;
    Position target{},projected{};
    unsigned candidate=0;
    bool uncertainTeleport=false;
    // H2.46: per-ring diagnostics. starved = roof-guard probes that could not be ENTERED for the
    // current candidate (budget), dz/horizontal = candidates the donor's own filters rejected.
    unsigned starved=0,dzRejects=0,horizontalRejects=0,roofRejects=0,probeStarved=0,probeStarvedRetried=0,probeStarvedGaveUp=0;
    // H2.84 cut-off: with a known counselor heading, try 4 close landings straight ahead of the runner first
    // (indices 16-19), then the donor's 16 ring cells, most-ahead first. Without a heading: donor order, 16 cells.
    bool lead=false;float headX=0,headY=0;unsigned count=16;std::array<std::uint8_t,20> order{};
    static constexpr float LeadDistances[4]={700.f,1000.f,1400.f,1800.f};
    static Position Offset(unsigned i,float hx,float hy){
        constexpr Position ring[]={{2200,0,0},{-2200,0,0},{0,2200,0},{0,-2200,0},
            {1556,1556,0},{1556,-1556,0},{-1556,1556,0},{-1556,-1556,0},
            {3000,0,0},{-3000,0,0},{0,3000,0},{0,-3000,0},
            {2121,2121,0},{2121,-2121,0},{-2121,2121,0},{-2121,-2121,0}};
        if(i<16)return ring[i];const float d=LeadDistances[(i-16)&3];return {hx*d,hy*d,0};
    }
    unsigned Index()const {return lead&&candidate<count?order[candidate]:candidate;}
    bool Pending()const {return stage==NavRingStage::Project||stage==NavRingStage::Teleport;}
    bool Begin(const NavRingIdentity& key,Position destination,NavRingPurpose why){
        // The caller must consume a terminal result before starting another
        // operation. Never erase an ambiguous entered teleport and replay it.
        if(stage!=NavRingStage::Idle||!key.Valid()||!std::isfinite(destination.X)||
           !std::isfinite(destination.Y)||!std::isfinite(destination.Z))return false;
        identity=key;target=destination;purpose=why;candidate=0;lead=false;headX=headY=0;count=16;
        for(unsigned i=0;i<order.size();++i)order[i]=static_cast<std::uint8_t>(i);
        stage=NavRingStage::Project;return true;
    }
    // Only before the first candidate. vx/vy: counselor ground velocity in cm/s. Walking pace or slower keeps the donor order.
    bool Lead(float vx,float vy){
        const float speed=std::sqrt(vx*vx+vy*vy);
        if(!CFG_H284_MORPH_CUTOFF||stage!=NavRingStage::Project||candidate||lead||!std::isfinite(speed)||speed<150.f)return false;
        headX=vx/speed;headY=vy/speed;lead=true;count=20;
        for(unsigned i=0;i<4;++i)order[i]=static_cast<std::uint8_t>(16+i);
        std::array<std::uint8_t,16> ring{};for(unsigned i=0;i<16;++i)ring[i]=static_cast<std::uint8_t>(i);
        const auto ahead=[&](std::uint8_t i){const auto p=Offset(i,0,0);return (p.X*headX+p.Y*headY)/std::sqrt(p.X*p.X+p.Y*p.Y);};
        std::stable_sort(ring.begin(),ring.end(),[&](std::uint8_t l,std::uint8_t r){return ahead(l)>ahead(r)+1e-4f;});
        for(unsigned i=0;i<16;++i)order[4+i]=ring[i];
        return true;
    }
    // H2.46: bounded retries of a candidate whose roof-guard probe was budget-starved (NotEntered).
    static constexpr unsigned MaxStarvedRetries=8;
    bool StarvedRetry(){return CFG_H246_TELEPORT_CONFIRM&&starved<MaxStarvedRetries&&(++starved,true);}
    // Outcome for a landing the roof guard rejected. A probe that was never ENTERED (call/read budget) is
    // no evidence about the landing: retry the same candidate on a fresh callback (bounded), do not burn it.
    NativeOutcome RoofRejected(bool starvedProbe){
        if(starvedProbe){++probeStarved;
            if(StarvedRetry()){++probeStarvedRetried;return NativeOutcome::NotEntered;}
            ++probeStarvedGaveUp;}
        ++roofRejects;return NativeOutcome::ReturnedFalse;
    }
    void Advance(){++candidate;starved=0;stage=candidate>=count?NavRingStage::Failed:NavRingStage::Project;}
};
// A bounded continuation of FEAT6437's *same* sixteen-candidate loop. The
// destination snapshot and successful projection survive admission yields.
// No new timer, candidate, landing geometry or native call cap is introduced.
template<class Host>NavRingStage DonorContinueNavRing(Host& host,NavRingContinuation& s){
    while(s.Pending()){
        const auto proof=host.Current(s.identity);
        if(proof==NavProof::Unavailable)return s.stage;
        if(proof!=NavProof::Current||!s.identity.Valid()||s.candidate>=s.count){s.stage=NavRingStage::Invalid;break;}
        NativeOutcome result=NativeOutcome::NotEntered;const bool teleporting=s.stage==NavRingStage::Teleport;
        if(s.stage==NavRingStage::Project){
            const auto index=s.Index();const auto offset=NavRingContinuation::Offset(index,s.headX,s.headY);
            const float minimum=index>=16?500.0f:1700.0f; // cut-off lands close ahead; donor ring keeps its 17 m floor
            Position p=s.target;p.X+=offset.X;p.Y+=offset.Y;
            p.Z=s.target.Z-90.0f+25.0f;
            result=host.Project(s.identity,p,Position{350,350,110},s.projected);
            if(result==NativeOutcome::ReturnedTrue){
                const float dz=std::fabs(s.projected.Z-(s.target.Z-90.0f));
                const float dx=s.projected.X-s.target.X,dy=s.projected.Y-s.target.Y;
                const float distance=std::sqrt(dx*dx+dy*dy);
                if(!std::isfinite(dz)||dz>120.0f||!std::isfinite(distance)||distance<minimum){
                    if(!std::isfinite(dz)||dz>120.0f)++s.dzRejects;else ++s.horizontalRejects;
                    s.Advance();
                }
                else s.stage=NavRingStage::Teleport;
            }else if(result==NativeOutcome::ReturnedFalse)s.Advance();
        }else{
            result=host.Teleport(s.identity,s.projected);
            if(result==NativeOutcome::ReturnedTrue)s.stage=NavRingStage::Succeeded;
            else if(result==NativeOutcome::ReturnedFalse)s.Advance();
        }
        if(result==NativeOutcome::NotEntered)return s.stage;
        if(result==NativeOutcome::Uncertain){s.uncertainTeleport=teleporting;s.stage=NavRingStage::Uncertain;break;}
    }
    return s.stage;
}
}
