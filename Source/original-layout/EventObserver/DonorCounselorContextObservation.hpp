#pragma once
#include "../Core/H248SweaterTarget.hpp"
#include "BundleObservations.hpp"
#include "JasonInstanceObservation.hpp"
#include "../RuntimeProbe/Compatibility/CfgJasonHunting35.generated.hpp"
namespace cfgdonorhunt {
// FJB5991's misleading Offset_CounselorDead is the donor's context-kill
// byte1031. Named class/setter correspondence proves current1070. It must
// exclude a counselor being killed even before health/PlayerState turns dead.
// Read-only observation, never native-call or lifetime authority by itself.
template<class R>bool ObserveCounselorContextKill(R& r,
    const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
    const cfgprobe::lifetime::SlotObservation& pawn,std::uintptr_t counselorClass,bool& active,std::uint8_t* raw=nullptr){
    using namespace cfgcompat::cfg35::j1a;active=false;std::uint8_t value=0;
    if(!pawn.HasAssignedSerial()||!cfgjason::SameSlot(r,objects,pawn)||
       !cfgspawn::ObjectIs(r,pawn.object,counselorClass)||
       !cfgbundle::Code(r,module,ContextKillSetter,ContextKillSetterCode)||
       !r.Read(pawn.object+CounselorContextKill,value)||(raw&&(*raw=value,false))||value>1||
       !cfgjason::SameSlot(r,objects,pawn))return false;
    active=value!=0;return true;
}
}
