#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
#include "../RuntimeProbe/DispatchObservation.hpp"
#include "../Core/CallbackObservationPolicy.hpp"
#include <type_traits>
#ifdef CFG_FRONTEND_BUNDLE
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#endif

namespace cfgobserver {
// Aggregate one-shot frontend preparation only, not recurring gameplay work.
inline constexpr unsigned FrontendPreparationReadLimit=32768;
constexpr std::uint32_t Magic=0x4346474F;
#if defined(CFG_FRONTEND_BUNDLE)
constexpr std::uint32_t Version=11;
#elif defined(CFG_GAMEPLAY_BUNDLE)
constexpr std::uint32_t Version=10;
#elif defined(CFG_PRIMARY_JASON)
#ifdef CFG_RETAINED_PRIMARY_JASON
constexpr std::uint32_t Version=15;
#else
constexpr std::uint32_t Version=8;
#endif
#elif defined(CFG_JASON_APPROACH)
constexpr std::uint32_t Version=7;
#elif defined(CFG_RETAINED_COUNSELOR_SPAWN)
constexpr std::uint32_t Version=14;
#elif defined(CFG_SPAWN_ADAPTER)
constexpr std::uint32_t Version=6;
#elif defined(CFG_EMOTE_TRACE)
constexpr std::uint32_t Version=5;
#elif defined(CFG_GC_REGISTRY_PROBE)
#ifdef CFG_SYNCHRONIZED_ROOT_PROBE
constexpr std::uint32_t Version=16;
#else
constexpr std::uint32_t Version=12;
#endif
#else
constexpr std::uint32_t Version=4;
#endif
struct Plan {
    std::uint32_t magic=Magic,version=Version,bytes=0,threadId=0;
    std::uintptr_t module=0,engine=0,viewport=0,world=0,mode=0,controller=0,pawn=0,player=0;
    std::uintptr_t gameInstance=0,localPlayerData=0;
    cfgprobe::lifetime::ArrayHeader objectArray{};
    cfgprobe::dispatch::Observation dispatch{};
    std::array<cfgprobe::lifetime::SlotObservation,4> objects{};
#ifdef CFG_FRONTEND_BUNDLE
    cfgmenu::active::Context frontend{}; // Initial menu transport only; fresh gameplay plan leaves this empty.
#endif
};
enum class Status : std::uint32_t { NotStarted, Preparing, Capturing, Complete, Stopped,
    RefusedPlan, RefusedIdentity, RefusedInstrumentation, RefusedHook, Prepared, Indeterminate };
enum class PreparationStage : std::uint32_t { None, Input, Build, Thread, ModeClass,
    ControllerClass, PawnClass, ContextBefore, Trampoline, ContextAfter, Capture, ModulePin, EmoteBindings, SpawnBindings };
#ifdef CFG_SPAWN_ADAPTER
constexpr auto LastPreparationStage=PreparationStage::SpawnBindings;
#else
constexpr auto LastPreparationStage=PreparationStage::EmoteBindings;
#endif
struct Report {
    std::uint32_t magic=Magic,version=Version,bytes=0,status=static_cast<std::uint32_t>(Status::NotStarted);
    std::uint32_t publishedCount=0,stopReason=0,threadId=0,hookInstalled=0;
    std::int32_t hookStatus=0;
    std::uint32_t preparationStage=0;
    std::uintptr_t pawn=0,target=0;
    std::uintptr_t trampoline=0,relay=0,detour=0;
    std::array<unsigned char,5> desiredPatch{};
    std::uint32_t preparationReads=0;
    std::uint32_t dllPinned=0;
    std::uint32_t preparationError=0;
    // ContextFailure diagnostic for a CFG_GC_REGISTRY_PROBE preparation
    // refusal; zero means no additional context diagnosis was recorded.
    std::uint32_t preparationContextFailure=0;
    std::array<cfgport::callbacks::Event,cfgport::callbacks::Capacity> events{};
};
static_assert(std::is_trivially_copyable<Plan>::value && std::is_trivially_copyable<Report>::value,"Fixed transfer protocol");
static_assert(sizeof(Plan)<4096 && sizeof(Report)<4096,"Bounded transfer only");
} // namespace cfgobserver
