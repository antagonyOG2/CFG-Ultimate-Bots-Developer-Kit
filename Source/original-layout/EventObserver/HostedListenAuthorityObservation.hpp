#pragma once
#include "HostedRegisteredRoster.hpp"
#include "../RuntimeProbe/HostedAuthority35.generated.hpp"

// H1 read-only listen-host admission. This header is not wired into the
// gameplay observer; passing it is not GC retention or native-call permission.
namespace cfghosted::listen35 {
struct Bindings {
    static constexpr std::array<std::array<std::uintptr_t,2>,4> Caches(){return {{
        {{disk35::WorldReflection,disk35::WorldConstructor}},{{disk35::NetDriverReflection,disk35::NetDriverConstructor}},
        {{disk35::HuntReflection,disk35::HuntConstructor}},
        {{cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor}}}};}
};
struct LocalPlayers {std::uintptr_t data=0;std::int32_t count=0,capacity=0;};
inline bool operator==(const LocalPlayers& a,const LocalPlayers& b){return a.data==b.data&&a.count==b.count&&a.capacity==b.capacity;}
struct Context {
    native::Context session{};
    std::array<native::Slot,3> local{}; // GameInstance, ILLLocalPlayer, SCPlayerController
    std::array<std::uintptr_t,3> classes{};
    LocalPlayers players{};
};
template<class R,std::size_t N> bool Code(R& r,std::uintptr_t module,std::uintptr_t at,const std::array<unsigned char,N>& expected){
    std::array<unsigned char,N> actual{};return r.Read(module+at,actual)&&actual==expected;
}
template<class R> bool Pins(R& r,std::uintptr_t module){
    return Code(r,module,disk35::WorldClassConstructor,disk35::WorldClassConstructorCode)&&
        Code(r,module,disk35::NetDriverClassConstructor,disk35::NetDriverClassConstructorCode)&&
        Code(r,module,disk35::HuntClassConstructor,disk35::HuntClassConstructorCode)&&
        Code(r,module,disk35::SCGameInstanceClassConstructor,disk35::SCGameInstanceClassConstructorCode)&&
        Code(r,module,disk35::ILLLocalPlayerClassConstructor,disk35::ILLLocalPlayerClassConstructorCode)&&
        Code(r,module,disk35::PlayerControllerClassConstructor,disk35::PlayerControllerClassConstructorCode);
}
template<class R> bool Capture(R& source,const native::Admission& a,std::uint32_t currentThread,Context& output,unsigned* reads=nullptr,native::Trace* trace=nullptr,bool allowUnassignedLocal=false){
    output={};native::Budget<R,8192> r(source);Context c;
    const auto mark=[&](unsigned stage,std::uintptr_t object=0,std::uintptr_t expected=0){if(trace)*trace={stage,object,expected};};
    struct Finish{native::Budget<R,8192>& r;unsigned* result;~Finish(){if(result)*result=r.Used();}}finish{r,reads};
    if(!native::Capture<decltype(r),Bindings>(r,a,currentThread,c.session,nullptr,trace,allowUnassignedLocal))return false;
    mark(101);if(!Pins(r,a.module))return false;
    const std::array<std::array<std::uintptr_t,2>,3> caches{{
        {{disk35::SCGameInstanceReflection,disk35::SCGameInstanceConstructor}},
        {{disk35::ILLLocalPlayerReflection,disk35::ILLLocalPlayerConstructor}},
        {{disk35::PlayerControllerReflection,disk35::PlayerControllerConstructor}}}};
    std::array<std::uintptr_t,3> objects{};
    mark(110,c.session.Driver());
    if(!native::Same(r,c.session.Driver()+disk35::ServerConnection,std::uintptr_t{})||
       !native::Same(r,c.session.Driver()+disk35::DriverWorld,a.world)||
       !r.Read(a.world+disk35::WorldGameInstance,objects[0])||
       !r.Read(objects[0]+disk35::GameInstanceLocalPlayers,c.players)||
       c.players.count<1||c.players.count>16||c.players.capacity<c.players.count||c.players.capacity>16||!cfgprobe::ObjectPointer(c.players.data)||
       !r.Read(c.players.data,objects[1])||!r.Read(objects[1]+disk35::PlayerController,objects[2]))return false;
    for(unsigned i=0;i<objects.size();++i){
        for(unsigned j=0;j<i;++j)if(objects[i]==objects[j])return false;
        for(const auto& identity:c.session.identities)if(objects[i]==identity.object)return false;
        mark(120+i,objects[i]);
        if(!native::Class(r,a.module,caches[i][0],caches[i][1],c.classes[i]))return false;
        mark(130+i,objects[i],c.classes[i]);
        if(!(allowUnassignedLocal?native::TypedAllowUnassigned(r,c.session.table,objects[i],c.classes[i],c.local[i]):
             native::Typed(r,c.session.table,objects[i],c.classes[i],c.local[i])))return false;
    }
    mark(140,objects[2]+disk35::ControllerPlayer,objects[1]);
    if(!native::Same(r,objects[2]+disk35::ControllerPlayer,objects[1])||
       !native::Same(r,objects[2]+disk35::ActorRole,disk::AuthorityRole)||
       !native::WorldOwned(r,objects[2],a.world))return false;
    native::Context again;
    mark(150);
    if(!native::Capture<decltype(r),Bindings>(r,a,currentThread,again,nullptr,nullptr,allowUnassignedLocal)||!native::SameSession(c.session,again))return false;
    for(unsigned i=0;i<objects.size();++i){native::Slot identity;std::uintptr_t type=0;
        if(!native::Class(r,a.module,caches[i][0],caches[i][1],type)||type!=c.classes[i]||
           !(allowUnassignedLocal?native::TypedAllowUnassigned(r,again.table,c.local[i].object,c.classes[i],identity):
             native::Typed(r,again.table,c.local[i].object,c.classes[i],identity))||!(identity==c.local[i]))return false;
    }
    if(!Pins(r,a.module)||!native::Same(r,a.world+disk35::WorldGameInstance,c.local[0].object)||
       !native::Same(r,c.local[0].object+disk35::GameInstanceLocalPlayers,c.players)||
       !native::Same(r,c.players.data,c.local[1].object)||
       !native::Same(r,c.local[1].object+disk35::PlayerController,c.local[2].object)||
       !native::Same(r,c.local[2].object+disk35::ControllerPlayer,c.local[1].object)||
       !native::Same(r,c.local[2].object+disk35::ActorRole,disk::AuthorityRole)||
       !native::WorldOwned(r,c.local[2].object,a.world)||
       !native::Same(r,again.Driver()+disk35::ServerConnection,std::uintptr_t{})||
       !native::Same(r,again.Driver()+disk35::DriverWorld,a.world)||
       !cfgprobe::lifetime::RevalidateBatch(r,a.module+cfgprobe::lifetime::ObjectArrayRva,again.table,c.local))return false;
    mark(0);c.session=again;output=c;return true;
}
}
