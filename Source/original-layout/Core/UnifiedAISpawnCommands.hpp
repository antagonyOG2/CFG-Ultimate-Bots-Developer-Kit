#pragma once
#include "GameplayLimits.hpp"
#include <cstdint>
namespace cfgspawncommand {
enum class Kind {ArmJason,AddCounselor};
enum class Mode {Offline,PrivateHosted};
enum class Result {Refused,Queued,AlreadyPresent,AlreadyQueued};
// Adapter-supplied, freshly proved authority; never infer this from a class
// name, hotkey, selected menu or cached lobby pointer. No native bindings here.
struct Scope {
    std::uintptr_t world=0,mode=0,netDriver=0;
    std::int32_t worldSerial=0,modeSerial=0,driverSerial=0;
    Mode route=Mode::Offline;
    bool gameThread=false,authority=false,privateLobby=false,inProgress=false,bindings=false;
};
inline bool Valid(const Scope& s){
    if(!s.world||!s.mode||s.worldSerial<=0||s.modeSerial<=0||
       !s.gameThread||!s.authority||!s.inProgress||!s.bindings)return false;
    return s.route==Mode::Offline?!s.netDriver:
        s.privateLobby&&s.netDriver&&s.driverSerial>0;
}
inline bool Same(const Scope& a,const Scope& b){
    return a.world==b.world&&a.mode==b.mode&&a.netDriver==b.netDriver&&a.route==b.route&&
        a.worldSerial==b.worldSerial&&a.modeSerial==b.modeSerial&&a.driverSerial==b.driverSerial;
}
// Shared command bookkeeping only. Both adapters must use the ONE registered
// AI backend; this does not create a second online AI implementation, call any
// offline lifecycle from a hosted world, or grant authority from a request.
class Queue {
    Scope owner_{};bool bound_=false,arm_=false,counselor_=false;
public:
    void Reset(){owner_={};bound_=arm_=counselor_=false;}
    Result Request(const Scope& s,Kind kind,bool explicitForegroundEdge,bool registeredJason,unsigned counselorTotal){
        if(!Valid(s)||!explicitForegroundEdge)return Result::Refused;
        if(bound_&&!Same(owner_,s)){Reset();return Result::Refused;} // fresh explicit rearm after travel
        if(!bound_){owner_=s;bound_=true;}
        if(kind==Kind::ArmJason){if(registeredJason)return Result::AlreadyPresent;
            if(arm_)return Result::AlreadyQueued;arm_=true;return Result::Queued;}
        if(!registeredJason||counselorTotal>=cfglimits::MaximumCounselors)return Result::Refused;
        if(counselor_)return Result::AlreadyQueued;counselor_=true;return Result::Queued;
    }
    bool Consume(const Scope& s,Kind kind){
        if(!bound_||!Valid(s)||!Same(owner_,s)){Reset();return false;}
        bool& pending=kind==Kind::ArmJason?arm_:counselor_;
        if(!pending)return false;pending=false;return true;
    }
};
}
