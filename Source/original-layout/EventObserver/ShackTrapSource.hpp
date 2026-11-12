#pragma once
#include "../Core/ShackTrapPolicy.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/RadarWorldBindings.generated.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
namespace cfgshacktrap {
template<class R> class SurveyBudget {
 R& source_;
public:
 unsigned reads=0;bool exhausted=false;explicit SurveyBudget(R& r):source_(r){}
 template<class T>bool Read(std::uintptr_t at,T& value){if(reads>=768){exhausted=true;return false;}++reads;if(!source_.Read(at,value)){exhausted=true;return false;}return true;}
};
struct List {std::uintptr_t data=0;int count=0,capacity=0;
    bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
    bool Sane(int max)const{return count>=0&&count<=max&&capacity>=count&&capacity<=1000000&&(!count||cfgprobe::ObjectPointer(data));}};
struct Survey {
    static constexpr unsigned MaximumSlices=64;
    unsigned stage=0,failure=0,levelIndex=0,actorIndex=0,slices=0,restarts=0;std::uintptr_t world=0,module=0,actor=0,level=0,cell=0,component=0,type=0;
    cfgreflection::Property property{};cfgprobe::lifetime::SlotObservation componentSlot{};
    std::uintptr_t propertyOwner=0,propertyHead=0,propertyCursor=0;
    std::array<std::uintptr_t,256> propertyVisited{};unsigned propertySeen=0,propertyOwners=0,propertyMatches=0;
    std::array<std::uintptr_t,32> propertyParents{},propertySupers{};
    cfgreflection::Field propertyMatch{};bool propertyOpen=false;
    List levels{},actors{};cfgprobe::lifetime::SlotObservation actorSlot{};
    bool Done()const{return stage>=2;}
    void Skip(unsigned why){failure=why;stage=3;}
    template<class R> bool Typed(R& r,std::uintptr_t object,std::uintptr_t cache,std::uintptr_t ctor){
        std::uintptr_t a=0,b=0;return r.Read(module+cache,a)&&a&&r.Read(module+ctor,b)&&a==b&&cfgspawn::ObjectIs(r,object,a);
    }
    template<class R> bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& registry){
        List now{};std::uintptr_t value=0;unsigned char flag=0;cfgprobe::lifetime::SlotObservation slot{};
        return actor&&r.Read(world+cfgradarworld::disk::WorldLevels,now)&&now==levels&&
            r.Read(levels.data+8*levelIndex,value)&&value==level&&
            r.Read(level+cfgkill::disk::OwningWorld,value)&&value==world&&
            r.Read(level+cfgradarworld::disk::LevelActors,now)&&now.Sane(100000)&&
            actorIndex<static_cast<unsigned>(now.count)&&now.data+8*actorIndex==cell&&r.Read(cell,value)&&value==actor&&
            r.Read(actor+0x20,value)&&value==level&&r.Read(actor+cfgprobe::ObjectClass,value)&&value==type&&
            Typed(r,actor,cfgradarworld::disk::CabinClassCache,cfgradarworld::disk::CabinConstructorCache)&&
            r.Read(actor+cfgradarworld::disk::CabinIsJasonShack,flag)&&flag==1&&
            cfgspawn::ObjectSlot(r,registry,actor,slot)&&slot==actorSlot;
    }
    template<class R> void Step(R& input,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t mod,std::uintptr_t ownedWorld){
        if(Done())return;
        SurveyBudget<R> r(input);
        if(++slices>MaximumSlices){Skip(7);return;} // secondary work cap; native preparation also has a wall-clock limit
        if(!stage){module=mod;world=ownedWorld;if(!r.Read(world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(32)||!levels.count){Skip(1);return;}stage=1;}
        List fresh{};if(world!=ownedWorld||!r.Read(world+cfgradarworld::disk::WorldLevels,fresh)||!(fresh==levels)){Skip(2);return;}
        // Optional work has only8s. The previous16 actors / ONE level per
        // callback could expire after192 entries (physical Failure10). Read
        // pages and memoize exact class ancestry only inside this callback;
        // actor/door ownership is still proved independently at the match.
        std::uintptr_t cabinType=0,ctor=0;
        if(!r.Read(module+cfgradarworld::disk::CabinClassCache,cabinType)||!cabinType||
           !r.Read(module+cfgradarworld::disk::CabinConstructorCache,ctor)||ctor!=cabinType){if(!r.exhausted)Skip(4);return;}
        struct Classification{std::uintptr_t type=0;bool cabin=false;};std::array<Classification,64> cache{};unsigned cached=0,taken=0;
        while(levelIndex<static_cast<unsigned>(levels.count)&&taken<1024&&r.reads<640){
        std::uintptr_t currentLevel=0,owner=0;
        if(!r.Read(levels.data+8*levelIndex,currentLevel)||!currentLevel||
           !Typed(r,currentLevel,cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache)||
           !r.Read(currentLevel+cfgkill::disk::OwningWorld,owner)||owner!=world||
           !r.Read(currentLevel+cfgradarworld::disk::LevelActors,fresh)||!fresh.Sane(100000)){if(!r.exhausted)Skip(4);return;}
        if(level!=currentLevel){level=currentLevel;actors=fresh;actorIndex=0;}
        if(fresh.data!=actors.data||fresh.count<actors.count){
            // Native startup can compact/reallocate while helpers are born.
            // Reacquire only this level, never dereference retired storage.
            // Actor destruction during native startup can shrink this list
            // repeatedly. The finite slice cap bounds the whole survey;
            // a separate eight-restart cap abandoned valid shack searches.
            ++restarts;actors=fresh;actorIndex=0;return;
        }actors=fresh;
        std::array<std::uintptr_t,64> page{};unsigned pageAt=~0u;
        for(;taken<1024&&actorIndex<static_cast<unsigned>(actors.count)&&r.reads<640;++taken,++actorIndex){
            std::uintptr_t candidate=0,candidateType=0;unsigned char flag=0;
            if(static_cast<unsigned>(actors.count)-actorIndex>=64&&pageAt==~0u){
                if(!r.Read(actors.data+8*actorIndex,page))return;pageAt=actorIndex;}
            if(pageAt!=~0u){candidate=page[actorIndex-pageAt];if(actorIndex-pageAt==63)pageAt=~0u;}
            else if(!r.Read(actors.data+8*actorIndex,candidate))return;
            if(!candidate)continue;
            if(!r.Read(candidate+cfgprobe::ObjectClass,candidateType))return;
            bool cabin=false,known=false;
            for(unsigned i=0;i<cached;++i)if(cache[i].type==candidateType){cabin=cache[i].cabin;known=true;break;}
            if(!known){cabin=cfgspawn::ClassContains(r,candidateType,cabinType);if(r.exhausted)return;
                if(cached<cache.size())cache[cached++]={candidateType,cabin};}
            if(!cabin)continue;
            if(!r.Read(candidate+cfgradarworld::disk::CabinIsJasonShack,flag))return;if(flag!=1)continue;
            actor=candidate;cell=actors.data+8*actorIndex;
            if(!r.Read(actor+cfgprobe::ObjectClass,type)||!cfgspawn::ObjectSlot(r,registry,actor,actorSlot)||!Current(r,registry)){if(!r.exhausted)Skip(6);return;}
            stage=2;return;
        }
        if(actorIndex>=static_cast<unsigned>(actors.count)){++levelIndex;level=0;actorIndex=0;}
        else return;
        }
        if(levelIndex>=static_cast<unsigned>(levels.count))Skip(3);
    }
    // Return0 pending,1 ready,2 unavailable. Only16 reflected fields per slice;
    // never run Reflection::Find's full class traversal in a gameplay callback.
    template<class R> unsigned WarmProperty(R& r,const cfgprobe::lifetime::ArrayHeader& registry){
        if(stage!=2||!Current(r,registry))return 2;
        if(property.field.object)return 1;
        cfgreflection::Observation<R> reflection(r,module);
        if(!propertyOwner)propertyOwner=type;
        std::uintptr_t current=0;
        for(unsigned i=0;i+1<propertyOwners;++i)
            if(!r.Read(propertyParents[i]+cfgprobe::ClassSuper,current)||current!=propertySupers[i])return 2;
        if(!propertyOpen){
            if(propertyOwners>=propertyParents.size())return 2;
            for(unsigned i=0;i<propertyOwners;++i)if(propertyParents[i]==propertyOwner)return 2;
            propertyParents[propertyOwners++]=propertyOwner;
            if(!r.Read(propertyOwner+cfgreflection::disk::Children,propertyHead))return 2;
            propertyCursor=propertyHead;propertyOpen=true;propertyMatches=0;
        }
        if(!r.Read(propertyOwner+cfgreflection::disk::Children,current)||current!=propertyHead)return 2;
        for(unsigned taken=0;taken<16&&propertyCursor;++taken){
            if(propertySeen>=propertyVisited.size())return 2;
            for(unsigned i=0;i<propertySeen;++i)if(propertyVisited[i]==propertyCursor)return 2;
            cfgreflection::Field field{};if(!reflection.ReadField(propertyCursor,propertyOwner,field))return 2;
            propertyVisited[propertySeen++]=propertyCursor;
            if(field.label.View()==u"Shack_Jason_door_01"){propertyMatch=field;++propertyMatches;}
            propertyCursor=field.next;
        }
        if(!r.Read(propertyOwner+cfgreflection::disk::Children,current)||current!=propertyHead)return 2;
        if(propertyCursor)return 0;
        if(propertyMatches){
            if(propertyMatches!=1||!reflection.ReadProperty(propertyMatch,property)||!Current(r,registry))return 2;
            return 1;
        }
        if(!r.Read(propertyOwner+cfgprobe::ClassSuper,current)||!current)return 2;
        propertySupers[propertyOwners-1]=current;propertyOwner=current;propertyOpen=false;return 0;
    }
    template<class R> bool Position(R& r,const cfgprobe::lifetime::ArrayHeader& registry,Vector cabin,Vector& door,Vector& forward){
        if(stage!=2||!Current(r,registry))return false;
        cfgreflection::Observation<R> reflection(r,module);std::uintptr_t part=0,owner=0;Vector location{},again{};
        cfgprobe::lifetime::SlotObservation slot{},final{};
        if(!property.field.object||!cfgspawn::ObjectIs(r,actor,property.field.owner))return false;
        cfgreflection::Property fresh{};
        if(!reflection.ReadProperty(property.field,fresh)||fresh.offset!=property.offset||fresh.flags!=property.flags||
           fresh.field.kind.View()!=u"ObjectProperty"||fresh.dimension!=1||fresh.elementBytes!=8||fresh.offset%8||
           !r.Read(actor+static_cast<unsigned>(fresh.offset),part)||!part||(component&&part!=component)||
           !Typed(r,part,cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache)||
           !r.Read(part+0x20,owner)||owner!=actor||!r.Read(part+0xE0,owner)||owner!=actor||
           !cfgspawn::ObjectSlot(r,registry,part,slot)||(component&&!(slot==componentSlot))||!r.Read(part+cfgposition::disk::SceneWorldTranslation,location)||
           !r.Read(part+cfgposition::disk::SceneWorldTranslation,again)||
           location.x!=again.x||location.y!=again.y||location.z!=again.z||
           !cfgspawn::ObjectSlot(r,registry,part,final)||!(slot==final)||!r.Read(actor+static_cast<unsigned>(fresh.offset),owner)||owner!=part||!Current(r,registry))return false;
        component=part;componentSlot=slot;return Anchor(cabin,location,door,forward);
    }
};
}
