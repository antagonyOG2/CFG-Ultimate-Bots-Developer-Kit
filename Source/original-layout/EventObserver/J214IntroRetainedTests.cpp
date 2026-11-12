#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG_J214_INTRO_END_TO_END 1
#define CFG_CHALLENGE_INTRO_TEST_MAIN IntroObservationBaselineMain
#include "NativeChallengeIntroTests.cpp"
#undef CFG_CHALLENGE_INTRO_TEST_MAIN
#include "CallbackRootLease.hpp"
#include "NativeMenuRetention.hpp"
#include "NativeMenuDispatch.hpp"
#include <windows.h>
namespace {
ChallengeReader* active=nullptr;bool windowHeld=false,callbackComplete=true;
struct Reader {
 unsigned reads=0,cap=8192;
 template<class T>bool Read(std::uintptr_t at,T& out){return active&&reads++<cap&&active->Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){return active&&reads++<cap&&active->ReadBytes(at,out,bytes);}
};
struct RegistryState {cfggcretain::ReferencerList list{0x10000,0,64};
 std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};} roots;
}
namespace cfgtick {
// Owned completed-callback witness ONLY; no game address/return-PC is claimed.
inline bool Observe(Reader&,std::uintptr_t,std::uintptr_t actor,std::uintptr_t function,std::uintptr_t pc){
 return callbackComplete&&actor==Fixture::Mode&&function==0x810000&&pc==0x820000;
}
}
namespace {
class GcRegistryAccess {
public:
 struct NativeContextProof {void* context=nullptr;bool(*current)(Reader&,void*)=nullptr;
  bool(*completed)(Reader&,void*,std::uintptr_t,std::uintptr_t)=nullptr;};
private:
 NativeContextProof proof_;Reader reader_;bool held_=false;
 bool Current(){return proof_.context&&proof_.current&&proof_.current(reader_,proof_.context);}
public:
 explicit GcRegistryAccess(NativeContextProof proof,unsigned cap=8192):proof_(proof){reader_.cap=cap;}
 bool Begin(std::uintptr_t f,std::uintptr_t pc){if(windowHeld||!Current()||!proof_.completed(reader_,proof_.context,f,pc))return false;
  held_=windowHeld=true;return true;}
 void End(){if(held_){held_=windowHeld=false;}}
 bool ProvenSlotWindow(){return held_&&windowHeld&&Current();}
 bool NativeEntryUnlocked(){return !windowHeld&&Current();}
 bool Snapshot(cfggcretain::ReferencerList& list,decltype(roots.entries)& entries){if(!ProvenSlotWindow())return false;list=roots.list;entries=roots.entries;return true;}
 bool Register(std::uintptr_t address){if(!ProvenSlotWindow())return false;roots.entries[static_cast<unsigned>(roots.list.count++)]=address;return true;}
};
#include "NativeChallengeRetainedScope.hpp"
unsigned nativeCalls=0;bool transition=true;
struct OwnedPlatform {
 static DWORD Thread(){return GetCurrentThreadId();}
 static std::uintptr_t Module(){return Fixture::Module;}
 static bool Read(std::uintptr_t at,void* out,std::size_t bytes){return active&&active->ReadBytes(at,out,bytes);}
 static void Invoke(std::uintptr_t address,std::uintptr_t target,std::uintptr_t function,void* packet){
  if(address!=Fixture::Module+cfgspawn::GenericDispatcherRva||target!=IntroFixture::Settings||
     function!=IntroFixture::SkipFunction||!packet)RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);
  ++nativeCalls;if(transition)active->Set(IntroFixture::Settings+0x729,std::uint8_t{0});
 }
 static std::uintptr_t Factory(std::uintptr_t){return 0;}
};
using RetainedEngine=cfgmenu::rows::NativeDispatchBase<OwnedPlatform>;
bool FaultMenuDispatcher(RetainedEngine& engine){
 // An owned menu-only scope models a returned menu transition whose output
 // proof was refused. No collector/root bank is touched by this stub.
 bool current=true;cfgmenu::rows::NativeRetentionHooks h{&current,
  [](void*,std::uintptr_t){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t,void*){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,void*){return false;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return false;},
  [](void*,std::uintptr_t,std::uintptr_t){return false;},
  [](void*){},[](void*){},[](void*){return true;},nullptr};
 cfgmenu::rows::NativeRetentionScope menu(Fixture::Module,h,true);
 std::array<unsigned char,16> packet{};
 transition=false;
 return engine.BeginPhase()&&!engine.Invoke(IntroFixture::Settings,IntroFixture::SkipFunction,packet.data())&&engine.Faulted();
}
struct Consumer {
 cfgmenu::challengeintro::Runtime& runtime;RetainedEngine& engine;std::uint32_t thread;
 bool CurrentIntro(const cfgmenu::challengeintro::DispatchRequest& request){Reader r;unsigned stage=0;
  return cfgmenu::rows::NativeRetentionScope::Current(request.module)&&runtime.CurrentRequest(r,request,Fixture::Mode,thread,stage);}
 bool InvokeRetainedIntro(const cfgmenu::challengeintro::DispatchRequest& request){
  if(!engine.BeginPhase())return false;std::array<unsigned char,16> packet{};
  return engine.Invoke(request.settings,request.function,packet.data());}
};
}
#ifndef CFG_J214_INTRO_RETAINED_TEST_MAIN
#define CFG_J214_INTRO_RETAINED_TEST_MAIN main
#endif
int CFG_J214_INTRO_RETAINED_TEST_MAIN(){unsigned failures=0,checks=0;const auto check=[&](bool good,const char* text){++checks;if(!good){++failures;std::printf("FAIL %s\n",text);}};
 using namespace cfgmenu::challengeintro;
 for(unsigned fault=0;fault<9;++fault){
  IntroFixture f;ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=1000000;active=&memory;
  const auto thread=GetCurrentThreadId();memory.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,thread);
  memory.Set(f.SkipFunction+cfgprobe::lifetime::ObjectIndexOffset,int{9});memory.Set(f.Registry+24*9,f.SkipFunction);
  memory.Set(f.Registry+24*9+16,int{109});memory.Set(f.Registry+24*9+8,std::uint32_t{});memory.Set(f.SkipFunction+8,std::uint32_t{});
  memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{10});
  memory.Set(f.Module+cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix);
  // Exact live child case: no allocator, no promoting zero into a root lease.
  memory.Set(f.Registry+24*4+16,int{0});memory.Set(f.Registry+24*7+16,int{0});
  auto menu=f.Menu();menu.thread=thread;Runtime runtime;runtime.Arm(menu,true);
  IntroEngine producer{memory};runtime.Step(memory,producer,f.Mode,thread,100,true,false);
  check(runtime.dispatch.Pending()&&producer.calls==0,"producer posts without native skip");
  if(!runtime.dispatch.Pending())continue;
  const auto request=runtime.dispatch.request;unsigned stage=0;
  callbackComplete=fault!=1;
  if(fault==2)memory.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==3)memory.Set(f.Registry+24*9+8,std::uint32_t{0x20000000}); // pending-kill native function must refuse
  if(fault==7)memory.Set(f.Registry+24*9+16,int{0}); // exact cached function uses registered root, no weak serial assignment
  if(fault==8)memory.Set(f.SkipFunction+8,std::uint32_t{0x10});
  RetainedEngine engine{f.Module,thread,true},isolated{f.Module,thread,true};
  if(fault==6)check(FaultMenuDispatcher(engine),"actual persistent menu dispatcher quarantines refused output");
  transition=fault!=4;nativeCalls=0;
  const auto consumeAt=fault==5?request.expires:(fault==4?request.expires-1:std::uint64_t{101});
  {
   ChallengeIntroRootProvider provider(runtime,request,f.Mode,0x810000,0x820000,thread,stage);
   cfgmenu::rows::NativeRetentionScope scope(request.module,provider.Hooks(),provider.Admitted());
   if(fault==6)check(!engine.BeginPhase(),"quarantined menu cannot service independent gameplay intro");
   Consumer host{runtime,fault==6?isolated:engine,thread};ScopeDiagnostics d;
   const bool done=Consume(runtime.dispatch,host,consumeAt,d);
   const bool expected=fault==0||fault==4||fault==6||fault==7;
   if(fault==3)check(runtime.dispatch.claimed==1&&runtime.dispatch.reason==DispatchReason::NativeRefused,
     "replays live Claimed1/Reason5 as native refusal, not expiry");
   check(done==expected,"real provider/root lease completes or rejects the expected fault");
   check(nativeCalls==(expected?1u:0u),"exactly one simulated native Invoke, none on refused inputs");
   if(fault==7){int serial=-1;memory.Read(f.Registry+24*9+16,serial);check(serial==0,"native asset rooting never allocates or invents a weak serial");}
   check(!Consume(runtime.dispatch,host,consumeAt+1,d),"claimed action never replays");
  }
  check(!windowHeld&&frontendChallengeRootBank.Healthy(),"registry windows close and root bank stays healthy");
 }
 active=nullptr;std::printf("J214 actual ChallengeIntroRootProvider + real callback root lease: %u checks %u failures; registry/native platform simulated.\n",checks,failures);
 return failures?1:0;
}
