#pragma once
#include "../EventObserver/SpawnBindings.hpp"
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
#include "SelectedJasonClass.hpp"
#include "CounselorPracticeSettings.hpp"
namespace cfgbundle {
// Immutable native-menu submission for the existing gameplay runtime. No
// guessed asset names, base catalogue indexes or "first loaded Jason" fallback.
constexpr std::uint32_t ConfigurationMagic=0x43464743,ConfigurationVersion=7;
struct Configuration {
    std::uint32_t magic=ConfigurationMagic,version=ConfigurationVersion,bytes=sizeof(Configuration),reserved=0;
    cfgspawn::Identity identity{};
    std::uint64_t submission=0;
    std::uint32_t counselorBots=7,jasonCatalogueIndex=0xFFFFFFFF;
    cfgprimary::SoftClassBytes selectedJason{};
    std::uint32_t matchSeconds=0,nativeCounselorSkill=3; // Zero / 3 preserve native defaults.
    std::uint32_t restrictedInventory=0; // 0=None, 1=Pocket Knife, 2=First Aid Spray, 3=Map. NOT base indexes.
    std::uint32_t startingWeapon=0,startingInventory=0; // Donor indexes, Random resolved once at commit.
    std::uint32_t difficulty=1; // Easy half damage, Normal, Hard double damage/speed.
    cfgselectedjason::Choice selectedClass{}; // explicit loaded class; no catalogue index
    std::uint32_t counselorPractice=0; // independent OFF defaults; owned Counselor submission only
};
inline bool ValidConfiguration(const Configuration& c,unsigned maximumBots) noexcept {
    bool nonempty=false;for(auto byte:c.selectedJason)nonempty|=byte!=0;
    const bool indexed=c.selectedClass.Empty()&&nonempty&&c.jasonCatalogueIndex<64;
    const bool explicitClass=c.selectedClass.Valid()&&!nonempty&&c.jasonCatalogueIndex==cfgselectedjason::NoCatalogueIndex;
    return c.magic==ConfigurationMagic&&c.version==ConfigurationVersion&&c.bytes==sizeof(c)&&!c.reserved&&
        c.identity.Valid()&&c.submission&&(indexed||explicitClass)&&c.counselorBots<=maximumBots&&
        (c.matchSeconds==0||c.matchSeconds==1800||c.matchSeconds==3600||c.matchSeconds==5400)&&c.nativeCounselorSkill<=3&&c.restrictedInventory<=3&&
        cfgpractice::Valid(c.counselorPractice)&&c.difficulty<=2&&c.startingWeapon<5&&c.startingWeapon!=1&&c.startingInventory<6&&c.startingInventory!=1&&
        !(c.restrictedInventory&&(c.startingWeapon||c.startingInventory));
}
inline bool MatchesConfiguration(const Configuration& c,const cfgspawn::Identity& identity,
    const cfgprimary::Report& resources,unsigned maximumBots) noexcept {
    return ValidConfiguration(c,maximumBots)&&c.identity==identity&&cfgprimary::SaneCatalogue(resources.catalogue)&&
        (c.selectedClass.Valid()||(c.jasonCatalogueIndex<static_cast<unsigned>(resources.catalogue.count)&&
        resources.entries[c.jasonCatalogueIndex]==c.selectedJason));
}
}
