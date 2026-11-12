#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "NativeEmoteMirror.hpp"
#include "ConvergenceBrainObservation.hpp"
#include "BundleObservations.hpp"
#include "../RuntimeProbe/ConvergenceTravelBindings.generated.hpp"
#include "../RuntimeProbe/StartupTrapBindings.generated.hpp"
#include "../Core/KillPolicy.hpp"
static std::uint64_t testNow=100;
static std::uint64_t TestTickCount64(){return testNow;}
struct {std::uintptr_t module=0;}plan;
struct {unsigned bots=1;}bundleReport;
struct {struct {std::array<std::uintptr_t,8> classes{};}resources;}spawnAssets;
std::array<cfgemotemirror::Slot,9> bundlePawnSlots{},bundleControllerSlots{};
struct EmoteBrainHold {cfgemotemirror::Slot pawn{},controller{},brain{};unsigned phase=0;bool changed=false;
 std::uintptr_t montage=0;std::uint64_t until=0;};
std::array<EmoteBrainHold,9> bundleEmoteBrains{};
std::uint64_t bundleEmoteMaintainAt=0;unsigned bundleEmoteMaintainCursor=0,bundleEmoteStops=0,bundleEmoteResumes=0;
struct {cfgkill::Policy policy;}bundleKill;
bool bundleCompletionReady=false;
cfgemotemirror::Runtime bundleEmotes;
struct Travel {struct {bool owned=false;}policy;};std::array<Travel,9> bundleConvergenceTravel{};
struct Access {
 Fixture& r_;struct {cfgprobe::lifetime::ArrayHeader objectArray{};}context;
 bool valid=true,isLiving=true,tickEnabled=true,callOkay=true,assignSerial=false;unsigned sets=0,queries=0,stops=0,cancels=0,resumes=0;
 explicit Access(Fixture& f):r_(f){context.objectArray=f.objects;}
 bool Context()const{return valid;}
 bool Known(unsigned index,std::uintptr_t& pawn){pawn=r_.bot;return index==1&&valid;}
 bool Alive(std::uintptr_t pawn,bool& out){out=isLiving;return valid&&pawn==r_.bot;}
 void TravelResume(unsigned index,std::uint64_t,bool){bundleConvergenceTravel[index].policy.owned=false;++resumes;}
 template<class B,class C>bool CarCall(const B& binding,std::uintptr_t,const C&,std::uintptr_t object,void* packet){
  if(binding.factory==cfgtrap::disk::StopMovement.factory){CHECK(object==r_.botController);++stops;return callOkay;}
  if(binding.factory==cfgemotemirror::disk::SERVER_CancelEmote.factory){++cancels;r_.Put(object+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});return callOkay;}
  const auto brain=bundleEmoteBrains[0].brain.object;CHECK(object==brain);
  auto* value=static_cast<std::uint8_t*>(packet);
  if(binding.factory==cfgconvergencetravel::disk::SetComponentTickEnabled.factory){++sets;tickEnabled=*value!=0;
   if(assignSerial){int index=0;CHECK(r_.Read(object+12,index));r_.Put(r_.objects.data+24ULL*static_cast<unsigned>(index)+16,int{19});}}
  else {++queries;*value=tickEnabled?1u:0u;}
  return callOkay;
 }
 template<class B>bool FeatureFunction(const B&,std::uintptr_t& function,std::uintptr_t& owner){function=r_.spawn;owner=r_.botClass;return true;}
 bool Invoke(std::uintptr_t object,std::uintptr_t,void*,bool){++cancels;r_.Put(object+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});return true;}
#define GetTickCount64 TestTickCount64
#include "EmoteBrainRuntimeMembers.hpp"
#undef GetTickCount64
};
static cfgemotemirror::Slot Assign(Fixture& f,std::uintptr_t object){int index=0;CHECK(f.Read(object+12,index));
 f.Put(f.objects.data+24ULL*static_cast<unsigned>(index)+16,int{11});cfgemotemirror::Slot slot{};CHECK(cfgspawn::ObjectSlot(f,f.objects,object,slot));return slot;}
static void Reset(Fixture& f){
 bundleEmoteBrains={};bundleEmoteMaintainAt=0;bundleEmoteMaintainCursor=bundleEmoteStops=bundleEmoteResumes=0;
 bundleKill={};bundleCompletionReady=false;bundleConvergenceTravel={};plan.module=f.module;
 spawnAssets.resources.classes=f.classes;bundlePawnSlots[0]=Assign(f,f.bot);bundleControllerSlots[0]=Assign(f,f.botController);
 std::uintptr_t brain=0;CHECK(f.Read(f.botController+0x418,brain));f.Put(brain+0x20,f.botController);f.Put(brain+cfgjason::disk::ComponentOwner,f.botController);
 f.Put(f.spawn+0xB0,f.module+cfgemotemirror::disk::SERVER_CancelEmoteExecutor);
 f.Put(f.module+cfgemotemirror::disk::SERVER_CancelEmoteExecutor,cfgemotemirror::disk::SERVER_CancelEmoteCode);
 f.reads=0;testNow=100;
}
static void Paused(Fixture& f,Access& a){
 cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};
 for(unsigned step=0;step<3;++step){f.reads=0;CHECK(a.MirrorInterrupt(target)==0&&f.reads<500);testNow+=25;}
 f.reads=0;CHECK(a.MirrorInterrupt(target)==1&&a.sets==1&&a.stops==1&&!a.tickEnabled);
 a.MirrorPlayed(target,0xAB0000);f.Put(f.bot+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{0xAB0000});
 CHECK(bundleEmoteBrains[0].phase==4&&bundleEmoteStops==1);
}
int main(){
 {Fixture f;Reset(f);Access a(f);Paused(f,a);f.reads=0;CHECK(a.MirrorMaintain(200)&&!a.tickEnabled&&!bundleEmoteResumes);
  f.Put(f.bot+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});f.reads=0;
  CHECK(a.MirrorMaintain(300)&&!a.tickEnabled&&a.sets==1&&bundleEmoteBrains[0].phase==6);
  f.reads=0;CHECK(a.MirrorMaintain(400)&&a.tickEnabled&&a.sets==2&&bundleEmoteResumes==1&&!bundleEmoteBrains[0].phase);
  f.reads=0;CHECK(!a.MirrorMaintain(500)&&!f.reads);}
 {Fixture f;Reset(f);Access a(f);a.tickEnabled=false;cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};
  CHECK(a.MirrorInterrupt(target)==2&&!a.sets&&!a.stops&&!bundleEmoteBrains[0].phase);}
 {Fixture f;Reset(f);Access a(f);Paused(f,a);a.valid=false;f.reads=0;
  CHECK(a.MirrorMaintain(200)&&a.sets==1&&bundleEmoteBrains[0].phase==4&&!a.tickEnabled);
  a.valid=true;f.Put(f.bot+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});f.reads=0;
  CHECK(a.MirrorMaintain(300)&&!a.tickEnabled&&bundleEmoteBrains[0].phase==6);
  f.reads=0;CHECK(a.MirrorMaintain(400)&&a.tickEnabled&&bundleEmoteResumes==1&&!bundleEmoteBrains[0].phase);}
 {Fixture f;Reset(f);Access a(f);Paused(f,a);const auto replacement=f.Object(f.classes[cfgspawn::Brain]);
  f.Put(replacement+0x20,f.botController);f.Put(replacement+cfgjason::disk::ComponentOwner,f.botController);
  f.Put(f.botController+0x418,replacement);f.reads=0;CHECK(a.MirrorMaintain(200)&&a.sets==1&&!bundleEmoteBrains[0].phase);}
 {Fixture f;Reset(f);Access a(f);Paused(f,a);f.reads=0;const auto until=bundleEmoteBrains[0].until;
  CHECK(a.MirrorMaintain(until)&&a.cancels==1&&bundleEmoteBrains[0].phase==5&&!a.tickEnabled);
  f.reads=0;CHECK(a.MirrorMaintain(until+100)&&!a.tickEnabled&&bundleEmoteBrains[0].phase==6);
  f.reads=0;CHECK(a.MirrorMaintain(until+200)&&a.tickEnabled&&bundleEmoteResumes==1);}
 {Fixture f;Reset(f);Access a(f);bundleConvergenceTravel[0].policy.owned=true;
  cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};
  CHECK(a.MirrorInterrupt(target)==0&&a.resumes==1&&!a.sets&&!a.stops&&!bundleEmoteBrains[0].phase);}
 {Fixture f;Reset(f);Access a(f);a.callOkay=false;cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};
  CHECK(a.MirrorInterrupt(target)==2&&!a.sets&&!a.stops);}
 {Fixture f;Reset(f);Access a(f);a.assignSerial=true;Paused(f,a);CHECK(bundleEmoteBrains[0].brain.serial==19);
  cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};f.reads=0;
  CHECK(a.MirrorInterrupt(target)==0&&a.cancels==1&&bundleEmoteBrains[0].phase==1&&!a.tickEnabled);}
 {Fixture f;Reset(f);Access a(f);Paused(f,a);f.cap=1;f.reads=0;
  CHECK(a.MirrorMaintain(200)&&bundleEmoteBrains[0].phase==4&&!a.tickEnabled&&a.sets==1);
  f.cap=1024;f.reads=0;f.Put(f.bot+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});
  CHECK(a.MirrorMaintain(300)&&!a.tickEnabled&&bundleEmoteBrains[0].phase==6);
  f.reads=0;CHECK(a.MirrorMaintain(400)&&a.tickEnabled&&bundleEmoteResumes==1&&!bundleEmoteBrains[0].phase);}
 {Fixture f;Reset(f);Access a(f);Paused(f,a);a.isLiving=false;f.reads=0;
  CHECK(a.MirrorMaintain(200)&&!bundleEmoteBrains[0].phase&&a.sets==1);}
 {Fixture f;Reset(f);Access a(f);const cfgemotemirror::Recipient target{bundlePawnSlots[0],bundleControllerSlots[0]};
  CHECK(a.MirrorInterrupt(target)==0);testNow+=25;f.reads=0;CHECK(a.MirrorInterrupt(target)==0&&!a.tickEnabled);
  f.reads=0;const auto queries=a.queries,sets=a.sets;
  CHECK(a.MirrorMaintain(150)&&!a.tickEnabled&&a.queries==queries+1&&a.sets==sets&&bundleEmoteBrains[0].phase==6);
  f.reads=0;CHECK(a.MirrorMaintain(250)&&a.tickEnabled&&a.queries==queries+1&&a.sets==sets+1&&!bundleEmoteBrains[0].phase&&bundleEmoteResumes==1);
  CHECK(testNow<3100); // cancelled/unowned queue must not wait the full3s interruption deadline
 }
 {Fixture f;Reset(f);Access a(f);Paused(f,a);f.Put(f.bot+cfgemotemirror::disk::CurrentEmote,std::uintptr_t{});
  f.reads=0;CHECK(a.MirrorMaintain(200)&&bundleEmoteBrains[0].phase==6);
  a.valid=false;f.reads=0;const auto sets=a.sets;
  CHECK(a.MirrorMaintain(300)&&bundleEmoteBrains[0].phase==6&&a.sets==sets&&!a.tickEnabled);
  a.valid=true;f.reads=0;CHECK(a.MirrorMaintain(400)&&a.tickEnabled&&!bundleEmoteBrains[0].phase&&bundleEmoteResumes==1);
 }
 std::printf("Production emote brain pause/StopMovement/playback/resume: %d failures; bounded fresh phases, no game access\n",failures);
 return failures?1:0;
}
