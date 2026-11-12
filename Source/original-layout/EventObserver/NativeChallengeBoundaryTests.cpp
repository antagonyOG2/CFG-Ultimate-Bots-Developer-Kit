#define CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeActorsTests.cpp"
#undef CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeBoundaryRuntime.hpp"
#include "NativeChallengeBoundaryWriter.hpp"
#include <functional>
#include <string>
namespace boundary=cfgchallengeboundary;
struct BoundaryFixture:ActorFixture {
 static constexpr std::uintptr_t Dispatcher=0x3400000,DispatcherClass=0x3410000,ClassType=0x3420000,FunctionType=0x3430000;
 std::array<std::uintptr_t,3> funcs{{0x3440000,0x3450000,0x3460000}},scripts{{0x3470000,0x3480000,0x3490000}};
 template<std::size_t N>void Locals(std::uintptr_t fn,const std::array<boundary::Local,N>& specs){
  for(std::size_t j=N;j;--j){const auto& s=specs[j-1];std::string name,kind;
   for(auto ch:s.name)name.push_back(static_cast<char>(ch));for(auto ch:s.kind)kind.push_back(static_cast<char>(ch));
   Add(fn,name.c_str(),kind.c_str(),s.offset,1,s.bytes);r.Set(heads[fn]+cfgreflection::disk::PropertyFlags,s.flags);
  }
 }
 BoundaryFixture(){
  r.Set(DispatcherClass+cfgprobe::ClassSuper,ActorType);r.Set(DispatcherClass+cfgspawn::ClassDefaultObject,DispatcherClass+0x8000);
  Object(Dispatcher,DispatcherClass,Level);Object(DispatcherClass,ClassType,WorldPackage);
  Label(DispatcherClass,"SinglePlayer_Dispatcher_C");Label(FunctionType,"Function");
  for(unsigned i=0;i<3;++i){Object(funcs[i],FunctionType,DispatcherClass);std::string name;
   for(auto ch:boundary::Names[i])name.push_back(static_cast<char>(ch));Label(funcs[i],name.c_str());
   r.Set(funcs[i]+cfgreflection::disk::Next,i==2?std::uintptr_t{}:funcs[i+1]);
   r.Set(funcs[i]+cfgreflection::disk::FunctionFlags,std::uint32_t{i==2?0x0C020000u:0x0C820000u});
   r.Set(funcs[i]+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t(i==2?0:8));
   r.Set(funcs[i]+cfgreflection::disk::StorageBytes,int{i==0?160:i==1?184:17});
   r.Set(funcs[i]+0xB0,Module+cfgmenu::startdisk::ScriptExecutor);r.Set(funcs[i]+0x68,std::uintptr_t{});
   const int length=i==0?692:i==1?922:161;std::array<unsigned char,1024> bytes{};bytes.fill(0x55);
   bytes[length-3]=0xA5;bytes[length-2]=0x39;bytes[length-1]=0xE5;
   if(i==2){bytes[0]=0x0D;bytes[1]=0x76;}else{bytes[0]=0x22;const auto target=static_cast<std::uint32_t>(length-3);std::memcpy(bytes.data()+1,&target,4);}
   r.Set(scripts[i],bytes);r.Set(funcs[i]+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{scripts[i],length,length});
   int index=0;r.Read(funcs[i]+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(Registry+24ULL*static_cast<unsigned>(index)+16,int{});
  }
  Locals(funcs[0],boundary::InsideLocals);Locals(funcs[1],boundary::OutsideLocals);Locals(funcs[2],boundary::RestartLocals);
  auto local=heads[funcs[1]];for(unsigned i=0;i<14;++i)r.Read(local+cfgreflection::disk::Next,local);r.Set(funcs[1]+0x68,local);
  r.Set(DispatcherClass+cfgreflection::disk::Children,funcs[0]);
  for(const auto object:{Dispatcher,DispatcherClass,Level}){int index=0;r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(Registry+24ULL*static_cast<unsigned>(index)+16,int{});}
  const std::array<std::uintptr_t,1> roster{{Dispatcher}};r.Set(ActorArray,roster);
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,1,1});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
 bool Capture(ChallengeReader& reader,std::array<std::unique_ptr<boundary::Evidence>,3>& out){
  cfgreflection::Observation<ChallengeReader> reflection(reader,Module);
  for(unsigned i=0;i<3;++i){cfgreflection::Field descriptor;out[i]=std::make_unique<boundary::Evidence>();
   if(!reflection.ReadField(funcs[i],DispatcherClass,descriptor)||!cfgmenu::capture::KnownFunction(reader,Module,descriptor,boundary::Names[i],*out[i]))return false;
  }return true;
 }
};
struct BoundaryWriter {
 ChallengeReader& r;unsigned writes=0,attempts=0,failAt=0;bool mutateThenFail=false,protectionFault=false;std::uintptr_t pendingAt=0;std::function<void(unsigned)> after;
 bool Pending()const{return pendingAt!=0;}void Forget(std::uintptr_t at){if(pendingAt==at)pendingAt=0;}
 bool Exchange(std::uintptr_t at,const std::array<unsigned char,2>& old,const std::array<unsigned char,2>& value){
  ++attempts;if(pendingAt&&pendingAt!=at)return false;pendingAt=0;std::array<unsigned char,2> actual{};if(!r.Read(at,actual)||actual!=old)return false;
  if(attempts==failAt&&!mutateThenFail)return false;
  r.Set(at,value);++writes;if(after)after(writes);if(attempts==failAt&&protectionFault)pendingAt=at;return attempts!=failAt;
 }
};
struct TransientBoundaryReader:ChallengeReader {
 std::uintptr_t failRead=0;unsigned failuresRemaining=0;
 template<class T>bool Read(std::uintptr_t at,T& out){
  if(at==failRead&&failuresRemaining){--failuresRemaining;return false;}
  return ChallengeReader::Read(at,out);
 }
};
int main(){unsigned checks=0,failures=0,peak=0;
 const auto check=[&](bool ok,const char* name){++checks;if(!ok){++failures;std::printf("FAIL %s\n",name);}};
 {cfgj29walls::Census names;
  check(!names.Observe(u"OrdinaryActor")&&!names.total,"unrelated classes excluded");
  check(names.Observe(u"BlockingVolume_C")&&names.Observe(u"BlockingVolume_C")&&names.distinct==1&&names.total==2&&names.entries[0].count==2,
   "duplicate wall classes counted without retained UObject identity");
  check(!names.Observe(u"BarrierActor",false)&&names.total==2,"exhausted scan budget drops optional wall count without failure");
  std::u16string longName=u"Boundary";longName+=std::u16string(80,u'Z');
  check(names.Observe(longName)&&names.distinct==2&&names.entries[1].name[63]=='\0',"wall name truncated to fixed 63 ASCII bytes");
 }
 // Task005: production Scan consumes already decoded class names; the
 // fixture exercises the integrated roster path under its real read budget.
 {BoundaryFixture f;constexpr std::uintptr_t WallClass=0x3810000,WallA=0x3820000,WallB=0x3830000;
  f.Object(WallClass,f.ClassType,f.WorldPackage);f.Label(WallClass,"BlockingVolume_C");
  f.Object(WallA,WallClass,f.Level);f.Object(WallB,WallClass,f.Level);
  f.r.Set(f.ActorArray+8,WallA);f.r.Set(f.ActorArray+16,WallB);
  f.r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,3,3});
  f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(f.nextSlot));
  f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
  ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;BoundaryWriter writer{r};boundary::Runtime runtime;
  boundary::Budget<ChallengeReader> start{r};check(runtime.Arm(start,writer,f.Menu(),true,0),"wall census arms stock Challenge scope");
  for(unsigned i=0;i<8&&!runtime.Ready();++i){boundary::Budget<ChallengeReader> next{r};runtime.Step(next,writer,42,100+50*i);
   check(next.reads<=next.cap,"wall census never exceeds original budget");}
  check(runtime.Ready(),"wall census preserves both donor boundary script edits");
  const auto& walls=runtime.WallReport();
  check(walls.total==(CFG_J29_CHALLENGE_BOUNDARY_FULL?2u:0u)&&walls.distinct==(CFG_J29_CHALLENGE_BOUNDARY_FULL?1u:0u),
   "flag controls read-only counts from the actual scan; OFF leaves zero report");
  check(!CFG_J29_CHALLENGE_BOUNDARY_FULL||walls.entries[0].count==2,
   "same-class walls aggregate without a second roster scan");
 }
 for(unsigned misses:{1u,2u}){BoundaryFixture f;TransientBoundaryReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<TransientBoundaryReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  check(runtime.Ready()&&writer.writes==2,"post-install verifier fixture installed");
  r.failRead=f.funcs[1]+cfgreflection::disk::FunctionFlags;r.failuresRemaining=misses;
  boundary::Budget<TransientBoundaryReader> verify{r};runtime.Step(verify,writer,42,150);
  check(runtime.VerifyRetries()==1&&runtime.VerifyFailure().stage==6&&runtime.VerifyFailure().record==1,
    "exact failed snapshot leaf retained through bounded same-callback recheck");
#if CFG_H294_BOUNDARY_VERIFY_DEFER
  // H2.94: a same-callback recheck that still fails keeps the unchanged, still-patched edit and re-verifies next callback.
  check(runtime.Ready()&&runtime.VerifyRecoveries()==(misses==1?1u:0u)&&(misses==1||runtime.verifyDeferrals==1),
    "one incomplete read recovers in-callback; a second keeps the proven edit for a bounded re-verify");
  if(misses==2){boundary::Budget<TransientBoundaryReader> again{r};runtime.Step(again,writer,42,200);}
  check(runtime.Ready()&&writer.writes==2&&runtime.Restored()==0,"recovered verification never repatches or restores");
#else
  check(runtime.Ready()==(misses==1)&&runtime.VerifyRecoveries()==(misses==1?1u:0u),
    "one incomplete read recovers only with complete fresh authority; persistent failure cancels");
  check(writer.writes==(misses==1?2u:4u)&&runtime.Restored()==(misses==1?0u:2u),
    "recovered verification never repatches; persistent failure conditionally restores");
#endif
  check(verify.reads<65536,"recheck and cleanup share original bounded reader");peak=(std::max)(peak,verify.reads);
 }
#if CFG_H294_BOUNDARY_VERIFY_DEFER
 {// H2.94: a verify failure that never clears still retires after the bounded window (no new writes meanwhile).
  BoundaryFixture f;TransientBoundaryReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<TransientBoundaryReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  // A read only the full Verify performs (function locals head); the fallback's own reads stay complete.
  r.failRead=f.funcs[1]+cfgreflection::disk::Children;r.failuresRemaining=1000000;bool kept=true;
  for(unsigned i=0;i<boundary::Runtime::VerifyDeferLimit;++i){boundary::Budget<TransientBoundaryReader> v{r};runtime.Step(v,writer,42,150+i);
   kept=kept&&runtime.Ready()&&writer.writes==2;}
  check(kept&&runtime.verifyDeferrals==boundary::Runtime::VerifyDeferLimit,"persistent verify failure keeps the edit only for the bounded window");
  boundary::Budget<TransientBoundaryReader> last{r};runtime.Step(last,writer,42,999);
  check(!runtime.Armed()&&!runtime.Ready()&&(runtime.failure==6||(runtime.failure==7&&runtime.CleanupPending())),"then it retires exactly as before");}
 for(unsigned meta=0;meta<2;++meta){// H2.96 (audit 2 P2): a changed executor or flags with an unchanged script is a real change: retire, no deferral.
  BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  if(meta==0)r.Set(f.funcs[1]+0xB0,f.funcs[1]+0x1234);else{std::uint32_t flags=0;r.Read(f.funcs[1]+cfgreflection::disk::FunctionFlags,flags);r.Set(f.funcs[1]+cfgreflection::disk::FunctionFlags,flags^0x1000u);}
  boundary::Budget<ChallengeReader> verify{r};runtime.Step(verify,writer,42,150);
  check(!runtime.Armed()&&!runtime.Ready()&&!runtime.verifyDeferrals,"metadata change with unchanged script retires at once (no deferral)");}
#endif
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> install{r};
  check(runtime.Arm(install,writer,f.Menu(),true,0),"transient-scope fixture arms");runtime.Step(install,writer,42,100);
  check(runtime.Ready(),"transient-scope fixture installed");
  boundary::Budget<ChallengeReader> shortRead{r};shortRead.cap=1;runtime.Step(shortRead,writer,42,150);
  check(runtime.Armed()&&runtime.Ready()&&writer.writes==2,"single incomplete scope observation cannot permanently remove protection or write");
  boundary::Budget<ChallengeReader> recovered{r};runtime.Step(recovered,writer,42,200);
  check(runtime.Ready()&&writer.writes==2,"complete fresh scope recovers without repatching");
  r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);
  boundary::Budget<ChallengeReader> departed{r};runtime.Step(departed,writer,42,250);
  check(!runtime.Armed()&&runtime.Restored()==2,"proved world departure still retires immediately");
 }
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  for(unsigned attempt=0;attempt<4;++attempt){boundary::Budget<ChallengeReader> incomplete{r};incomplete.cap=1;
   runtime.Step(incomplete,writer,42,150+attempt*50);
   check(runtime.Armed()==(attempt<3),"persistent incomplete scope has bounded grace");
   check(writer.writes==2,"incomplete scope performs no new writes");
  }
  boundary::Budget<ChallengeReader> cleanup{r};runtime.Step(cleanup,writer,42,400);
  check(!runtime.Armed()&&!runtime.CleanupPending()&&runtime.Restored()==2,"persistent failure restores originals when complete reads resume");
 }
 // Native actor effects can resize/relocate rosters; the engine may also assign
 // a first serial to an unchanged strong-linked object after installation.
 // None of these events changes this Challenge or its boundary dispatcher.
 for(unsigned change=0;change<6;++change){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  check(runtime.Ready()&&writer.writes==2,"continued-borrow fixture installed");
  if(change==0){r.Set(f.ActorArray+8,f.Npc);r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,2,2});}
  if(change==1){const std::uintptr_t moved=0x6800000;r.Set(moved,std::array<std::uintptr_t,2>{{f.Npc,f.Dispatcher}});
   r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{moved,2,2});r.Set(f.ActorArray,std::uintptr_t{});r.watch=f.ActorArray;}
  if(change==2){const std::uintptr_t moved=0x6900000;act::List old;r.Read(f.World+cfgradarworld::disk::WorldLevels,old);
   r.Set(moved,f.Level);r.Set(f.World+cfgradarworld::disk::WorldLevels,act::List{moved,1,1});r.Set(old.data,std::uintptr_t{});r.watch=old.data;}
  if(change>=3){const auto object=change==3?f.Dispatcher:change==4?f.DispatcherClass:f.Level;int index=0;
   r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(f.Registry+24ULL*static_cast<unsigned>(index)+16,int{7000});}
  boundary::Budget<ChallengeReader> next{r};runtime.Step(next,writer,42,150);peak=(std::max)(peak,next.reads);
  check(runtime.Armed()&&runtime.Ready()&&runtime.Restored()==0&&writer.writes==2,"legitimate roster/serial change must not restore active boundary scripts");
  boundary::Budget<ChallengeReader> stable{r};runtime.Step(stable,writer,42,200);
  check(runtime.Ready()&&writer.writes==2,"refreshed continued borrow remains stable on following callback");
  check(runtime.guard==0&&runtime.refreshes==1,"successful refresh is counted once with no lingering refusal");
  if(change==1||change==2)check(r.hits==0,"relocation never dereferences the old array storage");
  r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);
  boundary::Budget<ChallengeReader> cleanup{r};runtime.Step(cleanup,writer,42,250);
  check(!runtime.Armed()&&!runtime.CleanupPending()&&runtime.Restored()==2,"refreshed borrow still restores on actual world departure");
 }
 for(unsigned fault=0;fault<11;++fault){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> install{r};
  runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);unsigned expected=0;
  if(fault==0){r.Set(f.ActorArray,std::array<std::uintptr_t,2>{{f.Npc,0}});r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,2,2});expected=12;}
  if(fault==1){r.Set(f.ActorArray+8,f.Dispatcher);r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,2,2});expected=12;}
  if(fault>=2&&fault<=4){const auto object=fault==2?f.Dispatcher:fault==3?f.DispatcherClass:f.Level;int index=0;
   r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);const auto at=f.Registry+24ULL*static_cast<unsigned>(index)+16;
   r.Set(at,int{7000});boundary::Budget<ChallengeReader> upgrade{r};runtime.Step(upgrade,writer,42,125);
   check(runtime.Ready(),"first engine serial upgrade admitted before recycle test");r.Set(at,int{7001});expected=fault==2?3:fault==3?6:7;}
  if(fault==5){r.Set(f.Dispatcher+8,std::uint32_t{0x10});expected=4;}
  if(fault==6){r.Set(f.Module+d::ActorCtor,f.AiType);expected=5;}
  if(fault==7){const std::uintptr_t moved=0x6900000;r.Set(moved,f.Npc);r.Set(f.World+cfgradarworld::disk::WorldLevels,act::List{moved,1,1});expected=9;}
  if(fault==8){r.Set(f.scripts[1]+50,std::uint8_t{0x66});expected=14;}
  if(fault==9){r.Set(f.DispatcherClass+cfgspawn::ClassDefaultObject,f.DispatcherClass+0x9000);expected=6;}
  if(fault==10){r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,16001,16001});expected=11;}
  boundary::Budget<ChallengeReader> verify{r};runtime.Step(verify,writer,42,150);peak=(std::max)(peak,verify.reads);
  check(!runtime.Armed()&&!runtime.Ready()&&(runtime.failure==6||(runtime.failure==7&&runtime.CleanupPending())),"foreign/removed/recycled/script-changed borrower still retires");
  check(runtime.guard==expected,"continued-borrow refusal identifies the exact failed proof");
 }
 for(unsigned kind=0;kind<3;++kind){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  cfgchallengeescape::scope::Runtime owner;owner.Arm(f.Menu(),1,true,0);cfgchallengeescape::scope::Witness w;
  boundary::Budget<ChallengeReader> initial{r};owner.Observe(initial,42,100,w);boundary::Scan scan;
  scan.Step(initial,f.Module,w,[&]{return owner.Current(initial,42,w);});auto proof=scan.Proof();
  const auto object=kind==0?f.Dispatcher:kind==1?f.DispatcherClass:f.Level;int index=0;r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);
  const auto at=f.Registry+24ULL*static_cast<unsigned>(index)+16;r.Set(at,int{7000});
  r.watch=f.DispatcherClass+cfgspawn::ClassDefaultObject;r.hits=0;r.mutate=at;r.value=7001;
  unsigned guard=0,refreshes=0;boundary::Budget<ChallengeReader> changed{r};
  check(!boundary::RefreshBorrow(changed,f.Module,w,proof,guard,refreshes)&&guard==13,"serial mutation during final strong-link proof refused");
  check(!refreshes&&proof.slot.serial==0&&proof.owner.slot.serial==0&&proof.levelType.slot.serial==0,"failed double sample never latches candidate serials");
 }
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;BoundaryWriter writer{r};boundary::Runtime runtime;
  boundary::Budget<ChallengeReader> install{r};runtime.Arm(install,writer,f.Menu(),true,0);runtime.Step(install,writer,42,100);
  const std::uintptr_t moved=0x6800000;
  for(unsigned i=0;i<16000;++i)r.Set(moved+8ULL*i,i==15999?f.Dispatcher:std::uintptr_t{});
  r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{moved,16000,16000});
  boundary::Budget<ChallengeReader> verify{r};runtime.Step(verify,writer,42,150);peak=(std::max)(peak,verify.reads);
  check(runtime.Ready()&&runtime.refreshes==1&&writer.writes==2,"maximum16000-cell current roster relocation retains valid override");
  check(verify.reads<65536,"maximum-roster refresh retains dedicated callback read cap");
 }
 for(unsigned fault=0;fault<16;++fault){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;check(f.Capture(r,functions),"captured typed CFG fixture");r.count=0;
  if(fault==1)functions[1]->bytes[0]=0x4C;
  if(fault==2)functions[2]->bytes[0]=0x5F;
  if(fault==3)functions[0]->fields[0].flags=0x80;
  if(fault==4)functions[1]->fields[3].offset+=8;
  if(fault==5)functions[2]->fieldsCount=2;
  if(fault==6)functions[1]->destructorLink=0;
  if(fault==7)functions[0]->bytes[1]=0;
  if(fault==8)functions[1]->nativeExecutor+=1;
  if(fault==9)functions[1]->function.owner=f.ActorType;
  if(fault==10)functions[2]->bytes[160]=0x53;
  if(fault==11)r.Set(f.funcs[1]+cfgreflection::disk::FunctionFlags,std::uint32_t{0x0C820400});
  if(fault==12)r.Set(f.DispatcherClass+cfgreflection::disk::Children,std::uintptr_t{});
  if(fault==13){int index=0;r.Read(f.funcs[1]+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(f.Registry+24ULL*static_cast<unsigned>(index)+8,std::uint32_t{0x20000000});}
  BoundaryWriter writer{r};boundary::Budget<ChallengeReader> budget{r};boundary::Pair pair;
  if(fault==15)budget.cap=32767;
  const bool result=pair.Apply(budget,writer,f.Module,functions,[&]{return fault!=14;});peak=(std::max)(peak,budget.reads);
  check(result==(!fault),"exact CFG metadata and donor-style entry/return signature admission");
  if(fault){check(!writer.writes&&!pair.Owned(),"failed admission writes nothing");continue;}
  check(writer.writes==2&&pair.Ready(),"only outside and boundary restart edited");
  unsigned char inside=0;r.Read(f.scripts[0],inside);check(inside==0x22,"inside untouched");
  boundary::Budget<ChallengeReader> cleanup{r};check(pair.Restore(cleanup,writer)&&!pair.Owned()&&writer.writes==4,"both originals conditionally restored");
  peak=(std::max)(peak,cleanup.reads);
 }
 for(unsigned fault=1;fault<4;++fault){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};
  bool current=true;if(fault==1){writer.failAt=2;}if(fault==2){writer.after=[&](unsigned n){if(n==1)current=false;};}
  if(fault==3){writer.failAt=1;writer.mutateThenFail=true;}
  boundary::Budget<ChallengeReader> budget{r};boundary::Pair pair;
  check(!pair.Apply(budget,writer,f.Module,functions,[&]{return current;}),"partial write / changed scope / protection error refuses ready");
  check(!pair.Owned()&&pair.restored==1,"partial override rolled back without native calls");
 }
 for(unsigned fault=1;fault<6;++fault){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};
  boundary::Budget<ChallengeReader> budget{r};boundary::Pair pair;pair.Apply(budget,writer,f.Module,functions,[]{return true;});
  if(fault==1)r.Set(f.scripts[1]+50,std::uint8_t{0x66});
  if(fault==2)r.Set(f.scripts[1],std::array<unsigned char,2>{{0x12,0x34}});
  if(fault==3)r.Set(f.funcs[1]+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{f.scripts[0],922,922});
  if(fault==4){int index=0;r.Read(f.funcs[1]+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(f.Registry+24ULL*static_cast<unsigned>(index),f.Npc);}
  boundary::Budget<ChallengeReader> cleanup{r};if(fault==5)cleanup.cap=16383;
  const bool restored=pair.Restore(cleanup,writer);
  check(restored==(fault==4),"foreign script/header/refused-budget cleanup never overwrites; removed slot retires");
  check(writer.writes==(fault==5?2u:3u),"only matching owned edit restored");
  check(pair.Owned()==(fault!=4),"unresolved cleanup ownership retained");
 }
 for(unsigned fault=0;fault<11;++fault){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  if(fault==1)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==2)r.Set(f.Controller+0x578,f.PS);
  if(fault==3)r.Set(f.Dispatcher+0x110,std::uint8_t{1});
  if(fault==4)r.Set(f.Dispatcher+0x20,f.World);
  if(fault==5)r.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
  if(fault==6)r.Set(f.Dispatcher+8,std::uint32_t{0x10});
  if(fault==7){r.Set(f.ActorArray+8,f.Dispatcher);r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,2,2});}
  if(fault==8)r.Set(f.Module+d::ActorCtor,f.AiType);
  if(fault==9)r.Set(f.funcs[1]+cfgreflection::disk::Next,f.funcs[1]);
  BoundaryWriter writer{r};boundary::Runtime runtime;boundary::Budget<ChallengeReader> budget{r};
  check(runtime.Arm(budget,writer,f.Menu(),fault!=10,0),"explicit per-submission boundary choice arms independently");
  runtime.Step(budget,writer,42,100);peak=(std::max)(peak,budget.reads);
  check(runtime.Ready()==(!fault),"runtime requires current standalone stock scope, strong borrow and schemas");
  if(fault){check(!writer.writes,"invalid or Off scope cannot edit boundary scripts");continue;}
  check(writer.writes==2&&runtime.Applied()==2,"serial0 dispatcher/class/functions use no invented weak lease");
  r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);r.Set(f.Controller+cfgprobe::ControllerPawn,std::uintptr_t{});
  boundary::Budget<ChallengeReader> cleanup{r};runtime.Step(cleanup,writer,42,200);
  check(!runtime.Armed()&&!runtime.Ready()&&!runtime.CleanupPending()&&writer.writes==4,"world return restores owned metadata without old-world native calls");
  check(runtime.Arm(cleanup,writer,f.Menu(),false,300)&&!runtime.Armed(),"next Off submission does not inherit prior boundary flag");
 }
 check(peak<32768,"normal install retains reserved rollback budget");
 for(unsigned kind=0;kind<3;++kind){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};boundary::Pair pair;
  boundary::Budget<ChallengeReader> apply{r};check(pair.Apply(apply,writer,f.Module,functions,[]{return true;}),"unassigned metadata patched before engine serial assignment");
  int index=0;const auto object=kind==1?f.DispatcherClass:f.funcs[1];r.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);
  if(kind==2){int ownerIndex=0;r.Read(f.DispatcherClass+cfgprobe::lifetime::ObjectIndexOffset,ownerIndex);r.Set(f.Registry+24ULL*static_cast<unsigned>(ownerIndex)+16,int{6000});}
  r.Set(f.Registry+24ULL*static_cast<unsigned>(index)+16,int{5000});boundary::Budget<ChallengeReader> verify{r};
  check(pair.Verify(verify),"full unchanged owned borrow admits and latches one engine serial assignment");
  boundary::Budget<ChallengeReader> cleanup{r};check(pair.Restore(cleanup,writer)&&!pair.Owned()&&writer.writes==4,"cleanup after metadata serial assignment restores both scripts");
 }
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};boundary::Pair pair;
  boundary::Budget<ChallengeReader> apply{r};pair.Apply(apply,writer,f.Module,functions,[]{return true;});int index=0;
  r.Read(f.funcs[1]+cfgprobe::lifetime::ObjectIndexOffset,index);r.Set(f.Registry+24ULL*static_cast<unsigned>(index)+16,int{5000});
  boundary::Budget<ChallengeReader> verify{r};check(pair.Verify(verify),"first metadata serial latched");
  r.Set(f.Registry+24ULL*static_cast<unsigned>(index)+16,int{5001});boundary::Budget<ChallengeReader> cleanup{r};
  check(pair.Restore(cleanup,writer)&&!pair.Owned()&&writer.writes==3,"positive serial recycle retires old edit without touching recycled function");
 }
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};boundary::Pair pair;
  boundary::Budget<ChallengeReader> apply{r};pair.Apply(apply,writer,f.Module,functions,[]{return true;});
  int index=0;r.Read(f.funcs[1]+cfgprobe::lifetime::ObjectIndexOffset,index);const auto serialAt=f.Registry+24ULL*static_cast<unsigned>(index)+16;
  r.Set(serialAt,int{5000});r.watch=f.DispatcherClass+cfgspawn::ClassDefaultObject;r.hits=0;r.mutate=serialAt;r.value=5001;
  boundary::Budget<ChallengeReader> verify{r};check(!pair.Verify(verify),"serial mutation during final owner proof cannot be adopted");
  r.watch=0;boundary::Budget<ChallengeReader> cleanup{r};check(pair.Restore(cleanup,writer)&&!pair.Owned(),"later stable full metadata cleanup does not forget unresolved edit");
 }
 for(unsigned fault=0;fault<2;++fault){BoundaryFixture f;
  f.Label(f.ActorType,"Actor");std::array<std::uintptr_t,257> roster{};
  for(unsigned i=0;i<256;++i){roster[i]=0x5000000+0x1000ULL*i;f.Object(roster[i],f.ActorType,f.Level);}roster[256]=f.Dispatcher;
  f.r.Set(f.ActorArray,roster);f.r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,257,257});
  f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(f.nextSlot));
  f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
  ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;BoundaryWriter writer{r};boundary::Runtime runtime;
  boundary::Budget<ChallengeReader> first{r};runtime.Arm(first,writer,f.Menu(),true,0);runtime.Step(first,writer,42,100);
  check(!runtime.Ready()&&!writer.writes&&runtime.Cells()==128,"scan yields after128 cells without editing");
  if(fault)r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,256,257});
  r.count=0;boundary::Budget<ChallengeReader> second{r};runtime.Step(second,writer,42,150);
  check(!runtime.Ready()&&!writer.writes,"second batch cannot edit before full roster proof");
  if(fault){check(runtime.attempts==1&&runtime.Armed(),"streaming failure preserves remaining donor retry allowance");}
  r.count=0;boundary::Budget<ChallengeReader> third{r};runtime.Step(third,writer,42,200);
  check(runtime.Ready()==(!fault)&&writer.writes==(fault?0u:2u),"later-batch dispatcher works and changed roster revokes scan");
  if(fault){check(runtime.attempts==1&&runtime.Armed(),"subsecond pumps cannot exhaust ten donor retries");
   for(unsigned i=0;i<8;++i){r.count=0;boundary::Budget<ChallengeReader> wait{r};runtime.Step(wait,writer,42,250+50*i);}
   check(runtime.attempts==1&&runtime.Armed()&&!writer.writes,"donor one-second retry pacing retained during actor churn");}
  peak=(std::max)(peak,third.reads);
 }
 {BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;BoundaryWriter writer{r};boundary::Runtime runtime;
  boundary::Budget<ChallengeReader> apply{r};runtime.Arm(apply,writer,f.Menu(),true,0);runtime.Step(apply,writer,42,100);
  f.Phase("WaitingPostMatch");r.Set(f.Mode+0x3F8,cfgnames::Words{static_cast<int>(f.id-1),0});
  std::uintptr_t entry=0;f.r.Read(f.Chunk+8ULL*(f.id-1),entry);std::uint32_t header=0;std::array<char,128> text{};
  f.r.Read(entry,header);f.r.Read(entry+cfgnames::disk::EntryText,text);r.Set(entry,header);r.Set(entry+cfgnames::disk::EntryText,text);
  r.Set(f.Chunk+8ULL*(f.id-1),entry);r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
  boundary::Budget<ChallengeReader> cleanup{r};runtime.Step(cleanup,writer,42,200);
  check(!runtime.Armed()&&!runtime.CleanupPending()&&writer.writes==4,"terminal phase restores without travel or generic Reset suppression");
 }
 {BoundaryFixture f;const auto dummy=std::uintptr_t{0x6000000};f.r.Set(f.funcs[2]+cfgreflection::disk::Next,dummy);
  for(unsigned i=0;i<509;++i)f.r.Set(dummy+0x1000ULL*i+cfgreflection::disk::Next,i==508?std::uintptr_t{}:dummy+0x1000ULL*(i+1));
  ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;std::array<std::unique_ptr<boundary::Evidence>,3> functions;
  f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};writer.failAt=2;boundary::Pair pair;boundary::Budget<ChallengeReader> apply{r};
  check(!pair.Apply(apply,writer,f.Module,functions,[]{return true;})&&!pair.Ready(),"512-child class second-write failure cannot become ready");
  boundary::Budget<ChallengeReader> cleanup{r};check(pair.Restore(cleanup,writer)&&!pair.Owned()&&pair.restored==1,"max-chain rollback reserve and fresh cleanup restore partial edit");
  peak=(std::max)(peak,apply.reads);
 }
 for(unsigned kind=0;kind<2;++kind){BoundaryFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=131072;
  std::array<std::unique_ptr<boundary::Evidence>,3> functions;f.Capture(r,functions);r.count=0;BoundaryWriter writer{r};writer.failAt=kind?2:1;
  writer.mutateThenFail=writer.protectionFault=true;boundary::Pair pair;boundary::Budget<ChallengeReader> apply{r};
  check(!pair.Apply(apply,writer,f.Module,functions,[]{return true;})&&!pair.Ready(),"pending page protection failure cannot become ready");
  boundary::Budget<ChallengeReader> cleanup{r};check(pair.Restore(cleanup,writer)&&!pair.Owned()&&!writer.Pending(),"address-bound protection recovery restores both records with fresh cleanup pass");
 }
 {auto* memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
  check(memory!=nullptr,"owned heap writer fixture allocated");
  if(memory){const auto address=reinterpret_cast<std::uintptr_t>(memory);memory[0]=0x22;memory[1]=0x97;boundary::WindowsWriter writer;
   const std::array<unsigned char,2> old{{0x22,0x97}};
   check(writer.Exchange(address,old,boundary::Return)&&!writer.Pending(),"atomic owned private heap exchange");
   check(!writer.Exchange(address,old,boundary::Return),"compare failure does not overwrite");
   DWORD ignored=0;VirtualProtect(memory,4096,PAGE_READONLY,&ignored);
   check(writer.Exchange(address,boundary::Return,old),"readonly private script restored via temporary protection");
   MEMORY_BASIC_INFORMATION info{};VirtualQuery(memory,&info,sizeof(info));check(info.Protect==PAGE_READONLY,"original page protection restored");
   check(!writer.Exchange(address+1,old,boundary::Return),"unaligned prefix refused");
   VirtualProtect(memory,4096,PAGE_READWRITE|PAGE_GUARD,&ignored);check(!writer.Exchange(address,old,boundary::Return),"guard pages refused without clearing guard");
   VirtualProtect(memory,4096,PAGE_EXECUTE_READWRITE,&ignored);check(!writer.Exchange(address,old,boundary::Return),"executable memory refused");
   VirtualFree(memory,0,MEM_RELEASE);
  }
 }
 std::printf("CFG challenge boundary scripts/runtime: %u checks, %u failures, peak%u/65536; owned synthetic memory only, physical acceptance pending.\n",checks,failures,peak);
 return failures?1:0;
}
