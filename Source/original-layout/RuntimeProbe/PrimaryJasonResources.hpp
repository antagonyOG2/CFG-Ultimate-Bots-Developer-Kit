#pragma once
#include "AiResourceSnapshot.hpp"
#include "Compatibility/CfgBuildProfile.hpp"
#include <cstring>

namespace cfgprimary {
// Exact CFG metadata: Diagnostics/CFG-PrimaryJason-Owners.json and
// CFG-PrimaryResource-Owners.json. These are observations, not call bindings.
constexpr std::uintptr_t ModeReflection=cfgcompat::cfg35::objects::SCGameModeReflection;
constexpr std::uintptr_t ModeConstructor=cfgcompat::cfg35::objects::SCGameModeConstructor;
constexpr std::uintptr_t StateReflection=cfgcompat::cfg35::objects::SCGameStateReflection;
constexpr std::uintptr_t StateConstructor=cfgcompat::cfg35::objects::SCGameStateConstructor;
constexpr std::uintptr_t KillerClasses=0x750,CounselorTree=0x9C8;
constexpr std::uintptr_t CurrentKiller=0x420,CurrentKillerState=0x490,SelectedKiller=0x538;
// CFG's Conv_SoftClassReferenceToClass descriptor: SoftClass(kind13) at0,
// pointer return at40, frame48. Array inner descriptor is also kind13.
// Never interpret this opaque packet as a pointer or select stock index6.
using SoftClassBytes=std::array<unsigned char,40>;
struct Report {
    cfgspawn::PlayerArray catalogue{};
    std::array<SoftClassBytes,64> entries{};
    SoftClassBytes selected{};
    std::uintptr_t killer=0,killerState=0,counselorTree=0;
    cfgprobe::lifetime::SlotObservation modeSlot{},stateSlot{},killerSlot{},killerStateSlot{},treeSlot{};
    bool EmptyPrimary() const noexcept { return !killer && !killerState; }
};
inline bool SaneCatalogue(const cfgspawn::PlayerArray& value) noexcept {
    return value.count>=0 && value.count<=64 && value.capacity>=value.count && value.capacity<=256 &&
        (!value.count || (cfgprobe::ObjectPointer(value.data) &&
            cfgprobe::lifetime::Extent(value.data,static_cast<std::uintptr_t>(value.count)*40)));
}
template<class Reader> bool Capture(Reader& reader,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t mode,std::uintptr_t state,
    std::uintptr_t sandboxBase,std::uintptr_t human,std::uintptr_t counselorBase,
    std::uintptr_t killerBase,std::uintptr_t playerStateBase,std::uintptr_t treeBase,Report& output) {
    output={}; Report candidate;
    std::array<std::uintptr_t,4> caches{};
    constexpr std::array<std::uintptr_t,4> cacheOffsets{{ModeReflection,ModeConstructor,StateReflection,StateConstructor}};
    for (unsigned i=0;i<caches.size();++i)
        if (!reader.Read(module+cacheOffsets[i],caches[i])) return false;
    const auto nativeMode=cfgprobe::MatchingClassCache(caches[0],caches[1]);
    const auto nativeState=cfgprobe::MatchingClassCache(caches[2],caches[3]);
    if (!nativeMode || !nativeState ||
        !cfgspawn::ObjectIs(reader,mode,nativeMode) || !cfgspawn::ObjectIs(reader,mode,sandboxBase) ||
        !cfgspawn::ObjectIs(reader,state,nativeState) || !cfgspawn::ObjectIs(reader,human,counselorBase) ||
        cfgspawn::ObjectIs(reader,human,killerBase) ||
        !cfgspawn::ObjectSlot(reader,objects,mode,candidate.modeSlot) ||
        !cfgspawn::ObjectSlot(reader,objects,state,candidate.stateSlot) ||
        !reader.Read(mode+KillerClasses,candidate.catalogue) || !SaneCatalogue(candidate.catalogue) ||
        !reader.Read(state+CurrentKiller,candidate.killer) ||
        !reader.Read(state+CurrentKillerState,candidate.killerState) ||
        !reader.Read(state+SelectedKiller,candidate.selected) ||
        !reader.Read(mode+CounselorTree,candidate.counselorTree)) return false;
    if (candidate.killer && (candidate.killer==human ||
        !cfgspawn::ObjectIs(reader,candidate.killer,killerBase) ||
        !cfgspawn::ObjectSlot(reader,objects,candidate.killer,candidate.killerSlot))) return false;
    if (candidate.killerState && (!cfgspawn::ObjectIs(reader,candidate.killerState,playerStateBase) ||
        !cfgspawn::ObjectSlot(reader,objects,candidate.killerState,candidate.killerStateSlot))) return false;
    if (candidate.counselorTree && (!cfgspawn::ObjectIs(reader,candidate.counselorTree,treeBase) ||
        !cfgspawn::ObjectSlot(reader,objects,candidate.counselorTree,candidate.treeSlot))) return false;
    for (std::int32_t i=0;i<candidate.catalogue.count;++i)
        if (!reader.Read(candidate.catalogue.data+static_cast<std::uintptr_t>(i)*40,candidate.entries[static_cast<unsigned>(i)])) return false;
    cfgspawn::PlayerArray again{}; SoftClassBytes selectedAgain{};
    std::uintptr_t value=0;
    if (!reader.Read(mode+KillerClasses,again) || again.data!=candidate.catalogue.data ||
        again.count!=candidate.catalogue.count || again.capacity!=candidate.catalogue.capacity ||
        !reader.Read(state+SelectedKiller,selectedAgain) || selectedAgain!=candidate.selected ||
        !reader.Read(state+CurrentKiller,value) || value!=candidate.killer ||
        !reader.Read(state+CurrentKillerState,value) || value!=candidate.killerState ||
        !reader.Read(mode+CounselorTree,value) || value!=candidate.counselorTree) return false;
    for (std::int32_t i=0;i<candidate.catalogue.count;++i) {
        SoftClassBytes entry{};
        if (!reader.Read(candidate.catalogue.data+static_cast<std::uintptr_t>(i)*40,entry) ||
            entry!=candidate.entries[static_cast<unsigned>(i)]) return false;
    }
    cfgprobe::lifetime::SlotObservation fresh{};
    const auto stableSlot=[&](const cfgprobe::lifetime::SlotObservation& old) {
        return cfgspawn::ObjectSlot(reader,objects,old.object,fresh) && fresh==old;
    };
    if (!stableSlot(candidate.modeSlot) || !stableSlot(candidate.stateSlot) ||
        (candidate.killer && !stableSlot(candidate.killerSlot)) ||
        (candidate.killerState && !stableSlot(candidate.killerStateSlot)) ||
        (candidate.counselorTree && !stableSlot(candidate.treeSlot))) return false;
    if (!cfgspawn::ObjectIs(reader,mode,nativeMode) || !cfgspawn::ObjectIs(reader,mode,sandboxBase) ||
        !cfgspawn::ObjectIs(reader,state,nativeState) || !cfgspawn::ObjectIs(reader,human,counselorBase) ||
        (candidate.killer && !cfgspawn::ObjectIs(reader,candidate.killer,killerBase)) ||
        (candidate.killerState && !cfgspawn::ObjectIs(reader,candidate.killerState,playerStateBase)) ||
        (candidate.counselorTree && !cfgspawn::ObjectIs(reader,candidate.counselorTree,treeBase))) return false;
    for (unsigned i=0;i<caches.size();++i)
        if (!reader.Read(module+cacheOffsets[i],value) || value!=caches[i]) return false;
    output=candidate; return true;
}
} // namespace cfgprimary
