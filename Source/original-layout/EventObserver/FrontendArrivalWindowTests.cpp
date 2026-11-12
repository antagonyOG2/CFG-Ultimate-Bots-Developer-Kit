#include "../Core/FrontendArrivalWindow.hpp"
#include <cstdio>
int main(){using cfgmenu::arrivalwindow::Window;unsigned failures=0,checks=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    Window idle;check(!idle.Due(0)&&idle.checks==0,"zero idle work");
    Window w;check(w.Start(10)&&!w.Start(11),"one armed launch");
    check(w.Due(10)&&!w.Due(10)&&!w.Due(109)&&w.checks==1,"no burst");
    check(!w.Sample(10,1,2,true)&&!w.Sample(759,1,2,true)&&w.Sample(760,1,2,true),"stable birth delay");
    check(!w.ReadyFor(3)&&w.ReadyFor(2)&&w.Due(760)&&!w.Due(760)&&w.ReadyFor(2),"other actor cannot starve the next owner callback");
    check(!w.Sample(800,1,3,true)&&!w.Sample(1600,1,3,false)&&!w.Sample(1700,1,3,true),"changing birth resets");
    check(w.Due(3000)&&!w.Due(3000)&&w.checks==3,"no catchup");
    w.Finish();check(!w.Due(9999)&&!w.Sample(9999,1,3,true)&&w.checks==3,"zero retired work");
    Window timeout;check(timeout.Start(0),"zero clock accepted");
    for(unsigned ms=0;ms<45000;ms+=100)check(timeout.Due(ms),"bounded deadline sample");
    check(!timeout.Due(45000)&&timeout.Finished()&&timeout.checks==450&&!timeout.Start(45001),"timeout cannot restart");
    Window backwards;backwards.Start(100);check(!backwards.Due(99)&&backwards.Finished(),"clock regression refuses");
    std::printf("CFG frontend arrival window: %s (%u checks, %u failures). No game access.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
