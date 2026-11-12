#pragma once
#include <cstdint>

// H2.48 host local-view refresh: the donor rc1 sequence (OfflineSetup.cpp:9968-10085 PumpHostedCounselorView,
// RefreshHostedCounselorRootHUD 9773, HostedRestoreLocalCounselorView, HostedRestoreLocalGameInput) behind an Ops seam,
// so the order and the failure handling are fixture-proven and the production adapter only supplies pinned natives.
//
// Donor order (host repair + acknowledged-pawn tail), preserved:
//   cameraBefore=GetViewTarget (log only; no state, not transcribed)
//   ignoredBefore=IsMoveInputIgnored -> ClientRestart(NewPawn) -> [InitializeCounselorActiveCamera: SKIPPED, optional,
//   SetActiveCamera is only LIKELY on 3.5] -> if ignoredBefore: ResetIgnoreInputFlags -> ClientSetViewTarget +
//   SetViewTargetWithBlend (donor: either one suffices) -> IsMoveInputIgnored (log only) -> AcknowledgedPawn==pawn ->
//   ClientClearCameraLensEffects -> counselor root HUD (existing current menu skip, else ChangeRootMenu, owner proof)
//   -> SetInputMode_GameOnly.
// Fail-closed differences from the donor (which only logs): any step that failed after the first effect fails the refresh,
// which the takeover transaction answers with its verified rollback (host re-possessed into Jason).
#ifndef CFG_H248_LOCAL_VIEW
#define CFG_H248_LOCAL_VIEW 1
#endif
namespace cfghostview {
enum class Why : unsigned { None, Unpinned, Restart, ResetInput, ViewTarget, Acknowledged, Lens, Hud, GameInput };
// before/after: move-input-ignored around the restart (0 unknown, 1 not ignored, 2 ignored). Diagnostics only.
struct Result { bool ok=false; Why why=Why::None; unsigned steps=0,before=0,after=0; };

// Ops contract:
//   bool Pinned()                      read-only: every native step below is byte/schema pinned in the running image
//   bool MoveIgnored(bool& ignored)    IsMoveInputIgnored (false = unknown, never fatal)
//   bool ClientRestart()               ClientRestart(controller, NewPawn=counselor)
//   bool ResetInput()                  ResetIgnoreInputFlags(controller)
//   bool ViewTarget()                  SetViewTargetWithBlend(controller, counselor, 0 blend) (+ ClientSetViewTarget where the frame is admitted)
//   bool Acknowledged()                read-only: controller.AcknowledgedPawn == counselor
//   bool ClearLens()                   ClientClearCameraLensEffects(controller)
//   bool RootHud()                     counselor root HUD verified (already current, or ChangeRootMenu + owner proof)
//   bool GameInput()                   SetInputMode_GameOnly(WidgetBlueprintLibrary CDO, controller)
template<class Ops> bool Ready(Ops& o){
#if CFG_H248_LOCAL_VIEW
    return o.Pinned();
#else
    (void)o;return false;
#endif
}
// gameInput=false is the donor's first host repair; SetInputMode_GameOnly runs only on the last pass (donor FinalRepairSent).
template<class Ops> Result Refresh(Ops& o,bool gameInput=true){
    Result r;
#if CFG_H248_LOCAL_VIEW
    if(!o.Pinned()){r.why=Why::Unpinned;return r;}       // zero effects
    bool ignored=false;++r.steps;const bool known=o.MoveIgnored(ignored);r.before=known?(ignored?2u:1u):0u;
    ++r.steps;if(!o.ClientRestart()){r.why=Why::Restart;return r;}
    if(known&&ignored){++r.steps;if(!o.ResetInput()){r.why=Why::ResetInput;return r;}}
    ++r.steps;if(!o.ViewTarget()){r.why=Why::ViewTarget;return r;}
    bool after=false;++r.steps;r.after=o.MoveIgnored(after)?(after?2u:1u):0u;
    if(!o.Acknowledged()){r.why=Why::Acknowledged;return r;}
    ++r.steps;if(!o.ClearLens()){r.why=Why::Lens;return r;}
    ++r.steps;if(!o.RootHud()){r.why=Why::Hud;return r;}
    if(gameInput){++r.steps;if(!o.GameInput()){r.why=Why::GameInput;return r;}}
    r.ok=true;
#else
    (void)o;r.why=Why::Unpinned;
#endif
    return r;
}
}
