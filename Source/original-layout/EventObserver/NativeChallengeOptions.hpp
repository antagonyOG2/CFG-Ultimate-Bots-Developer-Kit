#pragma once
#include "NativeHumanJasonOptionsRuntime.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/MenuOptionsObservation.hpp"
#include "../RuntimeProbe/Compatibility/CfgBuildProfile.hpp"
#include <string>
#include <vector>
namespace cfgmenu::challengeoptions {
struct Options{bool knives=false,recharge=false,movement=false,stealth=false,untimed=false;};
inline bool Any(const Options& o){return o.knives||o.recharge||o.movement||o.stealth||o.untimed;}
// J28: -1 = accept the reflected property offset (name, kind, dimension and size are still pinned).
constexpr int kReflectedOffset(int literal)noexcept{
#if defined(CFG_J28_CHALLENGE_35_BINDINGS) && CFG_J28_CHALLENGE_35_BINDINGS
    return (void)literal,-1;
#else
    return literal;
#endif
}
inline bool StockClass(std::u16string_view name){
    constexpr std::array<std::u16string_view,10> allowed{{u"ChallengeC01_BrokenDown_C",u"ChallengeC02_PackanackParty_C",
        u"ChallengeC03_Stargazing_C",u"ChallengeC04_PowerStruggle_C",u"ChallengeC05_StripPoker_C",u"ChallengeC06_PowerOut_C",
        u"ChallengeC07_VacationParty_C",u"ChallengeC08_Escaping_C",u"ChallengeC09_JasonIsHere_C",u"ChallengeC10_SnuggleByTheFire_C"}};
    return std::find(allowed.begin(),allowed.end(),name)!=allowed.end();
}
// A distinct standalone-Challenges provider. It never relaxes OfflineBots
// ownership or touches counselor/online modes. Donor's five audited capabilities
// are applied only to the possessed primary Jason and submitted stock mission.
class Runtime {
    freshgame::Source source_{};Options options_{};std::uint64_t next_=0,expires_=0;
    std::uintptr_t hint_=0;bool armed_=false,bound_=false;
    std::array<cfgprobe::lifetime::SlotObservation,6> identities_{};
    cfgnames::Words map_{},mission_{};
    struct Owned {std::uintptr_t at=0;std::array<unsigned char,20> before{},applied{};unsigned bytes=0;};
    std::array<Owned,5> owned_{};
    struct Member {std::uintptr_t type=0,head=0;std::u16string name{};cfgreflection::Property property{};};
    std::vector<Member> members_;
    // Native killer has >256 declared fields. Resolve the requested native
    // member along its current class chain, not the generic UI's 256-field
    // full census. Cache no unvalidated raw offset or zero-serial ownership.
    template<class R>bool Find(R& r,std::uintptr_t type,std::u16string_view wanted,cfgreflection::Field& out){
        cfgreflection::Observation<R> reflection(r,source_.module);cfgnames::Observation<R> names(r,source_.module);
        std::array<std::uintptr_t,512> seen{},links{};unsigned count=0;
        for(unsigned depth=0;type&&depth<32;++depth){std::uintptr_t head=0,child=0;
            if(!r.Read(type+cfgreflection::disk::Children,head))return false;child=head;
            while(child){if(count==seen.size())return false;
                for(unsigned i=0;i<count;++i)if(seen[i]==child)return false;
                cfgnames::Text label;std::uintptr_t next=0;
                if(!r.Read(child+cfgreflection::disk::Next,next)||!names.Object(child,label))return false;
                seen[count]=child;links[count++]=next;
                if(label.View()==wanted){
                    if(!reflection.ReadField(child,type,out)||!offlineowner::Same(r,type+cfgreflection::disk::Children,head))return false;
                    for(unsigned i=0;i<count;++i)if(!offlineowner::Same(r,seen[i]+cfgreflection::disk::Next,links[i]))return false;
                    return true;
                }
                child=next;
            }
            if(!r.Read(type+cfgprobe::ClassSuper,type))return false;
        }return false;
    }
    template<class R>bool Property(R& r,const cfgreflection::Field& field,cfgreflection::Property& out){
        cfgreflection::Observation<R> reflection(r,source_.module);cfgreflection::Field fresh;
        cfgreflection::Property first,second;
        if(!reflection.ReadField(field.object,field.owner,fresh)||fresh.type!=field.type||!(fresh.name==field.name)||fresh.next!=field.next||
           !capture::RawProperty(r,fresh,first)||!capture::RawProperty(r,fresh,second)||!capture::SameProperty(first,second))return false;
        out=second;return true;
    }
    template<class R>bool Field(R& r,std::uintptr_t object,std::u16string_view name,std::u16string_view kind,int offset,int dim,int size,std::uintptr_t& at){
        at=0;std::uintptr_t type=0;if(!r.Read(object+cfgprobe::ObjectClass,type))return false;
        Member* member=nullptr;
        for(auto& item:members_)if(item.type==type&&item.name==name){member=&item;break;}
        if(!member){cfgreflection::Field f;cfgreflection::Property p;
            if(members_.size()>=16||!Find(r,type,name,f)||!Property(r,f,p)||f.kind.View()!=kind||
               (offset>=0&&p.offset!=offset)||(offset<0&&(p.offset<=0||p.offset>=0x0/*CFG3.5 address withheld*/))||p.dimension!=dim||p.elementBytes!=size)return false;
            std::uintptr_t head=0;if(!r.Read(p.field.owner+cfgreflection::disk::Children,head))return false;
            members_.push_back({type,head,std::u16string(name),p});member=&members_.back();}
        cfgreflection::Property fresh;cfgprobe::Ancestry ancestry;
        const auto& p=member->property;
        if(!cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(p.field.owner)||!cfgprobe::StableAncestry(r,object,ancestry)||
           !offlineowner::Same(r,p.field.owner+cfgreflection::disk::Children,member->head)||
           !Property(r,p.field,fresh)||!capture::SameProperty(p,fresh)||p.field.label.View()!=name||p.field.kind.View()!=kind||
           (offset>=0&&p.offset!=offset)||(offset<0&&(p.offset<=0||p.offset>=0x0/*CFG3.5 address withheld*/))||p.dimension!=dim||p.elementBytes!=size)return false;
        at=object+static_cast<unsigned>(offset>=0?offset:p.offset);return true;
    }
    template<class R>bool Typed(R& r,std::uintptr_t object,std::uintptr_t cache,bool exact=false){
        std::uintptr_t type=0,ctor=0,actual=0,defaults=0;
        return cfgprobe::ObjectPointer(object)&&r.Read(source_.module+cache,type)&&r.Read(source_.module+cache+8,ctor)&&
            cfgprobe::MatchingClassCache(type,ctor)&&r.Read(object+cfgprobe::ObjectClass,actual)&&
            (exact?actual==type:maps::Subclass(r,actual,type))&&r.Read(actual+cfgspawn::ClassDefaultObject,defaults)&&
            cfgprobe::ObjectPointer(defaults)&&defaults!=object;
    }
    template<class R,class W,class T,class Current>bool Apply(R&,W& writer,unsigned option,std::uintptr_t at,const T& old,const T& value,Current current){
        static_assert(sizeof(T)<=20);auto& owned=owned_[option];if(owned.at)return true;
        if(!current())return false;owned.at=at;owned.bytes=sizeof(T);
        std::memcpy(owned.before.data(),&old,sizeof(T));std::memcpy(owned.applied.data(),&value,sizeof(T));
        if(!writer.Write(at,old,value)){failure=6;armed_=false;return false;}
        ++writes[option];return current();
    }
    template<class R,class W,class Current>void Restore(R& r,W& writer,Current current){
        for(unsigned i=0;i<owned_.size();++i){auto& o=owned_[i];if(!o.at||!current())continue;
            std::uintptr_t at=0;const auto pawn=identities_[3].object;
            if(i==0&&(!Field(r,pawn,u"NumKnives",u"IntProperty",0x169C,1,4,at)||at!=o.at))continue;
            if(i==1&&(!Field(r,pawn,u"AbilityRechargeTime",u"FloatProperty",0x129C,5,4,at)||at!=o.at))continue;
            if(i==3){bool valid=true;const std::array<std::u16string_view,4> names{{u"StealthNoiseScale",u"StealthRadiusScale",u"AntiStealthNoiseScale",u"AntiStealthRadiusScale"}};
                for(unsigned j=0;j<4;++j)valid&=Field(r,pawn,names[j],u"FloatProperty",0x18E4+4*j,1,4,at)&&at==o.at+4*j;
                if(!valid)continue;}
            if(i==2){std::uintptr_t movement=0;
                if(!r.Read(identities_[3].object+0x3D0,movement)||!Typed(r,movement,cfgcompat::cfg35::objects::SCCharacterMovementReflection)||
                   !offlineowner::Same(r,movement+0x20,identities_[3].object)||o.at!=movement+0x720||
                   !Field(r,movement,u"MaxSprintSpeed",u"FloatProperty",0x720,1,4,at)||at!=o.at||
                   !Field(r,movement,u"MaxRunSpeed",u"FloatProperty",0x724,1,4,at)||at!=o.at+4)continue;}
            std::array<unsigned char,20> actual{};if(!r.ReadBytes(o.at,actual.data(),o.bytes)||std::memcmp(actual.data(),o.applied.data(),o.bytes))continue;
            // Typed compare-before-write; only our own unchanged field is restored.
            if(o.bytes==4){int before=0,applied=0;std::memcpy(&before,o.before.data(),4);std::memcpy(&applied,o.applied.data(),4);
                (void)writer.Write(o.at,applied,before);}
            else if(o.bytes==8){std::array<float,2> before{},applied{};std::memcpy(before.data(),o.before.data(),8);std::memcpy(applied.data(),o.applied.data(),8);
                (void)writer.Write(o.at,applied,before);}
            else if(o.bytes==16){std::array<float,4> before{},applied{};std::memcpy(before.data(),o.before.data(),16);std::memcpy(applied.data(),o.applied.data(),16);
                (void)writer.Write(o.at,applied,before);}
        }
        owned_={}; // RemainingTime is deliberately NOT restored on lifecycle exit.
    }
public:
    unsigned failure=0,bound=0,retired=0,peakReads=0;std::array<unsigned,5> writes{};
    bool CancelPending()noexcept{
        if(bound_)return false;
        for(const auto& field:owned_)if(field.at)return false;
        armed_=false;options_={};next_=expires_=0;hint_=0;members_.clear();return true;
    }
    bool Arm(const active::Context& source,Options choices,std::uint64_t now){
        if(armed_||now>UINT64_MAX-90000)return false;
        source_={source.module,source.engine,source.viewport,source.player,source.gameInstance,source.world,source.thread,false};
        options_=choices;armed_=Any(choices);expires_=now+90000;next_=0;hint_=0;bound_=false;identities_={};map_={};mission_={};
        members_.clear();owned_={};failure=bound=retired=0;writes={};return true;
    }
    bool Armed()const{return armed_;}
    bool Departed(const cfgprobe::lifetime::SlotObservation& world){
        if(!world.HasAssignedSerial()||(bound_&&world==identities_[0]))return false;
        // Only per-world actor fields are owned here. A positively different
        // world retires them without dereferencing the departed objects.
        *this=Runtime{};return true;
    }
    template<class R>bool Callback(R& r,std::uintptr_t object,std::uint64_t now){
        if(!armed_||now<next_)return false;std::uintptr_t world=0;
        // Menu return can have no pawn callbacks at all. Retire from a fresh
        // viewport-world observation rather than blocking the next submission
        // waiting for a callback on the departed pawn. Never write old actors.
        if(!r.Read(source_.viewport+cfgprobe::ViewportWorld,world))return false;
        if((bound_&&world!=identities_[0].object)||(!bound_&&now>=expires_)){armed_=false;retired=1;return false;}
        std::uintptr_t controller=0;hint_=0;
        return r.Read(source_.player+cfgprobe::PlayerController,controller)&&cfgprobe::ObjectPointer(controller)&&
            r.Read(controller+cfgprobe::ControllerPawn,hint_)&&hint_==object&&cfgprobe::ObjectPointer(hint_);
    }
    template<class R>void Step(R& r,std::uintptr_t callback,std::uint32_t thread,std::uint64_t now){
        struct Writer{R& reader;bool Write(std::uintptr_t at,const int& old,const int& value){return humanjason::Write(reader,at,old,value);}
            bool Write(std::uintptr_t at,const std::array<float,2>& old,const std::array<float,2>& value){return humanjason::Write(reader,at,old,value);}
            bool Write(std::uintptr_t at,const std::array<float,4>& old,const std::array<float,4>& value){return humanjason::Write(reader,at,old,value);}} writer{r};
        Step(r,callback,thread,now,writer);
    }
    template<class R,class W>void Step(R& r,std::uintptr_t callback,std::uint32_t thread,std::uint64_t now,W& writer){
        if(!armed_||thread!=source_.thread||now<next_)return;next_=now+100;
        using namespace cfgprobe;const auto same=[&](std::uintptr_t at,auto expected){return offlineowner::Same(r,at,expected);};
        std::uintptr_t world=0,mode=0,controller=0,pawn=0,state=0,ps=0,players=0;int count=0,cap=0;
        if(!r.Read(source_.viewport+ViewportWorld,world))return;
        if(bound_&&world!=identities_[0].object){armed_=false;retired=1;return;}
        if(world==source_.frontendWorld)return;
        if(!bound_&&now>=expires_){armed_=false;retired=1;failure=1;return;}
        failure=2;
        if(!offlineowner::ChallengeLookupSafe(r,source_.module)||!same(source_.module+dispatch::ThreadFlagRva,std::uint8_t{1})||!same(source_.module+dispatch::ThreadIdRva,thread)||
           !same(source_.module+EngineGlobalCandidate,source_.engine)||!same(source_.engine+EngineGameViewport,source_.viewport)||
           !same(world+WorldNetDriver,std::uintptr_t{})||!same(world+WorldGameInstance,source_.gameInstance)||
           !r.Read(world+WorldGameMode,mode)||!Typed(r,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)||!same(mode+0x110,std::uint8_t{3})||
           !r.Read(source_.gameInstance+GameInstanceLocalPlayers,players)||!ObjectPointer(players)||
           !r.Read(source_.gameInstance+GameInstanceLocalPlayers+8,count)||count!=1||
           !r.Read(source_.gameInstance+GameInstanceLocalPlayers+12,cap)||cap<1||cap>4||!same(players,source_.player)||
           !r.Read(source_.player+PlayerController,controller)||!Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)||
           !same(controller+ControllerPlayer,source_.player)||!r.Read(controller+ControllerPawn,pawn)||pawn!=callback||
           !Typed(r,pawn,NativeClasses[Killer].reflectionCacheRva)||!same(pawn+0x110,std::uint8_t{3})||!r.Read(controller+0x388,ps)||!ObjectPointer(ps)||
           !same(pawn+0x3A0,controller)||!same(pawn+0x388,ps)||!same(ps+0x88,controller)||
           !r.Read(world+WorldGameState,state)||!Typed(r,state,cfgcompat::cfg35::objects::SCGameStateReflection)||!same(state+0x420,pawn)||!same(state+0x490,ps))return;
        std::array<std::uintptr_t,6> objects{{world,mode,controller,pawn,state,ps}};
        lifetime::ArrayHeader registry;std::array<lifetime::SlotObservation,6> identities{};
        if(!lifetime::ObserveBatch(r,source_.module+lifetime::ObjectArrayRva,objects,registry,identities))return;
        for(const auto& identity:identities)if(!identity.HasAssignedSerial())return;
        if(bound_&&!(identities==identities_)){armed_=false;retired=1;failure=3;return;}
        cfgnames::Observation<R> names(r,source_.module);cfgnames::Text label,package;
        std::uintptr_t stateType=0,dossierAt=0,dossier=0,missionAt=0,klass=0,outer=0,worldOuter=0;
        failure=4;
        if(!r.Read(state+ObjectClass,stateType)||!names.Object(stateType,label)||label.View()!=u"SCGameState_SPChallenges"||
           !Field(r,state,u"Dossier",u"ObjectProperty",kReflectedOffset(0x5B8),1,8,dossierAt)||!r.Read(dossierAt,dossier)||!ObjectPointer(dossier)||
           !Field(r,dossier,u"ActiveChallengeClass",u"ClassProperty",kReflectedOffset(0x58),1,8,missionAt)||!r.Read(missionAt,klass)||!ObjectPointer(klass)||
           !names.Object(klass,label)||!StockClass(label.View())||!r.Read(klass+0x20,outer)||!names.Object(outer,package)||
           package.View().find(u"/Game/Blueprints/MapRegistry/Challenges/")!=0||
           !r.Read(world+0x20,worldOuter)||!names.Object(worldOuter,package)||package.View().find(u"/Game/Maps/Single_Player/")!=0)return;
        cfgnames::Words map{},mission{};if(!r.Read(worldOuter+cfgnames::disk::ObjectName,map)||!r.Read(klass+cfgnames::disk::ObjectName,mission))return;
        if(bound_&&(!(map==map_)||!(mission==mission_))){armed_=false;retired=1;failure=5;return;}
        const auto current=[&](){return offlineowner::ChallengeLookupSafe(r,source_.module)&&lifetime::RevalidateBatch(r,source_.module+lifetime::ObjectArrayRva,registry,identities)&&
            same(source_.viewport+ViewportWorld,world)&&same(world+WorldNetDriver,std::uintptr_t{})&&same(world+WorldGameMode,mode)&&
            same(source_.player+PlayerController,controller)&&same(controller+ControllerPawn,pawn)&&same(pawn+0x3A0,controller)&&
            same(pawn+0x388,ps)&&same(state+0x420,pawn)&&same(state+0x490,ps)&&same(dossierAt,dossier)&&same(missionAt,klass)&&
            Typed(r,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)&&Typed(r,pawn,NativeClasses[Killer].reflectionCacheRva);};
        if(!current())return;
        if(!bound_){bound_=true;bound=1;identities_=identities;map_=map;mission_=mission;}
        cfgnames::Words phase{};if(!r.Read(mode+0x3F8,phase)||!names.Decode(phase,label))return;
        if(label.View()!=u"InProgress"){
            if(label.View()==u"WaitingToStart"||label.View()==u"EnteringMap")return;
            Restore(r,writer,current);armed_=false;retired=1;failure=0;return;
        }
        // Same donor thresholds: preserve CFG Endure (slot4), x2 owned run/
        // sprint, x0.1 four perception endpoints, stock refill ceiling99.
        std::uintptr_t at=0;
        if(options_.recharge&&!owned_[1].at){std::array<float,4> old{},desired{};
            if(!Field(r,pawn,u"AbilityRechargeTime",u"FloatProperty",0x129C,5,4,at)||!r.Read(at,old))return;
            for(unsigned i=0;i<4;++i){if(!std::isfinite(old[i])||old[i]<=0||old[i]>3600)return;desired[i]=old[i]*0.5f;}
            if(!Apply(r,writer,1,at,old,desired,current))return;
        }
        if(options_.movement&&!owned_[2].at){std::uintptr_t movement=0,sprint=0,run=0;
            if(!r.Read(pawn+0x3D0,movement)||!Typed(r,movement,cfgcompat::cfg35::objects::SCCharacterMovementReflection)||!same(movement+0x20,pawn)||
               !Field(r,movement,u"MaxSprintSpeed",u"FloatProperty",0x720,1,4,sprint)||
               !Field(r,movement,u"MaxRunSpeed",u"FloatProperty",0x724,1,4,run))return;
            std::array<float,2> old{},desired{};if(!r.Read(sprint,old))return;
            for(unsigned i=0;i<2;++i){if(!std::isfinite(old[i])||old[i]<=0||old[i]>5000)return;desired[i]=old[i]*2;}
            const auto movementCurrent=[&](){return current()&&same(pawn+0x3D0,movement)&&same(movement+0x20,pawn)&&Typed(r,movement,cfgcompat::cfg35::objects::SCCharacterMovementReflection);};
            if(!Apply(r,writer,2,sprint,old,desired,movementCurrent))return;
        }
        if(options_.stealth&&!owned_[3].at){std::array<float,4> old{},desired{};
            const std::array<std::u16string_view,4> wanted{{u"StealthNoiseScale",u"StealthRadiusScale",u"AntiStealthNoiseScale",u"AntiStealthRadiusScale"}};
            for(unsigned i=0;i<4;++i)if(!Field(r,pawn,wanted[i],u"FloatProperty",0x18E4+4*i,1,4,at)||!r.Read(at,old[i])||
                !std::isfinite(old[i])||old[i]<0||old[i]>100)return;
            for(unsigned i=0;i<4;++i)desired[i]=old[i]*0.1f;
            if(!Apply(r,writer,3,pawn+0x18E4,old,desired,current))return;
        }
        if(options_.knives){int old=0;
            if(!Field(r,pawn,u"NumKnives",u"IntProperty",0x169C,1,4,at)||!r.Read(at,old)||old<0||old>9999||!current())return;
            if(old<99){if(!owned_[0].at){owned_[0].at=at;owned_[0].bytes=4;std::memcpy(owned_[0].before.data(),&old,4);}
                const int refill=99;std::memcpy(owned_[0].applied.data(),&refill,4);
                if(!writer.Write(at,old,refill)){armed_=false;failure=6;return;}++writes[0];}
        }
        if(options_.untimed){int old=0;
            if(!Field(r,state,u"RemainingTime",u"IntProperty",0x3FC,1,4,at)||!r.Read(at,old)||old < -1||old>86400||!current())return;
            if(old!=-1){if(!writer.Write(at,old,int{-1})){armed_=false;failure=6;return;}++writes[4];}
        }
        failure=0;
    }
};
}
