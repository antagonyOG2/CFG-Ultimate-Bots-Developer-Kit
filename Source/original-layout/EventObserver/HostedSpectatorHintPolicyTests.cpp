#include "../Core/HostedSpectatorHintPolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;auto check=[&](bool v){++checks;if(!v)++failures;};
    cfghostedlabel::Hint out{};const cfghostedlabel::Evidence good{true,true,true,true,0x1000,0x2000};
    check(cfghostedlabel::Publish(good,out)&&out.widget==good.widget&&out.function==good.function);
    check(!cfghostedlabel::Publish({false,true,true,true,good.widget,good.function},out)&&!out.widget);
    check(!cfghostedlabel::Publish({true,false,true,true,good.widget,good.function},out));
    check(!cfghostedlabel::Publish({true,true,false,true,good.widget,good.function},out));
    check(!cfghostedlabel::Publish({true,true,true,false,good.widget,good.function},out));
    check(!cfghostedlabel::Publish({true,true,true,true,0,good.function},out));
    check(!cfghostedlabel::Publish({true,true,true,true,good.widget,0},out));
    // The capture can run while LocalPawn is null; publication has no pawn input.
    check(cfghostedlabel::Publish(good,out));
    std::uint64_t next=0;check(!cfghostedlabel::RefreshDue(100,false,next)&&next==0);
    check(cfghostedlabel::RefreshDue(100,true,next)&&next==1100); // initial HUD absent; bounded retry scheduled
    check(!cfghostedlabel::RefreshDue(1099,true,next)&&next==1100);
    check(cfghostedlabel::RefreshDue(1100,true,next)&&next==2100); // terminal later: reacquire and publish
    check(!cfghostedlabel::RefreshDue(2100,false,next));
    std::printf("Hosted spectator hint policy: %u checks, %u failures (read-only metadata only; late terminal refresh is <=1Hz).\n",checks,failures);
    return failures?1:0;
}
