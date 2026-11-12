#define main NativeActionFixtureMain
#include "UrinationActionObservationTests.cpp"
#undef main
#include "UrinationRetainedLoad.hpp"
#include <functional>
namespace {
struct Registry {
    bool held=true;unsigned proofs=0,failAt=0;
    std::function<void(unsigned)> duringProof;
    cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
    bool ProvenSlotWindow()noexcept{if(!held)return false;++proofs;if(duringProof)duringProof(proofs);return proofs!=failAt;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t owner)noexcept{entries[static_cast<unsigned>(list.count++)]=owner;return true;}
};
struct RetainedHost:Host {
    bool inputsValid=true,loadedValid=true,residentCurrent=true;unsigned loadedChecks=0;
    std::function<void()> duringLoaded;
    bool Inputs(const LoadIdentity& id,const std::array<std::uintptr_t,5>& values){return inputsValid&&Current(id)&&values[2]==0xA00000&&values[3]==0xA10000;}
    bool Loaded(const LoadIdentity& id,std::uintptr_t value){++loadedChecks;if(duringLoaded)duringLoaded();return Current(id)&&loadedValid&&value==0xB00000;}
    bool CurrentResidentAssets(){return residentCurrent;}
    bool ResidentLoaded(std::uintptr_t value){if(duringLoaded)duringLoaded();return residentCurrent&&loadedValid&&value==0xB00000;}
};
struct Run {
    Fixture data;Registry registry;RetainedHost host;cfgretain::ResidentRootBank<1> bank;
    RetainedLoad<decltype(bank)> load{bank};
    bool Prepare(std::uintptr_t female=0xA20000){return load.Prepare(registry,host,data.Module,data.identity,
        {data.identity.world,data.identity.controller,0xA00000,0xA10000,female},100);}
    void Waiting(std::uintptr_t female=0xA20000){CHECK(Prepare(female));registry.held=false;CHECK(load.Enter(true));
        CHECK(load.Constructor(data.r,host,data.owner,data.Action,data.Action,data.Path,data.Latent,data.Module+disk::ConstructorReturn));
        CHECK(load.Returned(true,true));
        data.r.Set(data.Action,data.Module+disk::ActionVtable);
        data.r.Set(data.Action+disk::ActionHandle,NativeHandle{data.Handle,data.Control});
        data.r.Set(data.Handle+disk::HandleComplete,std::uint8_t{1});data.r.Set(data.Handle+disk::HandleCancelled,std::uint8_t{});
        data.r.Set(data.Action+disk::ActionDelegate,std::array<std::uint64_t,2>{});data.r.reads=0;host.calls=0;
    }
    bool Complete(std::uintptr_t caller=Fixture::Module+disk::DelegateReturn){return load.Complete(data.r,host,registry,
        data.owner,data.Action,data.Action+disk::ActionDelegate,caller,0xB00000,200);}
    unsigned Roots(std::uintptr_t wanted=0){unsigned count=0;
        if(registry.list.count){const auto* owner=reinterpret_cast<const cfggcretain::Owner*>(registry.entries[0]);
            for(const auto value:owner->slots)if(value&&(!wanted||value==wanted))++count;}
        return count;
    }
};
}
int main(){
    {Run f;f.Waiting();CHECK(f.Roots()==5);CHECK(!f.load.Enter(true));f.registry.held=true;
        bool inner=true;f.host.duringLoaded=[&]{CHECK(f.load.Request().Stage()==LoadStage::Completing);inner=f.Complete();};
        CHECK(f.Complete()&&!inner&&f.load.HasLoadedHandle()&&f.Roots()==6);f.host.duringLoaded={};
        CHECK(f.host.loadedChecks==1&&f.load.Request().Action()==0);f.registry.held=false;
        // Native callback may now forward/retire action. Only opaque asset
        // handles survive; output is not resolved from that action again.
        f.load.Destroyed(f.data.Action);f.data.r.Set(f.data.Action,std::uintptr_t{});
        std::uintptr_t value=123;CHECK(!f.load.Resolve(f.registry,f.host,value)&&!value);
        auto* owner=reinterpret_cast<cfgretain::SynchronizedRootOwner*>(f.registry.entries[0]);
        owner->CollectWith([](std::uintptr_t*)noexcept{},true);CHECK(f.bank.Healthy());
        f.registry.held=true;CHECK(f.load.Resolve(f.registry,f.host,value)&&value==0xB00000);
        CHECK(f.load.ReleaseTransient(f.registry)&&f.Roots()==4&&!f.Roots(f.data.identity.world)&&!f.Roots(f.data.identity.controller));
        f.host.loadedValid=false;value=123;CHECK(!f.load.Resolve(f.registry,f.host,value)&&!value);}
    for(unsigned fault=0;fault<5;++fault){Run f;
        if(fault==0)f.registry.held=false;
        if(fault==1)f.host.current=false;
        if(fault==2)f.host.inputsValid=false;
        if(fault==3)f.data.identity.generation=0;
        if(fault==4)f.data.identity.controllerSerial=7;
        CHECK(!f.Prepare()&&!f.load.Attempted()&&f.bank.Owners()==0);}
    for(unsigned fault=0;fault<6;++fault){Run f;f.Waiting();f.registry.held=true;
        if(fault==0)f.registry.held=false;
        if(fault==1)f.host.loadedValid=false;
        if(fault==2)f.host.current=false;
        if(fault==3)f.data.r.Set(f.data.Handle+disk::HandleCancelled,std::uint8_t{1});
        if(fault==4)f.bank.MarkUncertain();
        if(fault==5)f.data.r.Set(f.data.Action+disk::ActionDelegate,std::array<std::uint64_t,2>{0,1});
        CHECK(!f.Complete()&&!f.load.HasLoadedHandle()&&!f.Roots(0xB00000));
        CHECK(f.load.Request().Stage()==LoadStage::Rejected&&!f.Complete());
        f.registry.held=true;f.host.current=true;
        if(fault==4)CHECK(!f.load.RetireRejected(f.registry)&&f.Roots()==5); // uncertainty quarantines, never unpoisons
        else CHECK(f.load.RetireRejected(f.registry)&&f.Roots()==0&&f.load.RetiredInputs()==5);
    }
    {Run f;f.Waiting();f.registry.held=true;CHECK(!f.Complete(f.data.Module+disk::DelegateReturn+1));
        CHECK(f.load.Request().Stage()==LoadStage::Waiting);CHECK(f.Complete()&&f.load.HasLoadedHandle());}
    for(unsigned fault=0;fault<2;++fault){Run f;f.Waiting();f.registry.held=true;
        f.host.duringLoaded=[&]{if(!fault)f.load.Destroyed(f.data.Action);else f.load.Expire(200,false);};
        CHECK(!f.Complete()&&!f.load.HasLoadedHandle()&&!f.Roots(0xB00000));
        CHECK(f.load.Request().Stage()==LoadStage::Rejected);f.host.duringLoaded={};
        CHECK(f.load.RetireRejected(f.registry)&&f.Roots()==0);}
    {Run f;f.Waiting();f.registry.held=true;f.registry.proofs=0;
        f.registry.duringProof=[&](unsigned proof){if(proof==2)f.load.Destroyed(f.data.Action);};
        CHECK(!f.Complete()&&f.load.Request().Stage()==LoadStage::Rejected&&!f.bank.Healthy());
        CHECK(f.load.HasLoadedHandle()&&f.Roots(0xB00000)==1); // opaque uncertain root is quarantined, never exposed/replayed
        f.registry.duringProof={};std::uintptr_t value=123;CHECK(!f.load.Resolve(f.registry,f.host,value)&&!value);
        CHECK(!f.load.RetireRejected(f.registry));}
    {Run f;f.Waiting(0);CHECK(f.Roots()==4);f.registry.held=true;CHECK(f.Complete()&&f.Roots()==5);
        CHECK(f.load.ReleaseTransient(f.registry)&&f.Roots()==3);}
    {Run f;f.Waiting();std::uintptr_t value=123;CHECK(!f.load.ResolveResident(f.registry,f.host,value)&&!value);
        f.registry.held=true;CHECK(f.Complete()&&f.load.ReleaseTransient(f.registry));f.host.current=false;
        CHECK(!f.load.Resolve(f.registry,f.host,value)&&!value);
        CHECK(f.load.ResolveResident(f.registry,f.host,value)&&value==0xB00000&&!f.Roots(f.data.identity.world)&&!f.Roots(f.data.identity.controller));
        f.host.residentCurrent=false;CHECK(!f.load.ResolveResident(f.registry,f.host,value)&&!value);
        f.host.residentCurrent=true;f.host.loadedValid=false;CHECK(!f.load.ResolveResident(f.registry,f.host,value)&&!value);
        f.host.loadedValid=true;f.host.duringLoaded=[&]{f.bank.MarkUncertain();};CHECK(!f.load.ResolveResident(f.registry,f.host,value)&&!value);}
    {Run f;f.Waiting();std::array<std::uintptr_t,3> observed{};
        CHECK(!f.load.WithAssetRoots(f.registry,[&](const auto&){CHECK(false);return true;}));
        f.registry.held=true;CHECK(f.load.WithAssetRoots(f.registry,[&](const auto& assets){observed=assets;return true;}));
        CHECK((observed==std::array<std::uintptr_t,3>{0xA00000,0xA10000,0xA20000}));
        CHECK(f.Complete());CHECK(f.load.ReleaseTransient(f.registry));
        CHECK(f.load.WithAssetRoots(f.registry,[&](const auto& assets){return assets==observed;}));}
    for(unsigned failAt=1;failAt<18;++failAt){Run f;f.Waiting();f.load.Expire(30100,true);
        f.registry.held=true;f.registry.proofs=0;f.registry.failAt=failAt;
        const bool retired=f.load.RetireRejected(f.registry);
        CHECK(!f.load.HasLoadedHandle());f.registry.failAt=0;
        CHECK(f.load.RetireRejected(f.registry)&&f.Roots()==0&&f.load.RetiredInputs()==5);
        if(retired)CHECK(f.bank.Healthy());}
    std::printf("Actual resident-bank urination handoff: %u failures; retained inputs/reserved output, consume-before-reentry, no failed raw output cache/replay, native-action retirement, later resolve, transient release, fresh-window partial cleanup and uncertainty quarantine. No game/native authority.\n",failures);
    return failures?1:0;
}
