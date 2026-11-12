#pragma once
#include <array>
#include <string_view>
namespace cfgmenu::restrictedinventory {
constexpr std::array<std::u16string_view,4> Labels{{u"None",u"Pocket Knife",u"First Aid Spray",u"Map"}};
// Existing Selection.inventory retains released base indexes. The restricted
// wire configuration uses a separate enum, never those raw menu indexes.
inline bool BaseChoice(std::u16string_view label,unsigned& out){
    out=0;
    if(label==Labels[0])return true;
    if(label==Labels[1]){out=2;return true;}
    if(label==Labels[2]){out=3;return true;}
    if(label==Labels[3]){out=5;return true;}
    return false;
}
inline bool WireChoice(unsigned base,unsigned& out){
    out=0;switch(base){case 0:return true;case 2:out=1;return true;case 3:out=2;return true;case 5:out=3;return true;default:return false;}
}
}
