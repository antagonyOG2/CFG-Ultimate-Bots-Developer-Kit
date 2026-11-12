#include "../Core/UrinationPlaybackPolicy.hpp"
#include <cstdio>
int main(){using namespace cfgurination;unsigned failures=0;
    const auto check=[&](bool okay){if(!okay)++failures;};PlaybackPolicy p;
    check(!p.Due(30000,true));p.Observe(1,2,3,true,100);
    check(!p.Due(349,true)&&p.Due(350,true)&&!p.Due(25099,false)&&p.Due(25100,false));
    check(p.Consume(350,true)&&!p.Active()&&!p.Consume(351,true));p.Observe(1,2,3,true,1000);check(!p.Due(30000,false));
    p.Observe(1,2,3,false,1100);p.Observe(1,2,3,true,1200);check(!p.Due(1449,true)&&p.Due(1450,true));
    p.Observe(1,2,4,true,1500);check(!p.Due(1600,true));p.Observe(2,2,4,true,1700);check(!p.Due(1800,true));
    p.Observe(2,2,0,true,1800);check(!p.Active());
    MovementSample sample;check(!sample.Recent(0));sample.Observe(true,true,7849,0,100,7849);check(!sample.Recent(100));
    sample.Observe(true,true,-32768,0,100,7849);check(sample.Recent(100)&&sample.Recent(199)&&!sample.Recent(200)&&!sample.Recent(99));
    sample.Observe(false,true,30000,30000,200,7849);check(!sample.Recent(200));
    sample.Observe(true,false,30000,30000,201,7849);check(!sample.Recent(201));
    sample.Observe(true,true,0,-7850,202,7849);check(sample.Recent(202));
    {PlaybackPolicy repeat;repeat.Observe(1,2,3,true,100);check(repeat.Consume(25100,false));
     check(!repeat.NewNativePlayback(false,1,2)&&!repeat.NewNativePlayback(true,2,2));
     repeat.Observe(1,2,3,true,26000);check(!repeat.Active()&&!repeat.Consume(52000,false));
     check(repeat.NewNativePlayback(true,2,3));repeat.Observe(1,2,3,true,53000);
     check(repeat.Active()&&!repeat.Due(53249,true)&&repeat.Consume(53250,true));
     check(!repeat.Consume(90000,true));
     check(repeat.NewNativePlayback(true,3,4));repeat.Observe(1,2,3,true,100000);
     check(repeat.ConsumeWheel()&&!repeat.ConsumeWheel()&&!repeat.Consume(125000,false));}
    std::printf("Urination native cancellation policy: %u failures; movement250ms/expiry25s, exact replacement, positive owner, one cancel per verified native playback including repeat plays.\n",failures);
    return failures?1:0;
}
