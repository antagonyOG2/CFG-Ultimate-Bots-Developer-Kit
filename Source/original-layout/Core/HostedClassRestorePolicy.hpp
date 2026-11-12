#pragma once
#include <string_view>
namespace cfghostedrestore {
inline bool EndPhase(std::u16string_view phase){return !phase.empty()&&phase!=u"None"&&phase!=u"InProgress";}
}
