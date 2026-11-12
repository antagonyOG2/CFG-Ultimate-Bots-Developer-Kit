#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../Core/HostedMaskTriggerPolicy.hpp"
#define CFG_H1_JASON 1
using Reader=OptionsReader;
bool hostActive=true;
bool HostedRoutineActive(){return hostActive;}
struct MaskProbe {
 Reader& r_;struct {std::uintptr_t module=MenuFixture::Base;}plan;
 bool typed=true;
 bool TypedFeature(std::uintptr_t,unsigned id,bool){return typed&&id==cfgfeature::disk::Item;}
#include "HostedMaskTriggerMembers.hpp"
};
int main(){NativeFixture f;OptionsReader reader{f.fixture.r};MaskProbe p{reader};unsigned n=0,bad=0;
 auto check=[&](bool okay){++n;if(!okay)++bad;};
 for(const char* name:{"2009_MaskPickup_C","Part7_Mask_Pickup_C","SCKillerMask"}){
  f.fixture.Name(MenuFixture::DonorType,name);check(p.HostedMaskPickup(MenuFixture::Donor));}
 f.fixture.Name(MenuFixture::DonorType,"FirstAidSpray_C");check(!p.HostedMaskPickup(MenuFixture::Donor));
 f.fixture.Name(MenuFixture::DonorType,"InventoryPickup_C");
 f.fixture.Name(MenuFixture::Donor,"2009_MaskPickup_C");check(p.HostedMaskPickup(MenuFixture::Donor));
 f.fixture.Name(MenuFixture::Donor,"FirstAidSpray_0");check(!p.HostedMaskPickup(MenuFixture::Donor));
 f.fixture.Name(MenuFixture::DonorType,"2009_MaskPickup_C");p.typed=false;check(!p.HostedMaskPickup(MenuFixture::Donor));
 p.typed=true;hostActive=false;check(!p.HostedMaskPickup(MenuFixture::Donor));
 std::printf("H212 actual donor hosted mask classifier: %u checks, %u failures; native item-type seam, real class-name reads, offline refuses. Inventory owner/list reproof remains in HasPamelaTrigger.\n",n,bad);return bad?1:0;
}
