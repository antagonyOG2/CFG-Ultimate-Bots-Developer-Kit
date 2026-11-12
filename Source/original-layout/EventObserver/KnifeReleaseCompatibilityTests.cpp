#define CFG_H1_JASON 1
#include "KnifeReleaseCompatibility.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
namespace cfgknife::disk {constexpr std::uintptr_t Release=1;}
namespace cfgcompat::cfg35::constructBoundary {constexpr std::uintptr_t ReturnRva=0x69BDD2;}
struct TestLease {bool entered=false;bool NativeEntered()const{return entered;}} testLease;
TestLease* bundleRootLease=&testLease;
cfgknife::ReleaseFrame* bundleKnifeReleaseFrame=nullptr;
cfgknife::NotifyWindow* bundleKnifeNotifyWindow=nullptr;
cfgknife::ReleaseDiagnostics bundleKnifeReleaseDiagnostics;
unsigned bundleKnifeReleaseDepth=0,bundleKnifeNotifyAdmissionRefused=0,bundleKnifeNotifyEntries=0;
unsigned bundleKnifeNotifyMaxReads=0,bundleKnifeNotifyRefused=0,bundleKnifeNotifyCorrections=0;
struct {unsigned threadId=7;std::uintptr_t module=0x100000;} plan;
struct {struct {std::uintptr_t object=1;}pawn;} bundleJason;
struct {unsigned knifeNativeForwards=0;} bundleReport;
cfgknife::Runtime bundleKnife;
unsigned GetCurrentThreadId(){return 7;}std::uint64_t GetTickCount64(){return 100;}
namespace cfgbundle {constexpr unsigned ReadCap=1536;}
struct TestReader {unsigned reads=0,calls=0,cap=1536;
 bool Spend(unsigned amount,unsigned nativeCalls){if(amount>cap-reads||nativeCalls>12-calls)return false;reads+=amount;calls+=nativeCalls;return true;}
};bool retainOkay=true,inputsRetained=false;
template<std::size_t N>bool RetainPrimaryObjects(TestReader&,const std::array<std::uintptr_t,N>& values){
 inputsRetained=retainOkay&&N==3&&values[0]==1&&values[1]==2&&values[2]==3;return inputsRetained;}
class BundleAccess {
public:
 TestReader r_;unsigned calls_=0,mode=0,before=0,after=0,releases=0,corrections=0;
 bool exception_=false,uncertain=false,completed=false,hostedSibling=false;
 unsigned siblingReads=0,siblingCallDebt=0,entryCalls=0;
 unsigned context=0;
 BundleAccess()=default;
 BundleAccess(unsigned cap,unsigned debt):calls_(debt){r_.cap=cap;}
 bool HostedRoutineActive()const{return hostedSibling;}
 bool HostedAdoptionOwnsDriver()const{return hostedSibling;}
 unsigned Reads()const{return r_.reads;}
 struct ChildAccounting {
  BundleAccess& parent;BundleAccess& child;
  ChildAccounting(BundleAccess& p,BundleAccess& c):parent(p),child(c){}
  ~ChildAccounting(){parent.calls_=child.calls_;parent.after+=child.after;parent.corrections+=child.corrections;}
 };
 bool Spend(unsigned reads,unsigned calls){if(reads>1536-r_.reads||calls>12-calls_)return false;r_.reads+=reads;calls_+=calls;return true;}
 bool KnifeNotifyBefore(std::uintptr_t,cfgknife::NotifyWindow& w){
  ++before;assert(!testLease.entered&&inputsRetained);if(!Spend(100,2))return false;
  w.jason={1,10};w.driver={2,20};w.target={3,30};w.count=6;w.armed=true;w.geometryReady=true;return true;
 }
 bool KnifeReleaseEntryCurrent(const cfgknife::ReleaseFrame& frame){
  assert(testLease.entered&&frame.access==this&&frame.lease==bundleRootLease&&frame.depth==bundleKnifeReleaseDepth);
  if(hostedSibling){TestReader entry;entry.calls=calls_;siblingCallDebt=entry.calls;
   const bool current=entry.Spend(165,0)&&mode!=8;siblingReads=entry.reads;entryCalls=entry.calls;
   bundleKnifeReleaseDiagnostics.entryReads=siblingReads;return current;}
  const auto start=r_.reads;const bool current=Spend(165,0)&&mode!=8;
  bundleKnifeReleaseDiagnostics.entryReads=r_.reads-start;return current;
 }
 bool KnifeSimpleCall(std::uintptr_t){
  if(!Spend(10,1))return false;++releases;
  auto* frame=bundleKnifeReleaseFrame;if(!frame)return true;
  testLease.entered=true;++bundleKnifeReleaseDepth;
  if(mode==2)frame->window.stopped=true;
  const auto pc=mode==12?frame->returnPc+1:frame->returnPc;
  if(mode!=7&&frame->Enter(1,bundleRootLease,7,true,bundleKnifeReleaseDepth,pc)){
   if(mode==1){assert(frame->window.UseNativeGeometry(2,1));frame->window.multicast=frame->window.launched=true;}
   if(mode==3)frame->window.stopped=true;
   if(mode==10)frame->window.launched=true;
   if(mode==4)assert(!frame->Enter(1,bundleRootLease,7,true,bundleKnifeReleaseDepth,frame->returnPc));
   frame->Leave(mode!=5);
  }
  --bundleKnifeReleaseDepth;testLease.entered=false;completed=mode!=6;return completed;
 }
 bool KnifeNotifyAfter(cfgknife::NotifyWindow& w){
  ++after;assert((completed||!hostedSibling)&&!testLease.entered&&bundleKnifeReleaseFrame&&bundleKnifeNotifyWindow==&w);
  if(!Spend(100,0))return false;
  cfgknife::Snapshot fresh{{1,10},{3,30},{2,20},5,false,true};
  if(!w.Missing(fresh,1,true))return false;w.consumed=true;
  if(!Spend(20,3))return false;++corrections;return bundleKnife.CompleteNativeNotify(w.driver,100);
 }
 void KnifeLifecycleSample(std::uintptr_t,unsigned){}void BundleRetentionFailed(){uncertain=true;}
 unsigned knifeFailure=0;
#include "KnifeReleaseAdapterMembers.hpp"
};
int main(){
 for(unsigned mode=0;mode<9;++mode){
  bundleKnife={};bundleKnife.nativeCompletionRequired=true;bundleKnife.stage=cfgknife::Stage::Notify;
  bundleKnife.release.jason={1,10};bundleKnife.release.driver={2,20};bundleKnife.release.target={3,30};inputsRetained=false;
  BundleAccess access;access.mode=mode;const bool result=access.KnifeRetainedRelease({2,20});
  assert(access.before==1&&access.releases==1&&!bundleKnifeReleaseFrame&&!bundleKnifeNotifyWindow&&!testLease.entered&&!bundleKnifeReleaseDepth);
  if(mode==0)assert(result&&access.after==1&&access.corrections==1&&bundleKnife.launches==1);
  if(mode==1)assert(result&&access.after==1&&!access.corrections&&bundleKnife.launches==1);
  if(mode==2||mode==4||mode==7||mode==8)assert(result&&!access.after&&!access.corrections&&!bundleKnife.launches);
  if(mode==3)assert(!result&&access.after==1&&!access.corrections&&!bundleKnife.launches);
  if(mode==5)assert(!result&&!access.after&&access.exception_&&access.uncertain);
  if(mode==6)assert(!result&&!access.after&&!access.corrections);
  assert(bundleKnifeReleaseDiagnostics.reads==access.r_.reads&&bundleKnifeReleaseDiagnostics.calls==access.calls_);
 }
 {BundleAccess a;a.r_.reads=1536;assert(!a.KnifeRetainedRelease({2,20})&&!a.releases&&!a.after&&a.r_.reads==1536);}
 {BundleAccess a;a.calls_=12;assert(!a.KnifeRetainedRelease({2,20})&&!a.releases&&!a.after&&a.calls_==12);}
 {BundleAccess a;retainOkay=false;assert(a.KnifeRetainedRelease({2,20})&&a.releases==1&&!a.after);retainOkay=true;}
 {BundleAccess a;assert(a.KnifeRetainedRelease({2,99})&&a.releases==1&&!a.after);}
 {bundleKnife.launches=0;BundleAccess a;a.mode=10;a.hostedSibling=true;a.r_.reads=1360;
  assert(a.KnifeRetainedRelease({2,20})&&a.releases==1&&a.entryCalls==3&&a.siblingCallDebt==3&&
   a.siblingReads==165&&bundleKnife.launches==1&&!bundleKnifeReleaseFrame&&!testLease.entered);}
 {bundleKnife.stage=cfgknife::Stage::Notify;bundleKnife.launches=0;
  BundleAccess a;a.mode=11;a.hostedSibling=true;a.r_.reads=1360;
  assert(a.KnifeRetainedRelease({2,20})&&a.releases==1&&a.siblingReads==165&&
   a.after==1&&a.corrections==1&&a.calls_==6&&bundleKnife.launches==1&&!testLease.entered);}
 {bundleKnife.stage=cfgknife::Stage::Notify;bundleKnife.launches=0;
  BundleAccess a;a.mode=13;a.r_.reads=1360;
  assert(a.KnifeRetainedRelease({2,20})&&a.releases==1&&a.siblingReads==0&&a.after==0&&
   bundleKnife.launches==0&&!bundleKnifeReleaseFrame&&!testLease.entered);}
 {BundleAccess a;a.mode=12;a.hostedSibling=true;
  assert(a.KnifeRetainedRelease({2,20})&&a.releases==1&&a.siblingReads==0&&a.after==0&&
   bundleKnife.launches==0&&!bundleKnifeReleaseFrame&&!testLease.entered);}
 cfgknife::ReleaseFrame f;f.window.jason={1,10};f.window.armed=true;f.lease=&testLease;f.thread=7;f.depth=1;f.returnPc=100;
 f.current=[](void*,const cfgknife::ReleaseFrame&){return true;};
 assert(!f.Enter(99,&testLease,7,true,1,100)&&!f.entries);
 assert(!f.Enter(1,&testLease,8,true,1,100)&&!f.entries);
 assert(!f.Enter(1,&testLease,7,false,1,100)&&!f.entries);
 assert(!f.Enter(1,&testLease,7,true,2,100)&&!f.entries);
 assert(f.Enter(1,&testLease,7,true,1,100)&&f.Geometry(&testLease,7,true,1));
 assert(!f.Completed(&testLease,7,false));f.Leave(true);assert(f.Completed(&testLease,7,false));
 assert(!f.Enter(1,&testLease,7,true,1,100)&&!f.Completed(&testLease,7,false));
 std::puts("Actual lexical Release adapter member PASS: before native entry, original once, completion before correction, stopped/duplicate/identity/budget/exception refusals; owned stubs only, no engine authority.");
}
