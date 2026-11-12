#include "../Core/VehiclePursuitPolicy.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};
    cfgvehicle::MotionSample sample;cfgspawn::Vec3 measured{};
    check(!sample.Observe(1,2,100,{0,0,0},measured));
    check(sample.Observe(1,2,2300,{2200,0,0},measured)&&std::fabs(measured.x-1000.f)<.01f);
    check(sample.Observe(1,2,4500,{2200,0,0},measured)&&measured.x==0); // stopped clears motion
    check(!sample.Observe(1,3,6700,{4400,0,0},measured)); // changed driver
    check(!sample.Observe(2,3,8900,{6600,0,0},measured)); // changed vehicle
    check(!sample.Observe(2,3,13000,{8800,0,0},measured)); // stale sample
    check(!sample.Observe(2,3,15200,{1000000,0,0},measured)); // teleported, not velocity
    cfgvehicle::Pursuit lease;check(!lease.Active(10));lease.Renew(10);check(lease.Active(7009));check(!lease.Active(7010));
    lease.Renew(UINT64_MAX-1);check(lease.until==UINT64_MAX);
    cfgvehicle::TailIntercept tail;
    const auto observe=[&](std::uint64_t now){return tail.Observe(1,2,now,true,{0,0,0},{1000,0,0},{-500,0,0});};
    check(!observe(0));check(!observe(2500));check(!observe(4999));check(observe(5000));
    tail.Attempted(5000);check(tail.next==10000);check(!observe(5001));check(!observe(7500));check(observe(10001));
    tail.Vacant(9);check(tail.behind);tail.Vacant(1);check(!tail.behind&&tail.next==10000);
    check(!observe(16000));check(!tail.Observe(1,2,18000,false,{},{},{-500,0,0})&&!tail.behind);
    check(!observe(20000));check(!tail.Observe(1,3,22000,true,{},{1000,0,0},{-500,0,0}));
    check(!observe(24000));check(!observe(28000)); // long sampling gap resets continuous tail
    check(!tail.Observe(1,2,29000,true,{},{1000,0,0},{500,0,0})&&!tail.behind);
    check(!tail.Observe(1,2,30000,true,{},{1000,0,0},{NAN,0,0}));
    cfgspawn::Vec3 point{};check(cfgvehicle::Lead({1,2,3},{1000,0,0},point)&&point.x==1001&&point.y==2&&point.z==3);
    check(cfgvehicle::Landing({0,0,10},{0,0,0},point)&&point.z==90.f);
    check(!cfgvehicle::Landing({0,0,0},{601,0,0},point));
    check(!cfgvehicle::Landing({0,0,0},{0,0,701},point));
    check(!cfgvehicle::Landing({0,0,0},{NAN,0,0},point));
    check(!cfgvehicle::Lead({1,2,3},{0,0,0},point));check(!cfgvehicle::Lead({NAN,0,0},{1000,0,0},point));
    check(!cfgvehicle::Lead({0,0,0},{10001,0,0},point));cfgvehicle::Cooldown cooldown;
    check(!cooldown.Due(0,5000.f*5000.f));check(cooldown.Due(0,5001.f*5001.f));
    cooldown.Attempted(0);check(!cooldown.Due(19999,1.e8f));check(cooldown.Due(20000,1.e8f));check(!cooldown.Due(20000,NAN));
    std::printf("CFG car lead/50m Morph/20s denial cooldown/native packets: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;}
