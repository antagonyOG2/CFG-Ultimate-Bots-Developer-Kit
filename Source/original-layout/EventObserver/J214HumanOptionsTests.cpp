// Compile PrimaryLookupEligibility BEFORE legacy fixture data can define its
// unverified-runtime escape hatch. This reproduces the real no-scope stage 2.
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG_J214_HUMAN_JASON_OPTIONS 1
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_HUMAN_OPTIONS_TEST_MAIN ExistingOptionsChecks
#include "NativeHumanJasonOptionsTests.cpp"
int main(){
 Fixture f;cfgmenu::promotion::Context owner;unsigned reads=0;
 const bool strict=cfgmenu::offlineowner::Observe(f.r,f.source,f.Mode,42,true,owner,&reads);
 if(cfgprimary::LookupSafe(f.r,f.Module)||!reads){std::printf("FAIL production scoped lookup separation\n");return 1;}
 std::printf("Production strict owner refused after %u reads; no retained native capability granted.\n",reads);
 const bool typed=cfgmenu::offlineowner::Observe(f.r,f.source,f.Mode,42,true,owner,nullptr,nullptr,
   cfgmenu::offlineowner::Purpose::TypedOwnValueOptions);
 if(strict||!typed){std::printf("FAIL strict/typed options separation\n");return 1;}
 if(ExistingOptionsChecks())return 1;
 std::printf("J214 production LookupSafe stays closed; own typed options reach actual read/CAS and timer/skill paths.\n");return 0;
}
