#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../Core/SummaryPresentationPolicy.hpp"
#include "../RuntimeProbe/SummaryPresentationBindings.hpp"
#include <cstdio>
struct {DWORD threadId=GetCurrentThreadId();std::uintptr_t module=0x140000000;}plan;
using SummaryTextOriginal=void*(__fastcall*)(void*,void*);
using SummaryPanelOriginal=unsigned char(__fastcall*)(void*);
using SummaryStateReadyOriginal=bool(__fastcall*)(void*,bool);
SummaryTextOriginal bundleSummaryTextOriginal=nullptr;
SummaryPanelOriginal bundleSummaryPanelOriginal=nullptr,bundleSummaryCharacterOriginal=nullptr;
SummaryStateReadyOriginal bundleSummaryStateReadyOriginal=nullptr;
thread_local std::uintptr_t bundleSummaryPresentationRow=0;
extern bool bundleStarted;
bool BundleGameplayStarted(){return bundleStarted;}
bool BundleSummaryStarted(){return bundleStarted;}
bool bundleStarted=true,nativeReady=false,proof=true,lastReady=false;unsigned nativeCalls=0,uiCalls=0,proofCalls=0;
std::uintptr_t fakeCaller=0,expectedRow=0x20000,expectedState=0x30000;
struct BundleAccess {bool SummaryPresentedState(std::uintptr_t row,std::uintptr_t state){++proofCalls;return proof&&row==expectedRow&&state==expectedState;}};
#define _ReturnAddress() reinterpret_cast<void*>(fakeCaller)
#include "SummaryPresentationRuntime.hpp"
#undef _ReturnAddress
bool __fastcall NativeReady(void*,bool){++nativeCalls;return nativeReady;}
void* __fastcall NativeText(void* row,void* output){++uiCalls;*static_cast<int*>(output)=777;
    fakeCaller=plan.module+cfgsummarypresentation::disk::TextReturn;
    lastReady=BundleNativeSummaryStateReady(reinterpret_cast<void*>(expectedState),false);
    if(reinterpret_cast<std::uintptr_t>(row)!=expectedRow)proof=false;return output;
}
unsigned char __fastcall NativePanel(void*){++uiCalls;fakeCaller=plan.module+cfgsummarypresentation::disk::PanelReturn;
    lastReady=BundleNativeSummaryStateReady(reinterpret_cast<void*>(expectedState),false);return 7;}
int main(){unsigned failed=0;const auto check=[&](bool b){if(!b)++failed;};
 bundleSummaryTextOriginal=&NativeText;bundleSummaryPanelOriginal=&NativePanel;bundleSummaryStateReadyOriginal=&NativeReady;
 int output=0;check(BundleNativeSummaryText(reinterpret_cast<void*>(expectedRow),&output)==&output);
 check(output==777&&lastReady&&nativeCalls==1&&uiCalls==1&&proofCalls==1&&!bundleSummaryPresentationRow);
 check(BundleNativeSummaryPanel(reinterpret_cast<void*>(expectedRow))==7&&lastReady&&nativeCalls==2&&uiCalls==2&&!bundleSummaryPresentationRow);
 for(unsigned fault=0;fault<7;++fault){nativeReady=false;proof=true;bundleStarted=true;plan.threadId=GetCurrentThreadId();
  cfgsummarypresentation::Scope scope(bundleSummaryPresentationRow,expectedRow);fakeCaller=plan.module+cfgsummarypresentation::disk::TextReturn;
  if(fault==1)proof=false;if(fault==2)bundleStarted=false;if(fault==3)++plan.threadId;
  if(fault==4)fakeCaller=plan.module+0x3DB59D;
  const auto before=nativeCalls;
  const bool ready=BundleNativeSummaryStateReady(reinterpret_cast<void*>(fault==5?expectedState+8:expectedState),fault==6);
  check(ready==(fault==0)&&nativeCalls==before+1);
 }
 bundleStarted=true;plan.threadId=GetCurrentThreadId();nativeReady=true;proof=false;fakeCaller=0;
 check(BundleNativeSummaryStateReady(reinterpret_cast<void*>(expectedState),true));check(!bundleSummaryPresentationRow);
 std::printf("Actual included UI wrappers:original called once,FText/panel output preserved,scoped state/thread/caller refusals:%u failures;native resources mocked.\n",failed);
 return failed?1:0;
}
