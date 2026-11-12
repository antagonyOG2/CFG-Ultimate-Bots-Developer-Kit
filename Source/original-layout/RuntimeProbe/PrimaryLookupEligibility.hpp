#pragma once
#include "Compatibility/CfgBuildProfile.hpp"
#include <cstdint>
#include "ScopedLookupEligibility.hpp"

namespace cfgprimary {
// These image predicates are necessary veto checks only; they do not prove
// UObject lifetime, establish game-thread ownership, or exclude concurrent GC.
inline constexpr std::uintptr_t SerializationFlag=cfgcompat::cfg35::lookup::SerializationFlagRva;
inline constexpr std::uintptr_t GarbageCollectionState=cfgcompat::cfg35::lookup::GarbageCollectionStateRva;
inline constexpr std::uintptr_t GarbageCollectionCounter=GarbageCollectionState;
inline constexpr std::uintptr_t SerializationGuardCode=cfgcompat::cfg35::lookup::SerializationGuardRva;
inline constexpr std::uintptr_t GarbageCollectionGuardCode=cfgcompat::cfg35::lookup::GarbageCollectionGuardRva;
inline constexpr auto& SerializationGuardPrefix=cfgcompat::cfg35::lookup::SerializationGuardPrefix;
inline constexpr auto& GarbageCollectionGuardPrefix=cfgcompat::cfg35::lookup::GarbageCollectionGuardPrefix;
inline constexpr bool NativeLookupVetoBindingsVerified=true;
inline constexpr bool RuntimeLookupEligibilityVerified=cfgcompat::cfg35::lookup::RuntimeLookupEligibilityVerified;

template<class Reader> bool LookupVetoClearForObservation(Reader& r,std::uintptr_t module) {
    std::uint8_t serializing=1; std::uint32_t collecting=1;
    return r.Read(module+SerializationFlag,serializing) && !serializing &&
        r.Read(module+GarbageCollectionCounter,collecting) && !collecting;
}
template<class Reader> bool LookupSafe(Reader& r,std::uintptr_t module) {
#if defined(CFG_UNVERIFIED_RUNTIME_FIXTURE)
    return LookupVetoClearForObservation(r,module);
#elif defined(CFG_RETAINED_GAMEPLAY_BUNDLE)
    return ScopedLookupEligibility<Reader>::Current(r,module)&&LookupVetoClearForObservation(r,module);
#else
    (void)r; (void)module;
    return RuntimeLookupEligibilityVerified && LookupVetoClearForObservation(r,module);
#endif
}
}
