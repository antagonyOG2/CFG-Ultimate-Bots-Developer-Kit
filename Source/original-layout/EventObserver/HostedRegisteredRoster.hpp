#pragma once
#include "HostedNativeObservation.hpp"
#include "../RuntimeProbe/SpawnBindings.generated.hpp"
namespace cfghosted::native {
enum class Kind { Counselor, Killer };
struct Registered {Slot pawn{},controller{};Kind kind=Kind::Counselor;};
template<class R> bool WorldOwned(R& r,std::uintptr_t object,std::uintptr_t world){
    // Bounded Outer chain, not Actor.Owner or possession. Confirm every edge
    // again before admitting; no level/global object enumeration.
    std::array<std::uintptr_t,8> nodes{},parents{};unsigned count=0;
    while(object!=world){
        if(!cfgprobe::ObjectPointer(object)||count==nodes.size())return false;
        for(unsigned i=0;i<count;++i)if(nodes[i]==object)return false;
        nodes[count]=object;if(!r.Read(object+0x20,parents[count]))return false;
        object=parents[count++];
    }
    for(unsigned i=0;i<count;++i)if(!Same(r,nodes[i]+0x20,parents[i]))return false;
    return count>0;
}
template<class R> bool ObserveRegistered(R& source,const Context& context,std::uint32_t currentThread,
    std::uintptr_t pawn,std::uintptr_t controller,Kind kind,Registered& output,unsigned* reads=nullptr){
    output={};Budget<R> r(source);
    struct Finish{Budget<R>& r;unsigned* result;~Finish(){if(result)*result=r.Used();}}finish{r,reads};
    Context before,after;
    if(!Capture(r,context.admission,currentThread,before)||!SameSession(context,before)||pawn==controller)return false;
    const bool killer=kind==Kind::Killer;
    if(!killer&&kind!=Kind::Counselor)return false;
    const auto& actor=cfgprobe::NativeClasses[killer?cfgprobe::Killer:cfgprobe::Counselor];
    const auto& ai=cfgspawn::Classes[killer?3:2];std::uintptr_t pawnType=0,controllerType=0;
    Registered found;found.kind=kind;
    if(!Class(r,context.admission.module,actor.reflectionCacheRva,actor.constructorCacheRva,pawnType)||
       !Class(r,context.admission.module,ai.reflection,ai.constructor,controllerType)||
       !Typed(r,before.table,pawn,pawnType,found.pawn)||!Typed(r,before.table,controller,controllerType,found.controller)||
       !WorldOwned(r,pawn,context.World())||!WorldOwned(r,controller,context.World())||
       !Same(r,pawn+cfgescape::disk::Controller,controller)||!Same(r,controller+cfgprobe::ControllerPawn,pawn)||
       !Same(r,pawn+disk::ActorRole,disk::AuthorityRole)||!Same(r,controller+disk::ActorRole,disk::AuthorityRole))return false;
    if(!Capture(r,context.admission,currentThread,after)||!SameSession(before,after))return false;
    Slot a,b;
    if(!Typed(r,after.table,pawn,pawnType,a)||!(a==found.pawn)||
       !Typed(r,after.table,controller,controllerType,b)||!(b==found.controller)||
       !WorldOwned(r,pawn,context.World())||!WorldOwned(r,controller,context.World()))return false;
    if(!Same(r,pawn+cfgescape::disk::Controller,controller)||!Same(r,controller+cfgprobe::ControllerPawn,pawn)||
       !Same(r,pawn+disk::ActorRole,disk::AuthorityRole)||!Same(r,controller+disk::ActorRole,disk::AuthorityRole)||
       !Same(r,context.admission.module+ai.reflection,controllerType)||!Same(r,context.admission.module+ai.constructor,controllerType)||
       !Same(r,context.admission.module+actor.reflectionCacheRva,pawnType)||!Same(r,context.admission.module+actor.constructorCacheRva,pawnType))return false;
    output=found;return true;
}
// Owner-thread-only registry. An entry is supplied by a completed native spawn
// or explicit adoption request, never discovered by scanning. Registering does
// not dispatch gameplay, change possession or imply network replication proof.
class Roster {
    Context context_{};std::array<Registered,8> entries_{};unsigned count_=0,cursor_=0;bool armed_=false;
public:
    void Reset(){context_={};entries_={};count_=cursor_=0;armed_=false;}
    unsigned Count()const{return count_;}
    template<class R> bool Arm(R& r,const Admission& a,std::uint32_t thread){
        Context fresh;if(!Capture(r,a,thread,fresh)){Reset();return false;}
        if(armed_&&!SameSession(context_,fresh)){Reset();return false;}
        context_=fresh;armed_=true;return true;
    }
    template<class R> bool Register(R& r,std::uint32_t thread,std::uintptr_t pawn,std::uintptr_t controller,Kind kind){
        if(!armed_)return false;Registered entry;
        if(!ObserveRegistered(r,context_,thread,pawn,controller,kind,entry))return false;
        for(unsigned i=0;i<count_;++i)if(entries_[i].pawn.object==pawn||entries_[i].controller.object==controller)
            return entries_[i].pawn==entry.pawn&&entries_[i].controller==entry.controller&&entries_[i].kind==kind;
        if(count_==entries_.size())return false;entries_[count_++]=entry;return true;
    }
    template<class R> bool Next(R& r,std::uint32_t thread,Registered& output){
        output={};if(!armed_||!count_)return false;
        const auto index=cursor_++%count_;const auto expected=entries_[index];Registered current;
        if(!ObserveRegistered(r,context_,thread,expected.pawn.object,expected.controller.object,expected.kind,current)||
           !(current.pawn==expected.pawn)||!(current.controller==expected.controller)){
            // Fail closed; no Tick or cleanup writes on invalid ownership.
            Reset();return false;
        }
        output=current;return true;
    }
};
}
