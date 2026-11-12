#define main CfgResourceFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "PreviousWorldObservation.hpp"
int main(){
 Fixture f;f.cap=100000;const auto worldType=f.Object(),levelType=f.Object(),packageType=f.Object();
 const auto world=f.Object(worldType),level=f.Object(levelType),owned=f.Object(),asset=f.Object(),package=f.Object(packageType),otherPackage=f.Object(packageType);
 const auto serial=[&](std::uintptr_t object,int value){int index=0;CHECK(f.Read(object+12,index));f.Put(f.objects.data+24ULL*static_cast<unsigned>(index)+16,value);};
 serial(world,10);serial(level,11);serial(owned,12);f.Put(world+cfgoutro::disk::PersistentLevel,level);f.Put(owned+0x20,level);f.Put(asset+0x20,otherPackage);f.Put(world+0x20,package);
 cfgworlddiag::Previous previous;CHECK(cfgworlddiag::Record(f,f.objects,world,previous,packageType));
 f.reads=0;CHECK(cfgworlddiag::Reaches(f,f.objects,owned,previous)&&f.reads<64);
 f.reads=0;CHECK(!cfgworlddiag::Reaches(f,f.objects,asset,previous));
 f.Put(owned+0x20,owned);f.reads=0;CHECK(!cfgworlddiag::Reaches(f,f.objects,owned,previous)&&f.reads<64);
 serial(world,99);f.Put(owned+0x20,world);f.reads=0;CHECK(cfgworlddiag::Reaches(f,f.objects,owned,previous));
 // Native UWorld/level commonly have serial0. Later assignment is irrelevant
 // to diagnostic survival; recorded index/object/class must still match.
 serial(world,0);serial(level,0);f.reads=0;
 CHECK(cfgworlddiag::Record(f,f.objects,world,previous,packageType)&&previous.world.serial==0&&previous.level.serial==0);
 f.reads=0;CHECK(cfgworlddiag::StillCurrent(f,f.objects,previous.world,worldType));
 CHECK(cfgworlddiag::StillCurrent(f,f.objects,previous.level,levelType));
 f.Put(world+cfgprobe::ObjectClass,levelType);f.reads=0;
 CHECK(!cfgworlddiag::StillCurrent(f,f.objects,previous.world,worldType));
 f.Put(world+cfgprobe::ObjectClass,worldType);serial(world,1);f.reads=0;
 CHECK(cfgworlddiag::StillCurrent(f,f.objects,previous.world,worldType));
 serial(world,0);f.Put(level+cfgprobe::ObjectClass,worldType);f.reads=0;
 CHECK(!cfgworlddiag::StillCurrent(f,f.objects,previous.level,levelType));
 f.Put(level+cfgprobe::ObjectClass,levelType);f.reads=0;
 CHECK(cfgworlddiag::StillCurrent(f,f.objects,previous.world,worldType)&&cfgworlddiag::StillCurrent(f,f.objects,previous.level,levelType));
 {auto leaked=f;std::uint32_t flags=0;
  leaked.Put(leaked.objects.data+24ULL*static_cast<unsigned>(previous.world.index)+8,std::uint32_t{0x30000000});
  leaked.Put(leaked.objects.data+24ULL*static_cast<unsigned>(previous.world.index)+16,std::int32_t{777});leaked.reads=0;
  CHECK(cfgworlddiag::Alive(leaked,leaked.objects,previous.world,worldType,flags)&&flags==0x30000000);
  CHECK(!cfgworlddiag::NoLongerCurrent(leaked,leaked.objects,previous)); // flags/serial do not release load gate
  // Production observation MUST still reject pending-kill; diagnostic grants nothing.
  cfgprobe::lifetime::SlotObservation refused;CHECK(!cfgspawn::ObjectSlot(leaked,leaked.objects,world,refused));
  leaked.Put(leaked.objects.data+24ULL*static_cast<unsigned>(previous.world.index),std::uintptr_t{});
  CHECK(cfgworlddiag::NoLongerCurrent(leaked,leaked.objects,previous));
  leaked.Put(leaked.objects.data+24ULL*static_cast<unsigned>(previous.level.index),std::uintptr_t{});
  CHECK(!cfgworlddiag::Alive(leaked,leaked.objects,previous.world,worldType,flags));
  CHECK(cfgworlddiag::Alive(leaked,leaked.objects,previous.package,packageType,flags));
  leaked.Put(owned+0x20,package);CHECK(cfgworlddiag::Reaches(leaked,leaked.objects,owned,previous));
  leaked.Put(package+cfgprobe::ObjectClass,worldType);CHECK(!cfgworlddiag::Reaches(leaked,leaked.objects,owned,previous));}
 {auto before=f;before.reads=0;
  unsigned diagnostic=0;CHECK(!cfgworlddiag::NoLongerCurrent(before,before.objects,previous));
  CHECK(cfgworlddiag::ObserveAtLoad(before,before.objects,previous,[&](unsigned observed){diagnostic=observed;})==1&&diagnostic==1);
  auto retry=previous;before.cap=0;
  CHECK(!cfgworlddiag::Record(before,before.objects,world,retry,packageType)&&!retry.world.object&&retry.recordAttempted&&retry.generation==previous.generation&&!retry.checked);
  before.cap=100000;before.reads=0;
  CHECK(cfgworlddiag::ClassifyMapReference(before,before.objects,owned,previous)==cfgretain::MapReference::OldMap);
  CHECK(cfgworlddiag::ClassifyMapReference(before,before.objects,asset,previous)==cfgretain::MapReference::Unrelated);
  before.Put(owned+0x20,owned);CHECK(cfgworlddiag::ClassifyMapReference(before,before.objects,owned,previous)==cfgretain::MapReference::Unproven);
  before.cap=before.reads;CHECK(!cfgworlddiag::NoLongerCurrent(before,before.objects,previous));}
 {const auto cameraType=f.Object(),camera=f.Object(cameraType),controller=f.Object();
  f.Put(f.module+cfgcamera::disk::CameraClassCache,cameraType);f.Put(f.module+cfgcamera::disk::CameraConstructorCache,cameraType);
  f.Put(controller+cfgprobe::ControllerCameraManager,camera);std::uintptr_t observed=0;
  CHECK(cfgworlddiag::Camera(f,f.objects,f.module,controller,observed)&&observed==camera);
  f.Put(camera+cfgprobe::ObjectClass,package);CHECK(!cfgworlddiag::Camera(f,f.objects,f.module,controller,observed)&&!observed);
  f.Put(camera+cfgprobe::ObjectClass,cameraType);f.Put(f.module+cfgcamera::disk::CameraConstructorCache,package);
  CHECK(!cfgworlddiag::Camera(f,f.objects,f.module,controller,observed)&&!observed);}
 std::printf("Previous world/level/package diagnostic: pending-kill and later serial remain alive; exact slot/class reuse refuses; package-only Outer reach: %d failures; read-only fixture\n",failures);
 return failures?1:0;
}
