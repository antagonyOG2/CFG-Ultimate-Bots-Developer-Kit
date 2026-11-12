#include "ConvergenceFearObservation.hpp"
#define main CfgFearFixtureResourceTestMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    Fixture f;const auto type=f.Object(),manager=f.Object(type);
    const auto& binding=cfghook::disk::Types[cfghook::disk::Fear];
    f.Put(f.module+binding.reflection,type);f.Put(f.module+binding.constructor,type);
    f.Put(f.bot+cfghook::disk::FearComponent,manager);f.Put(manager+0x20,f.bot);
    f.Put(manager+cfghook::disk::FearValue,91.f);
    cfgconvergencefear::Evidence e;
    const auto capture=[&](Fixture& x){x.reads=0;return cfgconvergencefear::Capture(x,x.objects,x.module,x.bot,e);};
    CHECK(capture(f)&&e.amount==91&&e.manager.serial==0);const auto peak=f.reads;
    for(unsigned n=0;n<8;++n){auto bad=f;
        if(n==0)bad.Put(manager+0x20,bad.pawn);
        if(n==1)bad.Put(manager+cfgprobe::ObjectClass,bad.botClass);
        if(n==2)bad.Put(bad.module+binding.constructor,bad.botClass);
        if(n==3)bad.Put(manager+cfghook::disk::FearValue,NAN);
        if(n==4)bad.Put(manager+cfghook::disk::FearValue,-1.f);
        if(n==5)bad.Put(bad.objects.data+static_cast<std::uintptr_t>(e.manager.index)*24+8,std::uint32_t{0x20000000});
        if(n==6){bad.trigger=bad.bot+cfghook::disk::FearComponent;bad.mutation=bad.trigger;bad.newPointer=bad.pawn;bad.triggerAt=2;}
        if(n==7)bad.cap=10;
        CHECK(!capture(bad));CHECK(capture(f));
    }
    f.Put(manager+cfghook::disk::FearValue,0.f);CHECK(capture(f)&&e.amount==0);
    CHECK(peak+1024<1536);
    std::printf("Convergence donor fear observation: %u reads +1024 outer reserve; %d failures, no game access\n",peak,failures);
    return failures?1:0;
}
