#pragma once
#include "NativeStartupRetention.hpp"
#include "NativeOfflineIntroMetadata.hpp"
#include "../RuntimeProbe/SelectedJasonClassObservation.hpp"
#include "../RuntimeProbe/PrimaryJasonBindings.generated.hpp"
namespace cfgselectedjason {
inline bool ConstructPath(std::uintptr_t helper,cfgmenu::promotion::Packet& output,std::uintptr_t klass)noexcept{
    __try{return reinterpret_cast<std::uintptr_t(__fastcall*)(void*,const std::uintptr_t*)>(helper)(&output,&klass)==reinterpret_cast<std::uintptr_t>(&output);}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// This is the native Class -> SoftClass VALUE constructor, never a name load.
// Caller marks its one attempt before entry. An ambiguous call is quarantined
// by the existing Intro root scope; neither this path nor a retry allocates a
// serial to repair a missing selected-class identity.
template<class Reader,class Fresh>bool MakePath(Reader& r,std::uintptr_t module,const Choice& selected,
    std::uintptr_t killerBase,Fresh&& fresh,cfgmenu::promotion::Packet& output){
    namespace d=cfgprimary::disk;output={};cfgprobe::lifetime::ArrayHeader registry;
    if(!fresh()||!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
        !Current(r,module,registry,killerBase,selected)||
        !cfgmenu::promotion::detail::Code(module,module+d::SelectedClassPath,d::SelectedClassPathCode)||
        !cfgmenu::startupretain::Retain(module,std::array<std::uintptr_t,2>{selected.identity.object,selected.defaultsIdentity.object})||!fresh())return false;
    if(!cfgmenu::startupretain::Invoke(module,cfgmenu::startupretain::Call::SelectedClassPath,
        std::array<std::uintptr_t,2>{selected.identity.object,selected.defaultsIdentity.object},
        [&](){return ConstructPath(module+d::SelectedClassPath,output,selected.identity.object);})||!fresh()||
        !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
        !Current(r,module,registry,killerBase,selected)||!cfgmenu::intrometadata::Plain(output))return false;
    cfgnames::Words words{};std::memcpy(&words,output.bytes.data()+16,8);
    cfgnames::Observation<Reader> names(r,module);cfgnames::PathText path;
    return names.Decode(words,path)&&path.View()==selected.Path()&&
        cfgmenu::classchoice::MatchesPath(r,module,selected.identity.object,killerBase,output.bytes)&&fresh();
}
}
