#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <cstdio>
#include <cwchar>
#include <stdexcept>
#include "../Core/BotNamePolicy.hpp"
#include "../RuntimeProbe/BotNameBindings.generated.hpp"
#define CFG_RETAINED_GAMEPLAY_BUNDLE
namespace {
unsigned order=0,forwards=0,failures=0;bool nativeThrows=false,entered=false;
std::array<wchar_t,32> copied{};
void __fastcall OwnedNameSetter(void* object,const cfgbotname::String* packet){
 if(!entered||order!=3||object!=reinterpret_cast<void*>(0x20000))++failures;
 ++forwards;order=4;
 if(nativeThrows)RaiseException(0xE1234567,0,0,nullptr);
 std::wmemcpy(copied.data(),packet->data,static_cast<std::size_t>(packet->count));
}
struct {std::uintptr_t module=0;}plan;
struct Reader{};
bool publish=true;
template<std::size_t N>bool RetainPrimaryObjects(Reader&,const std::array<std::uintptr_t,N>& values){
 if(order!=1||N!=3||values[0]!=0x20000||values[1]!=0x30000||values[2]!=0x40000)++failures;
 if(!publish)return false;order=2;return true;
}
class Access{
public:
 Reader r_;unsigned calls_=0;bool exception_=false,uncertain=false,environment=true,retain=true,prepare=true,complete=true;
 bool NativeCallAllowed(){return environment&&calls_<12;}
 bool RetainBundleActors(std::uintptr_t state){if(state!=0x20000||order)++failures;if(!retain)return false;order=1;return true;}
 bool BeginBundleNative(){if(order!=2)++failures;if(!prepare)return false;order=3;entered=true;return true;}
 bool FinishBundleNative(){if(order!=4||!entered)++failures;order=5;entered=false;return complete;}
 void BundleRetentionFailed(){uncertain=true;}
#include "BotNameNativeMembers.hpp"
};
}
int main(){
 const auto check=[](bool okay){if(!okay)++failures;};
 static_assert(cfgbotname::disk::SetterVerified&&cfgbotname::disk::Setter==cfgcompat::cfg35::botName::SetterRva);
 plan.module=reinterpret_cast<std::uintptr_t>(&OwnedNameSetter)-cfgbotname::disk::Setter;
 for(unsigned scenario=0;scenario<10;++scenario){
  Access a;order=forwards=0;entered=false;copied={};nativeThrows=scenario==8;publish=scenario!=3;
  auto name=cfgbotname::Make(0,0);const auto original=name;
  cfgbotname::String packet{name.text.data(),name.count,name.count};
  if(scenario==1)a.environment=false;if(scenario==2)a.retain=false;if(scenario==4)a.prepare=false;
  if(scenario==5)a.calls_=12;if(scenario==6)packet.capacity++;
  if(scenario==7)packet.data[static_cast<unsigned>(packet.count-1)]='x';if(scenario==9)a.complete=false;
  const bool returned=a.BotNameNative(0x20000,packet,0x30000,0x40000);
  if(scenario==0){check(returned&&forwards==1&&a.calls_==1&&order==5&&!entered);
   name.text.fill('x');check(std::wcscmp(copied.data(),original.text.data())==0);}
  else if(scenario==8)check(!returned&&forwards==1&&a.calls_==1&&a.exception_&&a.uncertain);
  else if(scenario==9)check(!returned&&forwards==1&&order==5&&!entered);
  else check(!returned&&!forwards&&!a.exception_&&!a.uncertain&&a.calls_==(scenario==5?12u:0u));
 }
 std::printf("Actual bot-name native member: %u failures; input/root/prepare/forward/complete order, exact FString frame, preserved cap/refusals/SEH; owned stub only, not engine copy proof.\n",failures);
 return failures?1:0;
}
