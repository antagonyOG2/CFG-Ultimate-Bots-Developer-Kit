#include "../RuntimeProbe/SpectatorRuntime.hpp"
#define main SpectatorOwnedFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
using namespace cfgspectator;
struct TerminalAccess {
 Snapshot snapshot{};unsigned selections=0;
 TerminalAccess(){snapshot.world={1,1};snapshot.controller={2,1};snapshot.spectator={3,1};
  snapshot.offline=snapshot.localOwned=snapshot.gameThread=snapshot.inProgress=snapshot.spectatorPawn=true;
  snapshot.count=2;snapshot.counselors[0]={{10,1},{11,1},true};snapshot.counselors[1]={{20,1},{21,1},true};
  snapshot.jason={{30,1},{31,1},true};snapshot.observedState=snapshot.counselors[0].state;}
 bool Observe(Snapshot& s){s=snapshot;return true;}
 bool ReadPose(const Snapshot&,const Target&,Vector& p,float& yaw,Rotation& look,bool& hasLook){p={100,100,100};yaw=0;look={};hasLook=false;return true;}
 bool Publish(const Snapshot&,const Target& target,const CameraFrame&,bool selecting){if(selecting)++selections;snapshot.observedState=target.state;return true;}
 bool RestoreCounselor(const Snapshot&,const Target& target){snapshot.observedState=target.state;return true;}
 void Abandon(){}
};
int main(){
 Fixture f;const auto stateType=f.Object(),state=f.Object(stateType),controller=f.Object();
 f.Put(f.module+cfgescape::disk::SCPlayerStateReflection,stateType);f.Put(f.module+cfgescape::disk::SCPlayerStateConstructor,stateType);
 f.Put(controller+cfgescape::disk::PlayerState,state);f.Put(state+cfgescape::disk::bEscaped,std::uint8_t{1});f.Put(state+cfgescape::disk::bDead,std::uint8_t{0});
 cfgescape::State result;f.reads=0;CHECK(cfgspectator::LocalTerminal(f,f.objects,f.module,controller,result)&&result==cfgescape::State::Escaped);const auto reads=f.reads;
 // The old counselor pawn is destroyed and unreadable. Only native attached
 // PlayerState is consulted; zero serial is scoped readonly, not retained.
 for(unsigned i=0;i<0x2000;++i)f.memory.erase(f.bot+i);
 f.reads=0;CHECK(cfgspectator::LocalTerminal(f,f.objects,f.module,controller,result)&&cfgescape::Terminal(result));
 TerminalAccess access;access.snapshot.humanTerminal=cfgescape::Terminal(result);Runtime runtime;runtime.Step(access,100,true);CHECK(runtime.Following()&&access.selections==1);
 for(unsigned fault=0;fault<6;++fault){auto bad=f;bad.reads=0;
  switch(fault){case 0:bad.Put(state+cfgescape::disk::bEscaped,std::uint8_t{2});break;
   case 1:bad.Put(state+cfgescape::disk::bDead,std::uint8_t{2});break;
   case 2:bad.Put(controller+cfgescape::disk::PlayerState,std::uintptr_t{});break;
   case 3:bad.Put(state+cfgprobe::ObjectClass,bad.pawnClass);break;
   case 4:bad.trigger=controller+cfgescape::disk::PlayerState;bad.triggerAt=2;bad.mutation=bad.trigger;bad.newPointer=bad.pawn;break;
   case 5:bad.cap=3;break;}
  CHECK(!cfgspectator::LocalTerminal(bad,bad.objects,bad.module,controller,result));CHECK(result==cfgescape::State::Unknown);
 }
 f.Put(state+cfgescape::disk::bEscaped,std::uint8_t{0});f.Put(state+cfgescape::disk::bDead,std::uint8_t{0});f.reads=0;
 CHECK(cfgspectator::LocalTerminal(f,f.objects,f.module,controller,result)&&result==cfgescape::State::Living);
 access.snapshot.humanTerminal=false;runtime.Step(access,200,true);CHECK(!runtime.Following());
 f.Put(state+cfgescape::disk::bDead,std::uint8_t{1});f.reads=0;CHECK(cfgspectator::LocalTerminal(f,f.objects,f.module,controller,result)&&result==cfgescape::State::Dead);
 // CFG can enter its owned native spectator pawn before the attached state
 // receives bDead. Require the original current human's actual terminal health,
 // exact same state and a positive owned spectator; never steal living Tommy.
 {
  auto late=f;late.reads=0;late.cap=1536;
  const auto human=late.Object(late.pawnClass),spectator=late.Object();
  int hi=0,si=0;CHECK(late.Read(human+12,hi)&&late.Read(spectator+12,si));
  late.Put(late.objects.data+24*static_cast<std::uintptr_t>(hi)+16,int{51});
  late.Put(late.objects.data+24*static_cast<std::uintptr_t>(si)+16,int{52});
  late.Put(human+cfgescape::disk::PlayerState,state);late.Put(human+cfgescape::disk::Controller,std::uintptr_t{});
  late.Put(human+cfgescape::disk::bHasEscaped,std::uint8_t{});late.Put(human+cfghunter::disk::Health,0.f);
  late.Put(spectator+cfgescape::disk::Controller,controller);late.Put(controller+cfgescape::disk::Pawn,spectator);
  late.Put(state+cfgescape::disk::bDead,std::uint8_t{});late.Put(state+cfgescape::disk::bEscaped,std::uint8_t{});
  cfgprobe::lifetime::SlotObservation h{},s{};CHECK(cfgspawn::ObjectSlot(late,late.objects,human,h)&&cfgspawn::ObjectSlot(late,late.objects,spectator,s));
  late.reads=0;CHECK(TerminalSpectatorOwner(late,late.objects,late.module,controller,h,late.pawnClass,s,result)&&result==cfgescape::State::Dead);
  for(unsigned fault=0;fault<5;++fault){auto bad=late;bad.reads=0;
   switch(fault){case 0:bad.Put(human+cfghunter::disk::Health,100.f);break;
    case 1:bad.Put(spectator+cfgescape::disk::Controller,bad.bot);break;
    case 2:bad.Put(human+cfgescape::disk::PlayerState,bad.bot);break;
    case 3:bad.Put(bad.objects.data+24*static_cast<std::uintptr_t>(hi)+16,int{53});break;
    case 4:bad.Put(controller+cfgescape::disk::Pawn,bad.bot);break;}
   CHECK(!TerminalSpectatorOwner(bad,bad.objects,bad.module,controller,h,bad.pawnClass,s,result));
  }
 }
 const auto killer=f.Object();bool living=false;f.Put(killer+cfghunter::disk::Health,0.f);f.Put(killer+cfgescape::disk::DyingFlags,std::uint8_t{0});f.Put(state+cfgescape::disk::bDead,std::uint8_t{0});f.reads=0;
 CHECK(cfgspectator::KillerLiving(f,killer,state,living)&&living);
 f.Put(killer+cfgescape::disk::DyingFlags,std::uint8_t{1});CHECK(cfgspectator::KillerLiving(f,killer,state,living)&&!living);
 f.Put(killer+cfgescape::disk::DyingFlags,std::uint8_t{0});f.Put(state+cfgescape::disk::bDead,std::uint8_t{1});CHECK(cfgspectator::KillerLiving(f,killer,state,living)&&!living);
 f.Put(killer+cfghunter::disk::Health,std::numeric_limits<float>::quiet_NaN());CHECK(!cfgspectator::KillerLiving(f,killer,state,living));
 const auto ownerPawn=f.Object(),ownerController=f.Object();
 int pawnIndex=0,controllerIndex=0;CHECK(f.Read(ownerPawn+0xC,pawnIndex)&&f.Read(ownerController+0xC,controllerIndex));
 f.Put(f.objects.data+24*pawnIndex+16,11);f.Put(f.objects.data+24*controllerIndex+16,12);
 f.Put(ownerPawn+cfgescape::disk::Controller,ownerController);f.Put(ownerController+cfgescape::disk::Pawn,ownerPawn);
 f.Put(ownerPawn+cfgescape::disk::PlayerState,state);f.Put(ownerController+cfgescape::disk::PlayerState,state);
 Identity owned;f.reads=0;CHECK(OwnedPlayerState(f,f.objects,ownerPawn,ownerController,state,stateType,owned)&&owned.strongOwnedState&&owned.serial==0);
 for(unsigned fault=0;fault<5;++fault){auto bad=f;bad.reads=0;
  switch(fault){case 0:bad.Put(ownerPawn+cfgescape::disk::Controller,controller);break;
   case 1:bad.Put(ownerController+cfgescape::disk::Pawn,killer);break;
   case 2:bad.Put(ownerController+cfgescape::disk::PlayerState,killer);break;
   case 3:bad.Put(f.objects.data+24*pawnIndex+16,0);break;
   case 4:bad.Put(state+cfgprobe::ObjectClass,f.pawnClass);break;}
  CHECK(!OwnedPlayerState(bad,bad.objects,ownerPawn,ownerController,state,stateType,owned)&&!owned);
 }
 {TerminalAccess a;a.snapshot.humanTerminal=true;a.snapshot.jason.state={31,0,true};a.snapshot.counselors[0].state={11,0,true};a.snapshot.observedState=a.snapshot.counselors[0].state;
  Runtime r;r.Step(a,100,true);CHECK(r.Following()&&a.selections==1);r.Step(a,120,false);CHECK(r.Following());
  a.snapshot.world={1,0,true};r.Step(a,140,false);CHECK(!r.Following());}
 {TerminalAccess a;a.snapshot.humanTerminal=true;a.snapshot.jason.state={31,0,false};Runtime r;r.Step(a,100,true);CHECK(!r.Following());}
 std::printf("Spectator terminal native-state fixtures: %d failures, %u terminal reads; destroyed escaped pawn, native respawn/death, zeroHP liveJason, owned serial0 state only. No game access.\n",failures,reads);return failures?1:0;
}
