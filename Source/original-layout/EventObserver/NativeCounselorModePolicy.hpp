#pragma once
#include "../RuntimeProbe/NativeFrontendCatalogues.hpp"
namespace cfgmenu::launch {
// Default preserves the already-native Sandbox path. Only the coordinator's
// accepted dedicated counselor picker transaction may select the second case.
// This is admission policy, not possession/profile/URL manipulation or a write.
enum class Origin {NativeSandbox,AcceptedCounselorPicker};
inline bool ModesForOrigin(Origin origin,const catalogues::Modes& modes,std::uintptr_t selected){
    if(!selected||!modes.selectedMap||modes.count>catalogues::MaximumModes)return false;
    unsigned source=0,target=0;
    for(unsigned i=0;i<modes.count;++i){const auto& entry=modes.entries[i];
        const bool sandbox=entry.name.View()==u"ModeDef_Sandbox_C"&&entry.alias.View()==u"snbx";
        if(sandbox){if(!entry.supportsSelectedMap)return false;++target;}
        if(entry.type!=selected)continue;
        if(!entry.supportsSelectedMap)return false;
        if(origin==Origin::NativeSandbox&&sandbox)++source;
        else if(origin==Origin::AcceptedCounselorPicker&&(sandbox||
            (entry.name.View()==u"ModeDef_OfflineBots_C"&&entry.alias.View()==u"oflb")))++source;
    }
    return source==1&&target==1;
}
}
