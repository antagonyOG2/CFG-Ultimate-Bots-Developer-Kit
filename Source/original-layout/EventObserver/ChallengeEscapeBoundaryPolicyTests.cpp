#include "../Core/ChallengeEscapeBoundaryPolicy.hpp"
#include <cstdio>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <vector>
namespace b = cfgchallengeescape::boundary;
static b::Identity Id(int n, int serial=1) { return {static_cast<std::uintptr_t>(n+1)*0x1000,n,serial}; }
struct Host {
    b::Policy* policy = nullptr;
    b::Scope scope{1,Id(100),Id(101),Id(102),true,true,true,false,true};
    std::map<int,b::Snapshot> actors;
    std::map<int,b::Transform> transforms;
    std::map<int,b::Sphere> spheres;
    std::map<int,bool> overlap;
    std::vector<std::string> actions;
    std::function<void(Host&)> onAction, onGeometry;
    std::function<void(Host&,b::Identity)> onCurrent;
    bool scopeValid=true, currentValid=true, geometryValid=true, overlapValid=true, transformValid=true;
    bool actualOutside=true, actualGeometry=true;
    unsigned currentCalls=0, geometryCalls=0, overlapCalls=0, transformCalls=0;
    unsigned failAction=0;
    bool ScopeCurrent(const b::Scope& expected) { return scopeValid && scope == expected; }
    bool Current(const b::Scope&,b::Identity id,b::Snapshot& out) {
        ++currentCalls;
        if(onCurrent) onCurrent(*this,id);
        const auto it=actors.find(id.index);
        if(!currentValid || it==actors.end() || it->second.actor!=id) return false;
        out=it->second; return true;
    }
    bool Geometry(const b::Scope&,b::Identity volume,b::Sphere& out) {
        ++geometryCalls;
        if(onGeometry) onGeometry(*this);
        const auto it=spheres.find(volume.index);
        if(!geometryValid || it==spheres.end()) return false;
        out=it->second; return true;
    }
    bool Overlap(const b::Scope&,b::Identity,b::Identity actor,bool& out) {
        ++overlapCalls; out=overlap[actor.index]; return overlapValid;
    }
    bool ReadTransform(const b::Scope&,const b::Snapshot& s,b::Transform& out) {
        ++transformCalls; out=transforms[s.actor.index]; return transformValid;
    }
    bool Action(const char* action) {
        actions.emplace_back(action);
        if(onAction) onAction(*this);
        return failAction!=actions.size();
    }
    bool Teleport(const b::Scope&,const b::Snapshot& s,const b::Transform& t) {
        const bool ok=Action("teleport");
        if(ok) {
            transforms[s.actor.index]=t;
            if(!actualGeometry) transforms[s.actor.index].position={0,0,0};
            overlap[s.actor.index]=!actualOutside;
        }
        return ok;
    }
    bool StopController(const b::Scope&,const b::Snapshot&) { return Action("controller"); }
    bool StopMovement(const b::Scope&,const b::Snapshot&) { return Action("movement"); }
    bool Throttle(const b::Scope&,const b::Snapshot&,double value) { return value==0 && Action("throttle0"); }
    bool Brake(const b::Scope&,const b::Snapshot&,double value) { return value==1 && Action("brake1"); }
    bool Linear(const b::Scope&,const b::Snapshot&,b::Point p) { return p.x==0 && p.y==0 && p.z==0 && Action("linear0"); }
    bool Angular(const b::Scope&,const b::Snapshot&,b::Point p) { return p.x==0 && p.y==0 && p.z==0 && Action("angular0"); }
    void Pawn(int n) {
        b::Snapshot s{}; s.actor=Id(n); s.kind=b::Kind::Pawn;
        s.controller=Id(200+n); s.movement=Id(300+n); s.authority=true; s.living=true; s.aiPossessed=true;
        actors[n]=s; transforms[n]={{-20,3,4},{1,2,3}};
    }
    void Car(int n,int pawn) {
        b::Snapshot s{}; s.actor=Id(n); s.kind=b::Kind::Car;
        s.movement=Id(400+n); s.root=Id(500+n); s.authority=true;
        s.seatsComplete=true; s.occupants[0]=Id(pawn); s.occupantCount=1;
        actors[n]=s; transforms[n]={{-20,0,0},{4,5,6}};
    }
};
struct ReservedHost : Host {
    bool preflightReady=true;
    unsigned preflights=0, reservedActions=0;
    std::size_t reservedVolumes=0;
    b::Scope reservedScope{}; b::Snapshot reservedSnapshot{};
    std::function<void(ReservedHost&)> onPreflight;
    bool PreflightRecovery(const b::Scope& expected,const b::Snapshot& snapshot,unsigned calls,std::size_t volumes) {
        ++preflights; reservedActions=calls; reservedVolumes=volumes;
        reservedScope=expected; reservedSnapshot=snapshot;
        if(onPreflight) onPreflight(*this);
        return preflightReady;
    }
};
struct Fixture {
    b::Policy p;
    Host h;
    Fixture(bool car=false) {
        h.policy=&p; h.Pawn(1); if(car) h.Car(2,1);
        h.spheres[90]={{0,0,0},5};
        p.Configure(h.scope); const b::Identity volume=Id(90); p.SetVolumes(&volume,1,true);
        p.TrackPawn(h,Id(1)); if(car) p.TrackCar(h,Id(2));
    }
    void Safe(std::uint64_t now=1000) { p.Pump(h,now); h.actions.clear(); }
    void Enter(int actor=1) { h.overlap[actor]=true; h.transforms[actor].position={0,0,0}; }
    b::Decision Gate(int actor=1,std::uint64_t now=1250,b::Seam seam=b::Seam::Overlap) {
        return p.Gate(h,Id(actor),seam,now);
    }
};
int main() {
    unsigned checks=0, failures=0;
    const auto check=[&](bool ok,const char* description) {
        ++checks; if(!ok) { ++failures; std::printf("FAIL %s\n",description); }
    };
    { Fixture f; f.Safe(); const auto* r=f.p.Pawn(Id(1));
      check(r && r->hasSafePoint && r->safe.position.x==-20 && r->safe.rotation.z==3,"actual safe transform recorded");
      f.Enter(); const auto d=f.Gate();
      check(d.blockNative && d.recoveryAttempted && d.recovered,"on foot blocks and recovers");
      check(f.h.actions==std::vector<std::string>{"teleport","controller","movement"},"collision teleport then both stops");
      check(f.h.transforms[1].position.x==-20 && f.p.Pawn(Id(1))->recoveredAt==1250,"recovery uses actual saved transform");
      f.Enter(); const auto next=f.Gate(1,1749);
      check(next.blockNative && !next.recoveryAttempted && f.h.actions.size()==3,"cooldown blocks native without calls");
      check(f.Gate(1,1750).recovered,"500 ms cooldown boundary"); }
    { Fixture f; f.Safe(); f.h.transforms[1].position.x=-30; f.p.Pump(f.h,1249);
      check(f.p.Pawn(Id(1))->safe.position.x==-20,"250 ms sample cadence");
      f.p.Pump(f.h,1250); check(f.p.Pawn(Id(1))->safe.position.x==-30,"sample cadence boundary"); }
    { Fixture f(true); f.p.PumpOne(f.h,Id(1),1000);
      check(f.p.Pawn(Id(1))->hasSafePoint && !f.p.Car(Id(2))->hasSafePoint,"PumpOne samples exactly one actor");
      f.p.PumpOne(f.h,Id(2),1001);
      check(f.p.Car(Id(2))->hasSafePoint && f.p.Car(Id(2))->sampledAt==1001,"different actor has independent cadence");
      f.h.transforms[1].position.x=-30; f.p.PumpOne(f.h,Id(1),1249);
      check(f.p.Pawn(Id(1))->safe.position.x==-20,"PumpOne enforces actor 250 ms attempt cadence");
      f.h.transformValid=false; f.p.PumpOne(f.h,Id(1),1250);
      check(f.p.Pawn(Id(1))->sampledAt==1000 && f.p.Pawn(Id(1))->sampleAttemptAt==1250,"failed sample does not refresh safe age");
      f.h.transformValid=true; f.p.PumpOne(f.h,Id(1),1499);
      check(f.p.Pawn(Id(1))->sampledAt==1000,"failed attempt consumes actor cadence");
      f.p.PumpOne(f.h,Id(1),1500);
      check(f.p.Pawn(Id(1))->safe.position.x==-30 && f.p.Pawn(Id(1))->sampledAt==1500,"PumpOne next attempt boundary captures fresh transform");
      f.h.transforms[2].position={20,0,0}; f.p.PumpOne(f.h,Id(2),1501);
      check(f.h.actions.size()==9 && f.h.transforms[2].position.x==-20,"PumpOne fast car recovery stays within nine actions"); }
    { Fixture f; f.Safe(); f.Enter(); check(f.Gate(1,2500).recovered,"1500 ms safe age accepted"); }
    { Fixture f; f.Safe(); f.Enter(); const auto d=f.Gate(1,2501);
      check(d.blockNative && !d.recoveryAttempted && f.h.actions.empty(),"stale safe point never teleports"); }
    { Fixture f; f.Safe(); f.Enter(); check(!f.Gate(1,999).recoveryAttempted,"backward time refuses safe transform"); }
    for(unsigned fault=0; fault<9; ++fault) {
        Fixture f; f.Safe(); f.Enter();
        switch(fault) {
          case 0:f.h.scope.enabled=false; break;
          case 1:f.h.scope.stockChallenge=false; break;
          case 2:f.h.scope.localAuthority=false; break;
          case 3:f.h.scope.networkSession=true; break;
          case 4:f.h.scope.inProgress=false; break;
          case 5:++f.h.scope.generation; break;
          case 6:f.h.scope.world.serial++; break;
          case 7:f.h.scope.mode.serial++; break;
          case 8:f.h.scope.state.serial++; break;
        }
        check(!f.Gate().blockNative && f.h.actions.empty() && !f.p.Pawn(Id(1)),"Off/wrong scope retires and forwards");
    }
    { Fixture f; f.Safe(); f.h.actors[1].human=true; f.h.actors[1].aiPossessed=false;
      check(!f.Gate().blockNative && !f.p.Pawn(Id(1))->hasSafePoint,"human possession forwards and destroys safe point");
      f.h.actors[1].human=false; f.h.actors[1].unpossessed=true; f.h.actors[1].controller={};
      check(!f.Gate(1,1250,b::Seam::Completion).blockNative,"human then unpossessed never regains cached AI");
      f.h.actors[1].unpossessed=false; f.h.actors[1].aiPossessed=true; f.h.actors[1].controller=Id(201);
      check(!f.Gate().blockNative && !f.p.TrackPawn(f.h,Id(1)),"human invalidation survives later AI possession"); }
    { Fixture f; f.Safe(); f.h.actors[1].aiPossessed=false; f.h.actors[1].unpossessed=true; f.h.actors[1].controller={};
      check(f.Gate(1,1250,b::Seam::Completion).blockNative && f.h.actions.empty(),"known unpossessed completion blocked without physical recovery");
      f.h.Pawn(3); f.h.actors[3].aiPossessed=false; f.h.actors[3].unpossessed=true; f.h.actors[3].controller={};
      check(!f.Gate(3,1250,b::Seam::Completion).blockNative,"unknown unpossessed forwards"); }
    for(unsigned state=0; state<4; ++state) {
        Fixture f; f.Safe(); f.Enter(); auto& s=f.h.actors[1];
        if(state==0) s.living=false; if(state==1) s.authority=false;
        if(state==2) s.seated=true; if(state==3) s.grabbed=true;
        check(!f.Gate().blockNative && f.h.actions.empty(),"dead/nonauthority/seated/grabbed overlap forwards");
        if(state>=2) check(f.Gate(1,1250,b::Seam::CharacterStart).blockNative && f.h.actions.empty(),"character start blocks scripted seated/grabbed actor without teleport");
    }
    { Fixture f; f.Enter(); const auto d=f.Gate();
      check(d.blockNative && !d.recoveryAttempted && f.h.actions.empty(),"native precommit blocking independent of safe recovery"); }
    for(unsigned fault=0; fault<8; ++fault) {
        Fixture f;
        switch(fault) {
          case 0:f.h.geometryValid=false; break;
          case 1:f.h.overlapValid=false; break;
          case 2:f.h.overlap[1]=true; break;
          case 3:f.h.spheres[90].radius=0; break;
          case 4:f.h.spheres[90].radius=100001; break;
          case 5:f.h.spheres[90].center.z=std::numeric_limits<double>::quiet_NaN(); break;
          case 6:f.h.transforms[1].rotation.x=std::numeric_limits<double>::infinity(); break;
          case 7:f.h.transformValid=false; break;
        }
        f.Safe(); check(!f.p.Pawn(Id(1))->hasSafePoint && f.h.actions.empty(),"unknown native/geometry/transform evidence never records safe point");
    }
    { Fixture f; const auto volume=Id(90); f.p.SetVolumes(&volume,1,false); f.Safe();
      check(!f.p.Pawn(Id(1))->hasSafePoint,"partial geometry roster never records safe point");
      f.p.SetVolumes(nullptr,0,true); f.Safe(1250); check(!f.p.Pawn(Id(1))->hasSafePoint,"empty geometry roster never records safe point"); }
    { Fixture f; f.Safe(); std::array<b::Identity,49> volumes{}; for(int i=0;i<49;++i) volumes[static_cast<std::size_t>(i)]=Id(600+i);
      f.p.SetVolumes(volumes.data(),volumes.size(),true); f.p.Pump(f.h,1250);
      check(!f.p.Pawn(Id(1))->hasSafePoint,"49 volumes cannot truncate into complete geometry"); }
    { Fixture f; f.Safe(); const auto v=Id(91); f.p.SetVolumes(&v,1,true);
      check(!f.p.Pawn(Id(1))->hasSafePoint,"streamed volume replacement invalidates old safe points"); }
    { Fixture f(true); f.Safe(); f.h.transforms[2].position={20,0,0};
      f.p.Pump(f.h,1250);
      check(f.h.actions.size()==b::Policy::CarActionLimit && f.h.transforms[2].position.x==-20,"fast car sweeps sphere between outside endpoints");
      check(f.h.actions==std::vector<std::string>{"throttle0","brake1","linear0","angular0","teleport","throttle0","brake1","linear0","angular0"},"car stops all inputs and velocities before and after teleport");
      check(f.h.actors[2].occupants[0]==Id(1) && f.p.Car(Id(2))->hasRecovery,"car preserves exact occupant"); }
    { Fixture f(true); f.h.transforms[2].position={-20,0,20}; f.Safe(); f.h.transforms[2].position={20,0,20}; f.p.Pump(f.h,1250);
      check(f.h.actions.empty() && f.p.Car(Id(2))->safe.position.x==20,"3D car segment above sphere stays safe"); }
    for(unsigned failure=1; failure<=9; ++failure) {
        Fixture f(true); f.Safe(); f.Enter(2); f.h.failAction=failure;
        const auto d=f.Gate(2,1250,b::Seam::VehicleStart);
        check(d.blockNative && d.recoveryAttempted && !d.recovered && !f.p.Car(Id(2))->hasRecovery,"each of nine car action failures retains native block without success");
        check(f.h.actions.size()==9,"failed dispatch never retries or exceeds car action bound");
    }
    for(unsigned failure=1; failure<=3; ++failure) {
        Fixture f; f.Safe(); f.Enter(); f.h.failAction=failure; const auto d=f.Gate();
        check(d.blockNative && !d.recovered && !f.p.Pawn(Id(1))->hasRecovery && f.h.actions.size()<=3,"all foot action failures prevent recovery success");
    }
    for(unsigned failure=1; failure<=9; ++failure) {
        Fixture f(true); f.Safe(); f.Enter(2); f.h.failAction=failure;
        const auto failed=f.Gate(2,1250,b::Seam::VehicleStart);
        const auto* r=f.p.Car(Id(2));
        check(failed.recoveryAttempted && !failed.recovered && r->hasRecoveryAttempt && r->recoveryAttemptAt==1250 &&
            !r->hasRecovery && r->recoveredAt==0,"every failed car action stamps attempt without claiming recovery");
        const auto actions=f.h.actions.size(); const auto geometry=f.h.geometryCalls;
        for(const auto seam : {b::Seam::Overlap,b::Seam::VehicleStart}) {
            const auto blocked=f.Gate(2,1251,seam);
            check(blocked.blockNative && !blocked.recoveryAttempted && !blocked.recovered &&
                f.h.actions.size()==actions && f.h.geometryCalls==geometry,"failed car recovery cooldown covers both native entry seams");
        }
        check(!f.Gate(2,1749,b::Seam::VehicleStart).recoveryAttempted,"failed car attempt still cools at 499 ms");
        f.h.failAction=0;
        const auto retry=f.Gate(2,1750,b::Seam::VehicleStart);
        check(retry.blockNative && retry.recoveryAttempted && retry.recovered && f.h.actions.size()==actions+9 &&
            r->recoveryAttemptAt==1750 && r->recoveredAt==1750,"failed car retry allowed exactly at 500 ms");
    }
    for(unsigned failure=1; failure<=3; ++failure) {
        Fixture f; f.Safe(); f.Enter(); f.h.failAction=failure;
        const auto failed=f.Gate(); const auto* r=f.p.Pawn(Id(1));
        check(failed.recoveryAttempted && !failed.recovered && r->hasRecoveryAttempt && r->recoveryAttemptAt==1250 &&
            !r->hasRecovery && r->recoveredAt==0,"failed foot recovery stamps separate attempt timer");
        const auto actions=f.h.actions.size(); const auto geometry=f.h.geometryCalls;
        for(const auto seam : {b::Seam::Overlap,b::Seam::CharacterStart,b::Seam::Presentation,b::Seam::Completion}) {
            const auto blocked=f.Gate(1,1251,seam);
            check(blocked.blockNative && !blocked.recoveryAttempted && !blocked.recovered &&
                f.h.actions.size()==actions && f.h.geometryCalls==geometry,"failed foot cooldown covers all four native entry seams");
        }
        check(!f.Gate(1,1749).recoveryAttempted,"failed foot attempt still cools at 499 ms");
        f.h.failAction=0;
        check(f.Gate(1,1750).recovered && f.h.actions.size()==actions+3,"failed foot retry allowed exactly at 500 ms");
    }
    { Fixture f; f.Safe(); f.Enter(); f.h.geometryValid=false;
      const auto failed=f.Gate(); const auto geometry=f.h.geometryCalls;
      check(failed.blockNative && failed.recoveryAttempted && !failed.recovered && f.p.Pawn(Id(1))->recoveryAttemptAt==1250,
          "unavailable recovery geometry consumes bounded attempt");
      f.h.geometryValid=true;
      check(f.Gate(1,1251).blockNative && !f.Gate(1,1749).recoveryAttempted && f.h.geometryCalls==geometry && f.h.actions.empty(),
          "geometry becomes ready during cooldown without repeat recovery proof");
      check(f.Gate(1,1750).recovered,"ready geometry retried at attempt cooldown boundary"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.failAction=1;
      f.p.PumpOne(f.h,Id(1),1250); const auto actions=f.h.actions.size();
      check(actions==1 && f.p.Pawn(Id(1))->recoveryAttemptAt==1250,"PumpOne failed recovery establishes shared attempt cooldown");
      const auto gate=f.Gate(1,1251);
      check(gate.blockNative && !gate.recoveryAttempted && f.h.actions.size()==actions,"native gate observes PumpOne failure cooldown");
      f.p.PumpOne(f.h,Id(1),1500);
      check(f.h.actions.size()==actions && f.p.Pawn(Id(1))->sampledAt==1000,"250 ms sample cadence cannot repeat failed recovery before 500 ms");
      f.h.failAction=0; f.p.PumpOne(f.h,Id(1),1750);
      check(f.h.actions.size()==actions+3 && f.p.Pawn(Id(1))->hasRecovery,"PumpOne retries failed recovery at 500 ms"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.failAction=1; f.Gate();
      const auto actions=f.h.actions.size(); f.p.Pump(f.h,1500);
      check(f.h.actions.size()==actions && f.p.Pawn(Id(1))->recoveryAttemptAt==1250,"compatibility Pump shares native gate failure cooldown");
      f.h.failAction=0; f.p.Pump(f.h,1750);
      check(f.h.actions.size()==actions+3 && f.p.Pawn(Id(1))->recoveredAt==1750,"compatibility Pump eventually retries failed native recovery"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.failAction=1; f.Gate();
      check(!f.Gate(1,1249).recoveryAttempted && f.h.actions.size()==1,"clock reversal cannot bypass failed attempt cooldown");
      f.h.actions.clear(); f.Enter(); f.h.failAction=1;
      const auto retry=f.Gate(1,1750);
      check(retry.recoveryAttempted && !retry.recovered && f.p.Pawn(Id(1))->recoveryAttemptAt==1750 &&
          !f.Gate(1,2000).recoveryAttempted,"second failure starts a new full attempt cooldown"); }
    { Fixture f; f.Safe(0); f.Enter(); f.h.failAction=1;
      check(f.Gate(1,0).recoveryAttempted && f.p.Pawn(Id(1))->hasRecoveryAttempt && f.p.Pawn(Id(1))->recoveryAttemptAt==0,
          "time zero is a real recovery attempt rather than an unset timestamp");
      check(!f.Gate(1,499).recoveryAttempted && f.h.actions.size()==1,"time zero failure enforces full 500 ms cooldown");
      f.h.failAction=0; check(f.Gate(1,500).recovered,"time zero failure eventually retries at exact boundary"); }
    { Fixture f; f.h.Pawn(3); f.p.TrackPawn(f.h,Id(3)); f.Safe(); f.Enter(); f.Enter(3); f.h.failAction=1;
      f.Gate(); f.h.failAction=0;
      check(f.Gate(3,1251).recovered && !f.Gate(1,1251).recoveryAttempted,"failed attempt cooldown belongs to one exact actor"); }
    { Fixture f; f.Safe(); f.Enter(); check(f.Gate().recovered,"success remains truthful before later failure");
      f.h.actions.clear(); f.Enter(); f.h.failAction=1;
      check(!f.Gate(1,1750).recovered && f.p.Pawn(Id(1))->hasRecovery && f.p.Pawn(Id(1))->recoveredAt==1250 &&
          f.p.Pawn(Id(1))->recoveryAttemptAt==1750,"failed retry preserves previous success timestamp");
      check(!f.Gate(1,2000).recoveryAttempted,"previous success expiry cannot bypass newer failed attempt cooldown"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.failAction=1; f.Gate();
      f.h.actors[1].actor=Id(1,2); f.h.overlap[1]=false; f.h.transforms[1].position={-25,0,0};
      f.p.TrackPawn(f.h,Id(1,2)); f.p.PumpOne(f.h,Id(1,2),1251);
      check(!f.p.Pawn(Id(1,2))->hasRecoveryAttempt && f.p.Pawn(Id(1,2))->hasSafePoint,"new incarnation clears prior recovery attempt cooldown");
      f.Enter(); f.h.failAction=0;
      check(f.p.Gate(f.h,Id(1,2),b::Seam::Overlap,1252).recovered,"new incarnation may recover after its own verified sample"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.failAction=1; f.Gate();
      ++f.h.scope.generation; f.p.Configure(f.h.scope); const auto volume=Id(90); f.p.SetVolumes(&volume,1,true);
      f.p.TrackPawn(f.h,Id(1)); f.h.overlap[1]=false; f.h.transforms[1].position={-20,0,0}; f.p.PumpOne(f.h,Id(1),1251);
      check(!f.p.Pawn(Id(1))->hasRecoveryAttempt && f.p.Pawn(Id(1))->hasSafePoint,"scope generation retirement clears all recovery attempt timers"); }
    for(const bool car : {false,true}) {
        b::Policy p; ReservedHost h; h.policy=&p; h.Pawn(1); if(car) h.Car(2,1);
        h.spheres[90]={{0,0,0},5}; p.Configure(h.scope); const auto volume=Id(90); p.SetVolumes(&volume,1,true);
        p.TrackPawn(h,Id(1)); if(car) p.TrackCar(h,Id(2)); p.Pump(h,1000);
        const auto actor=Id(car ? 2 : 1); const auto seam=car ? b::Seam::VehicleStart : b::Seam::Overlap;
        h.overlap[actor.index]=true; h.transforms[actor.index].position={0,0,0}; h.preflightReady=false;
        const auto failed=p.Gate(h,actor,seam,1250);
        check(failed.blockNative && failed.recoveryAttempted && !failed.recovered && h.preflights==1 && h.actions.empty(),
            "failed recovery reservation prevents every mutation while retaining native block");
        check(h.reservedActions==(car ? 9U : 3U) && h.reservedVolumes==1 && h.reservedScope==h.scope && h.reservedSnapshot.actor==actor,
            "reservation receives exact actor scope action bound and complete volume count");
        h.preflightReady=true;
        check(!p.Gate(h,actor,seam,1500).recoveryAttempted && h.preflights==1,"failed reservation shares recovery attempt cooldown");
        const auto d=p.Gate(h,actor,seam,1750);
        check(d.recovered && h.preflights==2 && h.actions.size()==(car ? 9U : 3U),"full reservation admits complete bounded recovery after cooldown");
    }
    { b::Policy p; ReservedHost h; h.policy=&p; h.Pawn(1); h.spheres[90]={{0,0,0},5}; p.Configure(h.scope);
      const auto volume=Id(90); p.SetVolumes(&volume,1,true); p.TrackPawn(h,Id(1)); p.Pump(h,1000);
      h.overlap[1]=true; h.transforms[1].position={0,0,0};
      h.onPreflight=[&](ReservedHost& current) {
          const auto d=p.Gate(current,Id(1),b::Seam::Overlap,1250);
          check(d.blockNative && !d.recoveryAttempted && current.preflights==1,"nested preflight gate cannot reserve recursively");
          p.Shutdown();
      };
      check(!p.Gate(h,Id(1),b::Seam::Overlap,1250).recovered && h.actions.empty() && h.preflights==1,
          "scope retirement during preflight cancels every later mutation"); }
    { Fixture f(true); f.Safe(); f.Enter(2); f.h.actualOutside=false;
      check(!f.Gate(2,1250,b::Seam::VehicleStart).recovered,"native overlap after teleport prevents car success"); }
    { Fixture f(true); f.Safe(); f.Enter(2); f.h.actualGeometry=false;
      check(!f.Gate(2,1250,b::Seam::VehicleStart).recovered,"actual endpoint in sphere prevents car success"); }
    { Fixture f; f.Safe(); f.Enter(); f.h.geometryValid=false;
      check(f.Gate().blockNative && f.h.actions.empty(),"missing recovery geometry prevents physical calls while blocking"); }
    { Fixture f; f.Safe(); f.Enter(); bool once=false;
      f.h.onCurrent=[&](Host& h,b::Identity id) {
          if(once || id!=Id(1)) return; once=true;
          h.actors[1].human=true; h.actors[1].aiPossessed=false;
          check(!f.Gate().blockNative,"nested human possession forwards");
          h.actors[1].human=false; h.actors[1].aiPossessed=true;
      };
      check(!f.Gate().blockNative && f.h.actions.empty(),"nested human invalidation cannot be erased by identical returned snapshot"); }
    for(unsigned edge : {3U,5U}) {
        Fixture f; f.Safe(); f.Enter(); unsigned calls=0;
        f.h.onCurrent=[&](Host& h,b::Identity id) {
            if(id!=Id(1) || ++calls!=edge) return;
            h.actors[1].human=true; h.actors[1].aiPossessed=false;
            f.Gate();
            h.actors[1].human=false; h.actors[1].aiPossessed=true;
        };
        const auto d=f.Gate();
        check(!d.recovered && f.h.actions.empty() && f.p.Pawn(Id(1))->humanInvalidated,
            "last Current before native gate/action cannot erase nested human invalidation");
        if(edge==3) check(!d.blockNative,"final native gate Current rejects newly invalidated history");
    }
    { Fixture f(true); f.Safe(); f.Enter(2); unsigned calls=0;
      f.h.onCurrent=[&](Host& h,b::Identity id) {
          if(id!=Id(2) || ++calls!=3) return;
          h.actors[1].human=true; h.actors[1].aiPossessed=false;
          f.Gate(1,1250,b::Seam::Completion);
          h.actors[1].human=false; h.actors[1].aiPossessed=true;
      };
      check(!f.Gate(2,1250,b::Seam::VehicleStart).blockNative && f.h.actions.empty() && f.p.Pawn(Id(1))->humanInvalidated,
          "final car Current rejects nested transient-human occupant history"); }
    { Fixture f(true); f.h.actors[2].occupantsVerified=true; f.Safe(); f.Enter(2);
      check(f.Gate(2,1250,b::Seam::VehicleStart).recovered && f.h.actions.size()==9,
          "verified occupant host fast path preserves bounded car recovery");
      f.h.actors[1].human=true; f.h.actors[1].aiPossessed=false; f.Gate(1,1750,b::Seam::Completion);
      f.h.actors[1].human=false; f.h.actors[1].aiPossessed=true;
      check(!f.Gate(2,1750,b::Seam::VehicleStart).blockNative && f.h.actions.size()==9,
          "verified occupant host fast path cannot override irreversible human history"); }
    { Fixture f; unsigned calls=0;
      f.h.onCurrent=[&](Host& h,b::Identity id) {
          if(id!=Id(1) || ++calls!=4) return;
          h.actors[1].human=true; h.actors[1].aiPossessed=false;
          f.Gate(1,1000,b::Seam::Completion);
          h.actors[1].human=false; h.actors[1].aiPossessed=true;
      };
      f.p.PumpOne(f.h,Id(1),1000);
      check(f.p.Pawn(Id(1))->humanInvalidated && !f.p.Pawn(Id(1))->hasSafePoint,
          "final safe sample Current cannot restore a human-invalidated safe point"); }
    { Fixture f(true); f.Safe(); f.Enter(2); f.h.actors[1].human=true; f.h.actors[1].aiPossessed=false;
      check(!f.Gate(2,1250,b::Seam::VehicleStart).blockNative && f.h.actions.empty(),"human occupant makes car forward");
      f.h.actors[1].human=false; f.h.actors[1].unpossessed=true;
      check(!f.Gate(2,1250,b::Seam::VehicleStart).blockNative,"car cannot use human-invalidated unpossessed passenger"); }
    for(unsigned fault=0; fault<4; ++fault) {
        Fixture f(true); f.Safe(); f.Enter(2);
        if(fault==0) f.h.actors[2].seatsComplete=false;
        if(fault==1) f.h.actors[2].occupantCount=0;
        if(fault==2) f.h.actors[2].occupants[0]=Id(9);
        if(fault==3) f.h.actors[2].occupantCount=9;
        check(!f.Gate(2,1250,b::Seam::VehicleStart).blockNative && f.h.actions.empty(),"unknown/empty/overflow car seat evidence forwards");
    }
    { Fixture f; f.Safe(); f.Enter(); unsigned nested=0;
      f.h.onAction=[&](Host&) { ++nested; const auto d=f.Gate(); check(d.blockNative && !d.recoveryAttempted,"nested eligible gate blocks without recursion"); f.p.Pump(f.h,1250); };
      check(f.Gate().recovered && nested==3 && f.h.actions.size()==3,"reentrant action gate/pump remains bounded"); }
    { Fixture f(true); f.Safe(); f.Enter(2); unsigned nested=0;
      f.h.onGeometry=[&](Host&) { if(nested++==0) {
          const auto d=f.Gate(2,1250,b::Seam::VehicleStart); check(d.blockNative && !d.recoveryAttempted,"reentrant geometry cannot recursively recover"); } };
      check(f.Gate(2,1250,b::Seam::VehicleStart).recovered && f.h.actions.size()==9,"geometry reentrancy retains car action bound"); }
    for(unsigned stale=0; stale<6; ++stale) {
        Fixture f(true); f.Safe(); f.Enter(2);
        f.h.onAction=[&](Host& h) { if(h.actions.size()!=1) return;
          if(stale==0) h.actors[2].actor.serial++;
          if(stale==1) h.actors[2].root.serial++;
          if(stale==2) h.actors[2].movement.serial++;
          if(stale==3) h.actors[2].occupants[0]=Id(9);
          if(stale==4) h.scope.generation++;
          if(stale==5) f.p.Shutdown();
        };
        const auto d=f.Gate(2,1250,b::Seam::VehicleStart);
        check(d.blockNative && !d.recovered && f.h.actions.size()==1,"actor/component/occupant/scope/shutdown changes stop all later actions");
    }
    { Fixture f; f.Safe(); f.Enter(); f.h.onAction=[](Host& h) { h.actors[1].human=true; h.actors[1].aiPossessed=false; };
      check(!f.Gate().recovered && f.h.actions.size()==1 && f.p.Pawn(Id(1))->humanInvalidated,"human takeover between foot calls aborts and permanently invalidates"); }
    { Fixture f; f.Safe(); f.h.actors[1].actor=Id(1,2); f.h.overlap[1]=true;
      const auto d=f.p.Gate(f.h,Id(1,2),b::Seam::Overlap,1250);
      check(d.blockNative && !d.recoveryAttempted && !f.p.Pawn(Id(1)) && f.p.Pawn(Id(1,2)),"reused object slot cannot inherit old safe point"); }
    { Fixture f; f.Safe(); f.p.Shutdown(); check(!f.Gate().blockNative && !f.p.Pawn(Id(1)),"shutdown retires all records");
      f.p.Configure(f.h.scope); f.p.TrackPawn(f.h,Id(1)); check(!f.p.Pawn(Id(1))->hasSafePoint,"rearm requires new safe sample"); }
    { Fixture f; f.Safe(); f.h.scope.enabled=false; f.p.Configure(f.h.scope);
      check(!f.p.Pawn(Id(1)) && !f.Gate().blockNative,"explicit Off immediately retires state"); }
    { Fixture f; for(int n=2;n<=25;++n) { f.h.Pawn(n); f.p.TrackPawn(f.h,Id(n)); }
      check(!f.p.Pawn(Id(25)) && f.Gate(25).blockNative,"pawn record limit never unblocks positively current AI"); }
    { Fixture f(true); for(int n=3;n<=26;++n) { f.h.Car(n,1); f.p.TrackCar(f.h,Id(n)); }
      check(!f.p.Car(Id(26)) && f.Gate(26,1250,b::Seam::VehicleStart).blockNative,"car record limit never unblocks positively current AI occupancy"); }
    { bool valid=false; check(b::SegmentTouchesSphere({-10,0,0},{10,0,0},{{0,0,0},5},valid)&&valid,"donor swept center intersection");
      check(b::SegmentTouchesSphere({-10,5,0},{10,5,0},{{0,0,0},5},valid)&&valid,"tangent segment touches sphere");
      check(!b::SegmentTouchesSphere({1e308,0,0},{-1e308,0,0},{{0,0,0},5},valid)&&!valid,"finite input arithmetic overflow stays unknown"); }
    std::printf("ChallengeEscapeBoundaryPolicy: %u checks, %u failures; max foot actions=%u car actions=%u\n",checks,failures,b::Policy::FootActionLimit,b::Policy::CarActionLimit);
    return failures ? 1 : 0;
}
