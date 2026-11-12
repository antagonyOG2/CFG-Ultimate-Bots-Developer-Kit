#include "RBEmoteRuntime.hpp"
#include "../Core/KillPolicy.hpp"
#define main CfgRBExistingFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include <vector>
struct InputHost {
    int identity=1;bool source=true,current=true,showOK=true;unsigned native=0;
    std::vector<unsigned> actions;
    bool Source(int& out){out=identity;return source;}
    bool Current(int owner){return current&&owner==identity;}
    bool Warm(bool show){++native;actions.push_back(show?2u:1u);return true;}
    bool Call(int,bool show){++native;actions.push_back(show?4u:3u);return !show||showOK;}
};
struct NativeHost {
    Fixture& r;cfgobserver::Plan context{};cfgrbemote::Slot pawn{};
    std::uintptr_t owner=0,hide=0,show=0;unsigned calls=0;
    explicit NativeHost(Fixture& value):r(value){context.objectArray=r.objects;context.world=0x990000;}
    bool Context(){return true;}
    bool MirrorHuman(cfgrbemote::Slot& out){out=pawn;return true;}
    bool FeatureFunction(const cfgvehicle::disk::Binding& b,std::uintptr_t& fn,std::uintptr_t& type){
        CHECK(b.nativeBytes==0&&b.storageBytes==0);type=owner;fn=b.cache==cfgrbemote::disk::OnHideEmoteSelect.cache?hide:show;return true;
    }
    bool Invoke(std::uintptr_t object,std::uintptr_t fn,void* packet,bool actor){
        CHECK(object==pawn.object&&(fn==hide||fn==show)&&!packet&&actor);++calls;return true;
    }
};
#ifndef CFG_RB_MEMBER_FIXTURE_ONLY
int main(){using namespace cfgrbemote;
    const Input held{true,true,true},up{true,true,false};
    {EdgeCapture e;e.Observe(held,100,true);e.Observe(up,116,true);
        CHECK(e.Take(200).down&&!e.Take(225).down&&e.edges==1);
        e.Observe(held,300,false);CHECK(!e.pending&&e.blocked==1);
        e.Observe(up,316,true);e.Observe(held,400,true);e.Observe(up,416,true);CHECK(!e.Take(1400).down);
        CHECK(!e.pending&&e.expired==1);e.Observe({},1401,true);CHECK(!e.Take(1402).foreground);}
    {cfgkill::Policy p;p.stage=cfgkill::Stage::NativeContext;p.usedAt=100;p.until=60100;
        CHECK(p.Exclusive(30100)&&!p.InputExclusive(30100)&&p.InputExclusive(10100-1));
        CHECK(!p.InputExclusive(10100));
        p.interactionAttempted=true;CHECK(p.InputExclusive(30100));p.interactionAttempted=false;
        p.nativeActiveObserved=true;CHECK(p.InputExclusive(30100));p.nativeActiveObserved=false;
        p.stage=cfgkill::Stage::AwaitContext;CHECK(p.InputExclusive(30100));}
    {Policy<int> p;InputHost h;p.Step(h,held,false);p.CapturedPress();
        CHECK(p.Step(h,held,true)&&p.phase==Phase::WarmHide);
        p.Step(h,up,true);p.Step(h,up,true);p.Step(h,up,true);p.Step(h,up,true);
        CHECK(p.phase==Phase::Open&&p.shows==1);}
    {Policy<int> p;InputHost h;for(unsigned i=0;i<5;++i){const auto before=h.native;CHECK(p.Step(h,held,true));CHECK(h.native-before<=1);}
        CHECK(p.phase==Phase::Open&&p.shows==1&&p.hides==1&&h.actions==std::vector<unsigned>({1,2,3,4}));
        for(unsigned i=0;i<100;++i)CHECK(!p.Step(h,held,true));CHECK(h.native==4);
        CHECK(p.Step(h,up,true)&&p.phase==Phase::Idle&&p.hides==2);}
    for(unsigned failure=0;failure<5;++failure){Policy<int> p;InputHost h;for(unsigned i=0;i<5;++i)p.Step(h,held,true);
        if(failure==0){CHECK(p.Step(h,Input{},true)&&p.phase==Phase::Idle);}
        if(failure==1){CHECK(p.Step(h,{true,false,false},true)&&p.phase==Phase::Idle);}
        if(failure==2){CHECK(!p.Step(h,held,false)&&p.phase==Phase::Release);CHECK(p.Step(h,held,true)&&p.phase==Phase::Idle);}
        if(failure==3){h.identity=2;CHECK(p.Step(h,up,true)&&p.abandoned==1&&h.native==4);}
        if(failure==4){p.Release();CHECK(p.Step(h,held,true)&&p.phase==Phase::Idle);}
    }
    {Policy<int> p;InputHost h;h.showOK=false;for(unsigned i=0;i<5;++i)p.Step(h,held,true);
        CHECK(p.phase==Phase::Release&&p.refused==1);CHECK(p.Step(h,held,true)&&p.phase==Phase::Idle&&h.native==5);}
    {Policy<int> p;InputHost h;CHECK(!p.Step(h,held,false));CHECK(!p.Step(h,held,true));CHECK(h.native==0);
        p.Step(h,up,true);p.Step(h,held,true);CHECK(p.phase==Phase::WarmHide);
        for(unsigned i=0;i<4;++i)p.Step(h,up,true);
        CHECK(p.phase==Phase::Open&&p.shows==1&&h.native==4);
        CHECK(p.Step(h,up,true)&&p.phase==Phase::Idle&&p.hides==2);}
    for(unsigned stage=1;stage<5;++stage){Policy<int> p;InputHost h;
        for(unsigned i=0;i<stage;++i)p.Step(h,held,true);
        CHECK(!p.Step(h,Input{},true)&&p.phase==Phase::Idle&&p.shows==0);}
    Fixture r;r.cap=1536;NativeHost host(r);host.owner=r.Object();const auto controller=r.Object(),pawn=r.Object(host.owner);
    const auto positive=[&](std::uintptr_t object,Slot& slot){int index=0;CHECK(r.Read(object+0xC,index));r.Put(r.objects.data+24*index+16,index+1);CHECK(cfgspawn::ObjectSlot(r,r.objects,object,slot));};
    Slot controllerSlot{};positive(controller,controllerSlot);positive(pawn,host.pawn);
    r.Put(controller+cfgescape::disk::Pawn,pawn);r.Put(pawn+cfgescape::disk::Controller,controller);
    host.hide=r.Object();host.show=r.Object();
    r.Put(r.module+disk::OnHideEmoteSelect.cache,host.hide);r.Put(host.hide+0xB0,r.module+disk::OnHideEmoteSelectExecutor);r.Put(r.module+disk::OnHideEmoteSelectExecutor,disk::OnHideEmoteSelectCode);
    r.Put(r.module+disk::OnRequestShowEmoteSelect.cache,host.show);r.Put(host.show+0xB0,r.module+disk::OnRequestShowEmoteSelectExecutor);r.Put(r.module+disk::OnRequestShowEmoteSelectExecutor,disk::OnRequestShowEmoteSelectCode);
    NativeAccess<Fixture,NativeHost> access(r,host,r.module,controller,controllerSlot);Owner owner{};
    CHECK(access.Source(owner));CHECK(access.Warm(false)&&access.Warm(true));
    CHECK(access.Call(owner,false)&&access.Call(owner,true)&&host.calls==2);
    r.Put(controller+cfgescape::disk::Pawn,std::uintptr_t{});CHECK(!access.Call(owner,false)&&host.calls==2);r.Put(controller+cfgescape::disk::Pawn,pawn);
    r.Put(host.show+0xB0,r.module+disk::OnHideEmoteSelectExecutor);CHECK(!access.Call(owner,true)&&host.calls==2);
    r.Put(host.show+0xB0,r.module+disk::OnRequestShowEmoteSelectExecutor);
    r.Put(r.module+disk::OnHideEmoteSelect.cache,std::uintptr_t{});CHECK(!access.Call(owner,false)&&host.calls==2);
    r.Put(r.module+disk::OnHideEmoteSelect.cache,host.hide);r.reads=0;Runtime runtime;
    for(unsigned i=0;i<6;++i){r.reads=0;CHECK(runtime.policy.Step(access,held,true));CHECK(r.reads<1536);}
    CHECK(runtime.policy.Owned());CHECK(runtime.Close(access,true)&&!runtime.policy.Owned()&&host.calls==5);
    std::printf("RB emote fixtures: %s (%d failures); native zero-frame bindings, paced edges, owned close, exclusions, replacement and cold-call guards\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif
