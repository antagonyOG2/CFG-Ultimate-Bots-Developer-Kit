#include "HostedH1Report.hpp"
#include <cstdio>
#include <cstring>

int main(){
    unsigned failures=0,checks=0;
    const auto check=[&](bool ok){++checks;if(!ok)++failures;};
    cfgh1::Report report;
    check(report.version==cfgh1::Version&&report.bytes==sizeof(cfgh1::Report));
    check(report.hostedLabelEntered==0&&report.hostedLabelApplied==0&&report.hostedLabelRefused==0);
    check(cfgh1::RecordHardDamage(report,2,1,12.5f,25.0f)&&report.hardDamageScaled==1&&
        report.hardDamageInput==12.5f&&report.hardDamageOutput==25.0f);
    check(!cfgh1::RecordHardDamage(report,2,0,12.5f,25.0f)&&
        !cfgh1::RecordHardDamage(report,2,1,12.5f,24.0f));
    check(report.lanes.size()==21);
    check(static_cast<unsigned>(cfgh1::Lane::Hiding)==18);
    check(static_cast<unsigned>(cfgh1::Lane::Swim)==19);
    check(static_cast<unsigned>(cfgh1::Lane::Door)==20);
    report.lanes[static_cast<unsigned>(cfgh1::Lane::Hiding)]={3,1,4};
    report.lanes[static_cast<unsigned>(cfgh1::Lane::Swim)]={2,1,14};
    report.lanes[static_cast<unsigned>(cfgh1::Lane::Door)]={8,2,1};
    report.driver.stallSamples=2;
    report.presentationCopyRefusal=42;
    report.presentationCopyBundleNativeStage=2;
    report.grabAdmissionRefusals={3,4,5,6};
    report.grabKillEntry={7,8,9,10};
    report.driver.stallFirst.tick=100;report.driver.stallFirst.target=0x1234;
    report.driver.stallFirst.nextDistanceTeleportAt=12000;report.driver.stallFirst.distance=15110.23f;
    report.driver.stallFirst.gate=10;report.driver.stallFirst.navStage=1;report.driver.stallFirst.pathMoveResult=2;
    report.driver.stallFirst.distanceMorphRefusal=2;report.driver.stallFirst.phase=4;
    report.driver.stallLatest.tick=5100;report.driver.stallLatest.target=0x1234;
    report.driver.stallLatest.nextDistanceTeleportAt=12000;report.driver.stallLatest.distance=15110.23f;
    report.driver.stallLatest.gate=7;report.driver.stallLatest.externalMask=0x20;
    report.driver.stallLatest.navStage=1;report.driver.stallLatest.pathMoveResult=2;
    report.driver.stallLatest.distanceMorphRefusal=2;report.driver.stallLatest.phase=4;
    check(report.driver.huntLaneReached==0&&report.driver.burstWantedLeaseBlocked==0&&report.driver.morphWantedLeaseBlocked==0);
    report.driver.huntLaneReached=9;report.driver.burstWantedLeaseBlocked=4;report.driver.morphWantedLeaseBlocked=2;
    const auto copy=report;
    check(copy.driver.huntLaneReached==9&&copy.driver.burstWantedLeaseBlocked==4&&copy.driver.morphWantedLeaseBlocked==2);
    check(copy.lanes[18].entered==3&&copy.lanes[18].refused==1&&copy.lanes[18].last==4);
    check(copy.lanes[19].entered==2&&copy.lanes[19].refused==1&&copy.lanes[19].last==14);
    check(copy.lanes[20].entered==8&&copy.lanes[20].refused==2&&copy.lanes[20].last==1);
    check(copy.driver.stallSamples==2&&copy.driver.stallFirst.tick==100&&copy.driver.stallLatest.tick==5100&&
        copy.driver.stallLatest.externalMask==0x20&&copy.driver.stallLatest.distance==15110.23f);
    check(copy.presentationCopyRefusal==42); // inner HostedCopyKillerPresentation clause survives publication
    check(copy.presentationCopyBundleNativeStage==2); // distinguishes BeginBundleNative retention stages
    check(copy.grabAdmissionRefusals==std::array<std::uint32_t,4>{3,4,5,6}); // hosted context/admission/identity/receipt clauses remain distinct
    check(copy.grabKillEntry==std::array<std::uint32_t,4>{7,8,9,10}); // reflected grab-kill native outcome remains distinct
    cfgh1::StallTracker tracker;cfgh1::DriverTrace::StallSample sample{};
    const cfgh1::StallTracker::Position position{10,20,30};
    check(!tracker.Observe(1000,0x1234,15110.23f,position,true,true,sample));
    check(!tracker.Observe(5999,0x1234,15110.23f,position,true,true,sample));
    const bool stalled=tracker.Observe(6000,0x1234,15110.23f,position,true,true,sample);
    sample.gate=7;sample.externalMask=0x24;
    check(stalled&&sample.target==0x1234&&sample.distance==15110.23f&&sample.gate==7&&sample.externalMask==0x24);
    tracker.Reset();
    check(!tracker.Observe(1000,0x1234,270.f,{0,0,0},true,true,sample,true));
    check(!tracker.Observe(3500,0x1234,265.f,{45,0,0},true,true,sample,true));
    check(tracker.Observe(6000,0x1234,269.f,{0,0,0},true,true,sample,true));
    check(!tracker.Observe(6500,0x1234,40.f,{50,0,0},true,true,sample,true));
    std::printf("H228 hosted lane report schema: %u checks, %u failures.\n",checks,failures);
    return failures?1:0;
}
