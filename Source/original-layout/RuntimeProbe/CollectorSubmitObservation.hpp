#pragma once
#include "CollectorSubmitBindings.generated.hpp"
namespace cfgcollectorsubmit {
// Read-only code identity. Never infer a publication/lifetime lease from this.
template<class Reader>bool Known(Reader& reader,std::uintptr_t module,std::uintptr_t target){
    if(!module)return false;
    if(target==module+Parallel){std::array<unsigned char,ParallelCode.size()> bytes{};
        return reader.Read(target,bytes)&&bytes==ParallelCode;}
    if(target==module+Serial){std::array<unsigned char,SerialCode.size()> bytes{};
        return reader.Read(target,bytes)&&bytes==SerialCode;}
    return false;
}
}
