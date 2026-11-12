#pragma once
#include "KillPolicy.hpp"
#include <array>
namespace cfgkilldiag {
// Diagnostic output only. None of these observations grant kill authority.
struct Hit {
    unsigned clause=0;bool event=false,packet=false,mask=false,range=false;
    std::uintptr_t function=0,instigator=0,causer=0,weapon=0,damageType=0;
    cfgkill::Weapon kind=cfgkill::Weapon::Other;float squared=0;
};
struct Budget {
    std::array<std::uint64_t,16> keys{};unsigned count=0;
    std::uint64_t nextContext=0;unsigned candidate=0;
    bool Available()const{return count<keys.size();}
    bool Take(std::uint64_t key){
        if(!Available())return false;
        for(unsigned i=0;i<count;++i)if(keys[i]==key)return false;
        keys[count++]=key;return true;
    }
};
template<class H>auto Observe(H& h,const Hit& value,int)->decltype(h.TraceKillHit(value),void()){h.TraceKillHit(value);}
template<class H>void Observe(H&,const Hit&,long){}
}
