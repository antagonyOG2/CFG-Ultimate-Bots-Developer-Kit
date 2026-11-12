// STAGED observation prototype. Not the gameplay mod, not the inert bootstrap.
// Preparation outside DllMain only. This DLL has NO installer or patch writer.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <vector>
#include <cstring>
#include <cwchar>
#include <intrin.h>
#ifndef CFG_H216_HOSTED_SPECTATOR
#define CFG_H216_HOSTED_SPECTATOR 0
#endif
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
#include "../RuntimeProbe/HostedSpectatorRuntime.hpp"
#endif
#if defined(CFG_GC_REGISTRY_PROBE) || defined(CFG_RETAINED_COUNSELOR_SPAWN) || defined(CFG_RETAINED_PRIMARY_JASON)
#include "GcRegistryProbe.hpp"
#include "ReceiveTickObservation.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#ifdef CFG_SYNCHRONIZED_ROOT_PROBE
#include "ResidentRootRegistration.hpp"
#include "RootPublicationProbe.hpp"
#endif
#ifdef CFG_RETAINED_PRIMARY_JASON
#include "PermanentRootPool.hpp"
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
#include "RetainedNativeTransaction.hpp"
#include "CallbackRootLease.hpp"
#include "NativePublicationWindow.hpp"
#include "../Core/NativeStartupSessionReuse.hpp"
#endif
#include "BundleFrameInputObservation.hpp"
#include "CabinBoundsRuntime.hpp"
#include "../Core/CounselorPursuitPolicy.hpp"
#include "../RuntimeProbe/VehicleBindings.generated.hpp"
#include "../RuntimeProbe/ActorTeleportBindings.generated.hpp"
#endif
#endif
#ifdef CFG_GC_REGISTRY_PROBE
#if defined(CFG_GAMEPLAY_BUNDLE) || defined(CFG_FRONTEND_BUNDLE) || defined(CFG_SPAWN_ADAPTER) || defined(CFG_CALLBACK_BOUNDARY_TRACE) || defined(CFG_STATS_ADAPTER) || defined(CFG_EMOTE_TRACE)
#error GC registry probe must be built without other adapters.
#endif
#endif
#ifdef CFG_CALLBACK_BOUNDARY_TRACE
#include "CallbackBoundaryTrace.hpp"
#if defined(CFG_GAMEPLAY_BUNDLE) || defined(CFG_FRONTEND_BUNDLE) || defined(CFG_SPAWN_ADAPTER) || defined(CFG_STATS_ADAPTER)
#error Callback boundary observation must be built without feature adapters.
#endif
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
#include "../RuntimeProbe/Compatibility/CfgJasonHunting35.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgJasonMovement35.generated.hpp"
#include "../Core/DonorJasonNavContinuation.hpp"
#include "../Core/DonorOwedAttackRelease.hpp"
#include "../Core/DonorNativeMorphCompletion.hpp"
#include "../Core/DonorJasonPursuit.hpp"
#include "../Core/DonorJasonStartup.hpp"
#include "../Core/DonorJasonControllerTick.hpp"
#include "../Core/DonorJasonMovement.hpp"
#include "../Core/DonorDriverLiveness.hpp"
#include "../Core/DonorJasonRoster.hpp"
#include "DonorCounselorContextObservation.hpp"
#include "DonorCounselorObservation.hpp"
#include <atomic>
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include "../Core/ForegroundKeyEdge.hpp"
#include "../Core/CounselorInputMailbox.hpp"
#include "../Core/SpectatorInputIntent.hpp"
#include "../RuntimeProbe/Compatibility/CfgSummary35.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgAdditional35.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgSweaterHud35.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgSweaterPickup35.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgTrapPriority35.generated.hpp"
#include "../Core/DonorTrapPriorityBridge.hpp"
#ifdef CFG35_URINATION_NATIVE_PORT
#include "UrinationRetainedLoad.hpp"
#include "UrinationNativeHooks.hpp"
#include "UrinationAssetObservation.hpp"
#include "UrinationLoaderFrameObservation.hpp"
#include "UrinationWavePublication.hpp"
#include "UrinationWaveDiscovery.hpp"
#include "../Core/UrinationRuntimePhases.hpp"
#include "../Core/UrinationPlaybackPolicy.hpp"
#endif
#endif
#include "OfflineContextObservation.hpp"
#if defined(CFG_GC_REGISTRY_PROBE) || defined(CFG_RETAINED_COUNSELOR_SPAWN) || defined(CFG_RETAINED_PRIMARY_JASON)
#include "PrimaryContextObservation.hpp"
#endif
#include "../RuntimeProbe/OwnedGenericDispatcher.hpp"
#include "TrampolinePreparation.hpp"
#ifdef CFG_FRONTEND_BUNDLE
#include "NativeMenuLaunchRuntime.hpp"
#include "../RuntimeProbe/JasonPickerCatalogueObservation.hpp"
#include "NativeMenuDispatch.hpp"
#include "FrontendRetentionReport.hpp"
#include "NativeChallengePageResolver.hpp"
#include "NativeChallengeMenuRoute.hpp"
#include "NativeChallengeMenuTransition.hpp"
#include "../Core/J22TargetPolicy.hpp"
#include "NativeChallengeReturnObservation.hpp"
#include "NativeChallengeSettingsService.hpp"
#include "NativeChallengeOptions.hpp"
#include "NativeChallengeIntro.hpp"
#include "NativeChallengeRestart.hpp"
#include "../Core/ChallengeEscapeStartPolicy.hpp"
#include "NativeChallengeEscapeRuntime.hpp"
#include "NativeChallengeEscapeHooks.hpp"
#include "NativeChallengeEscapeSeams.hpp"
#include "NativeChallengeBoundaryRuntime.hpp"
#include "NativeChallengeEdgeWallsRuntime.hpp"
#include "NativeChallengeBoundaryWriter.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
#include "NativeGraphEntryBoundary.hpp"
#include "FrontendScreenGate.hpp"
#include "../RuntimeProbe/FrontendControllerObservation.hpp"
#include "ObservedMenuArrival.hpp"
#include "NativeOfflineStartup.hpp"
#include "../RuntimeProbe/NativeStartupEntryObservation.hpp"
#include "../RuntimeProbe/BorrowedSettingsStartObservation.hpp"
#include "TommyEndRaceObservation.hpp"
#include "NativeJasonCalls.hpp"
#include "NativeOfflineLifecycleHooks.hpp"
#include "FrontendIntegrationReport.hpp"
#include "AcceptedCounselorCommitPolicy.hpp"
#include "AcceptedCounselorPrebirth.hpp"
#include "NativeSandboxMenuContinuation.hpp"
#include "../RuntimeProbe/NativePickerSourceObservation.hpp"
#include "../RuntimeProbe/PickerPrecreationObservation.hpp"
#include "../RuntimeProbe/BorrowedConstructObservation.hpp"
#include "NativePickerCoordinator.hpp"
#include "NativeFrontendEpochSource.hpp"
#include "../Core/FrontendEpochService.hpp"
#include "NativePickerArrivalObservation.hpp"
#include "NativePickerStartAdmission.hpp"
#include "../Core/FrontendArrivalWindow.hpp"
#endif
#ifdef CFG_SPAWN_ADAPTER
#include "ReceiveTickObservation.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#ifdef CFG_JASON_APPROACH
#include "../Core/JasonApproachPolicy.hpp"
#include "JasonInstanceObservation.hpp"
#include "../RuntimeProbe/JasonApproachBindings.generated.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#ifdef CFG_PRIMARY_JASON
#include "PrimarySpawnObservation.hpp"
#include "CrowdObservation.hpp"
#include "PrimaryContextObservation.hpp"
#include "HostedAdoption.hpp"
#include "../Core/HostedOptions.hpp"
#include <intrin.h>
#include "../Core/HostedFillPolicy.hpp"
#include "../Core/H20FeatureFlags.hpp"
#include "../RuntimeProbe/HostedFill35.generated.hpp"
#if CFG_H21_LOBBY_DISCOVERY
#include "HostedLobbyDiscovery.hpp"
#if CFG_H21_LOBBY_ROWS
#include "HostedLobbyPlacement.hpp"
#include "HostedLobbyLayout.hpp"
#include "HostedLobbyStaticInputs.hpp"
#include "HostedRowRetry.hpp"
#include "HostedDeferredFill.hpp"
#include "HostedInputPremise.hpp"
#endif
#endif
#ifdef CFG_H1_OBSERVE
#include "HostedH1Sampler.hpp"
#if CFG_H22_HOTKEYS
#include "HostedBirthAdmission.hpp"
#include "../Core/HostedCounselorBirth.hpp"
#endif
#include "HostedPlanBuilder.hpp"
#include "HostedCounselorSource.hpp"
#include "../Core/HostedKeyAction.hpp"

#include "HostedJasonTakeover.hpp"
#endif
#include "KillerStartObservation.hpp"
#include "../Core/KillerStartBootstrap.hpp"
#include "../RuntimeProbe/ActorRotationBindings.generated.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
#ifdef CFG_GAMEPLAY_BUNDLE
#include "../Core/VehiclePursuitPolicy.hpp"
#include "VehicleRoadRuntime.hpp"
#include "FreshAbilityClassObservation.hpp"
#include "PoliceExitObservation.hpp"
#include "PreviousWorldObservation.hpp"
#include "J24FailedStartWorld.hpp"
#include "WorldRootInventory.hpp"
#include "../RuntimeProbe/FreshAbilityBindings.generated.hpp"
#include "../RuntimeProbe/NativePauseBindings.hpp"
#include "../Core/VehicleActionPolicy.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "../Core/CounselorPursuitPolicy.hpp"
#include "../Core/HidingInteractionPolicy.hpp"
#include "../RuntimeProbe/ActorTeleportBindings.generated.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include "../RuntimeProbe/VehicleBindings.generated.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include "../Core/GameplayConfiguration.hpp"
#include "DifficultyPolicy.hpp"
#include "../RuntimeProbe/DifficultyBindings.generated.hpp"
#include "../RuntimeProbe/HunterBindings.generated.hpp"
#include "../RuntimeProbe/ConvergenceBindings.generated.hpp"
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../Core/CounselorConvergencePolicy.hpp"
#include "../Core/ConvergencePublication.hpp"
#include "ConvergenceBlackboardSchema.hpp"
#include "ConvergenceFearObservation.hpp"
#include "../Core/SweaterLifecyclePolicy.hpp"
#include "RetiredOwnedAbility.hpp"
#include "../Core/SweaterRetention.hpp"
#include "../Core/SpectatorStatusObservation.hpp"
#include "../RuntimeProbe/SweaterRetentionBindings.generated.hpp"
#include "../RuntimeProbe/OfflineRecoveryBindings.generated.hpp"
#include "../RuntimeProbe/AiActionBindings.generated.hpp"
#include "../Core/SweaterPublication.hpp"
#include "SweaterSourceObservation.hpp"
#include "../RuntimeProbe/SweaterBindings.generated.hpp"
#include "../RuntimeProbe/SweaterHudBindings.generated.hpp"
#include "SweaterHudObservation.hpp"
#include "SweaterPickup.hpp"
#include "NativeOfflineSettings.hpp"
#include "NativeSettingsMenuOptions.hpp"
#include "NativeStartupTraps.hpp"
#include "CabinBoundsRuntime.hpp"
#include "../Core/StartupTrapMapScope.hpp"
#include "NativeRestrictedLoadout.hpp"
#include "ExpandedLoadoutRuntime.hpp"
#include "ConvergenceAxeRuntime.hpp"
#include "ObjectiveHudRuntime.hpp"
#include "ObjectiveHudWrite.hpp"
#include "ObjectiveStackObservation.hpp"
#include "../Core/JasonDeathCompletion.hpp"
#include "ObjectiveHudRepair.hpp"
#include "../Core/ObjectiveCacheAdmissionPolicy.hpp"
#include "../RuntimeProbe/NativeWidgetOwnershipObservation.hpp"
#include "../Core/LoadoutResourcePolicy.hpp"
#include "KillRuntime.hpp"
#include "PamelaDamageObservation.hpp"
#include "PamelaFinisherEligibility.hpp"
#include "../Core/PamelaFinisherPolicy.hpp"
#include "../RuntimeProbe/Compatibility/CfgPamelaFinisher35.hpp"
#include "../RuntimeProbe/KillContactBindings.generated.hpp"
#include "KnifeRuntime.hpp"
#include "KnifeNotifyCompletion.hpp"
#include "KnifeReleaseCompatibility.hpp"
#include "KnifeOwnerObservation.hpp"
#include "VehicleOccupancyObservation.hpp"
#include "VehicleStopObservation.hpp"
#include "../RuntimeProbe/KnifeSequenceBindings.generated.hpp"
#include "NativeKnifeFlightObservation.hpp"
#include "../Core/EndingMetadataPolicy.hpp"
#include "EndingRetainedCall.hpp"
#include "../Core/OnFootBurstPolicy.hpp"
#include "NativeEndingSkin.hpp"
#include "OfflineEndingPresentation.hpp"
#include "../Core/BotNamePolicy.hpp"
#include "BotNameObservation.hpp"
#include "../RuntimeProbe/BotNameBindings.generated.hpp"
#include "../RuntimeProbe/EndingMetadataBindings.generated.hpp"
#include "../RuntimeProbe/CounselorBirthBindings.generated.hpp"
#include "KillEligibility.hpp"
#include "KillDamageGate.hpp"
#include "BundleObservations.hpp"
#include "OwnedCounselorTemplate.hpp"
#include "CounselorStartObservation.hpp"
#include "ResolvedCounselorTemplate.hpp"
#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/CounselorLoadPolicy.hpp"
#include "../RuntimeProbe/CounselorLoadBindings.generated.hpp"
#include "../RuntimeProbe/NativeClassPathChoice.hpp"
#include "../RuntimeProbe/SpectatorRuntime.hpp"
#include "../RuntimeProbe/SpectatorNativeBindings.generated.hpp"
#include "SelectedDoorObservation.hpp"
#include "SpectatorCycleObservation.hpp"
#include "../Core/DoorRecoveryPolicy.hpp"
#include "../Core/DriverNavigationPolicy.hpp"
#endif
#endif
#else
#include "../Core/OneCounselorSpawnPolicy.hpp"
#endif
#include "SpawnResultObservation.hpp"
#include "SpawnAssetObservation.hpp"
#endif
#ifdef CFG_STATS_ADAPTER
#include "StatsRuntime.hpp"
#endif
#ifdef CFG_EMOTE_TRACE
#include "EmoteTrace.hpp"
#endif
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
#include "HostedSpectatorObservation.hpp"
#endif

#include "../Core/DonorJasonNativeOwnerRoute.hpp"
namespace {
cfgobserver::Plan plan;
#if defined(CFG_FRONTEND_BUNDLE) || defined(CFG_GAMEPLAY_BUNDLE)
// Published before the dispatcher patch; NEVER changes during world adoption.
DWORD observerOwnerThread=0;
#endif
cfgobserver::Report report;
cfgport::callbacks::Capture capture;
using Dispatch=void(__fastcall*)(void*,void*,void*);
Dispatch originalActorDispatcher=nullptr;
thread_local bool observerNested=false;
#ifdef CFG_CALLBACK_BOUNDARY_TRACE
cfgcallbacktrace::Capture boundaryTrace;
thread_local unsigned boundaryDepth=0;
thread_local bool boundaryRoot=false;
unsigned boundarySamples=0;
cfgcallbacktrace::Sample BoundarySample() noexcept {
    cfgcallbacktrace::Sample sample;
    if(boundarySamples>=256)return sample;
    ++boundarySamples; // Lifetime cap: at most 768 image-global/cache reads.
    boundaryTrace.Data().sampleAttempts=boundarySamples;
    SIZE_T bytes=0;const auto process=GetCurrentProcess();
    const auto read=[&](std::uintptr_t rva,void* out,SIZE_T size) noexcept {
        return ReadProcessMemory(process,reinterpret_cast<void*>(plan.module+rva),out,size,&bytes)&&bytes==size;
    };
    sample.valid=static_cast<std::uint8_t>(
        read(cfgcompat::cfg35::functions::ActorReceiveTick.cacheRva,&sample.tick,sizeof(sample.tick))&&
        read(cfgcompat::cfg35::lookup::GarbageCollectionStateRva,&sample.gc,sizeof(sample.gc))&&
        read(cfgcompat::cfg35::lookup::SerializationFlagRva,&sample.serializing,sizeof(sample.serializing)));
    return sample;
}
#endif
volatile LONG preparationAttempted=0;
#ifdef CFG_GAMEPLAY_BUNDLE
cfgbundle::Configuration bundleConfiguration;
volatile LONG bundleConfigurationState=0; // 0empty / 1copying / 2immutable.
#endif
#ifdef CFG_EMOTE_TRACE
cfgemote::Trace emoteTrace;
#endif
#ifdef CFG_STATS_ADAPTER
cfgstats::Runtime statsRuntime;
cfgport::stats::Identity StatsIdentity() noexcept {
    return {static_cast<std::uint64_t>(plan.objects[0].serial),plan.pawn,static_cast<std::uint64_t>(plan.objects[3].serial)};
}
#endif
class Reader {
    static bool Copy(std::uintptr_t address,void* output,std::size_t bytes,DWORD& error)noexcept{
#ifdef CFG_FRONTEND_BUNDLE
        // This DLL observes its own process, never a remote handle. Reuse the
        // guarded local-copy primitive from the escape callback reader while
        // retaining the caller's extent/read budget and every source proof.
        cfgchallengeescape::dispatch::WindowsSource source;
        if(source.ReadBytes(address,output,bytes))return true;
        error=ERROR_PARTIAL_COPY;return false;
#else
        SIZE_T got=0;
        if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),output,bytes,&got)){error=GetLastError();return false;}
        if(got!=bytes){error=ERROR_PARTIAL_COPY;return false;}return true;
#endif
    }
public:
    template<class T> bool Read(std::uintptr_t address,T& output) noexcept {
        if (reads>=cap || !cfgprobe::lifetime::Extent(address,sizeof(T))) { error=ERROR_INVALID_PARAMETER; return false; }
        ++reads;T value{};
        if(!Copy(address,&value,sizeof(value),error))return false;
        output=value; return true;
    }
    unsigned reads=0,cap=64;
    DWORD error=0;
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t bytes)noexcept{
        if(!bytes||bytes>16384||reads>=cap||!cfgprobe::lifetime::Extent(address,bytes)){error=ERROR_INVALID_PARAMETER;return false;}
        ++reads;return Copy(address,output,bytes,error);
    }
};
#ifdef CFG_FRONTEND_BUNDLE
void FrontendPreparationRefused(unsigned reads,DWORD error) noexcept;
Reader nativeOfflineReader;
cfgmenu::offlinestartup::Startup<Reader> nativeOfflineStartup(nativeOfflineReader);
cfgmenu::offlinehooks::WindowsBackend nativeOfflineHookBackend;
cfgmenu::offlinehooks::Pair<> nativeOfflineHooks(nativeOfflineHookBackend);
cfgmenu::offlinehooks::Handler nativeOfflineOriginalIntro=nullptr,nativeOfflineOriginalMatch=nullptr;
void __fastcall NativeOfflineIntro(void*);
void __fastcall NativeOfflineMatch(void*);
bool PrepareNativeOfflineHooks(std::uintptr_t,HMODULE)noexcept;
bool PrepareChallengeEscapeHooks(std::uintptr_t,HMODULE)noexcept;
bool NativeOfflineWorld()noexcept{
    const auto& s=nativeOfflineStartup.Status();
    return s.stage==cfgmenu::offlinestartup::Stage::Ready&&s.world==plan.world&&s.mode==plan.mode&&s.human==plan.pawn;
}
#endif
void SetStatus(cfgobserver::Status status) noexcept {
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&report.status),static_cast<LONG>(status));
}
DWORD Refuse(cfgobserver::Status status,cfgobserver::PreparationStage stage,unsigned reads,DWORD error=0) noexcept {
    report.preparationStage=static_cast<DWORD>(stage); report.preparationReads=reads; report.preparationError=error;
#ifdef CFG_FRONTEND_BUNDLE
    FrontendPreparationRefused(reads,error);
#endif
    MemoryBarrier(); SetStatus(status); return static_cast<DWORD>(status);
}
bool ExactBuild(Reader& reader,std::uintptr_t module) noexcept {
    if (module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))) return false;
    IMAGE_DOS_HEADER dos{}; IMAGE_NT_HEADERS64 nt{};
    if (!reader.Read(module,dos) || dos.e_magic!=IMAGE_DOS_SIGNATURE || dos.e_lfanew<64 || dos.e_lfanew>4096 ||
        !reader.Read(module+static_cast<std::uintptr_t>(dos.e_lfanew),nt) || nt.Signature!=IMAGE_NT_SIGNATURE ||
        nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 || nt.FileHeader.TimeDateStamp!=cfgprobe::ExecutableTimestamp ||
        nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC || nt.OptionalHeader.SizeOfImage!=cfgprobe::ExecutableImageBytes) return false;
    wchar_t path[32768]{}; const auto length=GetModuleFileNameW(nullptr,path,32768);
    if (!length || length>=32768) return false;
    // Any install location (any drive/folder). Identity = this process's own main image with the game's
    // layout, plus the exact size, PE timestamp/image size above and the SHA-256 of the locked file below.
    constexpr wchar_t ExpectedTail[]=L"\\CampForestGreen\\Binaries\\Win64\\SummerCamp-Win64-Shipping.exe";
    constexpr std::size_t tail=(sizeof(ExpectedTail)/sizeof(wchar_t))-1;
    if (length<=tail || _wcsicmp(path+(length-tail),ExpectedTail)!=0) return false;
    // Lock the exact image file against replacement during preparation. Hash is
    // of this handle, not a different path read; caller must re-pin installation.
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (file==INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    BCRYPT_ALG_HANDLE algorithm=nullptr; BCRYPT_HASH_HANDLE hash=nullptr;
    std::array<unsigned char,4096> storage{};
    std::array<unsigned char,65536> buffer{};
    std::array<unsigned char,32> digest{};
    DWORD objectBytes=0,got=0;
    bool okay=GetFileSizeEx(file,&size) && size.QuadPart==static_cast<LONGLONG>(cfgprobe::ExecutableBytes) &&
        BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0 &&
        BCryptGetProperty(algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&objectBytes),sizeof(objectBytes),&got,0)>=0 &&
        objectBytes>0 && objectBytes<=storage.size() &&
        BCryptCreateHash(algorithm,&hash,storage.data(),objectBytes,nullptr,0,0)>=0;
    while (okay) {
        got=0;
        if (!ReadFile(file,buffer.data(),static_cast<DWORD>(buffer.size()),&got,nullptr)) { okay=false; break; }
        if (!got) break;
        if (BCryptHashData(hash,buffer.data(),got,0)<0) { okay=false; break; }
    }
    if (okay) okay=BCryptFinishHash(hash,digest.data(),static_cast<ULONG>(digest.size()),0)>=0;
    if (okay) {
        constexpr char Hex[]="0123456789ABCDEF";
        for (std::size_t i=0;i<digest.size();++i)
            if (cfgprobe::ExecutableSha256[i*2]!=Hex[digest[i]>>4] || cfgprobe::ExecutableSha256[i*2+1]!=Hex[digest[i]&15]) { okay=false; break; }
    }
    if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    CloseHandle(file); return okay;
}
bool NativeClass(Reader& r,std::uintptr_t object,cfgprobe::NativeClass id) noexcept {
    const auto& binding=cfgprobe::NativeClasses[id];
    std::uintptr_t reflection=0,constructor=0;
    cfgprobe::Ancestry ancestry;
    if (!r.Read(plan.module+binding.reflectionCacheRva,reflection) ||
        !r.Read(plan.module+binding.constructorCacheRva,constructor)) return false;
    const auto native=cfgprobe::MatchingClassCache(reflection,constructor);
    return native && cfgprobe::CaptureAncestry(r,object,ancestry) && ancestry.Contains(native) &&
        cfgprobe::StableAncestry(r,object,ancestry) &&
        cfgobserver::Same(r,plan.module+binding.reflectionCacheRva,native) &&
        cfgobserver::Same(r,plan.module+binding.constructorCacheRva,native);
}
bool NativeThread(DWORD& error) noexcept {
    HANDLE thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION,FALSE,plan.threadId);
    if (!thread) { error=GetLastError(); return false; }
    const auto pid=GetProcessIdOfThread(thread); if (!pid) error=GetLastError();
    const bool belongs=pid==GetCurrentProcessId(); CloseHandle(thread); return belongs;
}
#ifdef CFG_STATS_ADAPTER
class StatsAccess {
public:
    StatsAccess() noexcept { reader.cap=256; }
    bool ContextMatches() noexcept {
        if (!cfgobserver::ContextUnchanged(reader,plan)) return false;
        if (!validated_ && (!NativeClass(reader,plan.mode,cfgprobe::SandboxMode) ||
            !NativeClass(reader,plan.pawn,cfgprobe::Counselor) || !cfgstats::SchemaMatches(reader,plan.module))) return false;
        validated_=true; // schema/classes once per request; identities/flags again before write
        // Native generic dispatcher rejects bit29 in an object's slot flags.
        // This is an additional conservative rejection, NOT complete GC proof.
        for (const auto& object:plan.objects) {
            std::uint32_t flags=0;
            const auto slot=plan.objectArray.data+static_cast<std::uintptr_t>(object.index)*cfgprobe::lifetime::ItemBytes;
            if (!reader.Read(slot+8,flags) || (flags&0x20000000)) return false;
        }
        return true;
    }
    bool ReadStats(cfgport::stats::Values& value) noexcept { return reader.Read(plan.pawn+cfgstats::StatOffset,value); }
    bool Writable() noexcept {
        const auto address=plan.pawn+cfgstats::StatOffset; MEMORY_BASIC_INFORMATION region{};
        if (!cfgprobe::lifetime::Extent(address,sizeof(cfgport::stats::Values)) ||
            VirtualQuery(reinterpret_cast<void*>(address),&region,sizeof(region))!=sizeof(region) ||
            region.State!=MEM_COMMIT || region.Type!=MEM_PRIVATE || region.Protect!=PAGE_READWRITE) return false;
        const auto start=reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        return address>=start && address-start<=region.RegionSize && sizeof(cfgport::stats::Values)<=region.RegionSize-(address-start);
    }
    bool WriteStats(const cfgport::stats::Values& desired) noexcept {
        SIZE_T written=0;
        if (!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(plan.pawn+cfgstats::StatOffset),desired.data(),desired.size(),&written)) {
            reader.error=GetLastError(); return false;
        }
        if (written!=desired.size()) { reader.error=ERROR_PARTIAL_COPY; return false; } return true;
    }
    void Publish(std::uint32_t& generation,std::uint32_t value) noexcept {
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&generation),static_cast<LONG>(value));
    }
    unsigned Reads() const noexcept { return reader.reads; }
    DWORD Error() const noexcept { return reader.error; }
private:
    Reader reader;
    bool validated_=false;
};
void ServiceStats(void* object,bool wasNested) noexcept {
    if (reinterpret_cast<std::uintptr_t>(object)!=plan.pawn || wasNested || GetCurrentThreadId()!=plan.threadId ||
        !statsRuntime.Pending()) return;
    StatsAccess access; statsRuntime.Service(access);
}
#endif
// Shared read-only access for trace entry and completion. Each phase has a
// hard64-call read cap; there is no access on unrelated callbacks.
#ifdef CFG_EMOTE_TRACE
class EmoteAccess : public Reader {
public:
    bool ReadBytes(std::uintptr_t address,void* output,unsigned bytes) noexcept {
        if (!bytes || bytes>16 || reads>=cap || !cfgprobe::lifetime::Extent(address,bytes)) return false;
        ++reads; SIZE_T got=0;
        return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),output,bytes,&got) && got==bytes;
    }
    void Publish(std::uint32_t& value,std::uint32_t next) noexcept {
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&value),static_cast<LONG>(next));
    }
    unsigned Reads() const noexcept { return reads; }
};
#endif
#ifdef CFG_SPAWN_ADAPTER
#ifdef CFG_JASON_APPROACH
#include "JasonApproachRuntime.hpp"
#ifdef CFG_GAMEPLAY_BUNDLE
#ifdef CFG_H1_JASON
bool HostedJasonReservationArmed()noexcept;bool HostedJasonReservationAdopted()noexcept;
bool RetireHostedBeforeOffline(std::uintptr_t world)noexcept;
#endif
#include "GameplayBundleRuntime.hpp"
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
using H216SelectorOriginal=void*(__fastcall*)(void*,std::int32_t);
using H216CanSpectateOriginal=bool(__fastcall*)(void*,void*,void*);
H216SelectorOriginal h216SelectorOriginal=nullptr;
H216CanSpectateOriginal h216CanSpectateOriginal=nullptr;
std::uintptr_t h216SelectorTarget=0,h216CanSpectateTarget=0,h216Image=0;
bool h216HookAttempted=false;
std::atomic<bool> h216HooksReady{false};
std::atomic<DWORD> h216OwnerThread{0};
std::atomic<std::uint64_t> h216GenerationCounter{0},h216ActiveGeneration{0};
cfghosted::spectator::State h216Policy{};
thread_local bool h216SelectorBusy=false,h216CanSpectateBusy=false;

bool H216OwnedHooksCurrent()noexcept {
    if(!h216HooksReady.load(std::memory_order_acquire)||h216Image!=plan.module||
       !h216SelectorOriginal||!h216CanSpectateOriginal)return false;
    cfgownednative::CurrentProcess code;
    return cfgownednative::resident.Resolve(code,h216Image,h216SelectorTarget,
            cfghosted::spectator35disk::SelectorEntry)==reinterpret_cast<std::uintptr_t>(h216SelectorOriginal)&&
        cfgownednative::resident.Resolve(code,h216Image,h216CanSpectateTarget,
            cfghosted::spectator35disk::HuntCanSpectateEnvelopeEntry)==reinterpret_cast<std::uintptr_t>(h216CanSpectateOriginal);
}

void* __fastcall H216NativeSelector(void* viewer,std::int32_t direction)noexcept {
    const auto original=h216SelectorOriginal;if(!original)return nullptr;
    return cfgh216hooks::SelectorCall(h216SelectorBusy,original,viewer,direction,
        [&](void* stock,std::int32_t signedDirection)->void*{
            const auto callbackThread=GetCurrentThreadId();
            const auto ownerThread=h216OwnerThread.load(std::memory_order_acquire);
            return cfgh216hooks::OwnerThreadAfterOriginal(stock,callbackThread,ownerThread,
                [&](DWORD ownedThread,DWORD,void* stockResult)->void*{
                    const auto generation=h216ActiveGeneration.load(std::memory_order_acquire);
                    Reader reader;reader.cap=cfgh216spectator::ReadLimit;
                    cfgh216spectator::Request request{&plan,&cfgprimary::CurrentHostedBinding(),&hostedAdoptionReceipt,
                        &h216ActiveGeneration,generation,ownedThread,spawnAssets.gameState,
                        reinterpret_cast<std::uintptr_t>(viewer),reinterpret_cast<std::uintptr_t>(stockResult),
                        reinterpret_cast<std::uintptr_t>(stockResult)!=hostedAdoptionReceipt.controllerState};
                    return cfgh216hooks::ReadOnlyAfterOriginal<void*,cfgh216spectator::Observation>(stockResult,
                        stockResult&&generation&&HostedRoutineActive(),h216HooksReady.load(std::memory_order_acquire),
                        ownedThread,ownerThread,true,
                        [&]{return H216OwnedHooksCurrent();},
                        [&](cfgh216spectator::Observation& observed){return cfgh216spectator::Observe(reader,request,observed);},
                        [&](void* unchanged,const cfgh216spectator::Observation& observed){
                            const auto selected=cfghosted::spectator::SelectAfterOriginal(h216Policy,observed.proof,observed.candidate,
                                observed.stockCounselor,signedDirection);
                            return selected.object?reinterpret_cast<void*>(selected.object):unchanged;
                        });
                });
        });
}
bool __fastcall H216NativeCanSpectate(void* mode,void* viewer,void* state)noexcept {
    const auto original=h216CanSpectateOriginal;if(!original)return false;
    return cfgh216hooks::CanSpectateCall(h216CanSpectateBusy,original,mode,viewer,state,
        [&](bool stockResult,void* exactMode,void* exactViewer,void* exactState)->bool{
            const auto callbackThread=GetCurrentThreadId();
            const auto ownerThread=h216OwnerThread.load(std::memory_order_acquire);
            return cfgh216hooks::OwnerThreadAfterOriginal(stockResult,callbackThread,ownerThread,
                [&](DWORD ownedThread,DWORD, bool unchanged)->bool{
                    const auto generation=h216ActiveGeneration.load(std::memory_order_acquire);
                    Reader reader;reader.cap=cfgh216spectator::ReadLimit;
                    cfgh216spectator::Request request{&plan,&cfgprimary::CurrentHostedBinding(),&hostedAdoptionReceipt,
                        &h216ActiveGeneration,generation,ownedThread,spawnAssets.gameState,
                        reinterpret_cast<std::uintptr_t>(exactViewer),reinterpret_cast<std::uintptr_t>(exactState),false};
                    return cfgh216hooks::ReadOnlyAfterOriginal<bool,cfgh216spectator::Observation>(unchanged,
                        !unchanged&&generation&&HostedRoutineActive(),h216HooksReady.load(std::memory_order_acquire),
                        ownedThread,ownerThread,exactMode==reinterpret_cast<void*>(plan.mode),
                        [&]{return H216OwnedHooksCurrent();},
                        [&](cfgh216spectator::Observation& observed){return cfgh216spectator::Observe(reader,request,observed);},
                        [&](bool stock,const cfgh216spectator::Observation& observed){
                            return cfghosted::spectator::CanSpectateAfterOriginal(stock,observed.proof,observed.candidate);
                        });
                });
        });
}
bool H216NextGeneration(std::uint64_t& generation)noexcept {
    auto current=h216GenerationCounter.load(std::memory_order_relaxed);
    if(current==UINT64_MAX)return false;
    generation=current+1;
    return h216GenerationCounter.compare_exchange_strong(current,generation,std::memory_order_acq_rel,
        std::memory_order_relaxed);
}
bool H216HostedLeaseCurrent(Reader& reader,std::uintptr_t& image)noexcept {
    image=0;cfgobserver::Plan fresh{};const auto& binding=cfgprimary::CurrentHostedBinding();
    std::uintptr_t gameState=0,killer=0,killerState=0;
    return GetCurrentThreadId()==plan.threadId&&bundleRootLease&&HostedRoutineActive()&&
        cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&cfgprimary::RetainedWorld(reader,plan,fresh,binding)&&
        cfghosted::adoption::Current(reader,fresh.objectArray,hostedAdoptionReceipt,plan)&&
        reader.Read(plan.world+cfghosted::disk35::WorldGameState,gameState)&&gameState==spawnAssets.gameState&&
        reader.Read(gameState+cfghosted::birth35::CurrentKiller,killer)&&killer==hostedAdoptionReceipt.pawn.object&&
        reader.Read(gameState+cfghosted::birth35::CurrentKillerState,killerState)&&
        killerState==hostedAdoptionReceipt.controllerState&&(image=plan.module)!=0;
}
bool H216TypedVtables(Reader& reader,std::uintptr_t image)noexcept {
    std::uintptr_t pc=0,pcCtor=0,hunt=0,huntCtor=0,selector=0,canSpectate=0;
    return reader.Read(image+cfgspectator::disk::ControllerClassCache,pc)&&pc&&
        reader.Read(image+cfgspectator::disk::ControllerConstructorCache,pcCtor)&&pcCtor==pc&&
        reader.Read(image+cfghosted::disk35::HuntReflection,hunt)&&hunt&&
        reader.Read(image+cfghosted::disk35::HuntConstructor,huntCtor)&&huntCtor==hunt&&
        reader.Read(image+cfghosted::spectator35disk::SCPlayerControllerVtable+cfghosted::spectator35disk::SelectorSlot,selector)&&
        selector==image+cfghosted::spectator35disk::Selector&&
        reader.Read(image+cfghosted::spectator35disk::HuntModeVtable+cfghosted::spectator35disk::CanSpectateSlot,canSpectate)&&
        canSpectate==image+cfghosted::spectator35disk::HuntCanSpectate;
}
bool PrepareHostedSpectatorHooks()noexcept {
    Reader reader;reader.cap=2048;std::uintptr_t image=0;
    if(!H216HostedLeaseCurrent(reader,image)||!H216TypedVtables(reader,image))return false;
    cfgownednative::CurrentProcess code;
    const auto selectorTarget=image+cfghosted::spectator35disk::Selector;
    const auto canTarget=image+cfghosted::spectator35disk::HuntCanSpectate;
    if(h216HooksReady.load(std::memory_order_acquire)){
        if(image!=h216Image||cfgownednative::resident.Resolve(code,image,selectorTarget,
            cfghosted::spectator35disk::SelectorEntry)!=reinterpret_cast<std::uintptr_t>(h216SelectorOriginal)||
           cfgownednative::resident.Resolve(code,image,canTarget,
            cfghosted::spectator35disk::HuntCanSpectateEnvelopeEntry)!=reinterpret_cast<std::uintptr_t>(h216CanSpectateOriginal))return false;
    }else{
        if(h216HookAttempted||!cfgbundle::Code(reader,image,cfghosted::spectator35disk::Selector,
                cfghosted::spectator35disk::SelectorEntry)||
           !cfgbundle::Code(reader,image,cfghosted::spectator35disk::HuntCanSpectate,
                cfghosted::spectator35disk::HuntCanSpectateEnvelopeEntry))return false;
        h216HookAttempted=true;
        std::uintptr_t selectorSlot=0,canSlot=0;
        if(!reader.Read(image+cfghosted::spectator35disk::SCPlayerControllerVtable+
                cfghosted::spectator35disk::SelectorSlot,selectorSlot)||
           !reader.Read(image+cfghosted::spectator35disk::HuntModeVtable+
                cfghosted::spectator35disk::CanSpectateSlot,canSlot)||
           selectorSlot!=selectorTarget||canSlot!=canTarget)return false;
        void* selectorOriginal=nullptr;void* canOriginal=nullptr;
        if(MH_CreateHook(reinterpret_cast<void*>(selectorTarget),reinterpret_cast<void*>(&H216NativeSelector),&selectorOriginal)!=MH_OK)return false;
        if(MH_CreateHook(reinterpret_cast<void*>(canTarget),reinterpret_cast<void*>(&H216NativeCanSpectate),&canOriginal)!=MH_OK){
            MH_RemoveHook(reinterpret_cast<void*>(selectorTarget));return false;}
        h216SelectorOriginal=reinterpret_cast<H216SelectorOriginal>(selectorOriginal);
        h216CanSpectateOriginal=reinterpret_cast<H216CanSpectateOriginal>(canOriginal);
        h216SelectorTarget=selectorTarget;h216CanSpectateTarget=canTarget;h216Image=image;
        if(!h216SelectorOriginal||!h216CanSpectateOriginal||
           !cfgownednative::resident.Prepare(code,image,selectorTarget,cfghosted::spectator35disk::SelectorEntry,
                reinterpret_cast<std::uintptr_t>(selectorOriginal),reinterpret_cast<std::uintptr_t>(&H216NativeSelector),5)||
           !cfgownednative::resident.Prepare(code,image,canTarget,cfghosted::spectator35disk::HuntCanSpectateEnvelopeEntry,
                reinterpret_cast<std::uintptr_t>(canOriginal),reinterpret_cast<std::uintptr_t>(&H216NativeCanSpectate),5)){
            MH_RemoveHook(reinterpret_cast<void*>(canTarget));MH_RemoveHook(reinterpret_cast<void*>(selectorTarget));
            h216SelectorOriginal=nullptr;h216CanSpectateOriginal=nullptr;return false;}
        if(MH_EnableHook(reinterpret_cast<void*>(selectorTarget))!=MH_OK||
           MH_EnableHook(reinterpret_cast<void*>(canTarget))!=MH_OK||
           !cfgownednative::resident.Enable(code,selectorTarget)||!cfgownednative::resident.Enable(code,canTarget)){
            cfgownednative::resident.Disable(selectorTarget);cfgownednative::resident.Disable(canTarget);
            MH_DisableHook(reinterpret_cast<void*>(selectorTarget));MH_DisableHook(reinterpret_cast<void*>(canTarget));
            MH_RemoveHook(reinterpret_cast<void*>(selectorTarget));MH_RemoveHook(reinterpret_cast<void*>(canTarget));
            h216SelectorOriginal=nullptr;h216CanSpectateOriginal=nullptr;return false;}
        h216HooksReady.store(true,std::memory_order_release);
    }
    std::uint64_t generation=0;
    if(!H216NextGeneration(generation))return false;
    h216Policy={};h216OwnerThread.store(GetCurrentThreadId(),std::memory_order_release);
    h216ActiveGeneration.store(generation,std::memory_order_release);
    return true;
}
void RetireHostedSpectatorHooks()noexcept {
    h216OwnerThread.store(0,std::memory_order_release);
    h216ActiveGeneration.store(0,std::memory_order_release);h216Policy={};
}
#endif
#endif
#else
#include "SpawnRuntime.hpp"
#endif
#endif
#ifdef CFG_FRONTEND_BUNDLE
#include "GameplayEpochReset.hpp"
#include "FrontendBundleRuntime.hpp"
bool FrontendGameplayReady() noexcept {return frontendGameplayReady;}
#endif
#if defined(CFG35_URINATION_NATIVE_PORT) && defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
#include "UrinationNativeRuntime.hpp"
#endif
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
#include "PreviousWorldDiagnosticsRuntime.hpp"
#include "WorldRootRetirementRuntime.hpp"
#endif
#if defined(CFG_GC_REGISTRY_PROBE) || defined(CFG_RETAINED_COUNSELOR_SPAWN) || defined(CFG_RETAINED_PRIMARY_JASON)
#if !defined(CFG_RETAINED_COUNSELOR_SPAWN) && !defined(CFG_RETAINED_PRIMARY_JASON)
#include "GcRegistryProbeRuntime.hpp"
#endif
// Preparation can outlive the host's object-array snapshot: ExactBuild hashes
// the executable before this DLL reaches its identity gate, and normal engine
// allocations may grow or relocate GUObjectArray during that interval. Refresh
// only the header and accept it only if the same four positive owner identities
// and the complete offline ownership/dispatcher chain still validate.
bool ProbePreparationContextStable(Reader& reader) noexcept {
    cfgobserver::Plan refreshed{};
    if (!cfgprimary::RefreshKnownObjectStorage(reader,plan,refreshed)) {
        const auto failure=cfgobserver::DiagnoseContextFailure(reader,plan);
        gcRegistryReport.contextFailure=static_cast<std::uint32_t>(failure);
        report.preparationContextFailure=static_cast<std::uint32_t>(failure);
        return false;
    }
    plan=refreshed; // Publish only after the exact saved owners revalidate.
    gcRegistryReport.contextFailure=static_cast<std::uint32_t>(cfgobserver::ContextFailure::None);
    return true;
}
#endif
void Before(void* object,void* function,void* parameters,bool wasNested) noexcept {
#ifdef CFG_EMOTE_TRACE
    if (reinterpret_cast<std::uintptr_t>(object)!=plan.pawn || GetCurrentThreadId()!=plan.threadId ||
        !emoteTrace.Candidate(reinterpret_cast<std::uintptr_t>(object),plan.threadId,wasNested)) return;
    EmoteAccess access;
    emoteTrace.Observe(access,reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),GetTickCount64());
    // Generic report remains the immutable prepared installation metadata.
    // Trace-specific publication carries callback proof; no generic Tick log.
    return;
#else
    // Immutable context is published before ANY target patch. Foreign threads
    // and objects are rejected before accessing owner-thread mutable capture.
    if (reinterpret_cast<std::uintptr_t>(object)!=plan.pawn || GetCurrentThreadId()!=plan.threadId ||
        !capture.Candidate(reinterpret_cast<std::uintptr_t>(object),plan.threadId,wasNested)) return;
    const auto now=GetTickCount64();
    if (!capture.Due(now)) return; // No object reads in same-interval bursts.
    Reader reader;
#ifdef CFG_GC_REGISTRY_PROBE
    const auto contextFailure=cfgobserver::DiagnoseContextFailure(reader,plan);
    gcRegistryReport.contextFailure=static_cast<std::uint32_t>(contextFailure);
    const bool same=contextFailure==cfgobserver::ContextFailure::None;
#else
    const bool same=cfgobserver::ContextUnchanged(reader,plan);
#endif
    const bool recorded=capture.Record(reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),
        plan.threadId,now,same);
    report.hookInstalled=1;
    if (recorded) {
        const auto index=capture.Count()-1; report.events[index]=capture.At(index);
        MemoryBarrier();
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&report.publishedCount),static_cast<LONG>(capture.Count()));
    }
    report.stopReason=static_cast<std::uint32_t>(capture.Reason());
    SetStatus(capture.Armed()?cfgobserver::Status::Capturing:
        capture.Reason()==cfgport::callbacks::Stop::Budget?cfgobserver::Status::Complete:cfgobserver::Status::Stopped);
#endif
}
#ifdef CFG_H1_JASON
// H1 hosted takeover lease (defined in HostedH1Runtime.hpp). Declared here so the existing returned-callback
// lease prologue can be reused without a second copy; offline sessions never enter (takeoverDue stays false).
bool HostedH1EnterPlan(std::uintptr_t actor)noexcept;
void HostedH1LeavePlan()noexcept;
bool HostedH1Window(void* object,bool wasNested)noexcept;
struct HostedPlanScope{bool active;explicit HostedPlanScope(std::uintptr_t actor,std::uintptr_t fn=0,std::uintptr_t pc=0)noexcept:active(HostedH1EnterPlan(actor)){
    (void)fn;(void)pc;
    }
    ~HostedPlanScope(){
      if(active)HostedH1LeavePlan();}HostedPlanScope(const HostedPlanScope&)=delete;HostedPlanScope& operator=(const HostedPlanScope&)=delete;};
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
template<class LookupReader>bool ValidateBundleLookupScope(LookupReader& reader,void* expectedPlan) {
    if(!expectedPlan)return false;
    const auto& saved=*static_cast<const cfgobserver::Plan*>(expectedPlan);
    cfgobserver::Plan current{};
    return plan.module==saved.module&&plan.world==saved.world&&plan.mode==saved.mode&&
        plan.controller==saved.controller&&plan.pawn==saved.pawn&&plan.objects==saved.objects&&
        GetCurrentThreadId()==saved.threadId&&plan.threadId==saved.threadId&&
        (!bundleRootLease||PrimaryRootsCurrent(reader))&&cfgprimary::RetainedWorld(reader,saved,current);
}
#endif
void ServiceQueuedGameplay(void* object,void* function,bool wasNested,bool returned,
    std::uintptr_t returnPc) noexcept {
    if(wasNested||!returned||GetCurrentThreadId()!=plan.threadId||NativeJasonControllerTickInFlight())return;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    // Before ANY callback-marker publication. A reentrant PE callback never
    // replaces the native Tick's witness (nor another PE callback's witness).
    if(bundleRootLease||bundleRootWitness.lease)return;
#endif
    FlushBundleFirstFailureTrace();
#ifdef CFG_FRONTEND_BUNDLE
#ifdef CFG_H1_JASON
    HostedPlanScope hostedScope(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),returnPc); // swaps in the hosted plan ONLY for a due, armed takeover
    if(!frontendGameplayReady&&!hostedScope.active&&!HostedRoutineActive())return;
#else
    if(!frontendGameplayReady)return;
#endif
#endif
    const auto actor=reinterpret_cast<std::uintptr_t>(object);
    bool owned=cfgbundle::OwnedCallback(actor,plan.pawn,bundleJason.pawn.object,BundleGameplayStarted(),
        plan.controller,(bundleCompletion.terminalMask&1u)!=0);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    const bool presentationOwner=bundlePresentationPending&&(actor==plan.mode||actor==spawnAssets.gameState);
    owned=owned||presentationOwner;
#endif
    bool returnedStartup=false;
#if defined(CFG_RETAINED_PRIMARY_JASON) && defined(CFG_RETAINED_GAMEPLAY_BUNDLE)
    returnedStartup=cfgstartuplease::ReturnedPending(spawnRuntime.Pending(),primaryBootstrapRootsOpen,
#if defined(CFG_H1_JASON) && defined(CFG_FRONTEND_BUNDLE)
        hostedScope.active||HostedRoutineActive(),
#else
        false,
#endif
        actor==plan.pawn);
#endif
    if(!cfgtick::CompletedWorkCandidate(GetCurrentThreadId()==plan.threadId,wasNested,returned,
        owned,spawnRuntime.Pending()&&!returnedStartup))return;
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    // Budget is a controller-tick epoch, not an invented engine frame counter.
    // No live owned controller: explicitly labelled PE callback fallback.
    const auto j21Now=GetTickCount64();
    // Timestamp is scheduling only, never cached health/native authority. Do
    // not repeat a class/registry/health survey on every ProcessEvent callback.
    bundleJ21Budget.Fallback(plan.world,bundleJ21Budget.world==plan.world&&bundleJ21LastControllerAt&&
        j21Now>=bundleJ21LastControllerAt&&j21Now-bundleJ21LastControllerAt<=250);
    // #290: core PE maintenance keeps its accepted cadence even when a native
    // Tick or optional convergence/emote work overruns. Only B/C are budgeted.
    bundleJ21Budget.CorePE();
    J21Timing peTiming(cfgj21::PE);J21CallbackMemoScope memo(plan.world);
#endif
    Reader reader;reader.cap=128;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    reader.cap=1024;
    PublishBundleRootState(0,true);
    if(!cfgprimary::LookupVetoClearForObservation(reader,plan.module)){PublishBundleRootState(1);return;}
    if(!cfgtick::Observe(reader,plan.module,actor,reinterpret_cast<std::uintptr_t>(function),returnPc)){PublishBundleRootState(2);return;}
    if(!ValidateBundleLookupScope(reader,&plan)){PublishBundleRootState(3);return;}
    primaryRootTickFunction=reinterpret_cast<std::uintptr_t>(function);primaryRootTickReturn=returnPc;primaryRootTickActor=actor;
    cfgobserver::Plan scopePlan=plan;
    cfgprimary::ScopedLookupEligibility<Reader> scope(plan.module,&scopePlan,&ValidateBundleLookupScope,true);
    if(!scope.Entered()||!cfgprimary::LookupSafe(reader,plan.module)){PublishBundleRootState(3);return;}
    // Objective helpers use a bounded view. Reader capabilities do not transfer
    // between types: renew the SAME owner/root proof through that bounded view.
    // This is callback-local and preserves its read cap and GC/serialization vetoes.
    cfgprimary::ScopedLookupEligibility<cfgobjective::Budget<Reader>> objectiveScope(
        plan.module,&scopePlan,&ValidateBundleLookupScope,true);
    if(!objectiveScope.Entered()){PublishBundleRootState(3);return;}
    BundleCallbackRootScope rootScope(plan.module);
    if(!rootScope.Entered()){PublishBundleRootState(3);return;}
    if(!DrainBundleDeferredRetirement())return; // renewed tick/window; BEFORE any new root acquisition or AI
    primaryBootstrapRootsOpen=false;
    // Admit the exact returned callback first, then retain that actor/function
    // using its lease. Never rebind retention to a dead human's old tick.
    if(!RetainPrimaryObjects(reader,std::array<std::uintptr_t,2>{actor,primaryRootTickFunction})){PublishBundleRootState(4);return;}
    PublishBundleRootState(0,false,true);
#ifdef CFG_H1_JASON
    if(hostedScope.active){
        (void)HostedH1Window(object,wasNested);return;} // hosted: takeover transaction only; never the offline bundle service
#endif
#ifdef CFG_RETAINED_PRIMARY_JASON
    if(returnedStartup&&spawnRuntime.Pending()){
        ServiceSpawn(object,function,wasNested,returned,returnPc);
        return; // one startup slice, no stacked gameplay dispatch
    }
#endif
    if(bundlePresentationPending){
        bundlePresentationPending=false;
        const bool serviced=BundleEndingSummaryService(GetTickCount64());
        if(serviced||presentationOwner)return;
    }
#else
    if(!cfgprimary::LookupSafe(reader,plan.module)||
       !cfgtick::Observe(reader,plan.module,actor,reinterpret_cast<std::uintptr_t>(function),returnPc)||
       !cfgprimary::LookupSafe(reader,plan.module))return;
#endif
    const auto now=GetTickCount64();
    const auto bundlePhase=[&]() noexcept {
        const auto phases=bundleReport.phases;
        const auto generation=bundleReport.generation;
        ServiceBundle(object,wasNested);
        return bundleReport.phases!=phases||bundleReport.generation!=generation;
    };
    static std::uint64_t bundleGap=0; // native owner thread only; 25ms shared phase gap
    if(bundleSchedule.pressed){if(bundlePhase())bundleGap=now+25;return;}
    if(now<bundleGap)return;
    if(bundlePhase())bundleGap=now+25;
}
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
#include "NativeJasonControllerTickRuntime.hpp"
#endif
#ifdef CFG_H1_OBSERVE
#include "HostedH1Runtime.hpp"
#endif
void __fastcall Observe(void* object,void* function,void* parameters) {
#if defined(CFG_SPAWN_ADAPTER) || defined(CFG_GC_REGISTRY_PROBE)
    const auto callbackReturnPc=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
#endif
#ifdef CFG_CALLBACK_BOUNDARY_TRACE
    auto& diagnostic=boundaryTrace.Data();
    // Bounded diagnostic counters use atomics because the hook can be entered
    // by foreign threads; capture storage itself belongs to the owner thread.
    const auto count=[](std::uint32_t& value) noexcept {
        auto* cell=reinterpret_cast<volatile LONG*>(&value);
        const auto current=InterlockedCompareExchange(cell,0,0);
        if(current<65536)InterlockedCompareExchange(cell,current+1,current);
    };
    count(diagnostic.entries);
    if(GetCurrentThreadId()==plan.threadId){count(diagnostic.ownerEntries);
        if(reinterpret_cast<std::uintptr_t>(object)==plan.pawn)count(diagnostic.pawnEntries);}
    if(GetCurrentThreadId()!=plan.threadId||boundaryTrace.Data().published==cfgcallbacktrace::Capacity||
        (!boundaryRoot&&reinterpret_cast<std::uintptr_t>(object)!=plan.pawn)){
        originalActorDispatcher(object,function,parameters);return;
    }
    const auto sample=BoundarySample();
    const auto index=boundaryTrace.Begin(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),
        reinterpret_cast<std::uintptr_t>(_ReturnAddress()),GetCurrentThreadId(),boundaryDepth,GetTickCount64(),sample);
    const bool root=!boundaryDepth&&index<cfgcallbacktrace::Capacity;
    if(root)boundaryRoot=true;
    if(!boundaryRoot){originalActorDispatcher(object,function,parameters);return;}
    ++boundaryDepth;bool returned=false;
    __try{originalActorDispatcher(object,function,parameters);returned=true;}
    __finally{
        boundaryTrace.Finish(index,returned,BoundarySample());--boundaryDepth;
        if(root){boundaryRoot=false;MemoryBarrier();InterlockedExchange(
            reinterpret_cast<volatile LONG*>(&boundaryTrace.Data().published),static_cast<LONG>(boundaryTrace.CompletePrefix()));}
    }
    return;
#endif
#ifdef CFG_FRONTEND_BUNDLE
    // Foreign dispatchers must not read the owner-thread mutable gameplay Plan.
    if(GetCurrentThreadId()!=observerOwnerThread){originalActorDispatcher(object,function,parameters);return;}
#endif
    const bool wasNested=observerNested; observerNested=true;
#ifdef CFG_GAMEPLAY_BUNDLE
    const auto nativeCycleBefore=bundleSpectatorNativeCycles;
    cfgkillcontact::Proof meleeContactProof{};
    bool queuedGameplaySuppressed=false;
#endif
#ifdef CFG_FRONTEND_BUNDLE
    std::uintptr_t additionalConstructionEvent=0;
#endif
    bool dispatchForwarded=false;
#if defined(CFG_EMOTE_TRACE)||defined(CFG_FRONTEND_BUNDLE)||defined(CFG_GAMEPLAY_BUNDLE)
    bool completed=false;
#endif
    __try {
#ifdef CFG_STATS_ADAPTER
        ServiceStats(object,wasNested); // independent of bounded capture/disarm
#endif
#ifdef CFG_SPAWN_ADAPTER
#ifdef CFG_FRONTEND_BUNDLE
        const bool objectiveCleanup=BundleObjectiveLifecycle(wasNested);
#ifdef CFG_GAMEPLAY_BUNDLE
        queuedGameplaySuppressed=objectiveCleanup;
#endif
        if(frontendGameplayReady&&!objectiveCleanup){
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
        // Ending presentation also receives controller/mode/state events after
        // pawn teardown. Queued AI runs only after a verified tick.
        queuedGameplaySuppressed=ServiceBundlePresentation(object,wasNested);
#endif
#ifdef CFG_FRONTEND_BUNDLE
        }
#endif
#endif
#ifdef CFG_FRONTEND_BUNDLE
        // Generic capture was armed for the immutable menu HUD at installation.
        // Frontend/AI reports provide separate proof after native world travel.
        FrontendPrebirthSafe(object,function,parameters);
#else
        Before(object,function,parameters,wasNested);
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
        cfgspectatorstatus::Frame spectatorLabelFrame{};
        const bool spectatorLabelReady=BundleSpectatorStatusBefore(object,function,parameters,spectatorLabelFrame);
        BundleEmoteBefore(object,function,parameters,wasNested);
        if(!BundleNativeMeleeContact(object))BundleMeleeContactBefore(object,function,meleeContactProof);
#ifdef CFG_FRONTEND_BUNDLE
        additionalConstructionEvent=BundleAdditionalConstructionEvent(object,function);
#endif
        cfgspectatorstatus::Forward(originalActorDispatcher,object,function,parameters,spectatorLabelFrame,spectatorLabelReady);
#else
        originalActorDispatcher(object,function,parameters); // Exactly once, including ignored/nested calls.
#endif
        dispatchForwarded=true;
#if defined(CFG_EMOTE_TRACE)||defined(CFG_FRONTEND_BUNDLE)||defined(CFG_GAMEPLAY_BUNDLE)
        completed=true;
#endif
    } __finally {
#ifdef CFG_GC_REGISTRY_PROBE
        ServiceGcRegistryProbe(object,function,wasNested,dispatchForwarded,callbackReturnPc);
#endif
#ifdef CFG_H1_OBSERVE
        ServiceHostedH1(wasNested,dispatchForwarded,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),callbackReturnPc);
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
        ServiceQueuedGameplay(object,function,wasNested,dispatchForwarded&&!queuedGameplaySuppressed,callbackReturnPc);
#endif
#ifdef CFG_SPAWN_ADAPTER
        ServiceSpawn(object,function,wasNested,dispatchForwarded,callbackReturnPc);
#endif
#ifdef CFG_FRONTEND_BUNDLE
        BundleAdditionalConstructionEventAfter(additionalConstructionEvent,completed);
#endif
#ifdef CFG_GAMEPLAY_BUNDLE
        BundleDamageAfter(object,function,parameters,completed);
        BundleMeleeContactAfter(meleeContactProof,completed);
        BundleEmoteAfter(object,function,wasNested,completed);
#ifdef CFG35_URINATION_NATIVE_PORT
        BundleUrinationSelectionAfter(object,function,wasNested,completed);
#endif
        // UserWidget events use the separately pinned UObject bridge. Actor
        // events must not consume its bounded negative-class discovery cache.
        BundleSpectatorCycleAfter(object,function,wasNested,completed,bundleSpectatorNativeCycles!=nativeCycleBefore);
#endif
#ifdef CFG_FRONTEND_BUNDLE
        FrontendAfterSafe(object,function,wasNested,completed,callbackReturnPc);
#endif
#ifdef CFG_EMOTE_TRACE
        if (GetCurrentThreadId()==plan.threadId && emoteTrace.Owner(reinterpret_cast<std::uintptr_t>(object),plan.threadId,wasNested)) {
            EmoteAccess access; emoteTrace.After(access,reinterpret_cast<std::uintptr_t>(function),completed);
        }
#endif
        observerNested=wasNested; // Does not catch/swallow native exceptions.
    }
}
} // namespace

#ifdef CFG_GAMEPLAY_BUNDLE
extern "C" __declspec(dllexport) const cfgbundle::Report* WINAPI CfgGameplayBundleReport(){return &bundleReport;}
extern "C" __declspec(dllexport) DWORD WINAPI CfgQueueSpectatorToggle(std::uintptr_t inputWindow){
    const auto window=reinterpret_cast<HWND>(inputWindow);DWORD pid=0;
    if(!window||GetForegroundWindow()!=window||!GetWindowThreadProcessId(window,&pid)||pid!=GetCurrentProcessId())return 0;
    bundleSpectatorInput.Queue(GetTickCount64());return 1; // input only; no engine access/authority
}
extern "C" __declspec(dllexport) DWORD WINAPI CfgConfigureGameplayBundle(const cfgbundle::Configuration* input){
#ifdef CFG_FRONTEND_BUNDLE
    // The live frontend owns configuration; no helper can race its Plan swap.
    (void)input;return 0;
#else
    if(InterlockedCompareExchange(&bundleConfigurationState,1,0)!=0)return 0;
    Reader r;r.cap=512;cfgbundle::Configuration candidate;
    // Configuration is copied after preparation, before the first spawn request.
    // Validate the exact catalogue packet and world/human identity; no raw asset
    // pointer from a previous frontend/session may become a spawn choice.
    const auto observerStatus=InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&report.status),0,0);
    const bool prepared=observerStatus==static_cast<LONG>(cfgobserver::Status::Prepared)||
        observerStatus==static_cast<LONG>(cfgobserver::Status::Capturing)||observerStatus==static_cast<LONG>(cfgobserver::Status::Complete);
    if(!prepared||
       spawnRuntime.report.state!=static_cast<unsigned>(cfgspawn::SpawnState::Ready)||
       !r.Read(reinterpret_cast<std::uintptr_t>(input),candidate)||
       !cfgbundle::MatchesConfiguration(candidate,spawnRuntime.report.identity,spawnAssets.primary,cfgbundle::MaximumBots)||
       !cfgobserver::ContextUnchanged(r,plan)||
       !cfgprimary::SameEntry(r,plan.mode,spawnAssets.primary,candidate.jasonCatalogueIndex)){
        InterlockedExchange(&bundleConfigurationState,0);return 0;
    }
    bundleConfiguration=candidate;MemoryBarrier();InterlockedExchange(&bundleConfigurationState,2);return 1;
#endif
}
#endif

extern "C" __declspec(dllexport) DWORD WINAPI CfgPrepareObservation(const cfgobserver::Plan* input) {
    if (InterlockedCompareExchange(&preparationAttempted,1,0)!=0)
        return static_cast<DWORD>(cfgobserver::Status::RefusedPlan); // Never reset published context.
    report.bytes=sizeof(report); SetStatus(cfgobserver::Status::Preparing);
    Reader reader; reader.cap=256;
#ifdef CFG_FRONTEND_BUNDLE
    reader.cap=cfgobserver::FrontendPreparationReadLimit; // Aggregate one-time preparation, never a gameplay tick budget.
#endif
    using cfgobserver::Status; using cfgobserver::PreparationStage;
    if (!reader.Read(reinterpret_cast<std::uintptr_t>(input),plan) || !cfgobserver::SanePlan(plan))
        return Refuse(Status::RefusedPlan,PreparationStage::Input,reader.reads,reader.error);
#if defined(CFG_FRONTEND_BUNDLE) || defined(CFG_GAMEPLAY_BUNDLE)
    observerOwnerThread=plan.threadId;
#endif
    if (!ExactBuild(reader,plan.module)) return Refuse(Status::RefusedPlan,PreparationStage::Build,reader.reads,reader.error);
    DWORD threadError=0;
    if (!NativeThread(threadError)) return Refuse(Status::RefusedIdentity,PreparationStage::Thread,reader.reads,threadError);
#ifdef CFG_FRONTEND_BUNDLE
    if(!plan.frontend.widget||!PrepareFrontendResources(reader))return Refuse(Status::RefusedIdentity,PreparationStage::ContextBefore,reader.reads,reader.error);
#elif defined(CFG_SPAWN_ADAPTER) && !defined(CFG_PRIMARY_JASON)
    if (!NativeClass(reader,plan.mode,cfgprobe::OfflineBotsMode)) return Refuse(Status::RefusedIdentity,PreparationStage::ModeClass,reader.reads,reader.error);
#else
    if (!NativeClass(reader,plan.mode,cfgprobe::SandboxMode)) return Refuse(Status::RefusedIdentity,PreparationStage::ModeClass,reader.reads,reader.error);
#endif
#ifdef CFG_FRONTEND_BUNDLE
    if (!cfgmenu::controllergate::Menu(reader,plan.module,plan.controller)) return Refuse(Status::RefusedIdentity,PreparationStage::ControllerClass,reader.reads,reader.error);
#else
    if (!NativeClass(reader,plan.controller,cfgprobe::PlayerControllerClass)) return Refuse(Status::RefusedIdentity,PreparationStage::ControllerClass,reader.reads,reader.error);
#endif
#ifndef CFG_FRONTEND_BUNDLE
#if defined(CFG_SPAWN_ADAPTER) && !defined(CFG_PRIMARY_JASON)
    if (!NativeClass(reader,plan.pawn,cfgprobe::Killer)) return Refuse(Status::RefusedIdentity,PreparationStage::PawnClass,reader.reads,reader.error);
#else
    if (!NativeClass(reader,plan.pawn,cfgprobe::Counselor)) return Refuse(Status::RefusedIdentity,PreparationStage::PawnClass,reader.reads,reader.error);
#endif
#endif
#if defined(CFG_GC_REGISTRY_PROBE) || defined(CFG_RETAINED_COUNSELOR_SPAWN) || defined(CFG_RETAINED_PRIMARY_JASON)
    if (!ProbePreparationContextStable(reader)) return Refuse(Status::RefusedIdentity,PreparationStage::ContextBefore,reader.reads,reader.error);
#else
    if (!cfgobserver::ContextUnchanged(reader,plan)) return Refuse(Status::RefusedIdentity,PreparationStage::ContextBefore,reader.reads,reader.error);
#endif
#ifdef CFG_SPAWN_ADAPTER
#ifndef CFG_FRONTEND_BUNDLE
    if (!PrepareSpawnResources()) return Refuse(Status::RefusedIdentity,PreparationStage::SpawnBindings,reader.reads,reader.error);
#endif
#endif
    cfgobserver::Prepared prepared;
    if (!cfgobserver::Prepare(reinterpret_cast<void*>(plan.dispatch.target),reinterpret_cast<void*>(&Observe),prepared))
        return Refuse(Status::RefusedInstrumentation,PreparationStage::Trampoline,reader.reads);
#if defined(CFG_GC_REGISTRY_PROBE) || defined(CFG_RETAINED_COUNSELOR_SPAWN) || defined(CFG_RETAINED_PRIMARY_JASON)
    if (!ProbePreparationContextStable(reader)) return Refuse(Status::RefusedIdentity,PreparationStage::ContextAfter,reader.reads,reader.error);
#else
    if (!cfgobserver::ContextUnchanged(reader,plan)) return Refuse(Status::RefusedIdentity,PreparationStage::ContextAfter,reader.reads,reader.error);
#endif
#ifdef CFG_EMOTE_TRACE
    std::uintptr_t counselorClass=0;
    if (!reader.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselorClass) ||
        !emoteTrace.Prepare(reader,plan,counselorClass))
        return Refuse(Status::RefusedIdentity,PreparationStage::EmoteBindings,reader.reads,reader.error);
#endif
    if (!capture.Arm(plan.pawn,plan.threadId)) return Refuse(Status::RefusedIdentity,PreparationStage::Capture,reader.reads);
    HMODULE resident=nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        reinterpret_cast<LPCWSTR>(&Observe),&resident)) return Refuse(Status::RefusedIdentity,PreparationStage::ModulePin,reader.reads,GetLastError());
    report.dllPinned=1; // Standard Windows lifetime pin, not a protection change.
#ifdef CFG_FRONTEND_BUNDLE
#ifdef CFG35_MENU_RETENTION_TEST
    // Integration stage: existing menu UI ONLY. Do not install native match,
    // gameplay, challenge or input hooks before their own retained adapters.
#if defined(CFG35_NATIVE_STARTUP_ENTRY_TEST) || defined(CFG35_NATIVE_COUNSELOR_STARTUP_TEST)
    // Two staged native hooks only. The counselor candidate uses the existing
    // startup coordinator; the older entry candidate forwards stock only.
    if(!PrepareNativeOfflineHooks(plan.module,resident))return Refuse(Status::RefusedInstrumentation,PreparationStage::Trampoline,reader.reads);
#if defined(CFG35_PRIMARY_GAMEPLAY_TEST) && CFG_J29_CHALLENGE_ESCAPE_HOOKS
    // The native pair has already applied its queue. Escape Five<> is a
    // second independent all-or-nothing transaction; failure is NON-FATAL.
    // Do not touch nativeOfflineHooks, and do not retry a quarantined group.
    (void)cfgj29escape::InstallAfterNative(true,[&]{return PrepareChallengeEscapeHooks(plan.module,resident);});
#endif
#if defined(CFG35_PRIMARY_GAMEPLAY_TEST) && CFG_J214_TRAP_PRIORITY
    // Optional observer: failure cannot roll back the enabled startup pair.
    // Queue owned trap intent only; Morph remains in the retained AI Tick.
    (void)PrepareBundleTrapHook(plan.module);
#endif
#endif
#else
    if(!PrepareNativeOfflineHooks(plan.module,resident))return Refuse(Status::RefusedInstrumentation,PreparationStage::Trampoline,reader.reads);
    if(!PrepareChallengeEscapeHooks(plan.module,resident))return Refuse(Status::RefusedInstrumentation,PreparationStage::Trampoline,reader.reads);
    // Preparation thread only: donor's normal XInput fallback. Missing input
    // support is optional and must not refuse otherwise valid offline setup.
    bundleRBEmote.PrepareSystemProvider();
    frontendChallengeStartPad.PrepareSystemProvider();
#endif
#endif
    originalActorDispatcher=reinterpret_cast<Dispatch>(prepared.trampoline);
    report.pawn=plan.pawn; report.threadId=plan.threadId; report.target=prepared.target;
#ifdef CFG_CALLBACK_BOUNDARY_TRACE
    boundaryTrace.Arm(plan.module,plan.pawn,plan.threadId);
#endif
    report.trampoline=prepared.trampoline; report.relay=prepared.relay; report.detour=prepared.detour;
    report.desiredPatch=prepared.patch; report.preparationReads=reader.reads;
#ifdef CFG_STATS_ADAPTER
    statsRuntime.Configure(StatsIdentity()); // immutable identity before target patch
#endif
#ifdef CFG_FRONTEND_BUNDLE
    FrontendState(8); // Only after all preparation gates have succeeded.
#endif
    MemoryBarrier(); SetStatus(cfgobserver::Status::Prepared);
    return static_cast<DWORD>(cfgobserver::Status::Prepared);
}
extern "C" __declspec(dllexport) const cfgobserver::Report* WINAPI CfgObserverReport() { return &report; }
#ifdef CFG_GC_REGISTRY_PROBE
extern "C" __declspec(dllexport) const cfggcprobe::Report* WINAPI CfgGcRegistryProbeReport(){return &gcRegistryReport;}
#ifdef CFG_SYNCHRONIZED_ROOT_PROBE
extern "C" __declspec(dllexport) const cfgrootprobe::Report* WINAPI CfgRootPublicationReport(){return &rootPublicationReport;}
#endif
#endif
#ifdef CFG_CALLBACK_BOUNDARY_TRACE
extern "C" __declspec(dllexport) const cfgcallbacktrace::Report* WINAPI CfgCallbackBoundaryReport(){return &boundaryTrace.Data();}
#endif
extern "C" __declspec(dllexport) const void* WINAPI CfgObserverDetourAddress() { return reinterpret_cast<const void*>(&Observe); }
#ifdef CFG_H1_OBSERVE
extern "C" __declspec(dllexport) const cfgh1::Report* WINAPI CfgHostedH1Report(){return &hostedH1Report;}
#if CFG_H21_LOBBY_DISCOVERY
extern "C" __declspec(dllexport) const cfghostedlobby::Report* WINAPI CfgHostedLobbyReport(){return &hostedLobbyReport;}
#endif
#endif
#ifdef CFG_FRONTEND_BUNDLE
extern "C" __declspec(dllexport) const FrontendReport* WINAPI CfgFrontendIntegrationReport(){return &frontendReport;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
extern "C" __declspec(dllexport) const cfgmenu::retention::Report* WINAPI CfgFrontendRetentionReport(){return &frontendRetentionReport;}
#endif
#endif
#ifdef CFG_SPAWN_ADAPTER
#ifdef CFG_PRIMARY_JASON
extern "C" __declspec(dllexport) const cfgspawn::SpawnReport* WINAPI CfgPrimaryJasonReport() { return &spawnRuntime.report; }
extern "C" __declspec(dllexport) DWORD WINAPI CfgRequestPrimaryJason(const cfgspawn::SpawnRequest* input) {
#elif defined(CFG_JASON_APPROACH)
extern "C" __declspec(dllexport) const cfgspawn::SpawnReport* WINAPI CfgJasonApproachReport() { return &spawnRuntime.report; }
extern "C" __declspec(dllexport) DWORD WINAPI CfgRequestJasonApproach(const cfgspawn::SpawnRequest* input) {
#else
extern "C" __declspec(dllexport) const cfgspawn::SpawnReport* WINAPI CfgCounselorSpawnReport() { return &spawnRuntime.report; }
extern "C" __declspec(dllexport) DWORD WINAPI CfgRequestCounselorSpawn(const cfgspawn::SpawnRequest* input) {
#endif
#ifdef CFG_FRONTEND_BUNDLE
    // One automatic request is emitted only by the native arrival owner.
    (void)input;
    return 0;
#else
    Reader r; cfgspawn::SpawnRequest request;
#ifdef CFG_GAMEPLAY_BUNDLE
    if(InterlockedCompareExchange(&bundleConfigurationState,2,2)!=2)return 0;
#endif
    return r.Read(reinterpret_cast<std::uintptr_t>(input),request) && spawnRuntime.Submit(request)?1:0;
#endif
}
#endif
#ifdef CFG_EMOTE_TRACE
extern "C" __declspec(dllexport) const cfgemote::Report* WINAPI CfgEmoteTraceReport() { return &emoteTrace.report; }
#endif
#ifdef CFG_STATS_ADAPTER
extern "C" __declspec(dllexport) const cfgstats::Report* WINAPI CfgStatsReport() { return &statsRuntime.report; }
extern "C" __declspec(dllexport) DWORD WINAPI CfgRequestStats(const cfgstats::Request* input) {
    Reader reader; reader.cap=1; cfgstats::Request request;
    return reader.Read(reinterpret_cast<std::uintptr_t>(input),request) && statsRuntime.Submit(request)?1:0;
}
#endif
BOOL WINAPI DllMain(HINSTANCE,DWORD,LPVOID) { return TRUE; }
