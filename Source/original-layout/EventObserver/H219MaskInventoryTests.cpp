#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../Core/HostedMaskTriggerPolicy.hpp"
#include "../Core/HostedMaskTriggerMemo.hpp"
#include "OfflineContextObservation.hpp"
#define CFG_H1_JASON 1
using Reader=OptionsReader;
bool hostActive=true;
bool HostedRoutineActive(){return hostActive;}
struct InventoryProbe {
 Reader& r_;struct {std::uintptr_t module=MenuFixture::Base,world=0x1603000;}plan;
 struct {cfgprobe::lifetime::ArrayHeader objectArray{};}context;
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
 p.context.objectArray={0x1610000,4};
 const auto putSlot=[&](std::uintptr_t object,int index){
  memory.Set(object+cfgprobe::lifetime::ObjectIndexOffset,index);
  memory.Set(0x1610000+24*index+cfgprobe::lifetime::ItemObjectOffset,object);
  memory.Set(0x1610000+24*index+cfgprobe::lifetime::ItemSerialOffset,index+1);
  memory.Set(0x1610000+24*index+8,std::uint32_t{});
 };
 putSlot(P::Inventory,0);putSlot(MenuFixture::Donor,1);putSlot(MenuFixture::DonorType,2);putSlot(P::Pawn,3);
 f.fixture.Name(MenuFixture::DonorType,"2009_MaskPickup_C");hostActive=true;bool earned=false;
 check(p.HasPamelaTrigger(earned)&&earned); // Actual function, cold native cache.
 const auto coldReads=r.reads;const auto cachedMemo=cfgh241mask::memo;
 check(cachedMemo.world==p.plan.world&&cachedMemo.pawn==P::Pawn&&cachedMemo.item==MenuFixture::Donor);
 check(p.HasPamelaTrigger(earned)&&earned&&r.reads-coldReads<100); // slot/list reproof, no inventory/class walk
 f.fixture.Name(MenuFixture::DonorType,"InventoryPickup_C");f.fixture.Name(MenuFixture::Donor,"2009_MaskPickup_C");
 check(p.HasPamelaTrigger(earned)&&earned);
 f.fixture.Name(MenuFixture::Donor,"FirstAidSpray_0");check(p.HasPamelaTrigger(earned)&&earned); // same cached live item identity
 f.fixture.Name(MenuFixture::Donor,"2009_MaskPickup_C");
 memory.Set(MenuFixture::Donor+OuterInventory,P::Inventory+8);check(!p.HasPamelaTrigger(earned)&&!earned);
 memory.Set(MenuFixture::Donor+OuterInventory,P::Inventory);
 memory.changeAt=P::Items;memory.changeSlot=P::Pawn+SpecialInventory;memory.newPointer=P::Inventory+8;
 check(!p.HasPamelaTrigger(earned)&&!earned);memory.changeSlot=0;memory.Set(P::Pawn+SpecialInventory,P::Inventory);
 p.typed=false;check(!p.HasPamelaTrigger(earned)&&!earned);p.typed=true;
 hostActive=false;check(!p.HasPamelaTrigger(earned)&&!earned);
 p.nativeCache=true;check(p.HasPamelaTrigger(earned)&&earned); // Original offline native-cache route.
 std::printf("H219 actual inventory trigger: %u checks, %u failures; cold-cache hosted mask and owned-list mutations, offline preserved.\n",checks,bad);return bad?1:0;
}

