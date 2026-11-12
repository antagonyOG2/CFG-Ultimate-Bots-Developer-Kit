#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/SoundBlipBindings.generated.hpp"
namespace cfgsoundblip {
using Slot=cfgprobe::lifetime::SlotObservation;
// Publish only after the offline counselor route has validated its owned AI
// killer/controller and their native types. Clear active on route teardown.
// Caller checks the native game thread and LookupSafe before using this guard.
struct Scope {
    bool active=false,offlineCounselor=false;
    std::uintptr_t module=0,viewport=0,world=0,mode=0,humanController=0,humanPawn=0;
    Slot killer{},controller{};
    std::uintptr_t killerClass=0,controllerClass=0;
};
template<class R> bool Suppress(R& r,const Scope& s,std::uintptr_t emitter,bool gameThread){
    if(!gameThread||!s.active||!s.offlineCounselor||!emitter||emitter!=s.killer.object||
       !s.killer.HasAssignedSerial()||!s.controller.HasAssignedSerial()||!s.module||!s.viewport||
       !s.world||!s.mode||!s.humanController||!s.humanPawn||s.humanPawn==emitter||
       s.humanController==s.controller.object||!s.killerClass||!s.controllerClass)return false;
    auto same=[&](std::uintptr_t at,std::uintptr_t expected){std::uintptr_t value=0;return r.Read(at,value)&&value==expected;};
    // Bounded direct references only: no object scans, ancestry walks, or full
    // Context() call on the killer's tick. Failure leaves native behavior intact.
    cfgprobe::lifetime::ArrayHeader array;Slot killer,controller;
    return same(s.viewport+cfgprobe::ViewportWorld,s.world)&&
        same(s.world+cfgprobe::WorldNetDriver,0)&&same(s.world+cfgprobe::WorldGameMode,s.mode)&&
        cfgprobe::lifetime::ReadArrayHeader(r,s.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
        cfgspawn::ObjectSlot(r,array,emitter,killer)&&killer==s.killer&&
        cfgspawn::ObjectSlot(r,array,s.controller.object,controller)&&controller==s.controller&&
        same(emitter+cfgprobe::ObjectClass,s.killerClass)&&
        same(s.controller.object+cfgprobe::ObjectClass,s.controllerClass)&&
        same(emitter+0x3A0,s.controller.object)&&same(s.controller.object+cfgprobe::ControllerPawn,emitter)&&
        same(s.humanController+cfgprobe::ControllerPawn,s.humanPawn)&&
        same(s.viewport+cfgprobe::ViewportWorld,s.world)&&same(s.world+cfgprobe::WorldNetDriver,0);
}
// Host supplies the existing FeatureFunction()/Invoke() accounting. safe must
// be the caller's fresh LookupSafe result, thread must be its native-thread test.
template<class R,class H> class NativeAccess {
    R& r_;H& h_;Scope scope_;bool safe_,thread_;
    template<class T> bool Same(std::uintptr_t at,T expected){T value{};return r_.Read(at,value)&&value==expected;}
    template<std::size_t N> bool Code(std::uintptr_t at,const std::array<unsigned char,N>& expected){
        std::array<unsigned char,N> value{};return r_.Read(scope_.module+at,value)&&value==expected;
    }
    template<class T>static auto Hosted(T& h,const Scope& s,int)->decltype(h.HostedSoundBlipScope(s)){return h.HostedSoundBlipScope(s);}
    template<class T>static bool Hosted(T&,const Scope&,long){return false;}
public:
    NativeAccess(R& r,H& h,const Scope& s,bool safe,bool thread):r_(r),h_(h),scope_(s),safe_(safe),thread_(thread){}
    const Scope& Input()const{return scope_;}
    bool Guard(){return safe_&&thread_&&(Suppress(r_,scope_,scope_.killer.object,true)||Hosted(h_,scope_,0));}
    bool Visible(bool& out){std::uint8_t value=0;if(!r_.Read(scope_.killer.object+disk::Visible,value)||value>1)return false;out=value!=0;return true;}
    bool SetVisible(bool value){
        std::uintptr_t function=0,owner=0;
        if(!Guard()||!h_.FeatureFunction(disk::SetSoundBlipVisibility,function,owner)||
           !Same(scope_.module+disk::SetSoundBlipVisibility.ownerReflection,owner)||
           !Same(scope_.module+disk::SetSoundBlipVisibility.ownerConstructor,owner)||
           !cfgspawn::ClassContains(r_,scope_.killerClass,owner)||
           !Same(function+0xB0,scope_.module+disk::VisibilityExecutor)||
           !Code(disk::VisibilityExecutor,disk::VisibilityExecutorCode)||
           !Code(disk::VisibilityImplementation,disk::VisibilityImplementationCode)||!Guard())return false;
        std::uint8_t packet=value?1u:0u;
        return h_.Invoke(scope_.killer.object,function,&packet,true);
    }
};
struct Report {unsigned attempts=0,applied=0,restores=0,restored=0,refused=0,lost=0;};
class Runtime {
    bool attempted_=false,owned_=false;Scope saved_{};
    template<class A> bool SameOwner(A& a)const{const auto& s=a.Input();return s.world==saved_.world&&s.module==saved_.module&&
        s.mode==saved_.mode&&s.killer==saved_.killer&&s.controller==saved_.controller&&
        s.killerClass==saved_.killerClass&&s.controllerClass==saved_.controllerClass;}
public:
    Report report;
    bool Attempted()const{return attempted_;}
    bool Owned()const{return owned_;}
    template<class A> bool Apply(A& a){
        if(attempted_)return false;attempted_=true;bool before=false;
        if(!a.Guard()||!a.Visible(before)){++report.refused;return false;}
        saved_=a.Input();
        // Preserve restoration responsibility even if native dispatch is
        // ambiguous; Release requires confirmed false and the exact owner.
        // Donor always runs the native clear-existing-blips operation, even
        // when the visibility flag already reads false. Do not claim restore
        // ownership for a flag that was false before our transaction.
        owned_=before;++report.attempts;const bool returned=a.SetVisible(false);bool after=true;
        if(!returned||!a.Guard()||!a.Visible(after)||after){++report.refused;return false;}
        ++report.applied;return true;
    }
    template<class A> void Release(A& a){
        if(!owned_)return;owned_=false;bool current=true;
        if(!SameOwner(a)||!a.Guard()||!a.Visible(current)){++report.lost;return;}
        if(current)return; // native code already changed it; do not overwrite
        ++report.restores;const bool returned=a.SetVisible(true);bool after=false;
        if(returned&&a.Guard()&&a.Visible(after)&&after)++report.restored;else ++report.refused;
    }
};
}
