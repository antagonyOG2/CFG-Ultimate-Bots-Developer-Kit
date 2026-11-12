#pragma once
#include "ActiveMenuObservation.hpp"
#include "MenuStackParent.generated.hpp"
namespace cfgmenu::parentcapture {
struct Record {
    std::uintptr_t menuClass=0,widget=0;int index=-1;
    bool operator==(const Record& b)const{return menuClass==b.menuClass&&widget==b.widget&&index==b.index;}
};
// This is the native stack's retained class, not a surviving widget lease.
// Diagnostic only: nullable Widget never approves creation, loading or calls.
template<class Reader> bool Observe(Reader& r,const active::Context& c,int index,Record& out){
    out={};if(!c.array.Sane()||index<0||index>=c.array.count-1)return false;
    active::StackArray header;Record a,b;a.index=b.index=index;
    const auto at=c.array.data+static_cast<std::uintptr_t>(index)*execdisk::RecordBytes;
    if(!r.Read(c.stack+execdisk::StackArray,header)||!(header==c.array)||
       !r.Read(at+parentdisk::RecordClass,a.menuClass)||!cfgprobe::ObjectPointer(a.menuClass)||
       !r.Read(at+execdisk::RecordWidget,a.widget)||(a.widget&&!cfgprobe::ObjectPointer(a.widget))||
       !r.Read(at+parentdisk::RecordClass,b.menuClass)||!r.Read(at+execdisk::RecordWidget,b.widget)||!(a==b)||
       !r.Read(c.stack+execdisk::StackArray,header)||!(header==c.array))return false;
    out=b;return true;
}
template<class Reader> bool Current(Reader& r,const active::Context& c,const Record& record){
    Record fresh;return Observe(r,c,record.index,fresh)&&fresh==record;
}
template<class Reader> bool NativeCode(Reader& r,std::uintptr_t module){
    std::array<unsigned char,parentdisk::PreviousCode.size()> executor{};
    std::array<unsigned char,parentdisk::PreviousLeaf.size()> helper{};
    return r.ReadBytes(module+parentdisk::PreviousExecutor,executor.data(),executor.size())&&executor==parentdisk::PreviousCode&&
        r.ReadBytes(module+parentdisk::PreviousHelper,helper.data(),helper.size())&&helper==parentdisk::PreviousLeaf;
}
}
