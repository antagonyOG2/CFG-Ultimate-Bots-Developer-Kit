#pragma once
#include "NativeStartingLoadout.hpp"
#include "../RuntimeProbe/RadarWorldBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
namespace cfgloadout::expanded {
// The map already owns loaded item classes. Survey its much smaller actor lists
// before using the UObject registry as a fallback (the donor's FindClassExact).
// Read-only pages and class classifications live only within ONE callback.
// A discovered pointer is only a hint; NativeAccess proves its name, ancestry,
// defaults and current object identities again before any native grant.
class ClassSurvey {
    struct List {std::uintptr_t data=0;int count=0,capacity=0;
        bool Sane(int maximum)const{return count>=0&&count<=maximum&&capacity>=count&&capacity<=1000000&&(!count||cfgprobe::ObjectPointer(data));}};
    struct RegistryEntry {std::uintptr_t object=0,flags=0,serial=0;};
    static_assert(sizeof(RegistryEntry)==24);
    struct Classification {std::uintptr_t type=0;bool eligible=false;};
    unsigned levelIndex_=0,actorIndex_=0;std::uintptr_t level_=0,actorData_=0;
    unsigned actorCount_=0;
public:
    // phase:0 world actors,1 global registry,2 found,3 finite survey exhausted.
    unsigned phase=0,cursor=0,total=0,slices=0,visited=0,readDeferrals=0,restarts=0,lastReads=0;
    std::uintptr_t found=0;
    template<class Reader> class Budget {
        Reader& reader_;
    public:
        unsigned reads=0;explicit Budget(Reader& r):reader_(r){}
        template<class T> bool Read(std::uintptr_t at,T& out){if(reads>=768)return false;++reads;return reader_.Read(at,out);}
    };
    template<class Reader> bool Step(Reader& reader,std::uintptr_t module,std::uintptr_t world,Slot slot,unsigned choice){
        if(phase>=2)return phase==2;++slices;Budget<Reader> r(reader);
        struct Finish{unsigned& output;Budget<Reader>& r;~Finish(){output=r.reads;}} finish{lastReads,r};
        std::uintptr_t item=0,other=0,meta=0;
        if(!r.Read(module+restricted::itemdisk::SCItemClass,item)||!r.Read(module+restricted::itemdisk::SCItemConstructor,other)||
            !cfgprobe::MatchingClassCache(item,other)||!r.Read(item+cfgprobe::ObjectClass,meta)||!cfgprobe::ObjectPointer(meta)){++readDeferrals;return false;}
        std::array<Classification,64> cache{};unsigned used=0;
        cfgnames::Observation<Budget<Reader>> names(r,module);
        const auto named=[&](std::uintptr_t type){cfgnames::Text name;return names.Object(type,name)&&name.View()==ClassName(slot,choice);};
        const auto classify=[&](std::uintptr_t type,bool registry){
            for(unsigned i=0;i<used;++i)if(cache[i].type==type)return cache[i].eligible;
            const bool okay=registry?cfgmenu::maps::Subclass(r,type,meta):named(type);
            if(used<cache.size())cache[used++]={type,okay};return okay;
        };
        if(!phase){
            List levels;if(!r.Read(world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(32)||!levels.count){phase=1;cursor=0;return true;}
            if(levelIndex_>=static_cast<unsigned>(levels.count)){phase=1;cursor=0;return true;}
            std::uintptr_t level=0,owner=0;List actors;
            if(!r.Read(levels.data+8*levelIndex_,level)||!cfgprobe::ObjectPointer(level)||
                !r.Read(level+cfgkill::disk::OwningWorld,owner)||owner!=world||
                !r.Read(level+cfgradarworld::disk::LevelActors,actors)||!actors.Sane(100000)){
                ++readDeferrals;++levelIndex_;actorIndex_=0;return false;}
            if(level_!=level){level_=level;actorData_=actors.data;actorCount_=static_cast<unsigned>(actors.count);actorIndex_=0;}
            else if(actorData_!=actors.data||static_cast<unsigned>(actors.count)<actorCount_){
                ++restarts;actorIndex_=0;actorData_=actors.data;}
            actorCount_=static_cast<unsigned>(actors.count);total=actorCount_;cursor=actorIndex_;
            std::array<std::uintptr_t,64> page{};unsigned pageAt=~0u;
            for(unsigned taken=0;taken<1024&&actorIndex_<actorCount_&&r.reads<640;++taken){
                std::uintptr_t actor=0,type=0;
                if(actorCount_-actorIndex_>=64&&pageAt==~0u){
                    if(!r.Read(actors.data+8*actorIndex_,page)){++readDeferrals;return false;}pageAt=actorIndex_;}
                if(pageAt!=~0u){actor=page[actorIndex_-pageAt];if(actorIndex_-pageAt==63)pageAt=~0u;}
                else if(!r.Read(actors.data+8*actorIndex_,actor)){++readDeferrals;return false;}
                ++actorIndex_;++visited;cursor=actorIndex_;
                if(!cfgprobe::ObjectPointer(actor)||!r.Read(actor+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type))continue;
                if(classify(type,false)){found=type;phase=2;return true;}
            }
            if(actorIndex_>=actorCount_){++levelIndex_;actorIndex_=0;level_=0;}return true;
        }
        cfgprobe::lifetime::ArrayHeader objects;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,objects)){++readDeferrals;return false;}
        total=static_cast<unsigned>(objects.count);
        // UObjects keep their index across an array-storage reallocation. Read
        // the fresh header each slice instead of abandoning on pointer growth.
        if(cursor>total){cursor=0;++restarts;}
        std::array<RegistryEntry,64> page{};unsigned pageAt=~0u;
        for(unsigned taken=0;taken<1024&&cursor<total&&r.reads<640;++taken){
            std::uintptr_t object=0,type=0;
            if(total-cursor>=64&&pageAt==~0u){
                if(!r.Read(objects.data+24*cursor,page)){++readDeferrals;return false;}pageAt=cursor;}
            if(pageAt!=~0u){object=page[cursor-pageAt].object;if(cursor-pageAt==63)pageAt=~0u;}
            else if(!r.Read(objects.data+24*cursor,object)){++readDeferrals;return false;}
            ++cursor;++visited;
            if(!cfgprobe::ObjectPointer(object)||!r.Read(object+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type))continue;
            if(classify(type,true)&&named(object)){found=object;phase=2;return true;}
        }
        if(cursor>=total)phase=3;return true;
    }
};
}
