#include "../Core/CounselorPracticePolicy.hpp"
#include "../Core/SetupMenuModel.hpp"
#include <cstdio>
int main(){
    unsigned failures=0,checks=0;const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    using namespace cfgpractice;check(SupportedMask()==7&&Valid(7)&&!Valid(8)&&!Valid(16));
    cfgmenu::Capabilities caps;caps.verified.fill(true);
    for(auto id:{cfgmenu::RowId::HardMovement,cfgmenu::RowId::BotSweater})check(!cfgmenu::Available(cfgmenu::Rows[static_cast<unsigned>(id)],cfgmenu::Mode::Counselor,caps));
    for(unsigned choices=0;choices<8;++choices)check(HardAutomaticOrSelected(choices)&&BotSweaterAutomaticOrSelected(choices));
    Report report;GrabEvent event;
    event.Observe(100,7,2,report);event.Observe(100,7,2,report);check(report.grabDistantSeen==1&&!report.grabNearSkipped);
    event.Observe(100,7,1,report);event.Observe(100,7,1,report);check(report.grabNearSkipped==1);
    event.Observe(0,0,0,report);event.Observe(100,7,2,report);check(report.grabDistantSeen==2);
    event.Observe(100,8,2,report);check(report.grabDistantSeen==3); // new identity at same address
    // Any archetype gets the resident axe, even without a human loadout hint.
    for(unsigned archetype=0;archetype<10;++archetype)check(ConvergenceWeapon(0xA000,0xB000,report)==0xA000);
    check(!report.axeClassMissing);check(ConvergenceWeapon(0,0xB000,report)==0xB000&&report.axeClassMissing==1);
    check(!ConvergenceWeapon(0,0,report)&&report.axeClassMissing==2);
    std::printf("J216 automatic defaults/removed rows/grab episodes/resident axe fallback: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
