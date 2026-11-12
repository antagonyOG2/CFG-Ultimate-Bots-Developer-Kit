#pragma once
#include "CounselorConvergencePolicy.hpp"
namespace cfgconvergence {
// One pair group per owner callback. Read back each mutation immediately,
// before the native behavior tree gets another gameplay slice. Optional names
// absent from a validated asset are skipped; live policy changes later are not
// schema failures. Scope/schema/context guards remain the host's responsibility.
class Publication {
    unsigned next_=0,index_=0,mask_=0;
    std::uintptr_t jason_=0,weapon_=0;
    Action action_=Action::None;
public:
    enum class Result {Pending,Complete,Refused};
    unsigned failureKey=KeyCount;
    std::uintptr_t expectedObject=0,actualObject=0;
    unsigned expectedBool=0,actualBool=0;
    bool Active()const{return action_!=Action::None;}unsigned Index()const{return index_;}Action CurrentAction()const{return action_;}
    bool Begin(unsigned index,Action action,unsigned mask,std::uintptr_t jason,std::uintptr_t weapon){
        if(Active()||index>=MaximumBots||action==Action::None||!jason||(action==Action::Publish&&!weapon))return false;
        index_=index;action_=action;mask_=mask;next_=0;jason_=jason;weapon_=weapon;
        failureKey=KeyCount;expectedObject=actualObject=0;expectedBool=actualBool=0;return true;
    }
    void Reset(){*this={};}
    template<class Host> Result Step(Host& host){
        if(!Active()||!host.Current())return Result::Refused;
        if(!next_){
            if(action_==Action::Publish){
                for(unsigned key=Weapon;key<=Jason;++key){
                    if(key==Weapon&&!(mask_&(1u<<Weapon)))continue;
                    failureKey=key;expectedObject=key==Weapon?weapon_:jason_;actualObject=0;
                    if(!(mask_&(1u<<key))||!host.SetObject(key,expectedObject)||!host.GetObject(key,actualObject)||actualObject!=expectedObject)return Result::Refused;
                }
            }
            next_=Fight;failureKey=KeyCount;return Result::Pending;
        }
        unsigned pairs=0;
        for(;next_<KeyCount&&pairs<2;++next_){
            if(!(mask_&(1u<<next_))||(action_==Action::Clear&&(next_==Hide||next_==Orient)))continue;
            ++pairs;failureKey=next_;expectedBool=Desired(static_cast<Key>(next_),action_)?1u:0u;actualBool=2;
            if(!host.SetBool(next_,expectedBool)||!host.GetBool(next_,actualBool)||actualBool!=expectedBool)return Result::Refused;
        }
        failureKey=KeyCount;
        while(next_<KeyCount&&(!(mask_&(1u<<next_))||(action_==Action::Clear&&(next_==Hide||next_==Orient))))++next_;
        return next_==KeyCount?Result::Complete:Result::Pending;
    }
};
}
