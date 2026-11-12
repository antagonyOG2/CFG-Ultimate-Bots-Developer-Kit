#pragma once
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
#include "../RuntimeProbe/FrontendCatalogues.generated.hpp"
namespace cfghelpercatalogue {
enum class Stage:unsigned {Ready,Context,HeaderRead,HeaderShape,Empty,EntryRead,ChangedHeader,ChangedEntry,FinalContext};
struct Diagnostic {Stage stage=Stage::Context;cfgspawn::PlayerArray header{};unsigned index=0;};
// An isolated read-only discovery callback. Commit only two equal samples of
// the bounded mode-owned catalogue under fresh owner proof; no conversions,
// factories, native spawn resources or engine calls belong in this pass.
template<class Reader,class Host,std::size_t N>bool Capture(Reader& reader,Host& host,std::uintptr_t mode,
    cfgspawn::PlayerArray& output,std::array<cfgprimary::SoftClassBytes,N>& entries,Diagnostic& diag){
    diag={};cfgspawn::PlayerArray header{},again{};std::array<cfgprimary::SoftClassBytes,N> observed{};
    if(!host.HelperContext())return false;
    diag.stage=Stage::HeaderRead;
    if(!reader.Read(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,header))return false;
    diag.header=header;diag.stage=Stage::HeaderShape;
    if(!cfgprimary::SaneCatalogue(header)||static_cast<unsigned>(header.count)>N)return false;
    diag.stage=Stage::Empty;if(!header.count)return false;
    diag.stage=Stage::EntryRead;
    for(unsigned i=0;i<static_cast<unsigned>(header.count);++i){diag.index=i;
        if(!reader.Read(header.data+40ULL*i,observed[i]))return false;}
    diag.stage=Stage::ChangedHeader;
    if(!reader.Read(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,again)||
       again.data!=header.data||again.count!=header.count||again.capacity!=header.capacity)return false;
    diag.stage=Stage::ChangedEntry;
    for(unsigned i=0;i<static_cast<unsigned>(header.count);++i){diag.index=i;cfgprimary::SoftClassBytes fresh{};
        if(!reader.Read(header.data+40ULL*i,fresh)||fresh!=observed[i])return false;}
    diag.stage=Stage::FinalContext;if(!host.HelperContext())return false;
    output=header;entries=observed;diag.stage=Stage::Ready;return true;
}
}
