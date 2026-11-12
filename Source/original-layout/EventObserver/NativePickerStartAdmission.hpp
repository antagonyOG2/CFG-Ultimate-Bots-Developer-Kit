#pragma once
#include "NativeMenuLaunchRuntime.hpp"
#include "NativeCounselorRoutePolicy.hpp"
namespace cfgmenu::pickerstart {
enum class Admission {Refused,Waiting,Submit,AlreadySubmitted};
inline Admission Admit(launch::State launchState,counselorroute::State routeState,bool actualFrame)noexcept{
    if(!actualFrame)return Admission::Refused;
    if(routeState==counselorroute::State::SettingsReady){
        if(launchState==launch::State::Opening)return Admission::Waiting;
        if(launchState==launch::State::Ready)return Admission::Submit;
    }
    if(routeState==counselorroute::State::Submitted&&
       (launchState==launch::State::Ready||launchState==launch::State::Submitted))return Admission::AlreadySubmitted;
    return Admission::Refused;
}
}
