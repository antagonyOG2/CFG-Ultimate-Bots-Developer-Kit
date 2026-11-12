#pragma once
#include "SpawnAssetObservation.hpp"
#include "../RuntimeProbe/CrowdFallbackBindings.generated.hpp"
namespace cfgcrowd {
struct Observation { unsigned kind=0; std::uint32_t status=0; std::uint8_t state=255,registered=0; };
template<class R> bool Observe(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgprobe::lifetime::SlotObservation& path,std::uintptr_t controller,Observation& out) {
    cfgprobe::lifetime::SlotObservation fresh{}; std::uintptr_t table=0,value=0;
    if (!cfgspawn::ObjectSlot(r,array,path.object,fresh) || !(fresh==path) ||
        !r.Read(path.object+0x20,value) || value!=controller ||
        !r.Read(path.object+0xE0,value) || value!=controller || !r.Read(path.object,table)) return false;
    Observation o{};
    if (table==module+OrdinaryVtable) o.kind=1;
    else if (table==module+CrowdVtable) o.kind=2;
    else if (table==module+SCVtable) o.kind=3;
    else return false;
    if (!r.Read(table+FollowSlot,value) || value!=module+(o.kind==1?OrdinaryFollow:CrowdFollow) ||
        !r.Read(path.object+PathStatus,o.status) || o.status>3) return false;
    if (o.kind!=1 && (!r.Read(table+SetterSlot,value) || value!=module+Setter ||
        !r.Read(path.object+SimulationState,o.state) || o.state>DisabledState ||
        !r.Read(path.object+RegistrationFlags,o.registered))) return false;
    if (!cfgspawn::ObjectSlot(r,array,path.object,fresh) || !(fresh==path) ||
        !r.Read(path.object,value) || value!=table ||
        !r.Read(path.object+0x20,value) || value!=controller || !r.Read(path.object+0xE0,value) || value!=controller) return false;
    out=o; return true;
}
inline bool Idle(const Observation& o) { return o.kind>=1 && o.kind<=3 && o.status==0; }
inline bool OrdinaryReady(const Observation& o) { return o.kind==1 || (o.kind>=2 && o.kind<=3 && o.state==DisabledState && !(o.registered&0x10)); }
enum class OptionalSetterAction : unsigned { Refuse,Skip,Apply };
inline OptionalSetterAction OptionalSetter(const Observation& before) noexcept {
    if(!Idle(before))return OptionalSetterAction::Refuse;
    if(OrdinaryReady(before))return OptionalSetterAction::Skip;
    // The native setter returns immediately when the requested state already matches.
    if(before.state==DisabledState)return OptionalSetterAction::Refuse;
    return OptionalSetterAction::Apply;
}
inline bool SetterPostcondition(const Observation& before,const Observation& after) noexcept {
    return Idle(after)&&OrdinaryReady(after)&&after.kind==before.kind;
}
struct MovementSample { std::uintptr_t component=0,updated=0; std::uint8_t mode=0; float maxWalkSpeed=0; cfgspawn::Vec3 velocity{}; };
template<class R> bool Movement(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t pawn,MovementSample& out) {
    MovementSample m{}; cfgprobe::lifetime::SlotObservation component{},updated{},fresh{};
    std::uintptr_t type=0,value=0;
    if (!r.Read(module+MovementClassCache,type) || !cfgprobe::ObjectPointer(type) ||
        !r.Read(pawn+CharacterMovement,m.component) || !cfgspawn::ObjectSlot(r,array,m.component,component) ||
        !cfgspawn::ObjectIs(r,m.component,type) || !r.Read(m.component+0xE0,value) || value!=pawn ||
        !r.Read(m.component+0x20,value) || value!=pawn || !r.Read(m.component+UpdatedComponent,m.updated) ||
        !cfgspawn::ObjectSlot(r,array,m.updated,updated) || !r.Read(m.updated+0xE0,value) || value!=pawn ||
        !r.Read(m.component+MovementMode,m.mode) || m.mode>6 || !r.Read(m.component+MaxWalkSpeed,m.maxWalkSpeed) ||
        !std::isfinite(m.maxWalkSpeed) || m.maxWalkSpeed<0 || m.maxWalkSpeed>10000 ||
        !r.Read(m.component+Velocity,m.velocity) || !cfgspawn::Finite(m.velocity)) return false;
    if (!r.Read(module+MovementClassCache,value) || value!=type || !r.Read(pawn+CharacterMovement,value) || value!=m.component ||
        !cfgspawn::ObjectSlot(r,array,m.component,fresh) || !(fresh==component) ||
        !cfgspawn::ObjectIs(r,m.component,type) || !r.Read(m.component+0xE0,value) || value!=pawn ||
        !r.Read(m.component+0x20,value) || value!=pawn || !r.Read(m.component+UpdatedComponent,value) || value!=m.updated ||
        !cfgspawn::ObjectSlot(r,array,m.updated,fresh) || !(fresh==updated) || !r.Read(m.updated+0xE0,value) || value!=pawn) return false;
    out=m; return true;
}
}
