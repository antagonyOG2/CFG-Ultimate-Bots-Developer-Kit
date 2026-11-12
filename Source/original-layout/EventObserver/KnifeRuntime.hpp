#pragma once
#include "../Core/KnifeAimGeometry.hpp"
#include <cstdint>
#include <cmath>
namespace cfgknife {
struct Identity{std::uintptr_t object=0;int serial=0;bool operator==(const Identity& b)const{return object==b.object&&serial==b.serial;}bool operator!=(const Identity& b)const{return !(*this==b);}};
struct Snapshot{Identity jason{},target{},driver{};int count=-1;bool pressed=false,targetLiving=false,ready=false;float squared=0;bool ownerTerminal=false,hosted=false;};
struct FlightSample{float x=0,y=0,z=0,speed=0;std::uintptr_t updated=0;bool active=false;};
enum class RefreshResult:unsigned{Pending,Ready,Unavailable};
struct NearestRefresh {
    unsigned cursor=0,last=0,winner=0;float squared=0;std::uint64_t until=0;bool active=false,found=false;
    void Start(unsigned helpers,std::uint64_t now){*this={};if(helpers>9||now>UINT64_MAX-2000)return;last=helpers;until=now+2000;active=true;}
    template<class Host>RefreshResult Step(Host& h,std::uint64_t now,unsigned& selected){
        if(!active||now>=until){active=false;return RefreshResult::Unavailable;}
        bool living=false;float distance=0;
        if(h.KnifeCandidate(cursor,living,distance)&&living&&std::isfinite(distance)&&distance>=0&&(!found||distance<squared)){
            winner=cursor;squared=distance;found=true;
        }
        if(cursor++<last)return RefreshResult::Pending;
        active=false;if(!found)return RefreshResult::Unavailable;
        selected=winner;return RefreshResult::Ready;
    }
};
enum class Stage:unsigned{Idle,Aim,Press,Windup,Release,Notify,Flight,Cleanup,Refused,Launch};
struct Runtime{
    Stage stage=Stage::Idle;Snapshot initial{},release{};Identity ownedDriver{};unsigned targetIndex=0;
    std::uint64_t readyAt=0,endAt=0,releasedAt=0,launchedAt=0,nextAt=0,pollAt=0,lastAimAt=0;
    unsigned presses=0,releases=0,launches=0,finalizes=0,refusals=0,timeouts=0;
    unsigned flightSamples=0,flightsMoved=0,flightsStationary=0,flightUnavailable=0;
    FlightSample flightOrigin{},lastFlight{};bool flightOriginValid=false,flightMoved=false;
    bool pressAttempted=false,cleanupReleased=false;
    bool nativeCompletionRequired=false;
    Stage firstAbortStage=Stage::Idle;unsigned firstAbortFailure=UINT32_MAX,firstAbortReads=0,firstAbortCalls=0;bool firstAbortRecorded=false;
    bool Busy()const{return stage!=Stage::Idle&&stage!=Stage::Refused;}
    bool HeldStopCandidate(std::uint64_t now)const{
        return stage==Stage::Windup&&pressAttempted&&initial.jason.object&&initial.jason.serial>0&&
            !initial.driver.object&&!releasedAt&&!launchedAt&&now<endAt;
    }
    bool MaySuppressHeldStop(const Snapshot& fresh,std::uintptr_t driver,std::uintptr_t thrownBy,
                            bool collisionEnabled,std::uint64_t now)const{
        return HeldStopCandidate(now)&&fresh.jason==initial.jason&&fresh.pressed&&fresh.count==initial.count&&
            fresh.driver.object==driver&&fresh.driver.serial>0&&!thrownBy&&collisionEnabled&&
            (!ownedDriver.object||ownedDriver==fresh.driver);
    }
    void Finish(std::uint64_t now,bool refused=false){stage=refused?Stage::Refused:Stage::Idle;pollAt=0;if(refused){++refusals;nextAt=now+5000;}}
    void Abort(std::uint64_t now){if(pressAttempted){stage=Stage::Cleanup;pollAt=now+125;}else Finish(now,true);}
    unsigned abortStages[10]={},abortOwnerObserve=0; // every Abort(h) by stage; failure==1 means owner proved then observe/read failed
    template<class Host>void Abort(Host& h,std::uint64_t now){
        if(static_cast<unsigned>(stage)<10)++abortStages[static_cast<unsigned>(stage)];if(h.KnifeFailureCode()==1)++abortOwnerObserve;
        if(!firstAbortRecorded){firstAbortStage=stage;firstAbortFailure=h.KnifeFailureCode();
            firstAbortReads=h.KnifeReadCount();firstAbortCalls=h.KnifeCallCount();firstAbortRecorded=true;}
        Abort(now);
    }
    bool CompleteNativeNotify(Identity driver,std::uint64_t now){
        if(!nativeCompletionRequired||stage!=Stage::Notify||!(release.driver==driver))return false;
        stage=Stage::Flight;launchedAt=now;++launches;pollAt=now+25;return true;
    }
    template<class Host>bool Begin(Host& h,unsigned index,std::uint64_t now){
        if(Busy()||now<nextAt)return false;Snapshot s{};
        if(!h.KnifeObserve(index,s,true)||!s.ready||!s.targetLiving||!std::isfinite(s.squared)||s.squared<850.f*850.f||s.squared>1550.f*1550.f||s.count<0||s.pressed||s.driver.object)return false;
        // Donor keeps an AI supply of six. Refill is its own entry phase,
        // never combined with Press or replayed over a native throw.
        if(s.count==0||s.count>20){h.KnifeRefill(s);nextAt=now+250;return false;}
        initial=s;release={};ownedDriver={};releasedAt=launchedAt=0;targetIndex=index;pressAttempted=cleanupReleased=false;
        flightOrigin={};lastFlight={};flightOriginValid=flightMoved=false;
        nextAt=now+12000;endAt=now+3500;readyAt=lastAimAt=0;pollAt=now+125;
        if(initial.hosted){
            // Hosted callbacks admit a bounded Begin window. Match the donor's
            // press-first order there, then let a fresh Windup callback aim;
            // keep offline scheduling unchanged. Journal before native input.
            pressAttempted=true;++presses;readyAt=now+900;stage=Stage::Windup;
            if(!h.KnifePress())Abort(h,now);
            return true;
        }
        stage=Stage::Aim;return true;
    }
    template<class Host>void Tick(Host& h,std::uint64_t now){
        if(!Busy()||now<pollAt)return;pollAt=now+125;Snapshot s{};
        if(!h.KnifeObserve(targetIndex,s,stage==Stage::Press)){if(s.ownerTerminal)Finish(now,true);else Abort(h,now);return;}
        if(!(s.jason==initial.jason)){Finish(now,true);return;}
        if(stage==Stage::Flight){
            // The850-1550cm shot crosses its target in~70-130ms. The old125ms
            // first sample could miss the entire crossing. Six short samples
            // only during this150ms window; no idle/high-rate global polling.
            if(now>=launchedAt&&now-launchedAt<150)pollAt=now+25;
            FlightSample sample{};
            if(h.KnifeFlight(release.driver,sample)){
                ++flightSamples;lastFlight=sample;
                if(flightOriginValid){const float dx=sample.x-flightOrigin.x,dy=sample.y-flightOrigin.y,dz=sample.z-flightOrigin.z;
                    if(dx*dx+dy*dy+dz*dz>25.f&&!flightMoved){flightMoved=true;++flightsMoved;}}
                else{flightOrigin=sample;flightOriginValid=true;}
            }else ++flightUnavailable;
            if(now>=releasedAt+500&&now>=launchedAt+250){
                if(flightOriginValid&&!flightMoved)++flightsStationary;
                // Native may already clear Jason.Driver; retain read-only
                // projectile evidence, but never finalize a replacement.
                if(s.driver==release.driver){++finalizes;if(!h.KnifeFinalize(release.driver)){Abort(h,now);return;}}
                Finish(now);
            }return;
        }
        // Latch only the first fresh driver after our own attempted Press.
        // Later pointer/serial replacements never receive launch or cleanup.
        if(pressAttempted&&!ownedDriver.object&&s.driver.object)ownedDriver=s.driver;
        if(ownedDriver.object&&s.driver.object&&!(ownedDriver==s.driver)){Finish(now,true);return;}
        if(stage==Stage::Cleanup){
            if(s.pressed){cleanupReleased=true;++releases;h.KnifeRelease(s.driver);return;}
            if(s.driver.object){++finalizes;if(!h.KnifeFinalize(ownedDriver))return;}
            Finish(now,true);return;
        }
        if(now>=endAt||!s.targetLiving||!(s.target==initial.target)){
            if(now>=endAt)++timeouts;Abort(h,now);return;
        }
        if(stage==Stage::Aim){if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}lastAimAt=now;stage=Stage::Press;return;}
        if(stage==Stage::Press){
            if(!s.ready||!std::isfinite(s.squared)||s.squared<850.f*850.f||s.squared>1550.f*1550.f||s.driver.object||s.pressed||s.count!=initial.count){Abort(h,now);return;}
            pressAttempted=true;++presses;readyAt=now+900;endAt=now+3500;stage=Stage::Windup;
            if(!h.KnifePress())Abort(h,now);return;
        }
        if(stage==Stage::Windup){
            if(!s.driver.object||s.count!=initial.count||!s.pressed){Abort(h,now);return;}
            if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}lastAimAt=now;
            if(now<readyAt)return; // donor keeps body/controller/throw view tracking during windup
            stage=Stage::Release;pollAt=now+25;return;
        }
        if(stage==Stage::Release){
            if(!s.driver.object||s.count!=initial.count||!s.pressed){Abort(h,now);return;}
            // A delayed callback must refresh donor body/view aim before
            // releasing. Separate phases retain the existing read/call caps.
            if(now<lastAimAt||now-lastAimAt>125){
                if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}
                lastAimAt=now;pollAt=now+25;return;
            }
            release=s;releasedAt=now;stage=Stage::Notify;++releases;
            if(!h.KnifeRelease(release.driver))Abort(h,now);else pollAt=now+25;return;
        }
        if(!(s.driver==release.driver)){Finish(now);return;}
        if(stage==Stage::Notify){
            if(s.count==release.count){
                if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}
                lastAimAt=now;return;
            }
            if(s.count!=release.count-1||s.pressed){Abort(h,now);return;}
            // Native boundary is the only corrective launch authority in CFG.
            // Its failure/unknown must never be resurrected by a later tick.
            if(nativeCompletionRequired){Abort(h,now);return;}
            // Stock notify consumed exactly one. Never call ThrowKnife again.
            if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}lastAimAt=now;
            // Aim and launch each reprove owner/target/native bindings. Running
            // both in this callback can exhaust the bounded reader before the
            // local multicast is reached. Launch gets a fresh callback budget.
            stage=Stage::Launch;pollAt=now+25;return;
        }
        if(stage==Stage::Launch){
            if(s.count!=release.count-1||s.pressed){Abort(h,now);return;}
            if(now<lastAimAt||now-lastAimAt>125){
                if(!h.KnifeAim(targetIndex,initial.target)){Abort(h,now);return;}
                lastAimAt=now;pollAt=now+25;return;
            }
            stage=Stage::Flight;launchedAt=now;++launches;
            if(!h.KnifeLaunch(targetIndex,initial.target,release.driver))Abort(h,now);
            else if(h.KnifeFlight(release.driver,flightOrigin)){flightOriginValid=true;lastFlight=flightOrigin;++flightSamples;pollAt=now+25;}
            else ++flightUnavailable;
            return;
        }
    }
};
}
