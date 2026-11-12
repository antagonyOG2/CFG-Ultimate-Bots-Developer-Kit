#define main ShackSourceFixtureMain
#include "ShackTrapTests.cpp"
#undef main
#include "RadioObjectiveSource.hpp"
struct RadioFixture:Fixture {
    static constexpr std::uintptr_t State=0x610000,RadioClass=0x620000,Radio=0x630000;
    cfgradioobjective::Survey radio;unsigned exchanges=0,getters=0,current=0,mutation=0;bool authority=true,attempted=false,ready=true;
    RadioFixture(){r.Put(Module+cfgradarobjective::disk::SCCBRadioReflection,RadioClass);
        r.Put(Module+cfgradarobjective::disk::SCCBRadioConstructor,RadioClass);r.Put(RadioClass+0x30,std::uintptr_t{});
        r.Put(State+cfgradioobjective::disk::Cache,std::uintptr_t{});
        r.Put(Radio+cfgprobe::ObjectClass,RadioClass);r.Put(Radio+0x20,Level);Slot(Radio,2);r.Put(Actors+8*16,Radio);}
    void Acquire(){for(unsigned i=0;i<130&&!radio.Done();++i){r.reads=256;radio.Step(r,registry,Module,World,State);peak=(std::max)(peak,r.reads);} }
    bool Current(){++current;for(unsigned i=0;i<200;++i){std::uintptr_t value=0;if(!r.Read(Level+cfgkill::disk::OwningWorld,value)||value!=World)return false;}return authority;}
    bool Prepare(){return ready&&Current();} // model the actual native getter admission before CAS
    bool Exchange(std::uintptr_t at,std::uintptr_t value){if(attempted)return false;attempted=true;++exchanges;
        std::uintptr_t old=0;if(!r.Read(at,old)||old)return false;r.Put(at,value);
        if(mutation==1)r.Put(Level+cfgkill::disk::OwningWorld,State);return true;}
    bool Getter(std::uintptr_t& out){++getters;if(!r.Read(State+cfgradioobjective::disk::Cache,out))return false;
        if(mutation==2)out=Actor;if(mutation==3)authority=false;return true;}
    bool Publish(){r.reads=0;const bool okay=cfgradioobjective::Publish(r,registry,radio,*this);peak=(std::max)(peak,r.reads);return okay;}
};
#ifndef CFG_RADIO_SOURCE_FIXTURE_ONLY
int main(){unsigned failed=0,peak=0;const auto checkAt=[&](bool value,int line){if(!value){++failed;std::printf("Radio failure line%d\n",line);}};
#define check(value) checkAt(value,__LINE__)
 {RadioFixture f;f.Acquire();check(f.radio.stage==2&&f.radio.radio==f.Radio);check(f.Publish()&&f.exchanges==1&&f.getters==1);
  check(!f.Publish()&&f.exchanges==1);peak=(std::max)(peak,f.peak);}
 {RadioFixture f;f.r.Put(f.State+cfgradioobjective::disk::Cache,f.Radio);f.Acquire();check(f.radio.cached&&f.Publish()&&f.exchanges==0);}
 {RadioFixture f;f.Acquire();f.ready=false;check(!f.Publish()&&!f.exchanges&&!f.getters);}
 for(unsigned mutation=1;mutation<=3;++mutation){RadioFixture f;f.Acquire();f.mutation=mutation;check(!f.Publish()&&f.exchanges==1);
  check(!f.Publish()&&f.exchanges==1);}
 for(unsigned bad=0;bad<7;++bad){RadioFixture f;f.Acquire();
  if(bad==0)f.r.Put(f.Radio+0x20,f.Actor);
  if(bad==1)f.r.Put(f.registry.data+24*2+16,int{100});
  if(bad==2)f.r.Put(f.State+cfgradioobjective::disk::Cache,f.Actor);
  if(bad==3)f.r.Put(f.Actors+8*16,f.Actor);
  if(bad==4)f.r.Put(f.Module+cfgradarobjective::disk::SCCBRadioConstructor,f.CabinClass);
  if(bad==5)f.authority=false;
  if(bad==6)f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,17,18});
  check(!f.Publish()&&!f.exchanges&&!f.getters);}
 {RadioFixture f;f.r.Put(f.Radio+cfgprobe::ObjectClass,f.CabinClass);f.Acquire();check(f.radio.stage==3&&f.radio.failure==7);}
 {RadioFixture f;const auto duplicate=f.Radio+0x10000;f.r.Put(duplicate+cfgprobe::ObjectClass,f.RadioClass);
  f.r.Put(duplicate+0x20,f.Level);f.Slot(duplicate,3);f.r.Put(f.Actors+8*15,duplicate);
  f.Acquire();check(f.radio.stage==3&&f.radio.failure==6);}
 {RadioFixture f;f.r.Put(f.State+cfgradioobjective::disk::Cache,f.Actor);f.Acquire();check(f.radio.stage==3&&!f.exchanges);}
 {RadioFixture f;f.r.Put(f.registry.data+24*2+16,int{});f.Acquire();
  check(f.radio.stage==3&&f.radio.typedStage==2&&f.radio.candidate==f.Radio&&f.radio.candidateSerial==0&&!f.exchanges);}
 {RadioFixture f;const auto dummy=f.Radio+0x20000;f.r.Put(dummy+cfgprobe::ObjectClass,f.CabinClass);
  f.r.Put(f.Level+cfgradarworld::disk::LevelActors,cfgshacktrap::List{f.Actors,8192,8192});
  for(unsigned i=0;i<8192;++i)f.r.Put(f.Actors+8*i,i==8191?f.Radio:dummy);
  f.Acquire();check(f.radio.stage==2&&f.radio.slices>1&&f.radio.slices<128&&f.Publish());peak=(std::max)(peak,f.peak);}
 check(peak<=1536);std::printf("Radio objective publication: %u failures peak%u/1536; finite world-only source, exact empty cache, native getter readback, mutations/no replay; owned fixtures only.\n",failed,peak);
 return failed?1:0;
}
#endif
