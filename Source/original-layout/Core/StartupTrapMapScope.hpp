#pragma once
#include "../EventObserver/TrapNativeParameters.hpp"
#include <string_view>
namespace cfgtrap::startup {
// Input is ONLY FreshWorldMapObservation.actualPackage from the acknowledged
// current world, retained with that world's positive identity. It is not a
// selected menu label, map-definition name, URL, or guessed snow suffix.
inline MapFamily MapFamilyForPackage(std::u16string_view actualPackage){
    // Normal route in CFG-NativeRoutes.json; snow route independently confirmed
    // by native Browse/LoadMap in CampForestGreen-backup-2026.10.05-23.23.53.log
    // (05:41:41UTC). Fresh-world association still proves the selected authored
    // definition before this exact package allowlist is consulted.
    if(actualPackage==u"/Game/Maps/CrystalLake_Small/CrystalLake_Small"||
       actualPackage==u"/Game/Maps/CrystalLake_Small/Snow/CrystalLake_Small_Snow")return MapFamily::GroundMap;
    // This rejects Grendel AND GrendelLarge without inventing their packages.
    // Unknown includes all uncorroborated paths and must skip startup traps.
    return MapFamily::Unknown;
}
inline MapFamily MapFamilyForArrival(std::u16string_view actualPackage,std::uintptr_t observedWorld,std::uintptr_t currentWorld){
    return observedWorld&&observedWorld==currentWorld?MapFamilyForPackage(actualPackage):MapFamily::Unknown;
}
}
