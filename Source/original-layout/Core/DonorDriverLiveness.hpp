#pragma once
#include <cstdint>
namespace cfgdonorhunt {
enum class UncertainAction:unsigned {None,Release,Stop,Move,Slash,Grab,GrabKill,Projection,SetLocation,Teleport,Running};
enum class DriverRefusal:unsigned {None,Owner,Code,Actor,Phase,Frame,ReadBudget,Census,NativeEntry,Quarantine,UncertaintyLimit,Retention,External,NoTarget};
// Review208 safety adapter. This clock owns no engine authority. A fresh
// native proof is still required to decide, and a poisoned GC bank stays shut.
struct DriverLiveness {
    std::uint64_t handoffAt=0,lastDonorDecisionAt=0,uncertainAt=0,settleUntil=0;
    UncertainAction uncertainAction=UncertainAction::None;
    DriverRefusal refusal=DriverRefusal::None;
    unsigned uncertainEvents=0;
    bool begun=false,decided=false,lapsed=false,permanentFallback=false,noTarget=false;
    void Begin(std::uint64_t now){if(!begun){begun=true;handoffAt=now;}}
    bool Active(std::uint64_t now)const {
        if(!begun||permanentFallback)return false;
        const auto at=decided?lastDonorDecisionAt:handoffAt;
        return now>=at&&now-at<=(decided?1500u:3000u);
    }
    // Rebuilding the next empty value frame must not hide the already-proved
    // NoTarget cause. Actual owner/code/retention refusals remain distinct.
    void Refused(DriverRefusal why){refusal=noTarget&&why==DriverRefusal::Frame?DriverRefusal::NoTarget:why;}
    bool ObserveLapse(std::uint64_t now){
        if(!begun||Active(now)||lapsed)return false;
        lapsed=true;return true;
    }
    bool Decision(std::uint64_t now,bool liveTarget=true){
        if(permanentFallback)return false;
        if(!liveTarget){noTarget=true;refusal=DriverRefusal::NoTarget;return false;}
        noTarget=false;
        const bool resumed=lapsed;Begin(now);decided=true;lastDonorDecisionAt=now;lapsed=false;refusal=DriverRefusal::None;
        return resumed;
    }
    void Uncertain(UncertainAction kind,std::uint64_t now){
        uncertainAction=kind;uncertainAt=now;settleUntil=now+1000;
        if(++uncertainEvents>=8){permanentFallback=true;refusal=DriverRefusal::UncertaintyLimit;}
        else refusal=DriverRefusal::Quarantine;
    }
    bool Settled(std::uint64_t now)const{return uncertainAction!=UncertainAction::None&&now>=settleUntil;}
};
}
