#include "J21cReportFormatting.hpp"
#include <cstdio>
#include <memory>
int main(){unsigned n=0,bad=0;auto check=[&](bool v){++n;if(!v)++bad;};
 for(const auto* file:{"Tests/EventObserver/Fixtures/J21b-PID5072-Rejected0.bin","Tests/EventObserver/Fixtures/J21b-PID5072-Rejected1.bin"}){
  auto r=std::make_unique<cfgbundle::Report>();FILE* f=nullptr;if(fopen_s(&f,file,"rb")||!f)return 2;
  check(fread(r.get(),1,24184,f)==24184);fclose(f);check(r->version==70&&r->bytes==24184&&r->killEligibilityInstalled==2);
  // Preserve ALL archived values. Only adapt the schema header to the new appended diagnostics.
  r->version=cfgbundle::ReportVersion;r->bytes=sizeof(*r);check(cfgbundle::Valid(*r));
  r->helperPreparation=12;check(cfgbundle::Valid(*r));r->helperPreparation=13;check(!cfgbundle::Valid(*r));
  r->helperPreparation=2;r->spectatorFailure=35;check(!cfgbundle::Valid(*r));r->spectatorFailure=3;
  r->j21Epoch.source=3;check(!cfgbundle::Valid(*r));r->j21Epoch.source=0;
  r->j2Held[0].slot=8;check(!cfgbundle::Valid(*r));
 }
 {cfgbundle::Report r;check(cfgbundle::Valid(r));r.j21cHeld[0].observeStage=9;check(!cfgbundle::Valid(r));r.j21cHeld[0].observeStage=6;r.j21cHeld[0].queryStage=4;check(!cfgbundle::Valid(r));r.j21cHeld[0].queryStage=3;r.j21cHeld[0].inputStage=7;check(!cfgbundle::Valid(r));r.j21cHeld[0].inputStage=6;check(cfgbundle::Valid(r));
  r.h283Kill[1].kills=1;r.h283Kill[1].killsZero=1;r.h283Kill[1].ownerRole=3;r.h283Kill[1].moveKillFlag=1;check(cfgbundle::Valid(r)&&r.h283Kill[0].ownerRole==255); // H2.83 report-only rows
  PrintJ21cHeldDiagnostics(r);}
 printf("J2.1c exact PID5072 rejected report/cumulative eligibility/range preservation: %u checks,%u failures. No game access.\n",n,bad);return bad?1:0;}