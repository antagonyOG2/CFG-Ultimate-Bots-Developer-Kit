#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../Core/HostedMaskTriggerPolicy.hpp"
#include "OfflineContextObservation.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#define CFG_H1_JASON 1
using Reader=OptionsReader;
bool hostActive=true;
bool HostedRoutineActive(){return hostActive;}
struct InventoryProbe {
 Reader& r_;struct {std::uintptr_t module=MenuFixture::Base;}plan;
 static constexpr std::uintptr_t Pawn=0x1600000,Inventory=0x1601000,Items=0x1602000;
 bool nativeCache=false,typed=true;
 bool Known(unsigned,std::uintptr_t& pawn){pawn=Pawn;return true;}
 bool NativeClass(std::uintptr_t,std::uintptr_t,std::uintptr_t& type){type=nativeCache?MenuFixture::DonorType:0;return nativeCache;}
 bool TypedFeature(std::uintptr_t,unsigned,bool){return typed;}
#include "HostedMaskTriggerMembers.hpp"
#include "PamelaTriggerInventoryMembers.hpp"
};
int main(){NativeFixture f;OptionsReader r{f.fixture.r};InventoryProbe p{r};auto& memory=f.fixture.r;
 using namespace cfgfeature::disk;using P=InventoryProbe;unsigned checks=0,bad=0;
 const auto check=[&](bool ok){++checks;if(!ok){++bad;std::printf("FAIL mask inventory %u\n",checks);}};
 memory.Set(P::Pawn+HasSweater,std::uint8_t{});memory.Set(P::Pawn+AbilityField,std::uintptr_t{});
 memory.Set(P::Pawn+SpecialInventory,P::Inventory);memory.Set(P::Pawn+SmallInventory,std::uintptr_t{});
 memory.Set(P::Inventory+0x20,P::Pawn);memory.Set(P::Inventory+0xE0,P::Pawn);
 struct List{std::uintptr_t data;int count,capacity;};memory.Set(P::Inventory+InventoryItems,List{P::Items,1,1});
 memory.Set(P::Items,MenuFixture::Donor);memory.Set(MenuFixture::Donor+OuterInventory,P::Inventory);
 f.fixture.Name(MenuFixture::DonorType,"2009_MaskPickup_C");hostActive=true;bool earned=false;
 check(p.HasPamelaTrigger(earned)&&earned); // Actual function, cold native cache.
 f.fixture.Name(MenuFixture::DonorType,"InventoryPickup_C");f.fixture.Name(MenuFixture::Donor,"2009_MaskPickup_C");
 check(p.HasPamelaTrigger(earned)&&earned);
 f.fixture.Name(MenuFixture::Donor,"FirstAidSpray_0");check(p.HasPamelaTrigger(earned)&&!earned);
 f.fixture.Name(MenuFixture::Donor,"2009_MaskPickup_C");
 memory.Set(MenuFixture::Donor+OuterInventory,P::Inventory+8);check(!p.HasPamelaTrigger(earned)&&!earned);
 memory.Set(MenuFixture::Donor+OuterInventory,P::Inventory);
 memory.changeAt=P::Items;memory.changeSlot=P::Pawn+SpecialInventory;memory.newPointer=P::Inventory+8;
 check(!p.HasPamelaTrigger(earned)&&!earned);memory.changeSlot=0;memory.Set(P::Pawn+SpecialInventory,P::Inventory);
 p.typed=false;check(!p.HasPamelaTrigger(earned)&&!earned);p.typed=true;
 hostActive=false;check(!p.HasPamelaTrigger(earned)&&!earned);
 p.nativeCache=true;check(p.HasPamelaTrigger(earned)&&earned); // Original offline native-cache route.
 // Two complete use/rearm/HUD cycles earned from the actual held-mask path.
 hostActive=true;p.nativeCache=false;cfgsweater::Policy policy;bool trigger=false;
 check(p.HasPamelaTrigger(trigger)&&trigger);
 check(policy.Decide(100,trigger,false,false)==cfgsweater::Action::Discover);
 check(policy.Decide(200,trigger,false,true)==cfgsweater::Action::Publish);
 policy.BeginPublish();policy.QueueHud(200,false);policy.ConfirmPublish();
 for(const auto usedAt:{1000ULL,14000ULL}){
  check(policy.ObserveUsed(usedAt,1)&&policy.stage==cfgsweater::Stage::Waiting);
  check(p.HasPamelaTrigger(trigger)&&trigger);
  check(policy.Decide(usedAt+10999,trigger,false,true)==cfgsweater::Action::None);
  check(policy.Decide(usedAt+11000,trigger,false,true)==cfgsweater::Action::Rearm);
  policy.BeginPublish();policy.QueueHud(usedAt+11000,true);
  check(policy.Decide(usedAt+11001,trigger,true,true)==cfgsweater::Action::PublishHud);
  policy.ConfirmPublish();check(policy.stage==cfgsweater::Stage::Owned);
 }
 policy.Refuse();check(policy.Decide(30000,true,false,true)==cfgsweater::Action::None);
 std::printf("H220 actual inventory + recharge policy: %u checks, %u failures; cold-cache hosted mask and owned-list mutations, offline preserved.\n",checks,bad);return bad?1:0;
}


