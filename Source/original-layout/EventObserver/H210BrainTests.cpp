// Replay the production brain member with owned mock reads and native seams.
// This proves ordering/one-shot/refusal evidence, not live native authority.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <map>
#include "HostedH1Report.hpp"
namespace cfgprobe {constexpr unsigned ControllerPawn=0x370,ObjectClass=0x10;
namespace lifetime {struct SlotObservation {std::uintptr_t object=0;};}}
namespace cfgescape::disk {constexpr unsigned Controller=0x3A0;}
struct Reader {unsigned reads=0,cap=1536;std::map<std::uintptr_t,std::uintptr_t> values;
 bool Spend(unsigned n){reads+=n;return reads<cap;}
 bool Read(std::uintptr_t at,std::uintptr_t& value){if(!Spend(1))return false;value=values[at];return true;}};
namespace cfgjason {bool SameSlot(Reader& r,int,const cfgprobe::lifetime::SlotObservation&){return r.Spend(10);}}
namespace cfgobserver {bool Same(Reader& r,std::uintptr_t at,std::uintptr_t want){std::uintptr_t v=0;return r.Read(at,v)&&v==want;}}
namespace cfgspawn {enum {CounselorAI,Brain};struct Vec3 {float x=0,y=0,z=0;};
 bool ObjectIs(Reader& r,std::uintptr_t,std::uintptr_t){return r.Spend(10);}}
namespace cfgconvergencetravel {bool Brain(Reader& r,int,const cfgprobe::lifetime::SlotObservation&,std::uintptr_t,std::uintptr_t,cfgprobe::lifetime::SlotObservation& out){out.object=4;return r.Spend(10);}}
namespace cfghostedbrain::disk {constexpr unsigned RunBehaviorTree=1,RunBehaviorTreeExecutor=0x900,RunBehaviorTreeCode=0,IsRunning=2,IsRunningExecutor=3,IsRunningCode=4,IsPaused=5,IsPausedExecutor=6,IsPausedCode=7;}
namespace cfgbundle {bool Code(Reader& r,std::uintptr_t,unsigned,unsigned){return r.Spend(50);}}
struct {unsigned bots=1;struct Bot {std::uintptr_t pawn=0x1000,controller=0x2000;};std::array<Bot,1> roster;} bundleReport;
unsigned hostedBotCursor=0;
std::array<cfgh1::BotTrace,1> hostedBots;
std::array<cfgprobe::lifetime::SlotObservation,1> bundlePawnSlots{{{0x1000}}},bundleControllerSlots{{{0x2000}}};
struct {std::uintptr_t module=0x10000;} plan;
struct {struct {std::array<std::uintptr_t,2> classes{{1,2}};} resources;struct {std::uintptr_t counselorTree=0x3000;} primary;} spawnAssets;
struct Fixture {
 Reader r_;struct {int objectArray=0;} context;
 bool feature=true,tree=true,invoke=true,allowed=true,nativeReturn=true;unsigned positions=0,calls=0,bundleInvocationStage_=5;
 Fixture(){hostedBots={};hostedBots[0].pawn=0x1000;hostedBots[0].controller=0x2000;
  r_.values[0x2000+cfgprobe::ControllerPawn]=0x1000;r_.values[0x1000+cfgescape::disk::Controller]=0x2000;
  r_.values[0x2000+cfgprobe::ObjectClass]=7;r_.values[0x4000+0xB0]=plan.module+cfghostedbrain::disk::RunBehaviorTreeExecutor;}
 bool HostedContext(bool){return r_.Spend(600);}
 bool Tree(){return tree&&r_.Spend(200);}
 bool FeatureFunction(unsigned,std::uintptr_t& fn,std::uintptr_t& owner){fn=0x4000;owner=1;return feature&&r_.Spend(400);}
 bool NativeCallAllowed(){return allowed;}
 bool Invoke(std::uintptr_t,std::uintptr_t,void* p,bool){++calls;if(invoke)static_cast<unsigned char*>(p)[8]=nativeReturn?1:0;return invoke;}
 bool CarCall(unsigned,unsigned,unsigned,std::uintptr_t,void* p){*static_cast<unsigned char*>(p)=1;return true;}
 bool DonorNativePosition(const cfgprobe::lifetime::SlotObservation&,cfgspawn::Vec3&){++positions;return r_.Spend(900);}
#include "HostedCounselorBrainMembers.hpp"
};
int main(){unsigned checks=0,bad=0;auto check=[&](bool ok){++checks;if(!ok){++bad;std::printf("FAIL brain replay %u\n",checks);}};
 {Fixture f;check(f.HostedCounselorBrainPhase());check(f.calls==1&&hostedBots[0].startAttempts==1&&hostedBots[0].startReturned==1&&f.positions==0);
  f.r_.reads=0;check(!f.HostedCounselorBrainPhase()&&f.calls==1);}
 {Fixture f;f.feature=false;check(!f.HostedCounselorBrainPhase()&&hostedBots[0].first==42&&!f.calls);
  f.r_.reads=0;f.r_.values[0x2000+cfgprobe::ControllerPawn]=0;check(!f.HostedCounselorBrainPhase()&&hostedBots[0].last==11&&hostedBots[0].first==42);}
 {Fixture f;f.invoke=false;check(!f.HostedCounselorBrainPhase()&&f.calls==1&&hostedBots[0].first==55);
  f.r_.reads=0;check(!f.HostedCounselorBrainPhase()&&f.calls==1);}
 {Fixture f;f.nativeReturn=false;check(!f.HostedCounselorBrainPhase()&&hostedBots[0].first==5&&f.calls==1);
  f.r_.reads=0;check(!f.HostedCounselorBrainPhase()&&hostedBots[0].first==5&&f.calls==1);}
 {Fixture f;f.r_.values[0x2000+0x418]=4;hostedBots[0].running=1;check(f.HostedCounselorBrainPhase()&&!f.calls);}
 {Fixture f;f.r_.values[0x2000+0x418]=4;hostedBots[0].paused=1;check(f.HostedCounselorBrainPhase()&&!f.calls);}
 std::printf("H210 production brain ordering/first-refusal/ambiguous-no-replay/running-paused: %u checks, %u failures.\n",checks,bad);return bad?1:0;
}
