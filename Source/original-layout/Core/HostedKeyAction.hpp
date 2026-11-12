#pragma once
namespace cfghostedkeys {
enum class Action{None,Disarm,Arm};
inline Action Select(bool preferenceHost,bool f8,bool f9){
 if(!preferenceHost)return Action::None;
 if(f9)return Action::Disarm;
 if(f8)return Action::Arm;
 return Action::None;
}
}
