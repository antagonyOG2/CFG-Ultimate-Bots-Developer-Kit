#include "../Core/ConvergencePublication.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <array>
#include <algorithm>
#include <cassert>
#include <cstdio>
struct Host {
    std::array<std::uintptr_t,2> objects{};std::array<unsigned,9> values{};
    unsigned calls=0,cap=12,pairs=0;bool current=true,refuse=false;
    bool Current(){return current;}
    bool SetObject(unsigned k,std::uintptr_t v){++calls;if(refuse||calls>cap)return false;objects[k]=v;return true;}
    bool GetObject(unsigned k,std::uintptr_t& v){++calls;if(calls>cap)return false;v=objects[k];return true;}
    bool SetBool(unsigned k,unsigned v){++calls;if(refuse||calls>cap)return false;values[k]=v;return true;}
    bool GetBool(unsigned k,unsigned& v){++calls;if(calls>cap)return false;v=values[k];return true;}
};
int main(){using namespace cfgconvergence;using R=Publication::Result;
    assert(CombatAllowed(false,false,false)); // trance/kneel without committed interaction
    assert(!CombatAllowed(true,false,false));
    assert(!CombatAllowed(false,true,false));
    assert(!CombatAllowed(false,false,true));
    {FeatureTurns turns;unsigned observed=0,helpers=0;
     for(unsigned i=0;i<100;++i){const auto work=turns.Choose(true,true);
        if(work==FeatureTurns::Work::Sweater)++observed;else if(work==FeatureTurns::Work::Helpers)++helpers;}
     assert(observed==50&&helpers==50); // active convergence cannot swallow all sweater phases
     assert(turns.Choose(false,true)==FeatureTurns::Work::Helpers);
     assert(turns.Choose(false,false)==FeatureTurns::Work::Other);}
    {PublicationContinuation resume;
        assert(!resume.Take(1000,false,true));
        assert(!resume.Take(1000,true,false)); // due hunt/release/camera always wins
        assert(resume.Take(1000,true,true));
        assert(!resume.Take(1099,true,true)&&resume.Take(1100,true,true));
        assert(resume.Take(5000,true,true)&&!resume.Take(5000,true,true)); // no catch-up burst
        assert(!resume.Take(6000,false,true)&&resume.next==0);
        assert(!resume.Take(UINT64_MAX,true,true));}
    {PublicationContinuation share;
        assert(share.Take(1000,true,false,true));
        assert(!share.Take(1100,true,false,true)); // owed vehicle turn
        assert(share.Take(1200,true,false,true));
        assert(!share.Take(1300,true,false,false)); // extraction/hood/Morph cannot yield
        assert(!share.Take(1400,false,false,true));}
    { // Production scheduler: idle continuations finish a transaction without
      // consuming due hunt/car/release slots, even with delayed callbacks.
        for(const auto step:{25u,50u,125u}){
            cfgbundle::Schedule schedule;assert(schedule.Start(0,0));schedule.helpersActive=false;
            Publication transaction;PublicationContinuation resume;Host host;
            assert(transaction.Begin(0,Action::Publish,511,0x10000,0x20000));
            unsigned groups=0,hunts=0,cars=0;std::uint64_t finished=0;
            for(std::uint64_t now=0;now<5000;now+=step){
                const auto lane=schedule.Due(now);
                if(lane==cfgbundle::Lane::Hunt)++hunts;
                if(lane==cfgbundle::Lane::Vehicle)++cars;
                if(resume.Take(now,transaction.Active(),lane==cfgbundle::Lane::None)){
                    host.calls=0;++groups;const auto result=transaction.Step(host);assert(host.calls<=4);
                    if(result==R::Complete){finished=now;transaction.Reset();}
                }
            }
            assert(groups==5&&finished&&finished<2500&&hunts>=4&&cars>=3);
        }
    }
    { // Compose the actual round-robin policy and transaction with busy AI lanes.
      // This covers scheduling, not native travel/schema/weapon compatibility.
      for(const auto carCadence:{1100u,419u})for(const auto step:{25u,50u,125u})for(const auto count:{4u,9u}){
        cfgbundle::Schedule schedule;assert(schedule.Start(0,count));schedule.helpersActive=false;
        Policy policy;policy.Acquired();Publication transaction;PublicationContinuation resume;
        FeatureTurns turns;std::array<Host,MaximumBots> hosts{};
        std::array<bool,MaximumBots> accepted{};unsigned published=0,hunts=0,cars=0,releases=0,sweaters=0,fears=0,travel=0;
        unsigned fearPending=0;bool travelTurn=false;std::uint64_t completed=0,travelAt=0;
        for(std::uint64_t now=0;now<60000;now+=step){
          if(now==step*20||now==step*80)schedule.Pressed(now);
          auto lane=schedule.Due(now);
          const bool acceptedCar=carCadence==419&&lane==cfgbundle::Lane::Vehicle&&!schedule.pressed;
          const bool continuation=resume.Take(now,transaction.Active(),lane==cfgbundle::Lane::None&&!schedule.pressed,acceptedCar);
          if(continuation){if(acceptedCar)schedule.vehicleAt=now+100;lane=cfgbundle::Lane::Convergence;}
          if(lane==cfgbundle::Lane::Hunt)++hunts;
          if(lane==cfgbundle::Lane::Vehicle){++cars;schedule.vehicleAt=now+carCadence;}
          if(lane==cfgbundle::Lane::Release){++releases;schedule.Released();}
          if(lane!=cfgbundle::Lane::Convergence)continue;
          if(!continuation){
            if(fearPending){--fearPending;++fears;continue;}
            if(now>=travelAt){travelTurn=!travelTurn;if(travelTurn){travelAt=now+333;++travel;continue;}}
            (void)turns.Damage(); // warm metadata: does not consume a slice
            if(turns.Sweater()){++sweaters;continue;}
          }
          if(!transaction.Active()){
            unsigned index=0;if(!policy.Pick(now,count,index))continue;
            const auto action=policy.Decide(index,true,true,now);if(action==Action::None)continue;
            assert(transaction.Begin(index,action,511,0x10000,0x20000+index*0x100));continue;
          }
          const auto index=transaction.Index();auto& host=hosts[index];host.calls=0;
          const auto result=transaction.Step(host);assert(result!=R::Refused&&host.calls<=4);
          if(result==R::Complete){
            assert(host.values[Fight]==1&&host.values[Flee]==0&&host.objects[Jason]==0x10000);
            policy.Confirm(index,transaction.CurrentAction(),now);transaction.Reset();++fearPending;
            if(!accepted[index]){accepted[index]=true;++published;if(published==count)completed=now;}
          }
        }
        std::printf("Fairness probe count%u step%u car%u finished%llu published%u hunt%u car%u releases%u\n",count,step,carCadence,static_cast<unsigned long long>(completed),published,hunts,cars,releases);std::fflush(stdout);
        assert(published==count&&completed&&completed<30000&&hunts>=40&&cars>=30&&releases==2);
        assert(travel>=count&&fears>=count&&sweaters>=count);
        std::printf("Composed convergence %u helpers/%u ms callback/%u ms car: all armed publications by %llu ms; hunt/car/release progress retained\n",count,step,carCadence,static_cast<unsigned long long>(completed));
      }
    }
    {FeatureTurns turns;unsigned damage=0,sweater=0,convergence=0;
        for(unsigned i=0;i<400;++i){if(turns.Damage()){++damage;continue;}
            if(turns.Sweater())++sweater;else ++convergence;}
        assert(damage==200&&sweater==100&&convergence==100);
        // Once warm, Damage false/true no longer consumes the callback.
        sweater=convergence=0;for(unsigned i=0;i<100;++i){(void)turns.Damage();if(turns.Sweater())++sweater;else ++convergence;}
        assert(sweater==50&&convergence==50);}
    Publication p;Host h;assert(p.Begin(0,Action::Publish,511,0x10000,0x20000));
    assert(p.Step(h)==R::Pending&&h.calls==4&&h.objects[0]==0x20000&&h.objects[1]==0x10000);
    unsigned peak=h.calls;
    for(unsigned n=0;n<4;++n){h.calls=0;const auto r=p.Step(h);peak=(std::max)(peak,h.calls);assert(h.calls<=4);
        assert(r==(n==3?R::Complete:R::Pending));
        if(n==0){h.values[Fight]=0;h.objects[Jason]=0;} // native policy changes old values, not failed publication
    }
    p.Reset();h={};assert(p.Begin(1,Action::Publish,(1u<<Weapon)|(1u<<Jason)|(1u<<Fight)|(1u<<Flee),0x10000,0x20000));
    assert(p.Step(h)==R::Pending);h.calls=0;assert(p.Step(h)==R::Complete&&h.calls==4&&h.values[Fight]==1&&h.values[Flee]==0);
    p.Reset();h={};assert(p.Begin(1,Action::Publish,(1u<<Jason)|(1u<<Fight)|(1u<<Flee),0x10000,0x20000));
    assert(p.Step(h)==R::Pending&&h.calls==2);h.calls=0;assert(p.Step(h)==R::Complete&&h.calls==4);
    p.Reset();h={};h.refuse=true;assert(p.Begin(0,Action::Publish,511,0x10000,0x20000));
    assert(p.Step(h)==R::Refused&&p.failureKey==Weapon&&h.calls==1);
    p.Reset();h={};h.current=false;assert(p.Begin(0,Action::Publish,511,0x10000,0x20000));
    assert(p.Step(h)==R::Refused&&!h.calls);
    Policy policy;policy.Acquired();policy.Retry(0,1000);assert(!policy.bots[0].unsupported);
    assert(policy.Decide(0,true,true,5999)==Action::None&&policy.Decide(0,true,true,6000)==Action::Publish);
    policy.Continue(2,6000);unsigned index=0;assert(!policy.Pick(6249,4,index)&&policy.Pick(6250,4,index)&&index==2);
    policy.Refuse(0);assert(policy.Decide(0,true,true,10000)==Action::None);
    p.Reset();h={};assert(p.Begin(0,Action::Clear,511,0x10000,0));
    assert(p.Step(h)==R::Pending&&!h.calls);
    for(unsigned n=0;n<3;++n){h.calls=0;const auto r=p.Step(h);assert(r==(n==2?R::Complete:R::Pending));assert(h.calls<=4);}
    assert(h.values[Fight]==0&&h.values[SeekWeapon]==1&&h.values[Flee]==1);
    policy.bots[1].published=true;policy.bots[1].refreshAt=15000;
    assert(policy.Decide(1,true,false,10000)==Action::Clear); // lost weapon must not wait for refresh
    std::printf("CFG convergence publication PASS: immediate readback, supported optional keys, policy churn not permanent failure, <=%u native calls/group\n",peak);
}
