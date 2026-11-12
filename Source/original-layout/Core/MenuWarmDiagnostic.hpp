#pragma once
#include <cstdint>
namespace cfgmenu {
// Samples from existing initialization reads only; never gameplay authority.
struct MenuWarmDiagnostic {
    std::uint32_t phase=0,stage=0,evidence=0,frame=0,parameter=0;
    std::uintptr_t owner=0,constructor=0,cached=0,result=0;
    std::uint32_t flags=0,parameterBytes=0,fields=0;
    std::int32_t storageBytes=0,offset=0,dimension=0,elementBytes=0;
    std::uint64_t propertyFlags=0;
};
}
