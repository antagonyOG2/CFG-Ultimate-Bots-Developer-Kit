#define CFG_J21C_HELD_RECOVERY 1
#define main EarlierHeldTimingFixture
#include "DonorHeldGrabKillTests.cpp"
#undef main
#include "J21cReportFormatting.hpp"
int main(){unsigned checks=0,failed=0;const auto check=[&](bool x){++checks;if(!x){++failed;std::printf("FAIL timing%u\n",checks);}};
 {State s;Host h;h.sample.allowed=false;check(s.Drive(h,1000));check(s.timing.report.holds==1);
  const std::uint64_t end=1000+FallbackDeadlineMs; // deadline from hold start 1000 (H2.76: 9000 ms with the held walk)
  check(s.Drive(h,end-1)&&!s.legacyFallback);check(!s.Drive(h,end)&&s.legacyFallback);
  h.sample.allowed=true;check(!s.Drive(h,end+300)&&s.legacyFallback);
  check(s.timing.report.allowed==1&&s.timing.report.samples[0].known==1&&s.timing.report.samples[0].allowedMs==FallbackDeadlineMs+300);
  s.Drive(h,end+500);check(s.timing.report.allowed==1&&s.timing.report.available==0);
  ++h.sample.hold.victimSerial;h.sample.allowed=false;s.Drive(h,end+1500);check(s.timing.report.holds==2&&s.timing.report.samples[1].known==0);
  h.sample.hold={};s.Drive(h,end+1600);check(s.stage==Stage::Idle&&cfggrabtiming::Valid(s.timing.report));}
 {State s;Host h;check(s.Drive(h,0));check(s.timing.report.samples[0].known==1&&s.timing.report.samples[0].allowedMs==0);
  s.Drive(h,100);check(s.timing.report.available==1&&s.timing.report.samples[0].availableMs==100&&s.timing.report.samples[0].slot==0);
  s.timing.Frozen(2,false,false,false);s.timing.Frozen(0,true,true,false);s.timing.Frozen(1,true,false,true);
  check(s.timing.report.frozenEntered[0]==1&&s.timing.report.frozenReturned[0]==1&&!s.timing.report.frozenEntered[2]);
  check(cfggrabtiming::Valid(s.timing.report));}
 {cfggrabtiming::Timing t;t.Allowed(1);t.Available(1,0);check(!t.report.holds&&!t.report.allowed);
  for(unsigned i=0;i<20;++i){t.Begin(100+i);t.Allowed(99);t.Available(99,0);t.Available(100+i,7);}
  check(t.report.holds==20&&t.report.count==16&&t.report.cursor==4&&!t.report.allowed&&!t.report.available);
  check(cfggrabtiming::Valid(t.report));t.report.cursor=16;check(!cfggrabtiming::Valid(t.report));}
 {cfgbundle::Report r;check(r.version==cfgbundle::ReportVersion&&r.bytes==sizeof(r)&&cfgbundle::Valid(r));
  r.j23GrabTiming[0].cursor=16;check(!cfgbundle::Valid(r));r.j23GrabTiming[0].cursor=0;
  r.j23GrabTiming[0].frozenReturned[0]=1;check(!cfgbundle::Valid(r));r.j23GrabTiming[0].frozenEntered[0]=1;check(cfgbundle::Valid(r));
  PrintJ21cHeldDiagnostics(r);}
 std::printf("J23 observed hold/first-native-allowed/slot timing: %u checks,%u failures; held fallback deadline, no extra native queries.\n",checks,failed);return failed?1:0;}
