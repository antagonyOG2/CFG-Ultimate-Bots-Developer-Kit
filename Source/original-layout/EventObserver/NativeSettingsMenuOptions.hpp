#pragma once
#include "NativeSetupRows.hpp"
#include "NativeHumanJasonMenu.hpp"
namespace cfgmenu::settingsoptions {
inline rows::Spec Practice(RowId id,std::u16string_view title){
    rows::Spec spec;spec.id=id;spec.title=title;spec.count=2;spec.selected=0;
    // Preserve the authored ValueText binding just as the Challenges rows do.
    // A one-shot SetText("Off") replaces that binding while SelectedIndex still changes.
    spec.labels[0]=u"Off";spec.labels[1]=u"On";spec.preserveNativeValue=CFG_J216_FINISHING!=0;return spec;
}
inline rows::Spec MatchLength(){
    rows::Spec spec;spec.id=RowId::Duration;spec.title=u"Match Length";
    spec.labels[0]=u"30 minutes";spec.labels[1]=u"60 minutes";spec.labels[2]=u"90 minutes";
    spec.count=3;spec.selected=1;spec.preserveNativeValue=true;return spec;
}
// Enable only alongside the concrete early-arrival duration backend. Native
// counselor count, weather and character choices remain their original rows.
inline void EnableDuration(Capabilities& capabilities){capabilities.verified[static_cast<unsigned>(RowId::Duration)]=true;}
inline rows::Spec JasonDifficulty(){
    rows::Spec spec;spec.id=RowId::Difficulty;spec.title=u"Jason Difficulty";spec.count=3;spec.selected=1;spec.preserveNativeValue=true;
    spec.labels[0]=u"Easy";spec.labels[1]=u"Normal";spec.labels[2]=u"Hard";return spec;
}
inline void EnableDifficulty(Capabilities& capabilities){capabilities.verified[static_cast<unsigned>(RowId::Difficulty)]=true;}
// Do not append a duplicate: NativeMenuLaunch captures the retained original
// DifficultyComboBox by exact native Items names at Start.
inline void EnableSkill(Capabilities& capabilities){capabilities.verified[static_cast<unsigned>(RowId::Skill)]=true;}
inline rows::Spec CounselorSkill(){
    rows::Spec spec;spec.id=RowId::Skill;spec.title=u"Counselor Bot Skill";spec.count=3;spec.selected=1;
    spec.labels[0]=u"Low";spec.labels[1]=u"Medium";spec.labels[2]=u"High";
    spec.reuseNativeSkill=true;spec.preserveNativeValue=true;return spec;
}
inline rows::Spec StartingInventory(){
    rows::Spec spec;spec.id=RowId::Inventory;spec.title=u"Starting Inventory";spec.count=4;spec.selected=0;spec.preserveNativeValue=true;
    for(unsigned i=0;i<spec.count;++i)spec.labels[i]=restrictedinventory::Labels[i];return spec;
}
inline void EnableRestrictedInventory(Capabilities& capabilities){capabilities.verified[static_cast<unsigned>(RowId::Inventory)]=true;}
inline rows::Spec StartingWeapon(){
    rows::Spec spec;spec.id=RowId::Weapon;spec.title=u"Starting Weapon";spec.count=5;spec.preserveNativeValue=true;
    spec.labels[0]=u"None";spec.labels[1]=u"Random";spec.labels[2]=u"Machete";spec.labels[3]=u"Axe";spec.labels[4]=u"Shotgun";return spec;
}
inline rows::Spec ExpandedStartingInventory(){
    rows::Spec spec;spec.id=RowId::Inventory;spec.title=u"Starting Inventory";spec.count=6;spec.preserveNativeValue=true;
    spec.labels[0]=u"None";spec.labels[1]=u"Random";spec.labels[2]=u"Pocket Knife";spec.labels[3]=u"First Aid Spray";
    spec.labels[4]=u"Firecrackers";spec.labels[5]=u"Map";return spec;
}
inline void EnableExpandedLoadout(Capabilities& capabilities){
    capabilities.verified[static_cast<unsigned>(RowId::Weapon)]=true;EnableRestrictedInventory(capabilities);
}
}
