#pragma once
#include "../Core/NativeSpawnParameters.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/SpawnRuntimeBindings.generated.hpp"
namespace cfgspawn {
// Preliminary dispatch guard: ReceiveTick is not known until the exact event
// has been validated. Do not use SpawnServiceWindow for this first phase.
constexpr bool SpawnServiceCandidate(bool ownerThread,bool nested,bool dispatchForwarded,
    bool pending) noexcept {
    return ownerThread && !nested && dispatchForwarded && pending;
}
constexpr bool SpawnServiceWindow(bool ownerThread,bool nested,bool dispatchForwarded,
    bool receiveTick,bool pending) noexcept {
    return SpawnServiceCandidate(ownerThread,nested,dispatchForwarded,pending) && receiveTick;
}
constexpr bool ActorReceiveTickIdentityMatches(std::uintptr_t function,std::uintptr_t cachedFunction,
    std::uint16_t parameterBytes) noexcept {
    return function!=0 && function==cachedFunction &&
        parameterBytes==cfgcompat::cfg35::functions::ActorReceiveTick.parameterBytes;
}
#if defined(CFG_PRIMARY_JASON)
#ifdef CFG_RETAINED_PRIMARY_JASON
constexpr std::uint32_t SpawnMagic=0x43464750,SpawnVersion=11,MaximumNativeCalls=31;
constexpr std::uint32_t MaximumServiceReads=16384;
#else
constexpr std::uint32_t SpawnMagic=0x43464750,SpawnVersion=8,MaximumNativeCalls=31;
constexpr std::uint32_t MaximumServiceReads=8192;
#endif
constexpr unsigned MaximumPrimaryPlacements=4;
#elif defined(CFG_JASON_APPROACH)
constexpr std::uint32_t SpawnMagic=0x4346474A,SpawnVersion=1;
constexpr std::uint32_t MaximumNativeCalls=9;
constexpr std::uint32_t MaximumServiceReads=4096;
#else
constexpr std::uint32_t SpawnMagic=0x43464753,SpawnVersion=2;
constexpr unsigned MaximumCounselorServiceReads=2048;
#endif
struct Identity {
    std::uintptr_t world=0,pawn=0;
    std::int32_t worldSerial=0,pawnSerial=0;
    bool Valid() const noexcept { return cfgprobe::ObjectPointer(world) && cfgprobe::ObjectPointer(pawn) && worldSerial>0 && pawnSerial>0; }
    bool operator==(const Identity& other) const noexcept {
        return world==other.world && pawn==other.pawn && worldSerial==other.worldSerial && pawnSerial==other.pawnSerial;
    }
};
struct SpawnRequest {
    std::uint32_t magic=SpawnMagic,version=SpawnVersion,bytes=sizeof(SpawnRequest),reserved=0;
    Identity identity{};
};
enum class SpawnState : std::uint32_t { Idle, Ready, RefusedPreparation, Queued, RefusedContext, RefusedResources,
    LocationFailed, PacketRejected, PreSpawnChanged, SpawnReturnedNull, ResultUnverified, LinkedNativeCounselor, NativeException
#ifdef CFG_JASON_APPROACH
    , UnpossessFailed, UnpossessUnverified, ClassWriteRefused, DefaultControllerFailed, KillerControllerUnverified, MoveFailed, MoveSubmitted,
      InstanceWriteIndeterminate, MoveResultUnverified
#ifdef CFG_PRIMARY_JASON
      , ClassResolutionFailed, NavigationReadinessFailed, StartMarkerFailed
#endif
#endif
};
struct SpawnReport {
    std::uint32_t magic=SpawnMagic,version=SpawnVersion,bytes=sizeof(SpawnReport),generation=0;
    std::uint32_t state=0,preparationReads=0,serviceReads=0,nativeCalls=0,error=0,attempts=0;
    std::uint32_t preparationStep=0,assetIndex=0;
    std::uint32_t refusalPhase=0,refusalCheck=0,refusalIndex=0,refusalSlotMask=0;
    std::int32_t refusalCapturedSerial=0,refusalFreshSerial=0;
    std::uint64_t worldAssetResets=0;
    std::array<std::int32_t,7> assetSerials{};
    Identity identity{};
    std::uintptr_t sourcePawn=0,pawnClass=0,tree=0,result=0,resultController=0;
    Vec3 location{};
#if !defined(CFG_JASON_APPROACH) || defined(CFG_RETAINED_PRIMARY_JASON)
    std::uint32_t retentionCalls=0,retentionReads=0,retentionStage=0,retentionResult=0,retentionReferences=0;
    std::uint32_t retentionChecks=0,retentionPreserved=0,retentionCallbacks=0,retentionCompleted=0;
    std::uint32_t retentionSubmissions=0,retentionCleared=0,retentionDestroyed=0,retentionFailure=0;
    std::uintptr_t retentionOwner=0;
#endif
#ifdef CFG_RETAINED_PRIMARY_JASON
    std::uint32_t retentionOwners=0,retentionPoisoned=0;
    std::uint32_t navigationSamples=0,navigationMoves=0,navigationStopped=0,navigationReads=0;
    std::uint32_t recoveryStage=0,recoveryFailure=0,recoveryProjections=0,recoveryTeleports=0;
    std::array<std::uintptr_t,32> retentionOwnerAddresses{},retentionOwnerFirsts{};
    std::array<unsigned,32> retentionOwnerReferences{};
#endif
#ifdef CFG_JASON_APPROACH
    std::uintptr_t initialController=0,pathComponent=0;
    std::uint32_t instanceWrites=0,moveResult=255;
#ifdef CFG_PRIMARY_JASON
    std::uint32_t catalogueIndex=0xFFFFFFFF,classConversions=0;
    std::uint32_t primaryKillerObserved=0,budgetRefused=0;
    std::uint32_t crowdKind=0,crowdFallbackCalls=0,crowdStateBefore=255,crowdStateAfter=255;
    std::int32_t resultPawnSerial=0,resultControllerSerial=0;
    std::uint32_t objectTableChanged=0;
    std::int32_t objectCountAtInstall=0,objectCountAtService=0;
    std::uintptr_t primaryCurrentKiller=0;
    std::uint32_t nativeLifecycle=0,nativeCreationStage=0,exceptionCode=0,exceptionStage=0;
    std::uintptr_t exceptionAddress=0;
    std::uint32_t nullSpawns=0,retryPending=0,placement=0;
    std::uint32_t startMarkerState=0,startMarkerSteps=0,startMarkerMaxReads=0;
    std::uint32_t startMarkerFailure=0,startClassWarmState=0,startClassWarmCalls=0;
    std::uint32_t startMarkerRestarts=0,startMarkerAppendUpdates=0,startMarkerChangedSource=0,startMarkerChangedLevel=0;
    std::uintptr_t startClassCache=0,startConstructorCache=0;
    std::uintptr_t startMarker=0;
    Vec3 rotation{};
#endif
#endif
};
inline bool ValidSpawnReport(const SpawnReport& r) noexcept {
    return r.magic==SpawnMagic && r.version==SpawnVersion && r.bytes==sizeof(SpawnReport) && !(r.generation&1) &&
        r.refusalPhase<=5&&r.refusalCheck<=56&&r.refusalIndex<=64&&r.refusalSlotMask<=15&&
#ifdef CFG_JASON_APPROACH
#ifdef CFG_PRIMARY_JASON
        r.state<=static_cast<unsigned>(SpawnState::StartMarkerFailed) && r.classConversions<=18 &&
        (r.catalogueIndex<64 || r.catalogueIndex==0xFFFFFFFF) &&
        r.primaryKillerObserved<=1 && r.budgetRefused<=1 &&
        r.nativeLifecycle<=1 && r.nativeCreationStage<=6 && r.exceptionStage<=static_cast<unsigned>(SpawnState::StartMarkerFailed) &&
        r.crowdKind<=3 && r.crowdFallbackCalls<=1 && r.resultPawnSerial>=0 && r.resultControllerSerial>=0 &&
        r.objectTableChanged<=1 && r.objectCountAtInstall>=0 && r.objectCountAtService>=0 &&
        r.objectCountAtInstall<=cfgprobe::lifetime::MaximumDiagnosticObjects && r.objectCountAtService<=cfgprobe::lifetime::MaximumDiagnosticObjects &&
        r.attempts<=MaximumPrimaryPlacements&&r.nullSpawns<=r.attempts&&r.retryPending<=1&&r.placement<MaximumPrimaryPlacements&&
        r.startMarkerState<=4&&r.startMarkerFailure<=13&&r.startClassWarmState<=3&&r.startClassWarmCalls<=2&&
        r.startMarkerRestarts<=16&&r.startMarkerChangedSource<=2&&r.startMarkerChangedLevel<32&&
        r.startMarkerMaxReads<=MaximumServiceReads&&cfgspawn::Finite(r.rotation)&&
#else
        r.state<=static_cast<unsigned>(SpawnState::MoveResultUnverified) && r.preparationReads<=4096 && r.serviceReads<=4096 &&
#endif
        r.preparationReads<=4096 && r.serviceReads<=MaximumServiceReads &&
#ifndef CFG_PRIMARY_JASON
        r.attempts<=1 &&
#endif
        r.nativeCalls<=MaximumNativeCalls && r.instanceWrites<=1
#ifdef CFG_RETAINED_PRIMARY_JASON
        &&r.retentionOwners<=32&&r.retentionPoisoned<=1&&r.retentionCalls<=32&&r.retentionReads<=32768&&
        r.retentionStage<=8&&r.retentionResult<=3&&r.retentionReferences<=1024&&r.retentionPreserved<=1
#endif
        ;
#else
        r.state<=static_cast<unsigned>(SpawnState::NativeException) && r.preparationReads<=4096 && r.serviceReads<=MaximumCounselorServiceReads &&
        r.attempts<=1 && r.nativeCalls<=2 && r.retentionCalls<=1 && r.retentionReads<=1024 &&
        r.retentionStage<=8 && r.retentionResult<=3 && r.retentionReferences<=32 &&
        r.retentionChecks<=8 && r.retentionPreserved<=1 && r.retentionFailure<=4;
#endif
}
static_assert(std::is_trivially_copyable<SpawnReport>::value && sizeof(SpawnReport)<2048,"Fixed spawn observation");
} // namespace cfgspawn
