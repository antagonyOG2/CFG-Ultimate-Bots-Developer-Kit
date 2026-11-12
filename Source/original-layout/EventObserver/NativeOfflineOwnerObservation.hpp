#pragma once
#include "NativeCounselorPromotion.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/FreshGameplayObservation.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include "../RuntimeProbe/OfflineIntroMetadataBindings.generated.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include "../RuntimeProbe/RadarWorldBindings.generated.hpp"
#include "../Core/EndingMetadataPolicy.hpp"
#include "../Core/NativeStartupDiagnostic.hpp"
#include "../Core/NativeSpawnedClassCache.hpp"
namespace cfgmenu::offlineowner {
constexpr unsigned ReadLimit=8192;
enum class Purpose {Runtime,DiagnosticReadOnly,TypedOwnValueOptions};
template<class Reader> class Budget {
    Reader& r_;
public:
    unsigned reads=0;explicit Budget(Reader& r):r_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads>=ReadLimit)return false;++reads;return r_.Read(at,out);}
};
template<class Reader,class T> bool Same(Reader& r,std::uintptr_t at,const T& expected){T v{};return r.Read(at,v)&&v==expected;}
template<class Reader> bool LookupSafe(Reader& r,std::uintptr_t module,Purpose purpose=Purpose::Runtime){
    // Diagnostics and verified, nonallocating typed own-value CAS grant NO
    // native-call capability. All other runtime consumers
    // still require the independently proved scoped root/lookup context.
    if(purpose==Purpose::DiagnosticReadOnly||(CFG_J214_HUMAN_JASON_OPTIONS&&purpose==Purpose::TypedOwnValueOptions))
        return cfgprimary::LookupVetoClearForObservation(r,module);
    return cfgprimary::LookupSafe(r,module);
}
// J28: Challenges runtimes never enter a scoped-lookup bundle (they add no new native call);
// 3.5 cfgprimary::LookupSafe is therefore always false for them. Use the read-only GC/serialization veto.
template<class Reader> bool ChallengeLookupSafe(Reader& r,std::uintptr_t module){
#if defined(CFG_J28_CHALLENGE_35_BINDINGS) && CFG_J28_CHALLENGE_35_BINDINGS
    return cfgprimary::LookupVetoClearForObservation(r,module);
#else
    return LookupSafe(r,module);
#endif
}
template<class Reader> bool Registry(Reader& r,const promotion::Context& c,cfgprobe::lifetime::ArrayHeader& current,Purpose purpose=Purpose::Runtime){
    return LookupSafe(r,c.module,purpose)&&cfgprobe::lifetime::ReadArrayHeader(r,c.module+cfgprobe::lifetime::ObjectArrayRva,current)&&
        cfgprobe::lifetime::RevalidateBatch(r,c.module+cfgprobe::lifetime::ObjectArrayRva,current,c.identities)&&LookupSafe(r,c.module,purpose);
}
template<class Reader> bool ExactType(Reader& r,std::uintptr_t module,std::uintptr_t object,std::uintptr_t cache){
    std::uintptr_t a=0,b=0,defaults=0;
    if(!cfgprobe::ObjectPointer(object)||!r.Read(module+cache,a)||!r.Read(module+cache+8,b)||
       !cfgprobe::MatchingClassCache(a,b)||!Same(r,object+cfgprobe::ObjectClass,a)||
       !r.Read(a+0xF8,defaults)||!cfgprobe::ObjectPointer(defaults)||defaults==object||
       !Same(r,defaults+cfgprobe::ObjectClass,a))return false;
    return Same(r,module+cache,a)&&Same(r,module+cache+8,a)&&Same(r,a+0xF8,defaults)&&Same(r,object+cfgprobe::ObjectClass,a);
}
// Read-only native ownership revalidation, including the mode's typed Level
// Outer -> OwningWorld backlink. No pawn is read or required here.
template<class Reader> bool Current(Reader& r,const promotion::Context& c,std::uint32_t callerThread,unsigned* stage=nullptr,Purpose purpose=Purpose::Runtime){
    const auto mark=[&](unsigned value){if(stage)*stage=value;};mark(1);
    using namespace cfgprobe;namespace d=offlinelifecycledisk;namespace i=intrometadatadisk;
    if(!c.exactImageVerified||!callerThread||callerThread!=c.thread||!c.registry.Sane())return false;
    const std::array<std::uintptr_t,10> pointers{{c.module,c.engine,c.viewport,c.world,c.gameInstance,c.localPlayers,c.player,c.mode,c.controller,c.playerState}};
    for(auto p:pointers)if(!ObjectPointer(p))return false;
    const std::array<std::uintptr_t,4> objects{{c.world,c.mode,c.controller,c.playerState}};
    for(unsigned n=0;n<objects.size();++n){if(!c.identities[n].HasAssignedSerial()||c.identities[n].object!=objects[n])return false;
        for(unsigned j=0;j<n;++j)if(objects[n]==objects[j]||c.identities[n].index==c.identities[j].index)return false;}
    const auto links=[&](){int count=0,cap=0;std::uint32_t flags=0;
        return Same(r,c.module+dispatch::ThreadIdRva,callerThread)&&Same(r,c.module+dispatch::ThreadFlagRva,std::uint8_t{1})&&
            Same(r,c.module+EngineGlobalCandidate,c.engine)&&Same(r,c.engine+EngineGameViewport,c.viewport)&&Same(r,c.viewport+ViewportWorld,c.world)&&
            Same(r,c.world+WorldNetDriver,std::uintptr_t{0})&&Same(r,c.world+WorldGameMode,c.mode)&&Same(r,c.world+WorldGameInstance,c.gameInstance)&&
            Same(r,c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)&&r.Read(c.gameInstance+GameInstanceLocalPlayers+8,count)&&count==1&&
            r.Read(c.gameInstance+GameInstanceLocalPlayers+12,cap)&&cap>=1&&cap<=1024&&Same(r,c.localPlayers,c.player)&&
            Same(r,c.player+PlayerController,c.controller)&&Same(r,c.controller+ControllerPlayer,c.player)&&
            Same(r,c.controller+0x388,c.playerState)&&Same(r,c.playerState+0x88,c.controller)&&
            Same(r,c.mode,c.module+d::OfflineBotsVtable)&&Same(r,c.playerState+0x110,std::uint8_t{3})&&
            r.Read(c.playerState+0x394,flags)&&!(flags&6);
    };
    lifetime::ArrayHeader currentRegistry{};
    mark(2);if(!LookupSafe(r,c.module,purpose)||!links())return false;
    mark(3);if(!Registry(r,c,currentRegistry,purpose))return false;
    mark(4);if(!ExactType(r,c.module,c.mode,d::OfflineBotsClass)||!ExactType(r,c.module,c.controller,i::SCPlayerController_OfflineBotsClass)||
       !ExactType(r,c.module,c.playerState,i::SCPlayerState_OfflineBotsClass))return false;
    std::uintptr_t level=0;cfgprobe::lifetime::SlotObservation slot{},again{};
    mark(5);if(!r.Read(c.mode+0x20,level))return false;
    mark(6);if(!lifetime::ReadSlot(r,currentRegistry,level,slot))return false;
    mark(7);if(!ExactType(r,c.module,level,cfgkill::disk::LevelReflection)||!Same(r,level+cfgkill::disk::OwningWorld,c.world))return false;
    for(const auto& owner:c.identities)if(owner.object==level||owner.index==slot.index)return false;
    // Level is sampled only within this callback, never retained as a weak
    // identity. Require a strong current World.Levels cell, including when
    // its serial is still zero. Keep positive serials for all retained owners.
    struct Levels {std::uintptr_t data=0;int count=0,capacity=0;} levels{},freshLevels{};
    mark(8);
    if(!r.Read(c.world+cfgradarworld::disk::WorldLevels,levels)||!ObjectPointer(levels.data)||levels.count<1||levels.count>256||
       levels.capacity<levels.count||levels.capacity>4096)return false;
    std::uintptr_t cell=0;
    for(int n=0;n<levels.count;++n){std::uintptr_t value=0;if(!r.Read(levels.data+8*n,value))return false;
        if(value==level){cell=levels.data+8*n;break;}}
    if(!cell)return false;
    mark(9);
    const bool okay=links()&&Same(r,c.mode+0x20,level)&&Same(r,level+cfgkill::disk::OwningWorld,c.world)&&
        r.Read(c.world+cfgradarworld::disk::WorldLevels,freshLevels)&&freshLevels.data==levels.data&&freshLevels.count==levels.count&&
        freshLevels.capacity==levels.capacity&&Same(r,cell,level)&&
        lifetime::ReadSlot(r,currentRegistry,level,again)&&again==slot&&
        ExactType(r,c.module,c.mode,d::OfflineBotsClass)&&ExactType(r,c.module,c.controller,i::SCPlayerController_OfflineBotsClass)&&
        ExactType(r,c.module,c.playerState,i::SCPlayerState_OfflineBotsClass)&&
        ExactType(r,c.module,level,cfgkill::disk::LevelReflection)&&
        Registry(r,c,currentRegistry,purpose);
    if(okay)mark(0);return okay;
}
template<class Reader> bool Observe(Reader& reader,const freshgame::Source& source,std::uintptr_t receiver,
    std::uint32_t callerThread,bool exactImageVerified,promotion::Context& output,unsigned* consumed=nullptr,unsigned* stage=nullptr,Purpose purpose=Purpose::Runtime){
    if(stage)*stage=10;
    output={};Budget<Reader> r(reader);promotion::Context c{};
    const auto finish=[&](bool result){if(consumed)*consumed=r.reads;return result;};
    using namespace cfgprobe;
    if(!exactImageVerified||!callerThread||callerThread!=source.thread||!ObjectPointer(receiver)||!ObjectPointer(source.frontendWorld))return finish(false);
    c.module=source.module;c.thread=callerThread;c.exactImageVerified=true;c.mode=receiver;
    if(!ObjectPointer(c.module)||!r.Read(c.module+EngineGlobalCandidate,c.engine)||c.engine!=source.engine||
       !r.Read(c.engine+EngineGameViewport,c.viewport)||c.viewport!=source.viewport||
       !r.Read(c.viewport+ViewportWorld,c.world)||c.world==source.frontendWorld||!ObjectPointer(c.world)||
       !r.Read(c.world+WorldGameInstance,c.gameInstance)||c.gameInstance!=source.gameInstance||
       !r.Read(c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)||!ObjectPointer(c.localPlayers)||
       !r.Read(c.localPlayers,c.player)||c.player!=source.player||
       !r.Read(c.player+PlayerController,c.controller)||!ObjectPointer(c.controller)||
       !r.Read(c.controller+0x388,c.playerState)||!ObjectPointer(c.playerState))return finish(false);
    const std::array<std::uintptr_t,4> objects{{c.world,c.mode,c.controller,c.playerState}};
    c.modeVtable=c.module+offlinelifecycledisk::OfflineBotsVtable;
    c.restartGate=c.module+offlinelifecycledisk::OfflineBotsRestartPlayer;
    if(!LookupSafe(r,c.module,purpose)||!lifetime::ObserveBatch(r,c.module+lifetime::ObjectArrayRva,objects,c.registry,c.identities)||!Current(r,c,callerThread,stage,purpose))return finish(false);
    output=c;return finish(true);
}
struct Human {
    std::uintptr_t pawn=0,type=0;
    cfgprobe::lifetime::SlotObservation identity{};
    promotion::Packet active{};
};
// Verify only after native Restart returns. Selected-class association uses
// the exact fresh loaded class and plain asset path, never a Sandbox guard or
// empty Active fallback. SpawnedClass is an optional cache, never authority;
// if present it must match. Output is published only after revalidation.
template<class Reader> bool VerifyHuman(Reader& reader,const promotion::Context& c,
    const promotion::Packet& expectedPlainCounselor,std::uintptr_t expectedLoadedClass,
    std::uint32_t callerThread,Human& output,unsigned* consumed=nullptr,NativeHumanDiagnostic* diagnostic=nullptr){
    output={};Budget<Reader> r(reader);Human h{};
    NativeHumanDiagnostic local{};auto& diag=diagnostic?*diagnostic:local;
    const auto mark=[&](unsigned stage){diag.stage=stage;};
    const auto finish=[&](bool result){diag.reads=r.reads;if(consumed)*consumed=r.reads;return result;};
    using namespace cfgprobe;
    mark(1);if(!cfgending::EmptySubpath(expectedPlainCounselor.bytes)||!ObjectPointer(expectedLoadedClass)||!Current(r,c,callerThread,&diag.ownerStage))return finish(false);
    std::uintptr_t counselor=0,killer=0,defaults=0;lifetime::ArrayHeader currentRegistry{};
    mark(2);if(!Registry(r,c,currentRegistry))return finish(false);
    mark(3);
    if(!r.Read(c.module+birthdisk::SCCounselorCharacterClass,counselor)||!Same(r,c.module+birthdisk::SCCounselorCharacterConstructor,counselor)||
       !r.Read(c.module+birthdisk::SCKillerCharacterClass,killer)||!Same(r,c.module+birthdisk::SCKillerCharacterConstructor,killer)||
       !ObjectPointer(counselor)||!ObjectPointer(killer)||expectedLoadedClass==killer)return finish(false);
    mark(4);if(
       !classchoice::MatchesPath(r,c.module,expectedLoadedClass,counselor,expectedPlainCounselor.bytes)||
       maps::Subclass(r,expectedLoadedClass,killer))return finish(false);
    mark(5);if(!r.Read(c.controller+ControllerPawn,h.pawn))return finish(false);
    diag.pawn=h.pawn;if(!ObjectPointer(h.pawn))return finish(false);
    mark(6);if(!lifetime::ReadSlot(r,currentRegistry,h.pawn,h.identity))return finish(false);
    diag.serial=h.identity.serial;if(!h.identity.HasAssignedSerial())return finish(false);
    mark(7);
    for(const auto& owner:c.identities)if(owner.object==h.pawn||owner.index==h.identity.index)return finish(false);
    h.type=expectedLoadedClass;
    const auto observed=[&](unsigned stage,std::uintptr_t at,std::uintptr_t expected,std::uintptr_t& sample){
        mark(stage);return r.Read(at,sample)&&sample==expected;};
    bool cacheSeen=false;std::uintptr_t cache=0;
    const auto spawnedCache=[&](){
        mark(13);if(!r.Read(c.playerState+birthdisk::SpawnedCharacterClass,diag.spawnedClass)||
            !SpawnedClassCacheAgrees(diag.spawnedClass,h.type)||(cacheSeen&&diag.spawnedClass!=cache))return false;
        cacheSeen=true;cache=diag.spawnedClass;return true;
    };
    const auto possession=[&](){return observed(8,c.controller+ControllerPawn,h.pawn,diag.pawn)&&
        observed(9,c.controller+ControllerAcknowledgedPawn,h.pawn,diag.acknowledged)&&
        observed(10,h.pawn+0x3A0,c.controller,diag.controller)&&observed(11,h.pawn+0x388,c.playerState,diag.state)&&
        observed(12,h.pawn+ObjectClass,h.type,diag.type)&&spawnedCache();};
    if(!possession())return finish(false);
    mark(14);if(!r.Read(h.type+0xF8,defaults)||!ObjectPointer(defaults)||defaults==h.pawn||!Same(r,defaults+ObjectClass,h.type)||
       !r.Read(c.playerState+birthdisk::ActiveCharacterClass,h.active)||!cfgending::SamePath(h.active.bytes,expectedPlainCounselor.bytes))return finish(false);
    promotion::Packet again{};lifetime::SlotObservation slot{};
    mark(15);if(!Current(r,c,callerThread,&diag.ownerStage)||!possession())return finish(false);
    mark(16);if(!r.Read(c.playerState+birthdisk::ActiveCharacterClass,again)||again.bytes!=h.active.bytes||
       !Same(r,h.type+0xF8,defaults)||!Same(r,defaults+ObjectClass,h.type)||!Registry(r,c,currentRegistry)||!lifetime::ReadSlot(r,currentRegistry,h.pawn,slot)||!(slot==h.identity)||
       !Same(r,c.module+birthdisk::SCCounselorCharacterClass,counselor)||!Same(r,c.module+birthdisk::SCCounselorCharacterConstructor,counselor)||
       !Same(r,c.module+birthdisk::SCKillerCharacterClass,killer)||!Same(r,c.module+birthdisk::SCKillerCharacterConstructor,killer)||
       !classchoice::MatchesPath(r,c.module,h.type,counselor,expectedPlainCounselor.bytes)||!possession())return finish(false);
    output=h;mark(0);return finish(true);
}
}
