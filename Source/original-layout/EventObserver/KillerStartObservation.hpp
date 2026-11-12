#pragma once
#include "../RuntimeProbe/KillerStartBindings.generated.hpp"
#include "../RuntimeProbe/ObjectLifetime.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include <vector>
namespace cfgkillerstart {
constexpr unsigned MaximumLevels=32,MaximumActors=100000,ActorsPerStep=16,MaximumReservedCells=1000000;
constexpr unsigned MaximumRestarts=16;
struct Header {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const Header& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
    bool Sane(unsigned limit)const{return count>=0&&count<=static_cast<int>(limit)&&capacity>=count&&capacity<=static_cast<int>(MaximumReservedCells)&&
        (!capacity?data==0:cfgprobe::ObjectPointer(data)&&cfgprobe::lifetime::Extent(data,8*static_cast<std::uintptr_t>(capacity)));}
};
enum class State {Empty,Scanning,Verifying,Ready,Refused};
enum class Failure {None,Context,ClassCache,LevelCache,WorldLevels,ActorRoster,DuplicateLevel,Caches,LevelWorld,LevelType,ChangedHeader,ActorIdentity,MissingMarker,StaleMarker};
struct Marker {std::uintptr_t actor=0,level=0,type=0;cfgprobe::lifetime::SlotObservation identity{};};
// Caller Current() must revalidate the supplied owned offline world and owner
// thread before/after every step. No calls, writes or global object discovery.
// Select the FIRST native start in level/actor order, as FindKillerPlayerStart
// does in the working base mod. Verify only the prefix through that marker,
// not every unrelated actor after it or a fabricated uniqueness requirement.
// Two bounded passes compare that sampled prefix. This is sampled proof,
// not a GC lock: CurrentMarker must be called immediately before native use.
class Observation {
    struct Level {std::uintptr_t object=0;Header actors{};std::vector<std::uintptr_t> cells;};
    std::array<Level,MaximumLevels> levels_{};Header header_{};
    std::uintptr_t module_=0,world_=0,klass_=0,levelClass_=0;
    unsigned level_=0,actor_=0,stopLevel_=0,stopActor_=0;State state_=State::Empty;Marker marker_{};
    std::uintptr_t markerCell_=0;
    Failure failure_=Failure::None;
    unsigned restarts_=0,appendUpdates_=0,changedSource_=0,changedLevel_=0;
    bool capturePending_=false;
    bool Failed(Failure value){failure_=value;return false;}
    static bool SameMarker(const Marker& expected,const Marker& fresh){
        return expected.actor==fresh.actor&&expected.level==fresh.level&&expected.type==fresh.type&&
            expected.identity.object==fresh.identity.object&&expected.identity.index==fresh.identity.index&&
            (expected.identity.serial==0?fresh.identity.serial>=0:expected.identity.serial==fresh.identity.serial);
    }
    template<class R> bool Pair(R& r,std::uintptr_t a,std::uintptr_t b,std::uintptr_t expected){
        std::uintptr_t x=0,y=0;return r.Read(module_+a,x)&&r.Read(module_+b,y)&&cfgprobe::MatchingClassCache(x,y)==expected;}
    template<class R> bool Typed(R& r,std::uintptr_t object,std::uintptr_t type){
        cfgprobe::Ancestry ancestry;return cfgprobe::CaptureAncestry(r,object,ancestry)&&ancestry.Contains(type)&&cfgprobe::StableAncestry(r,object,ancestry);}
    template<class R> bool Stable(R& r,bool acceptAppend=false){
        Header fresh{};
        if(!Pair(r,disk::ClassCache,disk::ConstructorCache,klass_)||
           !Pair(r,cfgradarworld::disk::LevelClassCache,cfgradarworld::disk::LevelConstructorCache,levelClass_))return Failed(Failure::Caches);
        if(!r.Read(world_+cfgradarworld::disk::WorldLevels,fresh)||!fresh.Sane(MaximumLevels)||!fresh.count)return Failed(Failure::WorldLevels);
        if(!(fresh==header_)){changedSource_=1;changedLevel_=0;return Failed(Failure::ChangedHeader);}
        for(unsigned i=0;i<static_cast<unsigned>(header_.count);++i){std::uintptr_t object=0,outer=0;
            // Streamed levels can have their source UWorld as Outer. The native
            // Level.OwningWorld field, not Outer, identifies the active world.
            if(!r.Read(header_.data+8*i,object)||object!=levels_[i].object||
               !r.Read(object+cfgkill::disk::OwningWorld,outer)||outer!=world_)return Failed(Failure::LevelWorld);
            if(!Typed(r,object,levelClass_))return Failed(Failure::LevelType);
            // After selecting a candidate, later levels cannot change the
            // first-match prefix. Their actor arrays are not a marker resource.
            if((state_==State::Verifying||state_==State::Ready)&&i>stopLevel_)continue;
            if(!r.Read(object+cfgradarworld::disk::LevelActors,fresh)||!fresh.Sane(MaximumActors))return Failed(Failure::ActorRoster);
            auto& old=levels_[i].actors;
            if(!(fresh==old)){
                const bool earlier=state_==State::Scanning?i<level_:i<stopLevel_;
                // Same-storage append/capacity changes preserve the sampled
                // prefix, except a completed earlier level could gain a new
                // FIRST marker and therefore needs a new bounded discovery.
                if(acceptAppend&&fresh.data==old.data&&fresh.count>=old.count&&
                   !(earlier&&fresh.count>old.count)){old=fresh;++appendUpdates_;}
                else{changedSource_=2;changedLevel_=i;return Failed(Failure::ChangedHeader);}
            }}
        return true;
    }
    template<class R> bool Capture(R& r){
        if(!r.Read(world_+cfgradarworld::disk::WorldLevels,header_)||!header_.Sane(MaximumLevels)||!header_.count)return Failed(Failure::WorldLevels);
        for(unsigned i=0;i<static_cast<unsigned>(header_.count);++i){auto& level=levels_[i];
            if(!r.Read(header_.data+8*i,level.object)||!cfgprobe::ObjectPointer(level.object)||
               !r.Read(level.object+cfgradarworld::disk::LevelActors,level.actors)||!level.actors.Sane(MaximumActors))return Failed(Failure::ActorRoster);
            for(unsigned j=0;j<i;++j)if(levels_[j].object==level.object)return Failed(Failure::DuplicateLevel);
            level.cells.clear();level.cells.reserve(ActorsPerStep);
        }
        return Stable(r,true);
    }
    template<class Host> bool Recover(Host& host){
        if(failure_!=Failure::ChangedHeader||!host.Current()||restarts_==MaximumRestarts)return Refuse();
        ++restarts_;capturePending_=true;state_=State::Scanning;
        level_=actor_=stopLevel_=stopActor_=0;marker_={};markerCell_=0;
        // Never use the old array after a header change. Reacquire the owned
        // world/levels on the NEXT scheduled phase, not a busy retry here.
        for(auto& level:levels_)level.cells.clear();
        failure_=Failure::None;return true;
    }
    template<class R> bool Inspect(R& r,std::uintptr_t object,std::uintptr_t level,Marker& out,bool& match){
        match=false;out={};cfgprobe::Ancestry ancestry;
        if(!cfgprobe::CaptureAncestry(r,object,ancestry)||!cfgprobe::StableAncestry(r,object,ancestry))return false;
        if(!ancestry.Contains(klass_))return true;
        std::uintptr_t outer=0,defaults=0;cfgprobe::lifetime::ArrayHeader table;
        if(!r.Read(object+0x20,outer)||outer!=level||!r.Read(ancestry.classes[0]+0xF8,defaults)||defaults==object||
           !cfgprobe::lifetime::ReadArrayHeader(r,module_+cfgprobe::lifetime::ObjectArrayRva,table)||
           !cfgprobe::lifetime::ReadSlot(r,table,object,out.identity))return false;
        // An authored marker may have no assigned weak-object serial. This is
        // strong owned-roster TRANSFORM observation only, not permission to
        // mutate it or treat serial0 as a durable cross-callback identity.
        // CurrentMarker must freshly prove its exact cell/type/Outer/slot under
        // the caller's no-GC native-owner context immediately around use.
        // CDO and class-owned templates cannot have the exact owned level Outer.
        out.actor=object;out.level=level;out.type=ancestry.classes[0];match=true;return true;
    }
    bool Refuse(){if(failure_==Failure::None)failure_=Failure::ActorIdentity;state_=State::Refused;marker_={};return false;}
public:
    State CurrentState()const{return state_;}
    Failure LastFailure()const{return failure_;}
    unsigned Restarts()const{return restarts_;}unsigned AppendUpdates()const{return appendUpdates_;}
    unsigned ChangedSource()const{return changedSource_;}unsigned ChangedLevel()const{return changedLevel_;}
    template<class R,class Host> bool Begin(R& r,Host& host,std::uintptr_t module,std::uintptr_t world){
        if(state_!=State::Empty||!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(world)||!host.Current()){Failed(Failure::Context);return Refuse();}
        module_=module;world_=world;
        if(!r.Read(module+disk::ClassCache,klass_)||!cfgprobe::ObjectPointer(klass_)){Failed(Failure::ClassCache);return Refuse();}
        if(!r.Read(module+cfgradarworld::disk::LevelClassCache,levelClass_)||!cfgprobe::ObjectPointer(levelClass_)){Failed(Failure::LevelCache);return Refuse();}
        if(!Capture(r))return Recover(host);
        if(!host.Current()){Failed(Failure::Context);return Refuse();}state_=State::Scanning;return true;
    }
    template<class R,class Host> bool Step(R& r,Host& host,unsigned actorQuota=ActorsPerStep){
        if(state_!=State::Scanning&&state_!=State::Verifying)return false;
        if(!host.Current()){Failed(Failure::Context);return Refuse();}
        if(capturePending_){
            capturePending_=false;if(!Capture(r))return Recover(host);
            if(!host.Current()){Failed(Failure::Context);return Refuse();}return true;
        }
        if(!Stable(r,true))return Recover(host);
        // Startup-only caller may double the small slice; never an unbounded scan.
        unsigned quota=(std::min)(actorQuota,ActorsPerStep*2);
        if(!quota)return false;
        while(quota&&level_<static_cast<unsigned>(header_.count)){
            auto& level=levels_[level_];
            if(actor_==static_cast<unsigned>(level.actors.count)){++level_;actor_=0;continue;}
            --quota;std::uintptr_t object=0,again=0;const auto cell=level.actors.data+8*actor_;
            if(!r.Read(cell,object)||!r.Read(cell,again)||object!=again)return Refuse();
            if(state_==State::Scanning)level.cells.push_back(object);
            else if(actor_>=level.cells.size()||level.cells[actor_]!=object)return Refuse();
            ++actor_;if(!object)continue;
            Marker found;bool match=false;
            if(!Inspect(r,object,level.object,found,match)||!r.Read(cell,again)||again!=object)return Refuse();
            if(match){
                if(state_==State::Scanning){
                    marker_=found;markerCell_=cell;stopLevel_=level_;stopActor_=actor_;
                    if(!Stable(r,true))return Recover(host);
                    if(!host.Current()){Failed(Failure::Context);return Refuse();}
                    state_=State::Verifying;level_=actor_=0;return true;
                }
                if(cell!=markerCell_||level_!=stopLevel_||actor_!=stopActor_||
                   !SameMarker(marker_,found))return Refuse();
                marker_=found; // a corroborated native0-to-positive assignment strengthens the observation
                state_=State::Ready;Marker out;
                if(!CurrentMarker(r,host,out))return Refuse();return true;
            }
            if(state_==State::Verifying&&level_==stopLevel_&&actor_==stopActor_)return Refuse();
        }
        if(!Stable(r,true))return Recover(host);
        if(!host.Current()){Failed(Failure::Context);return Refuse();}
        if(level_==static_cast<unsigned>(header_.count)){Failed(Failure::MissingMarker);return Refuse();}
        return true;
    }
    template<class R,class Host> bool CurrentMarker(R& r,Host& host,Marker& out){
        out={};if(state_!=State::Ready||!host.Current()||!Stable(r,true))return false;
        Marker a,b;bool first=false,second=false;
        if(!Inspect(r,marker_.actor,marker_.level,a,first)||!Inspect(r,marker_.actor,marker_.level,b,second)||!first||!second||
           !SameMarker(marker_,a)||!SameMarker(marker_,b)||!(a.identity==b.identity)||!host.Current())return false;
        // Revalidate its exact roster cell, not just a matching Outer.
        std::uintptr_t cell=0;
        if(!markerCell_||!r.Read(markerCell_,cell)||cell!=marker_.actor||!Stable(r,true)||!host.Current())return false;
        marker_=a;out=marker_;return true;
    }
};
}
