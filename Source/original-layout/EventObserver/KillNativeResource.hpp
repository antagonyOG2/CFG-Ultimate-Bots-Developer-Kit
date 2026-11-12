#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
namespace cfgkill {
// Components/levels are strongly rooted by the freshly proved positive actor.
// A native zero->assigned serial transition is not pointer recycling. Positive
// replacement still refuses, and callers repeat class/world/reciprocal links.
inline bool StrongChildIdentity(const cfgprobe::lifetime::SlotObservation& old,
 const cfgprobe::lifetime::SlotObservation& fresh){
 return old.object==fresh.object&&old.index==fresh.index&&
  (old.serial==fresh.serial||(!old.serial&&fresh.serial>0));
}
template<class Reader> bool ActorWorldCurrent(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t module,std::uintptr_t actor,std::uintptr_t world,
    cfgprobe::lifetime::SlotObservation& levelSlot,std::uintptr_t& levelType){
    levelSlot={};levelType=0;std::uintptr_t level=0,native=0,value=0;
    cfgprobe::lifetime::SlotObservation classSlot{};
    return r.Read(actor+0x20,level)&&
        r.Read(module+disk::LevelReflection,native)&&r.Read(module+disk::LevelConstructor,value)&&
        cfgprobe::MatchingClassCache(native,value)&&cfgspawn::ObjectSlot(r,array,native,classSlot)&&
        cfgspawn::ObjectSlot(r,array,level,levelSlot)&&cfgspawn::ObjectIs(r,level,native)&&
        r.Read(level+cfgprobe::ObjectClass,levelType)&&r.Read(level+disk::OwningWorld,value)&&value==world&&
        r.Read(actor+0x20,value)&&value==level;
}
// Thirteen bounded known-slot/metadata reads. No factory, namespace lookup,
// allocation or native call. Executor/code proof remains the caller's job.
template<class Reader,class Binding> bool ResourceCurrent(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t module,const Binding& binding,std::uintptr_t function,std::uintptr_t owner,
    const cfgprobe::lifetime::SlotObservation& functionSlot,const cfgprobe::lifetime::SlotObservation& ownerSlot){
    cfgprobe::lifetime::SlotObservation fresh{};std::uint16_t bytes=0;std::uintptr_t value=0;
    // Compiled metadata is held by exact native function/class caches, not weak
    // references. Zero serials are accepted only with these strong cache links,
    // exact registry slot equality, owner/frame and caller's native code proof.
    return function&&owner&&
        functionSlot.object==function&&ownerSlot.object==owner&&
        cfgspawn::ObjectSlot(r,array,function,fresh)&&fresh==functionSlot&&
        cfgspawn::ObjectSlot(r,array,owner,fresh)&&fresh==ownerSlot&&
        r.Read(function+0x20,value)&&value==owner&&
        r.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)&&
        cfgbundle::NativeFrame(bytes,binding.nativeBytes,binding.storageBytes)&&
        r.Read(module+binding.ownerReflection,value)&&value==owner&&
        r.Read(module+binding.ownerConstructor,value)&&value==owner&&
        r.Read(module+binding.cache,value)&&value==function;
}
}
