#pragma once
#include "JasonInstanceObservation.hpp"
#include "CounselorTerminalObservation.hpp"
#include <cmath>
namespace cfgconvergencefear {
struct Evidence {std::uintptr_t pawn=0,type=0;cfgprobe::lifetime::SlotObservation manager{};float amount=0;};
// Callback-local strong owner evidence, including native managers with serial0.
// No retained weak reference, serial allocation, global scan or remote player.
template<class R> bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t module,const Evidence& e){
    const auto& binding=cfghook::disk::Types[cfghook::disk::Fear];
    return cfgprobe::ObjectPointer(e.pawn)&&cfgprobe::ObjectPointer(e.type)&&
        cfgobserver::Same(r,module+binding.reflection,e.type)&&cfgobserver::Same(r,module+binding.constructor,e.type)&&
        cfgobserver::Same(r,e.pawn+cfghook::disk::FearComponent,e.manager.object)&&
        cfgobserver::Same(r,e.manager.object+cfgprobe::ObjectClass,e.type)&&
        cfgobserver::Same(r,e.manager.object+0x20,e.pawn)&&cfgjason::OwnedComponentSlot(r,array,e.manager);
}
template<class R> bool Capture(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t module,std::uintptr_t pawn,Evidence& out){
    out={};Evidence e;e.pawn=pawn;std::uintptr_t manager=0;
    const auto& binding=cfghook::disk::Types[cfghook::disk::Fear];
    if(!cfgprobe::ObjectPointer(pawn)||!r.Read(module+binding.reflection,e.type)||
        !r.Read(pawn+cfghook::disk::FearComponent,manager)||!cfgspawn::ObjectSlot(r,array,manager,e.manager)||
        !Current(r,array,module,e)||!r.Read(manager+cfghook::disk::FearValue,e.amount)||
        !std::isfinite(e.amount)||e.amount<0||e.amount>100000||!Current(r,array,module,e)||
        !cfgescape::Same(r,manager+cfghook::disk::FearValue,e.amount))return false;
    out=e;return true;
}
}
