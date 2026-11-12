#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "NativeStartupRetention.hpp"
#include "CallbackRootLease.hpp"
#include "NativePublicationWindow.hpp"
#include "NativeSelectedJasonPath.hpp"
#include <cstdio>
#include <stdexcept>
#include <thread>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Host {
    cfgretain::ResidentRootBank<2> bank;
    cfgretain::CallbackRootLease<decltype(bank),64> lease{bank,0x100000};
    cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
    bool held=false,current=true,permit=true,failEnter=false,failAfter=false,proofTracksBank=false;
    unsigned calls=0,cancels=0,uncertain=0;
    bool ProvenSlotWindow()noexcept{return held&&current&&(!proofTracksBank||bank.Healthy());}
    bool NativeEntryUnlocked()const noexcept{return !held;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t address)noexcept{entries[static_cast<unsigned>(list.count++)]=address;return true;}
    void Collect(std::uintptr_t clear=0){
        for(int i=0;i<list.count;++i){auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(entries[static_cast<unsigned>(i)]);
            owner.CollectWith([&](std::uintptr_t* slot)noexcept{if(*slot==clear)*slot=0;},true);}
    }
    bool Required(){for(unsigned first=0;first<lease.Claims();first+=32)
        if(!lease.RequiredRange(*this,first,(std::min)(32u,lease.Claims()-first)))return false;return true;}
    cfgmenu::startupretain::Hooks Hooks(){using namespace cfgmenu::startupretain;
        return {this,[](void* p,std::uintptr_t module){return static_cast<Host*>(p)->current&&module==0x100000;},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned){auto& h=*static_cast<Host*>(p);h.held=true;
                const bool ok=h.lease.Retain(h,values);h.held=false;return ok;},
            [](void* p,Call,const std::array<std::uintptr_t,32>& values,unsigned,unsigned maximum){auto& h=*static_cast<Host*>(p);
                h.held=true;const bool ok=h.permit&&h.Required()&&h.lease.Retain(h,values)&&h.lease.PrepareCall(h,maximum);h.held=false;return ok;},
            [](void* p){auto& h=*static_cast<Host*>(p);return !h.failEnter&&h.lease.EnterNative(h);},
            [](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& h=*static_cast<Host*>(p);h.held=true;
                const bool ok=!h.failAfter&&h.lease.CompleteCall(h,values,count)&&h.Required();h.held=false;return ok;},
            [](void* p){auto& h=*static_cast<Host*>(p);++h.cancels;h.held=true;(void)h.lease.CancelCall(h);h.held=false;},
            [](void* p){auto& h=*static_cast<Host*>(p);++h.uncertain;h.lease.Uncertain();}};
    }
    bool Close(){held=true;const bool ok=lease.Close(*this);held=false;return ok;}
};
Host* selectedPathHost=nullptr;
std::uintptr_t __fastcall OwnedSelectedPath(void* output,const std::uintptr_t* klass){
    CHECK(klass&&*klass==0x20000);
    if(selectedPathHost){
        CHECK(!selectedPathHost->held);selectedPathHost->Collect();
        CHECK(!cfgmenu::startupretain::Invoke(0x100000,cfgmenu::startupretain::Call::SelectedClassPath,
            std::array<std::uintptr_t,1>{0x20000},[](){return true;}));
    }
    auto& value=*static_cast<cfgmenu::promotion::Packet*>(output);value={};value.bytes[16]=17;
    return reinterpret_cast<std::uintptr_t>(output);
}
std::uintptr_t __fastcall FaultSelectedPath(void*,const std::uintptr_t*){
    RaiseException(0xE0010289,0,0,nullptr);return 0;
}
}
int main(){using namespace cfgmenu::startupretain;
    CHECK(Allowed(Role::Intro,Call::SelectedClassPath));
    CHECK(!Allowed(Role::Start,Call::SelectedClassPath)&&!Allowed(Role::Match,Call::SelectedClassPath));
    for(unsigned fault=0;fault<2;++fault){Host host;Scope scope(0x100000,Role::Intro,host.Hooks(),true);
        CHECK(scope.Entered());selectedPathHost=&host;cfgmenu::promotion::Packet value;
        const auto helper=reinterpret_cast<std::uintptr_t>(fault?&FaultSelectedPath:&OwnedSelectedPath);
        const bool okay=Invoke(0x100000,Call::SelectedClassPath,std::array<std::uintptr_t,1>{0x20000},
            [&](){return cfgselectedjason::ConstructPath(helper,value,0x20000);});
        CHECK(okay==(fault==0));
        if(!fault)CHECK(value.bytes[16]==17&&host.lease.Claims()==1&&host.uncertain==0);
        else CHECK(host.uncertain==1&&!Scope::Current(0x100000));
        selectedPathHost=nullptr;
    }
    const std::array<std::uintptr_t,1> inputs{0x20000};unsigned called=0;
    {Host host;host.held=true;CHECK(host.lease.Retain(host,inputs));host.held=false;
        unsigned attempts=0,tries=0,work=0,yields=0;std::uint64_t time=0;
        const bool closed=cfgretain::PublicationWindow([&](){return host.lease.Current()&&host.current;},
            [&](bool& acquired,bool& contended){++tries;
                if(tries<3){contended=true;return false;}
                acquired=true;host.held=true;++work;const bool okay=host.lease.Close(host);host.held=false;return okay;
            },[&](){return time;},[&](){CHECK(!host.held&&host.bank.Healthy()&&host.lease.Claims()==1);
                host.Collect();++yields;++time;},attempts);
        CHECK(closed&&tries==3&&work==1&&yields==2&&attempts==3&&host.bank.Healthy()&&!host.lease.Claims());
    }
    {Host broken;broken.held=true;broken.proofTracksBank=true;
        CHECK(broken.bank.Healthy());
        CHECK(!broken.lease.Retain(broken,inputs)&&broken.bank.Poisoned()&&broken.list.count==0);
        // PID3612 regression: Grow sets bank uncertain BEFORE the normal
        // registration checks. A window proof must NOT depend on bank health.
        Host repaired;repaired.held=true;
        CHECK(repaired.lease.Retain(repaired,inputs)&&repaired.bank.Healthy()&&repaired.list.count==1);
        CHECK(repaired.lease.RequiredRange(repaired,0,1));
        repaired.current=false;CHECK(!repaired.lease.PrepareCall(repaired,1));
        repaired.current=true;CHECK(repaired.Close()&&repaired.bank.Healthy());
    }
    CHECK(!Invoke(0x100000,Call::LoadClass,inputs,[&](){++called;return true;})&&!called);
    {Host host;Scope scope(0x100000,Role::Intro,host.Hooks(),true);CHECK(scope.Entered());
        std::array<std::uintptr_t,1> outputs{};
        CHECK(Invoke(0x100000,Call::LoadClass,inputs,outputs,[&](){
            CHECK(!host.held);host.Collect();outputs[0]=0x30000;return true;}));
        CHECK(host.lease.Claims()==2&&Retain(0x100000,std::array<std::uintptr_t,1>{0x40000}));
        CHECK(Invoke(0x100000,Call::ConsumeActive,std::array<std::uintptr_t,1>{0x30000},[&](){
            host.Collect();CHECK(!Current(0x100000));return true;}));
        CHECK(host.lease.Claims()==3&&host.Close()&&!host.uncertain);
    }
    {Host host;
        {Scope intro(0x100000,Role::Intro,host.Hooks(),true);std::array<std::uintptr_t,1> loaded{};
            CHECK(Invoke(0x100000,Call::LoadClass,inputs,loaded,[&](){loaded[0]=0x30000;return true;}));}
        // The SESSION owns the lease, not the expired callback capability.
        // Claims survive AwaitMatch while no native-call scope is available.
        CHECK(!Current(0x100000)&&host.lease.Claims()==2);host.Collect();
        {Scope match(0x100000,Role::Match,host.Hooks(),true);
            CHECK(Invoke(0x100000,Call::ParentMatch,std::array<std::uintptr_t,1>{0x30000},[&](){return true;})&&host.Close());}
    }
    for(unsigned fault=0;fault<8;++fault){Host host;Scope scope(0x100000,Role::Intro,host.Hooks(),true);called=0;
        if(fault==0)host.permit=false;if(fault==1)host.failEnter=true;if(fault==2)host.failAfter=true;
        if(fault==3)host.current=false;
        const bool result=Invoke(0x100000,fault==7?static_cast<Call>(255):fault==4?Call::ParentMatch:Call::CopyPath,inputs,[&](){
            ++called;host.Collect(fault==6?0x20000:0);if(fault==5)throw std::runtime_error("native model fault");return true;});
        CHECK(!result&&called==(fault==2||fault==5||fault==6?1u:0u));
        CHECK(host.uncertain==(fault==2||fault==5||fault==6?1u:0u));
        if(host.uncertain)CHECK(!Invoke(0x100000,Call::CopyPath,inputs,[&](){++called;return true;}));
        else {host.current=true;CHECK(host.Close());}
    }
    {Host host;Scope outer(0x100000,Role::Intro,host.Hooks(),true);CHECK(Current(0x100000));
        {Scope nested(0x100000,Role::Intro,host.Hooks(),true);CHECK(!nested.Entered()&&!Current(0x100000));}
        CHECK(Current(0x100000));bool foreign=false;std::thread thread([&](){foreign=Current(0x100000);});thread.join();CHECK(!foreign&&host.Close());}
    {Host host;Scope scope(0x100000,Role::Match,host.Hooks(),true);called=0;
        CHECK(!Invoke(0x100000,Call::SpawnController,inputs,[&](){++called;return true;})&&!called);
        CHECK(Invoke(0x100000,Call::ParentMatch,inputs,[&](){++called;return true;})&&called==1&&host.Close());}
    // Actual scope+bank+output reservation: a slow owner proof and first busy
    // lock must not re-run RestartBot, drop inputs or touch outputs unlocked.
    for(unsigned scenario=0;scenario<3;++scenario){Host host;auto hooks=host.Hooks();called=0;
        struct Completion {Host* host;unsigned scenario,tries=0,work=0;std::uint64_t clock=0;};
        // Hooks still receive the same Host. Completion observation is local
        // to this lexical test and never supplies engine authority.
        static Completion* observed=nullptr;Completion observation{&host,scenario};observed=&observation;
        hooks.after=[](void* p,const std::array<std::uintptr_t,32>& values,unsigned count){auto& h=*static_cast<Host*>(p);auto& c=*observed;
            unsigned attempts=0;return cfgretain::PublicationWindow([&](){c.clock+=20;return h.current;},
                [&](bool& acquired,bool& contended){++c.tries;if(c.tries==1){contended=true;return false;}
                    acquired=true;h.held=true;++c.work;const bool okay=h.lease.CompleteCall(h,values,count)&&h.Required();h.held=false;return okay;},
                [&](){return c.clock;},[&](){CHECK(!h.held&&h.lease.NativeEntered()&&h.lease.Claims()==1);
                    h.Collect(c.scenario==1?0x20000:0);if(c.scenario==2)h.current=false;},attempts,nullptr,cfgretain::StartupPublicationLimits);};
        {Scope scope(0x100000,Role::Intro,hooks,true);std::array<std::uintptr_t,1> outputs{};
            const bool okay=Invoke(0x100000,Call::RestartBot,inputs,outputs,[&](){++called;outputs[0]=0x30000;return true;});
            CHECK(called==1&&okay==(scenario==0));
            if(!scenario)CHECK(observation.tries==2&&observation.work==1&&host.lease.Claims()==2&&host.Close());
            else CHECK(host.uncertain==1&&host.bank.Poisoned()&&!Invoke(0x100000,Call::RestartBot,inputs,[&](){++called;return true;})&&called==1);
        }
        observed=nullptr;
    }
    std::printf("Typed native startup retention: %u failures; owned registry only, no engine calls.\n",failures);return failures?1:0;
}
