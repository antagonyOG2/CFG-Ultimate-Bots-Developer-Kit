#pragma once
#include "ClassIdentity.hpp"
#include "Compatibility/CfgBuildProfile.hpp"
namespace cfgmenu::controllergate {
// Pinned CFG native SCPlayerController_Menu. Disk descriptors show it and
// gameplay SCPlayerController are siblings under ILLPlayerController.
constexpr std::uintptr_t MenuClass=cfgcompat::cfg35::objects::SCMenuControllerReflection,
    MenuConstructor=cfgcompat::cfg35::objects::SCMenuControllerConstructor;
template<class Reader> bool Menu(Reader& r,std::uintptr_t module,std::uintptr_t object){
    std::uintptr_t native=0,constructor=0,value=0;cfgprobe::Ancestry ancestry;
    if(!cfgprobe::ObjectPointer(module)||!r.Read(module+MenuClass,native)||
       !r.Read(module+MenuConstructor,constructor)||!cfgprobe::MatchingClassCache(native,constructor)||
       !cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(native)||
       !cfgprobe::StableAncestry(r,object,ancestry))return false;
    return r.Read(module+MenuClass,value)&&value==native&&
        r.Read(module+MenuConstructor,value)&&value==native;
}
}
