#pragma once

#include <array>
#include <cstdint>

// Version-specific disk facts live here. These constants identify an image and
// describe observed layouts; they do NOT authorize runtime reads, calls, or hooks.
namespace cfgcompat {
struct ImageIdentity {
    const char* sha256;
    std::uint64_t bytes;
    std::uint32_t peTimestamp;
    std::uint32_t imageBytes;
};

constexpr bool SameSha256(const char* a,const char* b) noexcept {
    if(!a||!b)return false;
    for(std::size_t i=0;i<64;++i) {
        if(a[i]!=b[i])return false;
        if(a[i]=='\0')return false;
    }
    return a[64]=='\0'&&b[64]=='\0';
}

constexpr bool Matches(const ImageIdentity& expected,const char* sha256,
    std::uint64_t bytes,std::uint32_t peTimestamp,std::uint32_t imageBytes) noexcept {
    return SameSha256(expected.sha256,sha256)&&expected.bytes==bytes&&
        expected.peTimestamp==peTimestamp&&expected.imageBytes==imageBytes;
}

namespace cfg35 {
inline constexpr char Sha256[]="4FC4BB544747FFF85EA03DCB83DE0367AC2F5AF1AE5355614608C737E01A90A6";
inline constexpr ImageIdentity Identity{Sha256,53778432,0x6AD633AAu,0x03644000u};
inline constexpr bool RuntimeBindingsVerified=false;

namespace engine {
inline constexpr std::uintptr_t GlobalCandidateRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t EngineGameViewport=0x750,ViewportWorld=0x80;
inline constexpr std::uintptr_t WorldNetDriver=0x38,WorldGameMode=0xF0;
inline constexpr std::uintptr_t WorldGameState=0xF8,WorldGameInstance=0x140;
inline constexpr std::uintptr_t GameInstanceLocalPlayers=0x38;
inline constexpr std::uintptr_t PlayerController=0x30,ControllerPlayer=0x3D0;
inline constexpr std::uintptr_t ControllerAcknowledgedPawn=0x3E0,ControllerPawn=0x370;
inline constexpr std::uintptr_t ControllerCameraManager=0x400,ControllerSpectatorPawn=0x6A0;
inline constexpr std::uintptr_t CounselorStats=0x13F4,KillerTrapCount=0x1850;
}
namespace reflection {
inline constexpr std::uintptr_t ClassFactoryHelperRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ClassConstructorHelperRva=0x0/*CFG3.5 address withheld*/;
// Named StructProperty class registration + generated kind25 constructor,
// independently corroborated on the exact3.5 image. Not native-call authority.
inline constexpr std::uintptr_t StructPropertyVtable=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t StructPropertyType=0x70;
inline constexpr std::uintptr_t StructPropertyConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t StructPropertyClassGetter=0x0/*CFG3.5 address withheld*/;
}

// Cooked Blueprint evidence, not derivable from the native executable alone.
// Verified PID18212: Diagnostics/CFG35-SettingsSource-20261023-222453.txt.
// Runtime must still verify wrappers, graph, locals, ownership and identities.
namespace settingsStart {
inline constexpr std::int32_t StartEntry=0x2829,ConstructEntry=0x2834;
inline constexpr std::uint32_t StartDestination=0x26EC,ConstructDestination=0x25C3;
inline constexpr std::int32_t GraphBytes=10300,StorageBytes=969;
}
namespace worldMap {
// Exact3.5 disk proof: Inspect-Cfg35WorldMapBindings.ps1. Read-only schema
// facts only; runtime still checks all owners, identities, paths and metadata.
inline constexpr std::uintptr_t MapClass=0x0/*CFG3.5 address withheld*/,MapConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t WorldClass=0x0/*CFG3.5 address withheld*/,WorldConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t NativeClassCache=0x0/*CFG3.5 address withheld*/,NativePackageCache=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ClassGetter=0x0/*CFG3.5 address withheld*/,PackageGetter=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t FilePathCache=0x0/*CFG3.5 address withheld*/,FilePathFactory=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t FilePathDescriptor=0x0/*CFG3.5 address withheld*/,FilePathMember=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t FilePathHelper=0x0/*CFG3.5 address withheld*/;
inline constexpr unsigned Path=0x48,SnowMapPath=0x58,FilePathBytes=16,FilePathStringOffset=0;
}
namespace hunter {
// Named SCInteractableManagerComponent factory4F1100 and constructor4F01D0
// verified by Inspect-CfgHunterBindings.ps1 on the exact3.5 executable.
inline constexpr std::uintptr_t ManagerReflection=0x0/*CFG3.5 address withheld*/,ManagerConstructor=0x0/*CFG3.5 address withheld*/;
}
namespace startupEntry {
// Exact native call/tail-call chain on the supported image. These pins do not
// admit unknown callers or authorize any startup/birth native work by themselves.
inline constexpr std::uintptr_t IntroDispatcher=0x0/*CFG3.5 address withheld*/,IntroReturnPc=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t MatchBridge=0x0/*CFG3.5 address withheld*/,MatchReturnPc=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t MatchDispatcher=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ModeState=0x3F8,IntroState=0x0/*CFG3.5 address withheld*/,MatchState=0x0/*CFG3.5 address withheld*/;
}

namespace objects {
inline constexpr std::uintptr_t ObjectClass=0x10,ClassSuper=0x30;
struct NativeClassBinding { const char* name; std::uintptr_t reflectionCacheRva,constructorCacheRva; };
// These base-class cache pairs were resolved from the exact CFG 3.5 image
// using the verified class factory/constructor helpers. Keep generic feature
// code on this catalogue instead of carrying stale 3.4 cache RVAs.
inline constexpr std::uintptr_t SCGameModeReflection=0x0/*CFG3.5 address withheld*/,SCGameModeConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t SCGameStateReflection=0x0/*CFG3.5 address withheld*/,SCGameStateConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t SCMenuControllerReflection=0x0/*CFG3.5 address withheld*/,SCMenuControllerConstructor=0x0/*CFG3.5 address withheld*/;
// Disk-derived from the exact CFG 3.5 class factory chain; used by Jason's
// native movement-component observation rather than a duplicated 3.4 cache.
inline constexpr std::uintptr_t SCCharacterMovementReflection=0x0/*CFG3.5 address withheld*/,SCCharacterMovementConstructor=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<NativeClassBinding,9> NativeClasses{{
    {"SCCharacter",0x0304A290,0x0304A298},
    {"SCCounselorCharacter",0x0304B098,0x0304B0A0},
    {"SCKillerCharacter",0x0304DE10,0x0304DE18},
    {"SCPlayerController",0x0304ED20,0x0304ED28},
    {"SCGameMode",SCGameModeReflection,SCGameModeConstructor},
    {"SCGameState",SCGameStateReflection,SCGameStateConstructor},
    {"SCGameMode_OfflineBots",0x0304C348,0x0304C350},
    {"SCGameMode_Sandbox",0x0304C378,0x0304C380},
    {"SCGameMode_SPChallenges",0x0304C390,0x0304C398}
}};
}

// AI resource caches used by the bounded read-only roster snapshot. These are
// disk observations for the exact image, not permission to call native code.
namespace ai {
struct ClassBinding { const char* name; std::uintptr_t reflection,constructor; };
inline constexpr std::array<ClassBinding,8> Classes{{
    {"AIBlueprintHelperLibrary",0x032C1008,0x032C1010},
    {"AIController",0x032C10D0,0x032C10D8},
    {"SCCounselorAIController",0x0304AB78,0x0304AB80},
    {"SCKillerAIController",0x0304D8A0,0x0304D8A8},
    {"BehaviorTree",0x032C1700,0x032C1708},
    {"BrainComponent",0x032C1970,0x032C1978},
    {"PlayerState",0x032F3B88,0x032F3B90},
    {"GameStateBase",0x032ED470,0x032ED478}
}};
inline constexpr std::uintptr_t SpawnAIFromClassCacheRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ClassDefaultObject=0xF8;
inline constexpr std::uintptr_t NoCollisionFailSetterRva=0x0/*CFG3.5 address withheld*/;
}

// Jason pursuit/approach bindings resolved from the exact 3.5 image. Feature
// code consumes this version-owned catalogue through the legacy generated-name
// aliases, so the next game update changes one compatibility profile, not AI
// behavior code.
namespace jasonApproach {
struct ClassBinding { const char* name; std::uintptr_t reflection,constructor; };
inline constexpr std::array<ClassBinding,4> Classes{{
    {"Controller",0x032EB5D8,0x032EB5E0},
    {"Pawn",0x032F2D78,0x032F2D80},
    {"Actor",0x032E8C60,0x032E8C68},
    {"PathFollowingComponent",0x032C2758,0x032C2760}
}};
inline constexpr std::array<ClassBinding,2> AuxiliaryClasses{{
    {"AIController",0x032C10D0,0x032C10D8},
    {"ActorComponent",0x032E8E20,0x032E8E28}
}};
struct FunctionBinding { std::uintptr_t cache,factory; unsigned owner; std::array<unsigned char,47> prefix; };
// owner indices: 0=Controller, 1=Pawn, 2=Actor, 4=AuxiliaryClasses[0].
inline constexpr std::array<FunctionBinding,4> Functions{{
    {0x032EB5D0,0x01861170,0,{} /*CFG3.5 code signature withheld*/},
    {0x032F2D70,0x0190B7A0,1,{} /*CFG3.5 code signature withheld*/},
    {0x032E8A88,0x01828560,2,{} /*CFG3.5 code signature withheld*/},
    {0x032C1080,0x011B1A70,4,{} /*CFG3.5 code signature withheld*/}
}};
inline constexpr std::uintptr_t PathFollowing=0x410,ComponentOwner=0xE0;
inline constexpr std::uintptr_t ComponentOwnerCtorRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ComponentOwnerRegistrarRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ComponentOwnerNativeTableRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ComponentOwnerGetOwnerEntryRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ComponentOwnerGetOwnerThunkRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ComponentOwnerLoadRva=0x0/*CFG3.5 address withheld*/;
}

namespace primaryJason {
// #289 exact4FC UWorld::SpawnActor1516B90 -> validation1515C70.
// 1515D7A tests bit1 at UClass+B4; its branch logs26E0D90 "is abstract".
// 1515CED tests bit25 at the same field;26E0BD0 logs "is deprecated".
inline constexpr std::uintptr_t SelectedClassFlags=0xB4;
inline constexpr std::uint32_t UnspawnableSelectedClassFlags=0x0/*CFG3.5 address withheld*/;
// Exact-image factory/constructor decoding, 2026-10-23. The no-register
// factory is executable code, NOT the constructor's stored UClass pointer.
inline constexpr std::uintptr_t KismetReflection=0x0/*CFG3.5 address withheld*/,KismetConstructor=0x0/*CFG3.5 address withheld*/;
}
namespace softClassFrames {
// Exact named parameter descriptors on4FC4BB54..., 2026-10-24; corroborated by
// CFG35-SoftClassFrames-ParameterFlags.txt. Const-reference is NOT pure output.
inline constexpr std::uintptr_t ParameterCount=0x8C,ParameterBytes=0x8E;
inline constexpr std::uint64_t ConversionSourceFlags=0x0/*CFG3.5 address withheld*/08000182ULL;
inline constexpr std::uint64_t ConversionResultFlags=0x0/*CFG3.5 address withheld*/00000580ULL;
inline constexpr std::uint64_t PreloadClassFlags=0x0/*CFG3.5 address withheld*/00000080ULL;
inline constexpr std::uint64_t PreloadValueFlags=0x0/*CFG3.5 address withheld*/00000080ULL;
// Runtime UProperty flags are descriptor flags OR constructor contributions.
// Pinned-image builder7BB240 kinds13/17/18/25/26; constructor chain and latent
// native struct-ops computed flags verified by Inspect-CfgCounselorLoadBindings.
inline constexpr std::uint64_t SoftClassConstructorFlags=0x0/*CFG3.5 address withheld*/00000000ULL;
inline constexpr std::uint64_t ObjectConstructorFlags=0x0/*CFG3.5 address withheld*/40000200ULL;
inline constexpr std::uint64_t DelegateConstructorFlags=0x0000001000000200ULL;
inline constexpr std::uint64_t LatentConstructorFlags=0x0000001000000000ULL;
inline constexpr auto RuntimeConversionSourceFlags=ConversionSourceFlags|SoftClassConstructorFlags;
inline constexpr auto RuntimeConversionResultFlags=ConversionResultFlags|ObjectConstructorFlags;
inline constexpr auto RuntimePreloadClassFlags=PreloadClassFlags|SoftClassConstructorFlags;
inline constexpr auto RuntimePreloadWorldFlags=PreloadValueFlags|ObjectConstructorFlags;
inline constexpr auto RuntimePreloadDelegateFlags=PreloadValueFlags|DelegateConstructorFlags;
inline constexpr auto RuntimePreloadLatentFlags=PreloadValueFlags|LatentConstructorFlags;
}

namespace names {
inline constexpr std::uintptr_t GlobalRva=0x0/*CFG3.5 address withheld*/,GetterRva=0x0/*CFG3.5 address withheld*/,ObjectName=0x18;
inline constexpr unsigned MaxChunks=128,ChunkSize=16384,PointerStride=8;
inline constexpr unsigned ElementCount=0x400,ChunkCount=0x404,EntryText=0x10,EntryWideMask=1;
inline constexpr unsigned NameIndex=0,NameNumber=4,NameBytes=8;
inline constexpr std::array<unsigned char,101> GetterSignature{} /*CFG3.5 code signature withheld*/;
}

namespace objectArray {
inline constexpr std::uintptr_t GlobalRva=0x0/*CFG3.5 address withheld*/,ObjectIndex=0x0C;
inline constexpr std::uintptr_t Data=0x10,Count=0x1C,ItemBytes=24,Object=0,Serial=0x10;
}

namespace dispatch {
inline constexpr std::uintptr_t ThreadFlagRva=0x0/*CFG3.5 address withheld*/,ThreadIdRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ReflectedEventSlotBytes=0x1F8;
inline constexpr std::uintptr_t NativeActorDispatcherRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GenericDispatcherRva=0x0/*CFG3.5 address withheld*/;
// UFunction::Bind's non-native branch installs this script executor at +B0.
// This is NOT UObject::ProcessEvent / the generic dispatcher above.
inline constexpr std::uintptr_t BlueprintExecutorRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<unsigned char,19> GenericDispatcherPrefix{{0x40,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x81,0xEC,0xF0,0,0,0}};
}

namespace gcRetention {
// Exact-image engine reference-owner route. Per-operation runtime admission
// remains disabled until synchronization and callback lifetime are verified.
inline constexpr bool RuntimeVerified=false;
// PID8676, CFG35-GCRegistry-20261023-031942.txt: one pinned live counselor
// owner registered once, with 8 native collector callbacks / 8 submissions.
// This permits only the isolated permanent-input spawn candidate's guards;
// it does not verify mutable owners, removal, or general gameplay lookup.
inline constexpr bool PermanentOwnerCallbackObserved=true;
inline constexpr std::uintptr_t ReferencerGlobalRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ReferencerClassRva=0x0/*CFG3.5 address withheld*/,ReferencerClassCacheRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t RegisterRva=0x0/*CFG3.5 address withheld*/,UnregisterRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t EnumerateRva=0x0/*CFG3.5 address withheld*/,FixedSubmitRva=0x0/*CFG3.5 address withheld*/;
// Engine hash/collector lock, NOT the referencer's separate owner-list lock.
// Static ABI pins only; holding it across allocating gameplay is not approved.
inline constexpr std::uintptr_t HashLockGetterRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t HashLockAcquireRva=0x0/*CFG3.5 address withheld*/,HashLockReleaseRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t HashLockGlobalRva=0x0/*CFG3.5 address withheld*/,HashLockInitGuardRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t RegistryAddUniqueRva=0x0/*CFG3.5 address withheld*/,RegistryRemoveRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t OwnerArray=0x28,OwnerCount=0x30,OwnerListLock=0x38;
inline constexpr std::uintptr_t OwnerCollectSlot=0x08,CollectorSubmitSlot=0x38;
// Exact-image synchronous slot handlers. These identify two implementations,
// NOT all collector implementations or a general worker-join/mutation contract.
inline constexpr std::uintptr_t ParallelSubmitRva=0x0/*CFG3.5 address withheld*/,SerialSubmitRva=0x0/*CFG3.5 address withheld*/;
}

namespace spawn {
inline constexpr std::uintptr_t LocationFunctionCacheRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ActorClassReflectionCacheRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ModeCounselorTree=0x9C8;
}
namespace functions {
struct ReflectedFunctionBinding {
    const char* owner;
    const char* name;
    std::uintptr_t ownerFactoryRva,factoryRva,descriptorRva,cacheRva;
    std::uint16_t parameterBytes;
};
// Exact CFG 3.5 native /Script/Engine.Actor:ReceiveTick disk catalogue entry.
// This identifies the callback UFunction; it is not native-call authority.
inline constexpr ReflectedFunctionBinding ActorReceiveTick{
    "/Script/Engine.Actor","ReceiveTick",0x01827600,0x01828CB0,0x02E967A0,0x032E8BC0,4};
// The observed Blueprint-call wrapper is distinct from the base Actor tick
// function. Keep its exact extent-return PC, name-global operand, and code bytes
// in the same version profile used by callback admission.
inline constexpr std::uintptr_t ReceiveTickWrapperRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ReceiveTickReturnPcRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ReceiveTickNameGlobalRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<unsigned char,62> ReceiveTickWrapperCode{{
    0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x48,0x8B,0x15,0x57,0x14,0xAC,
    0x01,0x48,0x8B,0xF9,0x48,0x8B,0x19,0xF3,0x0F,0x11,0x4C,0x24,0x38,0xE8,0xAE,0x18,
    0xF8,0xFE,0x48,0x8B,0xD0,0x4C,0x8D,0x44,0x24,0x38,0x48,0x8B,0xCF,0xFF,0x93,0xF8,
    0x01,0x00,0x00,0x48,0x8B,0x5C,0x24,0x30,0x48,0x83,0xC4,0x20,0x5F,0xC3
}};
}
namespace menuStack {
// Named ILLMenuStackHUDComponent registration and complete previous-class
// executor/leaf proof: Inspect-CfgMenuStackParent.ps1, exact CFG 3.5 image.
inline constexpr std::uintptr_t OwnerRva=0x0/*CFG3.5 address withheld*/,ConstructorRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t PreviousExecutorRva=0x0/*CFG3.5 address withheld*/,PreviousHelperRva=0x0/*CFG3.5 address withheld*/;
}
namespace constructBoundary {
// Exact UFunction native-invoke body saves/sets/restores FFrame+88 around
// call [UFunction+B0]. These are provenance facts, not a generic GC exemption.
inline constexpr std::uintptr_t InvokeRva=0x0/*CFG3.5 address withheld*/,ReturnRva=0x0/*CFG3.5 address withheld*/;
}
namespace lookup {
inline constexpr std::uintptr_t SerializationFlagRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GarbageCollectionStateRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t SerializationGuardRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GarbageCollectionGuardRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<unsigned char,22> SerializationGuardPrefix{} /*CFG3.5 code signature withheld*/;
inline constexpr std::array<unsigned char,12> GarbageCollectionGuardPrefix{} /*CFG3.5 code signature withheld*/;
// Static rejection operands are known. Their complete writer/lifecycle,
// aliasing and synchronization contracts are not established.
inline constexpr bool RuntimeLookupEligibilityVerified=false;
}
namespace botName {
// Exact image constructor/vtable and complete deep-copy chain, verified by
// Inspect-CfgBotNameBindings.ps1. Module-relative, not live pointers.
inline constexpr std::uintptr_t SetterRva=0x0/*CFG3.5 address withheld*/,BaseSetterRva=0x0/*CFG3.5 address withheld*/,StringCopyRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t SetterSlot=0x628,PlayerNameOffset=0x370;
inline constexpr std::uintptr_t StateClassRva=0x0/*CFG3.5 address withheld*/,StateConstructorRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GameStateClassRva=0x0/*CFG3.5 address withheld*/,GameStateConstructorRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GameStateTableRva=0x0/*CFG3.5 address withheld*/;
}
namespace toolsStats {
// Named SCCounselorCharacter Stats property and its native GetStatAlpha
inline constexpr std::uintptr_t Offset=0x13F4,DescriptorRva=0x0/*CFG3.5 address withheld*/,NameRva=0x0/*CFG3.5 address withheld*/,IndexedLoadRva=0x0/*CFG3.5 address withheld*/;
}
} // namespace cfg35

// Historical 3.4 comparison values were recovered from repository baseline
// 43ab1c3 and cross-checked against 3.5 stale-binding rejection assertions.
// This is a migration record only; 3.4 is NOT a runtime-supported profile.
namespace cfg34History {
inline constexpr char Sha256[]="53B91EA6BC2D9EC69F5AAF41EB67ECB4E5393E71F8AD4B3276F2D74BC27E4BC6";
inline constexpr std::uint64_t ExecutableBytes=53770752;
inline constexpr bool RuntimeBindingsVerified=false;
namespace engine { inline constexpr std::uintptr_t GlobalCandidateRva=0x0/*CFG3.5 address withheld*/; }
namespace reflection { inline constexpr std::uintptr_t ClassFactoryHelperRva=0x0/*CFG3.5 address withheld*/; }
namespace names {
inline constexpr std::uintptr_t GlobalRva=0x0/*CFG3.5 address withheld*/,GetterRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<unsigned char,101> GetterSignature{} /*CFG3.5 code signature withheld*/;
}
namespace objectArray { inline constexpr std::uintptr_t GlobalRva=0x0/*CFG3.5 address withheld*/; }
namespace dispatch {
inline constexpr std::uintptr_t ThreadFlagRva=0x0/*CFG3.5 address withheld*/,ThreadIdRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t NativeActorDispatcherRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t GenericDispatcherRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::array<unsigned char,19> GenericDispatcherPrefix{{0x40,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x81,0xEC,0xF0,0,0,0}};
}
namespace spawn {
inline constexpr std::uintptr_t LocationFunctionCacheRva=0x0/*CFG3.5 address withheld*/;
inline constexpr std::uintptr_t ActorClassReflectionCacheRva=0x0/*CFG3.5 address withheld*/;
}
namespace objects {
inline constexpr std::array<cfg35::objects::NativeClassBinding,7> NativeClasses{{
    {"SCCharacter",0x03048E50,0x03048E58},
    {"SCCounselorCharacter",0x03049C58,0x03049C60},
    {"SCKillerCharacter",0x0304C9D0,0x0304C9D8},
    {"SCPlayerController",0x0304D8E0,0x0304D8E8},
    {"SCGameMode_OfflineBots",0x0304AF08,0x0304AF10},
    {"SCGameMode_Sandbox",0x0304AF38,0x0304AF40},
    {"SCGameMode_SPChallenges",0x0304AF50,0x0304AF58}
}};
}
} // namespace cfg34History

enum class Build : std::uint8_t { Unknown, CFG35 };
struct Selection {
    Build build=Build::Unknown;
    const ImageIdentity* identity=nullptr;
    bool identified=false;
    bool runtimeEligible=false;
};
constexpr Selection SelectProfile(const char* sha256,std::uint64_t bytes,
    std::uint32_t peTimestamp,std::uint32_t imageBytes) noexcept {
    if(!Matches(cfg35::Identity,sha256,bytes,peTimestamp,imageBytes))return {};
    return {Build::CFG35,&cfg35::Identity,true,cfg35::RuntimeBindingsVerified};
}
} // namespace cfgcompat
