#define main ExistingBankFixtureMain
#include "ResidentRootBankTests.cpp"
#undef main
#include "CallbackRootLease.hpp"
int main(){ExistingBankFixtureMain();using Bank=cfgretain::ResidentRootBank<2>;
 const auto classify=[](std::uintptr_t value){return value==0x20000?cfgretain::MapReference::OldMap:
  value==0x30000?cfgretain::MapReference::Unrelated:cfgretain::MapReference::Unproven;};
 {Bank bank;Registry registry;Bank::Reservation reserve;std::array<Bank::Handle,2> handles{};
  CHECK(bank.ReserveOutputs(registry,0x100000,2,reserve));
  CHECK(bank.CommitFreshReturns(registry,reserve,std::array<std::uintptr_t,2>{0x20000,0x30000},handles));
  bank.MarkUncertain();unsigned cleared=0;
  CHECK(!bank.Healthy()&&bank.RecoverMapReferencesAtMenu(registry,classify,cleared)&&cleared==1&&bank.Healthy()&&bank.RecoveryEpoch()==1);
  std::uintptr_t object=0;CHECK(!bank.ResolveRequired(registry,handles[0],object)&&!bank.ResolveRequired(registry,handles[1],object));
  std::array<std::uintptr_t,64> slots{};unsigned count=0;CHECK(bank.ObserveSlots(slots,count)&&!slots[0]&&slots[1]==0x30000);
  CHECK(bank.ReserveOutputs(registry,0x100000,2,reserve));std::array<Bank::Handle,2> next{};
  CHECK(bank.CommitFreshReturns(registry,reserve,std::array<std::uintptr_t,2>{0x20000,0x30000},next));
  CHECK(bank.ResolveRequired(registry,next[0],object)&&object==0x20000&&!bank.ResolveRequired(registry,handles[0],object));}
 for(unsigned fault=0;fault<4;++fault){Bank bank;Registry registry;Bank::Reservation reserve;std::array<Bank::Handle,2> handles{};
  CHECK(bank.ReserveOutputs(registry,0x100000,2,reserve));
  CHECK(bank.CommitFreshReturns(registry,reserve,std::array<std::uintptr_t,2>{0x20000,fault==0?std::uintptr_t{0x40000}:std::uintptr_t{0x30000}},handles));
  if(fault==1)CHECK(bank.ReserveOutputs(registry,0x100000,1,reserve));
  if(fault==2)registry.current=false;
  if(fault==3)reinterpret_cast<cfgretain::SynchronizedRootOwner*>(bank.Address(0))->unknownSubmit.store(1);
  bank.MarkUncertain();unsigned cleared=0;CHECK(!bank.RecoverMapReferencesAtMenu(registry,classify,cleared)&&!cleared&&bank.Poisoned()&&!bank.RecoveryEpoch());
  const auto& owner=*reinterpret_cast<cfggcretain::Owner*>(bank.Address(0));CHECK(owner.slots[0]==0x20000);}
 {Bank bank;Registry registry;using Lease=cfgretain::CallbackRootLease<Bank>;
  Lease::DeferredRetirement pending;Lease lease(bank,0x100000);
  CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&lease.TakeIdleClaims(pending));
  bank.MarkUncertain();CHECK(!pending.DiscardAfterMenuRecovery(bank,0x100000));unsigned released=0;
  CHECK(bank.RecoverMapReferencesAtMenu(registry,classify,released)&&released==1);
  CHECK(pending.DiscardAfterMenuRecovery(bank,0x100000)&&!pending.Claims());}
 std::printf("Poisoned bank menu recovery: %u failures; old-map cleared, metadata preserved, every old handle invalid, match2 reusable; unknown/reserved/unproved-window/unknown-collector refused.\n",failures);
 return failures?1:0;
}
