#define CFG_PRIMARY_RETIREMENT_MAIN PrimaryPoolTests
#include "PrimaryRootRetirementTests.cpp"
#include "ResidentRootBank.hpp"
#include "WorldRootInventory.hpp"
struct TwoHolderRegistry:RemovalRegistry {bool held=true;bool ProvenSlotWindow()const noexcept{return held;}};
struct ResidentReader {
 template<class T>bool Read(std::uintptr_t at,T& out)noexcept{std::memcpy(&out,reinterpret_cast<void*>(at),sizeof(out));return true;}
};
int main(){PrimaryPoolTests();using Bank=cfgretain::ResidentRootBank<1>;
 cfgretain::PermanentRootPool<1> primary;Bank startup;TwoHolderRegistry registry;cfggcprobe::Report report;
 const std::uintptr_t world=0x40000,level=0x50000,package=0x60000,metadata=0x90000;
 CHECK(primary.AddLocked(registry,std::array<std::uintptr_t,2>{world,level},report));
 Bank::Reservation reservation;std::array<Bank::Handle,3> handles{};
 CHECK(startup.ReserveOutputs(registry,0x100000,3,reservation));
 CHECK(startup.CommitFreshReturns(registry,reservation,std::array<std::uintptr_t,3>{world,package,metadata},handles));
 const std::array<cfgretain::NamedRootOwner,2> named{{{primary.Address(0),"primaryRoots","PermanentRootPool",0},
  {startup.Address(0),"startupRootBank","SynchronizedRootOwner",0}}};
 ResidentReader reader;unsigned messages=0,worldOwners=0,rootReferences=0;
 const auto classify=[&](std::uintptr_t object){return object==world||object==level||object==package?
  cfgretain::MapReference::OldMap:cfgretain::MapReference::Unrelated;};
 const auto emit=[&](const auto& owner,std::uintptr_t object,cfgretain::MapReference){
  CHECK(owner.address&&owner.name&&owner.type);if(object==world)++worldOwners;++messages;};
 // Name/type/address + both World owners captured before the first cleanup.
 const auto removals=registry.removes;for(auto owner:named){unsigned held=0;
  CHECK(cfgretain::InspectNamedRootOwner(reader,owner,classify,emit,held));rootReferences+=held;}
 CHECK(worldOwners==2&&rootReferences==4&&messages==6&&registry.removes==removals);
 unsigned cleared=0;registry.held=false;std::uintptr_t failedOwner=42;
 CHECK(!startup.RetireMapReferences(registry,classify,cleared,&failedOwner)&&!cleared&&!failedOwner);
 registry.held=true;Bank::Reservation pending;CHECK(startup.ReserveOutputs(registry,0x100000,1,pending));
 CHECK(!startup.RetireMapReferences(registry,classify,cleared)&&!cleared);CHECK(startup.CancelUnpublished(registry,pending));
 CHECK(!startup.RetireMapReferences(registry,[](auto){return cfgretain::MapReference::Unproven;},cleared)&&!cleared);
 CHECK(startup.RetireMapReferences(registry,classify,cleared)&&cleared==2);
 CHECK(primary.ReleaseAllLocked(registry,report)&&report.removed==1);
 worldOwners=messages=rootReferences=0;
 for(auto owner:named){unsigned held=0;CHECK(cfgretain::InspectNamedRootOwner(reader,owner,classify,emit,held));rootReferences+=held;}
 CHECK(!worldOwners&&!rootReferences&&registry.list.count==2&&registry.entries[0]==0x20000);
 std::uintptr_t resolved=0;CHECK(startup.ResolveRequired(registry,handles[2],resolved)&&resolved==metadata);
 CHECK(!startup.ResolveRequired(registry,handles[0],resolved)&&!startup.ResolveRequired(registry,handles[1],resolved));
 // Persistent metadata owner remains reusable; old handles cannot resurrect map.
 CHECK(startup.ReserveOutputs(registry,0x100000,1,reservation));std::array<Bank::Handle,1> next{};
 CHECK(startup.CommitFreshReturns(registry,reservation,std::array<std::uintptr_t,1>{0xA0000},next));
 CHECK(startup.ResolveRequired(registry,next[0],resolved)&&resolved==0xA0000);
 for(unsigned cycle=1;cycle<=10;++cycle){
  const std::uintptr_t departed=world+static_cast<std::uintptr_t>(cycle)*0x100000;
  CHECK(primary.AddLocked(registry,std::array<std::uintptr_t,1>{departed},report));
  CHECK(startup.ReserveOutputs(registry,0x100000,1,reservation));std::array<Bank::Handle,1> born{};
  CHECK(startup.CommitFreshReturns(registry,reservation,std::array<std::uintptr_t,1>{departed},born));
  const auto classifyEpoch=[&](std::uintptr_t object){return object==departed?cfgretain::MapReference::OldMap:cfgretain::MapReference::Unrelated;};
  registry.held=false;CHECK(!startup.RetireMapReferences(registry,classifyEpoch,cleared));registry.held=true;
  CHECK(startup.RetireMapReferences(registry,classifyEpoch,cleared)&&cleared==1);
  CHECK(primary.ReleaseAllLocked(registry,report));
  CHECK(!startup.ResolveRequired(registry,born[0],resolved));
  CHECK(startup.ResolveRequired(registry,handles[2],resolved)&&resolved==metadata);
  CHECK(startup.ResolveRequired(registry,next[0],resolved)&&resolved==0xA0000);
 }
 std::printf("Two-holder World retirement: %u failures; named pre-release inventory,10 consecutive reuse/retire cycles,both holders retired, foreign owner/metadata preserved, reservations/window/unknown refusals.\n",failures);
 return failures?1:0;
}
