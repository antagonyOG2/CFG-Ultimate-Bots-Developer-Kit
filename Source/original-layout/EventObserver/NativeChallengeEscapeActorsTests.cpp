#define CFG_ESCAPE_SCOPE_FIXTURE_ONLY
#include "NativeChallengeEscapeScopeTests.cpp"
#undef CFG_ESCAPE_SCOPE_FIXTURE_ONLY
#include "NativeChallengeEscapeActors.hpp"
namespace act=cfgchallengeescape::actors;
namespace d=cfgchallengeescape::disk;
struct ActorFixture:ScopeFixture {
 static constexpr std::uintptr_t Npc=0x2000000,Ai=0x2010000,Root=0x2020000,Movement=0x2030000,
  Car=0x2040000,CarRoot=0x2050000,CarMove=0x2060000,Seat=0x2070000,Volume=0x2080000,Sphere=0x2090000,
  Passenger=0x20A0000,PassengerAi=0x20B0000,PassengerRoot=0x20C0000,PassengerMove=0x20D0000,
  PassengerSeat=0x20E0000,Human=0x20F0000,ActorType=0x2300000,AiType=0x2310000,
  PrimitiveType=0x2320000,MovementType=0x2330000,CarType=0x2340000,CarMovementType=0x2350000,
  SeatType=0x2360000,VolumeType=0x2370000,SphereType=0x2380000,ActorArray=0x2F00000,SeatArray=0x2F10000;
 unsigned nextSlot=8;
 void Type(std::uintptr_t type,std::uintptr_t cache,std::uintptr_t ctor,std::uintptr_t parent=0){
  r.Set(Module+cache,type);r.Set(Module+ctor,type);r.Set(type+cfgprobe::ClassSuper,parent);
  r.Set(type+cfgspawn::ClassDefaultObject,type+0x8000);r.Set(type+cfgreflection::disk::Children,std::uintptr_t{});
 }
 void Object(std::uintptr_t object,std::uintptr_t type,std::uintptr_t outer){
  if(type==SphereType){r.Set(object+0x190,std::array<float,8>{{0,0,0,0,1,1,1,0}});r.Set(object+0x6F8,5.0f);}
  r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{});
  r.Set(object+cfgprobe::ObjectClass,type);r.Set(object+0x20,outer);r.Set(object+0x110,std::uint8_t{3});
  r.Set(object+8,std::uint32_t{});
  r.Set(object+cfgprobe::lifetime::ObjectIndexOffset,static_cast<int>(nextSlot));
  r.Set(Registry+24ULL*nextSlot,object);r.Set(Registry+24ULL*nextSlot+8,std::uint32_t{});
  r.Set(Registry+24ULL*nextSlot+12,std::uint32_t{});
  r.Set(Registry+24ULL*nextSlot+16,static_cast<int>(1000+nextSlot));r.Set(Registry+24ULL*nextSlot+20,std::uint32_t{});++nextSlot;
 }
 template<std::size_t N>void Specs(std::uintptr_t owner,const std::array<act::Spec,N>& specs){
  for(const auto& spec:specs){std::string name,kind;
   for(const auto ch:spec.name)name.push_back(static_cast<char>(ch));
   for(const auto ch:spec.kind)kind.push_back(static_cast<char>(ch));
   Add(owner,name.c_str(),kind.c_str(),spec.offset,1,spec.bytes);
   if(spec.mask)r.Set(heads[owner]+0x70,std::array<unsigned char,4>{{1,0,1,spec.mask}});
  }
 }
 void CounselorObject(std::uintptr_t npc,std::uintptr_t ai,std::uintptr_t root,std::uintptr_t move){
  Object(npc,Counselor,Level);Object(ai,AiType,Level);Object(root,PrimitiveType,npc);Object(move,MovementType,npc);
  r.Set(npc+0x3A0,ai);r.Set(ai+0x370,npc);r.Set(npc+0x160,root);r.Set(npc+0x3D0,move);
  r.Set(npc+0x15C0,std::uintptr_t{});r.Set(npc+0x1518,std::uintptr_t{});
  for(const auto offset:{0x1525,0xE54,0x1070,0x161C,0x15D4})r.Set(npc+offset,std::uint8_t{});
 }
 ActorFixture(){
  for(unsigned i=0;i<8;++i)r.Set(Registry+24ULL*i+12,std::uint32_t{});
  Type(ActorType,d::Actor,d::ActorCtor);Type(AiType,d::AIController,d::AIControllerCtor,ActorType);
  Type(Counselor,d::SCCounselorCharacter,d::SCCounselorCharacterCtor,ActorType);
  Type(PrimitiveType,d::PrimitiveComponent,d::PrimitiveComponentCtor);
  Type(MovementType,d::MovementComponent,d::MovementComponentCtor);
  Type(CarType,d::SCDriveableVehicle,d::SCDriveableVehicleCtor,ActorType);
  Type(CarMovementType,d::WheeledVehicleMovementComponent,d::WheeledVehicleMovementComponentCtor,MovementType);
  Type(SeatType,d::SCVehicleSeatComponent,d::SCVehicleSeatComponentCtor);
  Type(VolumeType,d::SCEscapeVolume,d::SCEscapeVolumeCtor,ActorType);
  Type(SphereType,d::SphereComponent,d::SphereComponentCtor,PrimitiveType);
  r.Set(Module+cfgradarworld::disk::LevelClassCache,LevelType);r.Set(Module+cfgradarworld::disk::LevelConstructorCache,LevelType);
  r.Set(LevelType+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(Level+cfgkill::disk::OwningWorld,World);
  r.Set(Level+8,std::uint32_t{});
  r.Set(Module+d::PlayerController,ControllerType);r.Set(Module+d::PlayerControllerCtor,ControllerType);
  r.Set(ControllerType+cfgprobe::ClassSuper,ActorType);
  Specs(Counselor,act::CounselorSpecs);Specs(AiType,std::array<act::Spec,1>{{{u"Pawn",u"ObjectProperty",0x370,8}}});
  Specs(CarType,std::array<act::Spec,3>{{{u"RootComponent",u"ObjectProperty",0x160,8},
   {u"VehicleMovement",u"ObjectProperty",0x3D0,8},{u"Seats",u"ArrayProperty",0x4E8,16}}});
  Specs(SeatType,act::SeatSpecs);Specs(VolumeType,std::array<act::Spec,1>{{{u"EscapeVolume",u"ObjectProperty",0x368,8}}});
  // The native Seats array owns its typed ObjectProperty inner.
  const auto seatsProperty=heads[CarType],inner=field;field+=0x1000;const auto kind=field;field+=0x1000;
  Label(inner,"Seats");Label(kind,"ObjectProperty");r.Set(inner+cfgprobe::ObjectClass,kind);r.Set(inner+0x20,seatsProperty);
  r.Set(inner+cfgreflection::disk::Next,std::uintptr_t{});r.Set(inner+cfgreflection::disk::PropertyDimensions,int{1});
  r.Set(inner+cfgreflection::disk::PropertyElementSize,int{8});r.Set(inner+cfgreflection::disk::PropertyOffset,int{});
  r.Set(inner+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000214ULL});r.Set(inner+0x70,SeatType);r.Set(seatsProperty+0x70,inner);
  CounselorObject(Npc,Ai,Root,Movement);CounselorObject(Passenger,PassengerAi,PassengerRoot,PassengerMove);
  Object(Car,CarType,Level);Object(CarRoot,PrimitiveType,Car);Object(CarMove,CarMovementType,Car);
  Object(Seat,SeatType,Car);Object(PassengerSeat,SeatType,Car);Object(Volume,VolumeType,Level);Object(Sphere,SphereType,Volume);
  Object(Human,ControllerType,Level);r.Set(Human+0x370,Npc);
  r.Set(Car+0x160,CarRoot);r.Set(Car+0x3D0,CarMove);r.Set(Car+0x3D8,std::uint8_t{1});r.Set(Car+0x47C,std::uint8_t{});
  r.Set(Car+0x4E8,act::List{SeatArray,2,2});r.Set(SeatArray,Seat);r.Set(SeatArray+8,PassengerSeat);
  r.Set(Seat+0x548,Car);r.Set(PassengerSeat+0x548,Car);r.Set(Seat+0x500,std::uintptr_t{});r.Set(PassengerSeat+0x500,std::uintptr_t{});
  r.Set(Seat+0x511,std::uint8_t{1});r.Set(PassengerSeat+0x511,std::uint8_t{});r.Set(Volume+0x368,Sphere);
  const std::array<std::uintptr_t,7> roster{{Npc,Ai,Car,Volume,Passenger,PassengerAi,Human}};
  r.Set(Level+cfgradarworld::disk::LevelActors,act::List{ActorArray,static_cast<int>(roster.size()),static_cast<int>(roster.size())});
  r.Set(ActorArray,roster);r.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(nextSlot));
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
 void Occupied(){r.Set(Seat+0x500,Npc);r.Set(Npc+0x15C0,Seat);r.Set(PassengerSeat+0x500,Passenger);r.Set(Passenger+0x15C0,PassengerSeat);}
 void WideCounselorHierarchy(unsigned counselorFields=365){
  constexpr std::uintptr_t characterType=0x2500000,pawnType=0x2510000;
  field=0x6000000;heads[Counselor]=heads[ActorType]=0;
  r.Set(Counselor+cfgprobe::ClassSuper,characterType);r.Set(characterType+cfgprobe::ClassSuper,pawnType);
  r.Set(pawnType+cfgprobe::ClassSuper,ActorType);
  Specs(Counselor,std::array<act::Spec,4>{{act::CounselorSpecs[1],act::CounselorSpecs[2],act::CounselorSpecs[4],act::CounselorSpecs[5]}});
  Specs(characterType,std::array<act::Spec,1>{{act::CounselorSpecs[3]}});
  Specs(pawnType,std::array<act::Spec,2>{{act::CounselorSpecs[0],act::CounselorSpecs[7]}});
  Specs(ActorType,std::array<act::Spec,1>{{act::CounselorSpecs[6]}});
  const auto pad=[&](std::uintptr_t owner,unsigned existing,unsigned total){
   Add(owner,"UnrelatedField","IntProperty",0,1,4);++existing;
   cfgnames::Words name{};r.Read(heads[owner]+cfgnames::disk::ObjectName,name);
   for(;existing<total;++existing){const auto node=field;field+=0x1000;
    r.Set(node+cfgnames::disk::ObjectName,name);r.Set(node+cfgreflection::disk::Next,heads[owner]);heads[owner]=node;
   }
   r.Set(owner+cfgreflection::disk::Children,heads[owner]);
  };
  pad(Counselor,4,counselorFields);pad(characterType,1,272);pad(pawnType,2,66);pad(ActorType,1,195);
  // Enlarging this fixture adds name entries that would otherwise overlap the
  // synthetic installed Overlap body near0x572000. Keep heap names separate
  // from pinned image bytes, as they are in the real CFG process.
  for(unsigned i=1;i<id;++i){std::uintptr_t old=0;std::uint32_t header=0;std::array<char,128> text{};
   const bool copied=r.Read(Chunk+8ULL*i,old)&&r.Read(old,header)&&r.Read(old+cfgnames::disk::EntryText,text);
   if(!copied)std::abort();const auto relocated=std::uintptr_t{0x7000000}+0x1000ULL*i;
   r.Set(relocated,header);r.Set(relocated+cfgnames::disk::EntryText,text);r.Set(Chunk+8ULL*i,relocated);
  }
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
};
struct ChangingIdentityReader {
 ChallengeReader& reader;std::uintptr_t item,object;unsigned mode,samples=0;
 template<class T>bool Read(std::uintptr_t at,T& out){
  const bool good=reader.Read(at,out);
  if(good&&at==item&&sizeof(T)==20&&++samples==1){
   if(mode==1)reader.Set(item+8,std::uint32_t{0x10000000});
   if(mode==2)reader.Set(item+16,int{99999});
   if(mode==3)reader.Set(item,object+0x1000);
   if(mode==4)reader.Set(object+cfgprobe::lifetime::ObjectIndexOffset,int{-1});
   if(mode==5)reader.Set(item+12,std::uint32_t{0xFFFFFFFF});
  }
  return good;
 }
};
#ifndef CFG_ESCAPE_ACTORS_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0,peak=0;const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 const auto current=[](){return true;};
 const auto eligible=[](const act::CounselorProof& p){return p.controllerKind==act::ControllerKind::AI;};
 for(unsigned fault=0;fault<4;++fault){ActorFixture f;f.WideCounselorHierarchy(fault==1?513u:365u);
  ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;scope::Runtime authority;scope::Witness w;
  authority.Arm(f.Menu(),1,true,0);check(authority.Observe(r,42,100,w),"wide native fixture scope binds");r.count=0;
  if(fault==2)r.Set(f.heads[f.Counselor]+cfgreflection::disk::Next,f.heads[f.Counselor]);
  if(fault==3)r.cap=1024;
  act::CounselorProof proof;
  check(act::ObserveCounselor(r,f.Module,w,current,f.Npc,proof)==(!fault),"captured CFG365/272/66/195 field hierarchy accepted, class overflow/cycle/read exhaustion refused");
  if(!fault){check(proof.controllerKind==act::ControllerKind::AI&&act::CurrentCounselor(r,f.Module,w,current,proof),"wide hierarchy retains exact AI/component/metadata ownership");peak=(std::max)(peak,r.count);}
 }
 for(unsigned fault=0;fault<15;++fault){ActorFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);check(authority.Observe(r,42,100,w),"real scope fixture binds");r.count=0;
  if(fault==1)r.Set(f.Npc+0x110,std::uint8_t{1});
  if(fault==2)r.Set(f.Npc+0x20,f.World);
  if(fault==3)r.Set(f.Root+0x20,f.Passenger);
  if(fault==4)r.Set(f.Movement+0x20,f.Passenger);
  if(fault==5)r.Set(f.Module+d::SCCounselorCharacterCtor,f.ActorType);
  if(fault==6)r.Set(f.ActorArray,f.Car);
  if(fault==7)r.Set(f.Level+cfgradarworld::disk::LevelActors,act::List{f.ActorArray,16001,16001});
  if(fault==8)r.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
  if(fault==9)r.Set(f.Npc+0x3A0,std::uintptr_t{});
  if(fault==10)r.Set(f.Npc+0x3A0,f.Human);
  if(fault==11)r.Set(f.Ai+0x370,f.Passenger);
  if(fault==12)r.Set(f.Npc+0x1525,std::uint8_t{1});
  if(fault==13)r.Set(f.Npc+0x1070,std::uint8_t{1});
  if(fault==14)r.Set(f.Npc+0x1518,f.Volume);
  act::CounselorProof proof;const bool observed=act::ObserveCounselor(r,f.Module,w,current,f.Npc,proof);
  const bool expected=!fault||fault>=9;check(observed==expected,"counselor exact source and component proof, state classification separate from mutation");
  peak=(std::max)(peak,r.count);
  if(observed){check(act::CurrentCounselor(r,f.Module,w,current,proof),"same positive source/metadata revalidates without fresh census");
   if(fault==9)check(proof.controllerKind==act::ControllerKind::Unpossessed,"null is unpossessed, never fresh AI");
   if(fault==10)check(proof.controllerKind==act::ControllerKind::Human,"human possession classification retained for tombstone");
   if(fault==11)check(proof.controllerKind==act::ControllerKind::Unknown,"nonreciprocal AI is unknown, never fresh AI");
   if(fault==12||fault==13||fault==14)check(!proof.PreCommit(),"native escape spline/dead/escaped are distinct precommit witnesses");
   r.Set(f.Movement+0x20,f.Passenger);check(!act::CurrentCounselor(r,f.Module,w,current,proof),"changed movement owner revokes action witness");
  }
 }
 for(unsigned fault=0;fault<11;++fault){ActorFixture f;f.Occupied();ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);authority.Observe(r,42,100,w);r.count=0;
  if(fault==1)r.Set(f.Passenger+0x3A0,f.Human);
  if(fault==2)r.Set(f.PassengerSeat+0x548,f.Volume);
  if(fault==3)r.Set(f.Passenger+0x15C0,f.Seat);
  if(fault==4)r.Set(f.Passenger+0x15D4,std::uint8_t{1});
  if(fault==5)r.Set(f.PassengerSeat+0x511,std::uint8_t{1});
  if(fault==6)r.Set(f.SeatArray+8,f.Seat);
  if(fault==7)r.Set(f.CarMove+0x20,f.Volume);
  if(fault==8)r.Set(f.Car+0x47C,std::uint8_t{1});
  if(fault==9)r.Set(f.Seat+0x500,std::uintptr_t{});
  if(fault==10)r.Set(f.Passenger+0x3A0,std::uintptr_t{});
  act::CarProof proof;const bool observed=act::ObserveCar(r,f.Module,w,current,f.Car,eligible,proof);
  check(observed==(!fault),"all occupied seats, reciprocal ownership and one occupied driver required");peak=(std::max)(peak,r.count);
  if(observed){check(proof.occupied==2&&act::CurrentCar(r,f.Module,w,current,proof,eligible),"both occupants revalidated at action boundary");
   r.Set(f.Passenger+0x3A0,f.Human);check(!act::CurrentCar(r,f.Module,w,current,proof,eligible),"human passenger prevents whole-car suppression");}
 }
 {ActorFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);authority.Observe(r,42,100,w);r.count=0;act::VolumeProof p;
  check(act::ObserveVolume(r,f.Module,w,current,f.Volume,p)&&act::CurrentVolume(r,f.Module,w,current,p),"positive sphere source used without retaining serial-unassigned volume root");
  r.Set(f.Sphere+0x20,f.Car);check(!act::CurrentVolume(r,f.Module,w,current,p),"foreign sphere ownership refused");
 }
 for(unsigned fault=0;fault<9;++fault){ActorFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);authority.Observe(r,42,100,w);r.count=0;
  act::life::SlotObservation level;act::life::ReadSlot(r,w.registry,f.Level,level);
  const auto slot=w.registry.data+24ULL*static_cast<unsigned>(level.index);
  r.Set(slot+16,int{});act::Membership proof;
  check(act::ObserveMembership(r,f.Module,w,f.Npc,proof),"zero-serial Level permitted only as typed current strong-link borrow");
  if(fault==1)r.Set(slot,std::uintptr_t{});
  if(fault==2)r.Set(slot+8,std::uint32_t{0x20000000});
  if(fault==3)r.Set(f.Level+8,std::uint32_t{0x10});
  if(fault==4)r.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
  if(fault==5)r.Set(f.ActorArray,f.Car);
  if(fault==6)r.Set(proof.levelCell,std::uintptr_t{});
  if(fault==7)r.Set(f.Module+cfgradarworld::disk::LevelConstructorCache,f.ActorType);
  if(fault==8)r.Set(slot+16,int{12345});
  check(act::MembershipCurrent(r,f.Module,w,proof)==(!fault),"borrow revalidates exact registry/GC/type/world/cells and refuses changed serial");
  act::NativeProof forbidden;
  if(!fault)check(!act::ObserveNative(r,f.Module,w,f.Level,cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache,forbidden),"zero-serial Level never promoted to positive native action proof");
 }
 {ActorFixture f;f.Occupied();ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);authority.Observe(r,42,100,w);r.count=0;
  r.Set(f.Passenger+0x3A0,f.Human);r.Set(f.Passenger+0x15D4,std::uint8_t{1});bool humanSeen=false;act::CarProof proof;
  const auto observeHuman=[&](const act::CounselorProof& p){humanSeen|=p.controllerKind==act::ControllerKind::Human;return p.controllerKind==act::ControllerKind::AI;};
  check(!act::ObserveCar(r,f.Module,w,current,f.Car,observeHuman,proof)&&humanSeen,"leaving human invalidates history before car precommit refusal");
 }
 {ActorFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;r.cap=32768;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);authority.Observe(r,42,100,w);r.count=0;
  r.Set(f.Npc+0x3A0,f.Human);r.Set(f.Movement+0x20,f.Car);r.Set(f.Root+0x20,f.Car);
  act::CounselorOwnershipProof ownership;act::CounselorProof recovery;
  check(act::ObserveCounselorOwnership(r,f.Module,w,current,f.Npc,ownership)&&ownership.HumanOrUnknown(),"human ownership observed even with unusable root and movement");
  check(!act::ObserveCounselor(r,f.Module,w,current,f.Npc,recovery),"ownership-only proof cannot authorize a recovery action");
  r.Set(f.Npc+0x3A0,std::uintptr_t{});check(!act::CurrentCounselorOwnership(r,f.Module,w,current,ownership),"changed controller revokes borrowed ownership witness");
 }
 for(unsigned mode=0;mode<6;++mode){ActorFixture f;ChallengeReader r;static_cast<StageReader&>(r)=f.r;
  scope::Runtime authority;scope::Witness w;authority.Arm(f.Menu(),1,true,0);check(authority.Observe(r,42,100,w),"two-sample identity fixture scope");
  int index=-1;r.Read(f.Npc+cfgprobe::lifetime::ObjectIndexOffset,index);r.count=0;
  ChangingIdentityReader changing{r,w.registry.data+24ULL*static_cast<unsigned>(index),f.Npc,mode};cfgprobe::lifetime::SlotObservation slot;
  check(act::Positive(changing,w.registry,f.Npc,slot)==(mode==0||mode==5),
   "two fresh grouped samples detect intervening flag, serial, object and index changes; ignored cluster is not authority");
 }
 check(peak<32768,"bounded actor cold proof stays inside32768 owned fixture read cap");
 std::printf("Challenge escape native actor/all-seat proof: %u checks, %u failures, peak%u/32768; owned memory only.\n",checks,failures,peak);
 return failures?1:0;
}
#endif
