#pragma once
#include <algorithm>
#include "VehiclePursuitPolicy.hpp"
#include "PursuitSpeedPolicy.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
namespace cfgvehicleaction {
struct Attempt {std::uintptr_t component=0;std::uint8_t method=1,autoLock=0,padding[6]{};};
static_assert(sizeof(Attempt)==16&&offsetof(Attempt,autoLock)==9);
struct DriverRoute {
    bool active=false,reached=false,opposite=false;float endSign=1.f;
    cfgspawn::Vec3 origin{},axis{},waypoint{};float best=0;
    std::uint64_t progressAt=0;
    void Reset(){*this={};}
    bool Goal(cfgspawn::Vec3 car,cfgspawn::Vec3 forward,cfgspawn::Vec3 door,cfgspawn::Vec3 jason,std::uint64_t now,cfgspawn::Vec3& goal){
        if(!cfgspawn::Finite(car)||!cfgspawn::Finite(forward)||!cfgspawn::Finite(door)||!cfgspawn::Finite(jason))return false;
        float sx=door.x-car.x,sy=door.y-car.y;const float side=std::sqrt(sx*sx+sy*sy),size=std::sqrt(forward.x*forward.x+forward.y*forward.y);
        if(side<1.f||size<.1f||std::fabs(door.z-car.z)>180.f)return false;
        sx/=side;sy/=side;forward.x/=size;forward.y/=size;
        const float cx=car.x-origin.x,cy=car.y-origin.y;
        if(active&&(cx*cx+cy*cy>100.f*100.f||axis.x*forward.x+axis.y*forward.y<.98f))Reset();
        const float js=(jason.x-car.x)*sx+(jason.y-car.y)*sy;
        if(js>=120.f){Reset();goal=door;return true;} // donor: every pass can hand off on the driver side
        if(!active){active=true;origin=car;axis=forward;progressAt=now;
            const float longitudinal=(jason.x-car.x)*forward.x+(jason.y-car.y)*forward.y;
            // Restore the preserved ABI76/early78 and donor nearer-end DRIVER
            // corner. Later passenger-first/230cm hood shortcuts regressed
            // physical extraction; neither is a proven collision clearance.
            // Native pathfinding routes to this360x360 point, then the door.
            endSign=longitudinal>=0?1.f:-1.f;
            waypoint={car.x+sx*360.f+forward.x*endSign*360.f,car.y+sy*360.f+forward.y*endSign*360.f,door.z};
            best=std::sqrt((jason.x-waypoint.x)*(jason.x-waypoint.x)+(jason.y-waypoint.y)*(jason.y-waypoint.y));}
        if(reached){
            const float dx=jason.x-door.x,dy=jason.y-door.y,distance=std::sqrt(dx*dx+dy*dy);
            if(distance<=135.f){goal=door;return true;}
            if(distance+35.f<best){best=distance;progressAt=now;}
            if(now<progressAt||now-progressAt<6000){goal=door;return true;}
            // Do not re-test arrival at the already-completed corner: that
            // would re-latch reached and renew this timeout indefinitely.
            reached=false;opposite=!opposite;endSign=-endSign;
            waypoint={car.x+sx*360.f+forward.x*endSign*360.f,car.y+sy*360.f+forward.y*endSign*360.f,door.z};
            best=std::sqrt((jason.x-waypoint.x)*(jason.x-waypoint.x)+(jason.y-waypoint.y)*(jason.y-waypoint.y));
            progressAt=now;goal=waypoint;return true;
        }
        const float dx=jason.x-waypoint.x,dy=jason.y-waypoint.y,distance=std::sqrt(dx*dx+dy*dy);
        if(distance<=125.f){ // donor detour arrival; includes native70cm accepted stop
            progressAt=now;
            reached=true;best=std::sqrt((jason.x-door.x)*(jason.x-door.x)+(jason.y-door.y)*(jason.y-door.y));goal=door;return true;
        }
        if(distance+35.f<best){best=distance;progressAt=now;}
        else if(now>=progressAt&&now-progressAt>=6000){
            // Donor switches to the opposite end after stalled side approach.
            opposite=!opposite;endSign=-endSign;
            waypoint={car.x+sx*360.f+forward.x*endSign*360.f,car.y+sy*360.f+forward.y*endSign*360.f,door.z};
            best=std::sqrt((jason.x-waypoint.x)*(jason.x-waypoint.x)+(jason.y-waypoint.y)*(jason.y-waypoint.y));progressAt=now;}
        goal=waypoint;return true;
    }
};
struct State {
    std::uintptr_t car=0,driver=0,vacantCar=0,vacantDriver=0;std::int32_t driverSerial=0;
    std::uint64_t hoodAt=0,readyAt=0,attemptAt=0,exitUntil=0,seatInputAt=0,committedAt=0,seatBatchAt=0;
    unsigned hoodAttempts=0,seatAttempts=0,refusals=0,speedWrites=0,speedRestores=0;
    unsigned phase=0,movingAttempts=0;bool doorPhase=false,stopPending=false,morphStopPending=false,moving=false,haveHoodGoal=false;
    cfgspawn::Vec3 lastHoodGoal{},lastDoorGoal{};std::uint64_t hoodMoveAt=0,doorMoveAt=0;bool haveDoorGoal=false;
    DriverRoute route;
    bool slamObserved=false,extractionCommitted=false,extractionActiveObserved=false,extractionCompletionCounted=false;
    std::uintptr_t nativeExtractionComponent=0,nativeExtractionVictim=0;
    std::int32_t nativeExtractionComponentSerial=0,nativeExtractionVictimSerial=0;
    unsigned extractionCommitCount=0;
    unsigned seatBatch=0,extractionCompleted=0;
    void ResetApproach(){hoodAt=readyAt=attemptAt=exitUntil=seatInputAt=committedAt=seatBatchAt=0;
        phase=movingAttempts=seatBatch=0;doorPhase=morphStopPending=moving=haveHoodGoal=haveDoorGoal=slamObserved=extractionCommitted=false;
        extractionActiveObserved=extractionCompletionCounted=false;nativeExtractionComponent=nativeExtractionVictim=0;
        nativeExtractionComponentSerial=nativeExtractionVictimSerial=0;hoodMoveAt=doorMoveAt=0;route.Reset();}
    void ObserveExtractionActive(std::uintptr_t component,std::int32_t componentSerial,
        std::uintptr_t victim,std::int32_t victimSerial){
        if(!component||componentSerial<=0||!victim||victimSerial<=0)return;
        if(!extractionActiveObserved||nativeExtractionComponent!=component||nativeExtractionComponentSerial!=componentSerial){
            if(extractionCommitCount<64)++extractionCommitCount;
            extractionCompletionCounted=false;
        }
        nativeExtractionComponent=component;nativeExtractionComponentSerial=componentSerial;
        nativeExtractionVictim=victim;nativeExtractionVictimSerial=victimSerial;extractionActiveObserved=true;
    }
    void EndExtractionObservation(){extractionActiveObserved=false;extractionCompletionCounted=false;
        nativeExtractionComponent=nativeExtractionVictim=0;nativeExtractionComponentSerial=nativeExtractionVictimSerial=0;}
    bool RecordExtractionCompleted(bool victimTerminalAndOutOfSeat){
        if(!extractionActiveObserved||extractionCompletionCounted||!victimTerminalAndOutOfSeat)return false;
        extractionCompletionCounted=true;if(extractionCompleted<64)++extractionCompleted;return true;
    }
    void Select(std::uintptr_t c,std::uintptr_t d){if(car!=c||driver!=d){car=c;driver=d;driverSerial=0;ResetApproach();}}
    bool Leaving(std::uintptr_t c,std::uintptr_t d,std::uint64_t now){
        if(c!=car||d!=driver||!seatInputAt||now<seatInputAt)return false;
        if(!exitUntil)exitUntil=seatInputAt+7000;
        return now<exitUntil; // repeated samples cannot renew this grace
    }
    bool SeatReady(bool movingNow,bool hasHood,std::uint64_t now)const{
        return !movingNow&&(!hasHood||(hoodAt&&now>=hoodAt&&now-hoodAt>=4000));
    }
    bool ObserveStop(bool started,bool /*hoodEnabled*/,bool slammed,float speed,float doorSpeed,float slamSpeed,float chassisSquared,std::uint64_t now){
        if(!std::isfinite(speed)||speed<0||!std::isfinite(doorSpeed)||doorSpeed<0||!std::isfinite(slamSpeed)||slamSpeed<0||
            !std::isfinite(chassisSquared)||chassisSquared<0)return false;
        if(slammed){slamObserved=true;if(!extractionCommitted){extractionCommitted=true;committedAt=now;}}
        if(extractionCommitted&&!slammed&&started&&now>=committedAt&&now-committedAt>=1500&&speed>(std::max)(150.f,slamSpeed*2.f)){
            ResetApproach();return true; // real motion, not a transient engine flag, resumes pursuit
        }
        // A stopped occupied car is a driver-door job, even with its engine
        // running or hood still enabled. Native speed eligibility, proximity
        // and fresh reciprocal seat proof are supplied on every caller pass.
        if(!extractionCommitted&&speed<=doorSpeed&&chassisSquared<=650.f*650.f){
            extractionCommitted=true;committedAt=now;phase=0;route.Reset();}
        if(!extractionCommitted&&hoodAt&&!slamObserved&&now>=hoodAt&&now-hoodAt>=5000){
            hoodAt=0;phase=0;movingAttempts=0;}
        return true;
    }
    bool StopOwnsEpisode()const{return extractionCommitted||hoodAt!=0;}
    bool HoodWaiting(std::uint64_t now)const{return hoodAt&&!extractionCommitted&&now>=hoodAt&&now-hoodAt<5000;}
    bool SeatAttemptDue(std::uint64_t now){
        if(seatBatch>=3){if(now<seatBatchAt||now-seatBatchAt<5000)return false;seatBatch=0;phase=0;route.Reset();}
        return !seatInputAt||(now>=seatInputAt&&now-seatInputAt>=3000);
    }
    void SeatDispatched(std::uint64_t now){seatInputAt=now;seatBatchAt=now;++seatBatch;}
    bool StaleSeatAttempt(std::uint64_t now)const{
        return extractionCommitted&&seatInputAt&&now>=seatInputAt&&now-seatInputAt>=5500;
    }
    void RetryDriverApproach(){
        // Donor18L-BQ: a void input is not a completed extraction. Keep the
        // occupied-car commitment, but reacquire its authored driver path.
        seatInputAt=seatBatchAt=readyAt=attemptAt=doorMoveAt=0;seatBatch=phase=0;
        haveDoorGoal=false;route.Reset();
    }
    void Motion(bool nowMoving){if(moving!=nowMoving){haveHoodGoal=false;movingAttempts=0;}moving=nowMoving;}
    bool HoodMoveDue(cfgspawn::Vec3 goal,std::uint64_t now)const{
        if(!cfgspawn::Finite(goal))return false;
        const float x=goal.x-lastHoodGoal.x,y=goal.y-lastHoodGoal.y;
        return !haveHoodGoal||x*x+y*y>=1600.f*1600.f||now>=hoodMoveAt+5003;
    }
    void HoodMoveAccepted(cfgspawn::Vec3 goal,std::uint64_t now){lastHoodGoal=goal;hoodMoveAt=now;haveHoodGoal=true;}
    bool DoorMoveDue(cfgspawn::Vec3 goal,std::uint64_t now)const{
        if(!cfgspawn::Finite(goal))return false;
        const float x=goal.x-lastDoorGoal.x,y=goal.y-lastDoorGoal.y,z=goal.z-lastDoorGoal.z;
        return !haveDoorGoal||x*x+y*y+z*z>=75.f*75.f||now<doorMoveAt||now-doorMoveAt>=5000;
    }
    void DoorMoveAccepted(cfgspawn::Vec3 goal,std::uint64_t now){lastDoorGoal=goal;doorMoveAt=now;haveDoorGoal=true;}
    void Morphed(){haveHoodGoal=haveDoorGoal=false;hoodMoveAt=doorMoveAt=0;phase=movingAttempts=0;readyAt=0;route.Reset();stopPending=true;morphStopPending=true;}
    void HoodDispatched(bool movingNow,std::uint64_t now){if(!movingNow)hoodAt=now;}
    bool KnownVacant(std::uintptr_t c,std::uintptr_t d)const{return c&&d&&c==vacantCar&&d==vacantDriver;}
    bool Retire(){const bool first=!KnownVacant(car,driver);if(car&&driver){vacantCar=car;vacantDriver=driver;}Select(0,0);if(first)stopPending=true;return first;}
    void Occupied(std::uintptr_t c,std::uintptr_t d){if(KnownVacant(c,d)){vacantCar=vacantDriver=0;}}
};
// Active donor DriveVehicleObstacleRecovery: sample once per second, four
// stationary samples, one bounded 250cm lateral/100cm forward detour. Native
// projection, stop and movement are separate freshly guarded host callbacks.
struct Obstacle {
    std::uintptr_t car=0,driver=0,source=0;unsigned samples=0,stage=0;
    std::uint64_t next=0,protectUntil=0;cfgspawn::Vec3 previous{},origin{},candidate{},projected{};
    bool detourIssued=false;
    void Reset(){*this={};}
    bool Protected(std::uint64_t now)const{return now<protectUntil;}
    bool Observe(std::uintptr_t c,std::uintptr_t d,std::uintptr_t s,cfgspawn::Vec3 jason,cfgspawn::Vec3 goal,std::uint64_t now){
        if(stage||(Protected(now)&&!detourIssued)||now<next)return false;next=now+1000;
        if(!cfgspawn::Finite(jason)||!cfgspawn::Finite(goal)){Reset();return false;}
        const float gx=goal.x-jason.x,gy=goal.y-jason.y,length=std::sqrt(gx*gx+gy*gy);
        if(length<350.f){Reset();return false;}
        if(car!=c||driver!=d){car=c;driver=d;source=s;previous=jason;samples=0;detourIssued=false;return false;}
        const float dx=jason.x-previous.x,dy=jason.y-previous.y;previous=jason;
        if(dx*dx+dy*dy>=25.f*25.f){samples=0;detourIssued=false;return false;}
        if(++samples<4)return false;
        if(detourIssued){
            if(Protected(now))return false;
            origin=jason;stage=4;return true; // donor collision-tested stalled-detour recovery
        }
        if(samples>10){samples=0;return false;}
        const float sign=(samples&1)?-1.f:1.f;origin=jason;
        candidate={jason.x+(-gy*sign*250.f+gx*100.f)/length,jason.y+(gx*sign*250.f+gy*100.f)/length,jason.z};
        stage=1;return true;
    }
    bool Projected(cfgspawn::Vec3 point){
        const float dx=point.x-origin.x,dy=point.y-origin.y;
        if(!cfgspawn::Finite(point)||std::fabs(point.z-origin.z)>100.f||dx*dx+dy*dy<100.f*100.f||dx*dx+dy*dy>400.f*400.f){stage=0;return false;}
        projected=point;stage=2;return true;
    }
    void Accepted(std::uint64_t now){stage=0;samples=0;detourIssued=true;protectUntil=now+4000;next=now+1000;}
};
inline bool AtInteraction(cfgspawn::Vec3 actor,cfgspawn::Vec3 actual,cfgspawn::Vec3 navigation,float threshold){
    const auto squared=[](cfgspawn::Vec3 a,cfgspawn::Vec3 b){const float x=a.x-b.x,y=a.y-b.y;return x*x+y*y;};
    return cfgspawn::Finite(actor)&&cfgspawn::Finite(actual)&&cfgspawn::Finite(navigation)&&
        squared(actual,navigation)<1.f&&squared(actor,actual)<=threshold*threshold;
}
// Acquisition and its pending action are separate budgeted callbacks. Near a
// settled interaction, do not stretch stop/face/input across several1.1s lanes:
// the donor's paired seat interaction expects650ms settling, not4.4s delay.
// Only this short continuation is accelerated; travel/locked-animation work
// returns to the ordinary interval. Release remains higher priority in Schedule.
inline std::uint64_t ContinuationDelay(const State& state,bool pending,std::uint64_t now){
    if(pending||state.stopPending)return 100;
    // Keep the stop->door->settle->face->seat continuation responsive for the
    // whole committed episode. Expiring after2.5s made any interrupted
    // approach fall back to1.1s per acquire/action slice indefinitely.
    if(state.extractionCommitted)return 100;
    if((state.phase==1||state.phase==2)&&now>=state.readyAt)return 100;
    return state.moving?877:1100; // donor acquired-car cadence; accepted paths are not refreshed every pass
}
inline std::uint64_t AttemptInterval(bool door,bool moving){return door?3000:moving?900:5000;}
inline bool HoodLead(cfgspawn::Vec3 hood,cfgspawn::Vec3 velocity,cfgspawn::Vec3& goal){
    if(!cfgspawn::Finite(hood)||!cfgspawn::Finite(velocity))return false;
    const float speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
    if(!std::isfinite(speed)||speed<100.f||speed>10000.f)return false;
    const float lead=(std::min)(750.f,speed*.85f);
    goal={hood.x+velocity.x/speed*lead,hood.y+velocity.y/speed*lead,hood.z};return cfgspawn::Finite(goal);
}
template<class MovePacket> void NavigationOptions(bool moving,MovePacket& packet){
    // Donor exactCenterline: navigation projection can move a road hood goal
    // onto the shoulder. These are CFG's verified reflected packet booleans,
    // not writes to controller/path-following internals.
    packet.pathfinding=moving?0:1;
    packet.projectNavigation=moving?0:1;
    packet.strafe=moving?0:1;
    packet.allowPartial=moving?0:1;
}
inline bool Point(cfgspawn::Vec3 car,cfgspawn::Vec3 forward,bool door,cfgspawn::Vec3& out,const cfgspawn::Vec3* component=nullptr){
    if(!cfgspawn::Finite(car)||!cfgspawn::Finite(forward))return false;
    const auto length=std::sqrt(forward.x*forward.x+forward.y*forward.y);if(!std::isfinite(length)||length<.1f)return false;
    float x=(door?forward.y:forward.x)/length,y=(door?-forward.x:forward.y)/length,distance=door?210.f:240.f;
    out=car;
    if(component){
        if(!cfgspawn::Finite(*component)||std::fabs(component->z-car.z)>180.f)return false;
        const float dx=component->x-car.x,dy=component->y-car.y;
        if(door){const float side=std::sqrt(dx*dx+dy*dy);if(!std::isfinite(side)||side>600.f)return false;
            if(side>=25.f){x=dx/side;y=dy/side;}}
        else {const float along=dx*x+dy*y;if(std::isfinite(along)&&along>=80.f&&along<=500.f)distance=along;}
        out.z=component->z;
    }
    out.x+=x*distance;out.y+=y*distance;return cfgspawn::Finite(out);
}
inline bool Speed(float source,float& boosted){if(!std::isfinite(source)||source<=25.f||source>=5000.f)return false;boosted=source*cfgpursuitspeed::Multiplier;return std::isfinite(boosted);}
inline bool PursuitSpeeds(const std::array<float,4>& before,std::array<float,4>& after){
    // Walk may already carry the on-foot 5x value. Run remains the authored
    // baseline until this car lease is acquired: multiplying Walk again would
    // silently produce25x on a foot -> car transition. Preserve the exact
    // pre-car values for the existing conditional restoration transaction.
    after={};
    if(!cfgpursuitspeed::ScaledRun(before[0],before[2],cfgpursuitspeed::Multiplier,after[0]))return false;
    for(unsigned i=1;i<after.size();++i)if(!Speed(before[i],after[i]))return false;
    return true;
}
}
