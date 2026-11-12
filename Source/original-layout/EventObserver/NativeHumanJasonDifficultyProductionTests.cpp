// Reuse the complete existing production refusal suite; add actual TakeDamage
// detour forwarding checks without inventing a live owner or installing hooks.
#define main CfgUrinationProductionMain
#include "UrinationNativeProductionTests.cpp"
#undef main
namespace {
unsigned hitForwards=0,hitFailures=0;
float __fastcall OwnedDamageForward(void* victim,float damage,void* event,void* instigator,void* causer){
 ++hitForwards;
 if(victim!=reinterpret_cast<void*>(0x100000)||damage!=20.f||event!=reinterpret_cast<void*>(0x200000)||
    instigator!=reinterpret_cast<void*>(0x300000)||causer!=reinterpret_cast<void*>(0x400000))++hitFailures;
 return 25.f;
}
}
int main(){const int earlier=CfgUrinationProductionMain();
 const auto saved=bundleDamageOriginal;const auto calls=gcRegistryReport.calls;
 bundleStarted=false;bundleDamageOriginal=&OwnedDamageForward;
 if(FrontendHumanJasonDifficultyDamage(0x100000,20.f,0x300000,0x400000)!=20.f)++hitFailures;
 const float returned=BundleNativeCounselorDamage(reinterpret_cast<void*>(0x100000),20.f,
     reinterpret_cast<void*>(0x200000),reinterpret_cast<void*>(0x300000),reinterpret_cast<void*>(0x400000));
 if(returned!=25.f||hitForwards!=1||gcRegistryReport.calls!=calls||bundleRootLease||!bundleRootBank.Healthy())++hitFailures;
 bundleDamageOriginal=saved;
 std::printf("Actual human-Jason production hit routing: %u failures; unknown owner preserves damage/all5arguments, original forwarded once, no native roots/hooks. Owned process only.\n",hitFailures);
 return earlier||hitFailures?1:0;
}
