#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG_J214_HUMAN_JASON_OPTIONS 1
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#define CFG_HUMAN_DIFFICULTY_TEST_MAIN ExistingDifficultyChecks
#include "NativeHumanJasonDifficultyTests.cpp"
int main(){return ExistingDifficultyChecks();}
