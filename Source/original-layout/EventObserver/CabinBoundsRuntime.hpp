#pragma once
#include "../Core/CabinBoundsPolicy.hpp"
#include "ShackTrapSource.hpp"
#include "../RuntimeProbe/CabinBoundsBindings.generated.hpp"
namespace cfgcabin {
struct Source {
 std::uintptr_t actor=0,type=0,level=0;unsigned levelIndex=0,actorIndex=0;
 cfgprobe::lifetime::SlotObservation identity{};
};
struct Runtime {
 Cache cache{};std::array<Source,Cache::MaximumCabins> sources{};
 cfgshacktrap::List levels{},actors{};std::uintptr_t module=0,level=0;
 std::array<std::uintptr_t,32> surveyedLevels{};std::array<cfgshacktrap::List,32> surveyedActors{};
 std::array<unsigned,32> starts{};
 unsigned levelIndex=0,actorIndex=0,slices=0,passSlices=0,prepareSlices=0,rebuilds=0,previousLevels=0;
 bool prepared=false,refresh=false;
 void Invalidate(unsigned why){
  // Keep conservative captured cabin footprints. Actor churn queues a finite
  // incremental census, not repeated native bounds calls for every old cabin.
  // No enumeration/native call occurs in the destination predicate.
  ++rebuilds;previousLevels=static_cast<unsigned>(levels.count);
  cache.stage=Stage::Dormant;cache.failure=why;refresh=true;
  actors={};level=0;levelIndex=0;actorIndex=0;passSlices=0;
 }
 template<class R>bool Typed(R& r,std::uintptr_t object,std::uintptr_t reflection,std::uintptr_t constructor)const{
  std::uintptr_t a=0,b=0;return r.Read(module+reflection,a)&&a&&r.Read(module+constructor,b)&&a==b&&cfgspawn::ObjectIs(r,object,a);
 }
 template<class R>bool WorldCurrent(R& r,std::uintptr_t ownedWorld)const{
  cfgshacktrap::List now{};return cache.world==ownedWorld&&r.Read(ownedWorld+cfgradarworld::disk::WorldLevels,now)&&now==levels;
 }
 template<class R>bool LayoutCurrent(R& r,std::uintptr_t ownedWorld)const{
  if(!WorldCurrent(r,ownedWorld)||levelIndex!=static_cast<unsigned>(levels.count))return false;
  for(unsigned i=0;i<levelIndex;++i){std::uintptr_t value=0;cfgshacktrap::List list{};
   if(!r.Read(levels.data+8*i,value)||value!=surveyedLevels[i]||
    !r.Read(value+cfgkill::disk::OwningWorld,value)||value!=ownedWorld||
    !r.Read(surveyedLevels[i]+cfgradarworld::disk::LevelActors,list)||!(list==surveyedActors[i]))return false;
  }return WorldCurrent(r,ownedWorld);
 }
 // Invoke only under the host's current offline/GC/thread scope. Header
 // changes invalidate rather than restarting discovery in a hot morph path.
 template<class R>bool Allows(R& r,std::uintptr_t ownedWorld,cfgspawn::Vec3 point){
  if(!cache.Allows(ownedWorld,point))return false;
  if(!LayoutCurrent(r,ownedWorld)){Invalidate(12);return false;}return true;
 }
 template<class R>bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& registry,const Source& source)const{
  std::uintptr_t value=0;cfgshacktrap::List now{};cfgprobe::lifetime::SlotObservation slot{};
  return WorldCurrent(r,cache.world)&&r.Read(levels.data+8*source.levelIndex,value)&&value==source.level&&
   Typed(r,source.level,cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache)&&
   r.Read(source.level+cfgkill::disk::OwningWorld,value)&&value==cache.world&&
   r.Read(source.level+cfgradarworld::disk::LevelActors,now)&&now.Sane(100000)&&source.actorIndex<static_cast<unsigned>(now.count)&&
   r.Read(now.data+8*source.actorIndex,value)&&value==source.actor&&r.Read(source.actor+0x20,value)&&value==source.level&&
   r.Read(source.actor+cfgprobe::ObjectClass,value)&&value==source.type&&
   Typed(r,source.actor,cfgradarworld::disk::CabinClassCache,cfgradarworld::disk::CabinConstructorCache)&&
   cfgspawn::ObjectSlot(r,registry,source.actor,slot)&&slot==source.identity;
 }
 template<class R>void Survey(R& input,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t mod,std::uintptr_t world){
  if(cache.stage!=Stage::Dormant&&cache.stage!=Stage::Survey)return;
  cfgshacktrap::SurveyBudget<R> r(input);
  ++slices;if(++passSlices>512){cache.Refuse(1);return;}
  if(cache.stage==Stage::Dormant){module=mod;cache.world=world;
   if(!r.Read(world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(32)||!levels.count){cache.Refuse(2);return;}
   starts={};
   if(refresh){
    for(unsigned i=0;i<static_cast<unsigned>(levels.count);++i){std::uintptr_t current=0,owner=0;cfgshacktrap::List list{};
     if(!r.Read(levels.data+8*i,current)||!current||!r.Read(current+cfgkill::disk::OwningWorld,owner)||owner!=world||
        !r.Read(current+cfgradarworld::disk::LevelActors,list)||!list.Sane(100000)){cache.Refuse(5);return;}
     if(i<previousLevels&&current==surveyedLevels[i]&&list.count>=surveyedActors[i].count){
      starts[i]=static_cast<unsigned>(surveyedActors[i].count);
      // Native TArray reallocation preserves the prefix. If known cabins moved
      // within that prefix, inspect the whole changed level, retaining bounds.
      for(unsigned j=0;j<cache.count;++j)if(sources[j].level==current){std::uintptr_t actor=0;
       if(sources[j].actorIndex>=static_cast<unsigned>(list.count)||!r.Read(list.data+8*sources[j].actorIndex,actor)||actor!=sources[j].actor){starts[i]=0;break;}}
     }
    }
   }
   cache.stage=Stage::Survey;}
  if(module!=mod||cache.world!=world){cache.Refuse(3);return;}
  if(!WorldCurrent(r,world)){Invalidate(3);return;}
  std::uintptr_t cabin=0,constructor=0;
  if(!r.Read(module+cfgradarworld::disk::CabinClassCache,cabin)||!cabin||
   !r.Read(module+cfgradarworld::disk::CabinConstructorCache,constructor)||constructor!=cabin){cache.Refuse(4);return;}
  struct Classification{std::uintptr_t type=0;bool cabin=false;};std::array<Classification,64> classes{};unsigned classified=0,taken=0;
  while(levelIndex<static_cast<unsigned>(levels.count)&&taken<1024&&r.reads<600){
   std::uintptr_t current=0,owner=0;cfgshacktrap::List fresh{};
   if(!r.Read(levels.data+8*levelIndex,current)||!current||
    !Typed(r,current,cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache)||
    !r.Read(current+cfgkill::disk::OwningWorld,owner)||owner!=world||
    !r.Read(current+cfgradarworld::disk::LevelActors,fresh)||!fresh.Sane(100000)){cache.Refuse(5);return;}
   if(level!=current){level=current;actors=fresh;actorIndex=starts[levelIndex];}
   // A changed actor roster could omit a cabin. Fail closed instead of
   // completing a partial cache; startup service should run after world setup.
   if(fresh.data!=actors.data||fresh.count<actors.count){Invalidate(6);return;}actors=fresh;
   std::array<std::uintptr_t,64> page{};unsigned pageAt=~0u;
   for(;actorIndex<static_cast<unsigned>(actors.count)&&taken<1024&&r.reads<600;++actorIndex,++taken){
    std::uintptr_t actor=0,type=0;
    if(pageAt==~0u&&static_cast<unsigned>(actors.count)-actorIndex>=64){if(!r.Read(actors.data+8*actorIndex,page))return;pageAt=actorIndex;}
    if(pageAt!=~0u){actor=page[actorIndex-pageAt];if(actorIndex-pageAt==63)pageAt=~0u;}else if(!r.Read(actors.data+8*actorIndex,actor))return;
    if(!actor)continue;if(!r.Read(actor+cfgprobe::ObjectClass,type))return;
    bool isCabin=false,known=false;
    for(unsigned i=0;i<classified;++i)if(classes[i].type==type){isCabin=classes[i].cabin;known=true;break;}
    if(!known){isCabin=cfgspawn::ClassContains(r,type,cabin);if(r.exhausted)return;if(classified<classes.size())classes[classified++]={type,isCabin};}
    if(!isCabin)continue;
    bool duplicate=false;for(unsigned i=0;i<cache.count;++i)if(sources[i].actor==actor){duplicate=true;sources[i].levelIndex=levelIndex;sources[i].actorIndex=actorIndex;}
    if(duplicate)continue;
    if(cache.count>=sources.size()){cache.Refuse(7);return;}
    Source source{actor,type,level,levelIndex,actorIndex,{}};
    if(!cfgspawn::ObjectSlot(r,registry,actor,source.identity)||!Current(r,registry,source)){
     if(!r.exhausted)cache.Refuse(8);return;}
    sources[cache.count++]=source;
   }
   if(actorIndex>=static_cast<unsigned>(actors.count)){surveyedLevels[levelIndex]=level;surveyedActors[levelIndex]=actors;++levelIndex;level=0;actorIndex=0;}else return;
  }
  if(levelIndex>=static_cast<unsigned>(levels.count))cache.stage=Stage::Capture;
 }
 // Host Scope() must prove retained offline world, owner thread and no GC.
 // PrepareBounds(): 0 pending,1 ready,2 unavailable; validates generated full
 // factory/executor/native body + typed reflected Actor function/frame.
 // ReadBounds(actor,Packet&) invokes only that prepared native function. No
 // other native operation may share this dedicated service callback.
 template<class R,class H>void Step(R& r,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t mod,std::uintptr_t world,H& host){
  if(cache.stage==Stage::Ready||cache.stage==Stage::Refused)return;
  if(!host.Scope())return;
  if(cache.stage==Stage::Dormant||cache.stage==Stage::Survey){Survey(r,registry,mod,world);return;}
  if(mod!=module||cache.world!=world){cache.Refuse(3);return;}
  if(!LayoutCurrent(r,world)){Invalidate(3);return;}
  if(!prepared){if(++prepareSlices>64){cache.Refuse(10);return;}const unsigned result=host.PrepareBounds();
   if(result==2)cache.Refuse(10);else if(result==1)prepared=true;return;}
  if(cache.captured==cache.count){cache.stage=Stage::Ready;return;}
  const auto& source=sources[cache.captured];Packet packet; // false includes non-colliding roof/wall geometry
  if(!Current(r,registry,source)){cache.Refuse(8);return;}
  ++cache.calls;
  if(!host.ReadBounds(source.actor,packet)||!host.Scope()||!Current(r,registry,source)){cache.Refuse(11);return;}
  cache.Add(packet);
 }
};
}
