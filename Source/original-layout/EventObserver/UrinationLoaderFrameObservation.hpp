#pragma once
#include "UrinationActionObservation.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
namespace cfgurination {
struct MakePathPacket {NativeString input{};NativePath result{};};
struct LoadPacket {
    std::uintptr_t world=0;std::array<unsigned char,40> asset{};
    struct Delegate {std::int32_t index=-1,serial=0;cfgnames::Words name{};}callback;
    NativeLatent latent{};
    LoadPacket(){const std::int32_t emptyWeakIndex=-1;std::memcpy(asset.data(),&emptyWeakIndex,sizeof(emptyWeakIndex));}
};
static_assert(sizeof(MakePathPacket)==40&&offsetof(MakePathPacket,result)==16);
static_assert(sizeof(LoadPacket)==88&&offsetof(LoadPacket,callback)==48&&offsetof(LoadPacket,latent)==64);
template<class R>bool LoaderPacket(R& r,std::uintptr_t module,const LoadIdentity& expected,void* parameters,bool load){
    if(!parameters||!expected.Valid())return false;
    if(load){const auto& packet=*static_cast<const LoadPacket*>(parameters);
        if(packet.world!=expected.world||packet.callback.name.index||packet.callback.name.number||packet.callback.serial||
            packet.callback.index!=-1||packet.latent.linkage!=-1||packet.latent.uuid!=expected.uuid||
            packet.latent.controller!=expected.controller||packet.latent.execution.index||packet.latent.execution.number)return false;
        for(unsigned i=0;i<16;++i)if(packet.asset[i]!=(i<4?0xFF:0))return false;
        NativePath path{};std::memcpy(&path,packet.asset.data()+16,sizeof(path));
        cfgnames::Observation<R> names(r,module);cfgnames::Text name;
        return !path.asset.number&&path.sub.Empty()&&names.Decode(path.asset,name)&&name.View()==ExpectedAsset;
    }
    const auto& packet=*static_cast<const MakePathPacket*>(parameters);
    const auto count=static_cast<int>(ExpectedAsset.size()+1);
    if(packet.input.count!=count||packet.input.capacity!=count||!cfgprobe::ObjectPointer(packet.input.data)||
        packet.result.asset.index||packet.result.asset.number||!packet.result.sub.Empty())return false;
    std::array<char16_t,ExpectedAsset.size()+1> text{};
    if(!r.Read(packet.input.data,text)||text.back())return false;
    for(unsigned i=0;i<ExpectedAsset.size();++i)if(text[i]!=ExpectedAsset[i])return false;
    return true;
}
// Only exact named cached functions from the pinned native Kismet owner. No
// factory invocation, output adoption, root/native authority or arbitrary frame.
template<class R>bool LoaderSchema(R& r,std::uintptr_t module,std::uintptr_t defaults,
    std::uintptr_t function,bool load,std::array<std::uintptr_t,4>& roots){
    roots={};cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field{};
    const auto& binding=load?disk::LoadAsset:disk::MakeSoftObjectPath;
    std::uintptr_t owner=0,constructor=0,cached=0,cdo=0,executor=0;
    std::array<unsigned char,47> code{};
    if(!function||!defaults||!r.Read(module+binding.cache,cached)||cached!=function||
        !r.Read(module+binding.ownerReflection,owner)||!r.Read(module+binding.ownerConstructor,constructor)||
        !cfgprobe::MatchingClassCache(owner,constructor)||!r.Read(owner+cfgspawn::ClassDefaultObject,cdo)||cdo!=defaults||
        !r.Read(defaults+cfgprobe::ObjectClass,cached)||cached!=owner||!r.Read(module+binding.factory,code)||code!=binding.code||
        !r.Read(function+0xB0,executor)||executor!=module+(load?disk::LoadAssetExecutor:disk::MakePathExecutor)||
        !reflection.ReadField(function,owner,field)||field.kind.View()!=u"Function"||
        field.label.View()!=(load?u"LoadAsset":u"MakeSoftObjectPath"))return false;
    if(load){std::array<unsigned char,disk::LoadAssetExecutorCode.size()> body{};
        if(!r.Read(executor,body)||body!=disk::LoadAssetExecutorCode)return false;}
    else{std::array<unsigned char,disk::MakePathExecutorCode.size()> body{};
        if(!r.Read(executor,body)||body!=disk::MakePathExecutorCode)return false;}
    std::uint8_t count=0;std::uint16_t bytes=0;std::uintptr_t head=0,initialHead=0;
    if(!r.Read(function+0x8C,count)||count!=(load?4:2)||!r.Read(function+0x8E,bytes)||bytes!=(load?88:40)||
        !r.Read(function+cfgreflection::disk::Children,head))return false;
    initialHead=head;unsigned mask=0;std::array<std::uintptr_t,4> seen{};
    struct Shape{int dimension,bytes;std::uint64_t flags;};
    for(unsigned i=0;head&&i<count;++i){
        for(unsigned j=0;j<i;++j)if(seen[j]==head)return false;seen[i]=head;
        cfgreflection::Field property{};Shape shape{},again{};int offset=0,finalOffset=0;
        if(!reflection.ReadField(head,function,property)||!r.Read(head+cfgreflection::disk::PropertyDimensions,shape)||shape.dimension!=1||
            !r.Read(head+cfgreflection::disk::PropertyOffset,offset))return false;
        unsigned bit=0;const auto name=property.label.View(),kind=property.kind.View();
        if(load){
            if(name==u"WorldContextObject"&&kind==u"ObjectProperty"&&offset==0&&shape.bytes==8&&shape.flags==disk::RuntimeLoadWorldFlags)bit=1;
            if(name==u"Asset"&&kind==u"SoftObjectProperty"&&offset==8&&shape.bytes==40&&shape.flags==disk::RuntimeLoadAssetFlags)bit=2;
            if(name==u"OnLoaded"&&kind==u"DelegateProperty"&&offset==48&&shape.bytes==16&&shape.flags==disk::RuntimeLoadDelegateFlags)bit=4;
            if(name==u"LatentInfo"&&kind==u"StructProperty"&&offset==64&&shape.bytes==24&&shape.flags==disk::RuntimeLoadLatentFlags)bit=8;
        }else{
            if(name==u"PathString"&&kind==u"StrProperty"&&offset==0&&shape.bytes==16&&shape.flags==disk::RuntimePathStringFlags)bit=1;
            if(name==u"ReturnValue"&&kind==u"StructProperty"&&offset==16&&shape.bytes==24&&shape.flags==disk::RuntimePathResultFlags)bit=2;
        }
        if(!bit||(mask&bit)||!r.Read(head+cfgreflection::disk::PropertyDimensions,again)||
            again.dimension!=shape.dimension||again.bytes!=shape.bytes||again.flags!=shape.flags||
            !r.Read(head+cfgreflection::disk::PropertyOffset,finalOffset)||finalOffset!=offset)return false;
        mask|=bit;head=property.next;
    }
    std::uint8_t finalCount=0;std::uint16_t finalBytes=0;
    if(head||mask!=(load?15u:3u)||!r.Read(module+binding.cache,cached)||cached!=function||
        !r.Read(function+cfgreflection::disk::Children,cached)||cached!=initialHead||
        !r.Read(function+0x8C,finalCount)||finalCount!=count||!r.Read(function+0x8E,finalBytes)||finalBytes!=bytes||
        !r.Read(defaults+cfgprobe::ObjectClass,cached)||cached!=owner||!r.Read(owner+cfgspawn::ClassDefaultObject,cdo)||cdo!=defaults)return false;
    roots={defaults,function,owner,cdo};return true;
}
}
