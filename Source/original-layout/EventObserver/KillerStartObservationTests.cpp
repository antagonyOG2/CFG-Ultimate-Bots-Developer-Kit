#include "KillerStartObservation.hpp"
#include <map>
#include <cstring>
#include <cstdio>
struct Reader {
    std::map<std::uintptr_t,unsigned char> memory;unsigned reads=0,retiredReads=0;
    std::uintptr_t retiredBegin=0,retiredEnd=0;
    template<class T> void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)memory[at+i]=p[i];}
    template<class T> bool Read(std::uintptr_t at,T& value){++reads;
        if(retiredBegin&&at<retiredEnd&&at+sizeof(T)>retiredBegin){++retiredReads;return false;}
        auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto it=memory.find(at+i);if(it==memory.end())return false;p[i]=it->second;}return true;}
};
struct Host {bool current=true;bool Current(){return current;}};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,World=0x200000,Levels=0x210000,Level=0x220000,Actors=0x230000,
        Actor=0x240000,Type=0x250000,LevelType=0x260000,Table=0x270000,Other=0x280000;
    Reader reader;Host host;cfgkillerstart::Observation observation;
    Fixture(){using namespace cfgkillerstart;using namespace cfgprobe;
        reader.Set(Module+disk::ClassCache,Type);reader.Set(Module+disk::ConstructorCache,Type);
        reader.Set(Module+cfgradarworld::disk::LevelClassCache,LevelType);reader.Set(Module+cfgradarworld::disk::LevelConstructorCache,LevelType);
        reader.Set(World+0x110,Header{Levels,1,1});reader.Set(Levels,Level);reader.Set(Level+0x20,World);
        reader.Set(Level+cfgkill::disk::OwningWorld,World);
        reader.Set(Level+ObjectClass,LevelType);reader.Set(LevelType+ClassSuper,std::uintptr_t{});
        reader.Set(Level+0xA0,Header{Actors,33,33});
        for(unsigned i=0;i<33;++i)reader.Set(Actors+8*i,i==20?Actor:std::uintptr_t{});
        reader.Set(Actor+ObjectClass,Type);reader.Set(Type+ClassSuper,std::uintptr_t{});reader.Set(Type+0xF8,Other);
        reader.Set(Actor+0x20,Level);reader.Set(Actor+0x0C,int{0});
        reader.Set(Module+lifetime::ObjectArrayRva+0x10,Table);reader.Set(Module+lifetime::ObjectArrayRva+0x1C,int{2});
        reader.Set(Table,Actor);reader.Set(Table+0x10,int{42});
    }
    bool Begin(){return observation.Begin(reader,host,Module,World);}
    bool Finish(){for(unsigned i=0;i<15&&observation.CurrentState()!=cfgkillerstart::State::Ready;++i)if(!observation.Step(reader,host))return false;return observation.CurrentState()==cfgkillerstart::State::Ready;}
};
int main(){unsigned checks=0,failures=0;auto check=[&](bool condition,const char* name){++checks;if(!condition){++failures;std::printf("FAIL %s\n",name);}};
    {Fixture f;check(f.Begin(),"startup larger slice begins");
     check(f.observation.Step(f.reader,f.host,32)&&f.observation.CurrentState()==cfgkillerstart::State::Verifying,"32-cell slice reaches marker without publishing early");
     check(f.observation.Step(f.reader,f.host,32)&&f.observation.CurrentState()==cfgkillerstart::State::Ready,"second bounded pass still required");}
    {Fixture f;check(f.Begin(),"owned roster begins");auto before=f.reader.reads;check(f.observation.Step(f.reader,f.host),"bounded first step");
     check(f.observation.CurrentState()==cfgkillerstart::State::Scanning&&f.reader.reads-before<200,"sixteen cells only; first match not returned");
     check(f.Finish(),"two prefix passes select first native start in base-mod order");cfgkillerstart::Marker marker;
     check(f.observation.CurrentMarker(f.reader,f.host,marker)&&marker.actor==f.Actor&&marker.identity.serial==42,"positive twice checked result");
     f.reader.Set(f.Actors+20*8,std::uintptr_t{});check(!f.observation.CurrentMarker(f.reader,f.host,marker)&&!marker.actor,"removed marker refused before use");}
    for(unsigned fault=0;fault<12;++fault){Fixture f;check(f.Begin(),"fault fixture begins");
        if(fault==0)f.host.current=false;
        if(fault==1)f.reader.Set(f.Module+cfgkillerstart::disk::ConstructorCache,f.Other);
        if(fault==2)f.reader.Set(f.Level+cfgkill::disk::OwningWorld,f.Other);
        if(fault==3)f.reader.Set(f.World+0x110,cfgkillerstart::Header{f.Levels,2,2});
        if(fault==4)f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,20,33});
        if(fault==5)f.reader.Set(f.Actor+0x20,f.World);
        if(fault==6)f.reader.Set(f.Type+0xF8,f.Actor);
        if(fault==7){check(f.observation.Step(f.reader,f.host)&&f.observation.Step(f.reader,f.host),"capture assigned marker before serial mutation");f.reader.Set(f.Table+0x10,int{43});}
        if(fault==8){check(f.observation.Step(f.reader,f.host),"sample initial prefix");f.reader.Set(f.Actors,f.Actor);}
        if(fault==9)f.reader.Set(f.Actors+20*8,std::uintptr_t{});
        if(fault==10)f.reader.Set(f.Type+cfgprobe::ClassSuper,f.Type);
        if(fault==11)f.reader.Set(f.Table,f.Other);
        check(!f.Finish()&&f.observation.CurrentState()==cfgkillerstart::State::Refused,"changed context/cache/owner/header/template/identity/prefix/missing refused");}
    {Fixture f;check(f.Begin(),"changed sampled cell fixture");check(f.observation.Step(f.reader,f.host),"first cells sampled");
     f.reader.Set(f.Actors,f.Actor);check(!f.Finish(),"earlier cell mutation caught by second pass");}
    {Fixture f;f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,100001,100001});check(!f.Begin(),"base actor bound");}
    {Fixture f;f.reader.Set(f.World+0x110,cfgkillerstart::Header{f.Levels,33,33});check(!f.Begin(),"level bound");}
    {Fixture f;f.reader.Set(f.Actors+21*8,f.Actor);check(f.Begin()&&f.Finish(),"later marker cells do not change base-mod first-match selection");}
    {Fixture f;f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,33,100000});check(f.Begin()&&f.Finish(),"large reserved capacity adds no extra reads or allocations");}
    {Fixture f;f.reader.Set(f.Level+0x20,f.Other);check(f.Begin()&&f.Finish(),"streamed-level source Outer differs; exact native OwningWorld remains required");}
    {Fixture f;f.reader.Set(f.Module+cfgkillerstart::disk::ClassCache,std::uintptr_t{});check(!f.Begin()&&f.observation.LastFailure()==cfgkillerstart::Failure::ClassCache,"cold class failure explicitly reported");}
    {Fixture f;f.reader.Set(f.Table+0x10,int{0});check(f.Begin()&&f.Finish(),"authored serial0 marker is only a strong owned transform observation");
     cfgkillerstart::Marker marker;check(f.observation.CurrentMarker(f.reader,f.host,marker)&&marker.identity.serial==0,"serial0 fresh exact cell/slot/type/Outer proof");
     f.reader.Set(f.Table+0x10,int{43});check(f.observation.CurrentMarker(f.reader,f.host,marker)&&marker.identity.serial==43,"corroborated0-to-positive assignment strengthens marker observation");
     f.reader.Set(f.Table+0x10,int{44});check(!f.observation.CurrentMarker(f.reader,f.host,marker),"originally positive serial change refuses stale candidate");}
    {Fixture f;check(f.Begin(),"append fixture begins");
     for(unsigned i=33;i<50;++i)f.reader.Set(f.Actors+8*i,std::uintptr_t{});
     f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,50,100});
     check(f.Finish()&&f.observation.AppendUpdates()==1&&!f.observation.Restarts(),"same-storage append preserves prefix without restart");
     f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,50,200});cfgkillerstart::Marker marker;
     check(f.observation.CurrentMarker(f.reader,f.host,marker)&&f.observation.AppendUpdates()==2,"capacity-only change preserves exact ready marker");}
    {Fixture f;check(f.Begin()&&f.observation.Step(f.reader,f.host),"relocation fixture sampled first prefix");
     constexpr std::uintptr_t moved=0x290000;
     for(unsigned i=0;i<33;++i)f.reader.Set(moved+8*i,i==20?f.Actor:std::uintptr_t{});
     f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{moved,33,64});
     f.reader.retiredBegin=f.Actors;f.reader.retiredEnd=f.Actors+33*8;
     check(f.Finish()&&f.observation.Restarts()==1&&f.observation.ChangedSource()==2,"actor storage relocation recaptures next phase and completes");
     check(!f.reader.retiredReads,"retired actor storage never dereferenced");}
    {Fixture f;check(f.Begin(),"world list relocation fixture");constexpr std::uintptr_t moved=0x290000;
     f.reader.Set(moved,f.Level);f.reader.Set(f.World+0x110,cfgkillerstart::Header{moved,1,8});
     f.reader.retiredBegin=f.Levels;f.reader.retiredEnd=f.Levels+8;
     check(f.Finish()&&f.observation.Restarts()==1&&f.observation.ChangedSource()==1&&!f.reader.retiredReads,"world-level storage relocation stays owned and never reads retired list");}
    {Fixture f;check(f.Begin(),"legal shrink fixture");f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,32,33});
     check(f.Finish()&&f.observation.Restarts()==1,"truncation outside surviving marker restarts full proof safely");}
    {Fixture f;check(f.Begin(),"repeated churn fixture");bool okay=true;
     constexpr std::uintptr_t moved=0x290000;
     for(unsigned i=0;i<33;++i)f.reader.Set(moved+8*i,i==20?f.Actor:std::uintptr_t{});
     for(unsigned n=0;n<=cfgkillerstart::MaximumRestarts;++n){
        f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{n%2?f.Actors:moved,33,33});
        okay=f.observation.Step(f.reader,f.host);
        if(!okay)break;
        check(f.observation.Step(f.reader,f.host),"recapture gets separate scheduled phase");
     }
     check(!okay&&f.observation.Restarts()==cfgkillerstart::MaximumRestarts&&f.observation.LastFailure()==cfgkillerstart::Failure::ChangedHeader,"persistent churn stops at bounded restart limit");}
    {Fixture f;check(f.Begin()&&f.Finish(),"ready relocation fixture");
     f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{0x290000,33,64});
     f.reader.retiredBegin=f.Actors;f.reader.retiredEnd=f.Actors+33*8;cfgkillerstart::Marker marker;
     check(!f.observation.CurrentMarker(f.reader,f.host,marker)&&!marker.actor&&!f.reader.retiredReads,"ready marker never adopted after storage move without new proof");}
    {Fixture f;constexpr std::uintptr_t second=0x2C0000,secondActors=0x300000,newFirst=0x2B0000;
     f.reader.Set(f.World+0x110,cfgkillerstart::Header{f.Levels,2,2});f.reader.Set(f.Levels+8,second);
     f.reader.Set(second+cfgkill::disk::OwningWorld,f.World);f.reader.Set(second+cfgprobe::ObjectClass,f.LevelType);
     f.reader.Set(second+0xA0,cfgkillerstart::Header{secondActors,33,33});f.reader.Set(f.Actor+0x20,second);
     for(unsigned i=0;i<33;++i){f.reader.Set(f.Actors+8*i,std::uintptr_t{});f.reader.Set(secondActors+8*i,i==20?f.Actor:std::uintptr_t{});}
     check(f.Begin(),"two-level order fixture begins");
     for(unsigned i=0;i<10&&f.observation.CurrentState()!=cfgkillerstart::State::Verifying;++i)
         check(f.observation.Step(f.reader,f.host),"bounded first selection in later level");
     check(f.observation.CurrentState()==cfgkillerstart::State::Verifying,"later-level marker selected provisionally");
     f.reader.Set(newFirst+cfgprobe::ObjectClass,f.Type);f.reader.Set(newFirst+0x20,f.Level);f.reader.Set(newFirst+0xC,int{1});
     f.reader.Set(f.Table+24,newFirst);f.reader.Set(f.Table+24+16,int{50});
     f.reader.Set(f.Actors+33*8,newFirst);f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{f.Actors,34,64});
     check(f.Finish()&&f.observation.Restarts()==1,"append to earlier completed level restarts first-match proof");
     cfgkillerstart::Marker marker;check(f.observation.CurrentMarker(f.reader,f.host,marker)&&marker.actor==newFirst,"native first-level marker wins after restart");
     f.reader.Set(second+0xA0,cfgkillerstart::Header{0x310000,0,0});
     check(f.observation.CurrentMarker(f.reader,f.host,marker)&&marker.actor==newFirst,"unrelated later actor array cannot invalidate ready prefix");}
    std::printf("Killer start owned-fixture observation: %u checks, %u failures. No game access.\n",checks,failures);return failures?1:0;
}
