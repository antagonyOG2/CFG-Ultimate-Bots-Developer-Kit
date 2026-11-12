#pragma once
#include <cstdint>
#include <array>
#include <cmath>
#include <type_traits>

// H1 hosted-lobby report: fixed-layout, trivially copyable, published by the primary DLL and
// read (read-only) by the matching reader. Nothing here grants authority; it records what the
// owner-thread observation service saw and what the (flag-gated, arm-file-gated) takeover did.
namespace cfgh1 {
constexpr std::uint32_t Magic=0x31314648u; // 'HF11'
#ifndef CFG_H245_PURSUIT
// H2.45 report-only counters: how often the J2.19 legacy 20m/7s Morph and
// Shift foot-burst lanes WANTED to run but were closed by the donor movement
// lease. No behaviour change; compile out with -DCFG_H245_PURSUIT=0.
#define CFG_H245_PURSUIT 1
#endif
constexpr std::uint32_t Version=32;
constexpr std::uint32_t FlagObserve=1u,FlagJason=2u,FlagFill=4u;
// Highest proof stage reached by the latest sample.
enum class Stage : std::uint32_t {
    NotSampled=0,        // service never ran a sample
    WorldUnreadable=1,   // engine/viewport/world chain not readable
    NoNetDriver=2,       // standalone/offline/menu world: NetDriver null (normal outside a lobby)
    ClientOrForeign=3,   // NetDriver present but NOT a listen host (ServerConnection/world mismatch)
    ContextRefused=4,    // listen-host tuple (Hunt, InProgress, typed ids, authority) not proved
    ContextPassed=5,     // listen-host context proved
    FrameRefused=6,      // killer/roster/ownership census refused
    FramePassed=7        // full roster/killer/ownership frame captured
};
enum class Kill : std::uint32_t { None=0, LocalHost=1, RemoteHuman=2, Unproven=3 };
// Bit i of probeBits = cheap diagnostic sub-proof i passed (set even when the full context refuses).
enum Probe : std::uint32_t {
    ProbeWorld=1u<<0, ProbeNetDriver=1u<<1, ProbeDriverWorld=1u<<2, ProbeListenHost=1u<<3,
    ProbeModePresent=1u<<4, ProbeStatePresent=1u<<5, ProbeModeHunt=1u<<6, ProbeModeAuthority=1u<<7,
    ProbeLocalController=1u<<8, ProbeLocalAuthority=1u<<9
};
struct ContextTrace {
    unsigned stage=0;std::int32_t serial=0;std::uintptr_t object=0,expected=0,actual=0,backlink=0;
    std::uint32_t internal=0,flags=0;std::int32_t locals=0,capacity=0;
    std::array<char,64> mode{},state{},controller{},failedClass{},match{};
};
struct DriverTrace {
    std::uint32_t publications=0,retirements=0,stage=0,ownerChecks=0,ownerRefusals=0,ticks=0,baseTicks=0,services=0,censusWaits=0,startupQueued=0,startupCompleted=0;
    std::uint32_t preparation=0,lastReads=0,readCap=0;
    std::uint64_t world=0,controller=0;
    std::uint32_t gate=0,exclusiveMask=0;
    std::uint64_t heldHint=0;
    std::uint32_t endFreezeCount=0,endDriverRetirements=0;
    std::uint64_t endFreezeAt=0,endDriverRetireAt=0;
    std::uint32_t morphLeaseChecks=0,morphLeaseHosted=0,morphLeaseActive=0;
    std::uint32_t burstLeaseChecks=0,burstLeaseHosted=0,burstLeaseActive=0;
    std::uint32_t huntLaneReached=0,burstWantedLeaseBlocked=0,morphWantedLeaseBlocked=0;
    std::uint32_t adoptedReadOwnerChecks=0,adoptedReadOwnerRefusals=0,adoptedReadOwnerClause=0;
    std::uint32_t adoptedPredicateChecks=0,adoptedPredicateRefusals=0,adoptedPredicateKind=0,adoptedPredicateClause=0;
    std::uint32_t hostedMoveFrameDispatched=0,hostedMoveFrameResult=255,hostedMoveFrameRetentionFinished=0,hostedMoveFrameRetentionRefusals=0;
    std::array<std::uint32_t,12> gates{};
    struct StallSample {
        std::uint64_t tick=0,morphNow=0,target=0,nextDistanceTeleportAt=0,sharedMorphNext=0,
            lastMorphTeleportAt=0,morphReadyAt=0,lastAcceptedMoveAt=0,pathLockUntil=0;
        float distance=0;
        std::uint32_t gate=0,externalMask=0,navStage=0,pathMoveResult=255,distanceMorphResult=0,
            distanceMorphRefusal=0,held=0,phase=0,pathLocked=0,moveRequests=0;
        std::uint8_t lastMoveReturn=255;
    };
    std::uint32_t stallSamples=0;
    StallSample stallFirst{},stallLatest{};
    // H2.46 (report only, append-only): lease-mode burst/Morph and nav-ring outcome counters.
    std::uint32_t h246BurstLeaseStarts=0,h246MorphLeaseQueued=0,h246MorphLeaseTeleported=0,h246MorphLeaseNavBlocked=0;
    // H2.47 (report only, append-only): lease-mode Morph/burst vetoed because Jason is combat-busy (donor busy predicate).
    std::uint32_t h247MorphQueueVetoes=0,h247MorphServiceVetoes=0,h247BurstVetoes=0,h247LastBusyCauses=0;
    // H2.48b (report only, append-only): first failing clause of adoption::Current at owner clause 8, and when it first failed.
    std::uint32_t h248CurrentClause=0,h248CurrentFirstClause=0;std::array<std::uint32_t,13> h248CurrentClauses{};
    std::uint64_t h248CurrentFirstFailAt=0,h248CurrentLastPassAt=0;
    // H2.50 (append-only): adopted controller slot detail at the latest clause-7 failure.
    std::int32_t h250CtlReceiptIndex=-1,h250CtlReceiptSerial=0,h250CtlFreshIndex=-1,h250CtlFreshSerial=0;
    std::uint32_t h250CtlFlags=0,h250CtlReadOk=0;std::uint64_t h250PawnController=0,h250ReceiptController=0;
};
// Pure diagnostic window. Caller supplies only freshly observed values, and
// drives Observe from the owned controller gate at a bounded cadence.
struct StallTracker {
    struct Position {float x=0,y=0,z=0;};
    Position previous{};std::uint64_t since=0,target=0;bool valid=false;
    std::uint64_t progressSince=0;float progressDistance=0;bool trackingProgress=false;
    void Reset() noexcept {*this={};}
    bool Observe(std::uint64_t now,std::uint64_t chosen,float distance,Position position,
        bool targetLive,bool inProgress,DriverTrace::StallSample& sample,bool running=false) noexcept {
        if(!chosen||!targetLive||!inProgress||chosen!=target||!valid||now<since){
            previous=position;target=chosen;since=now;valid=chosen&&targetLive&&inProgress;
            trackingProgress=valid&&running;progressSince=now;progressDistance=distance;return false;
        }
        if(!running||!trackingProgress||now<progressSince||distance<=progressDistance-200.f){
            trackingProgress=running;progressSince=now;progressDistance=distance;
        }
        const float dx=position.x-previous.x,dy=position.y-previous.y,dz=position.z-previous.z;
        const float moved=dx*dx+dy*dy+dz*dz;
        if(!(moved>=0.0f)||moved>900.0f){previous=position;since=now;}
        if(now-since<5000&&(!trackingProgress||now-progressSince<5000))return false;
        sample.tick=now;sample.target=chosen;sample.distance=distance;return true;
    }
};
enum class Lane:unsigned {Traps,CabinExit,TrapPriority,Roof,Car,Knife,Grab,Morph,Stuck,Hard,Convergence,Axes,Sweater,KillPrompt,TommyRadio,Objectives,Emote,Summary,Hiding,Swim,Door,Count};
struct LaneTrace {std::uint32_t entered=0,refused=0,last=0;};
struct BotTrace {
    std::uint64_t pawn=0,controller=0,controllerClass=0,brain=0,tree=0;
    std::uint32_t registered=0,samples=0,running=2,paused=2,startAttempts=0,startReturned=0,last=0,moves=0,first=0,reads=0,source=0;
    float x=0,y=0,z=0;
};
struct OutroEvidence {
    std::uint64_t originalPawn=0,originalState=0,localPawn=0,localState=0;
    std::uint64_t jasonController=0,jasonState=0,killerState=0,aiPawn=0;
    std::uint32_t reads=0,planSwapped=0,restorePending=0,restoreRefused=0,rootOwners=0,leaseFlags=0;
    template<class R> void Read(R& reader,std::uintptr_t address,std::uint64_t& out,unsigned bit){
        std::uintptr_t value=0;
        if(address&&reader.Read(address,value)){out=value;reads|=1u<<bit;}
    }
};
struct Report {
    Report() noexcept { bytes=sizeof(Report); }
    std::uint32_t magic=Magic,version=Version,bytes=0,generation=0;      // generation odd while being written
    std::uint32_t buildFlags=0,armed=0,stage=0,probeBits=0;
    std::uint32_t samples=0,netDriverSamples=0,contextPasses=0,contextRefusals=0;
    std::uint32_t framePasses=0,frameRefusals=0,lastContextReads=0,lastFrameReads=0;
    std::int32_t roster=-1,numBots=-1;
    std::uint32_t botFill=0xFFFFFFFFu,counselorTree=0,explicitPickPresent=0,explicitPickIsKiller=0;
    std::uint32_t eligibility=0,killerKind=0,matchInProgress=0,reserved0=0;
    std::uint32_t takeoverOutcome=0,takeoverStage=0,takeoverReason=0,takeoverRollbackVerified=0;
    std::uint32_t takeoverEffects=0,takeoverAttempts=0,adoptionPublished=0,adoptionRefusal=0;
    std::uint32_t adoptionStage5Clause=0,adoptionStage5Reads=0,adoptionStage5Cap=0,adoptionStage5Reserved=0;
    std::uint64_t adoptionStage5PawnClass=0,adoptionStage5KillerClass=0,adoptionStage5PathClass=0,adoptionStage5Path=0;
    std::int32_t adoptionStage5PawnIndex=-1,adoptionStage5PawnSerial=0,adoptionStage5ControllerIndex=-1,adoptionStage5ControllerSerial=0;
    std::int32_t adoptionStage5PathIndex=-1,adoptionStage5PathSerial=0,adoptionStage5Reserved2=0,adoptionStage5Reserved3=0;
    std::uint32_t driverOwnerChecks=0,driverOwnerRefusals=0,armRefusal=0,stableFrames=0;
    std::uint64_t world=0,mode=0,netDriver=0,killerPawn=0,killerController=0,killerState=0;
    std::uint64_t explicitPick=0,localController=0,localPawn=0,aiController=0;
    std::uint64_t firstHostedTick=0,lastSampleTick=0,takeoverTick=0,reserved1=0;
    std::uint32_t storedOptions=0,storedDifficulty=0,matchOptions=0,matchDifficulty=0;
    std::uint64_t optionsRevision=0,matchOptionsRevision=0;
    std::uint32_t f8Presses=0,fillReady=0,fillEntries=0,fillApplied=0,fillRefused=0,fillRestored=0;
    std::uint32_t fillDeferredQueued=0,fillDeferredAdded=0,fillDeferredRefused=0;
    std::uint32_t fillStockDeferred=0;std::uint64_t fillStockWorld=0;
    std::uint32_t classStage=0,classWarmCalls=0,retryCount=0;std::uint64_t classReflection=0,classConstructor=0,retryAt=0;
    std::uint32_t packetReason=0,packetKind=0,packetIndex=0;std::uint64_t packetAddress=0;
    std::uint32_t classRestorePending=0,classRestoreAttempts=0,classRestored=0,classRestoreRefused=0,classRestoreChanged=0,classRestoreLastRefusal=0;
    std::uint32_t localAdoptionPending=0,localAdoptionAttempts=0;
    OutroEvidence outro{};
    std::array<std::uint32_t,7> carAdmissionRefusals{};
    std::uint32_t carManagerClause=0,carManagerReads=0,carManagerCap=0;
    std::array<std::uint32_t,4> grabEntry{},grabAdmissionRefusals{},grabKillEntry{};
    std::array<std::uint32_t,12> doorLockedRefusals{};
    std::uint32_t hidingDetail=0,stuckRecoveryAttempts=0,stuckRecoveryReturns=0;
    std::uint32_t trapRosterEvents=0,trapRosterAccepted=0,hidingRosterProof=0;
    std::uint32_t presentationPrimaryAttempted=0,presentationPrimaryComplete=0,presentationExtraPending=0,presentationExtraAttempted=0,presentationExtraComplete=0,presentationRefusal=0,presentationCopyRefusal=0,presentationCopyBundleNativeStage=0;
    std::uint32_t presentationNotAttemptedReason=0,presentationNotAttemptedWindows=0;
    std::uint32_t extraRequestIssued=0,extraRequestAccepted=0,storedJasonCount=1;
    std::uint32_t f1InputEdges=0,f1RequestAccepted=0,f1RequestRefused=0,additionalJasonStage=0,additionalJasonRequests=0,additionalJasonBirths=0,additionalJasonFailures=0;
    std::uint32_t f1SelectionRefusal=0,f1SelectionIndex=0,f1SelectionConversionFailures=0,f1SelectionTemplateFailures=0;
    std::uint32_t counselorPreferencePreflight=0,counselorPreferencePreflightReason=0,counselorPreferencePreflightKind=0,counselorPreferencePreflightIndex=0;
    std::uint32_t counselorPreferenceHandoff=0,counselorPreferenceHandoffReason=0,counselorPreferenceHandoffKind=0,counselorPreferenceHandoffIndex=0;
    std::uint32_t rolePreferenceInstalled=0,rolePreferenceAttempts=0,rolePreferenceApplied=0,rolePreferenceRefused=0,rolePreferenceRefusal=0;
    std::uint32_t rolePreferenceRemoteCount=0,rolePreferenceHostBefore=0,rolePreferenceHostAfter=0,rolePreferenceWinnerLocal=0;
    std::uint64_t presentationSkinBefore=0,presentationSkinAfter=0;
    std::array<unsigned char,40> presentationWeaponBefore{},presentationWeaponAfter{};
    std::uint64_t primaryPresentationSkinBefore=0,primaryPresentationSkinAfter=0,extraPresentationSkinBefore=0,extraPresentationSkinAfter=0;
    std::array<unsigned char,40> primaryPresentationWeaponBefore{},primaryPresentationWeaponAfter{},extraPresentationWeaponBefore{},extraPresentationWeaponAfter{};
    std::array<std::uint32_t,6> trapRosterRefusals{};
    std::uint32_t morphAdmissionRefused=0,morphRingStage=0,morphRingBudgetRefused=0;
    std::array<std::uint32_t,6> labelStatusGates{};
    DriverTrace driver{};ContextTrace context{};std::uint32_t fillReason=0,fillContextStage=0;
    std::uint32_t hardDamageScaled=0,hardDamageDifficulty=0,hardDamageSource=0;
    float hardDamageInput=0,hardDamageOutput=0;
    std::uint32_t hostedLabelEntered=0,hostedLabelApplied=0,hostedLabelRefused=0;
    std::array<LaneTrace,static_cast<unsigned>(Lane::Count)> lanes{};
    std::array<BotTrace,7> bots{};
    std::array<BotTrace,7> stockBaseline{};
    std::uint64_t stockBaselineWorld=0;
    std::uint32_t rosterSamples=0,rosterRefused=0,rosterReason=0,offlineRetirements=0;
    std::int32_t fillHumans=-1,fillExpected=-1,fillBotsBefore=-1,fillBotsAfter=-1;
    std::uint32_t stuckRecoveryStage=0,stuckRecoveryReads=0;
    // H2.45 host-as-Jason lottery (append-only). PinResult bits: 1 prologue,2 pref==1 site,4 pref==2 site,8 prior-FString site (15 = all).
    // HostWon: 0 not evaluated, 1 host won, 2 host lost. RefusalCode: 10 pin mismatch, 12 offline, 14 adoption unproven, 1-9 preflight/apply.
    std::uint32_t h245PinResult=0,h245WritesApplied=0,h245HostWon=0,h245RefusalCode=0;
    // H2.48d (append-only): lottery bridge entry evidence. Gate bits at the last call: 1 armed, 2 options ready, 4 already applied.
    std::uint32_t h248BridgeCalls=0,h248BridgeLastGate=0;std::uint64_t h248BridgeInstallAt=0,h248BridgeFirstCallAt=0;
    // H2.50c (append-only): host local-view readiness: tens 1 resolve,2 receiver,3 HUD facts; units = function 1 ClientRestart..7 SetInputMode,8 impl/library,9 world.
    std::uint32_t h250ViewFailure=0;
};
inline bool RecordHardDamage(Report& targetReport,std::uint32_t difficulty,std::uint32_t source,float input,float output)noexcept{
    if(difficulty!=2||source==0||!std::isfinite(input)||input<=0.0f||output!=input*2.0f)return false;
    ++targetReport.hardDamageScaled;targetReport.hardDamageDifficulty=difficulty;targetReport.hardDamageSource=source;
    targetReport.hardDamageInput=input;targetReport.hardDamageOutput=output;return true;
}
static_assert(std::is_trivially_copyable<Report>::value,"Fixed transfer protocol");
static_assert(sizeof(Report)<4096,"Bounded transfer only");
inline const char* StageName(std::uint32_t s) noexcept {
    switch(static_cast<Stage>(s)){
    case Stage::NotSampled:return "not-sampled";case Stage::WorldUnreadable:return "world-unreadable";
    case Stage::NoNetDriver:return "no-netdriver";case Stage::ClientOrForeign:return "client-or-foreign";
    case Stage::ContextRefused:return "context-refused";case Stage::ContextPassed:return "context-passed";
    case Stage::FrameRefused:return "frame-refused";case Stage::FramePassed:return "frame-passed";}
    return "unknown";
}
inline const char* KillName(std::uint32_t k) noexcept {
    switch(static_cast<Kill>(k)){case Kill::None:return "none";case Kill::LocalHost:return "local-host";
    case Kill::RemoteHuman:return "remote-human";case Kill::Unproven:return "unproven";}
    return "unknown";
}
// Exactly the report a reader may accept (frozen layout, generation stable, no torn read).
inline bool Accept(const Report& a,const Report& b) noexcept {
    return a.magic==Magic&&a.version==Version&&a.bytes==sizeof(Report)&&b.magic==Magic&&b.bytes==sizeof(Report)&&
        !(a.generation&1u)&&a.generation==b.generation;
}
}
