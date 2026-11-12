#pragma once
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
namespace cfgdeathend {
// CFG native bool assignment: +71 byte displacement,+72 set mask,+73 clear
// mask, then read/modify/write at that displacement. Pinned disk evidence;
// never call this helper or infer the layout from the base game's addresses.
constexpr std::uintptr_t BoolAssignProof=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,39> BoolAssignCode{} /*CFG3.5 code signature withheld*/;
// Only after proved primary Jason death; optional native Blueprint outro rule.
// Finite cold metadata traversal,6 fields per slice. No object/name census.
struct OutroProperty {
    unsigned stage=0,seen=0,owners=0,matches=0;std::uintptr_t root=0,owner=0,head=0,cursor=0;
    std::array<std::uintptr_t,2048> visited{};std::array<std::uintptr_t,32> parents{};
    cfgreflection::Property property{};cfgreflection::Field match{};bool opened=false;
    template<class R> bool Warm(R& r,std::uintptr_t module,std::uintptr_t object){
        if(stage)return true;std::uintptr_t type=0;
        if(!r.Read(object+cfgprobe::ObjectClass,type)||!type||(root&&root!=type)){stage=2;return true;}
        if(!root)root=owner=type;cfgreflection::Observation<R> reflection(r,module);
        if(!opened){
            if(owners==parents.size()){stage=2;return true;}
            for(unsigned i=0;i<owners;++i)if(parents[i]==owner){stage=2;return true;}
            parents[owners++]=owner;if(!r.Read(owner+cfgreflection::disk::Children,head)){stage=2;return true;}
            cursor=head;opened=true;matches=0;
        }
        std::uintptr_t current=0;if(!r.Read(owner+cfgreflection::disk::Children,current)||current!=head){stage=2;return true;}
        for(unsigned i=0;i<6&&cursor;++i){
            if(seen==visited.size()){stage=2;return true;}
            for(unsigned n=0;n<seen;++n)if(visited[n]==cursor){stage=2;return true;}
            cfgreflection::Field field{};if(!reflection.ReadField(cursor,owner,field)){stage=2;return true;}
            visited[seen++]=cursor;if(field.label.View()==u"bSkipLevelOutro"){match=field;++matches;}cursor=field.next;
        }
        if(cursor)return false;
        if(matches){stage=matches==1&&reflection.ReadProperty(match,property)?1:2;return true;}
        if(!r.Read(owner+cfgprobe::ClassSuper,current)||!current){stage=2;return true;}
        owner=current;opened=false;return false;
    }
    template<class R> bool Value(R& r,std::uintptr_t module,std::uintptr_t object,std::uintptr_t& address,std::uint8_t& mask,std::uint8_t& value){
        std::array<unsigned char,39> code{};
        if(stage!=1||!r.Read(module+BoolAssignProof,code)||code!=BoolAssignCode||!cfgspawn::ObjectIs(r,object,property.field.owner))return false;
        cfgreflection::Observation<R> reflection(r,module);cfgreflection::Property fresh{};std::array<std::uint8_t,4> layout{},again{};
        if(!reflection.ReadProperty(property.field,fresh)||fresh.offset!=property.offset||fresh.flags!=property.flags||
            fresh.field.kind.View()!=u"BoolProperty"||fresh.dimension!=1||fresh.elementBytes!=1||
            !r.Read(fresh.field.object+0x70,layout)||layout[0]!=1||layout[1]!=0||!layout[2]||
            (layout[2]&(layout[2]-1))||(layout[3]!=0xff&&layout[3]!=layout[2])||
            !r.Read(fresh.field.object+0x70,again)||layout!=again)return false;
        address=object+static_cast<unsigned>(fresh.offset);mask=layout[2];return r.Read(address,value);
    }
};
}
