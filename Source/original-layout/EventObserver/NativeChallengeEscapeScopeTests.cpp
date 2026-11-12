#define CFG_CHALLENGE_FIXTURE_ONLY
#include "NativeChallengeOptionsTests.cpp"
#undef CFG_CHALLENGE_FIXTURE_ONLY
#include "NativeChallengeEscapeScope.hpp"
namespace scope=cfgchallengeescape::scope;
struct ScopeFixture:ChallengeFixture {
 ScopeFixture(){
  r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallenges,SCState);
  r.Set(Module+cfgchallengeescape::disk::SCGameState_SPChallengesCtor,SCState);
  r.Set(Module+cfgchallengeescape::disk::PlayerController,ControllerType);
  r.Set(Module+cfgchallengeescape::disk::PlayerControllerCtor,ControllerType);
  Add(ControllerType,"NetConnection","ObjectProperty",0x578,1,8);r.Set(Controller+0x578,std::uintptr_t{});
  r.Set(Dossier+cfgprobe::lifetime::ObjectIndexOffset,int{7});r.Set(Registry+24*7,Dossier);
  r.Set(Registry+24*7+16,int{0});r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,int{8});
  for(unsigned i=0;i<8;++i){r.Set(Registry+24*i+8,std::uint32_t{});r.Set(Registry+24*i+12,std::uint32_t{});}
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
};
#ifndef CFG_ESCAPE_SCOPE_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0,peak=0;
 const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 for(unsigned fault=0;fault<21;++fault){ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;
  if(fault==1)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==2)r.Set(f.Controller+0x578,f.PS);
  if(fault==3)r.Set(f.Module+cfgchallengeescape::disk::SCGameMode_SPChallenges,f.Killer);
  if(fault==4)r.Set(f.Module+cfgchallengeescape::disk::SCGameState_SPChallengesCtor,f.ModeType);
  if(fault==5)r.Set(f.World+cfgprobe::WorldGameState,f.PS);
  if(fault==6)r.Set(f.Registry+24*6+16,int{0});
  if(fault==7)r.Set(f.Registry+24*6+8,std::uint32_t{0x10000000});
  if(fault==8)r.Set(f.Controller+cfgprobe::ControllerPlayer,f.PS);
  if(fault==9)r.Set(f.Pawn+0x3A0,f.Player);
  if(fault==10)r.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
  if(fault==11)r.Set(f.Dossier+0x58,std::uintptr_t{});
  if(fault==12)r.Set(f.Dossier+cfgprobe::lifetime::ObjectIndexOffset,int{6});
  if(fault==13)r.Set(f.Registry+24*7+8,std::uint32_t{0x20000000});
  if(fault==14)r.Set(f.Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{});
  if(fault==15)r.Set(f.Pawn+0x110,std::uint8_t{1});
  if(fault==16)r.Set(f.Module+cfgchallengeescape::disk::SCGameMode_SPChallengesCtor,f.SCState);
  if(fault==17)r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);
  if(fault==18){f.Phase("WaitingPostMatch");static_cast<StageReader&>(r)=f.r;}
  if(fault==19){f.Label(f.ChallengePackage,"/Game/Blueprints/MapRegistry/Challenges/WrongMission");
   f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});static_cast<StageReader&>(r)=f.r;}
  scope::Runtime runtime;scope::Witness witness;
  check(runtime.Arm(f.Menu(),1,true,0),"explicit positive generation accepted");
  const bool accepted=runtime.Observe(r,fault==20?43u:42u,100,witness);peak=(std::max)(peak,r.count);
  check(accepted==(!fault),"exact standalone scope only, no partial witness");
  if(fault){check(!witness.generation&&!witness.World(),"refusal leaves empty authority");continue;}
  r.count=0;check(runtime.Current(r,42,witness),"fresh full witness validates positive owners and borrowed zero-serial dossier");
  peak=(std::max)(peak,r.count);r.Set(f.World+cfgprobe::WorldGameState,f.PS);r.count=0;
  check(!runtime.Current(r,42,witness),"world state edge rechecked immediately before actions");
 }
 {ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;scope::Runtime runtime;scope::Witness w;
  check(runtime.Arm(f.Menu(),1,false,0)&&!runtime.Observe(r,42,100,w)&&!r.count,"Off incurs no game reads or effects");
  check(!runtime.Arm(f.Menu(),1,true,0)&&!runtime.Arm(f.Menu(),0,true,0),"same or zero submission cannot arm");
  check(runtime.Arm(f.Menu(),2,true,100)&&runtime.Observe(r,42,200,w),"fresh larger explicit submission arms");
  r.count=0;r.Set(f.Controller+0x578,f.Mode);check(!runtime.Current(r,42,w),"network appearing after bind revokes current authority");
  runtime.Retire();r.count=0;check(!runtime.Current(r,42,w)&&!r.count,"retired scope performs no old reads");
 }
 {ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;scope::Runtime runtime;scope::Witness w;
  runtime.Arm(f.Menu(),1,true,0);check(!runtime.Observe(r,42,90000,w)&&!r.count&&!runtime.Armed(),"unbound travel deadline retires without late effects");
 }
 {ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;scope::Runtime runtime;scope::Witness w;
  runtime.Arm(f.Menu(),1,true,0);runtime.Observe(r,42,100,w);r.count=0;
  r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);
  check(!runtime.Observe(r,42,200,w)&&!runtime.Armed()&&runtime.retirements==1&&r.count==1,"menu/world return retires before old actor reads");
 }
 {ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;scope::Runtime runtime;scope::Witness w;
  runtime.Arm(f.Menu(),1,true,0);runtime.Observe(r,42,100,w);r.count=0;
  r.Set(f.Registry+24*6+16,int{200});check(!runtime.Observe(r,42,200,w)&&!runtime.Armed(),"same address with recycled positive owner serial permanently retires submission");
 }
 {ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;scope::Runtime runtime;scope::Witness w;
  runtime.Arm(f.Menu(),1,true,0);runtime.Observe(r,42,100,w);
  f.Label(f.Challenge,"ChallengeC02_PackanackParty_C");f.Label(f.ChallengePackage,"/Game/Blueprints/MapRegistry/Challenges/ChallengeC02_PackanackParty");
  f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});static_cast<StageReader&>(r)=f.r;r.count=0;
  check(!runtime.Observe(r,42,200,w)&&!runtime.Armed(),"different stock mission cannot inherit earlier submitted settings");
 }
 for(unsigned fault=0;fault<11;++fault){ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;
  scope::Runtime runtime;scope::Witness w;runtime.Arm(f.Menu(),1,true,0);check(runtime.Observe(r,42,100,w),"grouped identity fixture scope");
  const auto slot=w.owners[3];const auto item=w.registry.data+24ULL*static_cast<unsigned>(slot.index);r.count=0;
  if(fault==1)r.Set(slot.object+cfgprobe::lifetime::ObjectIndexOffset,int{-1});
  if(fault==2)r.Set(slot.object+cfgprobe::lifetime::ObjectIndexOffset,w.registry.count);
  if(fault==3)r.Set(item,f.World);
  if(fault==4)r.Set(item+16,int{0});
  if(fault==5)r.Set(item+8,std::uint32_t{0x10000000});
  if(fault==6)r.bytes.erase(item+12);
  if(fault==7)r.bytes.erase(item+19);
  if(fault==8)r.cap=1;
  if(fault==9)r.Set(item+12,std::uint32_t{0xFFFFFFFF});
  if(fault==10)r.Set(item+16,int{-1});
  cfgprobe::lifetime::SlotObservation observed;
  check(scope::ReadIdentity(r,w.registry,slot.object,observed)==(fault==0||fault==9),
   "fresh grouped slot rejects bad index, object, serial, flags, partial data and cap; cluster bits grant no authority");
  if(!fault)check(observed==slot&&r.count==2,"current exact identity and registry flags use two reads");
 }
 check(peak<8192,"scope observations remain bounded under callback reader cap");
 std::printf("Challenge escape exact native scope: %u checks, %u failures, peak%u/8192; owned fixtures only, no hook integration.\n",checks,failures,peak);
 return failures?1:0;
}
#endif
