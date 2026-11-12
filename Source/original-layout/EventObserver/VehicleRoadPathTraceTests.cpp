#include "../Core/VehicleRoadPathTrace.hpp"
#include <cstdio>
int main(){unsigned failures=0;
    const auto check=[&](bool ok){if(!ok)++failures;};
    cfgroad::PathTrace t;t.Begin(0x10000,42,{0,0,0},{1000,200,0},100);
    t.Sample(0x20000,43,{1100,0,0},200);check(t.segments==0&&t.state==1);
    t.Sample(0x10000,42,{500,0,0},300);check(t.segments==1&&t.state==1);
    t.Sample(0x10000,42,{1500,0,0},400);check(t.state==2&&t.segments==2&&std::fabs(t.nearest-200)<.01f&&std::fabs(t.lateral-200)<.01f);
    t.Sample(0x10000,42,{1500,900,0},500);check(t.segments==2); // completed evidence remains stable
    t.Begin(0x10000,42,{0,0,0},{1000,-300,0},100);t.Sample(0x10000,42,{2000,0,0},200);
    check(t.state==2&&std::fabs(t.lateral+300)<.01f);
    t.Begin(0x10000,42,{0,0,0},{1000,0,0},100);t.Sample(0x10000,43,{1000,0,0},200);check(t.state==4&&t.segments==0);
    t.Begin(0x10000,42,{0,0,0},{1000,0,0},100);t.Sample(0x10000,42,{9000,0,0},200);check(t.state==1&&t.segments==0);
    t.Sample(0x10000,42,{1000,0,0},30101);check(t.state==3&&t.segments==0);
    t.Begin(0x10000,0,{0,0,0},{1000,0,0},100);check(t.state==0);
    std::printf("Road landing versus driven path: %u failures; observation only.\n",failures);return failures?1:0;
}
