#pragma once
#include <cstdint>
#include "KillPolicy.hpp"
namespace cfgkill {
struct PamelaStockApproval {
    Identity identity{};std::uint64_t usedAt=0,at=0;
    std::uintptr_t context=0,component=0,pawn=0,weapon=0;
    std::int32_t index=-1,serial=0;
    bool Current(const Policy& p,std::uint64_t now)const{
        return p.PamelaAccepted()&&p.identity==identity&&usedAt==p.usedAt&&context&&context==p.context&&
            component&&pawn&&weapon&&index>=0&&serial>0&&at>=p.acceptedAt&&now>=at&&now-at<=20000&&now<p.until;
    }
    bool Remember(const Policy& p,std::uintptr_t ctx,std::uintptr_t comp,std::uintptr_t who,std::uintptr_t held,
        std::int32_t objectIndex,std::int32_t objectSerial,std::uint64_t now){
        if(!p.PamelaAccepted()||ctx!=p.context||!ctx||!comp||!who||!held||objectIndex<0||objectSerial<=0||
           now<p.acceptedAt||now>=p.until)return false;
        // Preserve the first stock approval for this exact earned context.
        if(identity==p.identity&&usedAt==p.usedAt&&context==ctx&&pawn)return false;
        *this={p.identity,p.usedAt,now,ctx,comp,who,held,objectIndex,objectSerial};return true;
    }
};
// Recheck the human first on EVERY phase, before one bounded AI fallback.
// Production Candidate proves lifetime, held weapon, manager and <=200cm.
template<class Host,class Weapon> bool SelectPamelaFinisher(Host& host,unsigned bots,unsigned& cursor,
    std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& kind){
    if(host.PamelaCandidate(0,pawn,weapon,kind))return true;
    if(!bots)return false;
    return host.PamelaCandidate(1+(cursor++%bots),pawn,weapon,kind);
}
}
