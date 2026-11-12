#define CFG_J22_ALL_SOUND_BLIPS 1
#define CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
#include "AdditionalJasonBirthRuntimeTests.cpp"
int main(){unsigned checks=0,failed=0;const auto check=[&](bool x){++checks;if(!x)++failed;};
 for(unsigned world=0;world<2;++world){BirthFixture f;f.nativeHunting=true;f.soundPending=true;f.bundleAdditional.policy.stage=cfgadditional::Stage::Active;
  f.Fresh();check(f.AdditionalPhase(1000));check(f.soundTurns==1&&!f.legacyDrives&&!f.soundPending);
  f.Fresh();check(f.AdditionalPhase(1100));check(f.soundTurns==1&&!f.legacyDrives);
 }
 std::printf("J22 actual F1 lifecycle sound path: %u checks,%u failures; native hunting ON, one visibility clear per world, no legacy AI drive. Owned mocks.\n",checks,failed);return failed?1:0;
}
