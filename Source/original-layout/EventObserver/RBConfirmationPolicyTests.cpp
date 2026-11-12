#include "../Core/RBEmoteInputPolicy.hpp"
#include <cstdio>
struct Host {
    unsigned shows=0,hides=0,warms=0,confirms=0,cancels=0;int owner=1;bool eligible=true,warm=true,cancel=true;
    cfgrbemote::Policy<int>* policy=nullptr;
    bool Source(int& out){out=owner;return true;}bool Current(int in){return in==owner;}
    bool Warm(bool){return true;}
    bool Call(int,bool show){if(show)++shows;else ++hides;return true;}
    bool WarmConfirm(bool){++warms;return warm;}
    bool CancelCurrent(int){++cancels;return cancel;}
    bool Confirm(int){++confirms;if(policy)policy->ObserveSelection();return eligible;}
};
int main(){unsigned failures=0;const auto check=[&](bool x){if(!x)++failures;};
    const cfgrbemote::Input down{true,true,true},up{true,true,false};
    for(unsigned fault=0;fault<8;++fault){cfgrbemote::Policy<int> p;Host h;h.policy=&p;
        for(unsigned i=0;i<6;++i)p.Step(h,down,true);
        check(p.phase==cfgrbemote::Phase::Open&&h.shows==1&&h.hides==1&&h.cancels==1);
        if(fault==1)p.ObserveSelection(); // native selection already happened
        if(fault==2)p.Release(); // shutdown/forced close must never confirm
        if(fault==3)h.owner=2; // replacement pawn never inherits old wheel
        if(fault==4)h.eligible=false;
        if(fault==5)h.warm=false;
        for(unsigned i=0;i<5;++i){
            auto input=up;if(fault==6)input.foreground=false;if(fault==7)input.connected=false;
            p.Step(h,input,true);
        }
        check(p.phase==cfgrbemote::Phase::Idle&&!p.Owned());
        check(h.confirms==((fault==0||fault==4)?1u:0u));
        check(h.hides==(fault==3?1u:2u));
        for(unsigned i=0;i<10;++i)p.Step(h,up,true);
        check(h.confirms<=1&&h.shows==1);
    }
    // A selection during warm-up also suppresses the fallback. ClearOld's stale
    // selection is reset immediately before opening the NEW wheel.
    {cfgrbemote::Policy<int> p;Host h;h.policy=&p;
     for(unsigned i=0;i<6;++i)p.Step(h,down,true);
     p.Step(h,up,true);p.ObserveSelection();
     for(unsigned i=0;i<4;++i)p.Step(h,up,true);
     check(!h.confirms&&h.hides==2&&!p.Owned());}
    {cfgrbemote::Policy<int> p;Host h;h.cancel=false;
     for(unsigned i=0;i<10;++i)p.Step(h,down,true);
     check(h.cancels==1&&!h.shows&&!h.hides&&!h.confirms&&!p.Owned()&&p.refused==1);}
    std::printf("Donor RB highlighted confirmation policy: %u failures; one-shot fallback, natural selection/forced close/focus/owner/eligibility preserved.\n",failures);
    return failures?1:0;
}
