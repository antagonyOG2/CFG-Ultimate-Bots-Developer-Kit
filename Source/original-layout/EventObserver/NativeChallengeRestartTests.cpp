#define CFG_ESCAPE_SCOPE_FIXTURE_ONLY
#include "NativeChallengeEscapeScopeTests.cpp"
#include "NativeChallengeRestart.hpp"
int main(){unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok){++failures;std::printf("FAIL check %u\n",checks);}};
 for(unsigned fault=0;fault<11;++fault){ScopeFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;
  cfgmenu::challengerestart::Runtime runtime;scope::Witness out;std::array<unsigned,8> choices{{0,1,0,0,0,1,1,0}};
  runtime.Arm(f.Menu(),choices);check(!runtime.Observe(r,f.Mode,42,100,out));
  check(runtime.choices==choices);r.count=0;
  check(!runtime.Observe(r,f.Mode,42,200,out)&&r.count<64);r.count=0;
  f.Phase("PreMatchIntro");static_cast<StageReader&>(r)=f.r;
  r.Set(f.Registry+16,int{200});r.Set(f.Registry+24+16,int{201});
  if(fault==1)r.Set(f.Registry+16,int{100});
  if(fault==2)r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
  if(fault==3)r.Set(f.Controller+0x578,f.PS);
  if(fault==4)runtime.Cancel();
  if(fault==5){r.Set(f.Viewport+cfgprobe::ViewportWorld,f.Menu().world);}
  if(fault==6){f.Label(f.Challenge,"ChallengeC02_PackanackParty_C");
   f.Label(f.ChallengePackage,"/Game/Blueprints/MapRegistry/Challenges/ChallengeC02_PackanackParty");}
  if(fault==9)f.Label(f.WorldPackage,"/Game/Maps/Single_Player/SPC02_PackanackParty");
  if(fault==6||fault==9){f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
   static_cast<StageReader&>(r)=f.r;r.Set(f.Registry+16,int{200});r.Set(f.Registry+24+16,int{201});}
  if(fault==7)r.Set(f.Registry+24+16,int{101});
  const bool accepted=runtime.Observe(r,fault==8?f.Pawn:f.Mode,fault==10?43:42,300,out);
  check(accepted==(fault==0));check(runtime.restarts==(fault==0?1u:0u));
  r.count=0;check(runtime.Observe(r,f.Mode,42,400,out)==(fault==8||fault==10));
 }
 std::printf("Challenge restart same mission/fresh owners/off preservation: %u checks %u failures\n",checks,failures);return failures?1:0;
}
