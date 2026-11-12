#include "../Core/VehicleActionPolicy.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;auto check=[&](bool ok){++checks;if(!ok){++failures;std::printf("FAIL H228 outcome %u\n",checks);}};
    cfgvehicleaction::State car;car.extractionCommitted=true;
    check(!car.RecordExtractionCompleted(true)); // the route heuristic alone is not a native extraction
    car.ObserveExtractionActive(0x1000,7,0x2000,9);
    check(car.extractionCommitCount==1&&car.nativeExtractionVictim==0x2000);
    check(!car.RecordExtractionCompleted(false)); // no terminal/out-of-seat evidence
    check(car.RecordExtractionCompleted(true)&&car.extractionCompleted==1);
    check(!car.RecordExtractionCompleted(true)&&car.extractionCompleted==1); // one completion per episode
    car.EndExtractionObservation();car.ObserveExtractionActive(0x1000,8,0x3000,10);
    check(car.extractionCommitCount==2); // a fresh object serial is a new event even at the same address
    car.EndExtractionObservation();car.ResetApproach();check(car.extractionCompleted==1&&!car.extractionActiveObserved&&car.extractionCommitCount==2);
    cfgbundle::Report report;check(cfgbundle::Valid(report)&&report.version==cfgbundle::ReportVersion);
    report.carExtractionCommitted=64;check(cfgbundle::Valid(report));
    report.carExtractionCommitted=65;check(!cfgbundle::Valid(report));
    report.carExtractionCommitted=64;
    report.carExtractionCompleted=64;check(cfgbundle::Valid(report));
    report.carExtractionCompleted=65;check(!cfgbundle::Valid(report));
    std::printf("H2.29 extraction outcome state/report: %u checks, %u failures; no engine calls.\n",checks,failures);
    return failures?1:0;
}
