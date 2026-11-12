#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../RuntimeProbe/SpawnRuntimeBindings.generated.hpp"
#define CFG_URINATION_LOADER_FIXTURE_ONLY
#include "UrinationLoaderFrameObservationTests.cpp"
namespace {
using Dispatch=void(__fastcall*)(void*,void*,void*);
unsigned nativeCalls=0,rootCalls=0;bool rootsAllowed=true,nativeFault=false;void* received=nullptr;
void __fastcall LoaderStub(void*,void*,void* packet){++nativeCalls;received=packet;if(nativeFault)RaiseException(0xE0000041,0,0,nullptr);}
struct {std::uintptr_t module=0,world=0x700000,controller=0x710000;}plan;
struct Reader {inline static LoaderFixture* fixture=nullptr;unsigned cap=0,reads=0;
    template<class T>bool Read(std::uintptr_t address,T& value){if(!fixture||reads>=cap)return false;++reads;return fixture->r.Read(address,value);}
};
std::array<std::uintptr_t,6> capturedRoots{};
template<std::size_t N>bool RetainPrimaryObjects(Reader&,const std::array<std::uintptr_t,N>& values){++rootCalls;capturedRoots=values;return rootsAllowed;}
struct Host {
    cfgurination::LoadIdentity owner{plan.world,plan.controller,1,2,GetCurrentThreadId(),0x18E030,1};
    bool current=true,enter=true;unsigned starts=0,returns=0;bool normal=false;
    const auto& Owner()const{return owner;}
    bool Current(const cfgurination::LoadIdentity&){return current;}
    bool BeginRequest(){++starts;return enter;}
    void RequestReturned(bool okay){++returns;normal=okay;}
};
struct Access {
    unsigned calls_=0,beginCalls=0,finishCalls=0,failures=0;bool exception_=false,allowed=true,begin=true,finish=true;
    bool NativeCallAllowed(){return allowed&&!exception_&&calls_<12;}
    bool BeginBundleNative(){++beginCalls;return begin;}
    bool FinishBundleNative(){++finishCalls;return finish;}
    void BundleRetentionFailed(){++failures;}
#include "UrinationLoaderInvokeMembers.hpp"
};
void Rebase(LoaderFixture& fixture,bool load){using namespace cfgurination;Reader::fixture=&fixture;
    const auto& b=load?disk::LoadAsset:disk::MakeSoftObjectPath;const auto fn=load?fixture.Load:fixture.Make;
    fixture.r.Set(plan.module+b.cache,fn);fixture.r.Set(plan.module+b.ownerReflection,fixture.Owner);
    fixture.r.Set(plan.module+b.ownerConstructor,fixture.Owner);fixture.r.Set(plan.module+b.factory,b.code);
    fixture.r.Set(fn+0xB0,plan.module+(load?disk::LoadAssetExecutor:disk::MakePathExecutor));
    if(load)fixture.r.Set(plan.module+disk::LoadAssetExecutor,disk::LoadAssetExecutorCode);
    else fixture.r.Set(plan.module+disk::MakePathExecutor,disk::MakePathExecutorCode);
    std::uintptr_t names=0;fixture.r.Read(fixture.Base+cfgnames::disk::Global,names);fixture.r.Set(plan.module+cfgnames::disk::Global,names);
}
}
int main(){plan.module=reinterpret_cast<std::uintptr_t>(&LoaderStub)-cfgspawn::GenericDispatcherRva;
    for(const bool load:{false,true}){
        LoaderFixture f(load);Rebase(f,load);Host host;
        cfgurination::MakePathPacket make;cfgurination::LoadPacket packet;
        constexpr std::uintptr_t stringAt=0x720000,nameAt=0x730000;
        std::array<char16_t,cfgurination::ExpectedAsset.size()+1> text{};std::copy(cfgurination::ExpectedAsset.begin(),cfgurination::ExpectedAsset.end(),text.begin());
        f.r.Set(stringAt,text);make.input={stringAt,static_cast<int>(text.size()),static_cast<int>(text.size())};
        f.Name(nameAt,"/Game/Cinematics/SP_Testing/m_normal_stand_piss_Montage.m_normal_stand_piss_Montage");
        cfgurination::NativePath path;f.r.Read(nameAt+cfgnames::disk::ObjectName,path.asset);
        std::memcpy(packet.asset.data()+16,&path,sizeof(path));packet.world=plan.world;packet.latent={-1,host.owner.uuid,{},plan.controller};
        void* input=load?static_cast<void*>(&packet):static_cast<void*>(&make);const auto fn=load?f.Load:f.Make;
        {Access a;const auto before=nativeCalls;Check(a.InvokeUrinationLoader(host,f.Defaults,fn,input,load));
            Check(nativeCalls==before+1&&received==input&&a.calls_==1&&a.beginCalls==1&&a.finishCalls==1);
            Check((capturedRoots==std::array<std::uintptr_t,6>{f.Defaults,fn,f.Owner,f.Defaults,plan.world,plan.controller}));
            Check(host.starts==(load?1u:0u)&&host.returns==(load?1u:0u));}
        for(unsigned fault=0;fault<9;++fault){Access a;Host rejected;const auto before=nativeCalls;
            if(fault==0)a.allowed=false;if(fault==1)rejected.current=false;if(fault==2)rootsAllowed=false;
            if(fault==3)a.begin=false;if(fault==4)a.calls_=12;
            if(fault==5)rejected.owner.controller=0;
            if(fault==6){if(load)packet.callback.name.index=1;else make.result.asset.index=1;}
            if(fault==7){if(load)packet.latent.uuid=0;else make.input.count=0;}
            if(fault==8){if(load)packet.asset[0]=0;else make.input.capacity=0;}
            Check(!a.InvokeUrinationLoader(rejected,f.Defaults,fn,input,load)&&nativeCalls==before);
            rootsAllowed=true;packet.callback.name={};packet.latent.uuid=host.owner.uuid;make.result={};make.input.count=static_cast<int>(text.size());
            packet.asset[0]=0xFF;make.input.capacity=static_cast<int>(text.size());
        }
        {Access a;Host broken;a.finish=false;const auto before=nativeCalls;Check(!a.InvokeUrinationLoader(broken,f.Defaults,fn,input,load));Check(nativeCalls==before+1&&a.finishCalls==1);}
        {Access a;Host broken;nativeFault=true;const auto before=nativeCalls;
            Check(!a.InvokeUrinationLoader(broken,f.Defaults,fn,input,load)&&nativeCalls==before+1&&a.exception_&&a.failures==1&&a.finishCalls==0);
            if(load)Check(broken.returns==1&&!broken.normal);nativeFault=false;}
        if(load){Access a;Host broken;broken.enter=false;const auto before=nativeCalls;
            Check(!a.InvokeUrinationLoader(broken,f.Defaults,fn,input,true)&&nativeCalls==before&&a.finishCalls==1&&!a.calls_);}
    }
    std::printf("Actual retained urination loader gateway: %u failures; real schemas/packets, exact6 capturedRoots before entry, callbudget/owner/refusal/SEH/no-native-replay, original40/88-byte packets to owned stub. No game access.\n",failures);
    return failures?1:0;
}
