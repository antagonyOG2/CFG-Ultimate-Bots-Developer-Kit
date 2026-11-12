#define main SweaterHudSetupMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "SweaterHudObservation.hpp"
struct HudReader{OwnedByteReader& r;unsigned reads=0;
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=1536)return false;++reads;return r.Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t n){if(reads>=1536)return false;++reads;auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<n;++i)if(!r.Read(at+i,p[i]))return false;return true;}
};
struct HudFixture:MenuFixture{
 static constexpr std::uintptr_t Registry=0x900000,Controller=0x910000,Hud=0x920000,HudWidget=0x930000,ControllerType=0x940000,HudType=0x950000,HudWidgetType=0x960000,Vtable=0x970000,Dispatcher=0x980000;
 HudReader reader{r};cfgprobe::lifetime::ArrayHeader registry{Registry,6};cfgprobe::lifetime::SlotObservation controller{Controller,0,1};
 void Slot(std::uintptr_t object,int i,int serial){r.Set(object+12,i);r.Set(Registry+24*i,object);r.Set(Registry+24*i+8,std::uint32_t{});r.Set(Registry+24*i+16,serial);}
 HudFixture(){using namespace cfgsweater::huddisk;
  r.Set(Base+SCPlayerControllerReflection,ControllerType);r.Set(Base+SCPlayerControllerConstructor,ControllerType);r.Set(Base+SCInGameHUDReflection,HudType);r.Set(Base+SCInGameHUDConstructor,HudType);r.Set(Base+SCHUDWidgetReflection,HudWidgetType);r.Set(Base+SCHUDWidgetConstructor,HudWidgetType);
  Slot(Controller,0,1);Slot(Hud,1,0);Slot(HudWidget,2,0);Slot(ControllerType,3,0);Slot(HudType,4,0);Slot(HudWidgetType,5,0);
  r.Set(Controller+16,ControllerType);r.Set(Hud+16,HudType);r.Set(HudWidget+16,HudWidgetType);
  for(auto type:{ControllerType,HudType,HudWidgetType})r.Set(type+cfgprobe::ClassSuper,std::uintptr_t{});
  r.Set(Controller+ControllerHUD,Hud);r.Set(Hud+cfgsweater::huddisk::HUDWidget,HudWidget);r.Set(HudWidget,Vtable);r.Set(Vtable+ProcessEventSlot,Dispatcher);
  r.Set(Base+GetHUD,GetHUDCode);r.Set(Base+GetSCHUD,GetSCHUDCode);r.Set(Base+Available,AvailableCode);r.Set(Base+Event,EventCode);r.Set(Base+NameInitializer,NameInitializerCode);r.Set(Base+BoolSetter,BoolSetterCode);
  Name(0x990000,"SetSweaterAbilityAvailable");cfgnames::Words words;r.Read(0x990000+24,words);r.Set(Base+EventName,words);
 }
 bool Observe(cfgsweater::HudEvidence& out){return cfgsweater::ObserveHud(reader,registry,Base,controller,Dispatcher,out);}
};
int main(){unsigned checks=0,failures=0,peak=0;auto check=[&](bool ok){++checks;if(!ok)++failures;};
 for(unsigned fault=0;fault<13;++fault){HudFixture f;cfgsweater::HudEvidence out;using namespace cfgsweater::huddisk;
  switch(fault){case 1:f.r.Set(f.Controller+ControllerHUD,f.HudWidget);break;case 2:f.r.Set(f.Hud+HUDWidget,f.Controller);break;
  case 3:f.r.Set(f.Registry+8,std::uint32_t{0x20000000});break;case 4:f.r.Set(f.Vtable+ProcessEventSlot,std::uintptr_t{});break;
  case 5:f.r.Set(f.Base+Available,std::uint8_t{});break;case 6:f.r.Set(f.Base+Event,std::uint8_t{});break;
  case 7:f.r.Set(f.Base+SCPlayerControllerConstructor,f.HudType);break;case 8:f.r.Set(f.Registry+16,0);break;
  case 9:f.Name(0x990000,"OtherEvent");{cfgnames::Words words;f.r.Read(0x990000+24,words);f.r.Set(f.Base+EventName,words);}break;
  case 10:f.r.changeAt=f.Hud+HUDWidget;f.r.changeSlot=f.Controller+ControllerHUD;f.r.newPointer=f.HudWidget;break;
  case 11:f.r.changeAt=f.Vtable+ProcessEventSlot;f.r.changeSlot=f.Registry+24+16;f.r.newPointer=1;break;
  case 12:f.r.Set(f.HudWidget+16,f.ControllerType);break;}
  f.reader.reads=512;const bool okay=f.Observe(out);check(okay==(fault==0));
  if(!fault)check(out.hud.object==f.Hud&&out.widget.object==f.HudWidget);
  else check(out.hud.object==0&&out.widget.object==0);
  check(f.reader.reads>512&&f.reader.reads<1536);peak=(std::max)(peak,f.reader.reads);
 }
 std::printf("Sweater HUD checks=%u failures=%u peak=%u/1536 including512 outer; zero native calls\n",checks,failures,peak);return failures?1:0;
}
