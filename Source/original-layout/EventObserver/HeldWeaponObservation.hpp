#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/RestrictedLoadoutBindings.generated.hpp"
namespace cfgkill {
// Caller has just invoked the exact native GetCurrentWeapon getter on a known
// living counselor. A newly spawned native weapon may have unassigned serial0;
// accept it only through this repeated strong owner/class/registry proof. This
// observation is callback-local, never a retained weak identity or allocator.
template<class Reader> bool OwnedHeldWeapon(Reader& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t pawn,std::uintptr_t weapon,std::uintptr_t weaponBase){
    cfgprobe::lifetime::SlotObservation p{},p2{},w{},w2{};std::uintptr_t owner=0,again=0;
    cfgprobe::Ancestry type;
    return cfgprobe::ObjectPointer(weaponBase)&&cfgspawn::ObjectSlot(r,objects,pawn,p)&&p.HasAssignedSerial()&&
        cfgspawn::ObjectSlot(r,objects,weapon,w)&&
        r.Read(weapon+cfgloadout::restricteddisk::OwningCharacter,owner)&&owner==pawn&&
        cfgprobe::CaptureAncestry(r,weapon,type)&&type.Contains(weaponBase)&&cfgprobe::StableAncestry(r,weapon,type)&&
        cfgspawn::ObjectSlot(r,objects,weapon,w2)&&w==w2&&
        cfgspawn::ObjectSlot(r,objects,pawn,p2)&&p==p2&&
        r.Read(weapon+cfgloadout::restricteddisk::OwningCharacter,again)&&again==pawn;
}
}
