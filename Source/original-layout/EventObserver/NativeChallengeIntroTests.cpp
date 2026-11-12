#define CFG_CHALLENGE_FIXTURE_ONLY
#include "NativeChallengeOptionsTests.cpp"
#undef CFG_CHALLENGE_FIXTURE_ONLY
#include "NativeChallengeIntro.hpp"
#include "NativeChallengeRestart.hpp"
struct IntroFixture:ChallengeFixture {
 static constexpr std::uintptr_t Settings=0x790000,SettingsType=0x7A0000,SkipFunction=0x7B0000,FunctionType=0x7C0000;
 IntroFixture(){using namespace cfgprobe;namespace d=cfgoutro::disk;
  Phase("PreMatchIntro");r.Set(LevelType+ClassSuper,std::uintptr_t{});
  r.Set(Module+d::LevelClass,LevelType);r.Set(Module+d::LevelConstructor,LevelType);
  r.Set(World+d::PersistentLevel,Level);r.Set(Level+d::OwningWorld,World);r.Set(Level+d::WorldSettings,Settings);
  r.Set(Settings+ObjectClass,SettingsType);r.Set(Settings+0x20,Level);r.Set(Settings+0x110,std::uint8_t{3});
  r.Set(Settings+0x729,std::uint8_t{1});r.Set(Settings+0x7C8,std::uint8_t{});r.Set(SettingsType+ClassSuper,std::uintptr_t{});
  r.Set(Module+d::SCWorldSettingsClass,SettingsType);r.Set(Module+d::SCWorldSettingsConstructor,SettingsType);
  r.Set(Settings+lifetime::ObjectIndexOffset,int{7});r.Set(Registry+24*7,Settings);r.Set(Registry+24*7+16,int{107});
  r.Set(Module+lifetime::ObjectArrayRva+28,int{8});
  for(unsigned i=0;i<8;++i)r.Set(Registry+24*i+8,std::uint32_t{});
  r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallenges,SCState);
  r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallengesCtor,SCState);
  r.Set(Module+cfgchallengeescape::disk::PlayerController,ControllerType);
  r.Set(Module+cfgchallengeescape::disk::PlayerControllerCtor,ControllerType);
  Add(ControllerType,"NetConnection","ObjectProperty",0x578,1,8);r.Set(Controller+0x578,std::uintptr_t{});
  r.Set(Dossier+lifetime::ObjectIndexOffset,int{8});r.Set(Registry+24*8,Dossier);
  r.Set(Registry+24*8+16,int{0});r.Set(Registry+24*8+8,std::uint32_t{});r.Set(Module+lifetime::ObjectArrayRva+28,int{9});
  Label(SkipFunction,"SkipLevelIntroOutro");Label(FunctionType,"Function");
  r.Set(SkipFunction+ObjectClass,FunctionType);r.Set(SkipFunction+0x20,SettingsType);r.Set(SkipFunction+cfgreflection::disk::Next,std::uintptr_t{});
  r.Set(SkipFunction+0x88,std::uint32_t{0x04020401});r.Set(SkipFunction+0x8C,std::uint8_t{});r.Set(SkipFunction+0x8E,std::uint16_t{});
  r.Set(SkipFunction+cfgreflection::disk::StorageBytes,int{});r.Set(SkipFunction+0x68,std::uintptr_t{});
  r.Set(SkipFunction+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{});
  r.Set(SkipFunction+0xB0,Module+d::SkipExecutor);r.Set(Module+d::SkipCache,SkipFunction);
  r.Set(Module+d::SkipExecutor,d::SkipExecutorCode);r.Set(Module+d::SkipNative,cfgmenu::challengeintro::disk::Native);
  r.Set(Module+d::SkipFactory,cfgmenu::challengeintro::disk::Factory);
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
};
struct IntroEngine {
 ChallengeReader& r;unsigned calls=0,factories=0;bool fail=false;
 bool BeginPhase(){return true;}
 bool Factory(std::uintptr_t at,std::uintptr_t& out){++factories;
  if(at!=Fixture::Module+cfgoutro::disk::SkipFactory)return false;
  r.Set(Fixture::Module+cfgoutro::disk::SkipCache,IntroFixture::SkipFunction);out=IntroFixture::SkipFunction;return true;}
 bool Invoke(std::uintptr_t target,std::uintptr_t fn,void* packet){++calls;
  return !fail&&target==IntroFixture::Settings&&fn==IntroFixture::SkipFunction&&packet&&*static_cast<unsigned char*>(packet)==0;}
};
// J31 changes the production contract: Step posts a request; only the J32
// retained gameplay consumer can execute the native skip. This fixture models
// that consumer's *read-only* CurrentRequest gate and its single Invoke, not a
// GC/root lease or a native callback. Full retained callback tests run on Win32.
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
struct IntroFixtureRetainedHost {
 cfgmenu::challengeintro::Runtime& runtime;ChallengeReader& reader;IntroEngine& engine;
 std::uintptr_t callback=0;std::uint32_t thread=0;unsigned stage=0;
 bool retained=true;
 bool CurrentIntro(const cfgmenu::challengeintro::DispatchRequest& request){
  return retained&&runtime.CurrentRequest(reader,request,callback,thread,stage);
 }
 bool InvokeRetainedIntro(const cfgmenu::challengeintro::DispatchRequest& request){
  if(!retained||!engine.BeginPhase())return false;
  std::array<unsigned char,16> packet{};
  return engine.Invoke(request.settings,request.function,packet.data());
 }
};
static bool IntroService(cfgmenu::challengeintro::Runtime& runtime,ChallengeReader& reader,IntroEngine& engine,
 std::uintptr_t callback,std::uint64_t now,bool retained=true){
 IntroFixtureRetainedHost host{runtime,reader,engine,callback,42,0,retained};
 cfgmenu::challengeintro::ScopeDiagnostics scope{};
 const bool returned=cfgmenu::challengeintro::Consume(runtime.dispatch,host,now,scope);
 if(returned)++runtime.input.returns; // the production frontend increments on consumer return
 return returned;
}
#endif
#ifndef CFG_CHALLENGE_INTRO_TEST_MAIN
#define CFG_CHALLENGE_INTRO_TEST_MAIN main
#endif
int CFG_CHALLENGE_INTRO_TEST_MAIN(){unsigned checks=0,failures=0,peak=0;const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 using namespace cfgmenu::challengeintro;
 {InputPolicy p;p.Observe(1,true,true,true,0);check(!p.Due(0),"held menu Start never auto-skips");
  p.Observe(1,true,true,false,50);p.Observe(1,true,true,true,100);check(p.Due(100),"fresh released foreground Start edge");
  p.Consume();p.Observe(1,true,true,false,150);p.Observe(1,true,true,true,200);check(!p.Due(200),"one action per intro world");
  p.Observe(2,true,true,true,250);check(!p.Due(250),"new-world held press not inherited");
  p.Observe(2,true,true,false,300);p.Observe(2,true,true,true,350);check(p.Due(350)&&!p.Due(1350),"pending edge bounded1s");
  p.Observe(2,false,true,true,400);check(!p.Due(400),"phase departure discards pending skip");}
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime runtime;runtime.Arm(f.Menu());
  runtime.Step(r,engine,f.Mode,42,0,true,true);runtime.Step(r,engine,f.Mode,42,50,true,false);
  r.count=0;runtime.Step(r,engine,f.Pawn,42,100,true,true);
  check(runtime.PollDue(100)&&!engine.calls&&r.count<10,"unrelated pawn does not consume due owner poll or call native");
  runtime.Step(r,engine,f.Mode,42,100,true,true);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  check(runtime.dispatch.Pending()&&!engine.calls&&runtime.input.returns==0,"same-time exact owner queues without a direct Invoke");
  IntroService(runtime,r,engine,f.Mode,101);
#endif
  check(engine.calls==1&&runtime.input.returns==1,"same-time exact owner services fresh edge after unrelated callback");}
 for(unsigned fault=0;fault<19;++fault){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime runtime;runtime.Arm(f.Menu());
  runtime.Step(r,engine,f.Mode,42,0,true,true);runtime.Step(r,engine,f.Mode,42,50,true,false);check(!engine.calls,"no skip before fresh input");
  if(fault==1)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==2)r.Set(f.Module+cfgprobe::NativeClasses[cfgprobe::ChallengesMode].reflectionCacheRva,f.Killer);
  if(fault==3)r.Set(f.Settings+0x729,std::uint8_t{});
  if(fault==4)r.Set(f.Settings+0x7C8,std::uint8_t{1});
  if(fault==5)r.Set(f.Registry+24*7+16,int{0});
  if(fault==6)r.Set(f.Settings+0x20,f.World);
  if(fault==7)r.Set(f.SkipFunction+0x88,std::uint32_t{0x04020001});
  if(fault==8)r.Set(f.SkipFunction+0x8E,std::uint16_t{1});
  if(fault==9)r.Set(f.Module+cfgoutro::disk::SkipNative,std::uint8_t{0xCC});
  if(fault==10)r.Set(f.Controller+cfgprobe::ControllerPlayer,f.Mode);
  if(fault==11)r.Set(f.Module+0x03198110,std::uint32_t{1});
  if(fault==12)r.Set(f.SkipFunction+0x68,f.State);
  if(fault==13)r.Set(f.Module+cfgoutro::disk::SkipCache,std::uintptr_t{});
  if(fault==14)engine.fail=true;
  if(fault==16){r.Set(f.Registry+24*4+16,int{0});r.Set(f.Registry+24*7+16,int{0});}
  if(fault==17)r.Set(f.Registry+24*7+8,std::uint32_t{0x10000000});
  if(fault==18)r.Set(f.Registry+24*4,f.Settings);
  r.count=0;runtime.Step(r,engine,fault==15?f.Pawn:f.Mode,42,100,true,true);peak=(std::max)(peak,r.count);
  if(!fault||fault==13||fault==14)std::printf("IntroFixture fault%u stage%u reads%u edge%u pending%u calls%u factories%u\n",fault,runtime.failure,r.count,runtime.input.edges,runtime.input.pending,engine.calls,engine.factories);
  if(fault==13){check(engine.factories==1&&!engine.calls&&runtime.input.Due(150),"cold factory only; edge retained, no same-slice skip");
   r.count=0;runtime.Step(r,engine,f.Mode,42,150,true,false);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
   check(runtime.dispatch.Pending()&&!engine.calls,"warm factory retry only posts; never calls directly");
   IntroService(runtime,r,engine,f.Mode,151);
#endif
   check(engine.calls==1&&runtime.input.returns==1,"fresh next callback repeats full ownership before skip");}
  else {
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
   const bool strongAllowed=CFG_J33_INTRO_STRONG_CHILDREN&&(fault==5||fault==16);
   const bool shouldPost=!fault||fault==14||strongAllowed||(CFG_J30_INTRO_SKIP_35_VALIDATE&&fault==12);
   if(shouldPost)check(runtime.dispatch.Pending()&&!engine.calls,"valid native intro request posted with no direct Invoke");
   else check(!runtime.dispatch.Pending()&&!engine.calls,"invalid owner, slot or metadata refuses before request posting");
   if(fault==5||fault==16)check((runtime.dispatch.posted==1)==static_cast<bool>(CFG_J33_INTRO_STRONG_CHILDREN),
       "unassigned child request is accepted only after J33 strong ownership evidence");
   IntroService(runtime,r,engine,f.Mode,101);
   if(fault==14)check(runtime.dispatch.claimed==1&&runtime.dispatch.reason==DispatchReason::NativeRefused,
       "native refusal consumed at the retained consumer and never replayed");
   else if(shouldPost)check(runtime.dispatch.claimed==1&&runtime.dispatch.returned==1,
       "posted request consumed once by the retained consumer");
#endif
   if(fault==14)check(engine.calls==1&&runtime.input.attempts==1&&!runtime.input.returns,"native refusal consumed exactly once");
   else {
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
    const auto expected=(!fault||(CFG_J33_INTRO_STRONG_CHILDREN&&(fault==5||fault==16))||
      (CFG_J30_INTRO_SKIP_35_VALIDATE&&fault==12))?1u:0u;
#else
    const auto expected=(!fault||fault==5||fault==16||(CFG_J30_INTRO_SKIP_35_VALIDATE&&fault==12))?1u:0u;
#endif
    check(engine.calls==expected&&runtime.input.returns==expected,"exact intro owner accepts valid request; refuses stale/unassigned owner and invalid metadata");
   }
  }
  const auto calls=engine.calls;r.count=0;runtime.Step(r,engine,fault==15?f.Pawn:f.Mode,42,200,true,true);check(engine.calls==calls,"held button never repeats action");
  if(fault==17){check(runtime.lastRefusal==4&&runtime.ownerFailure==5,"sticky precise native owner flag refusal");
   r.count=0;runtime.Step(r,engine,f.Mode,42,1250,true,false);check(runtime.failure==0&&runtime.lastRefusal==4&&runtime.ownerFailure==5,"idle input cannot erase last refused edge receipt");}
 }
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH && CFG_J33_INTRO_STRONG_CHILDREN
 // This is the real CurrentRequest reader/owner path, not only a policy mock.
 // Post with two unassigned strong children, then break each exact link or
 // trip either GC flag before consumer service: NO call, NO replay.
 for(unsigned fault=0;fault<6;++fault){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;
  IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  r.Set(f.Registry+24*4+16,int{0});r.Set(f.Registry+24*7+16,int{0});
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  intro.Step(r,engine,f.Mode,42,100,true,true);check(intro.dispatch.Pending(),"J33 strong children posted with no Invoke");
  if(fault==0)r.Set(f.World+cfgoutro::disk::PersistentLevel,f.Settings);
  if(fault==1)r.Set(f.Level+cfgoutro::disk::WorldSettings,f.World);
  if(fault==2)r.Set(f.Level+cfgoutro::disk::OwningWorld,f.Mode);
  if(fault==3)r.Set(f.Registry+24*4+8,std::uint32_t{0x10000000});
  if(fault==4)r.Set(f.Registry+24*7+8,std::uint32_t{0x20000000});
  if(fault==5)r.Set(f.Registry+24*4,f.Mode); // repurposed slot index
  check(!IntroService(intro,r,engine,f.Mode,101)&&!engine.calls&&intro.dispatch.refused==1,
      "J33 broken strong child at consume refuses without Invoke");
  check(!IntroService(intro,r,engine,f.Mode,102)&&!engine.calls,"J33 consumed refusal cannot replay");
 }
#endif
 // J30 validates the 3.5 post-match function proof, not the old exact
 // flags/cleanup/script checks. A deliberately non-3.4 shape must enter once.
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  r.Set(f.SkipFunction+0x88,std::uint32_t{0x02000400});r.Set(f.SkipFunction+0x68,f.Mode);
  r.Set(f.SkipFunction+cfgreflection::disk::StorageBytes,int{12});
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);peak=(std::max)(peak,r.count);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  check((!CFG_J30_INTRO_SKIP_35_VALIDATE||intro.dispatch.Pending())&&!engine.calls,"J30 valid 3.5 metadata queues only");
  IntroService(intro,r,engine,f.Mode,101);
#endif
  if(CFG_J30_INTRO_SKIP_35_VALIDATE)check(engine.calls==1&&intro.input.attempts==1&&intro.introValidateStage==0,"J30 3.5 flags/cleanup/storage call once");
  else check(engine.calls==0&&intro.lastRefusal==6&&intro.introValidateStage==5,"J30 OFF keeps legacy exact flags");
  r.count=0;intro.Step(r,engine,f.Mode,42,150,true,true);check(engine.calls==(CFG_J30_INTRO_SKIP_35_VALIDATE?1u:0u),"3.5 new metadata never replays");}
#if CFG_J30_INTRO_SKIP_35_VALIDATE
 // Stage-specific fail-closed cases: exactly zero native calls and no input
 // consumption for incorrect reflected label, flags, executor, native bytes.
 for(unsigned fault=0;fault<6;++fault){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  const unsigned expected[]={3,5,11,12,13,7};
  if(fault==0){f.Label(f.SkipFunction,"NotSkipLevelIntroOutro");f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});static_cast<StageReader&>(r)=f.r;}
  if(fault==1)r.Set(f.SkipFunction+0x88,std::uint32_t{0x02000000});
  if(fault==2)r.Set(f.SkipFunction+0xB0,f.Mode);
  if(fault==3){auto bytes=cfgoutro::disk::SkipExecutorCode;bytes[0]^=0xFF;r.Set(f.Module+cfgoutro::disk::SkipExecutor,bytes);}
  if(fault==4)r.Set(f.Module+cfgoutro::disk::SkipNative,std::uint8_t{0xCC});
  if(fault==5)r.Set(f.SkipFunction+0x8E,std::uint16_t{1});
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);peak=(std::max)(peak,r.count);
  check(!engine.calls&&intro.lastRefusal==6&&intro.introValidateStage==expected[fault]&&intro.input.attempts==0,"J30 failed function proof names stage; never invokes");
 }
#endif
 check(peak<8192,"production native skip stays within shared8192read budget");
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;
  cfgmenu::challengerestart::Runtime replay;cfgchallengeescape::scope::Witness unused;
  replay.Arm(f.Menu(),{});r.count=0;check(!replay.Observe(r,f.Mode,42,0,unused),"initial attempt recorded without replay");
  intro.Arm(f.Menu());r.count=0;intro.Step(r,engine,f.Mode,42,0,true,true);
  r.count=0;intro.Step(r,engine,f.Mode,42,50,true,false);r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,101);
#endif
  check(engine.calls==1,"first attempt skipped once");
  f.Phase("PreMatchIntro");static_cast<StageReader&>(r)=f.r;
  r.Set(f.Registry+16,int{200});r.Set(f.Registry+24+16,int{201});r.count=0;
  const bool restarted=replay.Observe(r,f.Mode,42,200,unused);check(restarted,"fresh same-map intro creates replay");
  if(restarted)intro.Arm(replay.Source());r.count=0;intro.Step(r,engine,f.Mode,42,200,true,true);
  check(engine.calls==1,"held Start cannot skip new attempt");
  r.count=0;intro.Step(r,engine,f.Mode,42,250,true,false);r.count=0;intro.Step(r,engine,f.Mode,42,300,true,true);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,301);
#endif
  check(engine.calls==2&&intro.input.returns==1,"second fresh intro skip after coordinator rearm");
 }
#if CFG_J29_CHALLENGE_INTRO_SCOPE
 // Current-owner authorization can establish a short-lived scope; a foreign
 // scoped reader cannot be borrowed or replaced for a native intro action.
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,101);
#endif
  check(engine.calls==1&&intro.scopeFailure==0,"J29 fresh Start with proven current owner gets one skip call");
  r.count=0;intro.Step(r,engine,f.Mode,42,200,true,true);
  check(engine.calls==1,"J29 consumed Start cannot issue a second native call");}
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  auto foreign=[](ChallengeReader&,void*){return true;};int token=0;
  {cfgprimary::ScopedLookupEligibility<ChallengeReader> occupied(f.Module,&token,foreign,true);
   check(occupied.Entered(),"J29 setup occupying a different lookup scope");
   r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);
   check(!engine.calls&&intro.lastRefusal==8&&intro.scopeFailure==2,"J29 foreign scope refuses native intro");}
  r.count=0;intro.Step(r,engine,f.Mode,42,150,true,false);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,151);
#endif
  check(engine.calls==1&&intro.input.attempts==1,"J29 fresh retry after scope refusal calls once");}
 for(unsigned veto=0;veto<2;++veto){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  if(veto)r.Set(f.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
  else r.Set(f.Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
  r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);
  check(!engine.calls&&intro.lastRefusal==8&&intro.scopeFailure==1,"J29 active GC/serialization refuses scope and native action");}
#endif
 // Donor Skip Intro on Replay: only a proved same-mission fresh attempt, once,
 // through the same intro owner/phase/witness proofs; never the first attempt.
 for(unsigned enabled=0;enabled<2;++enabled){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;
  cfgmenu::challengerestart::Runtime replay;cfgchallengeescape::scope::Witness unused;
  replay.Arm(f.Menu(),{});r.count=0;replay.Observe(r,f.Mode,42,0,unused);
  intro.Arm(f.Menu());r.count=0;intro.Step(r,engine,f.Mode,42,0,false,false);r.count=0;intro.Step(r,engine,f.Mode,42,50,false,false);
  check(!engine.calls&&!intro.ReplayPending(),"first attempt never skips its intro without Start");
  f.Phase("PreMatchIntro");static_cast<StageReader&>(r)=f.r;
  r.Set(f.Registry+16,int{200});r.Set(f.Registry+24+16,int{201});r.count=0;
  const bool restarted=replay.Observe(r,f.Mode,42,200,unused);check(restarted,"same mission fresh attempt detected");
  if(restarted)intro.Arm(replay.Source(),enabled!=0);
  check(intro.ReplayPending()==(enabled!=0),"replay request follows the committed option");
  r.count=0;intro.Step(r,engine,f.Mode,42,200,false,false);peak=(std::max)(peak,r.count);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  if(enabled)check(intro.dispatch.Pending()&&!engine.calls&&!intro.ReplayPending()&&!intro.Armed(),
      "replay skip posts exactly once without Start");
  IntroService(intro,r,engine,f.Mode,201);
#endif
  if(enabled)check(engine.calls==1&&intro.input.returns==1&&!intro.ReplayPending()&&!intro.Armed(),"enabled replay skips the intro once without Start");
  else check(!engine.calls&&intro.Armed(),"Off replay keeps the intro playing");
  r.count=0;intro.Step(r,engine,f.Mode,42,250,true,true);r.count=0;intro.Step(r,engine,f.Mode,42,300,true,false);
  r.count=0;intro.Step(r,engine,f.Mode,42,350,true,true);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,351);
#endif
  check(engine.calls==1,enabled?"consumed replay skip never repeats":"Off replay still allows a manual Start skip");
 }
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;
  f.Phase("InProgress");static_cast<StageReader&>(r)=f.r;intro.Arm(f.Menu(),true);
  r.count=0;intro.Step(r,engine,f.Mode,42,0,false,false);
  check(!engine.calls&&!intro.Armed()&&!intro.ReplayPending(),"replay request never skips outside PreMatchIntro");}
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;
  f.r.Set(IntroFixture::Settings+0x729,std::uint8_t{});static_cast<StageReader&>(r)=f.r;intro.Arm(f.Menu(),true);
  r.count=0;intro.Step(r,engine,f.Mode,42,0,false,false);
  check(!engine.calls&&intro.ReplayPending(),"inactive native intro witness refuses and retains the replay request");
  f.r.Set(IntroFixture::Settings+0x729,std::uint8_t{1});static_cast<StageReader&>(r)=f.r;
  r.count=0;intro.Step(r,engine,f.Mode,42,50,false,false);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,51);
#endif
  check(engine.calls==1&&!intro.ReplayPending(),"replay skip proceeds once the native intro is active");}
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};engine.fail=true;Runtime intro;intro.Arm(f.Menu(),true);
  r.count=0;intro.Step(r,engine,f.Mode,42,0,false,false);r.count=0;intro.Step(r,engine,f.Mode,42,50,false,false);
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
  IntroService(intro,r,engine,f.Mode,51);
  check(intro.dispatch.reason==DispatchReason::NativeRefused&&intro.dispatch.claimed==1,
      "refused replay skip claimed once at retained consumer");
#endif
  check(engine.calls==1&&!intro.ReplayPending(),"refused native replay skip is consumed, never retried");}
#if CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH
 // Request can be refused AFTER Step posts, and must not retry when the world,
 // owner, phase, function, callback or retained capability changes.
 for(unsigned fault=0;fault<8;++fault){IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  r.count=0;intro.Step(r,engine,f.Mode,42,100,true,true);
  check(intro.dispatch.Pending()&&!engine.calls,"request posted before delayed consumer tamper");
  const auto request=intro.dispatch.request;
  if(fault==0)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==1)r.Set(f.Registry+24*7+16,int{999});
  if(fault==2){f.Phase("InProgress");static_cast<StageReader&>(r)=f.r;}
  if(fault==3)r.Set(f.SkipFunction+0x88,std::uint32_t{});
  if(fault==4)r.Set(f.Module+cfgoutro::disk::SkipCache,std::uintptr_t{});
  if(fault==5)r.Set(f.Registry+24*7+8,std::uint32_t{0x10000000});
  const auto callback=fault==6?f.Pawn:f.Mode;
  const auto now=fault==7?request.expires:101;
  const bool returned=IntroService(intro,r,engine,callback,now);
  check(!returned&&!engine.calls&&intro.dispatch.refused==1&&!intro.dispatch.Pending(),
       "stale or expired request fails closed without a native call");
  if(fault==7)check(intro.dispatch.reason==DispatchReason::Expired,"expired pending request refused");
  else check(intro.dispatch.reason==DispatchReason::StaleOrUnproven,"tampered pending request fails CurrentRequest");
  const auto claims=intro.dispatch.claimed;
  IntroService(intro,r,engine,f.Mode,now+1);
  check(!engine.calls&&intro.dispatch.claimed==claims&&intro.dispatch.refused==1,
       "post-refusal consumer cannot replay native skip");
 }
 {IntroFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;IntroEngine engine{r};Runtime intro;intro.Arm(f.Menu());
  intro.Step(r,engine,f.Mode,42,0,true,true);intro.Step(r,engine,f.Mode,42,50,true,false);
  intro.Step(r,engine,f.Mode,42,100,true,true);
  check(intro.dispatch.Pending()&&!engine.calls,"one-shot retained request ready");
  IntroService(intro,r,engine,f.Mode,101,false);
  check(!engine.calls&&intro.dispatch.reason==DispatchReason::StaleOrUnproven&&intro.dispatch.claimed==0,
      "missing retained fixture capability refuses before Invoke");
 }
#endif
 std::printf("Challenge native intro Start edge/owner/phase/full-code/once path: %u checks %u failures peak%u/8192 reads. Owned process only.\n",checks,failures,peak);
 return failures?1:0;
}
