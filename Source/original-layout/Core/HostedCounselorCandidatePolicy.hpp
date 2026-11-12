#pragma once
#include <string_view>
namespace cfghostedcounselor {
inline bool PathLike(std::u16string_view value) noexcept {
    return value.find(u"/Counselors/")!=value.npos||value.find(u"Counselor")!=value.npos;
}
inline bool SupportedKind(std::u16string_view kind) noexcept {
    return kind==u"SoftClassProperty"||kind==u"ClassProperty"||kind==u"NameProperty"||
        kind==u"FNameProperty"||kind==u"StrProperty";
}
}
