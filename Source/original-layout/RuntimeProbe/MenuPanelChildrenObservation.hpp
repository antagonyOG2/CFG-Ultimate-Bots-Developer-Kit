#pragma once
#include "ClassIdentity.hpp"
#include <array>
namespace cfgmenu::panelcapture {
constexpr unsigned ChildLimit=64;
struct Array {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const Array& b)const noexcept{return data==b.data&&count==b.count&&capacity==b.capacity;}
    bool Valid()const noexcept{return count>0&&count<=static_cast<int>(ChildLimit)&&capacity>=count&&capacity<=1048576&&cfgprobe::ObjectPointer(data);}
};
struct Child {std::uintptr_t slot=0,widget=0,type=0;bool operator==(const Child& b)const noexcept{return slot==b.slot&&widget==b.widget&&type==b.type;}};
struct Snapshot {Array slots{};std::array<Child,ChildLimit> children{};};
// Diagnostic only: offsets must come from reflected named Slots/Parent/Content.
// Read Num elements, never spare capacity. No native call or global discovery.
template<class Reader> bool Observe(Reader& r,std::uintptr_t panel,unsigned slotsOffset,unsigned parentOffset,unsigned contentOffset,Snapshot& out){
    out={};Snapshot a;
    if(!cfgprobe::ObjectPointer(panel)||slotsOffset>65520||parentOffset>65528||contentOffset>65528||
       slotsOffset%8||parentOffset%8||contentOffset%8||parentOffset==contentOffset||!r.Read(panel+slotsOffset,a.slots)||!a.slots.Valid())return false;
    for(int i=0;i<a.slots.count;++i){auto& child=a.children[static_cast<unsigned>(i)];std::uintptr_t parent=0;
        if(!r.Read(a.slots.data+8*static_cast<unsigned>(i),child.slot)||!cfgprobe::ObjectPointer(child.slot)||
           !r.Read(child.slot+parentOffset,parent)||parent!=panel||!r.Read(child.slot+contentOffset,child.widget)||!cfgprobe::ObjectPointer(child.widget)||
           !r.Read(child.widget+cfgprobe::ObjectClass,child.type)||!cfgprobe::ObjectPointer(child.type))return false;
        for(int j=0;j<i;++j)if(a.children[static_cast<unsigned>(j)].slot==child.slot||a.children[static_cast<unsigned>(j)].widget==child.widget)return false;
    }
    Array again;if(!r.Read(panel+slotsOffset,again)||!(again==a.slots))return false;
    for(int i=0;i<a.slots.count;++i){Child fresh;std::uintptr_t parent=0;
        if(!r.Read(a.slots.data+8*static_cast<unsigned>(i),fresh.slot)||!r.Read(fresh.slot+parentOffset,parent)||parent!=panel||
           !r.Read(fresh.slot+contentOffset,fresh.widget)||!r.Read(fresh.widget+cfgprobe::ObjectClass,fresh.type)||!(fresh==a.children[static_cast<unsigned>(i)]))return false;
    }
    if(!r.Read(panel+slotsOffset,again)||!(again==a.slots))return false;
    out=a;return true;
}
}
