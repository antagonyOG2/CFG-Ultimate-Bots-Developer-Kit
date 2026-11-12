#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define main OfflineOwnerFixtureMain
#include "NativeOfflineOwnerObservationTests.cpp"
#undef main
#include "NativeHumanJasonMenu.hpp"
struct OptionReader {StageReader& r;unsigned reads=0;template<class T>bool Read(std::uintptr_t at,T& out){return ++reads<=1536&&r.Read(at,out);}};
struct OptionWriter {StageReader& r;unsigned writes=0;bool fail=false;
 template<class T>bool Write(std::uintptr_t at,const T& expected,const T& desired){T actual{};if(fail||!r.Read(at,actual)||actual!=expected)return false;r.Set(at,desired);++writes;return true;}
};
#ifndef CFG_HUMAN_OPTIONS_TEST_MAIN
#define CFG_HUMAN_OPTIONS_TEST_MAIN main
#endif
int CFG_HUMAN_OPTIONS_TEST_MAIN(){unsigned failures=0,peak=0;auto check=[&](bool okay){if(!okay)++failures;};
 const auto specs=cfgmenu::humanjason::Specs();check(specs[0].Valid()&&specs[1].Valid());
 for(unsigned fault=0;fault<8;++fault){Fixture f;f.Birth();OptionReader r{f.r};OptionWriter writer{f.r};
  constexpr std::uintptr_t state=0x700000,movement=0x710000,movementType=0x720000;
  f.r.Set(f.Selected+cfgprobe::ClassSuper,f.Killer);f.r.Set(f.World+cfgprobe::WorldGameState,state);
  f.r.Set(state+cfgprimary::CurrentKiller,f.Pawn);f.r.Set(state+cfgprimary::CurrentKillerState,f.PS);
  f.r.Set(f.Pawn+cfgmenu::humanjason::Knives,3);f.r.Set(f.Pawn+cfgmenu::humanjason::Movement,movement);
  f.r.Set(f.Module+cfgmenu::humanjason::MovementClass,movementType);f.r.Set(f.Module+cfgmenu::humanjason::MovementClass+8,movementType);
  f.r.Set(movement+cfgprobe::ObjectClass,movementType);f.r.Set(movement+0x20,f.Pawn);f.r.Set(movement+12,6);
  f.r.Set(movementType+cfgprobe::ClassSuper,std::uintptr_t{0});
  f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,7);f.r.Set(f.Registry+24*6,movement);f.r.Set(f.Registry+24*6+16,106);f.r.Set(f.Registry+24*6+8,std::uint32_t{});
  f.r.Set(movement+cfgmenu::humanjason::Speeds,std::array<float,2>{{600,400}});
  cfgmenu::active::Context menu;menu.module=f.Module;menu.engine=f.Engine;menu.viewport=f.Viewport;menu.player=f.Player;menu.gameInstance=f.Instance;menu.world=f.source.frontendWorld;menu.thread=42;
  f.r.Set(f.Pawn+0x129C,std::array<float,5>{{20,40,60,80,100}});
  cfgmenu::humanjason::Runtime runtime;check(runtime.Arm(menu,{true,true,true},0));
  if(fault==1)f.r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==2)f.r.Set(f.Pawn+0x3A0,f.Player);
  if(fault==3)f.r.Set(state+cfgprimary::CurrentKiller,f.PS);
  if(fault==4)f.r.Set(f.Selected+cfgprobe::ClassSuper,f.Counselor);
  if(fault==5)writer.fail=true;
  if(fault==6)f.r.Set(movement+0x20,f.Controller);
  if(fault==7)f.r.Set(f.Registry+24*6+16,int{0});
  runtime.Step(r,f.Pawn,42,100,writer);peak=(std::max)(peak,r.reads);
  if(!fault||fault==7){check(writer.writes==1&&runtime.report.movementWrites==1);r.reads=0;runtime.Step(r,f.Pawn,42,200,writer);peak=(std::max)(peak,r.reads);
   check(writer.writes==2&&runtime.report.rechargeWrites==1);r.reads=0;runtime.Step(r,f.Pawn,42,300,writer);peak=(std::max)(peak,r.reads);
   check(writer.writes==3&&runtime.report.knivesWrites==1);r.reads=0;runtime.Step(r,f.Pawn,42,400,writer);check(writer.writes==3);
   std::array<float,5> recharge{};f.r.Read(f.Pawn+0x129C,recharge);check(recharge==std::array<float,5>{{10,20,30,40,50}});
   std::array<float,2> values{};f.r.Read(movement+cfgmenu::humanjason::Speeds,values);check(values==std::array<float,2>{{1200,800}});
   f.r.Set(f.Viewport+cfgprobe::ViewportWorld,f.source.frontendWorld);r.reads=0;runtime.Step(r,f.Pawn,42,500,writer);
   check(writer.writes==3);
   auto gameMenu=menu;gameMenu.world=f.World;
   check(!runtime.ResetForVerifiedMenu(gameMenu));
   auto foreignMenu=menu;foreignMenu.player=f.PS;
   check(!runtime.ResetForVerifiedMenu(foreignMenu));
   check(runtime.ResetForVerifiedMenu(menu)&&!runtime.Armed()&&!runtime.report.bound&&writer.writes==3);
   check(runtime.Arm(menu,{true,true,true},600)&&runtime.report.movementWrites==0&&runtime.report.rechargeWrites==0&&runtime.report.knivesWrites==0);
  }else {check(writer.writes==0);r.reads=0;runtime.Step(r,f.Pawn,42,90000,writer);
   check(runtime.report.retired&&runtime.report.failure==(fault==5?6u:1u)&&r.reads==0&&writer.writes==0);}
 }
 {Fixture f;f.Birth();OptionReader r{f.r};OptionWriter writer{f.r};cfgmenu::active::Context menu;
  menu.module=f.Module;menu.engine=f.Engine;menu.viewport=f.Viewport;menu.player=f.Player;menu.gameInstance=f.Instance;menu.world=f.source.frontendWorld;menu.thread=42;
  cfgmenu::humanjason::Runtime quiet;check(quiet.Arm(menu,{},0)&&!quiet.Armed());check(!quiet.Callback(r,f.Pawn,100)&&r.reads==0);
  f.r.Set(f.Module+cfgofflinesettings::disk::SkillConsumer,cfgofflinesettings::disk::SkillConsumerCode);f.r.Set(f.Mode+cfgofflinesettings::disk::CounselorSkill,std::uint8_t{0});
  cfgmenu::humanjason::Runtime skill;check(skill.Arm(menu,{false,false,false,0,1},0));skill.BeforeIntro(r,f.Mode,42,writer);
  check(skill.report.skillState==3&&writer.writes==1);const auto before=r.reads;skill.BeforeIntro(r,f.Mode,42,writer);check(writer.writes==1&&r.reads==before);
 }
 { // Compose the NEW native Jason provider with the existing native timer adapter.
   // Generic timer-only tests do not cover the provider's repeated owner proof cost.
   Fixture f;f.Birth();OptionReader r{f.r};OptionWriter writer{f.r};
   namespace d=cfgofflinesettings::disk;
   constexpr std::uintptr_t state=0x700000,stateType=0x710000,scState=0x720000,scMode=0x730000,base=0x740000;
   f.r.Set(f.Selected+cfgprobe::ClassSuper,f.Killer);
   f.r.Set(f.World+cfgprobe::WorldGameState,state);f.r.Set(f.Mode+0x3C0,state);
   f.r.Set(state+cfgprimary::CurrentKiller,f.Pawn);f.r.Set(state+cfgprimary::CurrentKillerState,f.PS);
   f.r.Set(state+cfgprobe::ObjectClass,stateType);f.r.Set(stateType+cfgprobe::ClassSuper,scState);f.r.Set(scState+cfgprobe::ClassSuper,std::uintptr_t{});
   f.r.Set(stateType+0xF8,std::uintptr_t{0x750000});
   f.r.Set(f.ModeType+cfgprobe::ClassSuper,scMode);f.r.Set(scMode+cfgprobe::ClassSuper,base);f.r.Set(base+cfgprobe::ClassSuper,std::uintptr_t{});
   for(const auto pair:std::array<std::array<std::uintptr_t,2>,4>{{{{d::SCGameModeClass,scMode}},{{d::GameModeClass,base}},{{d::SCGameStateClass,scState}},{{cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass,stateType}}}}){
     f.r.Set(f.Module+pair[0],pair[1]);f.r.Set(f.Module+pair[0]+8,pair[1]);}
   f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{7});f.r.Set(state+12,int{6});f.r.Set(f.Registry+24*6,state);f.r.Set(f.Registry+24*6+16,int{106});
   f.r.Set(f.Module+d::TimerCopy,d::TimerCopyCode);f.r.Set(f.Module+d::TimerDecrement,d::TimerDecrementCode);f.r.Set(f.Module+d::ElapsedIncrement,d::ElapsedIncrementCode);
   f.Name(4,"InProgress");f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{5,1}});
   f.r.Set(f.Mode+d::MatchState,cfgnames::Words{4,0});f.r.Set(f.Mode+d::RoundTime,int{1200});f.r.Set(state+d::RemainingTime,int{1189});f.r.Set(state+d::ElapsedInProgressTime,int{11});
   cfgmenu::active::Context menu;menu.module=f.Module;menu.engine=f.Engine;menu.viewport=f.Viewport;menu.player=f.Player;menu.gameInstance=f.Instance;menu.world=f.source.frontendWorld;menu.thread=42;
   cfgmenu::humanjason::Runtime duration;check(duration.Arm(menu,{false,false,false,3600,3},0));duration.Step(r,f.Pawn,42,100,writer);
   peak=(std::max)(peak,r.reads);int round=0,left=0;f.r.Read(f.Mode+d::RoundTime,round);f.r.Read(state+d::RemainingTime,left);
   std::printf("Native Jason composed timer reads=%u state=%u failure=%u writes=%u\n",r.reads,duration.report.durationState,duration.report.failure,writer.writes);
   check(r.reads<1250&&duration.report.durationState==2&&writer.writes==1&&round==3600&&left==1189);
   auto timedOut=duration;r.reads=0;timedOut.Step(r,f.Pawn,42,90000,writer);
   check(timedOut.report.retired&&timedOut.report.durationState==6&&timedOut.report.failure==9&&r.reads==0&&writer.writes==1);
   r.reads=0;duration.Step(r,f.Pawn,42,200,writer);peak=(std::max)(peak,r.reads);f.r.Read(state+d::RemainingTime,left);
   check(r.reads<1250&&duration.report.durationState==3&&writer.writes==2&&left==3589);
   r.reads=0;duration.Step(r,f.Pawn,42,300,writer);check(writer.writes==2&&duration.report.retired);
   r.reads=0;duration.Step(r,f.Pawn,42,400,writer);check(r.reads==0&&writer.writes==2);
 }
 std::printf("NativeHumanJasonOptionsTests: %u failures; native ownership+knives+single2x movement peak%u/1536 reads\n",failures,peak);return failures?1:0;
}
