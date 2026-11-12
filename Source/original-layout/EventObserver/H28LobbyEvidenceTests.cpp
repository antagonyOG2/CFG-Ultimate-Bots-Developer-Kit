#include "HostedLobbyInputEvidence.hpp"
#include "HostedLobbyStaticInputs.hpp"
#include <cstdio>
struct Provider { unsigned attempts=0,failures=0,stage=0,role=0;cfghostedlobbystatic::Failure failure{};
 unsigned InputAttempts()const{return attempts;}unsigned InputFailures()const{return failures;}
 unsigned Stage()const{return stage;}unsigned Role()const{return role;}const auto& Failure()const{return failure;}
 const auto& Owners()const{static std::array<cfghostedlobby::OwnerEvidence,13> owners{};return owners;} };
struct Access {unsigned typeReason=0,currentReason=0,invokeReason=7;};
int main(){unsigned failures=0,checks=0;auto check=[&](bool x){++checks;if(!x)++failures;};
 cfghostedlobby::Report report;Provider empty;Access access;
 report.f3Refused=1;report.f3Reason=1;check(cfghostedlobby::F3RosterFull(report));
 report.f3Reason=10;check(!cfghostedlobby::F3RosterFull(report));
 report.f3Refused=0;report.f3Reason=1;check(!cfghostedlobby::F3RosterFull(report));
 report.f3Refused=0;report.f3Reason=0;
 cfghostedlobbyinputs::RecordInputs(report,empty,&access,3);check(!report.rowRootReason&&!report.rowRootStage);
 Provider bad;bad.attempts=1;bad.failures=1;bad.stage=4;bad.role=3;bad.failure.reason=11;bad.failure.value=0x123400;bad.failure.slot.serial=0;
 // Actual failure is published even when the caller returns from factory warmup.
 {struct Exit{cfghostedlobby::Report& r;Provider& p;Access& a;~Exit(){cfghostedlobbyinputs::RecordInputs(r,p,&a,3);}} exit{report,bad,access};}
 check(report.rowRootReason==11&&report.rowRootStage==4&&report.rowRootValue==0x123400&&report.rowRootSerial==0);
 cfghostedlobbyinputs::RecordInputs(report,empty,&access,3);check(report.rowRootReason==11&&report.rowRootValue==0x123400);
 bad.failure.reason=38;bad.failure.value=0;cfghostedlobbyinputs::RecordInputs(report,bad,&access,3);
 check(report.rowRootReason==11&&report.rowInputAttempts==2&&report.rowInputFailures==2);
 std::printf("H28 first real lobby Inputs/warmup evidence: %u checks, %u failures.\n",checks,failures);return failures?1:0;
}
