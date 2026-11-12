#define CFG_SWEATER_USE_MEMBER_ONLY
#include "SweaterUseRuntimeMembersTests.cpp"
int main(){
 {cfgh241sweater::ClassProofMemo memo;cfgprobe::lifetime::SlotObservation cls{0x1000,4,7},cdo{0x2000,5,8};
  memo={0x3000,0x4000,0x1000,0x2000,cls,cdo};
  CHECK(cfgh241sweater::Matches(memo,0x3000,0x4000,0x1000,0x2000,cls,cdo));
  CHECK(!cfgh241sweater::Matches(memo,0x3001,0x4000,0x1000,0x2000,cls,cdo));
  CHECK(!cfgh241sweater::Matches(memo,0x3000,0x4000,0x1000,0x2000,cls,{0x2000,5,9}));}
 {SweaterUseFixture host;host.hosted=true;
  host.Phase(100);host.Phase(600);
  CHECK(host.bundleReport.sweaterGrants==1);
  host.usedValue=1;host.Phase(2100);host.usedValue=0;
  for(std::uint64_t at=13100;at<15000;at+=100){
    host.r_.reads=0;host.SweaterPhase(at);
  }
  CHECK(host.bundleReport.sweaterRecharges==1);}
 {SweaterUseFixture tommy;tommy.bundleSweaterPolicy.stage=cfgsweater::Stage::Waiting;
  tommy.bundleSweaterPolicy.lastUse=100;tommy.bundleSweaterPolicy.rearmAt=1000;
  tommy.bundleSweaterPolicy.nativeOwnershipEarned=true;tommy.bundleSweaterPolicy.hostedOwnerHandoffUsed=true;
  tommy.fixtureMask=false;const auto calls=tommy.calls_;
  CHECK(!tommy.FreshSweaterRearm(1200)&&tommy.calls_==calls&&!tommy.bundleReport.sweaterFreshCreated);}
 {SweaterUseFixture x;x.Phase(100);x.Phase(600);const auto old=x.bundleSweaterAbility.object;
  CHECK(x.bundleSweaterClass.anchor==8);x.Retire();
  x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  x.Phase(1100);x.Phase(1600);x.Phase(2100);CHECK(!x.bundleSweaterAbility.object&&x.SweaterClassCurrent());
  x.r_.reads=0;CHECK(x.FreshSweaterRearm(12100)&&x.bundleReport.sweaterFreshCreated==1);
  CHECK(x.bundleSweaterAbility.object&&x.bundleSweaterAbility.object!=old&&x.bundleSweaterFreshPending);
  CHECK(x.bundleReport.sweaterFreshAttempts==1&&x.calls_==1);
  x.r_.reads=0;CHECK(!x.FreshSweaterRearm(12101)&&x.bundleReport.sweaterFreshAttempts==1); // no constructor replay
  for(std::uint64_t frame=12102;frame<14000;frame+=16){x.r_.reads=0;x.SweaterPhase(frame);CHECK(x.r_.reads<1536);}
  CHECK(x.bundleReport.sweaterRecharges==1&&x.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);
  CHECK(x.bundleKill.policy.until==31100&&!x.bundleKill.accepted);}
 {SweaterUseFixture mask;mask.Phase(100);mask.Phase(600);const auto grants=mask.bundleReport.sweaterGrants;
  mask.Retire();mask.r_.Put(mask.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  mask.bundleSweaterPolicy.Refuse();mask.bundleSweaterPolicy.lastFailure=112;mask.Phase(1100);
  CHECK(mask.bundleLateMaskRetryUsed&&mask.SweaterClassCurrent()&&!mask.bundleSweaterPolicy.lastUse);
  mask.r_.reads=0;CHECK(mask.FreshSweaterRearm(1700)&&mask.bundleReport.sweaterFreshCreated==1);
  for(std::uint64_t frame=1701;frame<3500;frame+=16){mask.r_.reads=0;mask.SweaterPhase(frame);}
  CHECK(mask.bundleReport.sweaterGrants==grants+1&&mask.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned&&!mask.bundleReport.sweaterUses);}
 {SweaterUseFixture zero;zero.Serial(zero.nativeAbility,0);zero.bundleSweaterAbility={};zero.r_.reads=600;
  CHECK(zero.BindOwnedSweater(zero.nativeAbility)&&zero.bundleSweaterClassAttempted&&zero.bundleSweaterClass.anchor==8);
  std::printf("S11 zero-serial earned bind including 600 prior reads: %u/1536 reads, class failure%u\n",zero.r_.reads,zero.bundleReport.sweaterClassFailure);
  CHECK(zero.r_.reads<1536);zero.bundleSweaterPolicy.ConfirmPublish();zero.Retire();
  zero.r_.Put(zero.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});zero.Phase(1000);zero.Phase(1500);zero.Phase(2000);
  zero.r_.reads=0;CHECK(zero.SweaterClassCurrent()&&zero.FreshSweaterRearm(12000)&&zero.bundleReport.sweaterFreshCreated==1);}
 {SweaterUseFixture denied;denied.classRetentionAllowed=false;denied.Phase(100);
  CHECK(denied.bundleReport.sweaterClassFailure==33&&!denied.bundleSweaterClass.slot.object);
  denied.bundleSweaterPolicy.stage=cfgsweater::Stage::Waiting;denied.bundleSweaterPolicy.lastUse=1;denied.bundleSweaterPolicy.rearmAt=1;
  denied.r_.reads=0;CHECK(!denied.FreshSweaterRearm(12000)&&!denied.bundleReport.sweaterFreshAttempts&&!denied.calls_);}
 {SweaterUseFixture respawn;respawn.Phase(100);respawn.Phase(600);
  CHECK(respawn.bundleSweaterClass.anchor==8&&respawn.SweaterClassCurrent());
  respawn.bundleSweaterFreshRequested=true;respawn.bundleSweaterFreshPending=true;
  const auto oldOwner=respawn.bundleSweaterOwner;const auto tommy=respawn.r_.Object();respawn.Serial(tommy,91);
  cfgprobe::lifetime::SlotObservation tommySlot;CHECK(cfgspawn::ObjectSlot(respawn.r_,respawn.context.objectArray,tommy,tommySlot));
  respawn.r_.Put(respawn.plan.controller+cfgprobe::ControllerPawn,tommy);
  respawn.r_.Put(tommy+cfgescape::disk::Controller,respawn.plan.controller);
  respawn.bundleHumanTarget=tommySlot;respawn.r_.reads=0;
  CHECK(respawn.FreshSweaterContinuation(9000));
  CHECK(respawn.bundleSweaterOwner==tommySlot&&!(respawn.bundleSweaterOwner==oldOwner));
  CHECK(!respawn.bundleSweaterFreshRequested&&!respawn.bundleSweaterFreshPending&&!respawn.bundleReport.sweaterFreshAttempts);
  CHECK(respawn.bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant);
  respawn.fixtureMask=true;
  for(std::uint64_t at=9001;at<10000;at+=100){respawn.Phase(at);
   if(respawn.bundleSweaterFreshRequested)CHECK(respawn.FreshSweaterContinuation(at+3));}
  CHECK(respawn.bundleSweaterOwner==tommySlot);
  CHECK(respawn.bundleReport.sweaterFreshCreated==1&&respawn.bundleSweaterAbility.object&&
      respawn.bundleSweaterAbility.object!=oldOwner.object);
  for(std::uint64_t at=10001;at<13000;at+=100){respawn.Phase(at);
   if(respawn.bundleSweaterFreshRequested)CHECK(respawn.FreshSweaterContinuation(at+3));}
  CHECK(respawn.bundleReport.sweaterGrants>=1&&respawn.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);
  respawn.r_.Put(tommy+cfgfeature::disk::HasSweater,std::uint8_t{});
  for(std::uint64_t at=13001;at<40000&&respawn.bundleReport.sweaterRecharges==0;at+=100){respawn.Phase(at);
   if(respawn.bundleSweaterFreshRequested)CHECK(respawn.FreshSweaterContinuation(at+3));}
  CHECK(respawn.bundleSweaterPolicy.lastUse&&respawn.bundleReport.sweaterRecharges>=1&&
      respawn.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);
 }
 std::printf("S11 actual FreshSweaterRearm: %d failures; captured class survives retirement -> one fresh instance -> recharge; female mask -> grant; capture failure -> zero constructors. Native constructor/world boundary supplied, real member identity/mutation gates.\n",failures);
 return failures?1:0;
}
