#include "HostedTakeoverFixture.hpp"
#include "HostedBirthAdmission.hpp"
#include "../Core/HostedCounselorBirth.hpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 check(cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,14,9,true),"same registered birth slot promotes after independent proof");
 check(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x820000,14,9,true),"different state pointer cannot be promoted");
 check(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,15,9,true),"different object-array index cannot be promoted");
 check(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,14,0,true),"still-unassigned serial cannot be promoted");
 check(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,3,0x810000,14,9,true),"already assigned birth serial must not be replaced");
 check(cfghostedfill::DecideBrainRetry(true,false,false,0)==cfghostedfill::BrainRetry::Wait,"stable birth waits for positive owned slots");
 check(cfghostedfill::DecideBrainRetry(true,true,false,2)==cfghostedfill::BrainRetry::Dispatch,"ready birth dispatches within retry budget");
 check(cfghostedfill::DecideBrainRetry(false,true,false,0)==cfghostedfill::BrainRetry::Cancel,"changed birth tuple cancels retry");
 check(cfghostedfill::DecideBrainRetry(true,true,true,0)==cfghostedfill::BrainRetry::Cancel,"entered native dispatch is never replayed");
 check(cfghostedfill::DecideBrainRetry(true,true,false,3)==cfghostedfill::BrainRetry::Cancel,"retry queue is bounded at three");
 const cfghostedfill::BrainRetryLocalIdentity localTicket{0x700000,0x300000,0x301000,3,7,4,11};
 check(cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x301000,0x300000,3,7,0x301000,4,11),"retry entry uses saved local host callback identity");
 check(!cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x901000,0x300000,3,7,0x901000,8,19),"born AI pawn cannot substitute for local callback pawn");
 check(!cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x301001,0x300000,3,7,0x301001,4,11),"changed local pawn cancels retry");
 check(!cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x301000,0x300000,3,8,0x301000,4,11),"changed local controller serial cancels retry");
 check(cfghostedfill::MatchBrainRetryBirth(0x700000,0x500000,0x501000,0x901000,0x700000,0x500000,0x501000,0x901000),"birth ticket accepts exact distinct AI birth tuple");
 check(!cfghostedfill::MatchBrainRetryBirth(0x700000,0x500000,0x501000,0x901000,0x700000,0x500000,0x501000,0x901001),"changed born pawn cancels retry");
 {unsigned retries=0,starts=0,unrelated=0;bool pending=true;
  const auto enter=cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x301000,0x300000,3,7,0x301000,4,11);
  auto step=cfghostedfill::DecideBrainRetry(enter,false,false,retries);check(enter&&step==cfghostedfill::BrainRetry::Wait,"entry waits while birth weak serial is zero");++retries; // leave scope without a native dispatch
  step=cfghostedfill::DecideBrainRetry(enter,true,false,retries);if(step==cfghostedfill::BrainRetry::Dispatch)++starts;
  check(step==cfghostedfill::BrainRetry::Dispatch&&starts==1,"later same local callback dispatches once after positive serial promotion");
  step=cfghostedfill::DecideBrainRetry(enter,true,true,retries);if(step==cfghostedfill::BrainRetry::Cancel)pending=false;
  check(!pending&&starts==1,"window closes after native entry; no replay");
  const bool wrongActor=cfghostedfill::MatchBrainRetryLocal(localTicket,0x700000,0x901000,0x300000,3,7,0x901000,8,19);
  if(!wrongActor)++unrelated;check(unrelated==1,"unrelated/newborn callback remains available to other work");}
 for(unsigned fault=0;fault<8;++fault){HostedFixture f;h::Context owner;check(h::Capture(f.r,f.a,99,owner),"current3.5 listen context");
  for(auto state:{f.humanState,f.hostState}){f.r.Put(state+8,std::uint32_t{});std::int32_t index=0;f.r.Read(state+12,index);f.r.Put(f.table+24ULL*index+8,std::uint32_t{});}
  if(fault==1)f.r.Put(f.state+b::PlayerArray,h::LocalPlayers{f.roster,8,8});
  if(fault==2)f.r.Put(f.driver+d::ServerConnection,f.connection);
  if(fault==3)f.r.Put(f.humanState+8,std::uint32_t{0x20});
  if(fault==4)f.r.Put(f.roster+8,f.hostState);
  if(fault==5)f.r.Put(f.table+24ULL*10+16,std::int32_t{});
  if(fault==6)f.r.Put(f.table+24ULL*10+8,std::uint32_t{0x20000000});
  if(fault==7)f.r.Put(f.mode+d::ActorRole,std::uint8_t{1});
  cfghosted::native::Population before;
  const bool captured=cfghosted::native::PopulationBefore<Memory,h::Bindings,true>(f.r,owner.session,99,before);if(captured!=(fault==0||fault==5))std::printf("Population fault=%u captured=%u reads=%u\n",fault,captured?1u:0u,f.r.reads);check(captured==(fault==0||fault==5),"full/client/duplicate/dying states refused; serial0 is observed only through strong roster");
 }
 for(unsigned fault=0;fault<6;++fault){HostedFixture f;h::Context owner;
  for(auto state:{f.humanState,f.hostState}){f.r.Put(state+8,std::uint32_t{});std::int32_t index=0;f.r.Read(state+12,index);f.r.Put(f.table+24ULL*index+8,std::uint32_t{});}
  check(h::Capture(f.r,f.a,99,owner),"birth host captured");cfghosted::native::Population before;
  check((cfghosted::native::PopulationBefore<Memory,h::Bindings,true>(f.r,owner.session,99,before)),"birth roster before captured");
  constexpr std::uintptr_t aiType=0x4C0000,created=0x800000,createdState=0x810000;
  f.Pair(cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,aiType);f.Object(created,13,aiType);f.Object(createdState,14,f.stateType);
  f.r.Put(f.a.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t{15});f.r.Put(created+0x20,f.a.world);f.r.Put(createdState+0x20,f.a.world);
  f.r.Put(created+cfgprobe::ControllerPawn,std::uintptr_t{});f.r.Put(created+cfgescape::disk::PlayerState,createdState);
  f.r.Put(createdState+0x88,created);f.r.Put(created+d::ActorRole,std::uint8_t{3});f.r.Put(createdState+d::ActorRole,std::uint8_t{3});
  f.r.Put(createdState+cfghosted::birthdisk::BotFlags,cfghosted::birthdisk::BotMask);
  f.r.Put(f.state+b::PlayerArray,h::LocalPlayers{f.roster,3,8});f.r.Put(f.roster+16,createdState);f.r.Put(f.mode+b::NumBots,std::int32_t{1});
  f.r.Put(f.table+24*14+16,std::int32_t{});
  if(fault==1)f.r.Put(created+cfgprobe::ControllerPawn,f.localPawn);
  if(fault==2)f.r.Put(created+cfgescape::disk::PlayerState,f.hostState);
  if(fault==3)f.r.Put(f.roster+16,f.hostState);
  if(fault==4)f.r.Put(f.driver+d::ServerConnection,f.connection);
  if(fault==5)f.r.Put(f.state+b::CurrentKillerState,f.hostState);
  cfghosted::native::NewControllerState born;
  check((cfghosted::native::ControllerBorn<Memory,h::Bindings,true>(f.r,before,99,created,born))==(fault==0),"new own AI state, original actors/roster preserved; aliases/client changes rejected");
  if(!fault)check(!born.stateSerialAssigned&&born.controller.object==created&&born.state.object==createdState,"serial0 controller-owned state observed, not claimed pinned");
 }
 struct Host {unsigned fail=0,stage=0,spawns=0,rollback=0;bool clean=true,restored=true;
  bool At(unsigned n){stage=n;return fail!=n;}bool Preflight(){return At(1);}bool Spawn(){++spawns;return At(2);}bool Clean(){return clean;}
  bool NewController(){return At(3);}bool SetClass(){return At(4);}bool Restart(){return At(5);}bool Traveled(){return At(6);}
  bool Account(){return At(7);}bool Postconditions(){return At(8);}bool Rollback(){++rollback;return restored;}
 };
 for(unsigned fail=0;fail<=8;++fail){Host host;host.fail=fail;unsigned reason=99;
  const auto result=cfghostedfill::AddOne(host,reason);
  check(result==(fail?cfghostedfill::Result::Refused:cfghostedfill::Result::Completed),"one birth result");
  check(host.spawns==(fail==1?0u:1u)&&host.rollback==(fail>=3?1u:0u),"no preflight mutation or repeated dispatched spawn");
  check(reason==fail,"exact failure stage retained");
 }
 {Host host;host.fail=2;host.clean=false;unsigned reason=0;check(cfghostedfill::AddOne(host,reason)==cfghostedfill::Result::Quarantined&&host.spawns==1&&!host.rollback,"ambiguous birth never replayed or blindly destroyed");}
 {Host host;host.fail=3;host.restored=false;unsigned reason=0;check(cfghostedfill::AddOne(host,reason)==cfghostedfill::Result::Quarantined&&host.spawns==1&&host.rollback==1,"unproved own-controller cleanup quarantines");}
 std::printf("H22 current-host counselor admission/native-birth ordering/no-replay/rollback: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
