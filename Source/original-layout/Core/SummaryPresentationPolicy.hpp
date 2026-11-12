#pragma once
#include <cstdint>
namespace cfgsummarypresentation {
struct Scope {
    std::uintptr_t& slot;std::uintptr_t previous;
    Scope(std::uintptr_t& current,std::uintptr_t row):slot(current),previous(current){slot=row;}
    ~Scope(){slot=previous;}
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
};
inline bool MayOverride(bool native,bool includePending,bool started,bool ownerThread,bool caller,std::uintptr_t row){
    return !native&&!includePending&&started&&ownerThread&&caller&&row;
}
}
