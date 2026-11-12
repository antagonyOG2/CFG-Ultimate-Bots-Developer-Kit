#pragma once
#include "GameplayLimits.hpp"
#include "NativeSpawnParameters.hpp"
#include <array>
namespace cfgbundle {
struct RandomCounselors {
    std::array<unsigned,64> order{};unsigned count=0,cursor=0;std::uint32_t random=1;
    std::array<std::uintptr_t,cfglimits::MaximumHelpers> classes{};
    std::array<cfgspawn::Vec3,cfglimits::MaximumHelpers> locations{};unsigned created=0;
    unsigned Next(){random^=random<<13;random^=random>>17;random^=random<<5;return random;}
    bool Begin(unsigned size,std::uint32_t seed){if(count||!size||size>order.size())return false;
        count=size;random=seed?seed:1;for(unsigned i=0;i<count;++i)order[i]=i;
        for(unsigned i=count-1;i;--i){const auto j=Next()%(i+1);std::swap(order[i],order[j]);}return true;}
    void NextSelection(){cursor=0;if(count>1)for(unsigned i=count-1;i;--i){const auto j=Next()%(i+1);std::swap(order[i],order[j]);}}
    // Characters already in the match from the native Offline Bots cohort
    // (user: two copies of the same counselor). F3 picks avoid them like
    // their own earlier picks; the resident fallback still allows reuse only
    // when every ordinary character is taken.
    std::array<std::uintptr_t,cfglimits::MaximumHelpers> reserved{};unsigned reservedCount=0;
    void Reserve(std::uintptr_t type){
        if(!type||Used(type)||reservedCount==reserved.size())return;
        reserved[reservedCount++]=type;}
    bool ClassAllowed(std::uintptr_t type,std::uintptr_t human)const{
        return type&&type!=human&&!Used(type);}
    bool Used(std::uintptr_t type)const{
        for(unsigned i=0;i<created;++i)if(classes[i]==type)return true;
        for(unsigned i=0;i<reservedCount;++i)if(reserved[i]==type)return true;return false;}
    bool LocationAllowed(cfgspawn::Vec3 point,cfgspawn::Vec3 human)const{
        if(!cfgspawn::Finite(point)||!cfgspawn::Finite(human))return false;
        const auto separated=[](cfgspawn::Vec3 a,cfgspawn::Vec3 b,float distance){const auto dx=a.x-b.x,dy=a.y-b.y;return dx*dx+dy*dy>=distance*distance;};
        if(!separated(point,human,2000))return false;
        for(unsigned i=0;i<created;++i)if(!separated(point,locations[i],1500))return false;return true;}
    bool Confirm(std::uintptr_t type,cfgspawn::Vec3 point){
        if(!type||!cfgspawn::Finite(point)||created==classes.size())return false;
        classes[created]=type;locations[created]=point;++created;return true;}
};
}
