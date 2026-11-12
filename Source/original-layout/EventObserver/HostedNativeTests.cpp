#include "HostedRegisteredRoster.hpp"
#include "HostedBirthAdmission.hpp"
#include <map>
#include <cstdio>
#include <cstring>
namespace hn=cfghosted::native;
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,failAt=0,mutateAt=0;
    std::uintptr_t mutationAddress=0,mutationValue=0;
    template<class T> void Put(std::uintptr_t at,const T& value){const auto p=reinterpret_cast<const unsigned char*>(&value);for(std::size_t i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    template<class T> bool Read(std::uintptr_t at,T& value){if(++reads==mutateAt)Put(mutationAddress,mutationValue);if(reads==failAt)return false;T temp{};auto p=reinterpret_cast<unsigned char*>(&temp);for(std::size_t i=0;i<sizeof(T);++i){auto it=bytes.find(at+i);if(it==bytes.end())return false;p[i]=it->second;}value=temp;return true;}
};
struct Fixture {
    Memory r;hn::Admission admission{0x140000000,0x200000,0x210000,0x220000,99,true,true};
    std::uintptr_t driver=0x230000,mode=0x240000,state=0x250000,pawn=0x260000,controller=0x270000,table=0x300000;
    std::array<std::uintptr_t,6> classes{{0x400000,0x410000,0x420000,0x430000,0x440000,0x450000}};
    void Pair(std::uintptr_t a,std::uintptr_t b,std::uintptr_t type){r.Put(admission.module+a,type);r.Put(admission.module+b,type);r.Put(type+cfgprobe::ClassSuper,std::uintptr_t{});}
    void Object(std::uintptr_t object,unsigned index,std::uintptr_t type){r.Put(object+cfgprobe::ObjectClass,type);r.Put(object+0x0C,static_cast<std::int32_t>(index));r.Put(table+24*index,object);r.Put(table+24*index+16,std::int32_t(5+index));}
    Fixture(){
        using namespace cfghosted;auto m=admission.module;
        r.Put(m+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t(1));r.Put(m+cfgprobe::dispatch::ThreadIdRva,admission.ownerThread);
        Pair(disk::WorldReflection,disk::WorldConstructor,classes[0]);Pair(disk::NetDriverReflection,disk::NetDriverConstructor,classes[1]);
        Pair(disk::HuntReflection,disk::HuntConstructor,classes[2]);Pair(cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor,classes[3]);
        Pair(cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,classes[4]);
        Pair(cfgspawn::Classes[3].reflection,cfgspawn::Classes[3].constructor,classes[5]);
        const std::array<std::uintptr_t,6> objects{{admission.world,driver,mode,state,pawn,controller}};
        for(unsigned i=0;i<objects.size();++i)Object(objects[i],i,classes[i]);
        r.Put(m+cfgprobe::lifetime::ObjectArrayRva+16,table);r.Put(m+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(6));
        r.Put(m+cfgprobe::EngineGlobalCandidate,admission.engine);r.Put(admission.engine+cfgprobe::EngineGameViewport,admission.viewport);
        r.Put(admission.viewport+cfgprobe::ViewportWorld,admission.world);r.Put(admission.world+cfgprobe::WorldNetDriver,driver);
        r.Put(admission.world+cfgprobe::WorldGameMode,mode);r.Put(admission.world+cfgprobe::WorldGameState,state);
        r.Put(mode+disk::ActorRole,disk::AuthorityRole);r.Put(mode+cfgescape::disk::MatchState,cfgnames::Words{});
        r.Put(m+cfgnames::disk::Global,std::uintptr_t(0x500000));r.Put(0x500400,std::array<std::int32_t,2>{{1,1}});
        r.Put(0x500000,std::uintptr_t(0x510000));r.Put(0x510000,std::uintptr_t(0x520000));r.Put(0x520000,std::uint32_t{});
        std::array<char,16> text{};std::memcpy(text.data(),"InProgress",10);r.Put(0x520010,text);
        r.Put(pawn+0x20,admission.world);r.Put(controller+0x20,admission.world);
        r.Put(pawn+cfgescape::disk::Controller,controller);r.Put(controller+cfgprobe::ControllerPawn,pawn);
        r.Put(pawn+disk::ActorRole,disk::AuthorityRole);r.Put(controller+disk::ActorRole,disk::AuthorityRole);
    }
};
int main(){unsigned errors=0;const auto check=[&](bool b){if(!b)++errors;};
    Fixture f;hn::Context c;unsigned reads=0;check(hn::Capture(f.r,f.admission,99,c,&reads));check(reads<hn::ReadLimit);
    const auto captureReads=reads;
    hn::Registered entry;check(hn::ObserveRegistered(f.r,c,99,f.pawn,f.controller,hn::Kind::Killer,entry,&reads));
    const auto registeredReads=reads;check(reads<hn::ReadLimit&&entry.pawn.HasAssignedSerial());
    for(unsigned i=1;i<=captureReads;++i){Fixture bad;bad.r.failAt=i;hn::Context result=c;check(!hn::Capture(bad.r,bad.admission,99,result)&&!result.World());}
    for(unsigned i=1;i<=registeredReads;++i){Fixture bad;bad.r.failAt=i;hn::Registered result=entry;check(!hn::ObserveRegistered(bad.r,c,99,bad.pawn,bad.controller,hn::Kind::Killer,result)&&!result.pawn.object);}
    hn::Roster roster;check(roster.Arm(f.r,f.admission,99));check(roster.Register(f.r,99,f.pawn,f.controller,hn::Kind::Killer));
    check(roster.Register(f.r,99,f.pawn,f.controller,hn::Kind::Killer)&&roster.Count()==1);check(roster.Next(f.r,99,entry));
    f.r.Put(f.controller+cfgprobe::ControllerPawn,std::uintptr_t{});check(!roster.Next(f.r,99,entry)&&!roster.Count()&&!entry.pawn.object);
    for(unsigned variant=0;variant<9;++variant){Fixture bad;auto a=bad.admission;std::uint32_t thread=99;
        if(variant==0)a.imagePinned=false;if(variant==1)a.requested=false;if(variant==2)thread=100;
        if(variant==3)bad.r.Put(bad.admission.world+cfgprobe::WorldNetDriver,std::uintptr_t{});
        if(variant==4)bad.r.Put(bad.mode+cfghosted::disk::ActorRole,std::uint8_t(2));
        if(variant==5)bad.r.Put(bad.mode+cfgescape::disk::MatchState,cfgnames::Words{0,1});
        if(variant==6)bad.r.Put(bad.table+16,std::int32_t{});
        if(variant==7)bad.r.Put(bad.admission.viewport+cfgprobe::ViewportWorld,bad.driver);
        if(variant==8)bad.r.Put(bad.admission.module+cfghosted::disk::HuntConstructor,bad.classes[1]);
        check(!hn::Capture(bad.r,a,thread,c)&&!c.World());
    }
    Fixture fresh;check(hn::Capture(fresh.r,fresh.admission,99,c));
    fresh.r.Put(fresh.controller+cfgprobe::ObjectClass,fresh.classes[4]); // Human/non-AI class refused.
    check(!hn::ObserveRegistered(fresh.r,c,99,fresh.pawn,fresh.controller,hn::Kind::Killer,entry));
    for(unsigned variant=0;variant<4;++variant){Fixture bad;check(hn::Capture(bad.r,bad.admission,99,c));
        if(variant==0)bad.r.Put(bad.pawn+0x20,bad.pawn); // Outer cycle
        if(variant==1)bad.r.Put(bad.controller+cfghosted::disk::ActorRole,std::uint8_t(2));
        if(variant==2)bad.r.Put(bad.table+5*24+16,std::int32_t{}); // unassigned AI lifetime
        if(variant==3)bad.r.Put(bad.pawn+cfgescape::disk::Controller,bad.pawn);
        check(!hn::ObserveRegistered(bad.r,c,99,bad.pawn,bad.controller,hn::Kind::Killer,entry)&&!entry.pawn.object);
    }
    Fixture capacity;hn::Roster bounded;check(bounded.Arm(capacity.r,capacity.admission,99));
    // GObjects growth is allowed only after fresh header/unchanged context
    // proof. The registry itself remains fixed eight entries, never a census.
    capacity.r.Put(capacity.admission.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(24));
    for(unsigned i=0;i<9;++i){const std::uintptr_t pawn=0x600000+0x2000*i,controller=pawn+0x1000;
        capacity.Object(pawn,6+2*i,capacity.classes[4]);capacity.Object(controller,7+2*i,capacity.classes[5]);
        capacity.r.Put(pawn+0x20,capacity.admission.world);capacity.r.Put(controller+0x20,capacity.admission.world);
        capacity.r.Put(pawn+cfgescape::disk::Controller,controller);capacity.r.Put(controller+cfgprobe::ControllerPawn,pawn);
        capacity.r.Put(pawn+cfghosted::disk::ActorRole,cfghosted::disk::AuthorityRole);
        capacity.r.Put(controller+cfghosted::disk::ActorRole,cfghosted::disk::AuthorityRole);
        check(bounded.Register(capacity.r,99,pawn,controller,hn::Kind::Killer)==(i<8));
    }
    check(bounded.Count()==8&&bounded.Next(capacity.r,99,entry));
    capacity.r.Put(capacity.table+6*24+16,std::int32_t(1000)); // same pointer/index, recycled serial
    check(!bounded.Register(capacity.r,99,0x600000,0x601000,hn::Kind::Killer)&&bounded.Count()==8);
    Fixture born;const std::uintptr_t playerStateType=0x460000,counselorAI=0x470000,primaryState=0x280000,newState=0x290000,players=0x550000;
    born.Pair(cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,playerStateType);
    born.Pair(cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,counselorAI);
    born.Object(primaryState,6,playerStateType);born.Object(newState,7,playerStateType);born.Object(born.controller,5,counselorAI);
    born.r.Put(born.admission.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(8));
    born.r.Put(born.pawn+0x20,born.admission.world);born.r.Put(primaryState+0x20,born.admission.world);
    born.r.Put(born.state+cfghosted::birthdisk::CurrentKiller,born.pawn);born.r.Put(born.state+cfghosted::birthdisk::CurrentKillerState,primaryState);
    born.r.Put(born.mode+cfgmenu::populationdisk::NumBots,std::int32_t{});
    born.r.Put(born.state+cfghosted::birthdisk::PlayerArray,hn::PlayerList{players,1,8});born.r.Put(players,primaryState);
    check(hn::Capture(born.r,born.admission,99,c));hn::Population population;
    check(hn::PopulationBefore(born.r,c,99,population)&&population.list.count==1);
    born.r.Put(born.state+cfghosted::birthdisk::PlayerArray,hn::PlayerList{players,2,8});born.r.Put(players+8,newState);
    born.r.Put(born.controller+cfgprobe::ControllerPawn,std::uintptr_t{});born.r.Put(born.controller+cfgescape::disk::PlayerState,newState);
    born.r.Put(newState+0x88,born.controller);born.r.Put(newState+0x20,born.admission.world);
    born.r.Put(newState+cfghosted::disk::ActorRole,cfghosted::disk::AuthorityRole);born.r.Put(newState+cfghosted::birthdisk::BotFlags,std::uint32_t(4));
    born.r.Put(born.table+7*24+16,std::int32_t{});
    hn::NewControllerState newborn;check(hn::ControllerBorn(born.r,population,99,born.controller,newborn)&&!newborn.stateSerialAssigned&&newborn.botDelta==0);
    born.r.Put(born.mode+cfgmenu::populationdisk::NumBots,std::int32_t(1));check(hn::ControllerBorn(born.r,population,99,born.controller,newborn)&&newborn.botDelta==1);
    for(unsigned fault=0;fault<7;++fault){auto bad=born;bad.r.reads=0;
        if(fault==0)bad.r.Put(players, newState); // replaces an existing human state
        if(fault==1)bad.r.Put(bad.mode+cfgmenu::populationdisk::NumBots,std::int32_t(2));
        if(fault==2)bad.r.Put(bad.state+cfghosted::birthdisk::CurrentKiller,newState);
        if(fault==3)bad.r.Put(newState+cfghosted::birthdisk::BotFlags,std::uint32_t{});
        if(fault==4)bad.r.Put(bad.controller+cfgprobe::ControllerPawn,bad.pawn);
        if(fault==5)bad.r.Put(newState+0x88,bad.pawn);
        auto prior=population;if(fault==6)prior.list.count=100;
        check(!hn::ControllerBorn(bad.r,prior,99,bad.controller,newborn)&&!newborn.controller.object);
    }
    for(unsigned variant=0;variant<4;++variant){Fixture bad;bad.r.mutateAt=50;
        bad.r.mutationAddress=variant==0?bad.admission.world+cfgprobe::WorldGameMode:
            variant==1?bad.mode+cfgprobe::ObjectClass:
            variant==2?bad.admission.module+cfgprobe::dispatch::ThreadIdRva:bad.table+16;
        bad.r.mutationValue=0;check(!hn::Capture(bad.r,bad.admission,99,c)&&!c.World());
    }
    // Hard adapter cap independently holds even with a permissive source.
    Fixture budget;hn::Budget<Memory> capped(budget.r);std::uintptr_t word=0;
    for(unsigned i=0;i<hn::ReadLimit;++i)check(capped.Read(budget.admission.world+cfgprobe::WorldGameMode,word));
    const auto before=budget.r.reads;check(!capped.Read(budget.admission.world+cfgprobe::WorldGameMode,word)&&budget.r.reads==before);
    std::printf("Hosted native authority/registered roster tests: %u failures; context=%u registered=%u reads (cap%u). No native calls.\n",errors,captureReads,registeredReads,hn::ReadLimit);
    return errors?1:0;
}
