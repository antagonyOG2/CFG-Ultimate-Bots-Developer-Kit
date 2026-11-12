#define CFG_J22_END_SUMMARY 1
#define CFG_SUMMARY_TEST_MAIN ExistingSummaryMain
#include "NativeSummaryTests.cpp"
int main(){
 const auto existing=ExistingSummaryMain();unsigned failed=0,checks=0;
 const auto check=[&](bool value){++checks;if(!value){++failed;std::printf("FAIL J22 check %u\n",checks);}};
 check(!cfgmenu::active::StackArray{0x9A0000,0,1}.Sane()); // no global menu admission weakening
 check(cfgstatussurface::StackHeader({0,0,0}));
 check(!cfgstatussurface::StackHeader({0,0,1}));
 check(!cfgstatussurface::StackHeader({0x9A0000,-1,1}));
 check(!cfgstatussurface::StackHeader({0x9A0000,0,65}));
 SummaryRuntimeFixture f;f.multiple=true;f.r_.cap=100000;
 std::array<std::uintptr_t,4> states{{f.state,0,0,0}};
 for(unsigned i=1;i<4;++i){states[i]=f.r_.Object(f.r_.classes[cfgspawn::PlayerState]);
  f.r_.Put(states[i]+0x88,f.r_.botController);f.r_.Put(states[i]+0x394,4u);f.r_.Put(states[i]+0x13C,std::uint8_t{});f.r_.Put(0x950000+8ULL*i,states[i]);}
 // Primary killer's native state is role bit8, not bIsABot bit4. Two ordinary
 // counselors + a late F3 counselor retain their native dead/escaped/alive data.
 f.r_.Put(states[0]+0x394,8u);f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKillerState,states[0]);
 f.r_.Put(states[1]+cfgescape::disk::bDead,std::uint8_t{1});f.r_.Put(states[2]+cfgescape::disk::bEscaped,std::uint8_t{1});
 f.r_.Put(f.spawnAssets.gameState+cfgsummary::disk::Players,cfgsummary::List{0x950000,4,4});
 for(unsigned now=100;now<=2400;now+=100){f.r_.reads=0;f.r_.cap=1536;check(f.SummaryPhase(now,true));check(f.r_.reads<=1536);}
 std::printf("J22 summary registrations=%u refused=%u\n",f.nativeRegistrations,f.bundleSummary.refused);check(f.nativeRegistrations==4);
 f.r_.reads=0;f.r_.cap=100000;cfgprobe::lifetime::SlotObservation stateSlot{};check(f.Slot(states[0],stateSlot));
 struct Host{SummaryRuntimeFixture& f;
 bool Scope(){return f.SummaryScope();}
 bool Slot(std::uintptr_t p,cfgprobe::lifetime::SlotObservation& s){return f.Slot(p,s);}
 bool StateType(std::uintptr_t p){return f.SummaryStateType(p);}}host{f};
 check(cfgsummarybots::Participant(f.r_,host,f.spawnAssets.gameState,states[0],stateSlot));
 f.r_.Put(f.spawnAssets.gameState+cfgprimary::CurrentKillerState,states[1]);
 check(!cfgsummarybots::Participant(f.r_,host,f.spawnAssets.gameState,states[0],stateSlot));
 std::printf("J22 summary empty-HUD + current-killer + two bots/late F3: %u checks,%u failures; native membership retained, no synthetic PlayerState/outcome. Owned model.\n",checks,failed);
 return existing||failed?1:0;
}
