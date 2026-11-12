#pragma once
#include "NativeChallengeEscapeActors.hpp"
#include "../Core/ChallengeEscapeBoundaryPolicy.hpp"

namespace cfgchallengeescape::seams {
using Seam=boundary::Seam;
struct Arguments {
    std::uintptr_t self=0,component=0,actor=0,otherComponent=0,exitPath=0,controller=0,volume=0;
    int bodyIndex=0;
    bool sweep=false;
    float delay=0;
};
inline std::uintptr_t ActorAddress(Seam seam,const Arguments& args) {
    switch(seam) {
        case Seam::CharacterStart: case Seam::VehicleStart: return args.self;
        case Seam::Overlap: case Seam::Presentation: case Seam::Completion: return args.actor;
    }
    return 0;
}
struct ActorProof {
    actors::Membership member{};
    actors::NativeProof type{};
};
struct Evidence {bool valid=false,hasVolume=false,hasOther=false;Seam seam=Seam::Overlap;Arguments args{};
 ActorProof actor{};actors::VolumeProof volume{};actors::NativeProof other{};};
inline bool Same(const Arguments& a,const Arguments& b){return a.self==b.self&&a.component==b.component&&a.actor==b.actor&&
 a.otherComponent==b.otherComponent&&a.exitPath==b.exitPath&&a.controller==b.controller&&a.volume==b.volume&&
 a.bodyIndex==b.bodyIndex&&a.sweep==b.sweep&&a.delay==b.delay;}
template<class R> bool ActorCurrent(R& r,std::uintptr_t module,const scope::Witness& w,const ActorProof& proof) {
    return actors::MembershipCurrent(r,module,w,proof.member) && actors::NativeCurrent(r,module,w,proof.type) &&
        actors::Same(r,proof.member.Slot().object+0x110,std::uint8_t{3});
}
template<class R> bool ObserveActor(R& r,std::uintptr_t module,const scope::Witness& w,std::uintptr_t actor,
    bool permitCar,bool requireCar,ActorProof& output) {
    output={}; ActorProof proof;
    if (!actors::ObserveMembership(r,module,w,actor,proof.member)) return false;
    bool typed=false;
    if (!requireCar) typed=actors::ObserveNative(r,module,w,actor,disk::SCCounselorCharacter,disk::SCCounselorCharacterCtor,proof.type);
    if (!typed && (permitCar || requireCar))
        typed=actors::ObserveNative(r,module,w,actor,disk::SCDriveableVehicle,disk::SCDriveableVehicleCtor,proof.type);
    if (!typed || !ActorCurrent(r,module,w,proof)) return false;
    output=proof; return true;
}
// Argument/source admission ONLY. Current must freshly prove the exact stock
// SP-challenge scope/committed setting/generation/local authority at each call.
// Validate never queries geometry, dispatches native APIs or infers AI/history.
// Policy/Access independently prove living AI eligibility and every car seat.
// The optional output is a positive current native actor identity, never an
// action lease. Call Validate again for another callback or changed arguments.
template<class R,class Current> bool Validate(R& r,std::uintptr_t module,const scope::Witness& w,
    Seam seam,const Arguments& args,Current&& current,boundary::Identity* actorIdentity=nullptr,Evidence* evidence=nullptr) {
    if (actorIdentity) *actorIdentity={};
    if (evidence) *evidence={};
    if (!actors::ScopeCurrent(r,module,w,current)) return false;
    const auto actor=ActorAddress(seam,args);
    ActorProof actorProof;
    actors::VolumeProof volumeProof;
    actors::NativeProof otherComponent;
    bool hasVolume=false,hasOther=false;
    switch(seam) {
        case Seam::Overlap:
            if (!actors::ObserveVolume(r,module,w,current,args.self,volumeProof) ||
                args.component!=volumeProof.sphere.slot.object || !ObserveActor(r,module,w,actor,true,false,actorProof)) return false;
            hasVolume=true;
            if (args.otherComponent) {
                if (!actors::ObserveNative(r,module,w,args.otherComponent,disk::PrimitiveComponent,disk::PrimitiveComponentCtor,otherComponent) ||
                    !actors::Same(r,args.otherComponent+0x20,actor)) return false;
                hasOther=true;
            }
            break;
        case Seam::CharacterStart:
            if (!ObserveActor(r,module,w,actor,false,false,actorProof)) return false;
            // ExitPath and delay are forwarded opaque arguments. Captured
            // serial-zero exit splines never become invented action receivers.
            break;
        case Seam::Presentation:
            if (!actors::ObserveVolume(r,module,w,current,args.self,volumeProof) ||
                !ObserveActor(r,module,w,actor,false,false,actorProof)) return false;
            hasVolume=true;
            break;
        case Seam::Completion:
            if (args.self!=w.Mode() || !ObserveActor(r,module,w,actor,false,false,actorProof)) return false;
            // Native R8 is the counselor. RDX is not required to equal its
            // current controller; completion can follow verified unpossession.
            break;
        case Seam::VehicleStart:
            if (!ObserveActor(r,module,w,actor,true,true,actorProof) ||
                !actors::ObserveVolume(r,module,w,current,args.volume,volumeProof)) return false;
            hasVolume=true;
            break;
        default: return false;
    }
    if (!actors::ScopeCurrent(r,module,w,current) || !ActorCurrent(r,module,w,actorProof) ||
        (hasVolume && !actors::CurrentVolume(r,module,w,current,volumeProof)) ||
        (hasOther && (!actors::NativeCurrent(r,module,w,otherComponent) || !actors::Same(r,args.otherComponent+0x20,actor))) ||
        !actors::ScopeCurrent(r,module,w,current) || !ActorCurrent(r,module,w,actorProof)) return false;
    if (actorIdentity) {
        const auto& slot=actorProof.member.Slot();
        *actorIdentity={slot.object,slot.index,slot.serial};
    }
    if(evidence)*evidence={true,hasVolume,hasOther,seam,args,actorProof,volumeProof,otherComponent};
    return true;
}
template<class R,class Current>bool CurrentEvidence(R& r,std::uintptr_t module,const scope::Witness& w,Seam seam,
 const Arguments& args,Current&& current,const Evidence& p){
 return p.valid&&p.seam==seam&&Same(args,p.args)&&actors::ScopeCurrent(r,module,w,current)&&
  ActorCurrent(r,module,w,p.actor)&&(seam!=Seam::Completion||args.self==w.Mode())&&
  (!p.hasVolume||actors::CurrentVolume(r,module,w,current,p.volume))&&
  (!p.hasOther||(actors::NativeCurrent(r,module,w,p.other)&&actors::Same(r,args.otherComponent+0x20,ActorAddress(seam,args))))&&
  actors::ScopeCurrent(r,module,w,current)&&ActorCurrent(r,module,w,p.actor);
}
}
