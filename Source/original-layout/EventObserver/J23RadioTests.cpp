#define CFG_RADIO_SOURCE_FIXTURE_ONLY
#include "RadioObjectiveSourceTests.cpp"
#undef CFG_RADIO_SOURCE_FIXTURE_ONLY
int main(){unsigned checks=0,failed=0;const auto check=[&](bool b){++checks;if(!b){++failed;std::printf("FAIL retry%u\n",checks);}};
 for(unsigned reason=0;reason<9;++reason)for(unsigned retry=0;retry<34;++retry){
  check(cfgradioretry::Allowed(true,false,reason,retry,1000)==(reason==5?retry<32:retry<8&&(reason==2||reason==4)));
  check(!cfgradioretry::Allowed(true,true,reason,retry,1000));check(!cfgradioretry::Allowed(true,false,reason,retry,90000));
 }
 check(!cfgradioretry::Allowed(false,false,5,8,8000));check(cfgradioretry::Allowed(true,false,5,8,8000));
 RadioFixture f;unsigned retries=0;std::uint64_t now=0;
 for(unsigned mutation=0;mutation<10;++mutation){
  f.radio={};f.r.reads=256;f.radio.Step(f.r,f.registry,f.Module,f.World,f.State);
  // First complete scan finds the owned radio; simulate a later actor-array
  // relocation before the final/current/publication proof, never accept it.
  const auto old=f.radio.actors[0];f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,17,18});
  f.r.reads=0;check(!f.radio.Current(f.r,f.registry));check(!f.Publish()&&!f.exchanges&&!f.getters);
  check(cfgradioretry::Allowed(true,false,5,retries,now));++retries;now+=cfgradioretry::Delay(true,5,retries);
  f.r.Put(f.Level+cfgradarworld::disk::LevelActors,old);
 }
 f.radio={};f.Acquire();check(f.radio.stage==2&&f.Publish()&&f.exchanges==1&&f.getters==1);
 check(!cfgradioretry::Allowed(true,true,5,retries,now)); // no post-publication replay
 check(cfgradioretry::Delay(true,5,4)==2500&&cfgradioretry::Delay(false,5,4)==1000);
 std::printf("J23 bounded read-only radio streaming retry: %u checks,%u failures; ten changed headers refused, fresh stable scan publishes once, native ambiguity never retried.\n",checks,failed);return failed?1:0;}
