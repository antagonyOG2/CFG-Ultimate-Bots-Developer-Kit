// Included inside the observer's anonymous namespace, before BundleAccess. Plain data + small SEH leaves.
#ifdef CFG_H1_JASON
struct HostedTxState {
    cfghostedtx::Snapshot snap{};
    cfghostedtx::Birth birth{};
    std::uintptr_t gameState=0,killerAIClass=0,killerBase=0,playerStateClass=0,counselorClass=0;
    std::uintptr_t restartFn=0,possessFn=0,humanPossessFn=0,destroyFn=0;
    cfgmenu::promotion::Packet savedActive{},counselorPacket{},expectedActive{};
    cfghostedpacket::Source counselorSource{};cfghostedpacket::Diagnostic packetDiagnostic{};
    bool haveSaved=false,haveCounselor=false,counselorConsumed=false;
    unsigned counselorPreferencePreflight=0,counselorPreferencePreflightReason=0,counselorPreferencePreflightKind=0,counselorPreferencePreflightIndex=0;
    unsigned counselorPreferenceHandoff=0,counselorPreferenceHandoffReason=0,counselorPreferenceHandoffKind=0,counselorPreferenceHandoffIndex=0;
    std::uintptr_t aiPath=0,aiPathAtSpawn=0,aiPathAfterPossess=0;
    unsigned refusal=0,crowd=0,adoptStage=0,classStage=0,classWarmCalls=0;
    std::uintptr_t classReflection=0,classConstructor=0;
    cfgjason::NewActorEvidence evidence{};
    cfghosted::adoption::Receipt receipt{};
    void Reset() noexcept { *this=HostedTxState{}; }
} hostedTx;
struct HostedClassRestoreState {
    bool pending=false,due=false;unsigned attempts=0,restored=0,refused=0,changed=0,lastRefusal=0;
    std::uint64_t next=0;
    std::uintptr_t world=0,mode=0,state=0,gameState=0;
    cfgprobe::lifetime::SlotObservation controller{},stateSlot{};
    cfgmenu::promotion::Packet original{},expected{};
} hostedClassRestore;
// H2.55: the Jason class the lottery gave the host, kept for the AI after H2.54 restores the host's counselor.
#pragma warning(push)
#pragma warning(disable:4324) // the packet's native ABI alignment pads this struct; intended
struct HostedPrebornState { bool saved=false,pending=false;cfgmenu::promotion::Packet killer{};std::uint64_t at=0;std::uintptr_t world=0,mode=0,state=0; } hostedPreborn;
#pragma warning(pop)
// H2.53: donor g_HostedViewHandoff timing (OfflineSetup.cpp:10376-10381) for the local host only. One shot per repair, never rolls back.
// H2.58: native SCKillerPlayerStart selection for the hosted F1 pawn spawn (read-only, multi-callback).
cfgkillerstart::Observation hostedF1Start;unsigned hostedF1StartTurns=0;bool hostedF1StartWait=false;
struct HostedViewRepairState {
    bool pending=false;unsigned done=0,code=0;
    std::uint64_t earlyAt=0,finalAt=0,deadline=0;
    std::uintptr_t world=0,counselor=0;
} hostedViewRepair;
#if CFG_H22_HOTKEYS
struct HostedCounselorState {
    unsigned stage=0,failedStage=0;bool travelReturned=false,travelReady=false;cfghostedpacket::Source counselorSource{};cfghostedpacket::Diagnostic packetDiagnostic{};
    cfghosted::listen35::Context host{};cfghosted::native::Population before{};
    cfghosted::native::NewControllerState born{};
    std::uintptr_t sourceState=0,ai=0,pawn=0,travel=0;cfgprobe::lifetime::SlotObservation pawnSlot{};
    bool entered=false,fault=false,accounted=false,copied=false,consumed=false;
} hostedCounselor;
struct HostedCounselorBrainTicket {
    bool pending=false;unsigned retries=0;std::uint64_t due=0,world=0;
    std::uintptr_t localController=0,localPawn=0;
    cfgprobe::lifetime::SlotObservation localControllerSlot{},localPawnSlot{};
    cfghosted::listen35::Context host{};cfghosted::native::Population before{};
    cfghosted::native::NewControllerState born{};std::uintptr_t pawn=0;
    cfgprobe::lifetime::SlotObservation pawnSlot{};
};
std::array<HostedCounselorBrainTicket,8> hostedCounselorBrainQueue{};
cfgmenu::promotion::Packet hostedCounselorPacket{}; // native ABI alignment, separate opaque payload owner
#endif
// Leaf SEH wrappers: no nontrivial automatic objects (MSVC C2712). No decision logic here.
inline bool HostedLeafVirtualDestroy(std::uintptr_t fn,std::uintptr_t actor)noexcept{
    __try{reinterpret_cast<void(__fastcall*)(void*)>(fn)(reinterpret_cast<void*>(actor));return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool HostedLeafCrowdSetter(std::uintptr_t fn,std::uintptr_t path,std::uint8_t state)noexcept{
    __try{reinterpret_cast<void(__fastcall*)(void*,std::uint8_t)>(fn)(reinterpret_cast<void*>(path),state);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// H2.48 host local view: UObject::ProcessEvent leaf (donor SafeProcessEventCall) and the pinned UserWidget owner getter.
inline bool HostedLeafProcessEvent(std::uintptr_t fn,std::uintptr_t object,std::uintptr_t function,void* params)noexcept{
    __try{reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(fn)(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),params);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool HostedLeafOwnerGetter(std::uintptr_t fn,std::uintptr_t widget,std::uintptr_t& owner)noexcept{
    __try{owner=reinterpret_cast<std::uintptr_t(__fastcall*)(void*)>(fn)(reinterpret_cast<void*>(widget));return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
#endif
#ifdef CFG_H1_JASON
#include "../Core/HostedEndFreezePolicy.hpp"
// Receipt of the CURRENT adoption (valid only while the hosted driver owns the adopted Jason).
bool hostedAdoptionActive=false;
cfghosted::adoption::Receipt hostedAdoptionReceipt{};
cfgh1::DriverTrace hostedDriverTrace{};
bool hostedDriverStartupOwed=false;
// Owned by the current hosted adoption, independent of the mod's kill policy.
bool hostedNativeDeathMotion=false;
std::uint64_t hostedNativeDeathMotionCheckAt=0;
bool hostedRoutineStarted=false;std::uint64_t hostedRoutineRosterAt=0,hostedBrainAt=0,hostedSpectatorHintRetryAt=0;
bool hostedEndFrozen=false;
std::array<cfgh1::LaneTrace,static_cast<unsigned>(cfgh1::Lane::Count)> hostedLanes{};
std::array<cfgh1::BotTrace,7> hostedBots{};
unsigned hostedBotCursor=0,hostedRosterSamples=0,hostedRosterRefused=0,hostedRosterReason=0,hostedOfflineRetirements=0;
#ifndef CFG_H246_FULL_LOBBY
#define CFG_H246_FULL_LOBBY 1
#endif
// H246 evidence only: first failing InitializeHostedRoutine clause (0=pass; see HostedRoutineRuntime.hpp). No behaviour.
unsigned hostedInitClause=0;
bool InitializeHostedRoutine(std::uint64_t now)noexcept;
void RetireHostedRoutine()noexcept;
inline bool HostedAdoptionOwnsDriver()noexcept{
    return cfghostedend::DriverActive(hostedEndFrozen,hostedAdoptionActive)&&hostedAdoptionReceipt.Complete()&&
        hostedAdoptionReceipt.world==plan.world&&hostedAdoptionReceipt.mode==plan.mode&&
        cfgprimary::CurrentHostedBinding().Matches(plan);
}
inline bool HostedRoutineActive()noexcept{return hostedRoutineStarted&&HostedAdoptionOwnsDriver();}
// H2.47: driver ownership ends at end-freeze but the adopted hosted world still shows the summary.
inline bool HostedSummaryWindow()noexcept{
    return CFG_H247_SUMMARY_BOTS&&hostedRoutineStarted&&hostedEndFrozen&&hostedAdoptionActive&&hostedAdoptionReceipt.Complete()&&
        hostedAdoptionReceipt.world==plan.world&&hostedAdoptionReceipt.mode==plan.mode&&cfgprimary::CurrentHostedBinding().Matches(plan);
}
#else
inline constexpr bool HostedRoutineActive()noexcept{return false;}
inline constexpr bool HostedSummaryWindow()noexcept{return false;}
inline constexpr bool HostedAdoptionOwnsDriver()noexcept{return false;}
inline const cfghosted::adoption::Receipt hostedAdoptionReceipt{};
#endif

inline void HostedLaneEnter(cfgh1::Lane lane)noexcept{
#ifdef CFG_H1_JASON
    if(HostedRoutineActive())++hostedLanes[static_cast<unsigned>(lane)].entered;
#else
    (void)lane;
#endif
}
inline bool HostedLaneRefuse(cfgh1::Lane lane,unsigned reason)noexcept{
#ifdef CFG_H1_JASON
    if(HostedRoutineActive()){auto& d=hostedLanes[static_cast<unsigned>(lane)];++d.refused;d.last=reason;}
#else
    (void)lane;(void)reason;
#endif
    return false;
}
