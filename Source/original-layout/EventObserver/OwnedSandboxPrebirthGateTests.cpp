#define main ExistingActiveGateTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "OwnedSandboxPrebirthGate.hpp"
#include <functional>
namespace gate=cfgmenu::promotion::gate;
static void __fastcall TestRestart(void* mode,void* controller);
static std::uintptr_t __fastcall TestGet(void* mode,void* controller);
struct GateFixture:Fixture {
    static constexpr std::uintptr_t State=0x400000,ModeClass=0x410000,Default=0x420000,Clone=0x430000;
    cfgmenu::promotion::Context c{};
    gate::Gates g{&TestRestart,&TestGet,0x500000};
    GateFixture(){
        using namespace cfgprobe;
        c.module=Module;c.engine=Engine;c.viewport=Viewport;c.world=World;c.gameInstance=Instance;c.localPlayers=Players;
        c.player=Player;c.mode=Mode;c.controller=Controller;c.playerState=State;c.thread=42;c.exactImageVerified=true;c.registry={Registry,4};
        const std::array<std::uintptr_t,4> actors{{World,Mode,Controller,State}};
        for(unsigned i=0;i<actors.size();++i){c.identities[i]={actors[i],static_cast<int>(i),static_cast<int>(100+i)};
            reader.Set(actors[i]+lifetime::ObjectIndexOffset,static_cast<int>(i));reader.Set(Registry+24*i,actors[i]);reader.Set(Registry+24*i+16,static_cast<int>(100+i));}
        reader.Set(Controller+ControllerPlayer,Player);reader.Set(Controller+ControllerPawn,std::uintptr_t{0});reader.Set(Controller+ControllerAcknowledgedPawn,std::uintptr_t{0});
        reader.Set(Controller+0x388,State);reader.Set(State+0x88,Controller);reader.Set(State+cfgmenu::birthdisk::SpawnedCharacterClass,std::uintptr_t{0});
        reader.Set(Module+cfgmenu::birthdisk::SCGameMode_SandboxClass,ModeClass);reader.Set(Module+cfgmenu::birthdisk::SCGameMode_SandboxConstructor,ModeClass);
        reader.Set(Mode+ObjectClass,ModeClass);reader.Set(ModeClass+0xF8,Default);reader.Set(Default+ObjectClass,ModeClass);
        reader.Set(Mode,Module+cfgmenu::gatedisk::Table);
        for(unsigned i=0;i<cfgmenu::gatedisk::Slots;++i)reader.Set(Module+cfgmenu::gatedisk::Table+8*i,Module+cfgmenu::gatedisk::Entries[i]);
    }
};
struct FakePublication {
    GateFixture& f;DWORD thread=42;unsigned allocations=0,frees=0,exchanges=0;
    bool ready=true,allocate=true,fault=false;std::function<void()> inAllocate,beforeExchange,afterExchange;
    explicit FakePublication(GateFixture& fixture):f(fixture){}
    DWORD Thread()const{return thread;}
    bool Ready(const cfgmenu::promotion::Context&,const gate::Gates&){return ready;}
    std::uintptr_t Allocate(const gate::Table& t){++allocations;if(!allocate)return 0;f.reader.Set(f.Clone,t);if(inAllocate)inAllocate();return f.Clone;}
    void Free(std::uintptr_t){++frees;}
    bool CompareExchange(std::uintptr_t object,std::uintptr_t expected,std::uintptr_t desired,std::uintptr_t& previous){
        ++exchanges;if(beforeExchange)beforeExchange();if(fault)return false;
        if(!f.reader.Read(object,previous))return false;if(previous==expected)f.reader.Set(object,desired);
        if(afterExchange)afterExchange();return true;
    }
};
using Installer=gate::OwnedSandboxPrebirthGate<OwnedByteReader,FakePublication>;
static unsigned restartCalls=0,getterCalls=0;
static void* restartMode=nullptr;static void* restartController=nullptr;
static void __fastcall TestRestart(void* mode,void* controller){++restartCalls;restartMode=mode;restartController=controller;}
static std::uintptr_t __fastcall TestGet(void* mode,void* controller){++getterCalls;return reinterpret_cast<std::uintptr_t>(mode)^reinterpret_cast<std::uintptr_t>(controller);}
int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* name){++checks;if(!ok){++failures;std::printf("FAIL %s\n",name);}};
    using cfgmenu::gatedisk::Table; // Native RVA, not the test table type.
    for(unsigned fault=0;fault<21;++fault){GateFixture f;FakePublication p(f);Installer a(f.reader,p);
        if(fault==1)f.c.exactImageVerified=false;
        if(fault==2)p.thread=43;
        if(fault==3)f.c.identities[1].serial=0;
        if(fault==4)f.reader.Set(f.Registry+24+16,int{999});
        if(fault==5)f.reader.Set(f.Mode+cfgprobe::ObjectClass,f.Default);
        if(fault==6)f.reader.Set(f.ModeClass+0xF8,f.Mode);
        if(fault==7)f.reader.Set(f.ModeClass+0xF8,std::uintptr_t{0});
        if(fault==8)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
        if(fault==9)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==10)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==11)f.reader.Set(f.State+cfgmenu::birthdisk::SpawnedCharacterClass,f.State);
        if(fault==12)f.reader.Set(f.Mode,f.Clone);
        if(fault==13)f.reader.Set(f.Module+Table+0x9D0,std::uintptr_t{0});
        if(fault==14)f.reader.Set(f.Module+Table,std::uintptr_t{0});
        if(fault==15)f.reader.Set(f.Module+cfgmenu::birthdisk::SCGameMode_SandboxConstructor,f.Default);
        if(fault==16)f.reader.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
        if(fault==17)p.ready=false;
        if(fault==18)f.g.getter=nullptr;
        if(fault==19)f.c.identities[2]=f.c.identities[1];
        const auto result=a.Install(f.c,f.g,fault!=20);
        check(result==(fault?gate::InstallResult::Refused:gate::InstallResult::Installed),"guards refuse before publication");
        check(p.exchanges==(fault?0u:1u),"only validated owned instance publishes");
        if(!fault){
            check(a.Status()==gate::State::Installed&&a.PrivateTable()==f.Clone,"private table is retained");
            for(unsigned i=0;i<cfgmenu::gatedisk::Slots;++i){std::uintptr_t native=0,copy=0;
                f.reader.Read(f.Module+Table+8*i,native);f.reader.Read(f.Clone+8*i,copy);
                const auto expected=i==cfgmenu::birthdisk::RestartSlot/8?reinterpret_cast<std::uintptr_t>(f.g.restart):
                    i==cfgmenu::birthdisk::GetterSlot/8?reinterpret_cast<std::uintptr_t>(f.g.getter):native;
                check(native==f.Module+cfgmenu::gatedisk::Entries[i]&&copy==expected,"all315 entries copied; shared table unchanged");}
            auto fresh=f.c;check(a.Bind(fresh)&&fresh.modeVtable==f.Clone&&fresh.restartGate==reinterpret_cast<std::uintptr_t>(f.g.restart),"fresh promotion context binds actual private gates");
            f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
            check(a.Restore(true)&&p.frees==1&&a.Status()==gate::State::Retired,"current-owner restore allowed after birth");
            check(!a.Restore(true)&&!a.Bind(fresh),"no second restoration or stale binding");
        }
    }
    for(unsigned scenario=0;scenario<8;++scenario){GateFixture f;FakePublication p(f);Installer a(f.reader,p);
        if(scenario==0)p.inAllocate=[&](){f.reader.Set(f.Registry+24+16,int{999});};
        if(scenario==1)p.beforeExchange=[&](){f.reader.Set(f.Mode,f.Default);};
        if(scenario==2)p.fault=true;
        if(scenario==3)p.afterExchange=[&](){f.reader.Set(f.World+cfgprobe::WorldGameMode,f.Default);};
        if(scenario==4)p.afterExchange=[&](){f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);};
        if(scenario==5)p.allocate=false;
        if(scenario==6)p.inAllocate=[&](){f.reader.Set(f.Clone+0x9D0,std::uintptr_t{0});};
        if(scenario==7)p.inAllocate=[&](){f.reader.Set(f.Module+Table+8,std::uintptr_t{0});};
        const auto result=a.Install(f.c,f.g,true);const bool quarantine=scenario==2||scenario==3;
        check(result==(quarantine?gate::InstallResult::Quarantined:gate::InstallResult::Refused),"mutation/fault outcomes explicit");
        check(p.frees==(quarantine||scenario==5?0u:1u),"uncertain publication never frees referenced clone");
        if(scenario==3){std::uintptr_t ptr=0;f.reader.Read(f.Mode,ptr);check(ptr==f.Clone&&p.exchanges==1,"travel change never blind-restores old mode");}
        if(scenario==4)check(p.exchanges==2&&a.Status()==gate::State::Retired,"postpublication rejection rolls back only while current");
    }
    for(unsigned scenario=0;scenario<7;++scenario){GateFixture f;FakePublication p(f);Installer a(f.reader,p);
        check(a.Install(f.c,f.g,true)==gate::InstallResult::Installed,"teardown fixture installed");
        if(scenario==0)f.reader.Set(f.Registry+24+16,int{999});
        if(scenario==1)f.reader.Set(f.Mode,f.Default);
        if(scenario==2)p.thread=43;
        if(scenario==3)f.reader.Set(f.Module+Table+8,std::uintptr_t{0});
        if(scenario==4)p.beforeExchange=[&](){f.reader.Set(f.Mode,f.Default);};
        if(scenario==5)p.fault=true;
        check(!a.Restore(scenario!=6)&&!p.frees,"stale/contended/uncertain/nonquiescent teardown retains clone");
    }
    {GateFixture f;FakePublication p(f);{Installer a(f.reader,p);check(a.Install(f.c,f.g,true)==gate::InstallResult::Installed,"destructor fixture installed");}
        check(!p.frees&&p.exchanges==1,"destructor performs no blind cleanup");}
    // Real current-process Windows heap/publication/callback ABI; NOT a CFG object.
    gate::WindowsPublication native;gate::Table original{};
    original.fill(reinterpret_cast<std::uintptr_t>(&TestGet));
    auto* ownedObject=static_cast<std::uintptr_t*>(HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,16));
    check(ownedObject!=nullptr,"own private instance allocated");
    if(ownedObject){const auto object=reinterpret_cast<std::uintptr_t>(ownedObject);
        auto clone=original;clone[cfgmenu::birthdisk::RestartSlot/8]=reinterpret_cast<std::uintptr_t>(&TestRestart);
        const auto table=native.Allocate(clone);*ownedObject=reinterpret_cast<std::uintptr_t>(original.data());std::uintptr_t before=0;
        check(table&&native.CompareExchange(object,*ownedObject,table,before)&&before==reinterpret_cast<std::uintptr_t>(original.data()),"real aligned instance CAS publishes private table");
        if(table){const auto* dispatch=reinterpret_cast<const std::uintptr_t*>(*ownedObject);
            reinterpret_cast<gate::RestartFunction>(dispatch[cfgmenu::birthdisk::RestartSlot/8])(ownedObject,reinterpret_cast<void*>(0x20000));
            const auto answer=reinterpret_cast<gate::GetterFunction>(dispatch[cfgmenu::birthdisk::GetterSlot/8])(ownedObject,reinterpret_cast<void*>(0x20000));
            check(restartCalls==1&&getterCalls==1&&restartMode==ownedObject&&restartController==reinterpret_cast<void*>(0x20000)&&answer==(object^0x20000),"compiled Restart/getter Windows x64 ABI");
            check(native.CompareExchange(object,table,reinterpret_cast<std::uintptr_t>(original.data()),before)&&before==table,"real reversible owned-vptr restoration");native.Free(table);}
        HeapFree(GetProcessHeap(),0,ownedObject);
    }
    GateFixture f;check(!native.Ready(f.c,f.g),"real publication refuses synthetic non-CFG identity");
    std::printf("CFG owned Sandbox gate: %s (%u checks, %u failures). Synthetic/own heap only; no game installation.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
