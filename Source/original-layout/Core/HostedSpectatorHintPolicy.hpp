#pragma once
#include <cstdint>
namespace cfghostedlabel {
struct Evidence {bool hosted=false,typedLocalHud=false,statusFunction=false,lookupClear=false;
    std::uintptr_t widget=0,function=0;};
struct Hint {std::uintptr_t widget=0,function=0;};
inline bool Publish(const Evidence& e,Hint& out) noexcept {
    out={};if(!e.hosted||!e.typedLocalHud||!e.statusFunction||!e.lookupClear||!e.widget||!e.function)return false;
    out={e.widget,e.function};return true;
}
inline bool RefreshDue(std::uint64_t now,bool localTerminal,std::uint64_t& next) noexcept {
    if(!localTerminal||now<next)return false;next=now+1000;return true;
}
}
