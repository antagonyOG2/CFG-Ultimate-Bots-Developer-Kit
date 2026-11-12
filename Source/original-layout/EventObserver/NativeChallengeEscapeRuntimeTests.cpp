#define CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeActorsTests.cpp"
#undef CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeRuntime.hpp"
#include "NativeChallengeEscapeSeams.hpp"
#include "NativeChallengeEscapeHooks.hpp"
#include <functional>
namespace rt=cfgchallengeescape;
namespace ds=rt::dispatch;
namespace b=rt::boundary;
namespace rs=rt::resources;
struct RuntimeFixture:ActorFixture {
 static constexpr std::uintptr_t Functions=0x2800000,FunctionKind=0x2900000,Parameters=0x2A00000,SceneType=0x2390000,BaseControllerType=0x23A0000;
 RuntimeFixture(){
  r.Set(Module+rt::geometrydisk::LocationBody,rt::geometrydisk::LocationCode);
  r.Set(Module+rt::geometrydisk::AbsMask,std::array<std::uint32_t,4>{{0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF}});
  Type(SceneType,d::SceneComponent,d::SceneComponentCtor);r.Set(PrimitiveType+cfgprobe::ClassSuper,SceneType);
  Type(BaseControllerType,d::Controller,d::ControllerCtor,ActorType);r.Set(AiType+cfgprobe::ClassSuper,BaseControllerType);Label(FunctionKind,"Function");
  for(unsigned i=0;i<d::Apis.size();++i){const auto& api=d::Apis[i];const auto fn=Functions+0x1000*i;std::uintptr_t owner=0;r.Read(Module+api.owner,owner);
   std::string name;for(auto ch:api.name)name.push_back(static_cast<char>(ch));Label(fn,name.c_str());
   r.Set(fn+cfgprobe::ObjectClass,FunctionKind);r.Set(fn+0x20,owner);r.Set(fn+cfgreflection::disk::Next,std::uintptr_t{});
   r.Set(Module+api.cache,fn);r.Set(fn+0x88,api.flags);r.Set(fn+0x8C,static_cast<std::uint8_t>(api.count));r.Set(fn+0x8E,static_cast<std::uint16_t>(api.logical));
   r.Set(fn+cfgreflection::disk::StorageBytes,static_cast<int>(api.storage));r.Set(fn+0x68,std::uintptr_t{});r.Set(fn+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{});r.Set(fn+0xB0,Module+api.executor);
   const auto head=Parameters+0x10000*i;r.Set(fn+cfgreflection::disk::Children,api.count?head:0);
   for(unsigned j=0;j<api.count;++j){const auto& p=api.parameters[j];const auto at=head+0x1000*j,kind=at+0x800;
    name.clear();for(auto ch:p.name)name.push_back(static_cast<char>(ch));Label(at,name.c_str());name.clear();for(auto ch:p.kind)name.push_back(static_cast<char>(ch));Label(kind,name.c_str());
    r.Set(at+cfgprobe::ObjectClass,kind);r.Set(at+0x20,fn);r.Set(at+cfgreflection::disk::Next,j+1<api.count?at+0x1000:0);
    r.Set(at+cfgreflection::disk::PropertyDimensions,int{1});r.Set(at+cfgreflection::disk::PropertyElementSize,static_cast<int>(p.bytes));r.Set(at+cfgreflection::disk::PropertyOffset,static_cast<int>(p.offset));
    r.Set(at+cfgreflection::disk::PropertyFlags,(p.flags&0x480)|0x0018001040000200ULL);r.Set(at+0x70,std::array<unsigned char,4>{{1,0,1,0xFF}});
   }
   for(unsigned j=0;j<47;++j)r.Set(Module+api.factory+j,api.factoryCode[j]);for(unsigned j=0;j<api.codeBytes;++j)r.Set(Module+api.executor+j,api.code[j]);
  }
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
 void ManyActors(unsigned count){
  unsigned actors=7;
  for(unsigned i=0;i<count;++i){const std::uintptr_t object=0x3400000+0x1000*i;Object(object,ActorType,Level);r.Set(ActorArray+8ULL*actors++,object);}
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,static_cast<int>(actors),static_cast<int>(actors)});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
 }
 void LiveSizeWorld(unsigned passengers=0){
  const unsigned extras=passengers>1?2u*(passengers-1):0u;
  Occupied();ManyActors(2179-extras);unsigned count=2186-extras;
  // Paused ABI71:2190 cells,5 volumes,one occupied driver seat.
  if(!passengers){r.Set(PassengerSeat+0x500,std::uintptr_t{});r.Set(Passenger+0x15C0,std::uintptr_t{});}
  for(unsigned i=1;i<passengers;++i){const std::uintptr_t npc=0xD000000+0x10000*i,ai=npc+0x1000,root=npc+0x2000,movement=npc+0x3000,seat=npc+0x4000;
   CounselorObject(npc,ai,root,movement);Object(seat,SeatType,Car);r.Set(seat+0x548,Car);r.Set(seat+0x500,npc);r.Set(seat+0x511,std::uint8_t{});
   r.Set(npc+0x15C0,seat);r.Set(SeatArray+8ULL*(i+1),seat);r.Set(ActorArray+8ULL*count++,npc);r.Set(ActorArray+8ULL*count++,ai);
  }
  if(passengers>1)r.Set(Car+0x4E8,act::List{SeatArray,static_cast<int>(passengers+1),static_cast<int>(passengers+1)});
  for(unsigned i=0;i<4;++i){const std::uintptr_t volume=0xA000000+0x10000*i,sphere=volume+0x1000;
   Object(volume,VolumeType,Level);Object(sphere,SphereType,volume);r.Set(volume+0x368,sphere);r.Set(ActorArray+8ULL*count++,volume);
  }
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,static_cast<int>(count),static_cast<int>(count)});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
 }
 void CapturedSizeWorld(){
  Occupied();ManyActors(544);unsigned actorCount=551;
  int levelIndex=0;r.Read(Level+cfgprobe::lifetime::ObjectIndexOffset,levelIndex);
  r.Set(Registry+24ULL*static_cast<unsigned>(levelIndex)+16,int{});
  for(unsigned i=0;i<2;++i){const std::uintptr_t volume=0x3800000+0x10000*i,sphere=volume+0x1000;
   Object(volume,VolumeType,Level);Object(sphere,SphereType,volume);r.Set(volume+0x368,sphere);r.Set(ActorArray+8ULL*actorCount++,volume);
   const std::uintptr_t npc=0x3900000+0x10000*i,ai=npc+0x1000,root=npc+0x2000,movement=npc+0x3000,seat=npc+0x4000;
   CounselorObject(npc,ai,root,movement);Object(seat,SeatType,Car);r.Set(seat+0x548,Car);r.Set(seat+0x500,npc);r.Set(seat+0x511,std::uint8_t{});
   r.Set(npc+0x15C0,seat);r.Set(SeatArray+8ULL*(i+2),seat);
   r.Set(ActorArray+8ULL*actorCount++,npc);r.Set(ActorArray+8ULL*actorCount++,ai);
  }
  r.Set(Car+0x4E8,act::List{SeatArray,4,4});r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,static_cast<int>(actorCount),static_cast<int>(actorCount)});
  r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
  // Authored unrelated actors do not automatically have assigned weak serials.
  for(unsigned i=7;i<551;++i){std::uintptr_t actor=0;int index=0;r.Read(ActorArray+8ULL*i,actor);r.Read(actor+cfgprobe::lifetime::ObjectIndexOffset,index);
   r.Set(Registry+24ULL*static_cast<unsigned>(index)+16,int{});
  }
 }
};
struct RuntimeSimulator {
 ChallengeReader& r;unsigned calls=0;std::vector<unsigned> actions;std::map<std::uintptr_t,rs::Vector> positions;std::function<void(unsigned)> nested;
 bool Invoke(std::uintptr_t target,std::uintptr_t function,void* data){++calls;const auto index=static_cast<unsigned>((function-RuntimeFixture::Functions)/0x1000);
  if(index>=d::Apis.size())return false;if(nested){auto callback=std::move(nested);nested={};callback(index);}
  switch(static_cast<rs::Api>(index)){
   case rs::Api::ActorLocation:case rs::Api::ComponentLocation:*static_cast<rs::Vector*>(data)=positions[target];break;
   case rs::Api::ActorRotation:*static_cast<rs::Rotation*>(data)={};break;
   case rs::Api::SphereRadius:*static_cast<float*>(data)=5;break;
   case rs::Api::OverlappingActor:{auto& p=*static_cast<rs::OverlapPacket*>(data);const auto point=positions[p.other],center=positions[target];
    const double x=point.x-center.x,y=point.y-center.y,z=point.z-center.z;p.returned=x*x+y*y+z*z<=25;break;}
   case rs::Api::Teleport:{auto& p=*static_cast<rs::TeleportPacket*>(data);positions[target]=p.location;p.returned=1;actions.push_back(index);break;}
   default:actions.push_back(index);break;
  }return true;
 }
};
struct RuntimePlatform {
 RuntimeSimulator& simulator;RuntimeFixture& fixture;unsigned factories=0;
 std::uint32_t thread=42;std::uintptr_t module=Fixture::Module;
 std::uint32_t Thread()const{return thread;}std::uintptr_t Module()const{return module;}
 bool Invoke(std::uintptr_t address,std::uintptr_t target,std::uintptr_t function,void* packet){
  return address==module+cfgspawn::GenericDispatcherRva&&simulator.Invoke(target,function,packet);
 }
 bool Factory(std::uintptr_t address,std::uintptr_t& out){++factories;
  for(unsigned i=0;i<d::Apis.size();++i)if(address==module+d::Apis[i].factory){
   out=RuntimeFixture::Functions+0x1000*i;simulator.r.Set(module+d::Apis[i].cache,out);return true;}return false;
 }
};
struct RuntimeHarness {
 RuntimeFixture f;ChallengeReader source;rt::Runtime runtime;RuntimeSimulator simulator{source};RuntimePlatform platform{simulator,f};
 bool installed=true;unsigned validatorCalls=0;b::Identity actor{},volume{};bool ready=false;std::function<b::Decision()> borrowedGate;
 explicit RuntimeHarness(bool car=false,unsigned many=0,bool capturedSize=false,bool wide=false,bool liveSize=false,unsigned passengers=0){if(car)f.Occupied();if(many)f.ManyActors(many);if(capturedSize)f.CapturedSizeWorld();if(liveSize)f.LiveSizeWorld(passengers);if(wide)f.WideCounselorHierarchy();
  f.r.Set(f.Module+cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix);
  static_cast<StageReader&>(source)=f.r;source.cap=ds::ReadLimit;
  cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::ReadArrayHeader(source,f.Module+cfgprobe::lifetime::ObjectArrayRva,registry);
  cfgprobe::lifetime::SlotObservation slot;cfgprobe::lifetime::ReadSlot(source,registry,car?f.Car:f.Npc,slot);actor=rt::Identity(slot);
  cfgprobe::lifetime::ReadSlot(source,registry,f.Volume,slot);volume=rt::Identity(slot);
  simulator.positions[f.Volume]=simulator.positions[f.Sphere]={50,0,0};source.Set(f.Sphere+0x190,std::array<float,4>{{50,0,0,0}});source.count=0;
  if(capturedSize)for(unsigned i=0;i<2;++i){const std::uintptr_t volumeObject=0x3800000+0x10000*i;
   simulator.positions[volumeObject]=simulator.positions[volumeObject+0x1000]={100.0f+50.0f*i,0,0};
   source.Set(volumeObject+0x1000+0x190,std::array<float,4>{{100.0f+50.0f*i,0,0,0}});
  }
  if(liveSize)for(unsigned i=0;i<4;++i){const std::uintptr_t volumeObject=0xA000000+0x10000*i;
   simulator.positions[volumeObject]=simulator.positions[volumeObject+0x1000]={100.0f+50.0f*i,0,0};
   source.Set(volumeObject+0x1000+0x190,std::array<float,4>{{100.0f+50.0f*i,0,0,0}});
  }
  ready=runtime.Arm(f.Menu(),1,true,0);
 }
 using Reader=ds::CallbackReader<ChallengeReader>;
 std::array<rt::hooks::InstalledLease,5> hookLeases{};std::array<std::array<unsigned char,5>,5> hookPatches{};
 std::array<void*,5> hookOriginals{};rt::hooks::Originals hookHandlers{};bool actualProof=false;
 void ActualHookProof(){std::array<void*,5> handlers{};
  for(unsigned i=0;i<5;++i){const auto& body=rt::hooks::Bodies[i];const auto target=f.Module+body.rva,original=target+0x10000,handler=target+0x20000;
   hookOriginals[i]=reinterpret_cast<void*>(original);handlers[i]=reinterpret_cast<void*>(handler);
   std::array<unsigned char,60> trampoline{};std::copy_n(body.code,body.copied,trampoline.begin());
   const auto back=cfgdispatch::AbsoluteJump(target+body.copied),relay=cfgdispatch::AbsoluteJump(handler);
   std::copy(back.begin(),back.end(),trampoline.begin()+body.copied);std::copy(relay.begin(),relay.end(),trampoline.begin()+body.copied+14);
   rt::hooks::TrampolineProof(body,target,original,handler,trampoline.data(),hookPatches[i]);
   for(unsigned j=0;j<body.bytes;++j)source.Set(target+j,j<5?hookPatches[i][j]:body.code[j]);
   for(unsigned j=0;j<body.copied+28;++j)source.Set(original+j,trampoline[j]);
   hookLeases[i].Capture(source,f.Module,i,hookPatches[i]);
  }
  hookHandlers=rt::hooks::Typed(handlers);actualProof=true;source.count=0;
 }
 bool HookCurrent(Reader& r){return installed&&(!actualProof||rt::hooks::BudgetedHookCurrent(r,f.Module,hookOriginals,hookHandlers,hookLeases,hookPatches,[](void*,unsigned){return true;}));}
 bool Installed(Reader& r,const scope::Witness&){return HookCurrent(r);}
 bool Validation(Reader&,const scope::Witness& w){++validatorCalls;return w.generation==runtime.Generation();}
 bool Service(std::uint64_t now,bool physicalAllowed=true){source.count=0;Reader reader(source);
  auto dispatchLease=[&](Reader& r){scope::Witness w;return HookCurrent(r)&&runtime.CurrentWitness(w)&&runtime.ScopeCurrent(r,42,w);};
  ds::Engine<Reader,RuntimePlatform,decltype(dispatchLease)> engine(reader,platform,f.Module,42,true,dispatchLease);
  auto lease=[&](Reader& r,const scope::Witness& w){return Installed(r,w);};
  auto validate=[&](Reader& r,const scope::Witness& w){return Validation(r,w);};
  borrowedGate=[&]{return runtime.Gate(reader,engine,42,now,lease,actor,b::Seam::Overlap,validate);};
  const auto result=runtime.Service(reader,engine,42,now,lease,physicalAllowed);borrowedGate={};return result;
 }
 b::Decision Gate(std::uint64_t now,b::Seam seam=b::Seam::Overlap){
  source.count=0;Reader reader(source);
  auto dispatchLease=[&](Reader& r){scope::Witness w;return HookCurrent(r)&&runtime.CurrentWitness(w)&&runtime.ScopeCurrent(r,42,w);};
  ds::Engine<Reader,RuntimePlatform,decltype(dispatchLease)> engine(reader,platform,f.Module,42,true,dispatchLease);
  auto lease=[&](Reader& r,const scope::Witness& w){return Installed(r,w);};
  rt::seams::Evidence evidence;rt::seams::Arguments args;
  args.self=seam==b::Seam::VehicleStart?actor.address:f.Volume;args.component=f.Sphere;args.volume=f.Volume;args.actor=actor.address;
  auto validate=[&](Reader& r,const scope::Witness& w){
   if(!actualProof)return Validation(r,w);
   const auto current=[&]{return runtime.ScopeCurrent(r,42,w);};
   if(!evidence.valid)return rt::seams::Validate(r,f.Module,w,seam,args,current,nullptr,&evidence);
   return rt::seams::CurrentEvidence(r,f.Module,w,seam,args,current,evidence);
  };
  borrowedGate=[&]{return runtime.Gate(reader,engine,42,now,lease,actor,seam,validate);};
  const auto result=runtime.Gate(reader,engine,42,now,lease,actor,seam,validate);borrowedGate={};return result;
 }
 void ReadyResources(){for(unsigned i=0;i<16;++i)Service(100+250*i);}
};
int main(){unsigned checks=0,failures=0,peak=0;const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 cfgdispatch::enabled=false;cfgdispatch::owned={};
 for(unsigned state=0;state<7;++state){RuntimeHarness h(true);h.ActualHookProof();h.ReadyResources();
  if(state==0)h.source.Set(h.f.Npc+0x1518,h.f.Volume);
  if(state==1)h.source.Set(h.f.Npc+0x161C,std::uint8_t{1});
  if(state==2)h.source.Set(h.f.Npc+0x15D4,std::uint8_t{1});
  if(state==3)h.source.Set(h.f.Npc+0x1525,std::uint8_t{1});
  if(state==4){h.source.Set(h.f.Npc+0x1518,h.f.Volume);h.source.Set(h.f.Npc+0x3A0,std::uintptr_t{});}
  if(state==5)h.source.Set(h.f.Npc+0x1070,std::uint8_t{1});
  if(state==6)h.source.Set(h.f.Npc+0xE54,std::uint8_t{1});
  h.simulator.actions.clear();const auto calls=h.simulator.calls;
  const auto result=h.Gate(4500,b::Seam::VehicleStart);
  check(result.blockNative==(state<5)&&!result.recoveryAttempted&&h.simulator.calls==calls&&h.simulator.actions.empty(),
   "read-only car admission blocks living verified AI transitions, rejects dead/dying, and never moves occupants");
  for(unsigned i=0;i<3;++i)h.Service(4750+250*i);
  check(h.simulator.actions.empty(),"transition-state occupants remain ineligible for physical car recovery");
 }
 for(unsigned fault=1;fault<=9;++fault){RuntimeHarness h(false,1);h.ReadyResources();
  for(unsigned i=0;i<6;++i)h.Service(4000+100*i);
  const auto object=std::uintptr_t{0x3400000};int index=0;h.source.Read(object+cfgprobe::lifetime::ObjectIndexOffset,index);
  const auto item=h.f.Registry+24ULL*static_cast<unsigned>(index);bool changed=false;
  h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();
  h.simulator.nested=[&](unsigned){changed=true;
   switch(fault){
    case 1:h.source.Set(h.f.ActorArray+8*7,std::uintptr_t{});break;
    case 2:h.source.Set(object+cfgprobe::lifetime::ObjectIndexOffset,int{});break;
    case 3:h.source.Set(item,h.f.Npc);break;
    case 4:h.source.Set(item+16,int{99999});break;
    case 5:h.source.Set(item+8,std::uint32_t{0x10000000});break;
    case 6:h.source.Set(object+8,std::uint32_t{0x20});break;
    case 7:h.source.Set(object+cfgprobe::ObjectClass,h.f.CarType);break;
    case 8:h.source.Set(object+0x20,h.f.World);break;
    case 9:h.source.bytes.erase(item+12);break;
   }
  };
  for(unsigned i=0;i<3&&!changed;++i)h.Service(5000+250*i);
  check(changed&&h.simulator.actions.empty(),"fresh grouped census rejects changed cell/index/object/serial/flags/class/outer or partial read after native query before recovery mutation");
 }
 {RuntimeHarness h(true);h.ActualHookProof();h.source.Set(h.f.Npc+0x3A0,std::uintptr_t{});
  check(!h.Gate(100,b::Seam::VehicleStart).blockNative&&h.runtime.gates.lastStage[4]==6,
   "unpossessed driver without prior AI proof forwards and records policy refusal");
  std::printf("Unpossessed driver diagnostic current%u eligibility%u\n",h.runtime.gates.lastCurrent[4],h.runtime.gates.lastEligibility[4]);
  check(h.runtime.gates.lastCurrent[4]==4&&h.runtime.gates.lastEligibility[4]==5,
   "forward records car proof and missing historical AI eligibility");
  h.source.Set(h.f.Npc+0x3A0,h.f.Ai);
  check(h.Gate(200,b::Seam::VehicleStart).blockNative&&h.runtime.gates.blocked[4]==1&&h.runtime.gates.forwarded[4]==1,
   "later positive AI proof blocks without erasing prior forward evidence");
  check(h.runtime.gates.lastCurrent[4]==4&&h.runtime.gates.lastEligibility[4]==5,
   "successful block preserves earlier failed proof details");
  check(!h.Gate(300,b::Seam::Completion).blockNative&&h.runtime.gates.lastStage[3]==4,
   "wrong completion source records seam refusal independently of policy refusal");
  h.source.cap=10;
  check(!h.Gate(400,b::Seam::VehicleStart).blockNative&&h.runtime.gates.budget[4]==1&&h.runtime.gates.lastRemaining[4]==0,
   "exhausted shared reader records budget forward without new reader or native action");
 }
 {RuntimeHarness h;
  h.runtime.RecordGate(b::Seam::Overlap,h.actor.address,6,100,false,3,5);
  h.runtime.RecordGate(b::Seam::Overlap,h.actor.address,7,99,false);
  check(h.runtime.gates.lastStage[0]==7&&!h.runtime.gates.lastCurrent[0]&&!h.runtime.gates.lastEligibility[0],
   "early actor refusal clears proof details from an unrelated previous refusal");
  h.runtime.recoveryStage=4;h.runtime.recoveryRemaining=100;h.runtime.recoveryReserve=360448;
  h.runtime.Cancel();
  check(!h.runtime.Armed()&&h.runtime.gates.attempts[0]==2&&h.runtime.gates.lastStage[0]==7&&h.runtime.recoveryStage==4&&
   h.runtime.recoveryRemaining==100&&h.runtime.recoveryReserve==360448,
   "mission retirement clears authority but retains failure evidence for results-screen report");
  check(h.runtime.Arm(h.f.Menu(),2,true,100)&&!h.runtime.gates.attempts[0]&&!h.runtime.gates.lastStage[0]&&!h.runtime.recoveryStage,
   "new settings generation resets retained gate and recovery diagnostics");
 }
 {RuntimeHarness h(true,2183,false,true);h.ActualHookProof();
  const auto result=h.Gate(100,b::Seam::VehicleStart);
  std::printf("Live-size wide car gate blocked%u reads%u/%u calls%u\n",result.blockNative,h.source.count,ds::ReadLimit,h.simulator.calls);
  check(result.blockNative&&!h.simulator.calls,"live-size 2190-actor wide counselor car blocks first native commitment within fixed budget");
  check(h.runtime.gates.attempts[4]==1&&h.runtime.gates.blocked[4]==1&&!h.runtime.gates.forwarded[4],"gate diagnostics distinguish vehicle-start block from all other seams");
 }
 {RuntimeHarness h(true,0,false,true,true);h.ActualHookProof();unsigned saturated=0;
  for(unsigned i=0;i<90;++i){h.Service(100+50*i);peak=(std::max)(peak,h.source.count);if(h.source.count==ds::ReadLimit)++saturated;}
  check(h.runtime.CensusComplete()&&h.runtime.VolumeCount()==5,"live-size scene establishes complete five-volume roster");
  h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();
  for(unsigned i=0;i<6;++i){h.Service(4700+100*i);peak=(std::max)(peak,h.source.count);if(h.source.count==ds::ReadLimit)++saturated;}
  std::printf("Live-size five-volume recovery position%.0f saturations%u recoveries%u\n",h.simulator.positions[h.actor.address].x,saturated,h.runtime.counters.recoveries);
  std::printf("Preflight stage%u remaining%u reserve%u\n",h.runtime.recoveryStage,h.runtime.recoveryRemaining,h.runtime.recoveryReserve);
  const std::vector<unsigned> actions{{10,11,8,9,3,10,11,8,9}};
  check(!saturated&&h.simulator.positions[h.actor.address].x==0&&h.runtime.counters.recoveries==1&&
   h.simulator.actions==actions&&h.runtime.recoveryStage==5&&h.runtime.recoveryRemaining>=h.runtime.recoveryReserve,
   "live-size five-volume car returns physically with all nine actions and full reservation inside unchanged cap");
 }
 for(unsigned occupants:{2u,4u}){RuntimeHarness h(true,0,false,true,true,occupants-1);h.ActualHookProof();
  for(unsigned i=0;i<90;++i)h.Service(100+50*i);
  check(h.runtime.CensusComplete()&&h.runtime.VolumeCount()==5,"occupied passenger live-size scene census complete");
  h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();
  for(unsigned i=0;i<12;++i){h.Service(4700+100*i);peak=(std::max)(peak,h.source.count);}
  std::printf("%u occupants live-size recovery position%.0f recoveries%u stage%u remaining%u reserve%u reads%u\n",
   occupants,h.simulator.positions[h.actor.address].x,h.runtime.counters.recoveries,h.runtime.recoveryStage,
   h.runtime.recoveryRemaining,h.runtime.recoveryReserve,h.source.count);
  check(h.runtime.counters.recoveries==1&&h.simulator.positions[h.actor.address].x==0,
   "live-size five-volume car with multiple living AI occupants recovers within unchanged caps and full reservation");
 }
 {RuntimeHarness h(true,0,false,true,true,3);h.ActualHookProof();
  for(unsigned i=0;i<90;++i)h.Service(100+50*i);
  h.source.Set(h.f.Passenger+0x3A0,h.f.Human);h.source.Set(h.f.PassengerRoot+0x20,h.f.Car);
  h.simulator.actions.clear();
  check(!h.Gate(4700,b::Seam::VehicleStart).blockNative&&h.simulator.actions.empty(),
   "warm ownership cache detects human passenger takeover even with broken physical root");
  h.source.Set(h.f.Passenger+0x3A0,h.f.PassengerAi);h.source.Set(h.f.PassengerRoot+0x20,h.f.Passenger);
  check(!h.Gate(4900,b::Seam::VehicleStart).blockNative&&h.simulator.actions.empty(),
   "cached ownership refresh cannot erase the human-incarnation tombstone when AI returns");
 }
 {RuntimeHarness h(false,0,false,true);h.ActualHookProof();
  const auto wideGate=h.Gate(100);
  check(h.ready&&wideGate.blockNative&&!h.simulator.calls,"captured wide counselor hierarchy blocks escape through production seam and installed-hook proof before warming");
  for(unsigned i=0;i<16;++i){h.Service(350+250*i);peak=(std::max)(peak,h.source.count);}
  check(h.runtime.CensusComplete()&&h.runtime.WarmProgress()==12&&!h.runtime.censusFailure,"wide counselor fields compose with complete scene census and native resources");
  check(h.Gate(4500).blockNative,"cached wide native fields remain eligible at repeated physical seam");
 }
 {RuntimeHarness h;check(h.ready&&h.runtime.Armed()&&!h.runtime.CensusComplete(),"explicit stock source generation arms portable runtime");
  const auto result=h.Gate(100);
  check(result.blockNative&&!result.recovered&&!h.simulator.calls&&!h.runtime.WarmProgress()&&!h.runtime.CensusComplete(),
   "fresh native AI gate blocks before API warming or volume census without gameplay dispatch");
  check(h.validatorCalls>1,"required native seam validator repeated on current gate edges");
 }
 for(unsigned car=0;car<2;++car){RuntimeHarness h(car!=0);check(h.ready,"service runtime fixture");
  for(unsigned i=0;i<12;++i){const auto before=h.runtime.WarmProgress(),pumps=h.runtime.counters.pumps;
   h.Service(100+250*i);check(h.runtime.WarmProgress()==before+1,"service validates one API per callback without eager all12 loop");
   check(h.runtime.counters.pumps<=pumps+1,"service spends at most one policy actor opportunity");peak=(std::max)(peak,h.source.count);}
  check(h.runtime.WarmProgress()==12&&h.runtime.CensusComplete()&&h.runtime.VolumeCount()==1&&!h.platform.factories,"finite current census and progressive hot native resource preparation complete");
  // Round-robin includes both counselors and the car. Give each a cadence.
  for(unsigned i=0;i<6;++i)h.Service(4000+250*i);
  h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();h.simulator.calls=0;
  const auto result=h.Gate(5500,car?b::Seam::VehicleStart:b::Seam::Overlap);peak=(std::max)(peak,h.source.count);
  check(result.blockNative&&!result.recoveryAttempted&&!h.simulator.calls,"native commitment admission never spends optional geometry/action budget");
  for(unsigned i=0;i<3;++i){h.Service(5500+250*i);peak=(std::max)(peak,h.source.count);}
  check(h.simulator.positions[h.actor.address].x==0,"paced sampler composes complete physical recovery independently of native suppression");
  check(h.simulator.calls<=ds::CallLimit&&h.source.count<ds::ReadLimit,"entire service and gate use one fixed outer callback budget");
 }
 // Task005 block-only mode: native service resources/geometry/recovery
 // are unavailable without an entered ScopedLookupEligibility; five seams
 // still deny an eligible AI after read-only world/actor admission.
 {RuntimeHarness h;check(h.Service(100,false),"block-only service observes scope/census");
  check(h.runtime.CensusComplete()&&!h.runtime.WarmProgress()&&!h.simulator.calls&&!h.platform.factories,
   "no recovery scope means zero factories and native calls");
  for(auto seam:{b::Seam::Overlap,b::Seam::CharacterStart,b::Seam::Presentation,b::Seam::Completion})
   check(h.Gate(4500,seam).blockNative&&!h.simulator.calls,"four read-only counselor seams block AI without native calls");
  h.source.Set(h.f.Npc+0x3A0,std::uintptr_t{});
  check(!h.Gate(4750).blockNative&&!h.simulator.calls,"unproven counselor forwards without native calls");
 }
 {RuntimeHarness h(true);h.Service(100,false);
  check(h.Gate(4500,b::Seam::VehicleStart).blockNative&&!h.simulator.calls,
   "vehicle seam blocks registered AI occupancy in block-only mode");
 }
 {RuntimeHarness h(false,90);check(h.ready,"incremental census fixture");
  for(unsigned i=0;i<3;++i){const auto before=h.runtime.counters.cells;h.Service(100+250*i);
   check(h.runtime.counters.cells-before<=rt::Runtime::ScanCellsPerService,"bounded world scan visits at most32actor cells per callback");
   if(i<2)check(!h.runtime.CensusComplete(),"partial finite scan does not claim complete volume geometry");
  }
  check(!h.runtime.CensusComplete(),"97cell census needs a fourth callback");
  h.Service(850);check(h.runtime.CensusComplete(),"incremental census finishes without any global registry scan");
 }
 {RuntimeHarness h;h.source.Set(h.f.Module+d::Apis[0].cache,std::uintptr_t{});
  const auto calls=h.simulator.calls;h.Service(100);
  check(h.platform.factories==1&&!h.runtime.WarmProgress()&&h.simulator.calls==calls,"cold API factory spends callback without same-slice native action");
  h.Service(350);check(h.runtime.WarmProgress()==1&&h.platform.factories==1,"next callback repeats exact ready native metadata after registration");
 }
 {RuntimeHarness h;h.ReadyResources();h.simulator.actions.clear();h.simulator.calls=0;
  h.source.Set(h.f.ActorArray+8*3,h.f.Npc); // Duplicate replacement inside unchanged header.
  const auto result=h.Gate(4500);
  check(result.blockNative&&!result.recovered&&h.simulator.actions.empty(),"changed non-target census cell revokes recovery while scope-only gate remains blocked");
  h.Service(4750);check(!h.runtime.CensusComplete()&&h.runtime.censusFailure==11&&h.runtime.censusResets&&h.runtime.censusRejectedObject==h.f.Npc,"duplicate actor census cannot become complete and reports exact failure/object");
 }
 {RuntimeHarness h;h.ReadyResources();h.simulator.calls=0;h.simulator.actions.clear();
  h.installed=false;check(!h.Gate(4500).blockNative&&!h.simulator.calls,"lost installed five-hook lease refuses gate and native actions");
  check(h.runtime.gates.forwarded[0]==1&&h.runtime.gates.lastStage[0]==3&&h.runtime.gates.lastRemaining[0]>0,
   "forwarded callback records lost hook stage separately from budget exhaustion");
  h.installed=true;h.source.Set(h.f.Viewport+cfgprobe::ViewportWorld,h.f.source.frontendWorld);
  check(!h.Service(4750)&&!h.runtime.Armed(),"return to frontend retires common authority before old world action");
 }
 {RuntimeHarness h;h.ReadyResources();check(!h.runtime.Arm(h.f.Menu(),1,false,5000)&&h.runtime.Armed(),"stale submission cannot replace active generation");
  check(h.runtime.Arm(h.f.Menu(),2,false,5000)&&!h.runtime.Armed()&&!h.runtime.WarmProgress()&&!h.runtime.CensusComplete(),"new explicit Off generation invalidates all prior scope/cache/history/resources");
  const auto reads=h.source.count,calls=h.simulator.calls;h.source.count=0;
  check(!h.Service(5250)&&!h.Gate(5250).blockNative&&!h.source.count&&h.simulator.calls==calls,"disabled generation adds zero reads or calls");
  (void)reads;
 }
 {RuntimeHarness h;h.ReadyResources();bool nestedService=true,nestedBlock=true;
  h.simulator.nested=[&](unsigned){nestedService=h.Service(5000);nestedBlock=h.Gate(5000).blockNative;};
  h.Service(4500);
  check(!nestedService&&!nestedBlock&&h.runtime.counters.nestedRefusals>=2,"native reentry cannot open a new runtime callback or duplicate caps");
 }
 {RuntimeHarness h;h.ReadyResources();bool nestedBlocked=false;unsigned nestedActions=0;
  for(unsigned i=0;i<6;++i)h.Service(4000+100*i);
  h.simulator.positions[h.actor.address]={50,0,0};
  h.simulator.nested=[&](unsigned){const auto before=h.simulator.actions.size();nestedBlocked=h.borrowedGate().blockNative;
   nestedActions=static_cast<unsigned>(h.simulator.actions.size()-before);};
  for(unsigned i=0;i<3&&!nestedBlocked;++i)h.Service(4750+250*i);
  check(nestedBlocked&&!nestedActions,"shared outer callback blocks nested AI commitment without recursive native calls");
 }
 {RuntimeHarness h;h.ReadyResources();h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();
  h.simulator.nested=[&](unsigned){h.runtime.Cancel();};
  h.Service(4500);
  check(!h.runtime.Armed()&&!h.Gate(4750).blockNative&&h.simulator.actions.empty(),"Cancel inside native query revokes held scope/witness/census without stale suppression or mutation");
 }
 {RuntimeHarness h;h.ReadyResources();
  h.source.Set(h.f.Npc+0x3A0,h.f.Human);h.source.Set(h.f.Root+0x20,h.f.Car);h.Gate(4500);
  h.source.Set(h.f.Npc+0x3A0,std::uintptr_t{});h.source.Set(h.f.Root+0x20,h.f.Npc);
  check(!h.Gate(4750,b::Seam::Completion).blockNative,"ownership-only human witness tombstones prior AI despite broken movement/root fullproof");
 }
 {RuntimeHarness h;h.ReadyResources();const auto inProgress=h.f.r;
  h.f.Phase("WaitingPostMatch");for(const auto& item:h.f.r.bytes)h.source.bytes[item.first]=item.second;
  check(!h.Service(4500)&&!h.runtime.Armed(),"terminal native phase retires current actor leases before later callbacks");
  h.source.bytes=inProgress.bytes;h.source.count=0;
  check(!h.Service(4750)&&!h.Gate(4750).blockNative,"same-world InProgress cannot revive a retired actor generation without explicit submission");
 }
 {RuntimeHarness h(true,0,true);h.ActualHookProof();
  for(unsigned i=0;i<32;++i){h.Service(100+50*i);peak=(std::max)(peak,h.source.count);}
  if(!h.runtime.CensusComplete())std::printf("Large census armed%u warm%u volumes%u cells%u steps%u reads%u\n",h.runtime.Armed(),h.runtime.WarmProgress(),h.runtime.VolumeCount(),h.runtime.counters.cells,h.runtime.counters.censusSteps,h.source.count);
  check(h.runtime.CensusComplete()&&h.runtime.VolumeCount()==3,"557 authored actors including serial0 unrelated cells establish complete three-volume roster");
  for(unsigned i=0;i<10;++i){h.Service(2000+100*i);peak=(std::max)(peak,h.source.count);}
  h.simulator.positions[h.actor.address]={50,0,0};h.simulator.actions.clear();h.simulator.calls=0;
  check(h.Gate(3000,b::Seam::VehicleStart).blockNative&&!h.simulator.calls,"four occupied seats remain blocked independently of large census work");
  for(unsigned i=0;i<5;++i){h.Service(3000+100*i);peak=(std::max)(peak,h.source.count);}
  check(h.simulator.positions[h.actor.address].x==0,"557-actor three-volume four-occupant recovery stays within fixed composed allowance");
 }
 check(peak<ds::ReadLimit,"owned end-to-end runtime stays within fixed reader budget");
 std::printf("Challenge escape portable runtime/finite census/dispatch orchestration: %u checks %u failures peak%u/%u; owned simulator only.\n",checks,failures,peak,ds::ReadLimit);
 return failures?1:0;
}
