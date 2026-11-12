#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/SpawnRuntimeBindings.generated.hpp"
namespace cfgspawn {
constexpr unsigned HelperDefaultAssetIndex=3,SpawnAssetCount=7;
// Donor pawn/class/CDO retain positive serial requirements. The helper CDO,
// native cached functions and mode-owned tree may have unassigned serials.
// These observations are NOT weak identities or GC pins: each resource's
// ownership/cache/type guards below must pass before the explicit native calls.
template<class Reader> bool ReadSpawnAssetSlot(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t object,unsigned index,cfgprobe::lifetime::SlotObservation& slot) {
    return index<SpawnAssetCount && ObjectSlot(r,array,object,slot) &&
        (slot.HasAssignedSerial() || index>=HelperDefaultAssetIndex);
}
template<class Reader> bool ObserveOwnedNativeHelper(Reader& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t helperClass,std::uintptr_t helperDefault,
    std::uintptr_t spawnFunction,const cfgprobe::lifetime::SlotObservation& classSlot,
    const cfgprobe::lifetime::SlotObservation& defaultSlot,const cfgprobe::lifetime::SlotObservation& functionSlot) {
    if (!cfgprobe::ObjectPointer(helperClass) || !cfgprobe::ObjectPointer(helperDefault) ||
        !cfgprobe::ObjectPointer(spawnFunction) || classSlot.object!=helperClass || defaultSlot.object!=helperDefault ||
        functionSlot.object!=spawnFunction) return false;
    // Two bounded samples of the engine's initialized native-class caches and
    // ownership references. Never allocate a serial, create an asset or adopt a
    // changed pointer. Called at preparation and on the verified owner callback.
    for (unsigned sample=0;sample<2;++sample) {
        std::uintptr_t reflection=0,constructor=0,cdo=0,actualClass=0,owner=0,function=0;
        cfgprobe::lifetime::SlotObservation fresh;
        if (!r.Read(module+Classes[Helper].reflection,reflection) || reflection!=helperClass ||
            !r.Read(module+Classes[Helper].constructor,constructor) || constructor!=helperClass ||
            !ObjectSlot(r,array,helperClass,fresh) || !(fresh==classSlot) ||
            !r.Read(helperClass+ClassDefaultObject,cdo) || cdo!=helperDefault ||
            !ObjectSlot(r,array,helperDefault,fresh) || !(fresh==defaultSlot) ||
            !r.Read(helperDefault+cfgprobe::ObjectClass,actualClass) || actualClass!=helperClass ||
            !r.Read(module+SpawnFunctionCache,function) || function!=spawnFunction ||
            !ObjectSlot(r,array,spawnFunction,fresh) || !(fresh==functionSlot) ||
            !r.Read(spawnFunction+0x20,owner) || owner!=helperClass) return false;
    }
    return true;
}
template<class Reader> bool ObserveModeTreeAndLocation(Reader& r,std::uintptr_t module,std::uintptr_t mode,
    const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t tree,std::uintptr_t treeClass,
    std::uintptr_t locationFunction,std::uintptr_t actorClass,const cfgprobe::lifetime::SlotObservation& treeSlot,
    const cfgprobe::lifetime::SlotObservation& functionSlot,const cfgprobe::lifetime::SlotObservation& actorClassSlot) {
    if (!cfgprobe::ObjectPointer(mode) || !cfgprobe::ObjectPointer(tree) || !cfgprobe::ObjectPointer(treeClass) ||
        !cfgprobe::ObjectPointer(locationFunction) || !cfgprobe::ObjectPointer(actorClass) || treeSlot.object!=tree ||
        functionSlot.object!=locationFunction || actorClassSlot.object!=actorClass) return false;
    for (unsigned sample=0;sample<2;++sample) {
        std::uintptr_t value=0;
        cfgprobe::lifetime::SlotObservation fresh;
        if (!r.Read(mode+ModeCounselorTree,value) || value!=tree ||
            !r.Read(module+Classes[Tree].reflection,value) || value!=treeClass ||
            !r.Read(module+Classes[Tree].constructor,value) || value!=treeClass ||
            !ObjectSlot(r,array,tree,fresh) || !(fresh==treeSlot) || !ObjectIs(r,tree,treeClass) ||
            !r.Read(module+LocationFunctionCache,value) || value!=locationFunction ||
            !r.Read(module+ActorClassReflectionCache,value) || value!=actorClass ||
            !ObjectSlot(r,array,actorClass,fresh) || !(fresh==actorClassSlot) ||
            !ObjectSlot(r,array,locationFunction,fresh) || !(fresh==functionSlot) ||
            !r.Read(locationFunction+0x20,value) || value!=actorClass) return false;
    }
    return true;
}
} // namespace cfgspawn
