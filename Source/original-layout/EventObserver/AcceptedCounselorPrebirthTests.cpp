#define CFG_STARTING_FIXTURE_ONLY
#include "StartingPlayerObservationTests.cpp"
#include "AcceptedCounselorPrebirth.hpp"
namespace pb=cfgmenu::prebirth;
struct PrebirthFixture:StartingFixture {
    static constexpr std::uintptr_t StateTable=0x950000;
    cfgmenu::Session session;cfgmenu::GameplayHandoff handoff{session};
    cfgmenu::MenuCatalogueEvidence evidence{};
    cfgmenu::freshgame::Source source{Module,Engine,Viewport,Player,Instance,0xE00000,42};
    bool armed=false;
    explicit PrebirthFixture(bool native=false,const char* path="/Game/Characters/Counselors/Rocker/Rocker_Counselor.Rocker_Counselor_C"){
        using namespace cfgmenu;
        reader.Set(Module+gatedisk::Table+birthdisk::RestartSlot,Module+birthdisk::Restart);
        reader.Set(Module+gatedisk::Table+birthdisk::GetterSlot,Module+birthdisk::Getter);
        reader.Set(State,StateTable);reader.Set(StateTable+0x858,Module+birthdisk::CanSpawnActive);
        Name(Selected,path);cfgnames::Words name{};reader.Read(Selected+cfgnames::disk::ObjectName,name);
        std::memcpy(evidence.selectedCounselor.data()+16,&name,8);
        // Deliberately stale frontend weak cache must never reach native birth.
        for(unsigned i=0;i<16;++i)evidence.selectedCounselor[i]=0xA5;
        Name(ModeDefault,"/Game/Characters/Counselors/Athlete/Athlete_Counselor.Athlete_Counselor_C");
        cfgnames::Words vanessa{};reader.Read(ModeDefault+cfgnames::disk::ObjectName,vanessa);
        std::memcpy(pick.bytes.data()+16,&vanessa,8);reader.Set(State+birthdisk::PickedCounselorClass,pick);
        reader.Set(State+birthdisk::ActiveCharacterClass,promotion::Packet{});
        Capabilities caps;caps.automaticJason=true;
        evidence.frontend=101;evidence.frontendWorld=source.frontendWorld;evidence.catalogueGeneration=1;
        evidence.map=11;evidence.jason=22;evidence.maximumBots=8;evidence.selectedJason[16]=7;
        evidence.cfgMetadata=evidence.ownedSelections=true;evidence.preserveNativeCounselor=native;
        evidence.counselorPathSelection=!native;evidence.counselor=native?0:softpath::Token(evidence.selectedCounselor);
        armed=session.Open(101,Mode::Counselor,caps,9);
        evidence.generation=session.Generation();
        armed=armed&&(native?session.PickNativeCustomization(101,session.Generation(),11,22):
            session.Pick(101,session.Generation(),11,22,evidence.counselor));
        armed=armed&&session.Commit(101,session.Generation())&&handoff.Arm(evidence);
        reader.reads=0;
    }
};
struct PrebirthCalls {
    PrebirthFixture& f;unsigned consumes=0,readyCalls=0;DWORD thread=42;
    bool ready=true,success=true,write=true;std::function<void()> during;
    cfgmenu::promotion::Packet received{};
    DWORD Thread()const{return thread;}
    bool Ready(const cfgmenu::promotion::Context&){++readyCalls;return ready;}
    bool Consume(const cfgmenu::promotion::Context& c,cfgmenu::promotion::Packet& packet){
        ++consumes;received=packet;
        if(write)f.reader.Set(c.playerState+cfgmenu::birthdisk::ActiveCharacterClass,packet);
        if(during)during();return success;
    }
    // No Restart/Get/Copy/Destroy methods: the helper must use only Consume.
};
struct CallbackReader {
    OwnedByteReader& underlying;std::uintptr_t trigger=0;std::function<void()> callback;
    template<class T>bool Read(std::uintptr_t at,T& out){
        const bool okay=underlying.Read(at,out);
        if(at==trigger&&callback){auto once=std::move(callback);once();}return okay;
    }
};
template bool pb::AcceptedCounselorPrebirth::Before<OwnedByteReader,cfgmenu::promotion::WindowsCalls>(
    OwnedByteReader&,cfgmenu::promotion::WindowsCalls&,const cfgmenu::starting::Event&,
    const cfgmenu::freshgame::Source&,const cfgmenu::GameplayHandoff&);
int main(){
    unsigned checks=0,failures=0,maximumReads=0;const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const auto countReads=[&](const OwnedByteReader& reader){maximumReads=(std::max)(maximumReads,reader.reads);
        check(reader.reads<32768,"total prebirth reads fit production callback budget");};
    using namespace cfgmenu;
    for(unsigned fault=0;fault<23;++fault){
        PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        check(f.armed,"real accepted-path handoff armed");
        if(fault==1)f.event.thread=41;
        if(fault==2)f.event.exactImageVerified=false;
        if(fault==3)f.event.function=f.Parameter;
        if(fault==4)f.reader.Set(f.Packet,f.OtherWidget);
        if(fault==5)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.State);
        if(fault==6)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.Selected);
        if(fault==7)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.Selected);
        if(fault==8)f.reader.Set(f.State+birthdisk::SpawnedCharacterClass,f.Selected);
        if(fault==9)f.reader.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
        if(fault==10)f.reader.Set(f.Module+gatedisk::Table+birthdisk::RestartSlot,f.Module+birthdisk::Getter);
        if(fault==11)f.reader.Set(f.Module+gatedisk::Table+birthdisk::GetterSlot,f.Module+birthdisk::Restart);
        if(fault==12)calls.ready=false;
        if(fault==13)calls.thread=41;
        if(fault==14)f.reader.Set(f.Registry+24*3+16,int{0});
        if(fault==15)f.handoff.Cancel();
        if(fault==16)f.source.engine=f.Mode;
        if(fault==17)f.reader.Set(f.State+0x88,f.Mode);
        if(fault==18)f.reader.Set(f.State+0x110,static_cast<unsigned char>(2));
        if(fault==19)f.reader.Set(f.Module+startingdisk::Executor,static_cast<unsigned char>(0));
        if(fault==20)f.reader.Set(f.Function+0x8E,std::uint16_t{16});
        if(fault==21)f.reader.Set(f.StateTable+0x858,f.Module+birthdisk::Getter);
        if(fault==22)f.event.module+=0x10000;
        const auto okay=birth.Before(f.reader,calls,f.event,f.source,f.handoff);
        check(okay==(fault==0),"concrete event/native-slot/owner checks");
        check(calls.consumes==(fault==0?1u:0u),"invalid owner never calls native setter");
        if(!fault){
            check(f.evidence.counselor!=f.Selected,"accepted token is not a UClass pointer");
            check(softpath::Same(calls.received.bytes,f.evidence.selectedCounselor)&&
                !softpath::Same(calls.received.bytes,f.pick.bytes),"AJ frozen choice wins over live Vanessa picked value");
            promotion::Packet canonical;check(pb::PacketForBirth(f.evidence.selectedCounselor,canonical)&&
                canonical.bytes==calls.received.bytes,"stale frontend weak cache is canonically cleared");
            check(birth.CurrentState()==pb::State::Applied&&birth.Report().attempts==1&&birth.Report().returns==1,"one setter applied and verified");
            promotion::Packet picked;f.reader.Read(f.State+birthdisk::PickedCounselorClass,picked);
            check(picked.bytes==f.pick.bytes,"native picked/profile source remains untouched");
        }
        if(birth.CurrentState()!=pb::State::Empty){const auto reads=f.reader.reads,consumes=calls.consumes;
            check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&f.reader.reads==reads&&calls.consumes==consumes,
                "completed/refused attempt is never retried");}
        check(f.reader.reads<starting::MaximumReads,"bounded fixture observation");
        countReads(f.reader);
    }
    {PrebirthFixture f(true);PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        check(f.armed&&!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&!f.reader.reads&&!calls.consumes,
            "preserved ordinary native counselor route performs zero reads/calls");}
    {PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        f.reader.Set(f.Packet,f.OtherWidget);
        check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&birth.CurrentState()==pb::State::Empty,"unrelated NewPlayer leaves human opportunity open");
        f.reader.Set(f.Packet,f.Controller);
        check(birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&calls.consumes==1,"human arrival after unrelated player still applies");}
    {PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        calls.during=[&](){const auto reads=f.reader.reads;
            check(birth.CurrentState()==pb::State::Applying,"setter broadcast sees Applying state");
            check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&reads==f.reader.reads,"nested exact callback cannot reenter setter");};
        check(birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&calls.consumes==1,"outer setter succeeds despite ignored nested callback");}
    for(unsigned fault=0;fault<8;++fault){PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        if(fault==0)calls.write=false;
        if(fault==1)calls.success=false;
        if(fault==2)calls.during=[&](){f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.Selected);};
        if(fault==3)calls.during=[&](){f.reader.Set(f.Registry+24*3+16,int{999});};
        if(fault==4)calls.during=[&](){f.reader.Set(f.State+birthdisk::ActiveCharacterClass,f.pick);};
        if(fault==5)calls.during=[&](){f.handoff.Cancel();};
        if(fault==6)calls.during=[&](){f.reader.Set(f.StateTable+0x858,f.Module+birthdisk::Getter);};
        if(fault==7)calls.during=[&](){f.reader.Set(f.Module+gatedisk::Table+birthdisk::GetterSlot,f.Module+birthdisk::Restart);};
        check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&calls.consumes==1,"failed setter/readback/changed ownership refuses");
        check(birth.CurrentState()==(fault==1?pb::State::Indeterminate:pb::State::Refused),"native fault is distinguished from verified refusal");
        const auto reads=f.reader.reads;check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&reads==f.reader.reads&&calls.consumes==1,"failure never triggers retry or cleanup call");
        countReads(f.reader);}
    {PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        promotion::Packet active;active.bytes=f.evidence.selectedCounselor;
        f.reader.Set(f.State+birthdisk::ActiveCharacterClass,active);calls.write=false;
        check(birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&calls.consumes==1&&birth.CurrentState()==pb::State::Applied,
            "native equal AJ path no-op setter still verifies successfully");
        promotion::Packet after;f.reader.Read(f.State+birthdisk::ActiveCharacterClass,after);
        check(after.bytes==active.bytes,"equal-path native no-op need not rewrite weak cache");countReads(f.reader);}
    {PrebirthFixture f(false,"/Game/Characters/Counselors/Flirt/Flirt_Counselor.Flirt_Counselor_C");
        PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        check(f.armed&&birth.Before(f.reader,calls,f.event,f.source,f.handoff),"another female counselor path is accepted without hardcoded AJ");
        promotion::Packet canonical;check(pb::PacketForBirth(f.evidence.selectedCounselor,canonical)&&canonical.bytes==calls.received.bytes,
            "female counselor packet retains accepted path and clears old weak cache");countReads(f.reader);}
    {PrebirthFixture f;PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        CallbackReader reader{f.reader,f.Module+gatedisk::Table+birthdisk::GetterSlot,[&](){f.handoff.Cancel();}};
        check(!birth.Before(reader,calls,f.event,f.source,f.handoff)&&!calls.consumes,"handoff changed during observation blocks setter");}
    for(const char* path:{"/Game/Characters/Killers/Part3.Part3_C","/Game/Characters/Counselors/Rocker_NoAssetSeparator"}){
        PrebirthFixture f(false,path);PrebirthCalls calls{f};pb::AcceptedCounselorPrebirth birth;
        check(!birth.Before(f.reader,calls,f.event,f.source,f.handoff)&&!calls.consumes,"invalid counselor asset path never calls setter");}
    {cfgprimary::SoftClassBytes invalid{};promotion::Packet packet;
        check(!pb::PacketForBirth(invalid,packet),"empty asset path refuses");
        invalid[16]=1;invalid[24]=1;check(!pb::PacketForBirth(invalid,packet),"borrowed subobject pointer refuses");}
    std::printf("CFG accepted counselor prebirth: %s (%u checks, %u failures; maximum reads=%u/32768). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",checks,failures,maximumReads);
    return failures?1:0;
}
