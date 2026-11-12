#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
#include <array>
#include <string_view>
namespace cfgselectedjason {
constexpr unsigned NoCatalogueIndex=0xFFFFFFFFu;
struct Choice {
    cfgprobe::lifetime::SlotObservation identity{},defaultsIdentity{};
    std::array<char16_t,512> path{};unsigned length=0;
    bool Empty()const noexcept{return !identity.object&&!defaultsIdentity.object&&!length;}
    std::u16string_view Path()const noexcept{return {path.data(),length<path.size()?length:0};}
    bool Valid()const noexcept{
        constexpr std::u16string_view prefix=u"/Game/Characters/Killers/";
        if(!identity.HasAssignedSerial()||!cfgprobe::ObjectPointer(defaultsIdentity.object)||defaultsIdentity.index<0||
            defaultsIdentity.serial<0||defaultsIdentity.object==identity.object||!length||length>=path.size()||path[length]||
            Path().substr(0,prefix.size())!=prefix)return false;
        for(unsigned i=0;i<length;++i)if(!path[i])return false;
        return true;
    }
    bool operator==(const Choice& b)const noexcept{return identity==b.identity&&defaultsIdentity==b.defaultsIdentity&&length==b.length&&path==b.path;}
};
}
