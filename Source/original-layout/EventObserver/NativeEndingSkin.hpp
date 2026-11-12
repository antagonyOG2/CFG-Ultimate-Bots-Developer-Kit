#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/EndingMetadataBindings.generated.hpp"
#include "../Core/EndingMetadataPolicy.hpp"
#include "PrimarySpawnObservation.hpp"
namespace cfgending::skin {
// Snapshot-local proof only. Serial zero is not promoted into a retained lease:
// the selected class must remain strongly linked from the current typed state.
template<class R> bool LiveSlot(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot,bool classDefault=false){
    std::uint32_t item=0,flags=0;
    return cfgspawn::ObjectSlot(r,registry,object,slot)&&
        r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,item)&&!(item&0x30000000u)&&
        // ClassDefaultObject/Archetype flags are legitimate on a CDO reached
        // through the verified class's exact +F8 link. Keep actor exclusions
        // for state/class inputs, and item-level GC exclusions for EVERY input.
        r.Read(object+8,flags)&&(classDefault||!(flags&0x30u));
}
template<class R,class T> bool Same(R& r,std::uintptr_t at,const T& expected){
    T actual{};return r.Read(at,actual)&&actual==expected;
}
template<class R> bool Safe(R& r,std::uintptr_t module){
    return cfgprimary::LookupSafe(r,module);
}
template<class R> bool Concrete(R& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t value){
    std::uintptr_t base=0,meta=0,defaults=0;
    cfgprobe::lifetime::SlotObservation klass{},baseSlot{},cdo{},again{};
    if(!Safe(r,module)||!r.Read(module+disk::SCJasonSkinClass,base)||!cfgprobe::ObjectPointer(base)||
       !Same(r,module+disk::SCJasonSkinConstructor,base)||!LiveSlot(r,registry,base,baseSlot)||
       !LiveSlot(r,registry,value,klass)||!r.Read(base+cfgprobe::ObjectClass,meta)||
       !cfgspawn::ObjectIs(r,value,meta)||!cfgspawn::ClassContains(r,value,base)||
       !r.Read(value+cfgspawn::ClassDefaultObject,defaults)||defaults==value||
       !LiveSlot(r,registry,defaults,cdo,true)||!Same(r,defaults+cfgprobe::ObjectClass,value))return false;
    return cfgspawn::ClassContains(r,value,base)&&cfgspawn::ObjectIs(r,value,meta)&&
        Same(r,value+cfgspawn::ClassDefaultObject,defaults)&&Same(r,defaults+cfgprobe::ObjectClass,value)&&
        Same(r,base+cfgprobe::ObjectClass,meta)&&Same(r,module+disk::SCJasonSkinClass,base)&&
        Same(r,module+disk::SCJasonSkinConstructor,base)&&
        LiveSlot(r,registry,base,again)&&again==baseSlot&&LiveSlot(r,registry,value,again)&&again==klass&&
        LiveSlot(r,registry,defaults,again,true)&&again==cdo&&Safe(r,module);
}
// Copy the selected native human loadout. A source-owned SCJasonSkin no-override
// value is valid ancestry equality in PlayOutro; NEVER manufacture that value
// as a fallback when source selection is missing. Refuse different selected
// killers, foreign owners, GC or changed slots.
// No asset load, serial assignment, object creation, or target write occurs here.
template<class R> bool Source(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t state,std::uintptr_t controller,const Packet& selected,std::uintptr_t& out){
    out=0;Packet picked{};std::uintptr_t type=0,value=0;
    cfgprobe::lifetime::SlotObservation slot{},again{};
    if(!Safe(r,module)||!EmptySubpath(selected)||!LiveSlot(r,registry,state,slot)||!slot.HasAssignedSerial()||
       !r.Read(module+disk::SCPlayerStateClass,type)||!Same(r,module+disk::SCPlayerStateConstructor,type)||
       !cfgspawn::ObjectIs(r,state,type)||!Same(r,state+0x88,controller)||!Same(r,controller+0x388,state)||
       !Same(r,state+0x110,std::uint8_t{3})||!r.Read(state+disk::PickedClass,picked)||!SamePath(picked,selected)||
       !r.Read(state+disk::PickedSkin,value)||!Concrete(r,module,registry,value))return false;
    Packet current{};
    if(!Same(r,state+disk::PickedSkin,value)||!r.Read(state+disk::PickedClass,current)||current!=picked||
       !Same(r,state+0x88,controller)||!Same(r,controller+0x388,state)||!cfgspawn::ObjectIs(r,state,type)||
       !Same(r,module+disk::SCPlayerStateClass,type)||!Same(r,module+disk::SCPlayerStateConstructor,type)||
       !LiveSlot(r,registry,state,again)||!(again==slot)||!Safe(r,module))return false;
    out=value;return true;
}
// Shared by the real BundleAccess adapter and synthetic tests. The caller marks
// SkinInFlight BEFORE entry; native callback reentry cannot replay this setter.
// Existing valid selections are retained. Never overwrite a non-null foreign or
// invalid cosmetic. Both source and target are freshly rechecked around native.
template<class H> unsigned Ensure(H& h,std::uintptr_t& out){
    out=0;std::uintptr_t current=0,source=0,again=0;
    if(!h.Fresh()||!h.Target(current))return 1;
    if(current){if(!h.Concrete(current)||!h.Fresh()||!h.Target(again)||again!=current)return 2;out=current;return 0;}
    // Source performs the full typed, concrete-class, double-sampled strong
    // owner proof itself. Do not repeat three cold proofs and exhaust the
    // shared1536-read callback budget before the setter can ever be reached.
    if(!h.Source(source)||!source)return 3;
    if(!h.Fresh()||!h.Target(current)||current)return 4;
    if(!h.Set(source))return 5;
    if(!h.Fresh()||!h.Target(current)||current!=source||!h.Source(again)||again!=source)return 6;
    out=current;return 0;
}
// Same donor skin-before-weapon transaction, split at the native call boundary.
// CFG's full owner/source proofs must fit EACH bounded callback. The previous
// single slice could exhaust its1536 reads AFTER the setter and stop before the
// weapon/SettingsLoaded step. Never replay an entered setter to recover budget.
struct Transfer {
    unsigned phase=0,failure=0;std::uintptr_t expected=0;bool calling=false;
    template<class H>bool Step(H& h){
        if(calling)return false;
        if(phase>=2)return phase==2;
        std::uintptr_t current=0,source=0,again=0;
        const auto refuse=[&](unsigned why){failure=why;phase=3;return false;};
        if(!h.Fresh()||!h.Target(current))return refuse(1);
        if(!phase){
            if(current){
                if(!h.Concrete(current)||!h.Fresh()||!h.Target(again)||again!=current)return refuse(2);
                expected=current;phase=2;return true;
            }
            if(!h.Source(source)||!source)return refuse(3);
            if(!h.Fresh()||!h.Target(again)||again)return refuse(4);
            expected=source;phase=1; // consume BEFORE native entry, including reentry/exception
            calling=true;const bool returned=h.Set(source);calling=false;
            if(!returned)return refuse(5);
            return false; // post-setter source/target proof gets a FRESH callback
        }
        if(current!=expected||!h.Concrete(current)||!h.Source(source)||source!=expected||
           !h.Fresh()||!h.Target(again)||again!=expected)return refuse(6);
        phase=2;return true;
    }
};
}
