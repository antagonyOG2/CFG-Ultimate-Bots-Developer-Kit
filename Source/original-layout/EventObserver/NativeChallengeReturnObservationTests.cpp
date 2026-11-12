#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeChallengeReturnObservation.hpp"
struct ReturnReader {
 OwnedByteReader& memory;unsigned reads=0,cap=4096;
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=cap)return false;++reads;return memory.Read(at,out);}
};
int main(){unsigned checks=0,failures=0,peak=0;
 const auto check=[&](bool good,const char* name){++checks;if(!good){++failures;std::printf("FAIL %s\n",name);}};
 using F=NativeFixture;constexpr std::uintptr_t controllerType=0x1950000,gameplayType=0x1960000;
 for(unsigned fault=0;fault<12;++fault){F f;auto& memory=f.fixture.r;
  f.fixture.Name(MenuFixture::Type,"SP_LobbyWidget_C");
  memory.Set(F::Controller+cfgprobe::ObjectClass,controllerType);f.Root(controllerType);
  memory.Set(MenuFixture::Base+cfgmenu::controllergate::MenuClass,controllerType);
  memory.Set(MenuFixture::Base+cfgmenu::controllergate::MenuConstructor,controllerType);f.Root(gameplayType);
  cfgmenu::active::Context origin;check(cfgmenu::active::Observe(memory,MenuFixture::Base,origin),"owned origin observed");
  // Old world/HUD pointers and identities can be gone. Only immutable process
  // roots and the actual newly observed context are source authority here.
  origin.world=0x55000;origin.mode=0x56000;origin.hud=0x57000;origin.stack=0x58000;origin.widget=0x59000;
  if(fault==1)f.fixture.Name(MenuFixture::Type,"ErrorPopupWidget_C");
  if(fault==2)memory.Set(F::World+cfgprobe::WorldNetDriver,F::Mode);
  if(fault==3)memory.Set(MenuFixture::Base+cfgmenu::controllergate::MenuConstructor,gameplayType);
  if(fault==4)memory.Set(F::Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
  if(fault==6)origin.engine+=0x1000;
  if(fault==7){int index=0;memory.Read(MenuFixture::Widget+cfgprobe::lifetime::ObjectIndexOffset,index);memory.Set(F::Registry+24ULL*static_cast<unsigned>(index)+16,int{});}
  if(fault==8)memory.Set(F::Mode+cfgprobe::ObjectClass,gameplayType);
  if(fault==9)origin.player+=0x1000;
  if(fault==10){memory.changeAt=MenuFixture::Base+cfgmenu::execdisk::FrontendConstructor;
   memory.changeSlot=F::Mode+cfgprobe::ObjectClass;memory.newPointer=gameplayType;}
  ReturnReader r{memory};if(fault==11)r.cap=16;cfgmenu::active::Context current;
  const bool accepted=cfgmenu::challengereturn::Observe(r,origin,fault==5?43u:42u,current);
  check(accepted==(!fault),"new positive lobby admitted without old HUD; popup/network/gameplay/foreign roots/changed class/budget refused");
  check(accepted?current.world==F::World&&!(current==origin):!current.world,"no partial current owner proof on refusal");
  peak=(std::max)(peak,r.reads);
 }
 std::printf("Challenge rebuilt-lobby source observation: %u checks %u failures peak%u/4096; owned memory only.\n",checks,failures,peak);
 return failures?1:0;
}
