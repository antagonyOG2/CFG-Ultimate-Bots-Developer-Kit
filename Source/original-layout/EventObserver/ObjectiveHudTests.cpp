#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "ObjectiveHudWrite.hpp"
#define main SetupWidgetFixtureMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "ObjectiveHudRuntime.hpp"
struct BytesReader {
 OwnedByteReader& r;unsigned reads=160;bool exhausted=false;
 bool Tick(){if(reads==1536){exhausted=true;return false;}++reads;return true;}
 template<class T>bool Read(std::uintptr_t at,T& out){return Tick()&&r.Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* output,std::size_t size){if(!Tick())return false;auto* out=static_cast<unsigned char*>(output);for(std::size_t i=0;i<size;++i)if(!r.Read(at+i,out[i]))return false;return true;}
};
struct ObjectiveFixture:MenuFixture {
 static constexpr std::uintptr_t Registry=0x900000,Generated=0x910000,BaseState=0x920000,Hunt=0x930000,Script=0x940000;
 cfgobjective::Bytes script{};BytesReader reader{r};
 ObjectiveFixture(){
  Name(Type,"Objective_Radio_Widget_C");Name(Generated,"WidgetBlueprintGeneratedClass");Name(BaseState,"SCGameState");Name(Hunt,"SCGameState_Hunt");
  Name(DonorType,"Class");r.Set(BaseState+16,DonorType);r.Set(Hunt+16,DonorType);
  r.Set(Type+16,Generated);r.Set(Hunt+cfgprobe::ClassSuper,BaseState);
  r.Set(Base+cfgcompat::cfg35::objects::SCGameStateReflection,BaseState);
  r.Set(Base+cfgcompat::cfg35::objects::SCGameStateConstructor,BaseState);
  r.Set(Base+0x03199662,std::uint8_t{0});r.Set(Base+0x03198110,std::uint32_t{0});
  r.Set(Base+cfgmenu::counselorpickerdisk::DynamicCastSlot,Base+cfgmenu::counselorpickerdisk::DynamicCastExecutor);
  r.Set(Base+cfgmenu::counselorpickerdisk::DynamicCastExecutor,cfgmenu::counselorpickerdisk::DynamicCastCode);
  r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x10,Registry);r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,2);
  r.Set(Registry,std::array<unsigned char,48>{});
  r.Set(Registry,Type);r.Set(Registry+16,0);r.Set(Type+12,0);
  r.Set(Registry+24,Start);r.Set(Registry+40,0);r.Set(Start+12,1);
  r.Set(Type+cfgreflection::disk::Children,Start);Field(Start,Type,FunctionClass,"HunterSpawned",0);
  r.Set(Start+cfgmenu::startdisk::Executor,Base+cfgmenu::startdisk::ScriptExecutor);
  Field(Member,Start,ObjectProperty,"CastDestination",Options);Property(Member,8,0);
  Field(Options,Start,ObjectProperty,"CastSource",0);Property(Options,8,8);
  r.Set(Member,Base+cfgmenu::counselorpickerdisk::ObjectPropertyVtable);r.Set(Options,Base+cfgmenu::counselorpickerdisk::ObjectPropertyVtable);
  script[93]=0x76;std::memcpy(script.data()+94,&Member,8);script[102]=0x0f;std::memcpy(script.data()+103,&Hunt,8);
  script[111]=0x76;std::memcpy(script.data()+112,&Options,8);script[120]=0x9e;
  r.Set(Script,script);r.Set(Start+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{Script,229,229});
 }
};
struct Host {
 ObjectiveFixture& f;unsigned writes=0;bool current=true,partial=false;
 bool Current(){return current;}
 bool Exchange(std::uintptr_t at,std::uintptr_t oldValue,std::uintptr_t newValue){
  std::uintptr_t actual=0;if(!f.r.Read(at,actual)||actual!=oldValue)return false;++writes;
  if(partial){f.r.Set(at,std::uint8_t{0x55});return false;}f.r.Set(at,newValue);return true;
 }
};
unsigned peakPhaseReads=0;
void Step(cfgobjective::Runtime& runtime,ObjectiveFixture& f,Host& host,std::uint64_t now=1){
 f.reader.reads=160;f.reader.exhausted=false;runtime.Step(f.reader,MenuFixture::Base,host,now);
 peakPhaseReads=(std::max)(peakPhaseReads,f.reader.reads);
}
void Retire(cfgobjective::Runtime& runtime,ObjectiveFixture& f,Host& host){
 f.reader.reads=160;f.reader.exhausted=false;runtime.RetireOne(f.reader,MenuFixture::Base,host);
 peakPhaseReads=(std::max)(peakPhaseReads,f.reader.reads);
}
#ifndef CFG_OBJECTIVE_HUD_FIXTURE_ONLY
int main(){unsigned failures=0;auto check=[&](bool okay){if(!okay)++failures;};
 for(unsigned fault=0;fault<9;++fault){ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  if(fault==1)f.r.Set(ObjectiveFixture::Script+102,std::uint8_t{0x2e});
  if(fault==2)f.r.Set(MenuFixture::Member+32,MenuFixture::Type);
  if(fault==3)f.r.Set(MenuFixture::Start+cfgmenu::startdisk::Executor,std::uintptr_t{0x123000});
  if(fault==4)f.r.Set(ObjectiveFixture::Hunt+cfgprobe::ClassSuper,ObjectiveFixture::Hunt);
  if(fault==5)f.r.Set(MenuFixture::Base+0x03198110,std::uint32_t{1});
  if(fault==6)host.current=false;
  if(fault==7)host.partial=true;
  if(fault==8)f.r.Set(ObjectiveFixture::Script+103,ObjectiveFixture::BaseState);
  Step(runtime,f,host);Step(runtime,f,host);Step(runtime,f,host);
  if(!fault){check(runtime.Status().changed==1&&host.writes==1&&runtime.PendingRestore());
   check(runtime.Status().processedMask==1&&runtime.Status().changedMask==1&&!runtime.Status().refusedMask&&!runtime.Status().baseMask);Retire(runtime,f,host);
   std::uintptr_t value=0;check(f.r.Read(ObjectiveFixture::Script+103,value)&&value==ObjectiveFixture::Hunt);check(runtime.Status().restored==1&&host.writes==2);}
  else if(fault==7){check(runtime.Status().indeterminate&&host.writes==1);Step(runtime,f,host);Retire(runtime,f,host);check(host.writes==1&&runtime.Status().restoreRefused==1);}
  else {check(!host.writes&&!runtime.Status().changed);
   if(fault==8)check(runtime.Status().baseMask==1&&runtime.Status().processedMask==1&&!runtime.Status().refusedMask);
   if(fault>=1&&fault<=4)check(runtime.Status().refusedMask==1&&runtime.Status().processedMask==1);}
 }
 {ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;Step(runtime,f,host);Step(runtime,f,host);Step(runtime,f,host);
  f.r.Set(ObjectiveFixture::Script+1,std::uint8_t{77});Retire(runtime,f,host);Retire(runtime,f,host);check(host.writes==1&&runtime.Status().restoreRefused==1);}
 // Native assigns weak serials to class/function metadata after the initial
 // patch. Retirement must not treat that exact zero->positive assignment as
 // recycling, and must not bypass changed scripts or positive replacements.
 for(unsigned fault=0;fault<4;++fault){ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  Step(runtime,f,host);Step(runtime,f,host);Step(runtime,f,host);
  f.r.Set(ObjectiveFixture::Registry+16,11);f.r.Set(ObjectiveFixture::Registry+40,12);
  if(fault==1)f.r.Set(ObjectiveFixture::Script+1,std::uint8_t{77});
  if(fault==2)f.r.Set(MenuFixture::Start+cfgmenu::startdisk::Executor,std::uintptr_t{0x123000});
  if(fault==3)f.r.Set(ObjectiveFixture::Registry+24,std::uintptr_t{0x980000});
  Retire(runtime,f,host);check(runtime.PendingRestore()&&!runtime.Status().restoreRefused&&host.writes==1);
  Retire(runtime,f,host);Retire(runtime,f,host);
  check(fault?(runtime.Status().restoreRefused==1&&host.writes==1):
     (runtime.Status().restored==1&&!runtime.PendingRestore()&&host.writes==2));
 }
 {ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;Step(runtime,f,host,1000);Step(runtime,f,host,91000);check(runtime.Status().expired&&!host.writes);}
 // A first objectives open may load classes after the initial finite census.
 for(unsigned reuse=0;reuse<2;++reuse){ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  f.r.Set(ObjectiveFixture::Registry,std::uintptr_t{0});Step(runtime,f,host,1);
  check(runtime.Status().discoveryDone&&!runtime.Pending(100)&&runtime.Pending(501));
  if(reuse){f.r.Set(ObjectiveFixture::Registry,MenuFixture::Type);Step(runtime,f,host,45001);}
  else{f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,3);
   f.r.Set(ObjectiveFixture::Registry+48,std::array<std::uintptr_t,3>{MenuFixture::Type,0,0});f.r.Set(MenuFixture::Type+12,2);Step(runtime,f,host,501);}
  const std::uint64_t now=reuse?45002:502;Step(runtime,f,host,now);Step(runtime,f,host,now+1);
  check(runtime.Status().changed==1);Retire(runtime,f,host);check(runtime.Status().restored==1);
 }
 {ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,1030);
  f.r.Set(MenuFixture::ObjectProperty+16,MenuFixture::FunctionClass);
  for(unsigned i=0;i<1030;++i){f.r.Set(ObjectiveFixture::Registry+24ULL*i,std::array<std::uintptr_t,3>{MenuFixture::ObjectProperty,0,0});}
  f.r.Set(ObjectiveFixture::Registry+24ULL*1028,MenuFixture::Start);f.r.Set(MenuFixture::Start+12,1028);
  f.r.Set(ObjectiveFixture::Registry+24ULL*1029,MenuFixture::Type);f.r.Set(MenuFixture::Type+12,1029);
  Step(runtime,f,host);check(runtime.Status().scanned==512&&!runtime.Status().discoveryDone);
  Step(runtime,f,host);check(runtime.Status().scanned==1024&&!runtime.Status().discoveryDone);
  Step(runtime,f,host);check(runtime.Status().scanned==1030&&runtime.Status().discoveryDone);
  Step(runtime,f,host);Step(runtime,f,host);check(runtime.Status().changed==1&&!f.reader.exhausted);
  Retire(runtime,f,host);check(runtime.Status().restored==1&&!runtime.PendingRestore());}
 check(cfgobjective::Specification(24).name==u"Find4DoorCar");
 {ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  Step(runtime,f,host);Step(runtime,f,host);Step(runtime,f,host);
  host.current=false;Retire(runtime,f,host);
  check(runtime.PendingRestore()&&!runtime.Status().restoreRefused&&host.writes==1);
  host.current=true;Retire(runtime,f,host);check(!runtime.PendingRestore()&&runtime.Status().restored==1);}
 {auto* memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));check(memory!=nullptr);
  if(memory){const std::uintptr_t oldValue=0x12340000,newValue=0x56780000;std::memcpy(memory+3,&oldValue,8);const auto at=reinterpret_cast<std::uintptr_t>(memory+3);
   check(!cfgobjective::CompareWrite(at,newValue,oldValue));check(cfgobjective::CompareWrite(at,oldValue,newValue));
   DWORD previous=0;check(VirtualProtect(memory,4096,PAGE_READONLY,&previous)!=0);check(!cfgobjective::CompareWrite(at,newValue,oldValue));
   check(VirtualProtect(memory,4096,PAGE_READWRITE,&previous)!=0);check(cfgobjective::CompareWrite(at,newValue,oldValue));check(VirtualFree(memory,0,MEM_RELEASE)!=0);}}
 check(peakPhaseReads<=1536);
 std::printf("ObjectiveHudTests: %u failures; peak phase reads %u/1536 including160 outer; exact cast/owner/executor/GC/refusal, zero-serial metadata, owned restore and partial-write retention\n",failures,peakPhaseReads);return failures?1:0;
}
#endif
