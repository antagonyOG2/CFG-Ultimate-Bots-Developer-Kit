// Exact production member replay; native calls target this fixture only.
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include "../RuntimeProbe/HostedCounselorBrainBindings.generated.hpp"
namespace cfgprobe {constexpr unsigned ControllerPawn=0x370;
namespace lifetime {struct SlotObservation{std::uintptr_t object=0;int serial=1;bool HasAssignedSerial()const{return serial>0;}};}}
namespace cfgescape::disk {constexpr unsigned Controller=0x3A0,PlayerState=0x388;}
namespace cfgreflection::arraydisk {constexpr unsigned FunctionParameterBytes=0xB6;}
namespace cfgcompat::cfg35::ai {struct Class{unsigned reflection,constructor;};constexpr std::array<Class,8> Classes{{{0,0},{0,0},{10,11},{0,0},{12,13}}};}
namespace cfghosted::birth35 {constexpr unsigned CounselorBehaviorTree=0x100;}
namespace cfgspawn {constexpr unsigned GenericDispatcherRva=0x1000;}
struct Reader{unsigned reads=0;std::map<std::uintptr_t,std::uintptr_t> data;
 template<class T>bool Read(std::uintptr_t at,T& out){++reads;auto i=data.find(at);if(i==data.end())return false;out=static_cast<T>(i->second);return true;}};
namespace cfgprobe {std::uintptr_t MatchingClassCache(std::uintptr_t a,std::uintptr_t b){return a==b?a:0;}}
namespace cfgjason {bool SameSlot(Reader&,int,const cfgprobe::lifetime::SlotObservation& s){return s.object!=0;}}
namespace cfgobserver {bool Same(Reader& r,std::uintptr_t at,std::uintptr_t v){std::uintptr_t x=0;return r.Read(at,x)&&x==v;}}
namespace cfgspawn {bool ObjectSlot(Reader&,int,std::uintptr_t v,cfgprobe::lifetime::SlotObservation& s){s.object=v;return v!=0;}
 bool ObjectIs(Reader&,std::uintptr_t v,std::uintptr_t t){return v&&t;}}
namespace cfghosted::native {bool Class(Reader&,std::uintptr_t,unsigned reflection,unsigned,std::uintptr_t& out){out=reflection==10?0x200:0x300;return true;}}
namespace cfgbundle {template<class T>bool Code(Reader&,std::uintptr_t,std::uintptr_t,const T&){return true;}
 bool NativeFrame(unsigned n,unsigned expected,unsigned storage){return n==expected&&storage==16;}}
struct Trace{std::uintptr_t pawn=0x400,controller=0x500,brain=0,tree=0;unsigned samples=0,last=0,first=0,reads=0,startAttempts=0,startReturned=0,running=2,paused=2;};
std::array<Trace,1> hostedBots;
struct {std::uintptr_t module=0,world=0x600,mode=0x700,controller=0x800,pawn=0x900;} plan;
struct {std::uintptr_t gameState=0xA00;} hostedTx;
struct {std::uintptr_t pawn=0x400;struct{cfgprobe::lifetime::SlotObservation controller{0x500},state{0xB00};} born;} hostedCounselor;
Reader* factoryReader=nullptr;bool factoryMutation=false;
namespace cfgmenu::population::detail {bool GetClass(std::uintptr_t,std::uintptr_t& out){out=0xE00;
 factoryReader->data[plan.module+cfghostedbrain::disk::RunBehaviorTree.cache]=out;
 if(factoryMutation)factoryReader->data[0x500+cfgprobe::ControllerPawn]=0;return true;}}
using Dispatch=void(__fastcall*)(void*,void*,void*);
unsigned dispatches=0;bool nativeReturn=true,nativeThrow=false;
void __fastcall Native(void*,void*,void* packet){++dispatches;if(nativeThrow)RaiseException(0xE1234567,0,0,nullptr);static_cast<unsigned char*>(packet)[8]=nativeReturn?1:0;}
struct Fixture{
 Reader r_;struct{int objectArray=0;}context;unsigned calls_=0;bool exception_=false,world=true,prepare=true,finish=true,uncertain=false;
 std::vector<std::uintptr_t> retained;
 Fixture(){hostedBots={};hostedBots[0].pawn=0x400;hostedBots[0].controller=0x500;dispatches=0;nativeReturn=true;nativeThrow=false;factoryReader=&r_;factoryMutation=false;
  plan.module=reinterpret_cast<std::uintptr_t>(&Native)-cfgspawn::GenericDispatcherRva;
  auto& v=r_.data;v[0x500+cfgprobe::ControllerPawn]=0x400;v[0x400+cfgescape::disk::Controller]=0x500;v[0x400+cfgescape::disk::PlayerState]=0xB00;v[0x500+cfgescape::disk::PlayerState]=0xB00;
  v[0xB00+0x88]=0x500;v[plan.mode+cfghosted::birth35::CounselorBehaviorTree]=0xC00;v[0x500+0x418]=0;
  const auto& b=cfghostedbrain::disk::RunBehaviorTree;
  v[plan.module+b.ownerReflection]=v[plan.module+b.ownerConstructor]=0xD00;v[plan.module+b.cache]=0xE00;
  v[0xE00+cfgreflection::arraydisk::FunctionParameterBytes]=9;v[0xE00+0x20]=0xD00;v[0xE00+0xB0]=plan.module+cfghostedbrain::disk::RunBehaviorTreeExecutor;}
 bool HostedWorld(){return world;}
 bool HostedInWorld(std::uintptr_t p){return p==0x400;}
 template<std::size_t N>bool HostedPrepare(const std::array<std::uintptr_t,N>& roots,unsigned){retained.assign(roots.begin(),roots.end());return prepare;}
 std::array<std::uintptr_t,2> HostedRoots(std::uintptr_t a,std::uintptr_t b){return {a,b};}
 bool RetainBundleReturnedObject(std::uintptr_t p){return p==0xE00;}
 bool FinishBundleNative(){if(!finish)BundleRetentionFailed();return finish;}
 void BundleRetentionFailed(){uncertain=true;}
#include "HostedCounselorBirthBrainMembers.hpp"
};
int main(){unsigned checks=0,failures=0;auto check=[&](bool b){++checks;if(!b)++failures;};
 {Fixture f;check(f.HostedCounselorBirthBrain()&&dispatches==1&&hostedBots[0].startReturned==1);
  check(std::find(f.retained.begin(),f.retained.end(),0xD00)!=f.retained.end()&&std::find(f.retained.begin(),f.retained.end(),0xE00)!=f.retained.end());
  check(!f.HostedCounselorBirthBrain()&&dispatches==1);}
 {Fixture f;f.world=false;check(!f.HostedCounselorBirthBrain()&&!dispatches);}
 {Fixture f;f.r_.data[0x500+cfgprobe::ControllerPawn]=0;check(!f.HostedCounselorBirthBrain()&&!dispatches);}
 {Fixture f;f.r_.data[0xE00+cfgreflection::arraydisk::FunctionParameterBytes]=16;check(!f.HostedCounselorBirthBrain()&&!dispatches);}
 {Fixture f;f.prepare=false;check(!f.HostedCounselorBirthBrain()&&!dispatches&&!hostedBots[0].startAttempts);}
 {Fixture f;nativeReturn=false;check(!f.HostedCounselorBirthBrain()&&dispatches==1);check(!f.HostedCounselorBirthBrain()&&dispatches==1);}
 {Fixture f;f.finish=false;check(!f.HostedCounselorBirthBrain()&&f.uncertain&&dispatches==1);check(!f.HostedCounselorBirthBrain()&&dispatches==1);}
 {Fixture f;nativeThrow=true;check(!f.HostedCounselorBirthBrain()&&f.uncertain&&f.exception_&&dispatches==1);check(!f.HostedCounselorBirthBrain()&&dispatches==1);}
 {Fixture f;f.r_.data[0x500+0x418]=0xF00;check(f.HostedCounselorBirthBrain()&&!dispatches&&hostedBots[0].brain==0xF00);}
 {Fixture f;f.r_.data[plan.module+cfghostedbrain::disk::RunBehaviorTree.cache]=0;check(f.HostedCounselorBirthBrain()&&dispatches==1&&f.calls_==2);}
 {Fixture f;f.r_.data[plan.module+cfghostedbrain::disk::RunBehaviorTree.cache]=0;factoryMutation=true;
  check(!f.HostedCounselorBirthBrain()&&!dispatches&&!hostedBots[0].startAttempts&&hostedBots[0].last==68);}
 std::printf("H217 production F3 birth brain: %u checks, %u failures (owned native seam; no adoption required).\n",checks,failures);return failures?1:0;}
