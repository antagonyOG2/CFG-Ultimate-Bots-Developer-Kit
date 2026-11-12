#define CFG_SWEATER_USE_MEMBER_ONLY
#include "SweaterUseRuntimeMembersTests.cpp"
#define CFG_PAMELA_HOST_ONLY
#include "PamelaKillTests.cpp"
struct HealthKillHost:PamelaHost {
 std::uintptr_t wearer=0;
 bool KnownLiving(std::uintptr_t pawn){return pawn==wearer||PamelaHost::KnownLiving(pawn);}
};
int main(){
 SweaterUseFixture x;x.Phase(100);x.Phase(600);x.Retire();
 x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
 x.Phase(1100);x.Phase(1600);CHECK(x.bundleKill.policy.pamela&&!x.bundleSweaterPolicy.armPending);
 CHECK(!x.bundleKill.accepted&&x.bundleKill.policy.stage==cfgkill::Stage::AwaitHit);
 x.health=90;x.Phase(2700); // REAL SweaterPhase call site, not direct PamelaAccept
 CHECK(x.bundleKill.accepted==1&&x.bundleReport.killAcceptedHits==1&&x.bundleKill.policy.PamelaAccepted());
 HealthKillHost native;native.identity=x.bundleKill.policy.identity;native.wearer=x.bundleSweaterOwner.object;
 CHECK(x.bundleKill.Tick(native,2703)==cfgkill::Step::StanceDispatched&&native.stances==1);
 // Probe-only callbacks must not consume the owed sweater opportunity.
 SweaterUseFixture waiting;waiting.Phase(100);waiting.Phase(600);waiting.Retire();
 waiting.r_.Put(waiting.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
 waiting.Phase(1100);waiting.Phase(1600);waiting.Phase(2100);waiting.Phase(12100);
 CHECK(waiting.bundleKill.policy.stage==cfgkill::Stage::AwaitHit&&!waiting.bundleKill.accepted);
 CHECK(waiting.bundleSweaterFreshRequested&&waiting.bundleSweaterPolicy.lastUse==1100);
 {SweaterUseFixture sparse;sparse.Phase(100);sparse.Phase(600);sparse.Retire();
  sparse.r_.Put(sparse.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  for(std::uint64_t frame=2000;frame<=16000;frame+=1000){sparse.r_.reads=0;sparse.SweaterPhase(frame);
   // An independent global health callback must NOT steal the owed feature phase.
   sparse.r_.reads=0;sparse.SweaterKillArmPhase(frame+300);}
  CHECK(sparse.bundleSweaterFreshRequested&&!sparse.bundleKill.accepted&&sparse.bundleSweaterPolicy.lastUse==2000);}
 std::printf("S11 actual unconditional health call-site: %d failures; armed->later health drop->accepted->native stance; unchanged-health rearm opportunity preserved.\n",failures);
 return failures?1:0;
}