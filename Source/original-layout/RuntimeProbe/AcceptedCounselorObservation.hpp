#pragma once
#include "PickerPrecreationObservation.hpp"
#include "NativePickerCapturePolicy.hpp"
#include "StructPropertyBindings.hpp"
#include "../Core/SoftClassPathIdentity.hpp"
#include "../Core/SetupMenuGameplayHandoff.hpp"
namespace cfgmenu::acceptedcounselor {
constexpr unsigned ReadLimit=4096;
struct Choice {cfgprimary::SoftClassBytes path{};cfgnames::Text name{};int index=-1;};
template<class R> bool Observe(R& r,const active::Context& actual,const pickerprepare::Prepared& prepared,Choice& out,unsigned* stage=nullptr){
    out={};if(stage)*stage=1;
    // Called only AFTER existing actual-receiver/frame/executor Guard proves
    // outer native Accept, BEFORE original forwarding. No native calls/load.
    if(prepared.kind!=pickerprepare::Kind::Counselor||actual.type!=prepared.type||!active::Current(r,actual,actual.thread))return false;
    cfgreflection::Observation<R> reflection(r,actual.module);cfgreflection::Field field;cfgreflection::Property index,roster,inner;
    for(const auto* saved:{&prepared.index,&prepared.roster}){
        cfgreflection::Property p;if(!reflection.ReadField(saved->field.object,actual.type,field)||
            !capture::RawProperty(r,field,p)||!pendingmember::SameSchema(p,*saved))return false;
        if(saved==&prepared.index)index=p;else roster=p;
    }
    std::uintptr_t innerAt=0,type=0,table=0,head=0;int bytes=0;
    if(stage)*stage=2;
    if(!r.Read(roster.field.object+cfgreflection::arraydisk::Inner,innerAt)||
       !reflection.ReadField(innerAt,roster.field.object,field)||field.kind.View()!=u"StructProperty"||!capture::RawProperty(r,field,inner)||
       inner.dimension!=1||inner.elementBytes!=96||inner.offset!=0||
       !r.Read(innerAt,table)||table!=actual.module+cfgreflection::structdisk::Vtable||
       !r.Read(innerAt+cfgreflection::structdisk::Type,type))return false;
    cfgnames::Observation<R> names(r,actual.module);cfgnames::Text name;cfgprobe::Ancestry ancestry;bool script=false;
    if(!names.Object(type,name)||name.View()!=u"FSCCharacterSelectData"||!cfgprobe::CaptureAncestry(r,type,ancestry))return false;
    for(unsigned i=0;i<ancestry.count;++i){if(!names.Object(ancestry.classes[i],name))return false;script|=name.View()==u"ScriptStruct";}
    if(!script||!cfgprobe::StableAncestry(r,type,ancestry)||!r.Read(type+cfgreflection::disk::StorageBytes,bytes)||bytes!=96||
       !r.Read(type+cfgreflection::disk::Children,head))return false;
    cfgreflection::Property character,transform;
    if(stage)*stage=3;
    if(!reflection.ReadField(head,type,field)||field.label.View()!=u"CharacterClass_10_29392D434148412C26DBACA4ECDABFEC"||
       field.kind.View()!=u"SoftClassProperty"||!capture::RawProperty(r,field,character)||character.dimension!=1||character.elementBytes!=40||character.offset||character.flags!=0x0/*CFG3.5 address withheld*/00000805ULL||
       !reflection.ReadField(field.next,type,field)||field.label.View()!=u"MenuTransform_9_E53DDB35467D99E1BA3E72AB59737E49"||
       field.kind.View()!=u"StructProperty"||field.next||!capture::RawProperty(r,field,transform)||
       transform.dimension!=1||transform.elementBytes!=48||transform.offset!=48||transform.flags!=0x0000001040000005ULL)return false;
    Choice choice;std::int32_t selected=-1,again=-1;pickercapture::Array array;std::array<unsigned char,pickercapture::MaximumRowBytes> row{};
    if(stage)*stage=4;
    if(!r.Read(actual.widget+static_cast<unsigned>(index.offset),selected)||
       !pickercapture::SelectedRow(r,actual.widget+static_cast<unsigned>(roster.offset),96,selected,array,row))return false;
    std::copy_n(row.begin(),40,choice.path.begin());cfgnames::Words path{};std::memcpy(&path,choice.path.data()+16,8);
    if(!softpath::Plain(choice.path)||!names.Decode(path,choice.name)||choice.name.View().substr(0,6)!=u"/Game/"||
       !r.Read(actual.widget+static_cast<unsigned>(index.offset),again)||again!=selected||
       !r.Read(innerAt+cfgreflection::structdisk::Type,innerAt)||innerAt!=type||
       !r.Read(type+cfgreflection::disk::Children,innerAt)||innerAt!=head||!cfgprobe::StableAncestry(r,type,ancestry)||
       !active::Current(r,actual,actual.thread))return false;
    for(const auto* saved:{&index,&roster,&inner,&character,&transform}){
        cfgreflection::Property p;if(!reflection.ReadField(saved->field.object,saved->field.owner,field)||
            !capture::RawProperty(r,field,p)||!pendingmember::Same(p,*saved))return false;
    }
    choice.index=selected;out=choice;if(stage)*stage=0;return true;
}
inline bool Apply(MenuCatalogueEvidence& e,const cfgprimary::SoftClassBytes& path){
    if(!softpath::Plain(path))return false;e.preserveNativeCounselor=false;e.counselorPathSelection=true;e.selectedCounselor=path;e.counselor=softpath::Token(path);return true;
}
}
