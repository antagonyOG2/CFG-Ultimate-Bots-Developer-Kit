#pragma once
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include <array>
#include <string_view>
namespace cfgofflinesettings {
// Native cooked Items, not localized display text or an assumed option order.
// Output uses the base menu semantic order Low / Medium / High.
template<class Reader> bool NativeSkillItems(Reader& r,std::uintptr_t items,std::uintptr_t index,unsigned& out){
    out=0;struct Array {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
        bool operator==(const Array& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
    struct Sample {Array header{};std::int32_t selected=-1;std::array<Array,3> strings{};
        std::array<std::array<char16_t,8>,3> text{};std::array<unsigned,3> semantic{};};
    const auto sample=[&](Sample& s){
        if(!r.Read(items,s.header)||!cfgprobe::ObjectPointer(s.header.data)||s.header.count!=3||s.header.capacity<3||s.header.capacity>32||
           !r.Read(index,s.selected)||s.selected<0||s.selected>2)return false;
        unsigned seen=0;
        for(unsigned i=0;i<3;++i){auto& str=s.strings[i];
            if(!r.Read(s.header.data+16*i,str)||!cfgprobe::ObjectPointer(str.data)||str.count<2||str.count>8||str.capacity<str.count||str.capacity>128||
               !r.ReadBytes(str.data,s.text[i].data(),static_cast<unsigned>(str.count)*2)||s.text[i][static_cast<unsigned>(str.count)-1])return false;
            const std::u16string_view label(s.text[i].data(),static_cast<unsigned>(str.count)-1);
            const auto semantic=(label==u"Easy"||label==u"Low")?0u:(label==u"Normal"||label==u"Medium")?1u:
                (label==u"Hard"||label==u"High")?2u:3u;
            if(semantic==3||(seen&(1u<<semantic)))return false;seen|=1u<<semantic;s.semantic[i]=semantic;
        }
        return seen==7;
    };
    Sample a,b;if(!sample(a)||!sample(b)||!(a.header==b.header)||a.selected!=b.selected||a.strings!=b.strings||a.text!=b.text||a.semantic!=b.semantic)return false;
    out=a.semantic[static_cast<unsigned>(a.selected)];return true;
}
}
