#define CFG_STARTING_FIXTURE_ONLY
#include "StartingPlayerObservationTests.cpp"
#include "OwnedCounselorStartup.hpp"
#include "LoadedCounselorResolver.hpp"
namespace su=cfgmenu::startup;
static void __fastcall StartupRestart(void*,void*){}
static std::uintptr_t __fastcall StartupGetter(void*,void*){return 0;}
struct StartupPublication {
    StartingFixture& f;unsigned exchanges=0,allocations=0;bool ready=true;
    static constexpr std::uintptr_t Clone=0xD00000;
    DWORD Thread()const{return 42;}
    bool Ready(const cfgmenu::promotion::Context&,const cfgmenu::promotion::gate::Gates&){return ready;}
    std::uintptr_t Allocate(const cfgmenu::promotion::gate::Table& table){++allocations;f.reader.Set(Clone,table);return Clone;}
    void Free(std::uintptr_t){}
    bool CompareExchange(std::uintptr_t object,std::uintptr_t expected,std::uintptr_t desired,std::uintptr_t& previous){
        ++exchanges;if(!f.reader.Read(object,previous))return false;if(previous==expected)f.reader.Set(object,desired);return true;}
};
struct StartupCalls {
    StartingFixture& f;unsigned copies=0,consumes=0,restarts=0,destroys=0;std::function<void()> inRestart;
    DWORD Thread()const{return 42;}bool Faulted()const{return false;}
    bool Ready(const cfgmenu::promotion::Context&){return true;}
    bool Copy(const cfgmenu::promotion::Context& c,cfgmenu::promotion::Packet& out){++copies;out=c.picked;return true;}
    bool Consume(const cfgmenu::promotion::Context& c,cfgmenu::promotion::Packet& value){
        ++consumes;f.reader.Set(c.playerState+cfgmenu::birthdisk::ActiveCharacterClass,value);value={};return true;}
    bool Destroy(const cfgmenu::promotion::Context&,cfgmenu::promotion::Packet&){++destroys;return true;}
    bool Get(const cfgmenu::promotion::Context& c,std::uintptr_t& out){out=c.selectedClass;return true;}
    bool Restart(const cfgmenu::promotion::Context&){++restarts;if(inRestart)inRestart();return true;}
};
template class su::OwnedCounselorStartup<OwnedByteReader,cfgmenu::promotion::gate::WindowsPublication,cfgmenu::promotion::WindowsCalls>;
int main(){unsigned failures=0,checks=0;const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;
    for(unsigned fault=0;fault<9;++fault){StartingFixture f;
        for(unsigned i=0;i<gatedisk::Slots;++i)f.reader.Set(f.Module+gatedisk::Table+8*i,f.Module+gatedisk::Entries[i]);
        Session session;Capabilities caps;caps.automaticJason=true;
        check(session.Open(101,Mode::Counselor,caps,9)&&session.PickNativeCustomization(101,session.Generation(),11,22)&&
            session.Commit(101,session.Generation()),"current counselor selection committed");
        GameplayHandoff handoff(session);MenuCatalogueEvidence evidence;
        evidence.frontend=101;evidence.generation=session.Generation();evidence.catalogueGeneration=1;evidence.frontendWorld=0xE00000;
        evidence.map=11;evidence.jason=22;evidence.maximumBots=8;evidence.selectedJason[16]=7;
        evidence.cfgMetadata=evidence.ownedSelections=evidence.preserveNativeCounselor=true;
        check(handoff.Arm(evidence),"one current route armed");
        freshgame::Source source{f.Module,f.Engine,f.Viewport,f.Player,f.Instance,evidence.frontendWorld,42};
        StartupPublication publication{f};StartupCalls calls{f,0,0,0,0,{}};
        su::OwnedCounselorStartup<OwnedByteReader,StartupPublication,StartupCalls> startup(f.reader,publication,calls,handoff,source,101,session.Generation());
        promotion::gate::Gates gates{&StartupRestart,&StartupGetter,0xF00000};
        if(fault==1)f.event.function=f.Parameter;
        if(fault==2)f.event.thread=41;
        if(fault==3)handoff.Cancel();
        if(fault==4)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==5)publication.ready=false;
        if(fault==7)f.reader.Set(f.Registry+24+16,int{0});
        if(fault==8)f.reader.Set(f.State+birthdisk::PickedCounselorClass,promotion::Packet{});
        const auto result=startup.Before(f.event,gates,fault!=6,[&](const auto&){return Resolver(f);});
        // PickedCounselorClass runtime UProperty flags are not proved for 3.5
        // (StartingPlayerBindings PickedFlagsVerified=false): the producer must
        // then refuse the otherwise valid owned prebirth install, fail closed.
        constexpr bool proved=cfgmenu::startingdisk::PickedFlagsVerified;
        check(result==(fault==0?(proved?su::Result::Installed:su::Result::Refused):fault<=3?su::Result::Ignored:su::Result::Refused),"exact owned prebirth install only");
        check(!calls.copies&&!calls.consumes&&!calls.restarts,"install never itself promotes/respawns");
        check(publication.exchanges==(fault||!proved?0u:1u),"one validated instance publication");
        if(!fault&&proved){
            const auto early=startup.PawnClass(f.Mode,f.Controller,42);check(early.handled&&!early.type,"pre-restart getter cannot default to Jason");
            calls.inRestart=[&](){const auto selected=startup.PawnClass(f.Mode,f.Controller,42);
                check(selected.handled&&selected.type==f.Selected,"native restart getter retains exact native chosen counselor");};
            check(startup.Restart(f.Mode,f.Controller,42)&&startup.BirthResult()==promotion::Result::Forwarded,"owned native restart exactly once");
            check(calls.copies==1&&calls.consumes==1&&calls.restarts==1&&!calls.destroys,"consuming native setter owns packet cleanup");
            check(startup.Restart(f.Mode,f.Controller,42)&&calls.restarts==1,"duplicate cannot spawn again");
            check(!startup.Restart(f.Mode,f.State,42)&&!startup.PawnClass(f.Mode,f.State,42).handled,"unrelated native controller not intercepted");
            check(!startup.Restart(f.Mode,f.Controller,41),"foreign thread route must forward natively");
        }
        const auto before=f.reader.reads;check(startup.Before(f.event,gates,true,[&](const auto&){return Resolver(f);})==su::Result::Ignored,
            "no automatic retry after installed/refused/unclaimed callback");
        check(f.reader.reads==before+(fault==1?1u:0u),"duplicate/foreign callback zero reads; unrelated function one cheap cache check");
    }
    std::printf("CFG exact producer/gate/native-promotion composition: %s (%u checks, %u failures). Owned fixtures only.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
