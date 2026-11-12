#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include "../EventObserver/SpectatorTerminalObservation.hpp"
namespace cfgspectator {
template<class A> auto DeferredPublication(A& a,int)->decltype(a.CameraPublicationDeferred()){return a.CameraPublicationDeferred();}
template<class A> bool DeferredPublication(A&,long){return false;}
struct Identity {
    std::uintptr_t object=0;std::int32_t serial=0;bool strongOwnedState=false;
    bool operator==(const Identity& b)const{return object==b.object&&serial==b.serial&&strongOwnedState==b.strongOwnedState;}
    bool operator!=(const Identity& b)const{return !(*this==b);}
    explicit operator bool()const{return object!=0&&(serial>0||(serial==0&&strongOwnedState));}
};
struct Vector {float x=0,y=0,z=0;};
struct Rotation {float pitch=0,yaw=0,roll=0;};
struct CameraFrame {Vector location;Rotation rotation;};
static_assert(sizeof(CameraFrame)==24,"CFG ClientSetSpectatorCamera frame");
static_assert(offsetof(CameraFrame,rotation)==12,"CFG camera rotation offset");
struct RestoreFrame {
    std::uintptr_t target=0;float blendTime=0;std::uint8_t blendFunction=0;
    std::uint8_t padding[3]{};float blendExponent=0;std::uint8_t lockOutgoing=0;
    std::uint8_t tail[3]{};
};
struct PauseFrame {std::uintptr_t context=0;std::uint8_t result=0;std::uint8_t padding[7]{};};
static_assert(sizeof(RestoreFrame)==24&&offsetof(RestoreFrame,blendExponent)==16&&offsetof(RestoreFrame,lockOutgoing)==20,"CFG restore frame");
static_assert(sizeof(PauseFrame)==16&&offsetof(PauseFrame,result)==8,"CFG pause frame");
struct Target {Identity pawn,state;bool living=false;};
constexpr unsigned MaximumCounselors=16;
struct Snapshot {
    Identity world,controller,spectator,observedState;
    // CFG SpectatingPlayer is SCCharacter*, unlike the canonical PlayerState
    // identities used by the donor cycling policy. Never publish state here.
    Identity nativeSelection;
    bool offline=false,localOwned=false,gameThread=false,inProgress=false;
    bool humanTerminal=false,spectatorPawn=false,paused=false;
    std::array<Target,MaximumCounselors> counselors{};unsigned count=0;
    // Optional monotonic native Next/Previous completion counter. Zero uses
    // donor pointer polling. An edge is needed when a one-person cycle repeats.
    std::uint64_t cycleEvent=0;
    Target jason;
};
inline bool ActorIdentity(const Identity& id){return id.object&&id.serial>0&&!id.strongOwnedState;}
inline bool Usable(const Target& t){return t.living&&ActorIdentity(t.pawn)&&t.state;}
inline void ResolveNativeSelection(Snapshot& s){
    s.observedState={};
    if(!ActorIdentity(s.nativeSelection))return;
    if(Usable(s.jason)&&s.nativeSelection==s.jason.pawn){s.observedState=s.jason.state;return;}
    for(unsigned i=0;i<s.count&&i<MaximumCounselors;++i)
        if(Usable(s.counselors[i])&&s.nativeSelection==s.counselors[i].pawn){s.observedState=s.counselors[i].state;return;}
}
// The stock spectator pawn may remain the camera's active view target while
// its native Viewing field changes during Next/Previous. Resolve that attached
// actor only against this snapshot's serial-validated native roster.
inline bool ResolveAttachedSelection(Snapshot& s,std::uintptr_t viewed){
    if(!viewed)return false;
    if(Usable(s.jason)&&s.jason.pawn.object==viewed){s.observedState=s.jason.state;return true;}
    for(unsigned i=0;i<s.count&&i<MaximumCounselors;++i)
        if(Usable(s.counselors[i])&&s.counselors[i].pawn.object==viewed){s.observedState=s.counselors[i].state;return true;}
    return false;
}
inline bool Finite(const Vector& p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&
    std::fabs(p.x)<=1000000.f&&std::fabs(p.y)<=1000000.f&&std::fabs(p.z)<=1000000.f;}
inline bool Finite(const Rotation& r){return std::isfinite(r.pitch)&&std::isfinite(r.yaw)&&std::isfinite(r.roll);}
inline bool Safe(const Snapshot& s){
    if(!s.offline||!s.localOwned||!s.gameThread||!s.inProgress||!s.humanTerminal||
       !s.spectatorPawn||!ActorIdentity(s.world)||!ActorIdentity(s.controller)||!ActorIdentity(s.spectator)||s.count>MaximumCounselors)return false;
    for(unsigned i=0;i<s.count;++i){
        if(!Usable(s.counselors[i])||s.counselors[i].pawn==s.jason.pawn||s.counselors[i].state==s.jason.state)return false;
        for(unsigned j=0;j<i;++j)if(s.counselors[i].pawn==s.counselors[j].pawn||s.counselors[i].state==s.counselors[j].state)return false;
    }
    return true;
}
// Camera orbit matches the donor: focus +92, 340-unit chase, upper-hemisphere
// pitch clamp, 14% follow smoothing and a snap on morphs over 1200 units.
struct Orbit {
    bool initialized=false;Vector focus{};Rotation rotation{};
    bool Build(Vector base,float yaw,const Rotation& look,bool hasLook,CameraFrame& out){
        if(!Finite(base)||!std::isfinite(yaw)||(hasLook&&!Finite(look)))return false;
        base.z+=92.f;if(!Finite(base))return false;
        if(!initialized){focus=base;rotation={-13.72f,yaw,0};initialized=true;}
        else {
            if(hasLook)rotation=look;
            const float dx=base.x-focus.x,dy=base.y-focus.y,dz=base.z-focus.z;
            if(dx*dx+dy*dy+dz*dz>1440000.f)focus=base;
            else {focus.x+=dx*.14f;focus.y+=dy*.14f;focus.z+=dz*.14f;}
        }
        rotation.pitch=std::remainder(rotation.pitch,360.f);
        if(rotation.pitch< -55.f)rotation.pitch=-55.f;
        if(rotation.pitch> -2.f)rotation.pitch=-2.f;
        rotation.yaw=std::remainder(rotation.yaw,360.f);rotation.roll=0;
        constexpr float radians=.01745329251994329577f;
        const float horizontal=std::cos(rotation.pitch*radians)*340.f;
        out.location={focus.x-std::cos(rotation.yaw*radians)*horizontal,
            focus.y-std::sin(rotation.yaw*radians)*horizontal,focus.z-std::sin(rotation.pitch*radians)*340.f};
        out.rotation=rotation;return Finite(out.location)&&Finite(out.rotation);
    }
};
enum class Result:unsigned {Inactive,Observing,Following,Restored,Refused};
struct Report {Result result=Result::Inactive;unsigned selections=0,cameras=0,restores=0,refusals=0,decision=0;};
// Access contract: Observe returns current registry-serial-validated identities
// and only living native roster entries, bounded to MaximumCounselors. Publish
// and RestoreCounselor MUST freshly revalidate Snapshot world, local ownership,
// terminal human, spectator pawn, match/pause, and target serial/liveness, and
// verify pinned code, reflection parameter frame and game thread at dispatch.
// CFG Publish hands the stock spectator view to its native attached follow.
// A selecting call may write SpectatingPlayer and hand off the view once;
// following is read-only ownership maintenance, not a custom camera-pose call.
// Never teleport the stock root to Orbit's eye: its native spring arm already
// supplies camera distance and its Tick owns the attached relative root pose.
// it must never SetViewTarget(Jason), possess Jason, or mutate watched actors.
// RestoreCounselor uses native controller camera/selection behavior only.
// Abandon performs no native RPC: its adapter independently validates the
// retained local ownership and CAS-clears only the Jason SpectatingPlayer it
// installed. It must not rely on a failed Snapshot or adopt replacement state.
class Runtime {
    Identity world_,controller_,spectator_,last_,active_,previous_;
    std::array<Identity,MaximumCounselors> seen_{};unsigned seenCount_=0;
    bool following_=false,keyHeld_=false;std::uint64_t cameraAt_=0;
    std::uint64_t cycleEvent_=0;
    Orbit orbit_;
    void Stop(){following_=false;active_={};previous_={};orbit_={};cameraAt_=0;}
    bool Seen(Identity state){
        for(unsigned i=0;i<seenCount_;++i)if(seen_[i]==state)return true;
        if(seenCount_<seen_.size())seen_[seenCount_++]=state;
        return false;
    }
    static const Target* Counselor(const Snapshot& s,Identity state){
        for(unsigned i=0;i<s.count;++i)if(s.counselors[i].state==state)return &s.counselors[i];
        return nullptr;
    }
    template<class A> void Restore(A& a,const Snapshot& s){
        report.decision=5;
        const auto* t=Counselor(s,previous_);if(!t&&s.count)t=&s.counselors[0];
        if(t&&a.RestoreCounselor(s,*t)){++report.restores;report.result=Result::Restored;last_=t->state;}
        else {if(following_)a.Abandon();++report.refusals;report.result=Result::Refused;}
        Stop();seenCount_=0;
    }
public:
    Report report;
    bool Following()const{return following_;}
    void Reset(){Stop();world_={};controller_={};spectator_={};last_={};seenCount_=0;keyHeld_=false;cycleEvent_=0;}
    template<class A> void Step(A& a,std::uint64_t now,bool f4Down){
        const bool pressed=f4Down&&!keyHeld_;keyHeld_=f4Down;
        Snapshot s;
        if(!a.Observe(s)){
            report.decision=1;
            if(following_)a.Abandon();
            // An incomplete bounded roster observation is not a new session.
            // Drop camera authority, but retain read-only cycle history until
            // the next fully proved snapshot verifies world/controller again.
            Stop();report.result=Result::Inactive;return;
        }
        if(!Safe(s)){
            report.decision=2;
            if(following_)a.Abandon();
            Reset();keyHeld_=f4Down;report.result=Result::Inactive;return;
        }
        if(world_!=s.world||controller_!=s.controller||spectator_!=s.spectator){
            if(following_)a.Abandon();
            Stop();last_={};seenCount_=0;world_=s.world;controller_=s.controller;spectator_=s.spectator;
            cycleEvent_=s.cycleEvent;
        }
        if(s.paused||s.count==0||!Usable(s.jason)){
            report.decision=3;
            // Donor offline guard: release the installed state while the
            // final counselor has retired. A single living counselor still
            // permits cycling through Jason; every camera dispatch repeats
            // terminal/roster identity checks. No RPC after final death/teardown.
            if(following_)a.Abandon();
            Stop();last_={};seenCount_=0;report.result=Result::Inactive;return;
        }
        if(following_&&active_!=s.jason.pawn){Restore(a,s);return;}
        if(following_&&s.observedState!=s.jason.state){
            if(Counselor(s,s.observedState)){
                Stop();seenCount_=0;last_=s.observedState;Seen(last_);
                cycleEvent_=s.cycleEvent;report.decision=6;report.result=Result::Observing;return;
            }
            else {
                // Donor: native Next/Previous already owns this transition.
                // Never restore a stale counselor over its selected view.
                a.Abandon();Stop();report.decision=4;report.result=Result::Observing;return;
            }
        }
        if(pressed&&following_){Restore(a,s);return;}
        bool select=pressed||s.observedState==s.jason.state;
        const bool cycleEdge=s.cycleEvent!=cycleEvent_;cycleEvent_=s.cycleEvent;
        if(!following_){
            if(Counselor(s,s.observedState)){
                if(!last_){last_=s.observedState;Seen(last_);}
                else if(last_!=s.observedState||cycleEdge){last_=s.observedState;select=Seen(last_)||select;}
            }else if(!select){
                // Native selection can be null/unrecognized between callbacks.
                // Donor waits without writing a view or clearing cycle history.
                // This state is NOT adopted as a target/camera authority.
                report.decision=4;report.result=Result::Observing;return;
            }
            if(!select){report.decision=6;report.result=Result::Observing;return;}
        }
        if(following_&&now<cameraAt_)return;
        Vector base;float yaw=0;Rotation look;bool hasLook=false;CameraFrame frame;
        Orbit next=orbit_;
        report.decision=7;
        const bool pose=a.ReadPose(s,s.jason,base,yaw,look,hasLook)&&next.Build(base,yaw,look,hasLook,frame);
        if(pose)report.decision=8;
        const bool published=pose&&a.Publish(s,s.jason,frame,!following_);
        if(!published&&pose&&following_&&DeferredPublication(a,0)){
            // Native camera placement refused, but the adapter re-proved the
            // same owner/world/selection and restored the last accepted view.
            // No new pose/camera success is claimed; retain the prior orbit.
            ++report.refusals;cameraAt_=now+33;report.result=Result::Following;return;
        }
        if(!published){
            a.Abandon(); // also covers a partially installed first selection
            ++report.refusals;report.result=Result::Refused;Stop();return;
        }
        if(!following_){previous_=Counselor(s,s.observedState)?s.observedState:Identity{};
            ++report.selections;following_=true;active_=s.jason.pawn;seenCount_=0;}
        orbit_=next;last_=s.jason.state;cameraAt_=now+16;++report.cameras;report.decision=9;report.result=Result::Following;
    }
};
}
