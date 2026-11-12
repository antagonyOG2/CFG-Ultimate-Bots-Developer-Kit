#include "ObjectiveHudRepair.hpp"
#include <cstdio>
using namespace cfgobjective::repair;
using Kind=cfgobjectivepanel::Kind;
struct Host {
 Snapshot current{};Runtime* runtime=nullptr;Target target=Target::Car2;
 unsigned calls=0,observes=0,prepares=0,dispatches=0,observePending=0,preparePending=0,dispatchPending=0;
 unsigned frameFault=0,phaseReads=0,peakReads=0,phaseCalls=0,peakCalls=0;
 bool scope=true,append=true,ambiguous=false,wrongKind=false,reorder=false,extra=false,lie=false,reenter=false,overBudget=false;
 bool secondMarkAccepted=false,reentryWorked=false;std::uint64_t now=0;
 Host(){current.owner={0x100000,{0x200000,1,10},{0x300000,2,20},{0x400000,3,30},{0x500000,4,40}};
  current.panel.widget=current.owner.widget.object;current.panel.panel=current.owner.panel.object;
  current.panel.type=0x600000;current.panel.panelType=0x700000;current.panel.rootIdentity=current.owner.widget;current.panel.panelIdentity=current.owner.panel;
  current.panel.slots={0x800000,0,0};}
 bool Read(unsigned reads){if(reads>1536-phaseReads)return false;phaseReads+=reads;return true;}
 void Phase(){peakReads=(std::max)(peakReads,phaseReads);peakCalls=(std::max)(peakCalls,phaseCalls);phaseReads=phaseCalls=0;}
 void Add(Kind kind){const auto at=static_cast<unsigned>(current.panel.slots.count);auto& c=current.panel.children[at];
  c.slot=0x900000+0x1000*at;c.widget=0xA00000+0x1000*at;c.type=0xB00000+0x1000*static_cast<unsigned>(kind);c.kind=kind;
  c.slotIdentity={c.slot,static_cast<int>(10+2*at),static_cast<int>(100+at)};
  c.widgetIdentity={c.widget,static_cast<int>(11+2*at),static_cast<int>(200+at)};
  ++current.panel.slots.count;current.panel.slots.capacity=current.panel.slots.count+4;current.panel.slots.data+=0x1000;}
 Status Observe(Snapshot& out){++observes;if(!Read(400))return Status::Unavailable;
  if(observePending){--observePending;return Status::Pending;}if(!scope)return Status::Unavailable;out=current;return Status::Ready;}
 Status Prepare(const Owner&,Target selected,Finder& f){++prepares;target=selected;if(!Read(700))return Status::Unavailable;
  if(preparePending){--preparePending;return Status::Pending;}
  f.helper.spec=HelperIndex(target);f.helper.owner=current.panel.type;f.helper.ownerSlot={current.panel.type,80,0};
  f.helper.functionSlot={0xC00000+0x1000*static_cast<unsigned>(target),90+static_cast<int>(target),0};
  f.helper.script={0xD00000+0x1000*static_cast<unsigned>(target),static_cast<int>(cfgobjective::Specification(f.helper.spec).bytes),1080};
  f.frameBytes=0;f.numParms=0;f.voidReturn=f.helperVerified=true;
  if(frameFault==1)f.numParms=1;if(frameFault==2)f.frameBytes=8;if(frameFault==3)f.voidReturn=false;
  if(frameFault==4)f.helperVerified=false;if(frameFault==5)++f.helper.spec;if(frameFault==6)f.helper.owner+=0x1000;
  if(frameFault==7)--f.helper.script.count;if(frameFault==8)f.helper.functionSlot.object=0;
  return Status::Ready;}
 template<class Mark>Status Dispatch(const Owner& owner,const Finder& f,const Snapshot& before,Mark mark){
  ++dispatches;if(!Read(overBudget?1537u:1200u))return Status::Unavailable;
  if(dispatchPending){--dispatchPending;return Status::Pending;}
  if(!scope||!(owner==current.owner)||!Same(before,current)||!ValidFinder(f,current,target))return Status::Unavailable;
  if(lie)return Status::Ready;
  if(!mark())return Status::Unavailable;secondMarkAccepted=mark();++calls;++phaseCalls;
  if(reenter&&runtime)reentryWorked=runtime->Step(*this,now+1000);
  if(append){constexpr Kind kinds[]{Kind::Car2,Kind::Car4,Kind::Radio,Kind::Phone,Kind::Boat};Add(wrongKind?Kind::Bridge:kinds[static_cast<unsigned>(target)]);}
  if(extra)Add(Kind::Other);
  if(reorder&&current.panel.slots.count>1)std::swap(current.panel.children[0],current.panel.children[1]);
  return ambiguous?Status::Unavailable:Status::Ready;
 }
 void Run(Runtime& r,unsigned steps=100){runtime=&r;for(unsigned i=0;i<steps&&!r.Finished();++i){Phase();now+=100;r.Step(*this,now);}Phase();}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* what){++checks;if(!okay){++failures;std::printf("FAIL %s\n",what);}};
 {Host h;h.Add(Kind::Phone);h.current.owner.panel.serial=0;h.current.panel.panelIdentity.serial=0;
  h.current.panel.children[0].slotIdentity.serial=h.current.panel.children[0].widgetIdentity.serial=0;
  check(!Sane(h.current),"unassigned child graph without rooted-topology proof refuses");
  h.current.panel.rootedTopology=true;check(Sane(h.current),"repeated strong graph under positive owner permits unassigned children");
  h.current.owner.widget.serial=h.current.panel.rootIdentity.serial=0;check(!Sane(h.current),"root identity may never borrow serial0");}
 {Runtime r;Host h;r.Configure(AllMask);h.reenter=true;h.Run(r);
  check(r.Finished()&&r.report.confirmed==5&&h.calls==5&&r.report.attemptedMask==31,"each missing finder adds exactly its row once");
  check(!h.secondMarkAccepted&&!h.reentryWorked,"mark and callback reentry cannot duplicate a native call");
  check(h.peakReads<=1536&&h.peakCalls<=12,"phases do not stack host budget");
  check(!r.Step(h,100000)&&h.calls==5,"completed runtime does not replay");}
 for(Kind radio:{Kind::Radio,Kind::GrendelRadio})for(Kind phone:{Kind::Phone,Kind::GrendelPhone}){Runtime r;Host h;
  h.Add(Kind::Car2);h.Add(Kind::Car4);h.Add(radio);h.Add(phone);h.Run(r);
  check(r.report.preserved==4&&!h.calls&&!h.prepares&&r.report.stage==Stage::Complete,"existing car/radio/phone variants all preserved");}
 for(unsigned fault=1;fault<=8;++fault){Runtime r;Host h;r.Configure(1);h.frameFault=fault;h.Run(r);
  check(!h.calls&&r.report.refused==1&&r.report.failure==Failure::Finder,"unproven zero-frame or helper identity cannot dispatch");}
 {Runtime r;Host h;r.Configure(1);h.scope=false;h.Run(r);check(!h.calls&&r.report.stage==Stage::Refused,"foreign/offline owner failure is terminal before native work");}
 {Runtime r;Host h;r.Configure(1);h.observePending=h.preparePending=h.dispatchPending=2;h.Run(r);
  check(h.calls==1&&r.report.confirmed==1&&r.report.deferred==6,"pending metadata remains read-only and resumes");}
 {Runtime r;Host h;r.Configure(1);h.preparePending=1000;h.Run(r,250);
  check(!h.calls&&r.Finished()&&r.report.failure==Failure::Timeout,"preparation has finite deadline");}
 {Runtime r;Host h;r.Configure(1);h.ambiguous=true;h.Run(r);
  check(h.calls==1&&r.report.stage==Stage::Indeterminate&&!r.report.confirmed,"uncertain native return never retried or counted");}
 {Runtime r;Host h;r.Configure(1);h.append=false;h.Run(r);
  check(h.calls==1&&r.report.failure==Failure::NoChange&&!r.report.confirmed,"native return alone does not prove a row");}
 for(unsigned fault=0;fault<3;++fault){Runtime r;Host h;r.Configure(1);h.Add(Kind::Phone);
  h.wrongKind=fault==0;h.extra=fault==1;h.reorder=fault==2;h.Run(r);
  check(h.calls==1&&r.report.stage==Stage::Indeterminate&&r.report.failure==Failure::UnexpectedChange,"wrong kind extra row or reordered old row stops repair");}
 {Runtime r;Host h;r.Configure(1);h.Run(r,2);h.Add(Kind::Car2);h.Run(r);
  check(!h.calls&&r.report.refused==1,"a native row appearing before dispatch prevents duplication");}
 {Runtime r;Host h;r.Configure(1);h.Run(r,2);++h.current.owner.widget.serial;h.Run(r);
  check(!h.calls&&r.report.refused==1,"owner identity mutation is refused before dispatch");}
 {Runtime r;Host h;r.Configure(1);h.overBudget=true;h.Run(r);
  check(!h.calls&&r.report.refused==1&&h.peakReads<=1536,"host budget refusal never consumes native call");}
 {Runtime r;Host h;r.Configure(1);h.lie=true;h.Run(r);
  check(!h.calls&&r.report.stage==Stage::Indeterminate&&r.report.failure==Failure::DispatchContract,"host cannot report native success without consumption marker");}
 {Runtime r;Host h;r.Configure(0);h.Run(r);check(!h.observes&&!h.calls&&r.Finished(),"disabled policy touches no widget");}
 {Runtime r;Host h;r.Configure(1);for(unsigned i=0;i<64;++i)h.Add(Kind::Other);h.Run(r);
  check(!h.calls&&r.report.refused==1,"bounded full panel cannot append");}
 std::printf("ObjectiveHudRepairTests: %u checks, %u failures. No game access.\n",checks,failures);return failures?1:0;
}
