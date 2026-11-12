#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
namespace cfgbundle { template<class T> inline void RecordSweaterFirstRefusal(T&,unsigned,unsigned,unsigned,
    std::uintptr_t,std::uint64_t,unsigned=0) noexcept {} }
#include "../Core/SweaterLifecyclePolicy.hpp"
#include "../Core/SweaterOwnerRefreshPolicy.hpp"
#include "RetiredOwnedAbility.hpp"
#include "FreshAbilityClassObservation.hpp"
#include "../Core/HostedMaskTriggerMemo.hpp"
#include "../Core/HostedSweaterClassProofMemo.hpp"
#include "../RuntimeProbe/FreshAbilityBindings.generated.hpp"
#include "../RuntimeProbe/SweaterBindings.generated.hpp"
#include "SweaterSourceObservation.hpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "KillRuntime.hpp"
struct SweaterUseFixture:SpectatorFixture {
 cfgprobe::lifetime::SlotObservation bundleSweaterOwner{},bundleSweaterAbility{},bundleSweaterSource{},bundleSweaterRetiredAbility{};
 cfgsweater::Policy bundleSweaterPolicy;
 cfgkill::Runtime bundleKill;
 struct {unsigned sweaterUses=0,sweaterUseEdgeSource=0,sweaterOwnershipUses=0,sweaterAbilityUses=0,sweaterRetiredAbilities=0;
  unsigned sweaterBound=0,sweaterStage=0,sweaterClassAttempts=0,sweaterChecks=0,sweaterUnavailable=0;
  unsigned sweaterFreshFailure=0,sweaterFreshAdmission=0,sweaterFreshAdmissionReads=0,sweaterFreshAdmissionCalls=0,sweaterFreshAttempts=0,sweaterFreshCreated=0;
  unsigned sweaterTurnOwnerReads=0,sweaterTurnContextReads=0,sweaterTurnPamelaReads=0,sweaterTurnClassReads=0;
  unsigned sweaterTurnDiscoveryReads=0,sweaterTurnHostedReads=0,sweaterTurnPreviousReads=0,sweaterTurnAdmissionReads=0;
  unsigned sweaterClassAnchor=0,sweaterClassFailure=0,sweaterClassFlags=0,sweaterClassCdoFlags=0,sweaterGrants=0,sweaterRecharges=0,killStage=0,killFailure=0,killAcceptedHits=0;
  unsigned sweaterTemplateStage=0;int sweaterClassSerial=0,sweaterClassCdoSerial=0,sweaterTemplateSourceSerial=0,sweaterTemplateSerial=0;
  std::uintptr_t sweaterTemplateSourceClass=0,sweaterTemplateSourceDefault=0,sweaterTemplateObject=0,sweaterTemplateClass=0;
  std::uintptr_t sweaterFreshClass=0;} bundleReport;
 cfgsweater::ClassLease bundleSweaterClass;
 struct PrimaryRoots {template<class R>bool CurrentReference(R&,std::uintptr_t object){return object!=0;}} primaryRoots;
 bool classRetentionAllowed=true,classMetadataRetained=false;
 unsigned bundleSweaterSourceIndex=~0u,bundleSweaterCursor=0,bundleSweaterSettingsCursor=0,bundleSweaterWarm=2;
 bool bundleSweaterFreshPending=false,bundleSweaterFreshRequested=false,bundleSweaterFreshGrantAttempted=false;
 bool bundleSweaterClassAttempted=true,bundleSweaterClassPending=false,bundleLateMaskRetryUsed=false;
 std::uint64_t bundleSweaterFreshUse=0;
 bool wearerLiving=true,fixtureMask=true,scope=true;float squared=100,health=100;unsigned usedValue=0,abilityReads=0,publications=0;
 std::uintptr_t nativeAbility=0;
 bool hosted=false;
 unsigned Reads()const{return r_.reads;}
 bool HostedRoutineActive()const{return hosted;}
 bool Context(){context=plan;return scope;} // supplies native admission, real weak slot/owner proofs below
 bool Known(unsigned i,std::uintptr_t& pawn){pawn=bundleHumanTarget.object;return i==0;}
 bool Alive(std::uintptr_t,bool& value){value=wearerLiving;return true;}
 bool HasPamelaTrigger(bool& value){value=fixtureMask;return true;}
 bool Scope(cfgkill::Identity& id){id={plan.world,r_.pawn,1};return scope;}
 bool KnownLiving(std::uintptr_t p){return wearerLiving&&p==bundleSweaterOwner.object;}
 bool MaskOff(bool& off){off=true;return true;}
 bool PamelaHealthValue(float& value){value=health;return true;}
 cfgsweater::KillArmResult ArmSweaterKill(cfgkill::Runtime& k,const cfgkill::Identity& id,std::uintptr_t wearer,std::uint64_t now){
  return k.policy.PamelaUsed(id,wearer,now,true,squared,health)?cfgsweater::KillArmResult::Accepted:cfgsweater::KillArmResult::Rejected;}
#include "SweaterOwnerRuntimeMembers.hpp"
#include "SweaterUseObservationMembers.hpp"
#include "SweaterKillArmRuntimeMembers.hpp"
 bool SweaterBinding(std::uintptr_t ability){++abilityReads;cfgprobe::lifetime::SlotObservation found;
  return cfgspawn::ObjectSlot(r_,context.objectArray,ability,found)&&found==bundleSweaterAbility;}
 bool SweaterFunction(bool,std::uintptr_t& out,bool=true){out=1;return true;}
 bool Invoke(std::uintptr_t,std::uintptr_t,void* out,bool){*static_cast<unsigned char*>(out)=static_cast<unsigned char>(usedValue);return true;}
 bool TypedFeature(std::uintptr_t object,cfgfeature::disk::ClassId kind,bool){
  std::uintptr_t native=0;const auto& b=cfgfeature::disk::Classes[kind];
  return NativeClass(b.reflection,b.constructor,native)&&cfgspawn::ObjectIs(r_,object,native);}
 bool SweaterClassMetadataCurrent(){return classMetadataRetained&&bundleSweaterClass.anchor==8;}
 bool SweaterClassCurrent(){return bundleSweaterClass.Current(r_,context.objectArray,plan.module,SweaterClassMetadataCurrent());}
 bool RetainSweaterClassMetadata(){if(!classRetentionAllowed||!bundleSweaterClass.Current(r_,context.objectArray,plan.module))return false;classMetadataRetained=true;return true;}
#include "SweaterClassCaptureRuntimeMembers.hpp"
#include "SweaterBindRuntimeMembers.hpp"
 bool DiscoverSweater(){bundleSweaterClassAttempted=true;return true;}
 bool SweaterPickupPhase(std::uint64_t){return false;}
 bool PublishSweater(bool rearm,std::uint64_t now){++publications;r_.Put(bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{1});
  bundleSweaterPolicy.QueueHud(now,rearm);return true;} // native write boundary stub; publication suite tests actual contract
 bool PublishSweaterHud(){if(bundleSweaterPolicy.pendingRearm)++bundleReport.sweaterRecharges;else ++bundleReport.sweaterGrants;
  bundleSweaterPolicy.ConfirmPublish();return true;}
 bool DiscoverSweaterSettings(){return false;}
 bool NativeCallAllowed(){return scope;}
 bool ConstructFreshSweater(std::uintptr_t& fresh){
  fresh=r_.Object(bundleSweaterClass.slot.object);r_.Put(fresh+0x20,bundleSweaterOwner.object);
  r_.Put(fresh+cfgfeature::disk::AbilityOwner,std::uintptr_t{});r_.Put(fresh+cfgsweater::disk::Used,std::uint8_t{});return true;
 } // native constructor boundary only; actual rearm/identity/publication body below
 bool FreshSweaterWorldAfterAllocation(){context=plan;return scope&&GetCurrentThreadId()==plan.threadId;}
 template<class T>bool SweaterStore(std::uintptr_t at,T before,T after){T current{};
  if(!r_.Read(at,current)||current!=before)return false;r_.Put(at,after);return true;}
#include "SweaterFreshRuntimeMembers.hpp"
#include "SweaterPhaseRuntimeMembers.hpp"
 SweaterUseFixture(){r_.cap=100000;bundleSweaterOwner=bundleHumanTarget;
  r_.Put(plan.controller+cfgprobe::ControllerPawn,bundleSweaterOwner.object);
  r_.Put(bundleSweaterOwner.object+cfgescape::disk::Controller,plan.controller);
  const auto native=r_.Object(),derived=r_.Object(),cdo=r_.Object(derived);
  const auto& counselor=cfgfeature::disk::Classes[cfgfeature::disk::Counselor];
  r_.Put(plan.module+counselor.reflection,r_.counselorBase);r_.Put(plan.module+counselor.constructor,r_.counselorBase);
  const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
  r_.Put(plan.module+binding.reflection,native);r_.Put(plan.module+binding.constructor,native);
  r_.Put(derived+cfgprobe::ClassSuper,native);r_.Put(derived+cfgspawn::ClassDefaultObject,cdo);
  nativeAbility=r_.Object(derived);Serial(nativeAbility,42);int abilityIndex=0;r_.Read(nativeAbility+cfgprobe::lifetime::ObjectIndexOffset,abilityIndex);
  r_.Put(r_.objects.data+24ULL*static_cast<unsigned>(abilityIndex),cfgsweater::RegistryItem{nativeAbility,0,0,42,0});CHECK(cfgspawn::ObjectSlot(r_,plan.objectArray,nativeAbility,bundleSweaterAbility));
  r_.Put(nativeAbility+cfgfeature::disk::AbilityOwner,bundleSweaterOwner.object);
  r_.Put(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,nativeAbility);
  r_.Put(bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{1});
  r_.Put(plan.module+cfgsweater::freshdisk::SpawnObject,cfgsweater::freshdisk::SpawnObjectCode);r_.reads=0;r_.cap=1536;
 }
 void Phase(std::uint64_t now){r_.reads=0;SweaterPhase(now);CHECK(r_.reads<1536);
  r_.reads=0;SweaterPhase(now+1);CHECK(r_.reads<1536);
  r_.reads=0;SweaterPhase(now+2);CHECK(r_.reads<1536);}
 void Retire(){const auto at=r_.objects.data+24ULL*static_cast<unsigned>(bundleSweaterAbility.index);
  r_.Put(at,std::uintptr_t{});r_.Put(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,std::uintptr_t{});}
};
#ifndef CFG_SWEATER_USE_MEMBER_ONLY
int main(){
 SweaterUseFixture f;f.Phase(100);CHECK(f.bundleSweaterPolicy.stage==cfgsweater::Stage::HudPending);f.Phase(600);
 CHECK(f.bundleReport.sweaterGrants==1&&f.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);
 CHECK(f.bundleSweaterClass.anchor==8&&f.bundleSweaterClassAttempted&&f.classMetadataRetained);
 f.Retire();f.r_.Put(f.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});const auto reads=f.abilityReads;
 f.Phase(1100);CHECK(f.bundleReport.sweaterUses==1&&f.bundleReport.sweaterUseEdgeSource==1&&f.abilityReads==reads);
 CHECK(f.bundleSweaterPolicy.lastUse==1100&&f.bundleSweaterPolicy.rearmAt==12100);
 f.Phase(1600);CHECK(f.bundleKill.policy.pamela&&f.bundleKill.policy.until==31100);
 f.Phase(2100);CHECK(!f.bundleSweaterAbility.object&&f.bundleSweaterRetiredAbility.object==f.nativeAbility);
 CHECK(f.bundleReport.sweaterRetiredAbilities==1&&!f.bundleSweaterPolicy.ObserveUsed(2101,1));
 CHECK(f.SweaterClassCurrent()); // no old ability instance read required
 // A real damage notification is still the donor B1 any-weapon kneel transition.
 CHECK(f.bundleKill.policy.PamelaAccept(f.bundleKill.policy.identity,2200)&&f.bundleKill.policy.PamelaAccepted());
 f.bundleKill.policy.Expire(70000); // completed/expired kill no longer owns cinematic
 CHECK(f.bundleSweaterPolicy.Decide(12099,false,false,false)==cfgsweater::Action::None);
 CHECK(f.bundleSweaterPolicy.Decide(12100,false,false,false)==cfgsweater::Action::Rearm);
 CHECK(f.SweaterRearmPrevious(f.nativeAbility));const auto old=f.bundleSweaterRetiredAbility;
 f.r_.Put(f.r_.objects.data+24ULL*static_cast<unsigned>(old.index),old.object);f.Serial(old.object,43);
 f.r_.reads=0;CHECK(!f.SweaterRearmPrevious(f.nativeAbility)); // valid address-reused native replacement wins
 // An untracked replacement is not authority to overwrite a native ability.
 CHECK(!f.SweaterRearmPrevious(f.plan.pawn));
 {SweaterUseFixture x;x.bundleSweaterPolicy.ConfirmPublish();x.Serial(x.nativeAbility,43);x.usedValue=0;
  x.Phase(1000);CHECK(x.bundleSweaterAbility.serial==43&&!x.bundleReport.sweaterRetiredAbilities);
  CHECK(x.bundleSweaterPolicy.stage==cfgsweater::Stage::HudPending&&!x.bundleReport.sweaterUses);}
 {SweaterUseFixture x;x.bundleSweaterPolicy.ConfirmPublish();x.Retire();
  x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  x.Phase(1000);x.Phase(1500);x.Phase(2000);x.Phase(12000);
  CHECK(x.bundleSweaterFreshRequested&&x.bundleKill.policy.stage==cfgkill::Stage::AwaitHit&&x.bundleKill.policy.until==31000);
  // Constructor/publication native boundaries are stubbed; the REAL phase
  // must finish its earned HUD recharge while the unused30s window remains.
  x.r_.cap=100000;const auto fresh=x.r_.Object();x.Serial(fresh,44);
  CHECK(cfgspawn::ObjectSlot(x.r_,x.context.objectArray,fresh,x.bundleSweaterAbility));
  x.r_.Put(fresh+cfgfeature::disk::AbilityOwner,x.bundleSweaterOwner.object);
  x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::AbilityField,fresh);
  x.bundleSweaterFreshPending=true;x.r_.cap=1536;
  for(std::uint64_t frame=12500;frame<14000;frame+=16){x.r_.reads=0;x.SweaterPhase(frame);CHECK(x.r_.reads<1536);}
  CHECK(x.bundleReport.sweaterRecharges==1&&x.bundleReport.sweaterUses==1&&x.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);
  CHECK(x.bundleKill.policy.until==31000&&x.bundleKill.policy.stage==cfgkill::Stage::AwaitHit);}
 for(unsigned warm=0;warm<=2;warm+=2){SweaterUseFixture early;early.bundleSweaterWarm=warm;
  early.Phase(100);CHECK(early.bundleSweaterPolicy.nativeOwnershipEarned);
  early.Retire();early.r_.Put(early.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  early.Phase(600);CHECK(early.bundleReport.sweaterUses==1&&early.bundleSweaterPolicy.armPending);
  CHECK(early.bundleReport.sweaterUseEdgeSource==1);}
 for(unsigned fault=0;fault<4;++fault){SweaterUseFixture x;x.bundleSweaterPolicy.ConfirmPublish();
  x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{});
  if(fault==0)x.scope=false;if(fault==1)x.Serial(x.bundleSweaterOwner.object,99);
  if(fault==2)x.wearerLiving=false;if(fault==3)x.r_.memory.erase(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater);
  x.r_.reads=0;x.SweaterPhase(1000);CHECK(!x.bundleReport.sweaterUses&&!x.bundleKill.policy.pamela);}
 {SweaterUseFixture x;const auto oldAbility=x.bundleSweaterAbility;x.r_.cap=0;CHECK(!x.RetireBoundSweaterAbility());
  CHECK(x.bundleSweaterAbility==oldAbility&&!x.bundleReport.sweaterRetiredAbilities);}
 {SweaterUseFixture x;x.bundleSweaterPolicy.Refuse();x.bundleSweaterPolicy.lastFailure=112;x.Retire();x.Phase(1000);
  CHECK(!x.bundleSweaterAbility.object&&x.bundleLateMaskRetryUsed&&(x.bundleSweaterPolicy.stage==cfgsweater::Stage::Dormant||x.bundleSweaterPolicy.stage==cfgsweater::Stage::Discovering));
  x.Phase(1500);CHECK(x.bundleSweaterFreshRequested&&!x.bundleReport.sweaterUses);}
#if CFG_J25_SWEATER_REBIND
 // Actual included phase/binding path: two native ability replacements between
 // a single use and the due rearm. Never borrow a new ability without a native
 // reciprocal pawn/ability edge and positive assigned registry identity.
 {SweaterUseFixture x;x.bundleSweaterPolicy.ConfirmPublish();
  CHECK(x.bundleSweaterPolicy.ObserveUsed(1000,1));x.bundleSweaterPolicy.armPending=false;
  x.Retire();x.r_.reads=0;CHECK(x.RetireBoundSweaterAbility());
  std::uintptr_t type=0;CHECK(x.r_.Read(x.nativeAbility+cfgprobe::ObjectClass,type));
  const auto nativeReplacement=[&](){const auto next=x.r_.Object(type);x.Serial(next,50+static_cast<int>(x.bundleSweaterPolicy.rebinds));
      x.r_.Put(next+cfgfeature::disk::AbilityOwner,x.bundleSweaterOwner.object);
      x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::AbilityField,next);
      x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::HasSweater,std::uint8_t{1});return next;};
  const auto first=nativeReplacement();x.usedValue=0;x.Phase(2000);
  CHECK(x.bundleSweaterAbility.object==first&&x.bundleSweaterPolicy.rebinds==1&&x.bundleReport.sweaterBound==1);
  CHECK(x.bundleSweaterPolicy.stage==cfgsweater::Stage::Waiting&&x.bundleSweaterPolicy.rearmAt==12000);
  const auto second=nativeReplacement();x.usedValue=1;x.Phase(2500);
  CHECK(x.bundleSweaterAbility.object==second&&x.bundleSweaterPolicy.rebinds==2);
  CHECK(x.bundleSweaterPolicy.lastUse==1000&&x.bundleSweaterPolicy.rearmAt==12000);
  x.Phase(11500);CHECK(x.bundleReport.sweaterRecharges==0);
  x.usedValue=0;x.Phase(12000);x.Phase(12500);
  CHECK(x.bundleReport.sweaterRecharges==1&&x.bundleSweaterPolicy.stage==cfgsweater::Stage::Owned);}
 // Retire-without-use: a positive new pickup must supersede an already
 // scheduled repair, without fabricating an ability-use edge.
 {SweaterUseFixture x;x.bundleSweaterPolicy.ConfirmPublish();x.Retire();x.Phase(1000);
  CHECK(x.bundleSweaterPolicy.stage==cfgsweater::Stage::Waiting&&x.bundleSweaterPolicy.repairingOwned);
  std::uintptr_t type=0;CHECK(x.r_.Read(x.nativeAbility+cfgprobe::ObjectClass,type));
  const auto replacement=x.r_.Object(type);x.Serial(replacement,53);
  x.r_.Put(replacement+cfgfeature::disk::AbilityOwner,x.bundleSweaterOwner.object);
  x.r_.Put(x.bundleSweaterOwner.object+cfgfeature::disk::AbilityField,replacement);
  x.usedValue=0;x.Phase(1500);
  CHECK(x.bundleSweaterAbility.object==replacement&&x.bundleSweaterPolicy.rebinds==1);
  CHECK(!x.bundleSweaterPolicy.repairingOwned&&!x.bundleReport.sweaterUses);}
#endif
 // J25: a new native mask/pickup ability can replace a positively retired
 // bound instance; rebind never accelerates the eleven-second recharge clock.
 {cfgsweater::Policy p;p.ConfirmPublish();CHECK(p.ObserveUsed(1000,1));
  const auto use=p.lastUse,due=p.rearmAt;
  CHECK(p.RebindNative(2000,0)&&p.rebinds==1&&p.stage==cfgsweater::Stage::Waiting);
  CHECK(p.lastUse==use&&p.rearmAt==due&&p.Decide(11999,false,true,true)==cfgsweater::Action::None);
  CHECK(p.RebindNative(8000,1)&&p.rebinds==2&&p.lastUse==use&&p.rearmAt==due);
  CHECK(p.Decide(12000,false,true,true)==cfgsweater::Action::Rearm);
  p.ConfirmPublish();CHECK(p.ObserveUsed(13000,1));
  CHECK(p.RebindNative(13500,0)&&p.rebinds==3&&p.rearmAt==24000);
  CHECK(p.RebindNative(14000,0)&&p.rebinds==4&&p.rearmAt==24000);
  CHECK(!p.RebindNative(14500,2)&&p.rebinds==4);
  p.Refuse();CHECK(!p.RebindNative(15000,0)&&p.rebinds==4);}
 {cfgsweater::Policy p;p.ConfirmPublish();CHECK(p.QueueRetiredOwned(1000));
  CHECK(p.repairingOwned&&p.RebindNative(1500,0)&&!p.repairingOwned);
  CHECK(p.Decide(1500,false,true,true)==cfgsweater::Action::Rearm);}
 {cfgsweater::Policy p;p.ConfirmPublish();p.BeginPublish();
  CHECK(!p.RebindNative(1000,0)&&!p.rebinds);}
 {cfgsweater::Policy p;CHECK(!p.ObserveOwnership(1,0));p.stage=cfgsweater::Stage::Owned;
  CHECK(!p.ObserveOwnership(2,0)&&!p.ObserveOwnership(3,1)&&p.ObserveOwnership(4,0));
  CHECK(!p.ObserveUsed(5,1)&&!p.ObserveOwnership(6,0));}
 {SweaterUseFixture denied;denied.classRetentionAllowed=false;denied.Phase(100);
  CHECK(denied.bundleReport.sweaterClassAttempts==1&&denied.bundleReport.sweaterClassFailure==33&&!denied.bundleSweaterClassAttempted);}
 std::printf("S1 actual wearer/phase/use/retired registry/arm/recharge/mask members: %d failures; sparse owned objects, native boundaries stubbed.\n",failures);
 return failures?1:0;
}
#endif
