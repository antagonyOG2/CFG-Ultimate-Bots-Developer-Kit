#pragma once
#include "../RuntimeProbe/SweaterHudBindings.generated.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
namespace cfgsweater {
struct HudEvidence {cfgprobe::lifetime::SlotObservation controller{},hud{},widget{};std::uintptr_t hudType=0,widgetType=0;};
template<class R,std::size_t N>bool HudCode(R& r,std::uintptr_t at,const std::array<unsigned char,N>& expected){std::array<unsigned char,N> actual{};return r.ReadBytes(at,actual.data(),N)&&actual==expected;}
// This is a callback-local strong controller->HUD->widget observation. HUD and
// widget serial0 are legal only while both owning links repeat unchanged. The
// caller independently proves exact offline controller/world/pawn/thread and GC.
template<class R>bool ObserveHud(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
 const cfgprobe::lifetime::SlotObservation& controller,std::uintptr_t dispatcher,HudEvidence& out){
 using namespace huddisk;out={};if(!RuntimeVerified)return false;HudEvidence e;e.controller=controller;std::uintptr_t controllerType=0,second=0,table=0,entry=0,value=0;
 const auto native=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t& type){cfgprobe::lifetime::SlotObservation slot;
  return r.Read(module+a,type)&&r.Read(module+b,second)&&cfgprobe::MatchingClassCache(type,second)&&cfgspawn::ObjectSlot(r,objects,type,slot);};
 cfgprobe::lifetime::SlotObservation fresh;cfgnames::Words words;cfgnames::Text text;cfgnames::Observation<R> names(r,module);
 if(!controller.HasAssignedSerial()||!cfgspawn::ObjectSlot(r,objects,controller.object,fresh)||!(fresh==controller)||
  !native(SCPlayerControllerReflection,SCPlayerControllerConstructor,controllerType)||!cfgspawn::ObjectIs(r,controller.object,controllerType)||
  !native(SCInGameHUDReflection,SCInGameHUDConstructor,e.hudType)||!native(SCHUDWidgetReflection,SCHUDWidgetConstructor,e.widgetType)||
  !HudCode(r,module+GetHUD,GetHUDCode)||!HudCode(r,module+GetSCHUD,GetSCHUDCode)||!HudCode(r,module+Available,AvailableCode)||
  !HudCode(r,module+Event,EventCode)||!HudCode(r,module+NameInitializer,NameInitializerCode)||!HudCode(r,module+BoolSetter,BoolSetterCode)||
  !r.Read(module+EventName,words)||!names.Decode(words,text)||text.View()!=u"SetSweaterAbilityAvailable"||
  !r.Read(controller.object+ControllerHUD,value)||!cfgspawn::ObjectSlot(r,objects,value,e.hud)||!cfgspawn::ObjectIs(r,value,e.hudType)||
  !r.Read(e.hud.object+HUDWidget,value)||!cfgspawn::ObjectSlot(r,objects,value,e.widget)||!cfgspawn::ObjectIs(r,value,e.widgetType)||
  !r.Read(value,table)||!cfgprobe::ObjectPointer(table)||!r.Read(table+ProcessEventSlot,entry)||entry!=dispatcher||
  !r.Read(controller.object+ControllerHUD,value)||value!=e.hud.object||!r.Read(e.hud.object+HUDWidget,value)||value!=e.widget.object||
  !cfgspawn::ObjectSlot(r,objects,controller.object,fresh)||!(fresh==e.controller)||
  !cfgspawn::ObjectSlot(r,objects,e.hud.object,fresh)||!(fresh==e.hud)||
  !cfgspawn::ObjectSlot(r,objects,e.widget.object,fresh)||!(fresh==e.widget))return false;
 out=e;return true;
}
}
