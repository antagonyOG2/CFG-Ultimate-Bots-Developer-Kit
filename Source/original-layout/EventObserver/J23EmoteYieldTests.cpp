#define CFG_DONOR_EMOTE_FIXTURE_ONLY
#include "DonorCounselorEmotesTests.cpp"
#undef CFG_DONOR_EMOTE_FIXTURE_ONLY
struct BudgetAccess:DonorAccess {
 bool active=false,exhausted=true;unsigned fault=0;
 bool Reject(unsigned stage)const{return active&&fault==stage;}
 bool ReadExhausted()const{return active&&exhausted;}
 bool Guard()const{return !Reject(0)&&Access::Guard();}
 bool RecipientCurrent(const Recipient& r){return !Reject(1)&&Access::RecipientCurrent(r);}
 bool CatalogueFor(const Slot& s,std::uintptr_t selected,Catalogue& out){return !Reject(2)&&Access::CatalogueFor(s,selected,out);}
 bool FunctionCurrent(unsigned id,const Function& f){return !(id==0&&Reject(3))&&Access::FunctionCurrent(id,f);}
 bool Montage(std::uintptr_t pawn,std::uintptr_t& out){return !(pawn!=human.object&&Reject(4))&&Access::Montage(pawn,out);}
};
int main(){unsigned checks=0,failed=0;const auto check=[&](bool b){++checks;if(!b){++failed;std::printf("FAIL yield%u\n",checks);}};
 for(unsigned fault=0;fault<5;++fault){DonorRuntime runtime;BudgetAccess a;a.donor=&runtime;a.input.count=1;
  check(!runtime.Prepare(a)&&runtime.Prepare(a));StartDonor(runtime,a,1000);a.fault=fault;a.active=true;
  const auto humanMontage=a.montage[a.human.object];runtime.Service(a,1025);
  check(runtime.Pending()&&runtime.readDeferrals==1&&!a.calls&&!runtime.report.skipped);
  runtime.Service(a,1074);check(runtime.readDeferrals==1&&!a.calls);
  a.active=false;runtime.Service(a,1075);check(a.calls==1&&runtime.report.started==1&&!runtime.Pending());
  check(a.montage[a.human.object]==humanMontage&&!a.cancelCalls);
 }
 {DonorRuntime runtime;BudgetAccess a;a.input.count=1;check(!runtime.Prepare(a)&&runtime.Prepare(a));StartDonor(runtime,a,1000);a.active=true;
  for(unsigned now=1025;now<=1175;now+=50)runtime.Service(a,now);
  check(runtime.readDeferrals==3&&runtime.budgetDropped==1&&runtime.report.skipped==1&&!runtime.Pending()&&!a.calls);
  a.active=false;runtime.Service(a,1200);check(!a.calls);}
 {DonorRuntime runtime;BudgetAccess a;a.input.count=1;check(!runtime.Prepare(a)&&runtime.Prepare(a));StartDonor(runtime,a,1000);a.active=true;a.exhausted=false;a.fault=1;
  runtime.Service(a,1025);check(!runtime.Pending()&&!runtime.readDeferrals&&runtime.report.skipped==1&&!a.calls);}
 {DonorRuntime runtime;BudgetAccess a;a.input.count=1;check(!runtime.Prepare(a)&&runtime.Prepare(a));StartDonor(runtime,a,1000);a.dispatch=false;
  runtime.Service(a,1025);check(!runtime.Pending()&&a.calls==1&&!runtime.readDeferrals);a.dispatch=true;runtime.Service(a,1100);check(a.calls==1);}
 std::printf("J23 emote pre-dispatch read-yield: %u checks,%u failures; five proof stages defer bounded3, invalid recipients skip, native ambiguity never replayed, human untouched.\n",checks,failed);return failed||failures?1:0;}
