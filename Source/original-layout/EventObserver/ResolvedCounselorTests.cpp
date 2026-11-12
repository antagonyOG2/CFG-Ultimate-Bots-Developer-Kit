#define main ExistingAiTests
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "ResolvedCounselorTemplate.hpp"
int main(){unsigned failed=0,checks=0;auto check=[&](bool value){++checks;if(!value)++failed;};
    {Fixture f;cfgspawn::TemplateObservation t{};
     // Human fixture is a killer class, so reparent it to counselor for this test.
     f.Put(f.pawnClass+cfgprobe::ClassSuper,f.counselorBase);f.Put(f.defaults+cfgspawn::PawnAIClass,f.classes[cfgspawn::CounselorAI]);
     f.Put(f.defaults+cfgspawn::CharacterTree,f.tree);
     check(cfgbundle::ResolvedCounselorTemplate(f,f.objects,f.pawnClass,f.counselorBase,f.classes[cfgspawn::CounselorAI],t));
     check(t.kind==1&&t.pawnClass==f.pawnClass&&t.defaults==f.defaults);
     f.Put(f.defaults+cfgprobe::ObjectClass,f.botClass);check(!cfgbundle::ResolvedCounselorTemplate(f,f.objects,f.pawnClass,f.counselorBase,f.classes[cfgspawn::CounselorAI],t));}
    {Fixture f;cfgspawn::TemplateObservation t{};f.Put(f.pawnClass+cfgprobe::ClassSuper,f.counselorBase);f.Put(f.defaults+cfgspawn::PawnAIClass,f.classes[cfgspawn::BaseAI]);
     check(!cfgbundle::ResolvedCounselorTemplate(f,f.objects,f.pawnClass,f.counselorBase,f.classes[cfgspawn::CounselorAI],t));}
    {Fixture f;cfgspawn::TemplateObservation t{};const auto customAI=f.Object();
     f.Put(customAI+cfgprobe::ClassSuper,f.classes[cfgspawn::CounselorAI]);
     f.Put(f.botDefaults+cfgspawn::PawnAIClass,customAI);
     check(cfgbundle::ResolvedCounselorTemplate(f,f.objects,f.botClass,f.counselorBase,f.classes[cfgspawn::CounselorAI],t));
     check(t.defaultAI==customAI);
     f.Put(customAI+cfgprobe::ClassSuper,f.classes[cfgspawn::KillerAI]);
     check(!cfgbundle::ResolvedCounselorTemplate(f,f.objects,f.botClass,f.counselorBase,f.classes[cfgspawn::CounselorAI],t));}
    std::printf("Resolved counselor CDO: %u checks, %u failures.\n",checks,failed);return failed?1:0;
}
