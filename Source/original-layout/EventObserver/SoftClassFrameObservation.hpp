#pragma once
#include "BundleFrameInputObservation.hpp"
#include "../Core/CounselorLoadPolicy.hpp"
#include "../RuntimeProbe/CounselorLoadBindings.generated.hpp"
#include "../RuntimeProbe/PrimaryJasonBindings.generated.hpp"
namespace cfgsoftframe {
template<class R,class T>bool Same(R& r,std::uintptr_t at,const T& expected){T fresh{};return r.Read(at,fresh)&&fresh==expected;}
enum class Kind {None,Conversion,Preload};
struct Result {Kind kind=Kind::None;std::array<std::uintptr_t,6> roots{};unsigned outputs=0;};
// Copied observations only, never a root, cached argument or admission capability.
struct Diagnostic {
    unsigned stage=0,mask=0,index=0,reads=0;std::uint8_t count=0;std::uint16_t bytes=0;
    std::uintptr_t property=0;int dimension=0,elementBytes=0,offset=0;std::uint64_t flags=0;
    cfgnames::Text name{},type{};
};
template<class R>bool Schema(R& r,std::uintptr_t module,std::uintptr_t function,std::uintptr_t owner,Kind kind,Diagnostic& d){
    namespace layout=cfgcompat::cfg35::softClassFrames;
    cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field;
    std::uint16_t bytes=0;std::uint8_t count=0;std::uintptr_t head=0;
    const bool load=kind==Kind::Preload;
    d.stage=20;if(!reflection.ReadField(function,owner,field))return false;
    d.name=field.label;d.type=field.kind;
    d.stage=21;if(field.kind.View()!=u"Function"||field.label.View()!=(load?u"LoadAssetClass":u"Conv_SoftClassReferenceToClass"))return false;
    d.stage=22;if(!r.Read(function+layout::ParameterCount,count))return false;d.count=count;
    d.stage=23;if(!r.Read(function+layout::ParameterBytes,bytes))return false;d.bytes=bytes;
    d.stage=24;if(count!=(load?4:2)||bytes!=(load?88:48))return false;
    d.stage=25;if(!r.Read(function+cfgreflection::disk::Children,head))return false;
    unsigned mask=0;std::array<std::uintptr_t,8> seen{};
    for(unsigned i=0;head&&i<seen.size();++i){
        d.index=i;d.property=head;d.stage=30;
        for(unsigned j=0;j<i;++j)if(seen[j]==head)return false;seen[i]=head;
        cfgreflection::Field property;struct Shape{int dimension,bytes;std::uint64_t flags;};Shape shape{};int offset=0;
        d.stage=31;if(!reflection.ReadField(head,function,property))return false;
        d.name=property.label;d.type=property.kind;
        d.stage=32;if(!r.Read(head+cfgreflection::disk::PropertyDimensions,shape))return false;
        d.dimension=shape.dimension;d.elementBytes=shape.bytes;d.flags=shape.flags;
        d.stage=33;if(!r.Read(head+cfgreflection::disk::PropertyOffset,offset))return false;d.offset=offset;
        d.stage=34;if(shape.dimension!=1||(shape.flags&0x80u)==0)return false;
        unsigned bit=0;const auto name=property.label.View(),type=property.kind.View();
        if(load){
            if(name==u"WorldContextObject"&&type==u"ObjectProperty"&&offset==0&&shape.bytes==8&&shape.flags==layout::RuntimePreloadWorldFlags)bit=1;
            if(name==u"AssetClass"&&type==u"SoftClassProperty"&&offset==8&&shape.bytes==40&&shape.flags==layout::RuntimePreloadClassFlags)bit=2;
            if(name==u"OnLoaded"&&type==u"DelegateProperty"&&offset==48&&shape.bytes==16&&shape.flags==layout::RuntimePreloadDelegateFlags)bit=4;
            if(name==u"LatentInfo"&&type==u"StructProperty"&&offset==64&&shape.bytes==24&&shape.flags==layout::RuntimePreloadLatentFlags)bit=8;
        }else{
            if(name==u"SoftClass"&&type==u"SoftClassProperty"&&offset==0&&shape.bytes==40&&shape.flags==layout::RuntimeConversionSourceFlags)bit=1;
            if(name==u"ReturnValue"&&type==u"ClassProperty"&&offset==40&&shape.bytes==8&&shape.flags==layout::RuntimeConversionResultFlags)bit=2;
        }
        d.stage=35;if(!bit||(mask&bit))return false;mask|=bit;d.mask=mask;
        Shape again{};int finalOffset=0;
        d.stage=36;if(!r.Read(head+cfgreflection::disk::PropertyDimensions,again)||again.dimension!=shape.dimension||again.bytes!=shape.bytes||again.flags!=shape.flags||
           !r.Read(head+cfgreflection::disk::PropertyOffset,finalOffset)||offset!=finalOffset)return false;
        head=property.next;
    }
    d.stage=37;return !head&&mask==(load?15u:3u)&&Same(r,function+layout::ParameterCount,count)&&Same(r,function+layout::ParameterBytes,bytes);
}
template<class R>bool Observe(R& r,std::uintptr_t module,std::uintptr_t actor,std::uintptr_t function,void* parameters,
    std::uintptr_t defaults,std::uintptr_t converter,std::uintptr_t world,std::uintptr_t controller,Result& out,Diagnostic* diagnostic=nullptr){
    Diagnostic local;auto& d=diagnostic?*diagnostic:local;d={};
    out={};std::uintptr_t cached=0,owner=0,ctor=0;
    d.stage=1;if(!r.Read(module+cfgprimary::disk::Conversion.cache,cached))return false;
    if(function&&function==cached&&function==converter)out.kind=Kind::Conversion;
    else{
        d.stage=2;if(!r.Read(module+cfgcounselorload::disk::LoadClass.cache,cached))return false;
        if(function&&function==cached)out.kind=Kind::Preload;else return true;
    }
    d.stage=3;if(!parameters||actor!=defaults)return false;
    d.stage=4;if(!r.Read(module+cfgprimary::disk::KismetReflection,owner)||!r.Read(module+cfgprimary::disk::KismetConstructor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor))return false;
    d.stage=5;if(!Same(r,owner+cfgspawn::ClassDefaultObject,defaults)||!Same(r,defaults+cfgprobe::ObjectClass,owner))return false;
    if(!Schema(r,module,function,owner,out.kind,d))return false;
    cfgprimary::ConversionParameters checked;
    if(out.kind==Kind::Conversion){
        d.stage=40;
        const auto& packet=*static_cast<const cfgprimary::ConversionParameters*>(parameters);
        if(packet.result||!cfgprimary::BuildConversion(packet.softClass,checked))return false;
        out.outputs=1;
    }else{
        d.stage=41;
        const auto& packet=*static_cast<const cfgcounselorload::Packet*>(parameters);
        if(packet.world!=world||packet.latent.target!=controller||packet.latent.linkage!=-1||packet.latent.function||
           packet.latent.uuid<0x43464700||packet.latent.uuid>=0x43464740||packet.onLoaded.index!=-1||packet.onLoaded.serial||
           packet.onLoaded.function||!cfgprimary::BuildConversion(packet.source,checked))return false;
        out.roots[4]=world;out.roots[5]=controller;
    }
    out.roots[0]=actor;out.roots[1]=function;out.roots[2]=owner;out.roots[3]=defaults;
    d.stage=42;if(!Same(r,module+(out.kind==Kind::Conversion?cfgprimary::disk::Conversion.cache:cfgcounselorload::disk::LoadClass.cache),function))return false;
    d.stage=0;return true;
}
}
