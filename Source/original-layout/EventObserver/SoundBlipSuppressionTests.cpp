#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "NativeSoundBlipSuppression.hpp"
static cfgsoundblip::Slot AssignSoundSlot(Fixture& f,std::uintptr_t object){
    int index=0;CHECK(f.Read(object+0xC,index));f.Put(f.objects.data+24*static_cast<unsigned>(index)+16,std::int32_t{9});
    cfgsoundblip::Slot out;CHECK(cfgspawn::ObjectSlot(f,f.objects,object,out));return out;
}
struct SoundAccess {
    cfgsoundblip::Scope s;bool guard=true,visible=true,changes=true,returns=true;unsigned calls=0;
    SoundAccess(){s.world=0x900000;s.killer.object=0x100000;s.killer.serial=1;s.controller.object=0x200000;s.controller.serial=1;}
    const cfgsoundblip::Scope& Input()const{return s;}
    bool Guard()const{return guard;}
    bool Visible(bool& out)const{out=visible;return true;}
    bool SetVisible(bool value){++calls;if(changes)visible=value;return returns;}
};
static void NativeVisibilityPolicy(){
    {SoundAccess a;cfgsoundblip::Runtime r;CHECK(r.Apply(a)&&!a.visible&&r.Owned()&&a.calls==1);
        CHECK(!r.Apply(a)&&a.calls==1);r.Release(a);CHECK(a.visible&&r.report.restored==1&&a.calls==2);r.Release(a);CHECK(a.calls==2);}
    {SoundAccess a;cfgsoundblip::Runtime r;a.visible=false;CHECK(r.Apply(a)&&!r.Owned()&&a.calls==1);r.Release(a);CHECK(a.calls==1&&!a.visible);}
    {SoundAccess a;cfgsoundblip::Runtime r;a.guard=false;CHECK(!r.Apply(a)&&!a.calls);}
    {SoundAccess a;cfgsoundblip::Runtime r;a.changes=false;CHECK(!r.Apply(a)&&r.report.refused==1);r.Release(a);CHECK(a.calls==1);}
    {SoundAccess a;cfgsoundblip::Runtime r;a.returns=false;CHECK(!r.Apply(a)&&!a.visible);a.returns=true;r.Release(a);CHECK(a.visible&&r.report.restored==1);}
    {SoundAccess a;cfgsoundblip::Runtime r;CHECK(r.Apply(a));++a.s.killer.serial;r.Release(a);CHECK(a.calls==1&&r.report.lost==1);}
    {SoundAccess a;cfgsoundblip::Runtime r;CHECK(r.Apply(a));a.s.world++;r.Release(a);CHECK(a.calls==1&&r.report.lost==1);}
    {SoundAccess a;cfgsoundblip::Runtime r;CHECK(r.Apply(a));a.guard=false;r.Release(a);CHECK(a.calls==1&&r.report.lost==1);}
    {SoundAccess a;cfgsoundblip::Runtime r;CHECK(r.Apply(a));a.visible=true;r.Release(a);CHECK(a.calls==1);}
}
int main(){
    NativeVisibilityPolicy();
    Fixture f;cfgsoundblip::Scope s;s.active=s.offlineCounselor=true;s.module=f.module;
    s.viewport=0x900000;s.world=0x910000;s.mode=0x920000;s.humanController=0x930000;s.humanPawn=f.bot;
    s.killer=AssignSoundSlot(f,f.pawn);s.controller=AssignSoundSlot(f,f.botController);
    s.killerClass=f.pawnClass;s.controllerClass=f.classes[cfgspawn::KillerAI];
    f.Put(s.viewport+cfgprobe::ViewportWorld,s.world);f.Put(s.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
    f.Put(s.world+cfgprobe::WorldGameMode,s.mode);f.Put(s.humanController+cfgprobe::ControllerPawn,s.humanPawn);
    f.Put(f.module+cfgprobe::lifetime::ObjectArrayRva+0x10,f.objects.data);
    f.Put(f.module+cfgprobe::lifetime::ObjectArrayRva+0x1C,f.objects.count);
    f.Put(s.killer.object+0x3A0,s.controller.object);f.Put(s.controller.object+cfgprobe::ControllerPawn,s.killer.object);
    f.Put(s.controller.object+cfgprobe::ObjectClass,s.controllerClass);
    auto suppress=[&](){f.reads=0;return cfgsoundblip::Suppress(f,s,s.killer.object,true);};
    CHECK(suppress());CHECK(f.reads<=20);
    f.reads=0;CHECK(!cfgsoundblip::Suppress(f,s,s.humanPawn,true)&&f.reads==0);
    CHECK(!cfgsoundblip::Suppress(f,s,s.killer.object,false)&&f.reads==0);
    s.active=false;CHECK(!suppress());s.active=true;s.offlineCounselor=false;CHECK(!suppress());s.offlineCounselor=true;
    const auto original=s;
    s.humanPawn=s.killer.object;CHECK(!suppress());s=original;
    s.humanController=s.controller.object;CHECK(!suppress());s=original;
    ++s.killer.serial;CHECK(!suppress());s=original;++s.controller.serial;CHECK(!suppress());s=original;
    for(const auto item:std::array<std::pair<std::uintptr_t,std::uintptr_t>,8>{{
        {s.viewport+cfgprobe::ViewportWorld,s.world},{s.world+cfgprobe::WorldNetDriver,0},
        {s.world+cfgprobe::WorldGameMode,s.mode},{s.killer.object+0x3A0,s.controller.object},
        {s.controller.object+cfgprobe::ControllerPawn,s.killer.object},{s.humanController+cfgprobe::ControllerPawn,s.humanPawn},
        {s.killer.object+cfgprobe::ObjectClass,s.killerClass},{s.controller.object+cfgprobe::ObjectClass,s.controllerClass}}}){
        f.Put(item.first,std::uintptr_t{0xDEAD0000});CHECK(!suppress());f.Put(item.first,item.second);
    }
    f.Put(f.objects.data+24*static_cast<unsigned>(s.killer.index)+8,std::uint32_t{0x20000000});CHECK(!suppress());
    f.Put(f.objects.data+24*static_cast<unsigned>(s.killer.index)+8,std::uint32_t{});
    f.trigger=s.viewport+cfgprobe::ViewportWorld;f.mutation=f.trigger;f.newPointer=0xDEAD0000;CHECK(!suppress());
    f.trigger=0;f.Put(s.viewport+cfgprobe::ViewportWorld,s.world);f.reads=0;
    struct NativeHost {
        Fixture& fixture;std::uintptr_t owner=0,function=0,pawn=0;unsigned calls=0;bool last=true;
        bool FeatureFunction(const cfgvehicle::disk::Binding&,std::uintptr_t& fn,std::uintptr_t& type){fn=function;type=owner;return true;}
        bool Invoke(std::uintptr_t receiver,std::uintptr_t fn,void* packet,bool actor){
            CHECK(receiver==pawn&&fn==function&&actor&&packet);++calls;last=*static_cast<std::uint8_t*>(packet)!=0;
            fixture.Put(pawn+cfgsoundblip::disk::Visible,static_cast<std::uint8_t>(last));return true;
        }
    } host{f,f.killerBase,0x940000,s.killer.object};
    const auto& binding=cfgsoundblip::disk::SetSoundBlipVisibility;
    f.Put(f.module+binding.ownerReflection,host.owner);f.Put(f.module+binding.ownerConstructor,host.owner);
    f.Put(host.function+0xB0,f.module+cfgsoundblip::disk::VisibilityExecutor);
    f.Put(f.module+cfgsoundblip::disk::VisibilityExecutor,cfgsoundblip::disk::VisibilityExecutorCode);
    f.Put(f.module+cfgsoundblip::disk::VisibilityImplementation,cfgsoundblip::disk::VisibilityImplementationCode);
    f.Put(s.killer.object+cfgsoundblip::disk::Visible,std::uint8_t{1});
    cfgsoundblip::NativeAccess<Fixture,NativeHost> native(f,host,s,true,true);cfgsoundblip::Runtime runtime;
    CHECK(runtime.Apply(native)&&host.calls==1&&!host.last);f.reads=0;
    runtime.Release(native);CHECK(host.calls==2&&host.last&&runtime.report.restored==1);
    f.Put(f.module+cfgsoundblip::disk::VisibilityExecutor,std::uint8_t{0});f.reads=0;
    CHECK(!native.SetVisible(false)&&host.calls==2);
    std::printf("Sound-blip suppression tests: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
