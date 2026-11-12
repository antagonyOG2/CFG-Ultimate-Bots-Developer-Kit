#pragma once
#include "ObserverProtocol.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include <type_traits>
namespace cfgobserver {
struct LocalPlayers {
    std::uintptr_t data=0;
    std::int32_t count=0,capacity=0;
};
static_assert(sizeof(LocalPlayers)==16,"Corroborated x64 local-player array ABI");
inline bool SanePlan(const Plan& p) noexcept {
    const std::array<std::uintptr_t,10> pointers{p.module,p.engine,p.viewport,p.world,p.mode,
        p.controller,p.pawn,p.player,p.gameInstance,p.localPlayerData};
    if (p.magic!=Magic || p.version!=Version || p.bytes!=sizeof(Plan) || !p.threadId ||
        !p.objectArray.Sane() || p.dispatch.flag!=1 || p.dispatch.threadId!=p.threadId ||
        !cfgprobe::ObjectPointer(p.dispatch.vtable)) return false;
    for (const auto value:pointers) if (!cfgprobe::ObjectPointer(value)) return false;
    if (!cfgprobe::lifetime::Extent(p.engine,cfgprobe::EngineGameViewport+8) ||
        !cfgprobe::lifetime::Extent(p.viewport,cfgprobe::ViewportWorld+8) ||
        !cfgprobe::lifetime::Extent(p.world,cfgprobe::WorldGameInstance+8) ||
        !cfgprobe::lifetime::Extent(p.gameInstance,cfgprobe::GameInstanceLocalPlayers+16) ||
        !cfgprobe::lifetime::Extent(p.player,cfgprobe::PlayerController+8) ||
        !cfgprobe::lifetime::Extent(p.controller,cfgprobe::ControllerAcknowledgedPawn+8) ||
        !cfgprobe::lifetime::Extent(p.localPlayerData,8)) return false;
    if (!cfgprobe::lifetime::Extent(p.module,cfgprobe::ExecutableImageBytes) ||
        p.dispatch.target!=p.module+cfgprobe::dispatch::NativeActorDispatcherRva) return false;
    const std::array<std::uintptr_t,4> objects{p.world,p.mode,p.controller,p.pawn};
    for (std::size_t i=0;i<objects.size();++i) {
        if (!p.objects[i].HasAssignedSerial() || p.objects[i].object!=objects[i] ||
            p.objects[i].index>=p.objectArray.count) return false;
        for (std::size_t j=0;j<i;++j)
            if (p.objects[j].object==p.objects[i].object || p.objects[j].index==p.objects[i].index) return false;
    }
#ifdef CFG_FRONTEND_BUNDLE
    if(p.frontend.widget&&(p.frontend.module!=p.module||p.frontend.world!=p.world||p.frontend.mode!=p.mode||
        p.frontend.controller!=p.controller||p.frontend.hud!=p.pawn||p.frontend.thread!=p.threadId||
        p.frontend.engine!=p.engine||p.frontend.viewport!=p.viewport||p.frontend.player!=p.player||
        p.frontend.gameInstance!=p.gameInstance||p.frontend.players!=p.localPlayerData))return false;
#endif
    return true;
}
template<class Reader> bool Same(Reader& r,std::uintptr_t address,std::uintptr_t expected) {
    std::uintptr_t actual=0; return r.Read(address,actual) && actual==expected;
}
// This helper always reads a pointer-sized field. Refuse silent scalar
// promotion (notably uint8_t flags/Role and float speeds). Null pointer edges
// must spell std::uintptr_t{}; scalar fields use a width-preserving helper.
template<class Reader,class T,std::enable_if_t<!std::is_same_v<std::decay_t<T>,std::uintptr_t>,int> =0>
bool Same(Reader&,std::uintptr_t,T)=delete;
enum class ContextFailure : std::uint32_t { None, InvalidPlan, EngineGlobal, EngineViewport,
    ViewportWorld, WorldNetDriver, WorldGameMode, WorldGameInstance, LocalPlayers,
    LocalPlayerEntry, PlayerController, ControllerPlayer, AcknowledgedPawn, ControllerPawn,
    ObjectRegistry, Dispatcher, Frontend };
// READ-ONLY, fixed known context only: 30 reads on the passing path. The caller's
// reader imposes a hard cap. No global actor/object search, names, asset loads,
// native calls, background worker, refresh of ownership or replacement adoption.
// Prep must separately classify Sandbox/counselor/controller and pin the build.
// Matching sampled serials are not a GC pin or approval to call gameplay APIs.
#ifdef CFG_FRONTEND_BUNDLE
enum class FrontendContextStage : unsigned { None, Plan, ActiveMenu, OwnerSlots, Dispatch };
inline const char* FrontendContextStageName(FrontendContextStage stage) noexcept {
    switch(stage){
    case FrontendContextStage::None:return "none";
    case FrontendContextStage::Plan:return "plan";
    case FrontendContextStage::ActiveMenu:return "active_menu";
    case FrontendContextStage::OwnerSlots:return "owner_slots";
    case FrontendContextStage::Dispatch:return "dispatcher";
    }
    return "unknown";
}
// Frontend allocations may append registry entries between capture and loading.
// Accept only growth in the SAME storage, retaining every exact positive owner
// identity. This does not adopt a new owner, storage, widget, stack or dispatcher.
template<class Reader> bool FrontendOwnersCurrent(Reader& r,const Plan& p) {
    using namespace cfgprobe::lifetime;
    ArrayHeader fresh,end;
    const auto array=p.module+ObjectArrayRva;
    if(!p.objectArray.Sane() || !ReadArrayHeader(r,array,fresh) ||
        fresh.data!=p.objectArray.data || fresh.count<p.objectArray.count)return false;
    for(const auto& expected:p.objects){
        SlotObservation actual;
        if(!expected.HasAssignedSerial() || !ReadSlot(r,fresh,expected.object,actual) ||
            !(actual==expected))return false;
    }
    return ReadArrayHeader(r,array,end) && end.data==fresh.data && end.count>=fresh.count;
}
template<class Reader> bool FrontendContextCurrent(Reader& r,const Plan& p,
    FrontendContextStage* failure=nullptr) {
    const auto fail=[&](FrontendContextStage stage){if(failure)*failure=stage;return false;};
    if(failure)*failure=FrontendContextStage::None;
    if(!p.frontend.widget || !SanePlan(p))return fail(FrontendContextStage::Plan);
    if(!cfgmenu::active::Current(r,p.frontend,p.threadId))return fail(FrontendContextStage::ActiveMenu);
    if(!FrontendOwnersCurrent(r,p))return fail(FrontendContextStage::OwnerSlots);
    if(!cfgprobe::dispatch::Revalidate(r,p.module,p.pawn,p.dispatch))return fail(FrontendContextStage::Dispatch);
    return true;
}
#endif
template<class Reader> ContextFailure DiagnoseContextFailure(Reader& r,const Plan& p) {
    using namespace cfgprobe;
#ifdef CFG_FRONTEND_BUNDLE
    if(p.frontend.widget)return FrontendContextCurrent(r,p)?ContextFailure::None:ContextFailure::Frontend;
#endif
    if(!SanePlan(p))return ContextFailure::InvalidPlan;
    if(!Same(r,p.module+EngineGlobalCandidate,p.engine))return ContextFailure::EngineGlobal;
    if(!Same(r,p.engine+EngineGameViewport,p.viewport))return ContextFailure::EngineViewport;
    if(!Same(r,p.viewport+ViewportWorld,p.world))return ContextFailure::ViewportWorld;
    if(!Same(r,p.world+WorldNetDriver,std::uintptr_t{}))return ContextFailure::WorldNetDriver;
    if(!Same(r,p.world+WorldGameMode,p.mode))return ContextFailure::WorldGameMode;
    if(!Same(r,p.world+WorldGameInstance,p.gameInstance))return ContextFailure::WorldGameInstance;
    LocalPlayers players;
    if(!r.Read(p.gameInstance+GameInstanceLocalPlayers,players)||players.data!=p.localPlayerData||
       players.count!=1||players.capacity<1||players.capacity>1024)return ContextFailure::LocalPlayers;
    if(!Same(r,players.data,p.player))return ContextFailure::LocalPlayerEntry;
    if(!Same(r,p.player+PlayerController,p.controller))return ContextFailure::PlayerController;
    if(!Same(r,p.controller+ControllerPlayer,p.player))return ContextFailure::ControllerPlayer;
    if(!Same(r,p.controller+ControllerAcknowledgedPawn,p.pawn))return ContextFailure::AcknowledgedPawn;
    if(!Same(r,p.controller+ControllerPawn,p.pawn))return ContextFailure::ControllerPawn;
    if(!lifetime::RevalidateBatch(r,p.module+lifetime::ObjectArrayRva,p.objectArray,p.objects))return ContextFailure::ObjectRegistry;
    if(!dispatch::Revalidate(r,p.module,p.pawn,p.dispatch))return ContextFailure::Dispatcher;
    return ContextFailure::None;
}
template<class Reader> bool ContextUnchanged(Reader& r,const Plan& p) {
    return DiagnoseContextFailure(r,p)==ContextFailure::None;
}
} // namespace cfgobserver
