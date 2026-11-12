#pragma once
#include "JasonPickerObservation.hpp"
#include "NativeFrontendCatalogues.hpp"
namespace cfgmenu::pickerlist {
struct Snapshot {std::uintptr_t widget=0,type=0;catalogues::CharacterList classes{};};
// Diagnostic only. Walk used rows, never capacity. The list's native96-byte
// user struct starts with a40-byte soft class, independently checked by name.
template<class R>bool Observe(R& r,const active::Context& c,Snapshot& out,unsigned& stage){
    out={};stage=1;if(!active::Current(r,c,c.thread))return false;
    cfgnames::Observation<R> names(r,c.module);cfgnames::Text name;
    if(!names.Object(c.type,name)||name.View()!=u"Jason_Select_Widget_C")return false;
    cfgreflection::Observation<R> reflection(r,c.module);cfgreflection::Field field,innerField,memberField;
    cfgreflection::Property array,inner,member;std::uintptr_t innerAt=0,structure=0;
    stage=2;if(!reflection.Find(c.type,u"JasonClassList",field)||!capture::RawProperty(r,field,array)||
        field.kind.View()!=u"ArrayProperty"||array.offset!=0x678||array.dimension!=1||array.elementBytes!=16||
        !r.Read(field.object+0x70,innerAt)||!reflection.ReadField(innerAt,field.object,innerField)||
        innerField.kind.View()!=u"StructProperty"||!capture::RawProperty(r,innerField,inner)||inner.offset||
        inner.dimension!=1||inner.elementBytes!=96||!r.Read(innerAt+0x70,structure)||
        !names.Object(structure,name)||name.View()!=u"FSCCharacterSelectData")return false;
    stage=3;if(!reflection.Find(structure,u"CharacterClass_10_29392D434148412C26DBACA4ECDABFEC",memberField)||
        memberField.owner!=structure||memberField.kind.View()!=u"SoftClassProperty"||!capture::RawProperty(r,memberField,member)||
        member.offset||member.dimension!=1||member.elementBytes!=40)return false;
    Snapshot a,b;a.widget=b.widget=c.widget;a.type=b.type=c.type;
    const auto sample=[&](Snapshot& value){
        auto& list=value.classes;
        if(!r.Read(c.widget+0x678,list.header)||!list.header.Sane(catalogues::MaximumCharacters)||
            (list.header.count&&!cfgprobe::lifetime::Extent(list.header.data,static_cast<std::uintptr_t>(list.header.count)*96)))return false;
        for(unsigned i=0;i<static_cast<unsigned>(list.header.count);++i){
            if(!r.Read(list.header.data+96*i,list.entries[i]))return false;
            // Never export a borrowed subobject FString pointer as a path.
            for(unsigned j=24;j<40;++j)if(list.entries[i][j])return false;
        }return true;
    };
    stage=4;if(!sample(a)||!sample(b)||!(a.classes==b.classes))return false;
    stage=5;if(!active::Current(r,c,c.thread)||!names.Object(c.type,name)||name.View()!=u"Jason_Select_Widget_C")return false;
    stage=0;out=b;return true;
}
}
