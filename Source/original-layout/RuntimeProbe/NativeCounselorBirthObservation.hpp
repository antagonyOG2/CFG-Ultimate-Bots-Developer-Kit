#pragma once
#include "../EventObserver/OfflineContextObservation.hpp"
#include "../EventObserver/PrimarySpawnObservation.hpp"
#include "SetupSelections.generated.hpp"
#include "CounselorBirthBindings.generated.hpp"
#include "NativeClassPathChoice.hpp"
#include "AiResourceSnapshot.hpp"
#include "OfflineIntroMetadataBindings.generated.hpp"
#include "OfflineLifecycleBindings.generated.hpp"
#include "KillBindings.generated.hpp"
#include "../Core/NativeSpawnedClassCache.hpp"
namespace cfgmenu::birth {
constexpr unsigned ReadLimit=1024;
enum class Step : unsigned {None,Context,Caches,ModeType,PawnType,KillerType,ControllerType,StatePointer,StateType,
    StateIdentity,PawnState,PawnController,StateOwner,StateFlags,PawnClass,SpawnedClass,ActiveClass,NonemptyActive,
    Compare,Revalidate,FinalCaches,FinalContext,Ready,NativeSelection};
struct Trace {Step step=Step::None;std::uintptr_t pawn=0,type=0,state=0,spawned=0;std::uint32_t flags=0,reads=0;bool activeNonempty=false,nativeActiveVerified=false;};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):reader_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
};
struct Observation {
    std::uintptr_t pawn=0,type=0,playerState=0,spawnedClass=0;
    cfgprobe::lifetime::SlotObservation stateSlot{};
    std::array<unsigned char,40> activeClass{};
    bool nativeActiveVerified=false;
    bool operator==(const Observation& b)const noexcept{return pawn==b.pawn&&type==b.type&&playerState==b.playerState&&spawnedClass==b.spawnedClass&&stateSlot==b.stateSlot&&activeClass==b.activeClass&&nativeActiveVerified==b.nativeActiveVerified;}
};
// Fresh owned Sandbox arrival only, on its native owner thread. Preserves
// normal customization instead of inventing a class from null frontend save.
// Positive identity + acknowledged/possessed pawn + controller/state backlinks
// + a nonzero native SpawnedCharacterClass must agree. Native first Sandbox
// birth may bypass Active/Spawned bookkeeping (Restart3F6330's null-pawn branch).
// When Spawned is absent, require exact native Sandbox controller and the live
// state-owned Active path to match the actual pawn's full package.class name.
// This is read-only class association, not a default/spectator fallback.
// Active is diagnostic only: weak indexFFFFFFFF is a canonical EMPTY packet,
// not selection proof. Never resolve/load/modify it. This proves current native
// birth consistency, not responsiveness, retention, or native-call authority.
template<class Reader> bool Observe(Reader& reader,const cfgobserver::Plan& fresh,std::uint32_t callerThread,
    bool exactImageVerified,Observation& output,unsigned* consumed=nullptr,Trace* diagnostic=nullptr,bool nativeOffline=false){
    output={};if(consumed)*consumed=0;Bounded<Reader> r(reader);
    Trace trace;trace.pawn=fresh.pawn;trace.step=Step::Context;
    const auto finish=[&](bool okay){trace.reads=r.reads;if(consumed)*consumed=r.reads;if(diagnostic)*diagnostic=trace;return okay;};
    if(!exactImageVerified||callerThread!=fresh.threadId||!cfgobserver::ContextUnchanged(r,fresh))return finish(false);
    std::array<std::uintptr_t,8> native{};
    const std::array<cfgprobe::NativeClass,4> ids{{nativeOffline?cfgprobe::OfflineBotsMode:cfgprobe::SandboxMode,cfgprobe::Counselor,cfgprobe::Killer,cfgprobe::PlayerControllerClass}};
    trace.step=Step::Caches;
    for(unsigned i=0;i<ids.size();++i){const auto& binding=cfgprobe::NativeClasses[ids[i]];
        if(!r.Read(fresh.module+binding.reflectionCacheRva,native[2*i])||
            !r.Read(fresh.module+binding.constructorCacheRva,native[2*i+1])||
            !cfgprobe::MatchingClassCache(native[2*i],native[2*i+1]))return finish(false);}
    std::uintptr_t stateBase=0,stateCtor=0,sandboxController=0,sandboxCtor=0;
    if(!r.Read(fresh.module+selectiondisk::SCPlayerStateClass,stateBase)||
        !r.Read(fresh.module+selectiondisk::SCPlayerStateConstructor,stateCtor)||!cfgprobe::MatchingClassCache(stateBase,stateCtor))return finish(false);
    cfgspawn::TypeObservations<Bounded<Reader>,8> types(r);
    trace.step=Step::ModeType;if(!types.ObjectIs(fresh.mode,native[0]))return finish(false);
    trace.step=Step::PawnType;if(!types.ObjectIs(fresh.pawn,native[2]))return finish(false);
    trace.step=Step::KillerType;if(types.ObjectIs(fresh.pawn,native[4]))return finish(false);
    trace.step=Step::ControllerType;if(!types.ObjectIs(fresh.controller,native[6]))return finish(false);
    std::array<std::uintptr_t,3> offlineClasses{};cfgprobe::lifetime::SlotObservation offlineLevel{};
    const auto nativeScope=[&](){
        if(!nativeOffline)return true;
        namespace d=intrometadatadisk;std::uintptr_t state=0,level=0;std::uint8_t role=0;std::uint32_t flags=0;
        if(!cfgprimary::LookupVetoClearForObservation(r,fresh.module)||
           !r.Read(fresh.controller+0x388,state)||!r.Read(fresh.mode+0x20,level)||
           !cfgobserver::Same(r,fresh.mode+cfgprobe::ObjectClass,native[0])||
           !cfgobserver::Same(r,fresh.mode,fresh.module+offlinelifecycledisk::OfflineBotsVtable)||
           !r.Read(state+0x110,role)||role!=3||!r.Read(state+0x394,flags)||(flags&6))return false;
        const std::array<std::uintptr_t,3> objects{{fresh.controller,state,level}},caches{{d::SCPlayerController_OfflineBotsClass,d::SCPlayerState_OfflineBotsClass,cfgkill::disk::LevelReflection}};
        for(unsigned n=0;n<3;++n){std::uintptr_t a=0,b=0,defaults=0;
            if(!r.Read(fresh.module+caches[n],a)||!r.Read(fresh.module+caches[n]+8,b)||!cfgprobe::MatchingClassCache(a,b)||
               (offlineClasses[n]&&offlineClasses[n]!=a)||!cfgobserver::Same(r,objects[n]+cfgprobe::ObjectClass,a)||
               !r.Read(a+0xF8,defaults)||!cfgprobe::ObjectPointer(defaults)||defaults==objects[n]||!cfgobserver::Same(r,defaults+cfgprobe::ObjectClass,a))return false;
            offlineClasses[n]=a;
        }
        cfgprobe::lifetime::SlotObservation slot{};
        if(!cfgprobe::lifetime::ReadSlot(r,fresh.objectArray,level,slot)||
           (offlineLevel.object&&!(offlineLevel==slot))||!cfgobserver::Same(r,level+cfgkill::disk::OwningWorld,fresh.world))return false;
        // Match the pre-intro owner proof: Level is callback-local, not a
        // retained weak identity. Native UE may leave its serial unassigned.
        // Only admit that case through the exact world's strong Levels list.
        if(!slot.HasAssignedSerial()){
            struct Levels{std::uintptr_t data=0;int count=0,capacity=0;} levels{},again{};
            if(!r.Read(fresh.world+0x110,levels)||!cfgprobe::ObjectPointer(levels.data)||levels.count<1||levels.count>256||
                levels.capacity<levels.count||levels.capacity>4096)return false;
            std::uintptr_t cell=0;
            for(int i=0;i<levels.count;++i){std::uintptr_t value=0;if(!r.Read(levels.data+8*i,value))return false;
                if(value==level){cell=levels.data+8*i;break;}}
            cfgprobe::lifetime::SlotObservation finalSlot{};
            if(!cell||!r.Read(fresh.world+0x110,again)||again.data!=levels.data||again.count!=levels.count||again.capacity!=levels.capacity||
                !cfgobserver::Same(r,cell,level)||!cfgobserver::Same(r,fresh.mode+0x20,level)||
                !cfgobserver::Same(r,level+cfgkill::disk::OwningWorld,fresh.world)||
                !cfgprobe::lifetime::ReadSlot(r,fresh.objectArray,level,finalSlot)||!(finalSlot==slot))return false;
        }
        offlineLevel=slot;return true;
    };
    if(!nativeScope())return finish(false);
    const auto sample=[&](Observation& value){
        value.pawn=fresh.pawn;std::uintptr_t backlink=0,spawned=0;std::uint32_t flags=0;
        trace.step=Step::StatePointer;if(!r.Read(fresh.controller+0x388,value.playerState))return false;trace.state=value.playerState;
        trace.step=Step::StateType;if(!types.ObjectIs(value.playerState,stateBase))return false;
        trace.step=Step::StateIdentity;if(!cfgspawn::ObjectSlot(r,fresh.objectArray,value.playerState,value.stateSlot)||!value.stateSlot.HasAssignedSerial())return false;
        trace.step=Step::PawnState;if(!r.Read(fresh.pawn+0x388,backlink)||backlink!=value.playerState)return false;
        trace.step=Step::PawnController;if(!r.Read(fresh.pawn+0x3A0,backlink)||backlink!=fresh.controller)return false;
        trace.step=Step::StateOwner;if(!r.Read(value.playerState+cfgspawn::ActorOwner,backlink)||backlink!=fresh.controller)return false;
        trace.step=Step::StateFlags;if(!r.Read(value.playerState+0x394,flags))return false;trace.flags=flags;if(flags&4)return false;
        trace.step=Step::PawnClass;if(!r.Read(fresh.pawn+cfgprobe::ObjectClass,value.type))return false;trace.type=value.type;
        trace.step=Step::SpawnedClass;if(!r.Read(value.playerState+selectiondisk::SpawnedCharacterClass,spawned))return false;
        trace.spawned=value.spawnedClass=spawned;
        if(!SpawnedClassCacheAgrees(spawned,value.type))return false;
        trace.step=Step::ActiveClass;if(!r.Read(value.playerState+selectiondisk::ActiveCharacterClass,value.activeClass))return false;
        // FSoftClassPtr's asset FName is at16. Cache index/tag bytes and an empty
        // weak index must not be interpreted as a selected character path.
        bool nonempty=false;for(unsigned i=16;i<24;++i)nonempty|=value.activeClass[i]!=0;
        trace.activeNonempty=nonempty;trace.step=Step::NonemptyActive;
        if(nativeOffline){
            if(!nonempty||!classchoice::MatchesPath(r,fresh.module,value.type,native[2],value.activeClass))return false;
            trace.nativeActiveVerified=value.nativeActiveVerified=true;
        }
        if(!spawned&&!nativeOffline){
            trace.step=Step::NativeSelection;
            if(!nonempty||!r.Read(fresh.module+birthdisk::SCPlayerController_SandboxClass,sandboxController)||
               !r.Read(fresh.module+birthdisk::SCPlayerController_SandboxConstructor,sandboxCtor)||
               !cfgprobe::MatchingClassCache(sandboxController,sandboxCtor)||!types.ObjectIs(fresh.controller,sandboxController))return false;
            catalogues::CharacterList live;live.header={value.playerState+selectiondisk::ActiveCharacterClass,1,1};live.entries[0]=value.activeClass;
            classchoice::Choice exact;
            if(!classchoice::MatchLoaded(r,fresh.module,value.type,native[2],live,exact)||exact.type!=value.type||exact.packet!=value.activeClass)return false;
            trace.nativeActiveVerified=value.nativeActiveVerified=true;
        }
        return true; // Native owned pawn/class proof is authoritative, not this unused soft packet.
    };
    Observation a,b;if(!sample(a)||!sample(b))return finish(false);
    trace.step=Step::Compare;if(!(a==b))return finish(false);
    trace.step=Step::Revalidate;if(!types.Revalidate())return finish(false);
    trace.step=Step::FinalCaches;
    for(unsigned i=0;i<ids.size();++i){std::uintptr_t value=0;const auto& binding=cfgprobe::NativeClasses[ids[i]];
        if(!r.Read(fresh.module+binding.reflectionCacheRva,value)||value!=native[2*i]||
            !r.Read(fresh.module+binding.constructorCacheRva,value)||value!=native[2*i+1])return finish(false);}
    std::uintptr_t value=0;
    if(!r.Read(fresh.module+selectiondisk::SCPlayerStateClass,value)||value!=stateBase||
        !r.Read(fresh.module+selectiondisk::SCPlayerStateConstructor,value)||value!=stateCtor)return finish(false);
    if(!nativeOffline&&b.nativeActiveVerified&&(!r.Read(fresh.module+birthdisk::SCPlayerController_SandboxClass,value)||value!=sandboxController||
        !r.Read(fresh.module+birthdisk::SCPlayerController_SandboxConstructor,value)||value!=sandboxCtor))return finish(false);
    trace.step=Step::FinalContext;if(!nativeScope()||!cfgobserver::ContextUnchanged(r,fresh))return finish(false);
    output=b;trace.step=Step::Ready;return finish(true);
}
}
