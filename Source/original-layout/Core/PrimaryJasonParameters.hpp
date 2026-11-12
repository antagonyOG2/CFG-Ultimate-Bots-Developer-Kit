#pragma once
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
namespace cfgprimary {
struct ConversionParameters {
    SoftClassBytes softClass{};
    std::uintptr_t result=0;
};
static_assert(sizeof(ConversionParameters)==48 && offsetof(ConversionParameters,result)==40,"CFG class conversion ABI");
inline bool BuildConversion(const SoftClassBytes& source,ConversionParameters& output) {
    output={};
    // This prototype accepts only catalogue packets with an empty subpath
    // payload, as observed in PID13344. No borrowed FString allocation/free.
    for (unsigned i=24;i<source.size();++i) if (source[i]) return false;
    bool pathPresent=false;
    for (unsigned i=16;i<24;++i) pathPresent=pathPresent || source[i]!=0;
    if (!pathPresent) return false;
    output.softClass=source; return true;
}
}
