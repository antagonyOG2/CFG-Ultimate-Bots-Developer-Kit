#include "DifficultyPolicy.hpp"
#include <cassert>
#include <limits>
#include <cstdio>
#include <initializer_list>
#include <map>
#include "../RuntimeProbe/KnifeSequenceBindings.generated.hpp"

// Compile the actual production members against a bounded synthetic memory
// reader. Engine primitives below are fixture seams; policy/branch ordering,
// source selection, native owner links, serial rereads and world checks are
// the production code, not a second implementation of DifficultyDamage.
namespace cfgprobe::lifetime {
struct SlotObservation{std::uintptr_t object=0;int serial=0;bool HasAssignedSerial()const{return object&&serial>0;}};
}
struct FixtureReader {
 std::map<std::uintptr_t,std::uintptr_t> memory,types;
 std::map<std::uintptr_t,int> serials;
 unsigned reads=0,cap=1536;bool safe=true;std::uintptr_t staleOnRecheck=0;
 bool Step(){return reads++<cap;}
 template<class T>bool Read(std::uintptr_t at,T& out){if(!Step())return false;auto i=memory.find(at);if(i==memory.end())return false;out=static_cast<T>(i->second);return true;}
};
namespace cfgspawn {
bool ObjectSlot(FixtureReader& r,unsigned,std::uintptr_t object,cfgprobe::lifetime::SlotObservation& out){
 if(!r.Step())return false;auto i=r.serials.find(object);if(i==r.serials.end())return false;out={object,i->second};return true;
}
bool ObjectIs(FixtureReader& r,std::uintptr_t object,std::uintptr_t type){return r.Step()&&r.types[object]==type;}
}
namespace cfgobserver {
bool Same(FixtureReader& r,std::uintptr_t at,std::uintptr_t expected){std::uintptr_t observed=0;return r.Read(at,observed)&&observed==expected;}
}
namespace cfgjason {
bool SameSlot(FixtureReader& r,unsigned registry,const cfgprobe::lifetime::SlotObservation& slot){
 cfgprobe::lifetime::SlotObservation observed{};
 return r.staleOnRecheck!=slot.object&&cfgspawn::ObjectSlot(r,registry,slot.object,observed)&&slot.HasAssignedSerial()&&observed.serial==slot.serial;
}
}
namespace cfgprimary {bool LookupSafe(FixtureReader& r,std::uintptr_t){return r.Step()&&r.safe;}}
namespace cfgkill::disk {constexpr std::uintptr_t LevelReflection=0x100,LevelConstructor=0x108,OwningWorld=0xB0;}
unsigned fixtureThread=1;
unsigned GetCurrentThreadId(){return fixtureThread;}
std::uint64_t GetTickCount64(){return 1;}
bool bundleStarted=true;
struct {bool active=true;}bundleSchedule;
struct KillFixture{bool exclusive=false;bool Exclusive(std::uint64_t)const{return exclusive;}};
struct {KillFixture policy;}bundleKill;
struct {unsigned threadId=1;std::uintptr_t world=0x10000,module=0x140000000;}plan;
struct {cfgprobe::lifetime::SlotObservation pawn{0x20000,1},killerController{0x30000,1};}bundleJason;
struct DifficultyFixture {
 FixtureReader r_;struct {unsigned objectArray=1;}context;
 bool contextOk=true,jasonOk=true,isKillerAlive=true;
 std::uintptr_t knownVictim=0x40000;
 static constexpr std::uintptr_t Weapon=0x50000,Knife=0x60000,Level=0x70000,LevelType=0x71000,WeaponType=0x72000,KnifeType=0x73000;
 DifficultyFixture(){
  for(auto object:{bundleJason.pawn.object,bundleJason.killerController.object,knownVictim,Weapon,Knife,Level})r_.serials[object]=1;
  for(auto object:{bundleJason.pawn.object,knownVictim,Weapon,Knife})r_.memory[object+0x20]=Level;
  r_.memory[Level+cfgkill::disk::OwningWorld]=plan.world;r_.types[Level]=LevelType;
  r_.memory[bundleJason.pawn.object+cfgknife::disk::Role]=3;
  r_.memory[Weapon+0x548]=bundleJason.pawn.object;r_.types[Weapon]=WeaponType;
  r_.memory[Knife+cfgknife::disk::ThrownBy]=bundleJason.pawn.object;r_.types[Knife]=KnifeType;
 }
 bool Context(){return r_.Step()&&contextOk;}
 bool Jason(){return r_.Step()&&jasonOk;}
 bool KnownLiving(std::uintptr_t victim){return r_.Step()&&victim==knownVictim;}
 bool Alive(std::uintptr_t,bool& out){out=isKillerAlive;return r_.Step();}
 bool NativeClass(std::uintptr_t reflection,std::uintptr_t,std::uintptr_t& out){
  out=reflection==cfgkill::disk::LevelReflection?LevelType:reflection==cfgknife::disk::ThrowableReflection?KnifeType:0;
  return r_.Step()&&out;
 }
 bool HeldWeapon(std::uintptr_t owner,std::uintptr_t weapon){
  return cfgspawn::ObjectIs(r_,weapon,WeaponType)&&cfgobserver::Same(r_,weapon+0x548,owner);
 }
#include "DifficultyRuntimeMembers.hpp"
 float Hit(std::uintptr_t causer,unsigned difficulty=2,std::uintptr_t instigator=0){return DifficultyDamage(difficulty,knownVictim,20.f,instigator,causer);}
};

static void ObservationTests(){
 for(auto source:{bundleJason.pawn.object,DifficultyFixture::Weapon,DifficultyFixture::Knife}){
  DifficultyFixture f;assert(f.Hit(source)==40.f);assert(f.r_.reads<1536);
  DifficultyFixture easy;assert(easy.Hit(source,0)==10.f);
  DifficultyFixture normal;assert(normal.Hit(source,1)==20.f&&normal.r_.reads==0);
 }
 {DifficultyFixture f;assert(f.Hit(DifficultyFixture::Knife,2,bundleJason.killerController.object)==40.f);}
 {DifficultyFixture f;assert(f.Hit(DifficultyFixture::Knife,2,0xDEAD)==20.f);}
 {DifficultyFixture f;f.contextOk=false;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.jasonOk=false;assert(f.Hit(bundleJason.pawn.object)==20.f);}
 {DifficultyFixture f;f.isKillerAlive=false;assert(f.Hit(DifficultyFixture::Weapon)==20.f);}
 {DifficultyFixture f;f.r_.memory[DifficultyFixture::Knife+cfgknife::disk::ThrownBy]=f.knownVictim;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.r_.memory[DifficultyFixture::Weapon+0x548]=f.knownVictim;assert(f.Hit(DifficultyFixture::Weapon)==20.f);}
 {DifficultyFixture f;f.r_.types[DifficultyFixture::Knife]=0xBAD;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.r_.memory[DifficultyFixture::Level+cfgkill::disk::OwningWorld]=0xBAD;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.r_.memory[bundleJason.pawn.object+cfgknife::disk::Role]=2;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 for(auto source:{bundleJason.pawn.object,bundleJason.killerController.object,std::uintptr_t{0x40000},DifficultyFixture::Knife}){
  DifficultyFixture f;f.r_.staleOnRecheck=source;assert(f.Hit(DifficultyFixture::Knife)==20.f);
 }
 {DifficultyFixture f;f.r_.serials[DifficultyFixture::Knife]=0;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.r_.safe=false;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;f.r_.cap=6;assert(f.Hit(DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;assert(f.DifficultyDamage(2,bundleJason.pawn.object,20.f,0,DifficultyFixture::Knife)==20.f);}
 {DifficultyFixture f;fixtureThread=2;assert(f.Hit(DifficultyFixture::Knife)==20.f&&f.r_.reads==0);fixtureThread=1;}
 {DifficultyFixture f;bundleKill.policy.exclusive=true;assert(f.Hit(DifficultyFixture::Knife)==20.f);bundleKill.policy.exclusive=false;}
 {DifficultyFixture f;bundleSchedule.active=false;assert(f.Hit(DifficultyFixture::Knife)==20.f);bundleSchedule.active=true;}
 {DifficultyFixture f;bundleStarted=false;assert(f.Hit(DifficultyFixture::Knife)==20.f);bundleStarted=true;}
}
int main(){
 ObservationTests();
 using namespace cfgdifficulty;
 for(auto source:{Source::Jason,Source::Weapon,Source::Projectile}){
  assert(Scale(20.f,0,source)==10.f);assert(Scale(20.f,1,source)==20.f);assert(Scale(20.f,2,source)==40.f);
 }
 assert(Scale(20.f,2,Source::None)==20.f);assert(Scale(20.f,3,Source::Jason)==20.f);
 assert(Scale(0.f,2,Source::Jason)==0.f);assert(Scale(-20.f,2,Source::Jason)==-20.f);
 assert(Scale(10000.f,2,Source::Jason)==10000.f);
 const float nan=std::numeric_limits<float>::quiet_NaN(),inf=std::numeric_limits<float>::infinity();
 assert(std::isnan(Scale(nan,2,Source::Jason)));assert(Scale(inf,2,Source::Jason)==inf);
 assert(SpeedScale(0)==1.f&&SpeedScale(1)==1.f&&SpeedScale(2)==2.f);
 std::puts("Difficulty PASS: actual production member fixture, owner/weapon/projectile/world/serial/budget rejection, 0.5/1/2 arithmetic; no game access.");
}
