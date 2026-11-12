#pragma once
#include "../Core/PrimaryJasonParameters.hpp"
#include "../RuntimeProbe/PrimaryJasonBindings.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgBuildProfile.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#include "SpawnAssetObservation.hpp"
#include "../RuntimeProbe/SelectedJasonClassObservation.hpp"
namespace cfgprimary {
constexpr unsigned CompletionReadReserve=3500;
inline bool CanComplete(unsigned consumed,unsigned cap) {
    return consumed<=cap && cap-consumed>=CompletionReadReserve;
}
template<class Reader> bool SameEntry(Reader& r,std::uintptr_t mode,const Report& prepared,unsigned index) {
    cfgspawn::PlayerArray header{}; SoftClassBytes fresh{};
    return index<static_cast<unsigned>(prepared.catalogue.count) && index<prepared.entries.size() &&
        r.Read(mode+KillerClasses,header) && SaneCatalogue(header) && header.data==prepared.catalogue.data &&
        header.count==prepared.catalogue.count && header.capacity==prepared.catalogue.capacity &&
        r.Read(header.data+static_cast<std::uintptr_t>(index)*40,fresh) && fresh==prepared.entries[index];
}
template<class Reader> bool RegistryMatches(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgprobe::lifetime::SlotObservation& gameState,std::uintptr_t expected,
    std::uintptr_t humanController,std::uintptr_t playerStateBase) {
    cfgprobe::lifetime::SlotObservation fresh{};
    std::uintptr_t killer=0,state=0,controller=0,value=0;
    if (!cfgspawn::ObjectSlot(r,array,gameState.object,fresh) || !(fresh==gameState) ||
        !r.Read(gameState.object+CurrentKiller,killer) || killer!=expected ||
        !r.Read(gameState.object+CurrentKillerState,state)) return false;
    if (!state) return true;
    return expected && r.Read(expected+0x3A0,controller) && cfgprobe::ObjectPointer(controller) && controller!=humanController &&
        r.Read(controller+0x370,value) && value==expected && r.Read(controller+0x388,value) && value==state &&
        cfgspawn::ObjectSlot(r,array,state,fresh) && cfgspawn::ObjectIs(r,state,playerStateBase);
}
template<class Reader> bool ObserveResolvedTemplate(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t pawnClass,std::uintptr_t killerBase,std::uintptr_t baseAI,cfgspawn::TemplateObservation& output) {
    output={}; cfgspawn::TemplateObservation candidate{}; candidate.pawnClass=pawnClass;
    if (!cfgspawn::ClassContains(r,pawnClass,killerBase) ||
        !cfgspawn::ObjectSlot(r,array,pawnClass,candidate.classSlot) ||
        !r.Read(pawnClass+cfgspawn::ClassDefaultObject,candidate.defaults) ||
        !cfgspawn::ObjectSlot(r,array,candidate.defaults,candidate.defaultSlot)) return false;
    std::uintptr_t value=0;
    if (!r.Read(candidate.defaults+cfgprobe::ObjectClass,value) || value!=pawnClass ||
        !r.Read(candidate.defaults+cfgspawn::PawnAIClass,candidate.defaultAI) || candidate.defaultAI!=baseAI ||
        !r.Read(candidate.defaults+cfgspawn::CharacterTree,candidate.defaultTree) || candidate.defaultTree) return false;
    cfgprobe::lifetime::SlotObservation fresh{};
    if (!cfgspawn::ObjectSlot(r,array,pawnClass,fresh) || !(fresh==candidate.classSlot) ||
        !r.Read(pawnClass+cfgspawn::ClassDefaultObject,value) || value!=candidate.defaults ||
        !cfgspawn::ObjectSlot(r,array,candidate.defaults,fresh) || !(fresh==candidate.defaultSlot) ||
        !r.Read(candidate.defaults+cfgprobe::ObjectClass,value) || value!=pawnClass ||
        !r.Read(candidate.defaults+cfgspawn::PawnAIClass,value) || value!=baseAI ||
        !r.Read(candidate.defaults+cfgspawn::CharacterTree,value) || value!=0 ||
        !cfgspawn::ClassContains(r,pawnClass,killerBase)) return false;
    candidate.kind=2; candidate.aiKind=3; output=candidate; return true;
}
template<class Reader>bool ObserveSelectedTemplate(Reader& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& registry,const cfgselectedjason::Choice& selected,
    std::uintptr_t killerBase,std::uintptr_t baseAI,cfgspawn::TemplateObservation& output){
    output={};cfgspawn::TemplateObservation candidate;
    if(!cfgselectedjason::Current(r,module,registry,killerBase,selected)||
        !ObserveResolvedTemplate(r,registry,selected.identity.object,killerBase,baseAI,candidate)||
        !(candidate.classSlot==selected.identity)||!(candidate.defaultSlot==selected.defaultsIdentity)||
        !cfgselectedjason::Current(r,module,registry,killerBase,selected))return false;
    output=candidate;return true;
}
}
