#pragma once
#include "InventoryNativeEvidence.hpp"
#include "FeatureSnapshotBindings.generated.hpp"
namespace cfginventory {
// Initialized native caches only: no hierarchy search for absent SpawnItem,
// registry census, serial assignment, asset loads or native invocation.
// The caller supplies a fresh bounded reader. Result is ABI evidence only.
template<class Reader> bool ObserveCachedFrame(Reader& r,std::uintptr_t module,bool add,
    cfgmenu::capture::FunctionEvidence& out) {
    out={};const auto& ownerBinding=cfgfeature::disk::Classes[cfgfeature::disk::Inventory];
    std::uintptr_t owner=0,constructor=0,function=0;
    if(!r.Read(module+ownerBinding.reflection,owner)||!r.Read(module+ownerBinding.constructor,constructor)||
        !cfgprobe::MatchingClassCache(owner,constructor)||
        !r.Read(module+(add?disk::AddItemCache:disk::FindItemCache),function)||!cfgprobe::ObjectPointer(function))return false;
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field known;
    cfgmenu::capture::FunctionEvidence candidate;
    if(!reflection.ReadField(function,owner,known)||
        !cfgmenu::capture::KnownFunction(r,module,known,add?u"AddItem":u"FindItem",candidate)||
        !NativeFrameMatches(candidate,module,owner,add))return false;
    std::uintptr_t fresh=0;
    if(!r.Read(module+ownerBinding.reflection,fresh)||fresh!=owner||
        !r.Read(module+ownerBinding.constructor,fresh)||fresh!=owner||
        !r.Read(module+(add?disk::AddItemCache:disk::FindItemCache),fresh)||fresh!=function)return false;
    out=candidate;return true;
}
}
