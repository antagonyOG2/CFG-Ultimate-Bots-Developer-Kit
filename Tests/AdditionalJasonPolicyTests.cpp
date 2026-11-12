#include "../Core/AdditionalJasonPolicy.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failed=0;const auto check=[&](bool value){++checks;if(!value)++failed;};
    using namespace cfgadditional;
    check(cfgbundle::PresentationDispatch(true,false,false,false,false)==cfgbundle::HostedPresentationDispatch::Primary);
    check(cfgbundle::PresentationDispatch(false,false,false,false,false)==cfgbundle::HostedPresentationDispatch::None);
    check(cfgbundle::PresentationDispatch(true,true,false,false,false)==cfgbundle::HostedPresentationDispatch::None);
    check(cfgbundle::PresentationDispatch(true,false,true,true,false)==cfgbundle::HostedPresentationDispatch::Extra);
    check(cfgbundle::PresentationDispatch(true,false,true,false,false)==cfgbundle::HostedPresentationDispatch::None);
    check(!cfgbundle::AdditionalJasonLaneEnabled(false,false)&&cfgbundle::AdditionalJasonLaneEnabled(true,false)&&
        cfgbundle::AdditionalJasonLaneEnabled(false,true));
    {Targets t;t.Sample(3,true,9000.f*9000.f,100);check(t.cursor==1);
     t.Sample(3,true,7000.f*7000.f,200);t.Sample(3,true,2000.f*2000.f,300);check(t.selected==2&&t.cursor==0&&t.next==1211);
     t.Sample(3,true,1,1300);t.Sample(3,false,1,1400);t.Sample(3,true,8001.f*8001.f,1500);check(t.selected==0);
     t.Sample(2,true,1,1600);check(t.cursor==1&&t.count==2);t.Sample(2,true,NAN,1700);check(t.selected==0);}
    {Targets t;t.Sample(2,true,500.f*500.f,100);t.Sample(2,true,600.f*600.f,200);check(t.selected==0);
     t.Sample(2,true,900.f*900.f,1200);t.Sample(2,true,500.f*500.f,1300);check(t.selected==1);
     t.Sample(2,false,1,2300);t.Sample(2,true,20000.f*20000.f,2400);check(t.selected==1);}
    {Policy p;check(p.CombatDue(1000));p.CombatDispatched(1000);
     check(p.pressed&&p.releaseAt==1150&&!p.CombatDue(1911));p.pressed=false;
     check(!p.CombatDue(1910)&&p.CombatDue(1911));}
    {unsigned selected[4]{};for(unsigned seed=1;seed<=400;++seed){Policy p;unsigned winner=0;
      for(unsigned i=0;i<4;++i)if(p.Choose(seed))winner=i;
      ++selected[winner];check(p.choices==4);}
     for(auto count:selected)check(count>60&&count<140);}
    {cfgspawn::SpawnAIParameters p;
     check(BuildPacket(0x10000,0x20000,{100,200,300},p)&&p.noCollisionFail==1&&!p.behaviorTree&&!p.result);
     check(p.world==0x10000&&p.pawnClass==0x20000&&p.location.z==300);
     check(!BuildPacket(0,0x20000,{},p)&&!p.noCollisionFail);
     check(!BuildPacket(0x10000,0x20000,{NAN,0,0},p));}
    {Policy p;check(LifecycleProved);check(!p.Request(true,true,false,1000));
     check(!p.BirthPending()&&!p.Due(2000)&&!p.requests&&!p.births);}
    {Policy p;check(!p.Request(true,false,true,1000));check(p.stage==Stage::Idle);
     check(!p.Request(true,true,false,1100));check(!p.Request(true,true,true,1200)); // held key never rearms
     check(!p.Request(false,true,true,1300));check(p.Request(true,true,true,1400));
     check(p.requests==1&&p.stage==Stage::Select&&p.Due(1400));
     for(unsigned i=0;i<9;++i){p.Advance(1500+i*100);check(!p.Due(1500+i*100));}
     check(p.stage==Stage::Active&&p.births==1&&!p.BirthPending());
     p.Request(false,true,true,3000);check(!p.Request(true,true,true,4000)); // one extra / two total
     check(p.requests==1&&p.births==1);}
    for(unsigned phase=0;phase<9;++phase){Policy p;p.Request(true,true,true,1000);
        for(unsigned i=0;i<phase;++i)p.Advance(1100+i*100);
        p.Fail(phase!=0);check(p.stage==(phase?Stage::Indeterminate:Stage::Refused));
        check(!p.Due(10000));p.Request(false,true,true,11000);check(!p.Request(true,true,true,12000));}
    std::printf("Additional Jason independent birth/edge/ambiguity policy: %u checks, %u failures. No game access.\n",checks,failed);
    return failed?1:0;
}
