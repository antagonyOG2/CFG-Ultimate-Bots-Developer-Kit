namespace cfgspawn {constexpr unsigned MaximumNativeCalls=31;} // fixture retains production31-call bound
#define CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
#include "AdditionalJasonBirthRuntimeTests.cpp"
#undef CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
struct SelectionFixture:BirthFixture {
    std::array<std::uintptr_t,3> choices{};cfgspawn::TemplateObservation primaryTemplate{};
    bool hosted=false;bool HostedRoutineActive()const{return hosted;}
    bool cold=false;unsigned conversions=0,driveTicks=0;bool exception_=false;
    cfgobserver::Plan& current_=context;
    bool ContextMatches(){return Context();}bool ResourcesMatch(){return Jason();}
    bool Refresh(){return AdditionalWorld();}
#include "SharedConversionRuntimeMembers.hpp"
    bool NativeAdditionalHuntingActive(){return false;}
    bool AdditionalDrive(std::uint64_t){++driveTicks;return AdditionalLinked();}
#include "AdditionalJasonLifecycleMembers.hpp"
    const cfgspawn::TemplateObservation& JasonTemplate(){return primaryTemplate;}
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* value,bool actor){
        CHECK(!actor&&object==spawnAssets.kismetDefault&&function==spawnAssets.conversionFunction);
        auto& p=*static_cast<cfgprimary::ConversionParameters*>(value);++conversions;
        const auto i=p.softClass[16];p.result=!cold&&i>=1&&i<=choices.size()?choices[i-1]:0;return true;
    }
#include "AdditionalJasonSelectionMembers.hpp"
    SelectionFixture(){
        r_.cap=100000;choices[1]=bundleAdditional.selected.pawnClass;
        choices[2]=r_.Object();const auto defaults=r_.Object(choices[2]);
        r_.Put(choices[2]+0x30,spawnAssets.killerBase);r_.Put(choices[2]+cfgspawn::ClassDefaultObject,defaults);
        r_.Put(defaults+cfgprobe::ObjectClass,choices[2]);
        r_.Put(defaults+cfgspawn::PawnAIClass,spawnAssets.resources.classes[cfgspawn::BaseAI]);
        r_.Put(defaults+cfgspawn::CharacterTree,std::uintptr_t{});Serial(choices[2]);Serial(defaults);
        // The donor catalogue can contain the already-selected Jason class.
        // Give the fixture's primary entry the same fully proven template.
        choices[0]=choices[2];primaryTemplate.pawnClass=choices[0];
        spawnAssets.kismetClass=r_.Object();spawnAssets.kismetDefault=r_.Object(spawnAssets.kismetClass);
        r_.Put(spawnAssets.kismetClass+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault);
        spawnAssets.conversionFunction=r_.Object();r_.Put(spawnAssets.conversionFunction+0x20,spawnAssets.kismetClass);
        r_.Put(plan.module+cfgprimary::disk::Conversion.cache,spawnAssets.conversionFunction);
        r_.Put(plan.module+cfgprimary::disk::Conversion.factory,cfgprimary::disk::Conversion.prefix);
        PublishHeader();
        CHECK(cfgspawn::ObjectSlot(r_,r_.objects,spawnAssets.conversionFunction,spawnAssets.conversionSlot));
        spawnAssets.primary.catalogue.count=spawnAssets.primary.catalogue.capacity=3;
        r_.Put(plan.mode+cfgprimary::KillerClasses,spawnAssets.primary.catalogue);
        for(unsigned i=0;i<3;++i){spawnAssets.primary.entries[i][16]=static_cast<unsigned char>(i+1);
            r_.Put(spawnAssets.primary.catalogue.data+40ULL*i,spawnAssets.primary.entries[i]);}
        bundleAdditional.selectedIndex=~0u;bundleAdditional.selected={};bundleAdditional.journal={};Fresh();
    }
    unsigned Step(){Fresh();const auto value=AdditionalSelect();CHECK(r_.reads<=1536&&calls_<=12&&!allocations);return value;}
};
 int main(){SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::setvbuf(stdout,nullptr,_IONBF,0);unsigned winners[3]{};
 for(unsigned seed=1;seed<=40;++seed){SelectionFixture f;f.bundleAdditional.policy.edgeAt=seed;
  // Entries 0 and 2 resolve to the current Jason; entry 1 is the only distinct class.
  CHECK(f.Step()==0&&f.bundleAdditional.policy.choices==0&&f.bundleAdditional.sameClassFallbackIndex==0); // current Jason kept as fallback only
  CHECK(f.Step()==0&&f.bundleAdditional.policy.choices==1);
  CHECK(f.Step()==0&&f.bundleAdditional.policy.choices==1);
  CHECK(f.Step()==1&&f.conversions==3&&f.bundleAdditional.selectedIndex==1); // distinct class preferred
  if(f.bundleAdditional.selectedIndex<=2)++winners[f.bundleAdditional.selectedIndex];
  else std::printf("Selection fixture index=%u choices=%u conversions=%u\n",f.bundleAdditional.selectedIndex,f.bundleAdditional.policy.choices,f.conversions);}
 CHECK(winners[1]==40&&!winners[0]&&!winners[2]);
 {SelectionFixture f;f.cold=true;CHECK(!f.Step()&&!f.Step()&&!f.Step()&&f.Step()==1&&!f.allocations);
  // H2.47: null conversions for every entry -> selection is the primary template.
  CHECK(f.bundleAdditional.selectionRefusal==0&&f.bundleAdditional.selected.pawnClass==f.primaryTemplate.pawnClass&&f.bundleAdditional.selectedIndex==0);}
 {SelectionFixture f;f.cold=true;f.primaryTemplate.pawnClass=0;CHECK(!f.Step()&&!f.Step()&&!f.Step()&&f.Step()==2&&f.bundleAdditional.selectionRefusal==1);}
 {// H2.48: hosted, null primary template -> adopted Jason's own pawn class is selected.
  SelectionFixture f;f.cold=true;f.hosted=true;f.primaryTemplate.pawnClass=0;
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);
#if CFG_H293_HOSTED_STALE_CATALOGUE
  // H2.93: hosted never captures the catalogue, so one present here is a finished offline match's leftover. It is ignored:
  // the first step selects the adopted Jason's own resident class (no conversions of the stale entries).
  CHECK(f.Step()==1&&f.bundleAdditional.hostedOwnClass&&!f.conversions);
#else
  CHECK(!f.Step()&&!f.Step()&&!f.Step()&&f.Step()==1);
#endif
  CHECK(f.bundleAdditional.selectionRefusal==0&&f.bundleAdditional.selected.pawnClass==f.choices[2]&&f.bundleAdditional.selectedIndex==0);
  CHECK(f.AdditionalTemplate());}
 {SelectionFixture f;f.cold=true;f.hosted=false;f.primaryTemplate.pawnClass=0;f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);
  CHECK(!f.Step()&&!f.Step()&&!f.Step()&&f.Step()==2&&f.bundleAdditional.selectionRefusal==1);} // offline unchanged
 {// H2.51: hosted match, NO captured catalogue (count==0): adopted Jason's own resident class is selected,
  // and the full birth pipeline (template re-proof at every stage) reaches Active.
  SelectionFixture f;f.hosted=true;f.primaryTemplate.pawnClass=0;
  f.spawnAssets.primary.catalogue={};f.r_.Put(f.plan.mode+cfgprimary::KillerClasses,f.spawnAssets.primary.catalogue);
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);
  CHECK(f.Step()==1&&f.bundleAdditional.hostedOwnClass&&f.bundleAdditional.selectionRefusal==0&&
        f.bundleAdditional.selected.pawnClass==f.choices[2]&&f.bundleAdditional.selectedIndex==0);
  f.Fresh();CHECK(f.AdditionalTemplate());
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[1]);f.Fresh();CHECK(!f.AdditionalTemplate()); // own class changed
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);}
 {SelectionFixture f;f.hosted=true;f.primaryTemplate.pawnClass=0;
  f.spawnAssets.primary.catalogue={};f.r_.Put(f.plan.mode+cfgprimary::KillerClasses,f.spawnAssets.primary.catalogue);
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);
  CHECK(f.bundleAdditional.policy.Request(true,true,true,1000));
  for(unsigned now=1000;now<6000&&f.bundleAdditional.policy.stage!=cfgadditional::Stage::Active;now+=100){f.Fresh();CHECK(f.AdditionalPhase(now));}
  CHECK(f.bundleAdditional.policy.stage==cfgadditional::Stage::Active&&f.allocations>=1&&f.bundleAdditional.hostedOwnClass);}
 {// missing hosted resources still refuse cleanly (no killerBase / no BaseAI).
  SelectionFixture f;f.hosted=true;f.primaryTemplate.pawnClass=0;
  f.spawnAssets.primary.catalogue={};f.r_.Put(f.plan.mode+cfgprimary::KillerClasses,f.spawnAssets.primary.catalogue);
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);f.spawnAssets.killerBase=0;
  CHECK(f.Step()==2&&f.bundleAdditional.selectionRefusal==1&&!f.bundleAdditional.hostedOwnClass);}
 {SelectionFixture f;f.hosted=true;f.primaryTemplate.pawnClass=0;
  f.spawnAssets.primary.catalogue={};f.r_.Put(f.plan.mode+cfgprimary::KillerClasses,f.spawnAssets.primary.catalogue);
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);f.spawnAssets.resources.classes[cfgspawn::BaseAI]=0;
  CHECK(f.Step()==2&&f.bundleAdditional.selectionRefusal==1);}
 {// offline (not hosted) with an empty catalogue is unchanged: clean refusal.
  SelectionFixture f;f.hosted=false;f.spawnAssets.primary.catalogue={};f.r_.Put(f.plan.mode+cfgprimary::KillerClasses,f.spawnAssets.primary.catalogue);
  f.r_.Put(f.bundleJason.pawn.object+cfgprobe::ObjectClass,f.choices[2]);
  CHECK(f.Step()==2&&f.bundleAdditional.selectionRefusal==1&&!f.bundleAdditional.hostedOwnClass);}
 for(unsigned route=0;route<2;++route){SelectionFixture f;f.bundleAdditional.policy={};
  // Primary explicit and indexed selection both publish the SAME shared resource.
  // Reproduce the live bug: native function exists, but SpawnAccess fields are cold.
  const auto primaryClass=f.primaryTemplate.pawnClass;f.spawnAssets.conversionFunction=0;f.spawnAssets.conversionSlot={};
  f.Fresh();CHECK(f.PrepareSharedConversion()&&f.spawnAssets.conversionFunction&&f.spawnAssets.conversionSlot.object);
  CHECK(f.primaryTemplate.pawnClass==primaryClass); // preparation never converts or replaces primary
  if(route){f.Fresh();CHECK(f.Step()==0);f.bundleAdditional.selection=0;f.bundleAdditional.policy.choices=0;}
  CHECK(f.bundleAdditional.policy.Request(true,true,true,1000));
  for(unsigned now=1000;now<6000&&f.bundleAdditional.policy.stage!=cfgadditional::Stage::Active;now+=100){
   f.Fresh();CHECK(f.AdditionalPhase(now));}
  CHECK(f.bundleAdditional.policy.stage==cfgadditional::Stage::Active&&f.allocations>=1);
  f.Fresh();CHECK(f.AdditionalPhase(7000)&&f.driveTicks==1&&f.AdditionalLinked());
 }
 SelectionFixture single;single.spawnAssets.primary.catalogue.count=single.spawnAssets.primary.catalogue.capacity=1;single.r_.Put(single.plan.mode+cfgprimary::KillerClasses,single.spawnAssets.primary.catalogue);single.bundleAdditional.policy.edgeAt=1;
 CHECK(single.Step()==0&&single.bundleAdditional.policy.choices==0);CHECK(single.Step()==1&&single.bundleAdditional.selectedIndex==0); // one-entry hosted list: same-class fallback
 std::printf("Included F1 donor random/resident catalogue selection, including one-entry hosted catalogue: %d failures; native conversions mocked.\n",failures);return failures?1:0;
}
