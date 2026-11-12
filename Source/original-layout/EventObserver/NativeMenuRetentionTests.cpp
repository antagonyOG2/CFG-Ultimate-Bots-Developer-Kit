#include "NativeMenuDispatch.hpp"
#include <cstdio>
#include <thread>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
constexpr std::uintptr_t Module=0x100000;
struct Host {
    bool current=true,input=true,output=true,revokeInput=false,revokeOutput=false;
    unsigned inputs=0,outputs=0,factories=0,results=0,uncertain=0,canceled=0;
    cfgmenu::rows::NativeRetentionHooks Hooks(){
        return {this,
            [](void* p,std::uintptr_t m){return m==Module&&static_cast<Host*>(p)->current;},
            [](void* p,std::uintptr_t,std::uintptr_t object,std::uintptr_t function,void* packet){
                auto& h=*static_cast<Host*>(p);++h.inputs;if(h.revokeInput)h.current=false;
                return h.input&&object==0x20000&&function==0x30000&&packet;},
            [](void* p,std::uintptr_t,std::uintptr_t function,void* packet){
                auto& h=*static_cast<Host*>(p);++h.outputs;if(h.revokeOutput)h.current=false;
                return h.output&&function==0x30000&&packet;},
            [](void* p,std::uintptr_t,std::uintptr_t address,std::uintptr_t owner){
                auto& h=*static_cast<Host*>(p);++h.factories;if(h.revokeInput)h.current=false;
                return h.input&&address==Module+0x100&&owner==0x40000;},
            [](void* p,std::uintptr_t,std::uintptr_t object){
                auto& h=*static_cast<Host*>(p);++h.results;if(h.revokeOutput)h.current=false;
                return h.output&&object==0x50000;},
            [](void* p){++static_cast<Host*>(p)->uncertain;},
            [](void* p){++static_cast<Host*>(p)->canceled;},
            [](void* p){return static_cast<Host*>(p)->current;}};
    }
};
struct LegacyEngine {unsigned calls=0;bool Factory(std::uintptr_t,std::uintptr_t& out){++calls;out=0x50000;return true;}};
struct TypedEngine:LegacyEngine {
    std::uintptr_t seen=0;
    bool FactoryOwned(std::uintptr_t,std::uintptr_t owner,std::uintptr_t& out){seen=owner;out=0x50000;return true;}
};
}
int main(){using Scope=cfgmenu::rows::NativeRetentionScope;unsigned packet=0;
    CHECK(!Scope::Current(Module));Host host;
    for(unsigned missing=0;missing<9;++missing){auto hooks=host.Hooks();
        switch(missing){case 0:hooks.context=nullptr;break;case 1:hooks.current=nullptr;break;
            case 2:hooks.inputs=nullptr;break;case 3:hooks.outputs=nullptr;break;
            case 4:hooks.factoryInput=nullptr;break;case 5:hooks.factoryOutput=nullptr;break;
            case 6:hooks.uncertain=nullptr;break;case 7:hooks.cancelUnpublished=nullptr;break;default:hooks.enterNative=nullptr;break;}
        Scope incomplete(Module,hooks,true);CHECK(!incomplete.Entered()&&!Scope::Current(Module));
    }
    {Scope denied(Module,host.Hooks(),false);CHECK(!denied.Entered());}
    {Scope scope(Module,host.Hooks(),true);CHECK(scope.Entered()&&Scope::Current(Module));
     {Scope nested(Module+1,host.Hooks(),true);CHECK(!nested.Entered()&&Scope::Current(Module)&&!Scope::Current(Module+1));}
     bool foreign=true;std::thread worker([&](){foreign=Scope::Current(Module);});worker.join();CHECK(!foreign);
     CHECK(Scope::Inputs(Module,0x20000,0x30000,&packet)&&host.inputs==1);
     CHECK(Scope::EnterNative(Module));
     CHECK(Scope::Outputs(Module,0x30000,&packet)&&host.outputs==1);
     CHECK(!Scope::FactoryInput(Module,Module+0x100,0)&&!host.factories);
     CHECK(Scope::FactoryInput(Module,Module+0x100,0x40000)&&host.factories==1);
     CHECK(Scope::FactoryOutput(Module,0x50000)&&host.results==1);
    }
    CHECK(!Scope::Current(Module)&&!Scope::Inputs(Module,0x20000,0x30000,&packet));
    CHECK(!Scope::EnterNative(Module));
    for(unsigned fault=0;fault<4;++fault){Host h;Scope scope(Module,h.Hooks(),true);
        if(fault==0)h.input=false;if(fault==1)h.revokeInput=true;
        if(fault==2)h.output=false;if(fault==3)h.revokeOutput=true;
        if(fault<2){CHECK(!Scope::Inputs(Module,0x20000,0x30000,&packet)&&h.inputs==1&&!h.uncertain&&h.canceled==1);}
        else {CHECK(!Scope::Outputs(Module,0x30000,&packet)&&h.outputs==1&&h.uncertain==1);
            CHECK(!Scope::Inputs(Module,0x20000,0x30000,&packet)&&!h.inputs);
            Scope::Uncertain(Module);CHECK(h.uncertain==1);}
    }
    for(unsigned fault=0;fault<3;++fault){Host h;Scope scope(Module,h.Hooks(),true);
        if(fault==0)h.output=false;if(fault==1)h.revokeOutput=true;
        CHECK(!Scope::FactoryOutput(Module,fault==2?0:0x50000)&&h.uncertain==1&&!Scope::Current(Module));
    }
    {Host h;Scope scope(Module,h.Hooks(),true);Scope::Uncertain(Module);
     CHECK(h.uncertain==1&&!Scope::Current(Module));}
    {Host hostedOwner;cfgmenu::rows::HostedLobbyRetentionScope hosted(Module,hostedOwner.Hooks(),true);
     CHECK(hosted.Entered()&&cfgmenu::rows::HostedLobbyRetentionScope::Current(Module)&&!Scope::Occupied());
     CHECK(!Scope::Current(Module)&&!Scope::Inputs(Module,0x20000,0x30000,&packet));
     CHECK(cfgmenu::rows::HostedLobbyRetentionScope::Inputs(Module,0x20000,0x30000,&packet));}
    LegacyEngine legacy;TypedEngine typed;std::uintptr_t out=0;
    CHECK(cfgmenu::rows::FactoryWithOwner(legacy,Module+0x100,0x40000,out)&&legacy.calls==1);
    CHECK(cfgmenu::rows::FactoryWithOwner(typed,Module+0x100,0x40000,out)&&!typed.calls&&typed.seen==0x40000);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    // Real dispatcher, exact current module, but no live retention lease.
    // It must refuse BEFORE touching this test executable's engine offsets.
    cfgmenu::rows::NativeDispatch native(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)),GetCurrentThreadId(),true);
    CHECK(!native.BeginPhase()&&!native.Invoke(0x20000,0x30000,&packet)&&
        !native.Factory(0x10000,out)&&!native.FactoryOwned(0x10000,0x40000,out)&&!native.Calls());
#endif
    std::printf("Existing menu retention lease/thread/nesting/expiry/input/output/factory-owner/refusal tests: %u failures; no game/native calls\n",failures);
    return failures?1:0;
}
