// Included after bundle globals, before BundleAccess.
#include "CounselorPracticeObservation.hpp"
struct PracticeAudio {
 cfgprobe::lifetime::SlotObservation component{},killer{};cfgpractice::Override<float> value{};
};
struct PracticeWidget {
 cfgprobe::lifetime::SlotObservation widget{};unsigned node=UINT32_MAX;
 cfgpractice::Override<std::uint8_t> value{};bool minimap=false;
};
struct PracticeState {
 cfgprobe::lifetime::SlotObservation world{};
 std::array<PracticeAudio,2> audio{};std::array<PracticeWidget,8> widgets{};
 std::array<cfgpractice::GrabEvent,2> grabs{};
 cfgpracticeobs::WidgetGraph graph{};
 unsigned audioCursor=0,widgetCursor=0,targets=0;std::uint64_t next=0;
 std::uintptr_t healthOwner=0;bool healthAttempted=false,healthTargetAdded=false;
} bundlePractice;
KnifeHookLease<cfgpractice::disk::VoiceNativeCode.size()> bundlePracticeVoiceHook;
unsigned bundlePracticeVoiceInstallFailure=0;
bool J215PracticeScope(Reader& r,bool additional,cfgsoundblip::Scope& scope){
 scope={};if(!bundleStarted||!bundleSchedule.active||!plan.module||GetCurrentThreadId()!=plan.threadId)return false;
 scope.module=plan.module;scope.viewport=plan.viewport;scope.world=plan.world;scope.mode=plan.mode;
 scope.killer=additional?bundleAdditional.actor.pawn:bundleJason.pawn;
 scope.controller=additional?bundleAdditional.actor.killerController:bundleJason.killerController;
 scope.humanController=plan.controller;
 cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation human;
 if(!scope.killer.HasAssignedSerial()||!scope.controller.HasAssignedSerial()||
    !cfgprimary::LookupVetoClearForObservation(r,plan.module)||
    !r.Read(plan.controller+cfgprobe::ControllerPawn,scope.humanPawn)||
    !cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)||
    !cfgpracticeobs::Clear(r,array,scope.humanPawn,human)||!human.HasAssignedSerial()||
    !bundleCounselorClass||!cfgspawn::ObjectIs(r,scope.humanPawn,bundleCounselorClass)||
    !r.Read(scope.killer.object+cfgprobe::ObjectClass,scope.killerClass)||
    !r.Read(scope.controller.object+cfgprobe::ObjectClass,scope.controllerClass)||
    !cfgspawn::ObjectIs(r,scope.killer.object,spawnAssets.killerBase)||
    !cfgspawn::ObjectIs(r,scope.controller.object,spawnAssets.resources.classes[cfgspawn::KillerAI]))return false;
 scope.active=scope.offlineCounselor=true;
 return cfgsoundblip::Suppress(r,scope,scope.killer.object,true);
}
bool J215PracticeHud(Reader& r,cfgsweater::HudEvidence& hud,cfgprobe::lifetime::ArrayHeader& array){
 hud={};cfgsoundblip::Scope scope;std::uintptr_t object=0,table=0,entry=0;cfgprobe::lifetime::SlotObservation controller;
 if(!J215PracticeScope(r,false,scope)||!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)||
    !cfgpracticeobs::Clear(r,array,plan.controller,controller)||!(controller==plan.objects[2])||
    !r.Read(plan.controller+cfgsweater::huddisk::ControllerHUD,object)||!cfgpracticeobs::Clear(r,array,object,hud.hud)||
    !cfgpracticeobs::Typed(r,array,plan.module,object,cfgpractice::disk::SCInGameHUDReflection)||
    !r.Read(object+cfgpractice::disk::HUDWidgetInstance,object)||!cfgpracticeobs::Clear(r,array,object,hud.widget)||
    !cfgpracticeobs::Typed(r,array,plan.module,object,cfgpractice::disk::SCHUDWidgetReflection)||
    !r.Read(object,table)||!r.Read(table+cfgsweater::huddisk::ProcessEventSlot,entry)||entry!=plan.module+cfgspawn::GenericDispatcherRva||
    !cfgobserver::Same(r,plan.controller+cfgsweater::huddisk::ControllerHUD,hud.hud.object)||
    !cfgobserver::Same(r,hud.hud.object+cfgpractice::disk::HUDWidgetInstance,hud.widget.object))return false;
 hud.controller=controller;return true;
}
