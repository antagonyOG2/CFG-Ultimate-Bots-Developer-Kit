#define CFG_J24_PRODUCTION_TEST_MAIN PriorJ24Checks
#include "J24ProductionTests.cpp"
namespace {
unsigned voiceForwards=0,voiceErrors=0;
void __fastcall FixtureVoiceForward(void* object,cfgnames::Words name,std::uintptr_t arg3,std::uintptr_t arg4){
 ++voiceForwards;
 if(object!=reinterpret_cast<void*>(0x123000)||name.index!=321||name.number!=2||arg3!=0x456000||arg4!=0x789000)++voiceErrors;
}
}
int main(){
 if(PriorJ24Checks())return 1;
 unsigned checks=0,failures=0;const auto check=[&](bool good){++checks;if(!good)++failures;};
 bundlePracticeVoiceHook={};bundlePracticeVoiceHook.original=reinterpret_cast<void*>(&FixtureVoiceForward);
 bundleStarted=false;bundleSchedule.active=false;bundleConfiguration={};
 const cfgnames::Words line{321,2};
 J215NativeVoice(reinterpret_cast<void*>(0x123000),line,0x456000,0x789000);
 check(voiceForwards==1&&!voiceErrors&&!bundleReport.j215Practice.voiceSeen);
 bundleStarted=true;bundleSchedule.active=true;plan.threadId=GetCurrentThreadId();
 bundleConfiguration.counselorPractice=0;
 J215NativeVoice(reinterpret_cast<void*>(0x123000),line,0x456000,0x789000);
 check(voiceForwards==2&&!voiceErrors&&!bundleReport.j215Practice.voiceSeen);
 bundleConfiguration.counselorPractice=cfgpractice::PamelaMute;
 J215NativeVoice(reinterpret_cast<void*>(0x123000),line,0x456000,0x789000);
 check(voiceForwards==3&&!voiceErrors&&bundleReport.j215Practice.voiceSeen==1&&bundleReport.j215Practice.voiceRefused==1);
 bundleConfiguration.counselorPractice=cfgpractice::MinimalHud;std::uint8_t show=1;
 check(!J215HideHealth(reinterpret_cast<void*>(0x123000),reinterpret_cast<void*>(0x456000),&show)&&show==1);
 BundleAccess access;
 for(auto mask:{0u,static_cast<unsigned>(cfgpractice::HardMovement),static_cast<unsigned>(cfgpractice::BotSweater)}){
  bundleConfiguration.counselorPractice=mask;const auto reads=access.Reads(),calls=access.Calls();
  check(!access.PracticePhase(1000)&&reads==access.Reads()&&calls==access.Calls());
 }
 bundlePractice.widgets[0].value.owned=true;bundlePractice.targets=1;bundlePractice.healthOwner=0x123000;
 BundleResetEpoch();check(!bundlePractice.targets&&!bundlePractice.healthOwner&&!bundlePractice.widgets[0].value.owned);
 check(bundlePracticeVoiceHook.original==reinterpret_cast<void*>(&FixtureVoiceForward));
 bundlePracticeVoiceHook={};
 std::printf("J215 real presentation callbacks: OFF/foreign forwards once with exact args; wrong context refuses; epoch reset weak-only: %u checks, %u failures. Own process only.\n",checks,failures);
 return failures?1:0;
}
