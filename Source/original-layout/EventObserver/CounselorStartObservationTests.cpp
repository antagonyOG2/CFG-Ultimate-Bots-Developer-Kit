#define main ExistingKillerStartTests
#include "KillerStartObservationTests.cpp"
#undef main
#include "CounselorStartObservation.hpp"
#include "../Core/RandomCounselorPolicy.hpp"
int main(){unsigned checks=0,failed=0,peak=0;auto check=[&](bool value,const char* text){++checks;if(!value){++failed;std::printf("FAIL %s\n",text);}};
    const auto prepare=[](Fixture& f){f.reader.Set(f.Module+cfgcounselorstart::ClassCache,f.Type);f.reader.Set(f.Module+cfgcounselorstart::ConstructorCache,f.Type);};
    const auto finish=[&](Fixture& f,cfgcounselorstart::Observation& o){
        for(unsigned i=0;i<20&&o.CurrentState()==cfgcounselorstart::State::Scanning;++i){
            const auto before=f.reader.reads;if(!o.Step(f.reader,f.host))return false;
            peak=(std::max)(peak,f.reader.reads-before);
        }return o.CurrentState()==cfgcounselorstart::State::Ready;
    };
    {Fixture f;prepare(f);cfgcounselorstart::Observation o;
     check(o.Begin(f.reader,f.host,f.Module,f.World)&&finish(f,o)&&o.Count()==1,"paced authored start collection");
     cfgcounselorstart::Marker m{};check(o.Current(f.reader,f.host,0,m)&&m.actor==f.Actor,"current authored actor");
     constexpr std::uintptr_t moved=0x290000;for(unsigned i=0;i<33;++i)f.reader.Set(moved+8*i,i==20?f.Actor:std::uintptr_t{});
     f.reader.Set(f.Level+0xA0,cfgkillerstart::Header{moved,33,64});f.reader.retiredBegin=f.Actors;f.reader.retiredEnd=f.Actors+264;
     check(o.Current(f.reader,f.host,0,m)&&!f.reader.retiredReads,"ready marker freshly follows relocated array without stale dereference");
     f.reader.Set(moved+160,std::uintptr_t{});check(!o.Current(f.reader,f.host,0,m)&&!m.actor,"removed start not adopted");}
    {Fixture f;prepare(f);f.reader.Set(f.Table+16,int{0});cfgcounselorstart::Observation o;cfgcounselorstart::Marker m{};
     check(o.Begin(f.reader,f.host,f.Module,f.World)&&finish(f,o)&&o.Current(f.reader,f.host,0,m)&&!m.slot.serial,"zero serial strong source");
     f.reader.Set(f.Table+16,int{5});check(o.Current(f.reader,f.host,0,m)&&m.slot.serial==5,"fresh zero to assigned observation");
     f.reader.Set(f.Table+16,int{6});check(!o.Current(f.reader,f.host,0,m),"recycled start refused");}
    for(unsigned fault=0;fault<5;++fault){Fixture f;prepare(f);cfgcounselorstart::Observation o;
        check(o.Begin(f.reader,f.host,f.Module,f.World),"fault begin");
        if(fault==0)f.host.current=false;if(fault==1)f.reader.Set(f.Module+cfgcounselorstart::ConstructorCache,f.Other);
        if(fault==2)f.reader.Set(f.Level+cfgkill::disk::OwningWorld,f.Other);if(fault==3)f.reader.Set(f.Actor+0x20,f.World);
        if(fault==4)f.reader.Set(f.Type+0xF8,f.Actor);
        check(!finish(f,o),"changed context/class/world/Outer/CDO refuses");}
    {Fixture f;prepare(f);f.reader.Set(f.Actors+8*21,f.Actor);cfgcounselorstart::Observation o;
     check(o.Begin(f.reader,f.host,f.Module,f.World)&&finish(f,o)&&o.Count()==1,"duplicate actor cells do not weight random starts");}
    for(unsigned seed=0;seed<100;++seed){cfgbundle::RandomCounselors p;check(p.Begin(32,seed),"shuffle begin");unsigned mask=0;
        for(unsigned i=0;i<p.count;++i)mask|=1u<<p.order[i];check(mask==0xFFFFFFFFu,"every roster entry exactly once");
        check(!p.Begin(32,seed)&&!p.ClassAllowed(0,1)&&!p.ClassAllowed(1,1),"never human or null template");
        check(p.LocationAllowed({3000,0,0},{})&&!p.LocationAllowed({1999,0,0},{}),"minimum separation from human");
        check(p.Confirm(2,{3000,0,0})&&!p.ClassAllowed(2,1)&&p.ClassAllowed(3,1),"helper class uniqueness");
        check(!p.LocationAllowed({3000,1499,0},{})&&p.LocationAllowed({3000,1500,0},{}),"helpers are dispersed");}
    check(peak<512,"bounded one-shot discovery reads");std::printf("Counselor start/random roster: %u checks, %u failures, peak %u reads. No game access.\n",checks,failed,peak);return failed?1:0;
}
