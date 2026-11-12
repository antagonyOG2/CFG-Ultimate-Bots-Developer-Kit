#pragma once
#include "KillerStartObservation.hpp"
#include "../RuntimeProbe/TypeObservations.hpp"
#include "../RuntimeProbe/CounselorStartBindings.generated.hpp"
namespace cfgcounselorstart {
constexpr std::uintptr_t ClassCache=disk::ClassCache,ConstructorCache=disk::ConstructorCache;
constexpr unsigned MaximumMarkers=64,ActorsPerStep=32;
using Header=cfgkillerstart::Header;
struct Marker {std::uintptr_t actor=0,level=0;unsigned levelIndex=0,index=0;cfgprobe::lifetime::SlotObservation slot{};};
enum class State:unsigned {Empty,Scanning,Ready,Refused};
// A one-shot, paced survey of this world's authored COUNSELOR starts. Native
// RestartPlayer uses these in the donor. Never invent positions around the
// human, enumerate global objects, or retain a zero-serial mutation target.
class Observation {
    State state_=State::Empty;Header levels_{};std::uintptr_t module_=0,world_=0,klass_=0,levelType_=0;
    unsigned level_=0,index_=0,count_=0;std::array<Marker,MaximumMarkers> markers_{};
    bool Refuse(){state_=State::Refused;return false;}
    template<class R> bool Classes(R& r){std::uintptr_t a=0,b=0;
        return r.Read(module_+ClassCache,a)&&a==klass_&&r.Read(module_+ConstructorCache,b)&&b==klass_&&
            r.Read(module_+cfgradarworld::disk::LevelClassCache,a)&&a==levelType_&&
            r.Read(module_+cfgradarworld::disk::LevelConstructorCache,b)&&b==levelType_;}
    template<class R> bool Levels(R& r){Header h{};return r.Read(world_+cfgradarworld::disk::WorldLevels,h)&&h==levels_;}
    template<class R> bool Level(R& r,unsigned index,std::uintptr_t& level,Header& actors){
        std::uintptr_t again=0;cfgprobe::Ancestry type;
        return index<static_cast<unsigned>(levels_.count)&&r.Read(levels_.data+8*index,level)&&cfgprobe::ObjectPointer(level)&&
            r.Read(level+cfgkill::disk::OwningWorld,again)&&again==world_&&cfgprobe::CaptureAncestry(r,level,type)&&
            type.Contains(levelType_)&&cfgprobe::StableAncestry(r,level,type)&&
            r.Read(level+cfgradarworld::disk::LevelActors,actors)&&actors.Sane(cfgkillerstart::MaximumActors)&&
            r.Read(levels_.data+8*index,again)&&again==level;
    }
    template<class R> bool Inspect(R& r,std::uintptr_t actor,std::uintptr_t level,cfgprobe::lifetime::SlotObservation& slot){
        std::uintptr_t outer=0,cdo=0;cfgprobe::lifetime::ArrayHeader table{};cfgprobe::Ancestry type;
        return cfgprobe::CaptureAncestry(r,actor,type)&&type.Contains(klass_)&&cfgprobe::StableAncestry(r,actor,type)&&
            r.Read(actor+0x20,outer)&&outer==level&&r.Read(type.classes[0]+0xF8,cdo)&&cdo!=actor&&
            cfgprobe::lifetime::ReadArrayHeader(r,module_+cfgprobe::lifetime::ObjectArrayRva,table)&&
            cfgprobe::lifetime::ReadSlot(r,table,actor,slot);
    }
public:
    State CurrentState()const{return state_;}unsigned Count()const{return count_;}
    template<class R,class Host> bool Begin(R& r,Host& host,std::uintptr_t module,std::uintptr_t world){
        if(state_!=State::Empty||!host.Current())return Refuse();module_=module;world_=world;
        std::uintptr_t ctor=0;
        if(!r.Read(module+ClassCache,klass_)||!r.Read(module+ConstructorCache,ctor)||
            !klass_||klass_!=ctor||!r.Read(module+cfgradarworld::disk::LevelClassCache,levelType_)||!levelType_||
            !r.Read(world+cfgradarworld::disk::WorldLevels,levels_)||!levels_.Sane(cfgkillerstart::MaximumLevels)||!levels_.count||
            !Classes(r)||!host.Current())return Refuse();state_=State::Scanning;return true;
    }
    template<class R,class Host> bool Step(R& r,Host& host){
        if(state_!=State::Scanning||!host.Current()||!Classes(r)||!Levels(r))return Refuse();
        cfgspawn::TypeObservations<R> types(r);
        unsigned quota=ActorsPerStep;
        while(quota&&level_<static_cast<unsigned>(levels_.count)&&count_<MaximumMarkers){
            std::uintptr_t level=0;Header actors{};if(!Level(r,level_,level,actors))return Refuse();
            if(index_>=static_cast<unsigned>(actors.count)){++level_;index_=0;continue;}
            // One level proof per bounded batch, not once per actor. Source
            // headers/cells are still checked before and after the batch.
            while(quota&&index_<static_cast<unsigned>(actors.count)&&count_<MaximumMarkers){
                const auto cell=actors.data+8*index_;std::uintptr_t actor=0,again=0;
                if(!r.Read(cell,actor)||!r.Read(cell,again)||actor!=again)return Refuse();
                const auto index=index_++;--quota;if(!actor)continue;
                if(!types.ObjectIs(actor,klass_))continue;
                Marker marker{actor,level,level_,index,{}};
                if(!Inspect(r,actor,level,marker.slot)||!r.Read(cell,again)||again!=actor)return Refuse();
                bool duplicate=false;for(unsigned i=0;i<count_;++i)duplicate|=markers_[i].actor==actor;
                if(!duplicate)markers_[count_++]=marker;
            }
            std::uintptr_t freshLevel=0;Header freshActors{};
            if(!Level(r,level_,freshLevel,freshActors)||freshLevel!=level||!(freshActors==actors))return Refuse();
        }
        if(!types.Revalidate()||!Classes(r)||!Levels(r)||!host.Current())return Refuse();
        if(count_==MaximumMarkers||level_==static_cast<unsigned>(levels_.count))state_=count_?State::Ready:State::Refused;
        return state_!=State::Refused;
    }
    template<class R,class Host> bool Current(R& r,Host& host,unsigned choice,Marker& out){
        out={};if(state_!=State::Ready||choice>=count_||!host.Current()||!Classes(r)||!Levels(r))return false;
        auto& saved=markers_[choice];std::uintptr_t level=0,actor=0;Header actors{};cfgprobe::lifetime::SlotObservation a{},b{};
        if(!Level(r,saved.levelIndex,level,actors)||level!=saved.level||saved.index>=static_cast<unsigned>(actors.count)||
            !r.Read(actors.data+8*saved.index,actor)||actor!=saved.actor||!Inspect(r,actor,level,a)||!Inspect(r,actor,level,b)||!(a==b)||
            a.object!=saved.slot.object||a.index!=saved.slot.index||(saved.slot.serial&&saved.slot.serial!=a.serial)||
            !r.Read(actors.data+8*saved.index,actor)||actor!=saved.actor||!Classes(r)||!Levels(r)||!host.Current())return false;
        saved.slot=a;out=saved;return true;
    }
};
}
