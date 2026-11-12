#include "../Core/OnFootBurstPolicy.hpp"
#include <cstdio>
struct Host {
    std::array<float,4> values{{450,500,450,300}};
    unsigned writes=0,failAt=0;bool current=true,ambiguous=false;
    bool Observe(std::array<float,4>& out){out=values;return current;}
    bool Exchange(unsigned i,float before,float after){
        if(!current||values[i]!=before)return false;
        ++writes;if(failAt&&writes==failAt){if(ambiguous)values[i]=after;return false;}
        values[i]=after;return true;
    }
    bool Restore(unsigned i,float before,float after){
        if(!current)return false;if(values[i]!=before)return true;values[i]=after;++writes;return true;
    }
};
int main(){unsigned failures=0,checks=0;const auto check=[&](bool x){++checks;if(!x){++failures;std::printf("FAIL %u\n",checks);}};
    {Host h;const auto baseline=h.values;cfgfootburst::Policy p;
     check(p.Begin(h,100,0x10000)&&p.Owned()&&p.readyAt==10100&&!h.writes);
     for(unsigned i=0;i<4;++i)check(p.Step(h,101+i,false));
     check(p.stage==cfgfootburst::Stage::Hold&&h.writes==4&&h.values[2]==2250);
     check(!p.Step(h,3099,false)&&!p.Begin(h,3100,0x20000));
     for(unsigned i=0;i<4;++i)check(p.Step(h,3100+i,false));
     check(!p.Owned()&&h.values==baseline&&p.restores==1&&p.readyAt==10103);
     check(!p.Begin(h,10102,0x10000)&&p.Begin(h,10103,0x10000));}
    for(unsigned interruptedAt=0;interruptedAt<5;++interruptedAt){Host h;const auto baseline=h.values;cfgfootburst::Policy p;
     check(p.Begin(h,100,0x10000));for(unsigned i=0;i<interruptedAt&&i<4;++i)p.Step(h,101+i,false);
     for(unsigned i=0;i<4&&p.Owned();++i)p.Step(h,200+i,true);
     check(!p.Owned()&&h.values==baseline&&p.starts==1&&!p.Begin(h,210,0x10000));}
    for(bool ambiguous:{false,true})for(unsigned fault=1;fault<=4;++fault){Host h;const auto baseline=h.values;cfgfootburst::Policy p;
     h.failAt=fault;h.ambiguous=ambiguous;check(p.Begin(h,100,0x10000));
     for(unsigned i=0;i<4&&p.stage==cfgfootburst::Stage::Apply;++i)p.Step(h,101+i,false);
     check(p.stage==cfgfootburst::Stage::Restore&&p.refused==1);
     for(unsigned i=0;i<4&&p.Owned();++i)p.Step(h,200+i,true);
     check(!p.Owned()&&h.values==baseline);}
    {Host h;cfgfootburst::Policy p;check(p.Begin(h,100,0x10000));for(unsigned i=0;i<4;++i)p.Step(h,101+i,false);
     h.values[2]=777;for(unsigned i=0;i<4;++i)p.Step(h,200+i,true);check(!p.Owned()&&h.values[2]==777);}
    {Host h;cfgfootburst::Policy p;check(p.Begin(h,100,0x10000));p.Step(h,101,false);h.current=false;
     check(p.Step(h,200,true)&&p.Owned()&&h.writes==1);h.current=true;
     for(unsigned i=0;i<4&&p.Owned();++i)p.Step(h,201+i,true);check(!p.Owned()&&h.values[0]==450);}
    {Host h;cfgfootburst::Policy p;check(!p.Begin(h,100,0));h.values[0]=NAN;
     check(p.Begin(h,100,0x10000)&&!p.Owned()&&!h.writes&&p.readyAt==1100);}
#if CFG_H285_CHASE_BURST
    // H2.85: target within 30 m at start -> 3 s recovery after the 3 s burst; farther or unknown keeps 7 s.
    {Host h;cfgfootburst::Policy p;float near=2900.f*2900.f,far=3100.f*3100.f;
     check(p.Begin(h,100,0x10000,true,&near)&&p.readyAt==6100&&p.chaseStarts==1);
     for(unsigned i=0;i<4;++i)p.Step(h,101+i,false);for(unsigned i=0;i<4;++i)p.Step(h,3100+i,false);
     check(!p.Owned()&&p.readyAt==6103&&p.Begin(h,6103,0x10000,true,&far)&&p.readyAt==16103&&p.chaseStarts==1);}
    {Host h;cfgfootburst::Policy p;check(p.Begin(h,100,0x10000)&&p.readyAt==10100&&!p.chaseStarts);}
    // H2.90 audit #2: unknown (-1) is never near; the distance is the one the host proves inside Observe, not the queued one.
    {Host h;cfgfootburst::Policy p;float unknown=-1.f;check(p.Begin(h,100,0x10000,true,&unknown)&&p.readyAt==10100&&!p.chaseStarts);}
    {struct Fresh:Host{float* out=nullptr;float proved=0;bool Observe(std::array<float,4>& v){*out=proved;return Host::Observe(v);}};
     for(const bool nearAtStart:{false,true}){Fresh h;cfgfootburst::Policy p;float distance=(nearAtStart?3100.f:2900.f)*(nearAtStart?3100.f:2900.f);
      h.out=&distance;h.proved=(nearAtStart?2900.f:3100.f)*(nearAtStart?2900.f:3100.f); // queued 29 m observed 31 m, and the reverse
      check(p.Begin(h,100,0x10000,true,&distance)&&p.readyAt==(nearAtStart?6100u:10100u)&&p.chaseStarts==(nearAtStart?1u:0u));}}
#endif
    std::printf("On-foot5x burst/cooldown/conditional restoration: %u checks,%u failures; synthetic only.\n",checks,failures);
    return failures?1:0;
}
