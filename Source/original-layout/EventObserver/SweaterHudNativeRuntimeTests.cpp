#define WIN32_LEAN_AND_MEAN
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include <windows.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include "../RuntimeProbe/SweaterHudBindings.generated.hpp"
namespace cfgsweater {struct HudEvidence {struct Slot{std::uintptr_t object=0;}hud,widget;};}
namespace {
unsigned failures=0,nativeCalls=0;bool nativeFault=false;void* receiver=nullptr;
void __fastcall Native(void* object,bool available){
    ++nativeCalls;receiver=object;if(!available)++failures;
    if(nativeFault)RaiseException(0xE0000001,0,0,nullptr);
}
struct {std::uintptr_t world=0x10000,mode=0x20000,controller=0x30000,module=0;}plan;
struct {std::uintptr_t gameState=0x40000;}spawnAssets;
struct {std::uintptr_t object=0x50000;}bundleSweaterOwner;
struct {std::uintptr_t object=0x60000;}bundleSweaterAbility;
struct {unsigned hudAttempts=0,begins=0;void BeginPublish(){++begins;}}bundleSweaterPolicy;
std::array<std::uintptr_t,8> roots{};
bool retained=true;
template<class R,std::size_t N>bool RetainPrimaryObjects(R&,const std::array<std::uintptr_t,N>& input){roots=input;return retained;}
struct Fixture {
    int r_=0;unsigned calls_=0,begins=0,finishes=0,poisons=0;
    bool allowed=true,begin=true,finish=true,exception_=false;
    bool NativeCallAllowed(){return allowed&&calls_<12;}
    bool BeginBundleNative(){++begins;return begin;}
    bool FinishBundleNative(){++finishes;return finish;}
    void BundleRetentionFailed(){++poisons;}
#include "SweaterHudNativeRuntimeMembers.hpp"
};
void Check(bool okay){if(!okay)++failures;}
}
int main(){
    plan.module=reinterpret_cast<std::uintptr_t>(&Native)-cfgsweater::huddisk::Available;
    const cfgsweater::HudEvidence hud{{0x70000},{0x80000}};
    {Fixture f;Check(f.SweaterHudNative(hud));Check(nativeCalls==1&&receiver==reinterpret_cast<void*>(hud.hud.object));
     Check(f.calls_==1&&f.begins==1&&f.finishes==1&&!f.poisons);
     Check(roots==std::array<std::uintptr_t,8>{plan.world,plan.mode,spawnAssets.gameState,plan.controller,
        bundleSweaterOwner.object,bundleSweaterAbility.object,hud.hud.object,hud.widget.object});}
    for(unsigned fault=0;fault<4;++fault){Fixture f;const auto before=nativeCalls;
        if(!fault)f.allowed=false;else if(fault==1)retained=false;else if(fault==2)f.begin=false;else f.calls_=12;
        Check(!f.SweaterHudNative(hud));Check(nativeCalls==before&&f.finishes==0);retained=true;}
    {Fixture f;f.finish=false;Check(!f.SweaterHudNative(hud));Check(f.calls_==1&&f.finishes==1);}
    {Fixture f;nativeFault=true;Check(!f.SweaterHudNative(hud));Check(f.exception_&&f.poisons==1&&f.finishes==0);nativeFault=false;}
    std::printf("Actual sweater HUD native wrapper: %u failures; exact8 roots, admission/budget/refusal/SEH guards, no game access.\n",failures);
    return failures?1:0;
}
