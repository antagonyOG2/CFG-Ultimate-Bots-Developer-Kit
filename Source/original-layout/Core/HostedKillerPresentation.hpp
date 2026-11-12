#pragma once
#include "../RuntimeProbe/ClassIdentity.hpp"

namespace cfghostedpresentation {
// Donor OfflineSetup.cpp:10331 passes jason->Class. Resolve that class from
// the currently selected killer pawn before the AI birth is adopted; the
// adopted spawn template is populated only after the takeover transaction.
template<class Reader> bool SelectedKillerClass(Reader& reader,std::uintptr_t pawn,
    std::uintptr_t killerBase,std::uintptr_t& selectedClass) noexcept {
    selectedClass=0;
    if(!cfgprobe::ObjectPointer(pawn)||!cfgprobe::ObjectPointer(killerBase))return false;
    cfgprobe::Ancestry ancestry{};std::uintptr_t type=0,again=0;
    if(!reader.Read(pawn+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type)||
       !cfgprobe::CaptureAncestry(reader,pawn,ancestry)||!ancestry.Contains(killerBase)||
       !cfgprobe::StableAncestry(reader,pawn,ancestry)||
       !reader.Read(pawn+cfgprobe::ObjectClass,again)||again!=type)return false;
    selectedClass=type;return true;
}
}
