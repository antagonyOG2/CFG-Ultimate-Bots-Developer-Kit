#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "ShackTrapSource.hpp"
#include <map>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <limits>
#include <algorithm>
struct Reader {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,limit=1536;
    template<class T> void Put(std::uintptr_t p,const T& v){const auto* b=reinterpret_cast<const unsigned char*>(&v);for(unsigned i=0;i<sizeof(T);++i)bytes[p+i]=b[i];}
    template<class T> bool Read(std::uintptr_t p,T& v){if(reads>=limit)return false;++reads;auto* b=reinterpret_cast<unsigned char*>(&v);
        for(unsigned i=0;i<sizeof(T);++i){auto it=bytes.find(p+i);if(it==bytes.end())return false;b[i]=it->second;}return true;}
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x140000000,World=0x200000,Level=0x210000,Actor=0x220000,Part=0x230000,
        Levels=0x240000,Actors=0x250000,LevelClass=0x300000,CabinClass=0x310000,SceneClass=0x320000,BP=0x330000,
        PropertyClass=0x340000,Names=0x400000,Chunk=0x410000,NameEntries=0x420000,Field=0x600000;
    Reader r;cfgprobe::lifetime::ArrayHeader registry{0x100000,128};cfgshacktrap::Survey survey;unsigned peak=0;
    void Slot(std::uintptr_t object,int index){r.Put(object+0xC,index);r.Put(registry.data+24*index,object);r.Put(registry.data+24*index+8,std::uint32_t{0});r.Put(registry.data+24*index+16,index+1);}
    void Name(unsigned index,const char* text){std::array<unsigned char,128> value{};std::memcpy(value.data(),text,std::strlen(text));
        r.Put(Chunk+8*index,NameEntries+256*index);r.Put(NameEntries+256*index,std::uint32_t{0});r.Put(NameEntries+256*index+cfgnames::disk::EntryText,value);}
    void Property(std::uintptr_t field,std::uintptr_t next,int name){r.Put(field+0x20,BP);r.Put(field+0x10,PropertyClass);r.Put(field+0x28,next);r.Put(field+0x18,cfgnames::Words{name,0});
        r.Put(field+0x30,int{1});r.Put(field+0x34,int{8});r.Put(field+0x38,std::uint64_t{});r.Put(field+0x44,int{0x800});}
    Fixture(){
        const auto pair=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t type){r.Put(Module+a,type);r.Put(Module+b,type);};
        pair(cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache,LevelClass);
        pair(cfgradarworld::disk::CabinClassCache,cfgradarworld::disk::CabinConstructorCache,CabinClass);
        pair(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,SceneClass);
        for(auto type:{LevelClass,CabinClass,SceneClass})r.Put(type+0x30,std::uintptr_t{});
        r.Put(BP+0x30,CabinClass);r.Put(Level+0x10,LevelClass);r.Put(Actor+0x10,BP);r.Put(Part+0x10,SceneClass);
        r.Put(World+cfgradarworld::disk::WorldLevels,cfgshacktrap::List{Levels,1,1});r.Put(Levels,Level);
        r.Put(Level+cfgkill::disk::OwningWorld,World);r.Put(Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{Actors,18,18});
        for(unsigned i=0;i<18;++i)r.Put(Actors+8*i,i==17?Actor:std::uintptr_t{});
        r.Put(Actor+0x20,Level);r.Put(Actor+cfgradarworld::disk::CabinIsJasonShack,std::uint8_t{1});Slot(Actor,0);Slot(Part,1);
        r.Put(Actor+0x800,Part);r.Put(Part+0x20,Actor);r.Put(Part+0xE0,Actor);r.Put(Part+cfgposition::disk::SceneWorldTranslation,cfgspawn::Vec3{1000,0,10});
        r.Put(Module+cfgnames::disk::Global,Names);struct Counts{int entries,chunks;};r.Put(Names+cfgnames::disk::ElementCount,Counts{8,1});r.Put(Names,Chunk);
        Name(1,"Shack_Jason_door_01");Name(2,"ObjectProperty");Name(3,"Unrelated");r.Put(PropertyClass+0x18,cfgnames::Words{2,0});
        r.Put(BP+0x38,Field);Property(Field,0,1);
    }
    void Tick(){r.reads=0;survey.Step(r,registry,Module,World);peak=(std::max)(peak,r.reads);assert(r.reads<1536);}
    bool Warm(){for(unsigned i=0;i<100;++i){r.reads=0;const auto result=survey.WarmProperty(r,registry);peak=(std::max)(peak,r.reads);assert(r.reads<1536);if(result)return result==1;}return false;}
    bool Position(){r.reads=0;cfgspawn::Vec3 door{},forward{};const bool result=survey.Position(r,registry,{},door,forward);peak=(std::max)(peak,r.reads);assert(r.reads<1536);return result;}
    void LargeList(std::uintptr_t storage=Actors){r.Put(Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{storage,2048,2048});
        for(unsigned i=0;i<2048;++i)r.Put(storage+8*i,i==2047?Actor:std::uintptr_t{});}
    void Ready(){for(unsigned i=0;i<64&&!survey.Done();++i)Tick();assert(survey.stage==2);assert(Warm());}
};
int main(){unsigned peak=0;
    {cfgspawn::Vec3 a{},b{},c{},d{};assert(cfgshacktrap::Anchor({0,0,0},{1000,0,10},a,b));assert(cfgshacktrap::Anchor({0,0,0},{0,1000,10},c,d));
        assert(std::fabs(a.x-759.187424f)<0.001f&&std::fabs(a.y+776.665588f)<0.001f&&std::fabs(a.z-10.878663f)<0.001f);
        assert(std::fabs(c.x+a.y)<0.001f&&std::fabs(c.y-a.x)<0.001f&&std::fabs(d.x+b.y)<0.001f&&std::fabs(d.y-b.x)<0.001f);
        assert(!cfgshacktrap::Anchor({},{},a,b));assert(!cfgshacktrap::Anchor({},{6000,0,0},a,b));assert(!cfgshacktrap::Anchor({},{std::numeric_limits<float>::quiet_NaN(),0,0},a,b));}
    {Fixture f;f.Ready();assert(f.Position());peak=(std::max)(peak,f.peak);f.r.reads=0;f.survey.Step(f.r,f.registry,f.Module,f.World);assert(!f.r.reads);}
    for(unsigned fault=0;fault<7;++fault){Fixture f;f.Ready();assert(f.Position());
        switch(fault){case 0:f.r.Put(f.Actor+0x20,f.World);break;case 1:f.r.Put(f.Level+cfgkill::disk::OwningWorld,f.Actor);break;
        case 2:f.r.Put(f.Actors+17*8,std::uintptr_t{});break;case 3:f.r.Put(f.registry.data+16,int{9});break;
        case 4:f.r.Put(f.Part+0x20,f.Level);break;case 5:f.r.Put(f.registry.data+24+16,int{9});break;
        case 6:f.r.Put(f.Actor+0x800,f.Level);break;}
        assert(!f.Position());peak=(std::max)(peak,f.peak);}
    {Fixture f;for(unsigned i=0;i<96;++i)f.Property(f.Field+256*i,i==95?0:f.Field+256*(i+1),i==95?1:3);
        f.Ready();assert(f.survey.propertySeen==96&&f.Position());peak=(std::max)(peak,f.peak);}
    {Fixture f;f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,100000,100000});
        for(unsigned i=0;i<100000;++i)f.r.Put(f.Actors+8*i,std::uintptr_t{});for(unsigned i=0;i<=cfgshacktrap::Survey::MaximumSlices;++i)f.Tick();
        assert(f.survey.stage==3&&f.survey.failure==7&&f.survey.slices==cfgshacktrap::Survey::MaximumSlices+1);peak=(std::max)(peak,f.peak);}
    {Fixture f;f.LargeList();f.Tick();constexpr std::uintptr_t moved=0x760000;
        f.LargeList(moved);
        for(unsigned i=0;i<2048*8;++i)f.r.bytes.erase(f.Actors+i);
        f.Tick();assert(f.survey.restarts==1&&f.survey.actorIndex==0&&f.survey.stage==1);
        f.Tick();f.Tick();assert(f.survey.stage==2&&f.Warm()&&f.Position());}
    {Fixture f;f.LargeList();f.Tick();for(unsigned restart=0;restart<12;++restart){
        const auto moved=std::uintptr_t{0x760000}+restart*0x10000;
        f.LargeList(moved);
        f.Tick();assert(f.survey.stage==1);}
        f.Tick();f.Tick();assert(f.survey.restarts==12&&f.survey.stage==2&&f.Warm()&&f.Position());}
    {Fixture f;f.LargeList();constexpr std::uintptr_t other=0x900000;
        f.r.Put(other+cfgprobe::ObjectClass,f.LevelClass);
        for(unsigned i=0;i<2047;++i)f.r.Put(f.Actors+8*i,other);
        for(unsigned i=0;i<96;++i)f.Property(f.Field+256*i,i==95?0:f.Field+256*(i+1),i==95?1:3);
        unsigned callbacks=0,combinedPeak=0;
        while(!f.survey.Done()&&callbacks<12){f.r.reads=256;f.survey.Step(f.r,f.registry,f.Module,f.World);combinedPeak=(std::max)(combinedPeak,f.r.reads);++callbacks;}
        unsigned warm=0;while(f.survey.stage==2&&warm==0&&callbacks<12){f.r.reads=256;warm=f.survey.WarmProperty(f.r,f.registry);combinedPeak=(std::max)(combinedPeak,f.r.reads);++callbacks;}
        ++callbacks;assert(warm==1&&f.Position()&&callbacks<12&&combinedPeak<1536);
        std::printf("Shack late2047/96fields completes%u callbacks (<8s at667ms), peak%u/1536 incl256 outer\n",callbacks,combinedPeak);}
    std::printf("Shack trap fixtures PASS: owned source, paged lookup,64-slice cap and retired-array reacquisition; peak=%u/1536 reads\n",peak);return 0;
}
