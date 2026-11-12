#pragma once
#include <cstdint>
namespace cfgbundle {
// UE computes UFunction.ParmsSize from the end of the final native parameter,
// not the padded descriptor/C++ allocation. Both sizes remain exact facts.
inline bool NativeFrame(std::uint16_t observed,unsigned nativeBytes,unsigned storageBytes){
    return nativeBytes<=storageBytes&&storageBytes<=4096&&observed==nativeBytes;
}
enum class MoveOutcome : unsigned { Refused, Accepted, AlreadyAtGoal, Invalid };
inline MoveOutcome ClassifyMove(unsigned result){
    return result==0?MoveOutcome::Refused:result==1?MoveOutcome::AlreadyAtGoal:
        result==2?MoveOutcome::Accepted:MoveOutcome::Invalid;
}
struct MoveCooldown {
    unsigned failures=0;
    std::uint64_t next=0;
    bool Due(std::uint64_t now)const{return now>=next;}
    void Clear(){failures=0;next=0;}
    void Completed(std::uint64_t now,unsigned result){
        if(ClassifyMove(result)==MoveOutcome::Invalid)return;
        if(ClassifyMove(result)==MoveOutcome::Refused){
            if(failures<3)++failures;
            next=now+(failures==1?1800:failures==2?3600:5000);
        }else{failures=0;next=now+1200;}
    }
};
struct ActorFollowPath {
    std::uintptr_t target=0;
    std::uint64_t until=0;
    bool Preserve(std::uint64_t now,std::uintptr_t actor)const{
        return actor&&actor==target&&now<until;
    }
    void Completed(std::uint64_t now,std::uintptr_t actor,unsigned result){
        // Native MoveToActor follows its actor continuously. Reuse the base
        // 6107ms accepted-path grace; Failed/AlreadyAtGoal do not retain it.
        if(actor&&ClassifyMove(result)==MoveOutcome::Accepted){target=actor;until=now+6107;}
        else Clear();
    }
    void Clear(){target=0;until=0;}
};
}
