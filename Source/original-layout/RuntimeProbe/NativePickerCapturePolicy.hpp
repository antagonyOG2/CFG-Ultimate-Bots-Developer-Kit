#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>
namespace cfgmenu::pickercapture {
enum class Kind { Counselor,Jason };
constexpr unsigned MaximumFields=192,MaximumFunctions=6,MaximumRowBytes=512;
inline bool Screen(Kind kind,std::u16string_view name){return name==(kind==Kind::Counselor?u"Counselor_Menu_C":u"Jason_Select_Widget_C");}
inline bool Function(std::u16string_view name){
    constexpr std::u16string_view graph=u"ExecuteUbergraph_";
    return name==u"Construct"||name==u"OnClick_Accept"||name==u"OfflineBots_AcceptJason"||name==u"GetPreviousMenu"||
        name==u"OnPlayTransitionOut"||name.substr(0,graph.size())==graph;
}
inline bool SelectionPointer(std::u16string_view name){
    for(const auto part:{u"Character",u"Counselor",u"Jason",u"Killer",u"PlayerState",u"SaveGame"})
        if(name.find(part)!=std::u16string_view::npos)return true;
    return false;
}
inline bool Extent(std::uintptr_t address,std::size_t bytes){return address>=65536&&bytes&&address<=UINTPTR_MAX-bytes;}
template<class Reader> bool Index(Reader& r,std::uintptr_t widget,int offset,int storage,std::int32_t& out){
    out=-1;std::int32_t first=-1,second=-1;
    if(offset<0||offset%4||storage<4||offset>storage-4||!Extent(widget,static_cast<unsigned>(offset)+4)||
       !r.Read(widget+static_cast<unsigned>(offset),first)||first<0||first>=64||
       !r.Read(widget+static_cast<unsigned>(offset),second)||first!=second)return false;
    out=first;return true;
}
struct Array {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const Array& other)const{return data==other.data&&count==other.count&&capacity==other.capacity;}};
inline bool ArrayBounds(const Array& header,int elementBytes,int selected){
    return elementBytes>0&&elementBytes<=static_cast<int>(MaximumRowBytes)&&header.count>0&&header.count<=64&&
        header.capacity>=header.count&&header.capacity<=128&&selected>=0&&selected<header.count&&
        Extent(header.data,static_cast<std::size_t>(header.count)*static_cast<unsigned>(elementBytes));
}
// Only the selected used row is sampled. No spare-capacity or neighbouring
// memory reads; opaque bytes do not grant soft-class decoding/load authority.
template<class Reader> bool SelectedRow(Reader& r,std::uintptr_t at,int bytes,int selected,Array& header,
    std::array<unsigned char,MaximumRowBytes>& out){
    header={};out={};Array first,second;std::array<unsigned char,MaximumRowBytes> one{},two{};
    if(!r.Read(at,first)||!ArrayBounds(first,bytes,selected)||
       !r.ReadBytes(first.data+static_cast<std::uintptr_t>(selected)*static_cast<unsigned>(bytes),one.data(),bytes)||
       !r.ReadBytes(first.data+static_cast<std::uintptr_t>(selected)*static_cast<unsigned>(bytes),two.data(),bytes)||
       one!=two||!r.Read(at,second)||!(first==second))return false;
    header=first;out=one;return true;
}
}
