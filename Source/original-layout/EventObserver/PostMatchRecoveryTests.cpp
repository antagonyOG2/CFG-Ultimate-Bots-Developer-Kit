#include "PostMatchRecoveryObservation.hpp"
#define main CfgExistingOutroFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    using namespace cfgoutro;
    Recovery p;CHECK(p.Next(u"InProgress",0)==Action::None);
    CHECK(p.Next(u"PostMatchOutro",100)==Action::None);CHECK(p.Next(u"PostMatchOutro",45099)==Action::None);
    CHECK(p.Elapsed(15100)==15000&&p.Elapsed(99)==0&&p.Elapsed(45100)==45000);
    CHECK(p.Next(u"PostMatchOutro",45100)==Action::Skip);p.skipped=true;
    CHECK(p.Next(u"PostMatchOutro",46100)==Action::Complete);p.finished=true;
    CHECK(p.Next(u"PostMatchOutro",90000)==Action::None);
    Recovery stalled;CHECK(stalled.Next(u"PostMatchOutro",100,true)==Action::None);
    CHECK(stalled.Next(u"PostMatchOutro",3099,true)==Action::None&&stalled.Next(u"PostMatchOutro",3100,false)==Action::None);
    CHECK(stalled.Next(u"PostMatchOutro",3100,true)==Action::Skip);
    Recovery cold;CHECK(cold.AttemptFactory(1));CHECK(!cold.AttemptFactory(1));
    CHECK(cold.AttemptFactory(2)&&cold.factoryAttempts==3);CHECK(!cold.AttemptFactory(4));
    cold.finished=true;CHECK(!cold.AttemptFactory(2));
    Recovery live;CHECK(live.Next(u"PostMatchOutro",1)==Action::None);
    CHECK(live.Next(u"LeavingMap",50000)==Action::None&&!live.observed);
    Fixture f;const auto world=f.Object(),levelType=f.Object(),settingsType=f.Object();
    const auto level=f.Object(levelType),settings=f.Object(settingsType);
    f.Put(f.module+disk::LevelClass,levelType);f.Put(f.module+disk::LevelConstructor,levelType);
    f.Put(f.module+disk::SCWorldSettingsClass,settingsType);f.Put(f.module+disk::SCWorldSettingsConstructor,settingsType);
    f.Put(world+disk::PersistentLevel,level);f.Put(level+disk::WorldSettings,settings);
    f.Put(level+disk::OwningWorld,world);f.Put(settings+0x20,level);f.Put(settings+0x110,std::uint8_t{3});
    Owner owner;f.reads=0;CHECK(ObserveOwner(f,f.objects,f.module,world,owner));const auto peak=f.reads;
    CHECK(owner.level.object==level&&owner.settings.object==settings);
    f.Put(settings+disk::LevelOutroSequence,std::uintptr_t{});f.reads=0;
    CHECK(InactiveSequence(f,f.objects,f.module,world,owner));
    const auto sequenceClass=f.Object(),playerClass=f.Object();
    const auto sequence=f.Object(sequenceClass),player=f.Object(playerClass);
    f.Put(f.module+disk::LevelSequenceActorClass,sequenceClass);f.Put(f.module+disk::LevelSequenceActorConstructor,sequenceClass);
    f.Put(f.module+disk::MovieSceneSequencePlayerClass,playerClass);f.Put(f.module+disk::MovieSceneSequencePlayerConstructor,playerClass);
    f.Put(sequence+0x20,level);f.Put(player+0x20,sequence);f.Put(sequence+disk::SequencePlayer,player);
    f.Put(settings+disk::LevelOutroSequence,sequence);f.Put(player+disk::Status,std::uint8_t{});
    f.Put(f.module+disk::IsPlayingGetter,disk::IsPlayingGetterCode);f.reads=0;
    CHECK(InactiveSequence(f,f.objects,f.module,world,owner)&&f.reads<200);
    for(unsigned state=1;state<=6;++state){f.reads=0;f.Put(player+disk::Status,static_cast<std::uint8_t>(state));
        CHECK(!InactiveSequence(f,f.objects,f.module,world,owner));}
    f.Put(player+disk::Status,std::uint8_t{});f.Put(player+0x20,settings);f.reads=0;
    CHECK(!InactiveSequence(f,f.objects,f.module,world,owner));f.Put(player+0x20,sequence);
    f.reads=f.cap;CHECK(!InactiveSequence(f,f.objects,f.module,world,owner));f.reads=0;
    for(unsigned fault=0;fault<7;++fault){auto bad=f;bad.reads=0;Owner rejected;
        if(fault==0)bad.Put(level+disk::OwningWorld,std::uintptr_t{});
        if(fault==1)bad.Put(settings+0x20,world);
        if(fault==2)bad.Put(settings+0x110,std::uint8_t{1});
        if(fault==3)bad.Put(bad.module+disk::SCWorldSettingsConstructor,levelType);
        if(fault==4)bad.Put(level+disk::WorldSettings,std::uintptr_t{});
        if(fault==5)bad.Put(settings+0x10,levelType);
        if(fault==6)bad.Put(world+disk::PersistentLevel,std::uintptr_t{});
        CHECK(!ObserveOwner(bad,bad.objects,bad.module,world,rejected));
    }
    CHECK(peak<300);std::printf("Donor45s stalled-outro policy/current native owner: %d failures, peak%u reads; no game access.\n",failures,peak);
    return failures?1:0;
}
