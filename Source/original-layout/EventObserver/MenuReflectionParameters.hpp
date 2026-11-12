#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
namespace cfgmenu::reflection {
// Packet construction only: no engine calls or permission to execute a binding.
// Native layouts independently extracted in CFG-Menu-DiskBindings.json.
inline bool Pointer(std::uintptr_t p) { return p>=0x0/*CFG3.5 address withheld*/ && p<=0x00007FFFFFFFFFFF && !(p&7); }
struct CreateParameters { std::uintptr_t world=0,type=0,player=0,result=0; };
struct AddParameters { std::uintptr_t content=0,result=0; };
static_assert(sizeof(CreateParameters)==32 && offsetof(CreateParameters,result)==24);
static_assert(sizeof(AddParameters)==16 && offsetof(AddParameters,result)==8);
inline bool Create(std::uintptr_t world,std::uintptr_t type,std::uintptr_t player,CreateParameters& out) {
    out={}; if(!Pointer(world)||!Pointer(type)||!Pointer(player))return false;
    out={world,type,player,0};return true;
}
inline bool Add(std::uintptr_t content,AddParameters& out) {
    out={};if(!Pointer(content))return false;out.content=content;return true;
}
struct String { const char16_t* data=nullptr;std::int32_t count=0,capacity=0; };
struct Array { const String* data=nullptr;std::int32_t count=0,capacity=0; };
static_assert(sizeof(String)==16 && sizeof(Array)==16 && sizeof(char16_t)==2);
enum class Kind { ArrayOfString,Int32 };
struct Field { std::string_view name;Kind kind;unsigned offset=0,size=0,dimension=0;bool input=false; };
struct OptionsSchema { unsigned frameBytes=0;std::array<Field,2> fields{}; };
// Provider must first establish current UFunction identity/owner and decode its
// property names, kinds, flags and offsets. These are NOT stock Blueprint offsets.
inline bool OptionsOffsets(const OptionsSchema& s,unsigned& items,unsigned& selected) {
    items=selected=0;if(!s.frameBytes||s.frameBytes>512)return false;
    bool haveItems=false,haveSelected=false;unsigned a=0,b=0;
    for(const auto& f:s.fields){
        if(!f.input||f.dimension!=1||f.offset>s.frameBytes||f.size>s.frameBytes-f.offset)return false;
        if(f.name=="Items"&&f.kind==Kind::ArrayOfString&&f.size==16&&!haveItems){a=f.offset;haveItems=true;}
        else if(f.name=="SelectedIndex"&&f.kind==Kind::Int32&&f.size==4&&!haveSelected){b=f.offset;haveSelected=true;}
        else return false;
    }
    if(!haveItems||!haveSelected||a%8||b%4||!(a+16<=b||b+4<=a))return false;
    items=a;selected=b;return true;
}
class OptionsPacket {
    // Stable own storage. Native SetOptions must be proven to copy its borrowed
    // FString inputs synchronously; never install these as engine-owned arrays.
    std::array<std::array<char16_t,128>,32> text_{};
    std::array<String,32> strings_{};
    alignas(16) std::array<unsigned char,512> frame_{};
    unsigned bytes_=0;
public:
    OptionsPacket()=default;
    OptionsPacket(const OptionsPacket&)=delete;
    OptionsPacket& operator=(const OptionsPacket&)=delete;
    OptionsPacket(OptionsPacket&&)=delete;
    OptionsPacket& operator=(OptionsPacket&&)=delete;
    bool Build(const OptionsSchema& schema,const std::u16string_view* labels,unsigned count,unsigned selected) {
        bytes_=0;frame_={};strings_={};text_={};unsigned items=0,index=0;
        if(!OptionsOffsets(schema,items,index)||!labels||!count||count>text_.size()||selected>=count)return false;
        for(unsigned i=0;i<count;++i){
            if(labels[i].empty()||labels[i].size()>=text_[i].size()||labels[i].find(char16_t{0})!=std::u16string_view::npos)return false;
        }
        for(unsigned i=0;i<count;++i){
            std::memcpy(text_[i].data(),labels[i].data(),labels[i].size()*sizeof(char16_t));
            const auto n=static_cast<std::int32_t>(labels[i].size()+1);
            strings_[i]={text_[i].data(),n,n};
        }
        const Array array{strings_.data(),static_cast<std::int32_t>(count),static_cast<std::int32_t>(count)};
        const auto chosen=static_cast<std::int32_t>(selected);
        std::memcpy(frame_.data()+items,&array,sizeof(array));std::memcpy(frame_.data()+index,&chosen,sizeof(chosen));
        bytes_=schema.frameBytes;return true;
    }
    void* Data(){return bytes_?frame_.data():nullptr;}
    unsigned Size()const{return bytes_;}
};
// FText is deliberately unsupported here. A visible adapter needs separately
// verified native conversion/copy/destruction (or retained owned lifetime).
} // namespace cfgmenu::reflection
