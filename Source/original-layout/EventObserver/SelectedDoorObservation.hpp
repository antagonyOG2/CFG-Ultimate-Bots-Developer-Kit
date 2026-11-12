#pragma once
#include "BundleObservations.hpp"
#include "../RuntimeProbe/DoorBindings.generated.hpp"
#include "../RuntimeProbe/DoorBreakBindings.generated.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
namespace cfgdoor {
struct Selected {
    std::uintptr_t manager=0,component=0,door=0;
    cfgprobe::lifetime::SlotObservation componentSlot{},doorSlot{};
};
struct BreakState {
    std::uintptr_t target=0,killer=0;
    std::uint8_t wants=0,open=0,locked=0,destroyed=0,barricaded=0;
    bool Blocked()const{return locked||barricaded;}
    bool operator==(const BreakState& b)const{return target==b.target&&killer==b.killer&&wants==b.wants&&open==b.open&&locked==b.locked&&destroyed==b.destroyed;}
    bool Owned(std::uintptr_t jason,std::uintptr_t door)const{return target==door&&killer==jason&&!open&&!destroyed;}
    // Selection admits closed doors, but an attackable component is not proof
    // that opening is unavailable. Unlocked doors take the verified native
    // Interact path; only locked closed doors request Jason's breakdown.
    bool CanRequestNative(std::uint8_t attackable)const{
        // Intent can remain set after native completion cleared both ownership
        // edges. The donor does not treat intent alone as a live breakdown.
        // Native Press/CanAttack still decides whether another action can start.
        return attackable==1&&Blocked()&&!target&&!killer&&wants<=1&&!open&&!destroyed;
    }
    bool CanRequestNativeOpen(std::uint8_t attackable)const{
        return attackable==1&&!Blocked()&&!target&&!killer&&wants<=1&&!open&&!destroyed;
    }
};
// Native SCDoor open predicate3903A0: !Locked && (!LinkedBarricade ||
// !LinkedBarricade.bEnabled). Link presence/animation transition are not locks.
template<class R>bool ObserveBarricade(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    const Selected& selected,std::uintptr_t type,std::uint8_t& enabled){
    enabled=0;std::uintptr_t actor=0;cfgprobe::lifetime::SlotObservation a{},b{},door{};std::uint8_t value=2,again=2;
    if(!r.Read(selected.door+breakdisk::LinkedBarricade,actor))return false;
    if(actor&&(!cfgspawn::ObjectSlot(r,objects,actor,a)||!a.HasAssignedSerial()||!cfgspawn::ObjectIs(r,actor,type)||
       !cfgobserver::Same(r,actor+breakdisk::LinkedDoor,selected.door)||
       !r.Read(actor+breakdisk::BarricadeEnabled,value)||value>1||
       !r.Read(actor+breakdisk::BarricadeEnabled,again)||again!=value||
       !cfgobserver::Same(r,actor+breakdisk::LinkedDoor,selected.door)||
       !cfgspawn::ObjectSlot(r,objects,actor,b)||!(a==b)))return false;
    if(!cfgobserver::Same(r,selected.door+breakdisk::LinkedBarricade,actor)||
       !cfgspawn::ObjectSlot(r,objects,selected.door,door)||!(door==selected.doorSlot))return false;
    enabled=actor?value:0;return true;
}
template<class R>bool NativeSelectionMatches(R& r,const Selected& selected){
    std::uintptr_t best=0,fallback=0,again=0;
    if(!r.Read(selected.manager+disk::Best,best)||!r.Read(selected.manager+disk::Fallback,fallback)||
       (best?best:fallback)!=selected.component||
       !r.Read(selected.manager+disk::Best,again)||again!=best||
       !r.Read(selected.manager+disk::Fallback,again)||again!=fallback)return false;
    return true; // exact Best-or-null-Fallback branch in the pinned native Press
}
template<class R>bool ObserveBreak(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t jason,const Selected& selected,std::uintptr_t doorClass,BreakState& out){
    out={};BreakState a,b;cfgprobe::lifetime::SlotObservation fresh{};
    const auto read=[&](BreakState& s){return r.Read(jason+breakdisk::Target,s.target)&&
        r.Read(jason+breakdisk::Wants,s.wants)&&s.wants<=1&&r.Read(selected.door+disk::Open,s.open)&&s.open<=1&&
        r.Read(selected.door+breakdisk::Locked,s.locked)&&s.locked<=1&&
        r.Read(selected.door+breakdisk::Destroyed,s.destroyed)&&s.destroyed<=1&&
        r.Read(selected.door+breakdisk::DoorKiller,s.killer);};
    if(!selected.doorSlot.HasAssignedSerial()||!cfgspawn::ObjectSlot(r,objects,selected.door,fresh)||!(fresh==selected.doorSlot)||
       !cfgspawn::ObjectIs(r,selected.door,doorClass)||!read(a)||!read(b)||!(a==b)||
       !cfgspawn::ObjectSlot(r,objects,selected.door,fresh)||!(fresh==selected.doorSlot))return false;
    out=a;return true;
}
template<class R>bool ObserveBreakTarget(R& r,std::uintptr_t jason,std::uintptr_t& out){
    out=0;std::uintptr_t a=0,b=0;
    if(!r.Read(jason+breakdisk::Target,a)||!r.Read(jason+breakdisk::Target,b)||a!=b)return false;
    out=a;return true; // current Jason identity/GC/thread is owned by caller
}
template<class R> bool ObserveSelected(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t jason,std::uintptr_t doorClass,std::uintptr_t componentClass,Selected& result){
    result={};Selected s;std::uintptr_t best=0,backup=0;std::uint8_t open=0;
    if(!r.Read(jason+cfghunter::disk::Manager,s.manager)||!r.Read(s.manager+disk::Best,best)||
       !r.Read(s.manager+disk::Fallback,backup))return false;
    // The selected, strongly owned component may have no weak serial. Its
    // exact slot/owner/selection are sampled again; the door actor must have
    // a positive identity. Never assign a component serial to make this pass.
    const auto candidate=[&](std::uintptr_t component){s.component=component;
    return cfgspawn::ObjectSlot(r,objects,s.component,s.componentSlot)&&
       cfgspawn::ObjectIs(r,s.component,componentClass)&&r.Read(s.component+disk::ComponentOwner,s.door)&&
       cfgobserver::Same(r,s.component+0x20,s.door)&&
       cfgspawn::ObjectSlot(r,objects,s.door,s.doorSlot)&&s.doorSlot.HasAssignedSerial()&&
       cfgspawn::ObjectIs(r,s.door,doorClass)&&r.Read(s.door+disk::Open,open)&&open==0;};
    // Best may be a nearby window/other interaction while Fallback is the
    // closed door blocking this path. Evaluate both native-selected edges,
    // never scan for another door or bypass a lock.
    if(!candidate(best)&&(!backup||backup==best||!candidate(backup)))return false;
    cfgprobe::lifetime::SlotObservation c{},d{};
    if(!cfgobserver::Same(r,jason+cfghunter::disk::Manager,s.manager)||
       !cfgobserver::Same(r,s.manager+disk::Best,best)||!cfgobserver::Same(r,s.manager+disk::Fallback,backup)||
       !cfgobserver::Same(r,s.component+disk::ComponentOwner,s.door)||
       !r.Read(s.door+disk::Open,open)||open!=0||
       !cfgspawn::ObjectSlot(r,objects,s.component,c)||!(c==s.componentSlot)||
       !cfgspawn::ObjectSlot(r,objects,s.door,d)||!(d==s.doorSlot))return false;
    result=s;return true;
}
inline bool Same(const Selected& a,const Selected& b){
    return a.manager==b.manager&&a.component==b.component&&a.door==b.door&&a.componentSlot==b.componentSlot&&a.doorSlot==b.doorSlot;
}
template<class R>bool ObserveSelectedAnchor(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    const Selected& selected,std::uintptr_t sceneClass,cfgspawn::Vec3& out){
    out={};cfgspawn::Vec3 a{},b{};cfgprobe::lifetime::SlotObservation slot{};
    if(!cfgspawn::ObjectIs(r,selected.component,sceneClass)||
       !cfgobserver::Same(r,selected.component+disk::ComponentOwner,selected.door)||
       !cfgobserver::Same(r,selected.component+0x20,selected.door)||
       !r.Read(selected.component+cfgposition::disk::SceneWorldTranslation,a)||!cfgspawn::Finite(a)||
       !r.Read(selected.component+cfgposition::disk::SceneWorldTranslation,b)||
       a.x!=b.x||a.y!=b.y||a.z!=b.z||
       !cfgspawn::ObjectSlot(r,objects,selected.component,slot)||!(slot==selected.componentSlot))return false;
    out=a;return true;
}
template<class R>bool ObserveAnchor(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    const Selected& selected,std::uintptr_t componentClass,std::uintptr_t sceneClass,cfgspawn::Vec3& out){
    out={};std::uintptr_t anchor=0;cfgprobe::lifetime::SlotObservation a{},b{};
    cfgspawn::Vec3 point{},again{};
    const auto currentDoor=[&](){cfgprobe::lifetime::SlotObservation slot{};
        return cfgspawn::ObjectSlot(r,objects,selected.door,slot)&&slot==selected.doorSlot;};
    // Native CanBreakDownDoor uses SCDoor.InteractComponent, not actor origin.
    // A strongly owned component may have serial zero; repeated exact owner,
    // native typed edges and slot equality supply its lifetime proof.
    if(!currentDoor()||
       !r.Read(selected.door+breakdisk::InteractComponent,anchor)||!anchor||
       !cfgspawn::ObjectSlot(r,objects,anchor,a)||!cfgspawn::ObjectIs(r,anchor,componentClass)||
       !cfgspawn::ObjectIs(r,anchor,sceneClass)||!cfgobserver::Same(r,anchor+disk::ComponentOwner,selected.door)||
       !cfgobserver::Same(r,anchor+0x20,selected.door)||
       !r.Read(anchor+cfgposition::disk::SceneWorldTranslation,point)||!cfgspawn::Finite(point)||
       !r.Read(anchor+cfgposition::disk::SceneWorldTranslation,again)||
       point.x!=again.x||point.y!=again.y||point.z!=again.z||
       !cfgspawn::ObjectSlot(r,objects,anchor,b)||!(a==b)||
       !cfgobserver::Same(r,selected.door+breakdisk::InteractComponent,anchor)||
       !cfgobserver::Same(r,anchor+disk::ComponentOwner,selected.door)||
       !cfgobserver::Same(r,anchor+0x20,selected.door)||!currentDoor())return false;
    out=point;return true;
}
}
