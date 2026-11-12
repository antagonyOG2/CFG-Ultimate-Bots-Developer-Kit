#pragma once
#include <cstdint>
#include "NativeSpawnParameters.hpp"
namespace cfgadditional {
// Donor reinforcement packet is distinct from primary/counselor birth. The
// donor explicitly sets bNoCollisionFail=true after same-level projection.
// Never mutate a class default/collision component or change primary births.
inline bool BuildPacket(std::uintptr_t world,std::uintptr_t type,cfgspawn::Vec3 at,cfgspawn::SpawnAIParameters& out){
    if(!cfgspawn::BuildParameters(world,type,type,at,{},out))return false;
    out.behaviorTree=0;out.noCollisionFail=1;return true;
}
// Enabled ONLY through the runtime pinned registry/delegate admission and
// per-extra immutable birth journal. Unknown listeners/owners still refuse.
constexpr bool LifecycleProved=true;
// Donor reinforcement ownership: never substitute the primary singleton.
// First CFG milestone deliberately allows one reinforcement (two Jasons).
enum class Stage {Idle,Select,Project,Pawn,Detach,ControllerClass,Controller,Possess,Navigation,RetireGeneric,Active,Refused,Indeterminate,Retired};
// One observational sample per turn. Closest living counselor includes the
// human equally; do not permanently exclude them while any bot is within80m.
struct Targets {
    unsigned cursor=0,count=0,selected=0,best=0;float distance=INFINITY;
    bool have=false;std::uint64_t next=0;
    void Sample(unsigned total,bool living,float squared,std::uint64_t now){
        if(!total){*this={};return;}
        if(count!=total){count=total;cursor=0;best=0;distance=INFINITY;have=false;}
        if(living&&std::isfinite(squared)&&squared>=0&&squared<distance){best=cursor;distance=squared;have=true;}
        if(++cursor>=count){selected=have?best:0;cursor=0;best=0;distance=INFINITY;have=false;next=now+911;}
        else next=now+100;
    }
};
struct Policy {
    Stage stage=Stage::Idle;bool down=false,pressed=false;
    unsigned requests=0,births=0,failures=0;std::uint64_t next=0,edgeAt=0,releaseAt=0,nextCombatAt=0;
    unsigned choices=0;std::uint64_t entropy=0;
    bool Choose(std::uint64_t seed){
        if(!choices)entropy=seed^0x9E3779B97F4A7C15ULL;
        entropy^=entropy>>12;entropy^=entropy<<25;entropy^=entropy>>27;
        ++choices;return (entropy*2685821657736338717ULL)%choices==0;
    }
    bool Request(bool key,bool foreground,bool scope,std::uint64_t now){
        const bool edge=foreground&&key&&!down;down=foreground&&key;
        if(!edge||!scope||stage!=Stage::Idle||(edgeAt&&now-edgeAt<700))return false;
        edgeAt=now;++requests;stage=Stage::Select;next=now;return true;
    }
    bool BirthPending()const{return stage>=Stage::Select&&stage<=Stage::RetireGeneric;}
    bool Due(std::uint64_t now)const{return (BirthPending()||stage==Stage::Active)&&now>=next;}
    bool CombatDue(std::uint64_t now)const{return !pressed&&now>=nextCombatAt;}
    void CombatDispatched(std::uint64_t now){pressed=true;releaseAt=now+150;nextCombatAt=now+911;}
    void Fail(bool allocated){++failures;stage=allocated?Stage::Indeterminate:Stage::Refused;}
    void Advance(std::uint64_t now){stage=static_cast<Stage>(static_cast<unsigned>(stage)+1);next=now+100;
        if(stage==Stage::Active)++births;}
};
}
