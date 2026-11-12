#include "SweaterSourceObservation.hpp"
#include <map>
#include <cstdio>
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;std::uintptr_t change=0;unsigned hits=0;
    template<class T>void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    template<class T>bool Read(std::uintptr_t at,T& value){++reads;if(reads>128)return false;
        if(at==change&&++hits==2)return false;
        auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;p[i]=f->second;}return true;}
};
struct Fixture {
    static constexpr std::uintptr_t World=0x10000,State=0x20000,Human=0x30000,Source=0x40000,Ability=0x50000,Table=0x60000,Items=0x70000;
    struct List {std::uintptr_t data;int count,capacity;};
    Memory memory;cfgprobe::lifetime::ArrayHeader table{Table,2};
    cfgprobe::lifetime::SlotObservation source{Source,0,0},ability{Ability,1,0};
    Fixture(){memory.Set(World+cfgprobe::WorldGameState,State);memory.Set(State+cfgfeature::disk::WorldItems,List{Items,1,1});memory.Set(Items,Source);
        memory.Set(Source+cfgfeature::disk::SweaterAbility,Ability);memory.Set(Source+cfgloadout::restricteddisk::OwningCharacter,std::uintptr_t{});
        memory.Set(Source+0xC,int{0});memory.Set(Ability+0xC,int{1});memory.Set(Table,Source);memory.Set(Table+24,Ability);
        memory.Set(Table+16,int{0});memory.Set(Table+40,int{0});}
    bool Read(){return cfgsweater::StrongSource(memory,table,World,State,Human,source,ability,0);}
};
int main(){unsigned checks=0,failures=0;auto check=[&](bool ok){++checks;if(!ok)++failures;};
    {Fixture f;check(f.Read());check(f.memory.reads<=32);std::printf("Strong sweater source reads=%u, serials=0; no calls/writes\n",f.memory.reads);}
    {Fixture f;f.memory.Set(f.Source+cfgloadout::restricteddisk::OwningCharacter,f.Human);check(f.Read());}
    for(unsigned fault=0;fault<9;++fault){Fixture f;
        if(fault==0)f.memory.Set(f.World+cfgprobe::WorldGameState,f.Human);
        if(fault==1)f.memory.Set(f.Items,f.Human);
        if(fault==2)f.memory.Set(f.Source+cfgfeature::disk::SweaterAbility,f.Human);
        if(fault==3)f.memory.Set(f.Source+cfgloadout::restricteddisk::OwningCharacter,f.State);
        if(fault==4)f.memory.Set(f.Table+16,int{1});
        if(fault==5)f.memory.Set(f.Table+24,f.Source);
        if(fault==6)f.memory.Set(f.State+cfgfeature::disk::WorldItems,Fixture::List{f.Items,257,257});
        if(fault==7)f.memory.change=f.Items;
        if(fault==8)f.memory.change=f.State+cfgfeature::disk::WorldItems;
        check(!f.Read());}
    std::printf("Sweater strong-source fixtures: %u checks, %u failures. No game access.\n",checks,failures);return failures?1:0;
}
