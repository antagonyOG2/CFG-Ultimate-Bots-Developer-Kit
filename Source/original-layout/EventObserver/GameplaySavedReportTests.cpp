#define _CRT_SECURE_NO_WARNINGS
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
#include <cstring>
int main(){using namespace cfgbundle;unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    static_assert(offsetof(Report,initializationStage)==4424,"Saved report57 prefix must remain exact");
    std::array<unsigned char,4432> saved{};
    auto* file=std::fopen("Tests/EventObserver/Fixtures/PID14472-Report57.bin","rb");if(!file)return 1;
    check(std::fread(saved.data(),1,saved.size(),file)==saved.size());check(std::fgetc(file)==EOF);std::fclose(file);
    Report r{};std::memcpy(&r,saved.data(),offsetof(Report,initializationStage)+sizeof(r.initializationStage));
    check(r.magic==ReportMagic&&r.version==57&&r.bytes==saved.size());
    check(!Valid(r)); // runtime NEVER accepts a historical schema
    r.version=ReportVersion;r.bytes=sizeof(r); // owned replay fixture only, no game
    check(Valid(r)&&r.initializationStage==29&&r.state==1&&r.phases==152&&r.moves==0&&r.botNameFailure==9&&r.endingRepairFailure==27);
    ++r.botNameFailure;check(!Valid(r));--r.botNameFailure;r.endingRepairFailure=28;check(!Valid(r));
    r.endingRepairFailure=27;r.rootPoisoned=2;check(!Valid(r));r.rootPoisoned=0;r.rootLastFailure=7;check(!Valid(r));
    r.rootLastFailure=0;r.rootReferencerFailure=4;r.rootStrandedClaims=1024;check(Valid(r));
    r.rootReferencerFailure=5;check(!Valid(r));r.rootReferencerFailure=0;r.rootStrandedClaims=1025;check(!Valid(r));
    r.rootStrandedClaims=0;r.knifeBoundaryCount=1;r.knifeBoundaries[0]={0x1000,0x2000,1,7,9,1};
    r.actorRetentionPeakReads=512*CallCap;check(Valid(r));
    ++r.actorRetentionPeakReads;check(!Valid(r));r.actorRetentionPeakReads=0;
    r.knifeBoundaries[0].returnPc=0;check(!Valid(r));r.knifeBoundaries[0].returnPc=0x1000;
    r.knifeBoundaries[0].subject=0;check(!Valid(r));r.knifeBoundaries[0].subject=0x2000;
    r.knifeBoundaries[0].flags=8;check(!Valid(r));r.knifeBoundaries[0].flags=0;
    r.knifeBoundaries[0].kind=3;check(!Valid(r));r.knifeBoundaries[0].kind=2;
    r.knifeBoundaries[0].count=0;check(!Valid(r));r.knifeBoundaries[0].count=1;
    r.knifeBoundaryCount=9;check(!Valid(r));r.knifeBoundaryCount=1;check(Valid(r));
    r.knifeAdapterStage=8;r.knifeAdapterFailure=6;r.knifeAdapterReads=ReadCap;r.knifeAdapterEntryReads=ReadCap;r.knifeAdapterCalls=CallCap;check(Valid(r));
    ++r.knifeAdapterStage;check(!Valid(r));--r.knifeAdapterStage;++r.knifeAdapterFailure;check(!Valid(r));--r.knifeAdapterFailure;
    ++r.knifeAdapterReads;check(!Valid(r));--r.knifeAdapterReads;++r.knifeAdapterCalls;check(!Valid(r));--r.knifeAdapterCalls;
    ++r.knifeAdapterEntryReads;check(!Valid(r));--r.knifeAdapterEntryReads;check(Valid(r));
    r.rootDeferredRetirements=1;r.rootDrainedRetirements=1;r.rootDeferredClaims=1024;check(Valid(r));
    ++r.rootDeferredClaims;check(!Valid(r));--r.rootDeferredClaims;++r.rootDrainedRetirements;check(!Valid(r));--r.rootDrainedRetirements;check(Valid(r));
    r.knifeNativePostSpeed=-1;check(!Valid(r));r.knifeNativePostSpeed=0;check(Valid(r));
    r.f3Polls=2;r.f3Foreground=1;r.f3DownSamples=1;r.f3ShortSamples=1;check(Valid(r));
    ++r.f3Foreground;++r.f3Foreground;check(!Valid(r));r.f3Foreground=1;
    ++r.f3DownSamples;check(!Valid(r));--r.f3DownSamples;
    ++r.f3ShortSamples;check(!Valid(r));--r.f3ShortSamples;check(Valid(r));
    r.sweaterUseEdgeSource=2;r.sweaterUses=2;r.sweaterOwnershipUses=1;r.sweaterAbilityUses=1;check(Valid(r));
    ++r.sweaterUseEdgeSource;check(!Valid(r));--r.sweaterUseEdgeSource;
    r.sweaterOwnershipUses=3;check(!Valid(r));r.sweaterOwnershipUses=1;
    r.sweaterAbilityUses=3;check(!Valid(r));r.sweaterAbilityUses=1;check(Valid(r));
    r.sweaterClassAnchor=8;r.sweaterClassFailure=33;check(Valid(r));
    ++r.sweaterClassFailure;check(!Valid(r));--r.sweaterClassFailure;r.sweaterClassAnchor=10;check(!Valid(r));
    std::printf("Saved PID14472 replay, disabled-feature diagnostics, root-report, knife-witness, lexical-adapter, deferred-retirement and tools/input bounds: %u failures; no game access.\n",failures);return failures?1:0;}
