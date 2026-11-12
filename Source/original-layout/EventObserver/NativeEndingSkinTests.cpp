#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "NativeEndingSkin.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
#include <functional>
namespace s=cfgending::skin;
struct Fixture {
    static constexpr std::uintptr_t Module=0x10000000,Data=0x20000000,Base=0x30000000,
        Meta=0x30010000,Skin=0x30020000,Cdo=0x30030000,State=0x30040000,StateType=0x30050000,Controller=0x30060000,BaseCdo=0x30070000;
    OwnedByteReader r;cfgprobe::lifetime::ArrayHeader registry{Data,20};cfgending::Packet selected{};
    bool eligible=true;
    cfgprimary::ScopedLookupEligibility<OwnedByteReader> lookup{Module,this,
        [](OwnedByteReader&,void* value){return static_cast<Fixture*>(value)->eligible;},true};
    void Object(std::uintptr_t value,int index,int serial=0){
        r.Set(value+8,std::uint32_t{});r.Set(value+cfgprobe::lifetime::ObjectIndexOffset,index);
        r.Set(Data+24ULL*index,value);r.Set(Data+24ULL*index+8,std::uint32_t{});r.Set(Data+24ULL*index+16,serial);
    }
    Fixture(){
        r.Set(Module+0x03199662,std::uint8_t{});r.Set(Module+0x03198110,std::uint32_t{});
        r.Set(Module+cfgending::disk::SCJasonSkinClass,Base);r.Set(Module+cfgending::disk::SCJasonSkinConstructor,Base);
        Object(Base,1);Object(Skin,2);Object(Cdo,3);Object(State,4,44);Object(BaseCdo,5);
        // Authored class defaults legitimately carry default-object flags.
        // They are NOT actor instances and must not use actor-only exclusions.
        r.Set(Cdo+8,std::uint32_t{0x10});r.Set(BaseCdo+8,std::uint32_t{0x10});
        r.Set(Base+cfgspawn::ClassDefaultObject,BaseCdo);r.Set(BaseCdo+cfgprobe::ObjectClass,Base);
        r.Set(Base+cfgprobe::ObjectClass,Meta);r.Set(Skin+cfgprobe::ObjectClass,Meta);
        r.Set(Meta+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(Base+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(Skin+cfgprobe::ClassSuper,Base);r.Set(Skin+cfgspawn::ClassDefaultObject,Cdo);r.Set(Cdo+cfgprobe::ObjectClass,Skin);
        r.Set(Module+cfgending::disk::SCPlayerStateClass,StateType);r.Set(Module+cfgending::disk::SCPlayerStateConstructor,StateType);
        r.Set(State+cfgprobe::ObjectClass,StateType);r.Set(StateType+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(State+0x88,Controller);r.Set(Controller+0x388,State);r.Set(State+0x110,std::uint8_t{3});
        selected[16]=3;r.Set(State+cfgending::disk::PickedClass,selected);r.Set(State+cfgending::disk::PickedSkin,Skin);
    }
    bool Source(std::uintptr_t& out){return s::Source(r,Module,registry,State,Controller,selected,out);}
};
struct Host {
    Fixture f;std::uintptr_t target=0;unsigned calls=0,sources=0;
    bool fresh=true,setReturns=true,write=true,concrete=true;unsigned targetReads=0,failTarget=0;
    std::function<void()> during;
    bool Fresh(){return fresh;}
    bool Target(std::uintptr_t& out){out=target;return ++targetReads!=failTarget;}
    bool Concrete(std::uintptr_t value){return concrete&&s::Concrete(f.r,f.Module,f.registry,value);}
    bool Source(std::uintptr_t& out){++sources;return concrete&&f.Source(out);}
    bool Set(std::uintptr_t value){++calls;if(write)target=value;if(during)during();return setReturns;}
};
int main(){
    unsigned checks=0,failures=0;
    const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check %u\n",checks);}};
    {Host h;std::uintptr_t result=0;check(s::Ensure(h,result)==0&&result==h.f.Skin&&h.target==result&&h.calls==1&&h.sources==2);}
    {Host h;h.target=h.f.Skin;std::uintptr_t result=0;check(s::Ensure(h,result)==0&&result==h.target&&!h.calls&&!h.sources);}
    {Host h;h.target=h.f.Base;std::uintptr_t result=0;check(s::Ensure(h,result)==0&&result==h.target&&!h.calls);}
    {Host h;h.f.r.Set(h.f.State+cfgending::disk::PickedSkin,h.f.Base);std::uintptr_t result=0;
     check(s::Ensure(h,result)==0&&result==h.f.Base&&h.target==h.f.Base&&h.calls==1);}
    for(unsigned fault=0;fault<21;++fault){Host h;auto& f=h.f;
        if(fault==0)h.fresh=false;
        if(fault==1)h.failTarget=1;
        if(fault==2)h.target=f.Meta; // unrelated UClass is never adopted
        if(fault==3)f.r.Set(f.State+0x88,std::uintptr_t{});
        if(fault==4)f.r.Set(f.Controller+0x388,std::uintptr_t{});
        if(fault==5){auto other=f.selected;other[16]=9;f.r.Set(f.State+cfgending::disk::PickedClass,other);}
        if(fault==6)f.r.Set(f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
        if(fault==7)f.r.Set(f.Skin+cfgprobe::ClassSuper,std::uintptr_t{});
        if(fault==8)f.r.Set(f.Cdo+cfgprobe::ObjectClass,f.Base);
        if(fault==9)f.r.Set(f.Module+cfgending::disk::SCJasonSkinConstructor,std::uintptr_t{});
        if(fault==10)f.r.Set(f.Module+0x03199662,std::uint8_t{1});
        if(fault==11)f.r.Set(f.Data+24ULL*2+8,std::uint32_t{0x10000000});
        if(fault==12)f.r.Set(f.Skin+8,std::uint32_t{0x20});
        if(fault==13)f.r.Set(f.Data+24ULL*4+16,int{0});
        if(fault==14)f.r.Set(f.State+0x110,std::uint8_t{2});
        if(fault==15)f.r.Set(f.State+cfgprobe::ObjectClass,f.Meta);
        if(fault==16)h.concrete=false;
        if(fault==17)h.failTarget=2;
        if(fault==18)f.r.Set(f.State+8,std::uint32_t{0x10});
        if(fault==19)f.r.Set(f.Data+24ULL*3+8,std::uint32_t{0x20000000});
        if(fault==20)f.eligible=false;
        std::uintptr_t result=99;check(s::Ensure(h,result)!=0&&!result&&!h.calls);
    }
    // Failed native return, source/target mutation, and lost admission are never
    // silently accepted. Each trial issues at most ONE native setter.
    for(unsigned fault=0;fault<5;++fault){Host h;
        if(fault==0)h.setReturns=false;
        if(fault==1)h.write=false;
        if(fault==2)h.during=[&]{h.fresh=false;};
        if(fault==3)h.during=[&]{h.f.r.Set(h.f.State+cfgending::disk::PickedSkin,std::uintptr_t{});};
        if(fault==4)h.during=[&]{h.f.r.Set(h.f.Skin+cfgprobe::ClassSuper,std::uintptr_t{});};
        std::uintptr_t result=0;check(s::Ensure(h,result)!=0&&!result&&h.calls==1);
    }
    // Mutation inside a coherent proof is refused (including unassigned
    // concrete-class serial becoming a DIFFERENT observation mid-snapshot).
    {Fixture f;f.r.changeAt=f.Skin+cfgspawn::ClassDefaultObject;f.r.changeSlot=f.Data+24ULL*2;f.r.newPointer=f.Base;
        std::uintptr_t result=0;check(!f.Source(result)&&!result);}
    {Host h;s::Transfer transfer;
        check(!transfer.Step(h)&&transfer.phase==1&&h.calls==1&&!transfer.failure);
        check(transfer.Step(h)&&transfer.phase==2&&transfer.expected==h.f.Skin&&h.calls==1);
        check(transfer.Step(h)&&h.calls==1);}
    {Host h;h.target=h.f.Base;s::Transfer transfer;
        check(transfer.Step(h)&&transfer.expected==h.f.Base&&!h.calls);}
    {Host h;s::Transfer transfer;h.during=[&]{check(!transfer.Step(h)&&transfer.phase==1&&h.calls==1);};
        check(!transfer.Step(h)&&!transfer.calling&&h.calls==1);check(transfer.Step(h)&&h.calls==1);}
    for(unsigned fault=0;fault<5;++fault){Host h;s::Transfer transfer;
        if(fault==0)h.setReturns=false;
        check(!transfer.Step(h)&&h.calls==1);
        if(fault==1)h.target=0;
        if(fault==2)h.fresh=false;
        if(fault==3)h.f.r.Set(h.f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
        if(fault==4)h.f.r.Set(h.f.Skin+cfgprobe::ClassSuper,std::uintptr_t{});
        check(!transfer.Step(h)&&transfer.phase==3&&transfer.failure&&h.calls==1);
        check(!transfer.Step(h)&&h.calls==1);}
    std::printf("Native ending selected-skin proof/order: %u checks, %u failures; synthetic memory only.\n",checks,failures);
    return failures?1:0;
}
