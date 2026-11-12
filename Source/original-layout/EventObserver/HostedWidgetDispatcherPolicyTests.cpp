#include "../Core/HostedWidgetDispatchPolicy.hpp"
#include <cstdio>
int main(){
    unsigned failures=0;
    const auto check=[&](bool value,const char* name){if(!value){++failures;std::printf("FAIL %s\n",name);}};
    using R=cfghostedwidget::Route;
    check(cfghostedwidget::Select(false,false)==R::FullJ219,"offline retains all existing J2.19 widget consumers");
    check(cfghostedwidget::Select(false,true)==R::FullJ219,"offline status widget retains existing route");
    check(cfghostedwidget::Select(true,false)==R::ForwardOnly,"hosted health/objective/summary events forward untouched");
    check(cfghostedwidget::Select(true,true)==R::SpectatorStatusOnly,"hosted status widget reaches only the label consumer");
    const bool endFrozen=true; // Driver ownership is false; the world remains hosted.
    const bool hostedWorld=true;
    check(endFrozen&&cfghostedwidget::Select(hostedWorld,false)==R::ForwardOnly,"end-frozen hosted summary/health/objective widgets stay forwarded");
    check(endFrozen&&cfghostedwidget::Select(hostedWorld,true)==R::SpectatorStatusOnly,"end-frozen hosted status still uses only the spectator label consumer");
    const auto fwd=cfghostedwidget::Select(true,false);
    check(!cfghostedwidget::SummaryOpportunity(fwd,false),"H2.48 summary never runs mid-match (no end window)");
    check(cfghostedwidget::SummaryOpportunity(fwd,true)==(CFG_H247_SUMMARY_BOTS!=0),"H2.48 summary runs in the end window when flagged");
    check(!cfghostedwidget::SummaryOpportunity(cfghostedwidget::Select(true,true),true)&&!cfghostedwidget::SummaryOpportunity(cfghostedwidget::Select(false,false),true),"status widget and offline route unchanged");
    return failures?1:0;
}
