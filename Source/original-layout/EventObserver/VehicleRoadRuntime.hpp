#pragma once
#include "../Core/VehicleRoadInterceptPolicy.hpp"
#include "../Core/VehicleRoadChasePolicy.hpp"
#include "../Core/VehicleExitBlockPolicy.hpp"
#include "../RuntimeProbe/ChallengeEscapeRecovery.generated.hpp"
#include "../Core/VehicleRoadPathTrace.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/VehicleRoadBindings.generated.hpp"
#include "../RuntimeProbe/RadarWorldBindings.generated.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
namespace cfgroad {
struct List {
 std::uintptr_t data=0;int count=0,capacity=0;
 bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
 bool Sane(int maximum)const{return count>=0&&count<=maximum&&capacity>=count&&capacity<=1000000&&(!count||cfgprobe::ObjectPointer(data));}
};
enum class Stage {Dormant,Survey,Capture,Ready,Refused};
template<class R>class Budget {
 R& input;
public:
 unsigned reads=0,limit=1536;bool exhausted=false,failed=false;
 explicit Budget(R& r,unsigned available=1536):input(r),limit((std::min)(1536u,available)){}
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=limit){exhausted=true;return false;}++reads;if(!input.Read(at,out)){failed=true;return false;}return true;}
};
struct RoadSource {
 std::uintptr_t actor=0,type=0,level=0,root=0;unsigned levelIndex=0,actorIndex=0;
 cfgprobe::lifetime::SlotObservation slot{},rootSlot{};
 List neighbors{};std::array<std::uintptr_t,MaximumNeighbors> links{};
};
// Owner-thread, exact-image and owned-offline-world authorization belong to
// the host. Every public observation uses <=1536 reads and ZERO native calls.
// Call Step in a dedicated callback. A finite paged level census is performed
// once per owned world; Refused does not start an ongoing rediscovery loop.
struct Runtime {
 Stage stage=Stage::Dormant;unsigned failure=0,lastReads=0,slices=0,levelIndex=0,actorIndex=0,captured=0;
 std::uintptr_t module=0,world=0,roadClass=0,sceneClass=0,levelClass=0;
 cfgprobe::lifetime::SlotObservation worldSlot{};
 List levels{};std::array<std::uintptr_t,32> levelPointers{};std::array<List,32> actorLists{};
 std::array<RoadSource,MaximumPoints> sources{};Graph graph{};
 std::uintptr_t exitClass=0,blockedCar=0;unsigned exitCount=0,exitCaptured=0,blockedExit=NoPoint;
 std::array<RoadSource,16> exits{};std::array<Vector,16> exitPositions{};
 template<class R>bool ExitClass(R& r){
  std::uintptr_t type=0,ctor=0;
  if(!r.Read(module+cfgchallengeescape::disk::SCEscapeVolume,type)||!r.Read(module+cfgchallengeescape::disk::SCEscapeVolumeCtor,ctor))return false;
  if(type!=ctor||(exitClass&&exitClass!=type))return false;exitClass=type;return true;
 }
 bool ExitCandidate(Vector car,Vector travel,Candidate& output,std::uintptr_t vehicle=0)const{
  output={};if(stage!=Stage::Ready)return false;float best=std::numeric_limits<float>::max();
  for(unsigned i=0;i<exitCount;++i){if(vehicle&&blockedCar==vehicle&&blockedExit==i)continue;
   Candidate c;if(!ResolveExit(graph,car,travel,exitPositions[i],c)||c.forwardLead>=best)continue;
   best=c.forwardLead;c.exitIndex=i;output=c;
  }
  return output.exitBlock;
 }
 void Refuse(unsigned why){stage=Stage::Refused;failure=why;}
 template<class R>bool Classes(R& r){
  const std::array<std::uintptr_t,6> offsets{{disk::RoadClassCache,disk::RoadConstructorCache,
   cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,
   cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache}};
  std::array<std::uintptr_t,6> types{};
  for(unsigned i=0;i<types.size();++i)if(!r.Read(module+offsets[i],types[i]))return false;
  for(unsigned i=0;i<types.size();i+=2)if(!cfgprobe::MatchingClassCache(types[i],types[i+1]))return false;
  if(roadClass&&(roadClass!=types[0]||sceneClass!=types[2]||levelClass!=types[4]))return false;
  roadClass=types[0];sceneClass=types[2];levelClass=types[4];return true;
 }
 template<class R>bool WorldCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& registry){
  List now{};cfgprobe::lifetime::SlotObservation slot{};
  return cfgspawn::ObjectSlot(r,registry,world,slot)&&slot==worldSlot&&
   r.Read(world+cfgradarworld::disk::WorldLevels,now)&&now==levels;
 }
 template<class R>bool SourceCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& registry,const RoadSource& s,bool exit=false){
  std::uintptr_t value=0;List list{};cfgprobe::lifetime::SlotObservation slot{};
  return s.levelIndex<static_cast<unsigned>(levels.count)&&r.Read(levels.data+8*s.levelIndex,value)&&value==s.level&&
   cfgspawn::ObjectIs(r,s.level,levelClass)&&r.Read(s.level+cfgkill::disk::OwningWorld,value)&&value==world&&
   r.Read(s.level+cfgradarworld::disk::LevelActors,list)&&list.Sane(100000)&&s.actorIndex<static_cast<unsigned>(list.count)&&
   r.Read(list.data+8*s.actorIndex,value)&&value==s.actor&&r.Read(s.actor+0x20,value)&&value==s.level&&
   r.Read(s.actor+cfgprobe::ObjectClass,value)&&value==s.type&&cfgspawn::ClassContains(r,value,exit?exitClass:roadClass)&&
   cfgspawn::ObjectSlot(r,registry,s.actor,slot)&&slot==s.slot;
 }
 template<class R>bool Position(R& r,const cfgprobe::lifetime::ArrayHeader& registry,RoadSource& s,Vector& p,bool compare){
  std::uintptr_t root=0,value=0;cfgprobe::lifetime::SlotObservation slot{},again{};Vector second{};
  if(!r.Read(s.actor+cfgposition::disk::ActorRootComponent,root)||!cfgprobe::ObjectPointer(root)||(compare&&root!=s.root)||
   !cfgspawn::ObjectIs(r,root,sceneClass)||!r.Read(root+0x20,value)||value!=s.actor||!r.Read(root+0xE0,value)||value!=s.actor||
   !cfgspawn::ObjectSlot(r,registry,root,slot)||(compare&&!(slot==s.rootSlot))||
   !r.Read(root+cfgposition::disk::SceneWorldTranslation,p)||!Finite(p)||
   !r.Read(root+cfgposition::disk::SceneWorldTranslation,second)||p.x!=second.x||p.y!=second.y||p.z!=second.z||
   !r.Read(s.actor+cfgposition::disk::ActorRootComponent,value)||value!=root||
   !cfgspawn::ObjectSlot(r,registry,root,again)||!(again==slot))return false;
  s.root=root;s.rootSlot=slot;return true;
 }
 template<class R>bool Links(R& r,RoadSource& s,bool compare){
  List list{},again{};std::array<std::uintptr_t,MaximumNeighbors> links{},second{};
  if(!r.Read(s.actor+disk::Neighbors,list)||!list.Sane(MaximumNeighbors)||(compare&&!(list==s.neighbors)))return false;
  // Read only the array's live extent, never a full8-pointer page past Count.
  for(int i=0;i<list.count;++i)if(!r.Read(list.data+8*i,links[i]))return false;
  for(int i=0;i<list.count;++i)if(!r.Read(list.data+8*i,second[i])||links[i]!=second[i])return false;
  if(!r.Read(s.actor+disk::Neighbors,again)||!(again==list)||(compare&&links!=s.links))return false;
  s.neighbors=list;s.links=links;return true;
 }
 template<class R>void Survey(R& r,const cfgprobe::lifetime::ArrayHeader& registry){
  struct ClassMemo{std::uintptr_t type=0;bool road=false,exit=false;};std::array<ClassMemo,64> memo{};unsigned used=0;
  while(levelIndex<static_cast<unsigned>(levels.count)&&r.reads+400<r.limit){
   std::uintptr_t level=0,owner=0;List list{};
   if(!r.Read(levels.data+8*levelIndex,level)||!cfgspawn::ObjectIs(r,level,levelClass)||
    !r.Read(level+cfgkill::disk::OwningWorld,owner)||owner!=world||!r.Read(level+cfgradarworld::disk::LevelActors,list)||!list.Sane(100000)){Refuse(4);return;}
   if(!levelPointers[levelIndex]){levelPointers[levelIndex]=level;actorLists[levelIndex]=list;}
   if(levelPointers[levelIndex]!=level){Refuse(5);return;}
   // Trap/projectile/helper births can grow or relocate a level's roster.
   // Keep the finite cursor; CURRENT authored source cells are proved again
   // during Capture/Intercept. No second census or dead-array dereference.
   actorLists[levelIndex]=list;
   std::array<std::uintptr_t,64> page{};unsigned pageStart=~0u;
   for(;actorIndex<static_cast<unsigned>(list.count)&&r.reads+400<r.limit;++actorIndex){
    std::uintptr_t actor=0,type=0;
    if(pageStart==~0u&&static_cast<unsigned>(list.count)-actorIndex>=64){if(!r.Read(list.data+8*actorIndex,page)){Refuse(6);return;}pageStart=actorIndex;}
    if(pageStart!=~0u){actor=page[actorIndex-pageStart];if(actorIndex-pageStart==63)pageStart=~0u;}
    else if(!r.Read(list.data+8*actorIndex,actor)){Refuse(6);return;}
    if(!actor)continue;if(!r.Read(actor+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type)){Refuse(6);return;}
    bool road=false,exit=false,known=false;for(unsigned i=0;i<used;++i)if(memo[i].type==type){road=memo[i].road;exit=memo[i].exit;known=true;break;}
    if(!known){road=cfgspawn::ClassContains(r,type,roadClass);exit=exitClass&&cfgspawn::ClassContains(r,type,exitClass);
     if(r.failed||r.exhausted){Refuse(6);return;}if(used<memo.size())memo[used++]={type,road,exit};}
    if(exit){
     if(exitCount==exits.size()){Refuse(14);return;}
     RoadSource e;e.actor=actor;e.type=type;e.level=level;e.levelIndex=levelIndex;e.actorIndex=actorIndex;
     if(!cfgspawn::ObjectSlot(r,registry,actor,e.slot)||!SourceCurrent(r,registry,e,true)){Refuse(14);return;}
     exits[exitCount++]=e;continue;
    }
    if(!road)continue;if(graph.count==MaximumPoints){Refuse(7);return;}
    RoadSource s;s.actor=actor;s.type=type;s.level=level;s.levelIndex=levelIndex;s.actorIndex=actorIndex;
    if(!cfgspawn::ObjectSlot(r,registry,actor,s.slot)||!SourceCurrent(r,registry,s)){Refuse(8);return;}
    for(unsigned i=0;i<graph.count;++i)if(sources[i].actor==actor){Refuse(9);return;}
    sources[graph.count++]=s;
   }
   if(actorIndex>=static_cast<unsigned>(list.count)){++levelIndex;actorIndex=0;}else return;
  }
  if(levelIndex==static_cast<unsigned>(levels.count)){if(graph.count<2)Refuse(10);else stage=Stage::Capture;}
 }
 template<class R>void Step(R& input,std::uintptr_t mod,std::uintptr_t ownedWorld,bool scopeVerified,unsigned available=1536){
  lastReads=0;if(!scopeVerified)return;
  if(world&&world!=ownedWorld)*this=Runtime{};
  if(stage==Stage::Ready||stage==Stage::Refused)return;
  if(available<700)return;
  Budget<R> r(input,available);struct Finish{unsigned& count;Budget<R>& r;~Finish(){count=r.reads;}} finish{lastReads,r};
  if(++slices>1024){Refuse(1);return;}
  cfgprobe::lifetime::ArrayHeader registry{},end{};
  if(!cfgprobe::lifetime::ReadArrayHeader(r,mod+cfgprobe::lifetime::ObjectArrayRva,registry)){Refuse(2);return;}
  if(stage==Stage::Dormant){module=mod;world=ownedWorld;
   if(!cfgprobe::ObjectPointer(world)||!cfgspawn::ObjectSlot(r,registry,world,worldSlot)||
    !r.Read(world+cfgradarworld::disk::WorldLevels,levels)||!levels.Sane(32)||!levels.count){Refuse(2);return;}stage=Stage::Survey;}
  if(mod!=module||!Classes(r)||!ExitClass(r)||!WorldCurrent(r,registry)){Refuse(3);return;}
  if(stage==Stage::Survey)Survey(r,registry);
  else if(stage==Stage::Capture){
   for(unsigned taken=0;taken<8&&captured<graph.count&&r.reads+400<r.limit;++taken,++captured){auto& s=sources[captured];auto& node=graph.nodes[captured];
    if(!SourceCurrent(r,registry,s)||!Position(r,registry,s,node.position,false)||!Links(r,s,false)||!SourceCurrent(r,registry,s)){Refuse(11);return;}
    node.count=static_cast<unsigned>(s.neighbors.count);
    for(unsigned j=0;j<node.count;++j){node.neighbors[j]=NoPoint;
     if(!s.links[j])continue;for(unsigned k=0;k<graph.count;++k)if(sources[k].actor==s.links[j]){node.neighbors[j]=k;break;}
     if(node.neighbors[j]==NoPoint){Refuse(12);return;}}
   }
   if(captured==graph.count){
    for(unsigned taken=0;taken<4&&exitCaptured<exitCount&&r.reads+400<r.limit;++taken,++exitCaptured){
     auto& e=exits[exitCaptured];if(!SourceCurrent(r,registry,e,true)||!Position(r,registry,e,exitPositions[exitCaptured],false)||
        !SourceCurrent(r,registry,e,true)){Refuse(14);return;}
    }
    if(exitCaptured==exitCount)stage=Stage::Ready;
   }
  }
  if(r.exhausted||!WorldCurrent(r,registry)||!cfgprobe::lifetime::ReadArrayHeader(r,mod+cfgprobe::lifetime::ObjectArrayRva,end)||!(end==registry))Refuse(13);
 }
 // Resolve against the prepared authored graph, then cheaply prove only its
 // selected route and cached links. No census or native call in this path.
 // Host must consume the output in the same verified callback; re-sample car
 // motion and preserve cabin/teleport eligibility gates at the actual write.
 template<class R>bool Intercept(R& input,std::uintptr_t ownedWorld,Vector car,Vector travel,float speed,bool scopeVerified,Candidate& output,unsigned available=1536,std::uintptr_t vehicle=0){
  output={};lastReads=0;if(!scopeVerified||stage!=Stage::Ready||world!=ownedWorld)return false;
  Candidate c;if(!ExitCandidate(car,travel,c,vehicle)&&!Resolve(graph,car,travel,speed,c))return false;
  return ProveRoute(input,ownedWorld,c,output,available);
 }
 template<class R>bool ChaseWaypoint(R& input,std::uintptr_t ownedWorld,Vector jason,Vector car,Candidate& output,unsigned available=1536){
  output={};lastReads=0;if(stage!=Stage::Ready||world!=ownedWorld)return false;
  Candidate c;if(!Chase(graph,jason,car,c))return false;
  return ProveRoute(input,ownedWorld,c,output,available);
 }
 template<class R>bool ProveRoute(R& input,std::uintptr_t ownedWorld,const Candidate& c,Candidate& output,unsigned available){
  output={};if(stage!=Stage::Ready||world!=ownedWorld||!c.count||c.count>c.route.size())return false;
  Budget<R> r(input,available);struct Finish{unsigned& count;Budget<R>& r;~Finish(){count=r.reads;}} finish{lastReads,r};
  cfgprobe::lifetime::ArrayHeader registry{},end{};
  if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||!Classes(r)||!WorldCurrent(r,registry))return false;
  if(c.exitBlock){
   if(c.exitIndex>=exitCount||!ExitClass(r))return false;auto& e=exits[c.exitIndex];Vector p{};
   if(!SourceCurrent(r,registry,e,true)||!Position(r,registry,e,p,true)||
      p.x!=exitPositions[c.exitIndex].x||p.y!=exitPositions[c.exitIndex].y||p.z!=exitPositions[c.exitIndex].z||!SourceCurrent(r,registry,e,true))return false;
  }
  for(unsigned i=0;i<c.count;++i){const auto index=c.route[i];if(index>=graph.count)return false;auto& s=sources[index];Vector p{};
   if(!SourceCurrent(r,registry,s)||!Position(r,registry,s,p,true)||!Links(r,s,true)||
    p.x!=graph.nodes[index].position.x||p.y!=graph.nodes[index].position.y||p.z!=graph.nodes[index].position.z)return false;
   for(unsigned j=0;j<graph.nodes[index].count;++j){const unsigned next=graph.nodes[index].neighbors[j];if(next==NoPoint)continue;
    const auto& linked=sources[next];cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t type=0,outer=0;
    if(!cfgspawn::ObjectSlot(r,registry,linked.actor,slot)||!(slot==linked.slot)||!r.Read(linked.actor+cfgprobe::ObjectClass,type)||type!=linked.type||
     !r.Read(linked.actor+0x20,outer)||outer!=linked.level)return false;
   }
   if(!SourceCurrent(r,registry,s))return false;
  }
  if(!WorldCurrent(r,registry)||!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,end)||!(end==registry))return false;
  output=c;return true;
 }
};
}
