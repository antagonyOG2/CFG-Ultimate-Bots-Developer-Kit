#pragma once
#include "OfflineContextObservation.hpp"
#include "ShackTrapSource.hpp"
#include "../RuntimeProbe/RadarObjectiveBindings.generated.hpp"
#include "../RuntimeProbe/RadioObjectiveBindings.hpp"
#include "../Core/RadioAcquisitionRetry.hpp"
#include "../Core/J22FeatureFlags.hpp"
namespace cfgradioobjective {
// Finite world-local acquisition. No object registry census, actor creation or
// display-only UI. Cache the native radio just as the donor does for GetCBRadio.
struct Survey {
    unsigned stage=0,failure=0,slices=0,levelIndex=0,actorIndex=0,typedStage=0;
    std::uintptr_t candidate=0;std::int32_t candidateSerial=0;
    std::uintptr_t module=0,world=0,state=0,type=0,radio=0,radioLevel=0,radioCell=0;
    cfgprobe::lifetime::SlotObservation slot{};
    cfgprobe::Ancestry ancestry{};
    cfgshacktrap::List levels{};
    std::array<std::uintptr_t,32> levelObjects{};
    std::array<cfgshacktrap::List,32> actors{};
    bool cached=false;
    bool Done()const{return stage>=2;}
    void Refuse(unsigned why){stage=3;failure=why;}
    template<class R>bool Typed(R& r,std::uintptr_t actor,cfgprobe::lifetime::ArrayHeader registry){
        std::uintptr_t level=0,levelType=0;cfgprobe::lifetime::SlotObservation before{},after{};
        candidate=actor;candidateSerial=0;typedStage=1;
        if(!cfgspawn::ObjectSlot(r,registry,actor,before))return false;
        candidateSerial=before.serial;typedStage=2;if(!before.HasAssignedSerial())return false;
        typedStage=3;if(!cfgspawn::ObjectIs(r,actor,type))return false;
        typedStage=4;if(!r.Read(actor+0x20,level))return false;
        typedStage=5;if(!r.Read(module+cfgradarworld::disk::LevelClassCache,levelType)||!levelType||
            !cfgobserver::Same(r,module+cfgradarworld::disk::LevelConstructorCache,levelType))return false;
        typedStage=6;if(!cfgspawn::ObjectIs(r,level,levelType))return false;
        typedStage=7;if(!cfgobserver::Same(r,level+cfgkill::disk::OwningWorld,world))return false;
        typedStage=8;if(!cfgspawn::ObjectSlot(r,registry,actor,after)||!(after==before))return false;
        typedStage=0;return true;
    }
    template<class R>bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& registry,bool published=false)const{
        std::uintptr_t level=0;cfgprobe::lifetime::SlotObservation now{};cfgshacktrap::List fresh{};
        if(!radio||!slot.HasAssignedSerial()||
           !cfgobserver::Same(r,module+cfgradarobjective::disk::SCCBRadioReflection,type)||
           !cfgobserver::Same(r,module+cfgradarobjective::disk::SCCBRadioConstructor,type)||
           !cfgspawn::ObjectSlot(r,registry,radio,now)||!(now==slot)||
           !cfgprobe::StableAncestry(r,radio,ancestry)||!ancestry.Contains(type)||
           !r.Read(radio+0x20,level)||level!=radioLevel||
           !cfgobserver::Same(r,level+cfgkill::disk::OwningWorld,world)||
           !cfgobserver::Same(r,state+disk::Cache,(cached||published)?radio:std::uintptr_t{}))return false;
        if(!cached){
            if(!r.Read(world+cfgradarworld::disk::WorldLevels,fresh)||!(fresh==levels)||!levels.Sane(32))return false;
            for(unsigned i=0;i<static_cast<unsigned>(levels.count);++i)
                if(!levelObjects[i]||!cfgobserver::Same(r,levels.data+8*i,levelObjects[i])||
                   !cfgobserver::Same(r,levelObjects[i]+cfgkill::disk::OwningWorld,world)||
                   !r.Read(levelObjects[i]+cfgradarworld::disk::LevelActors,fresh)||!(fresh==actors[i]))return false;
            if(!cfgobserver::Same(r,radioCell,radio))return false;
        }
        return cfgspawn::ObjectSlot(r,registry,radio,now)&&now==slot;
    }
    template<class R>void Step(R& input,const cfgprobe::lifetime::ArrayHeader& registry,
        std::uintptr_t mod,std::uintptr_t ownedWorld,std::uintptr_t ownedState){
        if(Done())return;cfgshacktrap::SurveyBudget<R> r(input);
        if(++slices>128){Refuse(1);return;}
        if(!stage){module=mod;world=ownedWorld;state=ownedState;
            if(!r.Read(module+cfgradarobjective::disk::SCCBRadioReflection,type)||!type||
               !cfgobserver::Same(r,module+cfgradarobjective::disk::SCCBRadioConstructor,type)){Refuse(2);return;}
            if(!r.Read(state+disk::Cache,radio)){Refuse(2);return;}
            if(radio){cached=true;
                if(!Typed(r,radio,registry)||!r.Read(radio+0x20,radioLevel)||
                   !cfgspawn::ObjectSlot(r,registry,radio,slot)||!cfgprobe::CaptureAncestry(r,radio,ancestry)||
                   !Current(r,registry)){Refuse(3);return;}stage=2;return;}
            if(!r.Read(world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(32)||!levels.count){Refuse(4);return;}
            stage=1;
        }
        cfgshacktrap::List fresh{};
        if(module!=mod||world!=ownedWorld||state!=ownedState||
           !cfgobserver::Same(r,state+disk::Cache,std::uintptr_t{})||
           !r.Read(world+cfgradarworld::disk::WorldLevels,fresh)||!(fresh==levels)){Refuse(4);return;}
        struct Class{std::uintptr_t type=0;bool radio=false;};std::array<Class,64> types{};unsigned count=0;
        while(levelIndex<static_cast<unsigned>(levels.count)&&r.reads<540){
            std::uintptr_t level=0,levelType=0;
            if(!r.Read(levels.data+8*levelIndex,level)||!level||
               !r.Read(module+cfgradarworld::disk::LevelClassCache,levelType)||!levelType||
               !cfgobserver::Same(r,module+cfgradarworld::disk::LevelConstructorCache,levelType)||
               !cfgspawn::ObjectIs(r,level,levelType)||!cfgobserver::Same(r,level+cfgkill::disk::OwningWorld,world)||
               !r.Read(level+cfgradarworld::disk::LevelActors,fresh)||!fresh.Sane(100000)){if(!r.exhausted)Refuse(4);return;}
            if(!levelObjects[levelIndex]){levelObjects[levelIndex]=level;actors[levelIndex]=fresh;}
            else if(levelObjects[levelIndex]!=level||!(actors[levelIndex]==fresh)){Refuse(5);return;}
            // Page reads reduce per-actor work, but every candidate is typed
            // and positively identified before it can become the publication.
            std::array<std::uintptr_t,64> page{};unsigned pageAt=~0u;
            for(;actorIndex<static_cast<unsigned>(fresh.count)&&r.reads<540;++actorIndex){
                std::uintptr_t object=0,objectType=0;
                if(pageAt==~0u&&static_cast<unsigned>(fresh.count)-actorIndex>=64){if(!r.Read(fresh.data+8*actorIndex,page))return;pageAt=actorIndex;}
                if(pageAt!=~0u){object=page[actorIndex-pageAt];if(actorIndex-pageAt==63)pageAt=~0u;}
                else if(!r.Read(fresh.data+8*actorIndex,object))return;
                if(!object)continue;if(!r.Read(object+cfgprobe::ObjectClass,objectType))return;
                bool known=false,match=false;for(unsigned i=0;i<count;++i)if(types[i].type==objectType){known=true;match=types[i].radio;break;}
                if(!known){match=cfgspawn::ClassContains(r,objectType,type);if(r.exhausted)return;if(count<types.size())types[count++]={objectType,match};}
                if(!match)continue;
                if(radio&&radio!=object){Refuse(6);return;} // ambiguous level radio: never guess
                if(!Typed(r,object,registry)||!cfgspawn::ObjectSlot(r,registry,object,slot)||
                   !cfgprobe::CaptureAncestry(r,object,ancestry)){if(!r.exhausted)Refuse(3);return;}
                radio=object;radioLevel=level;radioCell=fresh.data+8*actorIndex;
            }
            if(actorIndex<static_cast<unsigned>(fresh.count))return;
            ++levelIndex;actorIndex=0;
        }
        if(levelIndex==static_cast<unsigned>(levels.count)){
            if(!radio){Refuse(7);return;}if(!Current(r,registry)){if(!r.exhausted)Refuse(5);return;}stage=2;
        }
    }
};
template<class R,class Host>bool Publish(R& r,const cfgprobe::lifetime::ArrayHeader& registry,const Survey& source,Host& host){
    if(source.stage!=2||!host.Current()||!source.Current(r,registry)||!host.Prepare())return false;
    if(!source.cached&&!host.Exchange(source.state+disk::Cache,source.radio))return false;
    std::uintptr_t returned=0;
    return host.Getter(returned)&&returned==source.radio&&host.Current()&&source.Current(r,registry,true);
}
}
