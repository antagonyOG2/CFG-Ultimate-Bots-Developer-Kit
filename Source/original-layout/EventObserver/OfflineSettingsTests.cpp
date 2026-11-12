#include "NativeOfflineSettings.hpp"
#include "NativeSettingsMenuOptions.hpp"
#include "NativeSkillItems.hpp"
#include <map>
#include <cstdio>
using namespace cfgofflinesettings;
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;
    template<class T> void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    template<class T> bool Read(std::uintptr_t at,T& value){++reads;auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto found=bytes.find(at+i);if(found==bytes.end())return false;p[i]=found->second;}return true;}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t size){++reads;auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<size;++i){auto found=bytes.find(at+i);if(found==bytes.end())return false;p[i]=found->second;}return true;}
};
struct Host {
    Memory& memory;bool current=true,beforeHelpers=true;unsigned writes=0,failWrite=0;
    bool Current(const Request&){return current;}
    bool BeforeHelpers(const Request&){return beforeHelpers;}
    Write ExchangeByte(std::uintptr_t at,std::uint8_t oldValue,std::uint8_t next){
        ++writes;if(failWrite==writes)return Write::Refused;
        std::uint8_t prior=0;if(!memory.Read(at,prior)||prior!=oldValue)return Write::Refused;memory.Set(at,next);return Write::Changed;
    }
    Write Exchange(std::uintptr_t at,std::int32_t oldValue,std::int32_t next){
        ++writes;if(failWrite==writes)return Write::Refused;
        std::int32_t prior=0;if(!memory.Read(at,prior)||prior!=oldValue)return Write::Refused;memory.Set(at,next);return Write::Changed;
    }
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,World=0x200000,Pawn=0x210000,Mode=0x220000,GameState=0x230000,
        ModeClass=0x240000,SandboxClass=0x250000,StateClass=0x260000,BaseClass=0x270000,Table=0x280000,
        Names=0x290000,Chunk=0x2A0000,Entry=0x2B0000;
    Memory memory;Host host{memory};Request request{7,World,Pawn,100,101,3600};Context context{Module,Mode,GameState};
    NativeAccess<Memory,Host> access{memory,host,request,context};
    Fixture(){using namespace cfgprobe;
        memory.Set(Module+disk::SCGameModeClass,ModeClass);memory.Set(Module+disk::SCGameModeConstructor,ModeClass);
        memory.Set(Module+disk::SCGameMode_SandboxClass,SandboxClass);memory.Set(Module+disk::SCGameMode_SandboxConstructor,SandboxClass);
        memory.Set(Module+disk::SCGameStateClass,StateClass);memory.Set(Module+disk::SCGameStateConstructor,StateClass);
        memory.Set(Module+disk::GameModeClass,BaseClass);memory.Set(Module+disk::GameModeConstructor,BaseClass);
        memory.Set(Mode+ObjectClass,SandboxClass);memory.Set(SandboxClass+ClassSuper,ModeClass);memory.Set(ModeClass+ClassSuper,BaseClass);
        memory.Set(BaseClass+ClassSuper,std::uintptr_t{});memory.Set(GameState+ObjectClass,StateClass);memory.Set(StateClass+ClassSuper,std::uintptr_t{});
        memory.Set(SandboxClass+0xF8,std::uintptr_t{0x300000});memory.Set(StateClass+0xF8,std::uintptr_t{0x310000});
        memory.Set(World+WorldNetDriver,std::uintptr_t{});memory.Set(World+WorldGameMode,Mode);memory.Set(World+WorldGameState,GameState);
        memory.Set(Module+disk::TimerCopy,disk::TimerCopyCode);memory.Set(Module+disk::TimerDecrement,disk::TimerDecrementCode);memory.Set(Module+disk::ElapsedIncrement,disk::ElapsedIncrementCode);
        memory.Set(Module+disk::SkillConsumer,disk::SkillConsumerCode);memory.Set(Mode+disk::CounselorSkill,std::uint8_t{0});
        memory.Set(Module+lifetime::ObjectArrayRva+0x10,Table);memory.Set(Module+lifetime::ObjectArrayRva+0x1C,int{4});
        std::array<std::uintptr_t,4> objects{{World,Pawn,Mode,GameState}};
        for(unsigned i=0;i<objects.size();++i){memory.Set(objects[i]+0xC,static_cast<int>(i));memory.Set(Table+i*24,objects[i]);memory.Set(Table+i*24+16,100+static_cast<int>(i));}
        memory.Set(Mode+disk::RoundTime,int{1200});memory.Set(GameState+disk::RemainingTime,int{1189});memory.Set(GameState+disk::ElapsedInProgressTime,int{11});
        memory.Set(Mode+disk::MatchState,cfgnames::Words{0,0});memory.Set(Module+cfgnames::disk::Global,Names);
        struct Counts {int entries,chunks;};memory.Set(Names+cfgnames::disk::ElementCount,Counts{1,1});memory.Set(Names,Chunk);memory.Set(Chunk,Entry);
        memory.Set(Entry,std::uint32_t{});Phase("InProgress");
    }
    void Phase(const char* text){std::array<unsigned char,128> storage{};for(unsigned i=0;text[i]&&i<127;++i)storage[i]=static_cast<unsigned char>(text[i]);memory.Set(Entry+cfgnames::disk::EntryText,storage);}
};
int main(){unsigned tests=0,failures=0;auto check=[&](bool value,const char* label){++tests;if(!value){++failures;std::printf("FAIL %s\n",label);}};
    {Fixture f;Runtime runtime;check(runtime.Begin(f.request,100),"begin committed duration");check(runtime.Step(f.access,100),"apply early owned duration");
     int round=0,left=0;f.memory.Read(f.Mode+disk::RoundTime,round);f.memory.Read(f.GameState+disk::RemainingTime,left);
     check(round==3600&&left==3589&&f.host.writes==2,"native elapsed eleven seconds preserved");check(!runtime.Step(f.access,1000)&&f.host.writes==2,"no repeat timer reset");
     std::printf("Duration full native fixture reads=%u\n",f.memory.reads);}
    {Fixture f;Runtime runtime;f.request.seconds=0;check(runtime.Begin(f.request,0)&&runtime.CurrentState()==State::Preserved,"disabled setting preserves native");check(!runtime.Step(f.access,10)&&!f.host.writes&&!f.memory.reads,"disabled zero work");}
    {Fixture f;f.request.seconds=UINT32_MAX;NativeAccess<Memory,Host> unlimited(f.memory,f.host,f.request,f.context);
     Runtime runtime;check(f.request.Valid()&&runtime.Begin(f.request,0)&&runtime.Step(unlimited,0),"native unlimited timer applied once");
     int round=0,left=0;f.memory.Read(f.Mode+disk::RoundTime,round);f.memory.Read(f.GameState+disk::RemainingTime,left);
     check(round==-1&&left==-1&&f.host.writes==2,"unlimited uses donor native minus-one sentinel");check(!runtime.Step(unlimited,500)&&f.host.writes==2,"unlimited has no tick maintenance");}
    {Fixture f;f.Phase("WaitingToStart");Runtime runtime;check(runtime.Begin(f.request,0),"pre-match request");check(!runtime.Step(f.access,0)&&runtime.CurrentState()==State::Waiting&&!f.host.writes,"pre-match timer untouched");
     f.Phase("InProgress");check(!runtime.Step(f.access,499),"paced no scan");check(runtime.Step(f.access,500)&&f.host.writes==2,"native in-progress transition enables once");}
    {Fixture f;f.Phase("WaitingToStart");Runtime runtime;runtime.Begin(f.request,0);check(!runtime.Step(f.access,30001)&&runtime.CurrentState()==State::Refused&&!f.host.writes,"finite arrival deadline");}
    for(unsigned fault=0;fault<12;++fault){Fixture f;Runtime runtime;runtime.Begin(f.request,0);
        if(fault==0)f.host.current=false;
        if(fault==1)f.memory.Set(f.World+cfgprobe::WorldNetDriver,std::uintptr_t{0x900000});
        if(fault==2)f.memory.Set(f.World+cfgprobe::WorldGameMode,f.GameState);
        if(fault==3)f.memory.Set(f.World+cfgprobe::WorldGameState,f.Mode);
        if(fault==4)f.memory.Set(f.Module+disk::SCGameModeConstructor,f.BaseClass);
        if(fault==5)f.memory.Set(f.Table+16,int{999});
        if(fault==6)f.memory.Set(f.Table+3*24,f.Mode);
        if(fault==7)f.memory.Set(f.SandboxClass+0xF8,f.Mode);
        if(fault==8)f.memory.Set(f.StateClass+0xF8,f.GameState);
        if(fault==9)f.memory.Set(f.GameState+disk::ElapsedInProgressTime,int{121});
        if(fault==10)f.memory.Set(f.GameState+disk::RemainingTime,int{-2});
        if(fault==11)f.memory.Set(f.Module+disk::TimerCopy,std::uint8_t{});
        check(!runtime.Step(f.access,0)&&runtime.CurrentState()==State::Refused&&!f.host.writes,"invalid context/schema/time does no writes");}
    {Fixture f;f.host.failWrite=2;Runtime runtime;runtime.Begin(f.request,0);check(!runtime.Step(f.access,0)&&runtime.CurrentState()==State::Indeterminate&&f.host.writes==2,"partial write explicit");check(!runtime.Step(f.access,1000)&&f.host.writes==2,"partial cannot retry");}
    {Fixture f;Snapshot snapshot;bool ready=false;check(f.access.Observe(snapshot,ready)&&ready,"snapshot captured");f.memory.Set(f.GameState+disk::RemainingTime,int{1188});check(f.access.Apply(snapshot)==Write::Refused&&!f.host.writes,"timer changed before transaction refused");}
    for(unsigned fault=0;fault<6;++fault){Fixture f;Snapshot snapshot;bool ready=false;check(f.access.Observe(snapshot,ready)&&ready,"paced timer snapshot");
     if(fault==1)f.host.current=false;
     if(fault==2)f.memory.Set(f.GameState+cfgprobe::ObjectClass,f.BaseClass);
     if(fault==3)f.memory.Set(f.GameState+disk::ElapsedInProgressTime,int{12});
     if(fault==4)f.memory.Set(f.Table+3*24+16,int{999});
     if(fault==5)f.memory.Set(f.Mode+disk::RoundTime,int{1199});
     check(f.access.ApplyPart(snapshot,false)==(fault==0?Write::Changed:Write::Refused),"paced timer refuses changed identity/type/elapsed/scope/value");
     check(f.host.writes==(fault?0u:1u),"at most one paced timer write");
     check(f.access.ApplyPart(snapshot,false)==Write::Refused,"observed part cannot be reused");
     if(!fault){NativeAccess<Memory,Host> next(f.memory,f.host,f.request,f.context);f.memory.Set(f.GameState+disk::ElapsedInProgressTime,int{12});
      check(next.Observe(snapshot,ready)&&ready&&next.ApplyPart(snapshot,true)==Write::Changed,"second callback reconciles NEW native elapsed time");
      int remaining=0;f.memory.Read(f.GameState+disk::RemainingTime,remaining);check(remaining==3588&&f.host.writes==2,"elapsed twelve preserved by second write");}
    }
    {const auto spec=cfgmenu::settingsoptions::MatchLength();check(spec.Valid()&&spec.preserveNativeValue&&spec.count==3&&spec.selected==1,"real native duration spec preserves live value binding");}
    for(unsigned skill=0;skill<3;++skill){Fixture f;f.request.nativeSkill=skill;f.request.seconds=0;
        NativeAccess<Memory,Host> access(f.memory,f.host,f.request,f.context);SkillRuntime runtime;
        check(runtime.Apply(access,f.request)&&runtime.CurrentState()==State::Applied,"native skill applied before helpers");
        std::uint8_t actual=3;f.memory.Read(f.Mode+disk::CounselorSkill,actual);
        check(actual==skill&&f.host.writes==(skill?1u:0u),"byte only or no-op existing value");
        check(!runtime.Apply(access,f.request),"skill consumed once");}
    for(unsigned fault=0;fault<6;++fault){Fixture f;f.request.nativeSkill=2;NativeAccess<Memory,Host> access(f.memory,f.host,f.request,f.context);SkillRuntime runtime;
        if(fault==0)f.host.beforeHelpers=false;
        if(fault==1)f.Phase("WaitingPostMatch");
        if(fault==2)f.memory.Set(f.Module+disk::SkillConsumer,std::uint8_t{});
        if(fault==3)f.memory.Set(f.Mode+disk::CounselorSkill,std::uint8_t{3});
        if(fault==4)f.memory.Set(f.GameState+disk::ElapsedInProgressTime,int{121});
        if(fault==5)f.host.current=false;
        check(!runtime.Apply(access,f.request)&&runtime.CurrentState()==State::Refused&&!f.host.writes,"skill refused unsafe arrival");}
    {Fixture f;f.request.nativeSkill=2;f.Phase("WaitingToStart");NativeAccess<Memory,Host> access(f.memory,f.host,f.request,f.context);SkillRuntime runtime;
        check(runtime.Apply(access,f.request)&&f.host.writes==1,"pre-match skill before construction");}
    {Fixture f;f.memory.Set(f.Table+2*24+16,int{0});f.memory.Set(f.Table+3*24+16,int{0});Runtime runtime;runtime.Begin(f.request,0);
        check(runtime.Step(f.access,0)&&f.host.writes==2,"native strongly held mode and state need no weak serial assignment");}
    for(unsigned selected=0;selected<3;++selected){Memory memory;struct Array {std::uintptr_t data;int count,capacity;};
        memory.Set(0x10000,Array{0x20000,3,3});memory.Set(0x11000,static_cast<int>(selected));
        // Deliberately reordered native array. Semantic output must follow text.
        const std::array<std::array<char16_t,8>,3> strings{{{{u'H',u'a',u'r',u'd',0}},{{u'E',u'a',u's',u'y',0}},{{u'N',u'o',u'r',u'm',u'a',u'l',0}}}};
        for(unsigned i=0;i<3;++i){memory.Set(0x20000+i*16,Array{0x30000+i*0x100,i==2?7:5,i==2?7:5});memory.Set(0x30000+i*0x100,strings[i]);}
        unsigned result=99;check(NativeSkillItems(memory,0x10000,0x11000,result)&&result==std::array<unsigned,3>{{2,0,1}}[selected],"native skill array order not assumed");
        memory.Set(0x20000+16,Array{0x30000,5,5});check(!NativeSkillItems(memory,0x10000,0x11000,result),"duplicate native skill refused");}
    for(unsigned selected=0;selected<3;++selected){Memory memory;struct Array {std::uintptr_t data;int count,capacity;};
        memory.Set(0x10000,Array{0x20000,3,3});memory.Set(0x11000,static_cast<int>(selected));
        const std::array<std::u16string_view,3> labels{{u"Low",u"Medium",u"High"}};
        for(unsigned i=0;i<3;++i){std::array<char16_t,8> text{};std::copy(labels[i].begin(),labels[i].end(),text.begin());
            const auto count=static_cast<int>(labels[i].size()+1);memory.Set(0x20000+16*i,Array{0x30000+0x100*i,count,count});memory.Set(0x30000+0x100*i,text);}
        unsigned semantic=99;check(NativeSkillItems(memory,0x10000,0x11000,semantic)&&semantic==selected,"donor Low/Medium/High preserves native skill semantics");}
    for(unsigned fault=0;fault<5;++fault){Fixture f;constexpr std::uintptr_t nativeMode=0x400000,nativeState=0x410000;
        const auto family=cfgmenu::offlinelifecycledisk::OfflineBotsClass,gs=cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass;
        f.memory.Set(f.Module+family,nativeMode);f.memory.Set(f.Module+family+8,nativeMode);
        f.memory.Set(f.Module+gs,nativeState);f.memory.Set(f.Module+gs+8,nativeState);
        f.memory.Set(nativeMode+cfgprobe::ClassSuper,f.ModeClass);f.memory.Set(nativeState+cfgprobe::ClassSuper,f.StateClass);
        f.memory.Set(f.Mode+cfgprobe::ObjectClass,nativeMode);f.memory.Set(f.GameState+cfgprobe::ObjectClass,nativeState);
        f.memory.Set(nativeMode+0xF8,std::uintptr_t{0x420000});f.memory.Set(nativeState+0xF8,std::uintptr_t{0x430000});
        f.memory.Set(f.Mode,f.Module+cfgmenu::offlinelifecycledisk::OfflineBotsVtable);f.memory.Set(f.Mode+0x3C0,f.GameState);
        f.context.nativeOffline=fault!=1;
        if(fault==2)f.memory.Set(f.Mode+cfgprobe::ObjectClass,f.SandboxClass);
        if(fault==3)f.memory.Set(f.GameState+cfgprobe::ObjectClass,f.StateClass);
        if(fault==4)f.memory.Set(f.Mode+0x3C0,f.World);
        NativeAccess<Memory,Host> native(f.memory,f.host,f.request,f.context);Snapshot snapshot;bool ready=false;
        check(native.Observe(snapshot,ready)==(fault==0),"explicit native settings family and state ownership");
        if(!fault)check(ready&&native.Apply(snapshot)==Write::Changed&&f.host.writes==2,"native settings owned timer writes verified");
        else check(!f.host.writes,"wrong family never writes settings");
    }
    std::printf("Offline settings owned fixtures: %u checks, %u failures. No game access.\n",tests,failures);return failures?1:0;
}
