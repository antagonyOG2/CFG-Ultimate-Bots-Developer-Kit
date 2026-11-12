#pragma once
#include "../Core/OfflineSettingsSelection.hpp"
#include "../Core/GameplayConfiguration.hpp"
#include "../RuntimeProbe/OfflineSettingsBindings.generated.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include "../RuntimeProbe/OfflineIntroMetadataBindings.generated.hpp"
namespace cfgofflinesettings {
inline Request FromConfiguration(const cfgbundle::Configuration& config){
    if(!cfgbundle::ValidConfiguration(config,31))return {};
    return {config.submission,config.identity.world,config.identity.pawn,config.identity.worldSerial,config.identity.pawnSerial,
        config.matchSeconds,config.nativeCounselorSkill};
}
enum class Write {Changed,Refused,Indeterminate};
struct Context {std::uintptr_t module=0,mode=0,gameState=0;bool nativeOffline=false;};
struct Snapshot {
    std::int32_t round=0,remaining=0,elapsed=0;cfgnames::Words phase{};
    std::uint8_t skill=3;bool skillPhase=false;
    std::array<cfgprobe::lifetime::SlotObservation,4> identities{};
};
// Host.Current(request) proves current native owner thread, no GC/serialization,
// exact captured human/world ownership and successful menu handoff. Host.Exchange
// is one aligned compare/exchange of the supplied current instance int32 only.
// This adapter never touches class defaults, profiles, weather or human control.
template<class Reader,class Host> class NativeAccess {
    Reader& r_;Host& host_;Request request_;Context context_;Snapshot observed_{};bool observedReady_=false;
    std::uintptr_t observedModeType_=0,observedStateType_=0,observedStateBase_=0;
    template<class T> bool Same(std::uintptr_t at,const T& expected){T actual{};return r_.Read(at,actual)&&actual==expected;}
    bool Typed(std::uintptr_t object,std::uintptr_t type){cfgprobe::Ancestry a;
        return cfgprobe::CaptureAncestry(r_,object,a)&&a.Contains(type)&&cfgprobe::StableAncestry(r_,object,a);}
    bool Pair(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& type){std::uintptr_t c=0;
        return r_.Read(context_.module+reflection,type)&&r_.Read(context_.module+constructor,c)&&cfgprobe::MatchingClassCache(type,c);}
    template<std::size_t N> bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& expected){std::array<unsigned char,N> actual{};
        return r_.Read(context_.module+rva,actual)&&actual==expected;}
public:
    NativeAccess(Reader& reader,Host& host,const Request& request,Context context):r_(reader),host_(host),request_(request),context_(context){}
    bool Observe(Snapshot& out,bool& inProgress){
        out={};inProgress=false;observedReady_=false;
        if(!request_.Valid()||(!request_.seconds&&request_.nativeSkill==3)||!host_.Current(request_)||!cfgprobe::ObjectPointer(context_.module)||
           !cfgprobe::ObjectPointer(context_.mode)||!cfgprobe::ObjectPointer(context_.gameState))return false;
        std::uintptr_t mode=0,sandbox=0,state=0,base=0,modeType=0,stateType=0,defaults=0;
        const auto familyCache=context_.nativeOffline?cfgmenu::offlinelifecycledisk::OfflineBotsClass:disk::SCGameMode_SandboxClass;
        if(!Pair(disk::SCGameModeClass,disk::SCGameModeConstructor,mode)||
           !Pair(familyCache,familyCache+8,sandbox)||
           !Pair(disk::SCGameStateClass,disk::SCGameStateConstructor,state)||
           !Pair(disk::GameModeClass,disk::GameModeConstructor,base)||
           !Typed(context_.mode,mode)||!Typed(context_.mode,sandbox)||!Typed(context_.mode,base)||!Typed(context_.gameState,state)||
           !r_.Read(context_.mode+cfgprobe::ObjectClass,modeType)||!r_.Read(modeType+0xF8,defaults)||defaults==context_.mode||
           !r_.Read(context_.gameState+cfgprobe::ObjectClass,stateType)||!r_.Read(stateType+0xF8,defaults)||defaults==context_.gameState||
           !Same(request_.world+cfgprobe::WorldNetDriver,std::uintptr_t{})||
           !Same(request_.world+cfgprobe::WorldGameMode,context_.mode)||!Same(request_.world+cfgprobe::WorldGameState,context_.gameState)||
           !Code(disk::TimerCopy,disk::TimerCopyCode)||!Code(disk::TimerDecrement,disk::TimerDecrementCode)||!Code(disk::ElapsedIncrement,disk::ElapsedIncrementCode))return false;
        std::uintptr_t nativeState=0;
        const auto nativeFamily=[&](){if(!context_.nativeOffline)return true;std::uintptr_t actual=0;
            if(!Pair(cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass,cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass+8,actual)||
               (nativeState&&nativeState!=actual)||modeType!=sandbox||stateType!=actual||
               !Same(context_.mode,context_.module+cfgmenu::offlinelifecycledisk::OfflineBotsVtable)||!Same(context_.mode+0x3C0,context_.gameState))return false;
            nativeState=actual;return true;};
        if(!nativeFamily())return false;
        cfgprobe::lifetime::ArrayHeader table;Snapshot value;
        const std::array<std::uintptr_t,4> objects{{request_.world,request_.pawn,context_.mode,context_.gameState}};
        if(!cfgprobe::lifetime::ObserveBatch(r_,context_.module+cfgprobe::lifetime::ObjectArrayRva,objects,table,value.identities))return false;
        // World/pawn are the durable handoff identities. Strongly owned native
        // mode/state slots may have no weak serial yet; observe their exact slots
        // twice under the caller's no-GC owner-thread gate, never assign serials.
        if(!value.identities[0].HasAssignedSerial()||!value.identities[1].HasAssignedSerial())return false;
        if(value.identities[0].serial!=request_.worldSerial||value.identities[1].serial!=request_.pawnSerial||
           !r_.Read(context_.mode+disk::MatchState,value.phase)||value.phase.number||
           !r_.Read(context_.mode+disk::RoundTime,value.round)||!r_.Read(context_.gameState+disk::RemainingTime,value.remaining)||
           !r_.Read(context_.gameState+disk::ElapsedInProgressTime,value.elapsed))return false;
        cfgnames::Observation<Reader> names(r_,context_.module);cfgnames::Text text;
        if(!names.Decode(value.phase,text))return false;
        if(request_.nativeSkill<3&&(!Code(disk::SkillConsumer,disk::SkillConsumerCode)||
           !r_.Read(context_.mode+disk::CounselorSkill,value.skill)||value.skill>2))return false;
        value.skillPhase=text.View()==u"WaitingToStart"||text.View()==u"InProgress";
        // Only the native InProgress name authorizes writes. Every other
        // valid native phase may wait within the same bounded arrival window.
        if(!Same(context_.mode+disk::MatchState,value.phase)||!Same(context_.mode+disk::RoundTime,value.round)||
           !Same(context_.gameState+disk::RemainingTime,value.remaining)||!Same(context_.gameState+disk::ElapsedInProgressTime,value.elapsed)||
           !Same(context_.mode+cfgprobe::ObjectClass,modeType)||!Same(context_.gameState+cfgprobe::ObjectClass,stateType)||
           !Same(context_.module+disk::SCGameModeClass,mode)||!Same(context_.module+disk::SCGameModeConstructor,mode)||
           !Same(context_.module+familyCache,sandbox)||!Same(context_.module+familyCache+8,sandbox)||
           !Same(context_.module+disk::SCGameStateClass,state)||!Same(context_.module+disk::SCGameStateConstructor,state)||
           !Same(context_.module+disk::GameModeClass,base)||!Same(context_.module+disk::GameModeConstructor,base)||
           (request_.nativeSkill<3&&!Same(context_.mode+disk::CounselorSkill,value.skill))||
           !cfgprobe::lifetime::RevalidateBatch(r_,context_.module+cfgprobe::lifetime::ObjectArrayRva,table,value.identities)||!host_.Current(request_))return false;
        if(!nativeFamily())return false;out=value;inProgress=text.View()==u"InProgress";
        observed_=value;observedReady_=inProgress;observedModeType_=modeType;observedStateType_=stateType;observedStateBase_=state;return true;
    }
    // Optional paced native-Jason consumer: one timer field per callback.
    // Observe must have succeeded on THIS access instance/callback. Re-read
    // mutable values, identities and full host ownership immediately before
    // and after the single typed write. No retained unassigned mode/state is used.
    Write ApplyPart(const Snapshot& captured,bool remainingOnly){
        const auto& fresh=observed_;std::int32_t remaining=0;
        if(!observedReady_||fresh.round!=captured.round||fresh.remaining!=captured.remaining||fresh.elapsed!=captured.elapsed||
           !(fresh.phase==captured.phase)||fresh.identities!=captured.identities||
           !Reconcile(request_.seconds,fresh.elapsed,fresh.remaining,remaining)||fresh.round < -1||fresh.round>86400||
           (remainingOnly&&fresh.round!=static_cast<std::int32_t>(request_.seconds)))return Write::Refused;
        observedReady_=false;
        cfgprobe::lifetime::ArrayHeader table;
        const auto typesCurrent=[&](){
            return Same(context_.mode+cfgprobe::ObjectClass,observedModeType_)&&Same(context_.gameState+cfgprobe::ObjectClass,observedStateType_)&&
                Same(context_.module+disk::SCGameStateClass,observedStateBase_)&&Same(context_.module+disk::SCGameStateConstructor,observedStateBase_)&&
                (!context_.nativeOffline||(Same(context_.module+cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass,observedStateType_)&&
                  Same(context_.module+cfgmenu::intrometadatadisk::SCGameState_OfflineBotsClass+8,observedStateType_)));};
        if(!host_.Current(request_)||!typesCurrent()||!cfgprobe::lifetime::ReadArrayHeader(r_,context_.module+cfgprobe::lifetime::ObjectArrayRva,table)||
           !cfgprobe::lifetime::RevalidateBatch(r_,context_.module+cfgprobe::lifetime::ObjectArrayRva,table,fresh.identities)||
           !Same(context_.mode+disk::MatchState,fresh.phase)||!Same(context_.mode+disk::RoundTime,fresh.round)||
           !Same(context_.gameState+disk::RemainingTime,fresh.remaining)||!Same(context_.gameState+disk::ElapsedInProgressTime,fresh.elapsed))return Write::Refused;
        const auto at=remainingOnly?context_.gameState+disk::RemainingTime:context_.mode+disk::RoundTime;
        const auto old=remainingOnly?fresh.remaining:fresh.round;
        const auto desired=remainingOnly?remaining:static_cast<std::int32_t>(request_.seconds);
        const auto result=old==desired?Write::Changed:host_.Exchange(at,old,desired);
        if(result!=Write::Changed)return result;
        return host_.Current(request_)&&typesCurrent()&&Same(context_.mode+disk::MatchState,fresh.phase)&&
            Same(context_.gameState+disk::ElapsedInProgressTime,fresh.elapsed)&&Same(at,desired)?Write::Changed:Write::Indeterminate;
    }
    Write ApplySkill(){
        // BeforeHelpers must prove this arrival still precedes ALL owned helper
        // construction. A late write would leave cached AI startup settings mixed.
        Snapshot fresh;bool ready=false;
        if(request_.nativeSkill>2||!host_.BeforeHelpers(request_)||!Observe(fresh,ready)||!fresh.skillPhase||
           fresh.elapsed<0||fresh.elapsed>static_cast<int>(MaximumArrivalElapsedSeconds)||!host_.BeforeHelpers(request_))return Write::Refused;
        if(fresh.skill==request_.nativeSkill)return Write::Changed;
        const auto result=host_.ExchangeByte(context_.mode+disk::CounselorSkill,fresh.skill,static_cast<std::uint8_t>(request_.nativeSkill));
        if(result!=Write::Changed)return result;
        if(!host_.Current(request_)||!host_.BeforeHelpers(request_)||!Same(context_.mode+disk::MatchState,fresh.phase)||
           !Same(context_.mode+disk::CounselorSkill,static_cast<std::uint8_t>(request_.nativeSkill)))return Write::Indeterminate;
        return Write::Changed;
    }
    Write Apply(const Snapshot& captured){
        Snapshot fresh;bool ready=false;std::int32_t remaining=0;
        if(!Observe(fresh,ready)||!ready||fresh.round!=captured.round||fresh.remaining!=captured.remaining||fresh.elapsed!=captured.elapsed||
           !(fresh.phase==captured.phase)||fresh.identities!=captured.identities||
           !Reconcile(request_.seconds,fresh.elapsed,fresh.remaining,remaining)||fresh.round < -1||fresh.round>86400)return Write::Refused;
        // Both writes are consumed once. Never retry after an ambiguous write;
        // partial application is reported, not disguised as a successful setting.
        const auto first=host_.Exchange(context_.mode+disk::RoundTime,fresh.round,static_cast<std::int32_t>(request_.seconds));
        if(first!=Write::Changed)return first;
        if(!host_.Current(request_)||!Same(context_.mode+disk::MatchState,fresh.phase)||
           !Same(context_.gameState+disk::ElapsedInProgressTime,fresh.elapsed))return Write::Indeterminate;
        const auto second=host_.Exchange(context_.gameState+disk::RemainingTime,fresh.remaining,remaining);
        if(second!=Write::Changed||!host_.Current(request_)||!Same(context_.mode+disk::RoundTime,static_cast<std::int32_t>(request_.seconds))||
           !Same(context_.gameState+disk::RemainingTime,remaining))return Write::Indeterminate;
        return Write::Changed;
    }
};
// Run this once in the early-arrival lane BEFORE helper creation; duration may
// subsequently wait for InProgress independently. No automatic retry on failure.
class SkillRuntime {
    State state_=State::Empty;
public:
    template<class Access> bool Apply(Access& access,const Request& request){
        if(state_!=State::Empty||!request.Valid())return false;
        if(request.nativeSkill==3){state_=State::Preserved;return true;}
        state_=State::Applying;const auto result=access.ApplySkill();
        state_=result==Write::Changed?State::Applied:result==Write::Refused?State::Refused:State::Indeterminate;
        return state_==State::Applied;
    }
    State CurrentState()const{return state_;}
};
class Runtime {
    State state_=State::Empty;Request request_{};std::uint64_t next_=0,deadline_=0;
    cfgnames::Words partPhase_{};bool partCalling_=false;
public:
    unsigned partFailure=0;
    bool Begin(const Request& request,std::uint64_t now){
        if(state_!=State::Empty||!request.Valid())return false;request_=request;next_=now;deadline_=now+30000;
        state_=request.seconds?State::Waiting:State::Preserved;return true;
    }
    template<class Access> bool Step(Access& access,std::uint64_t now){
        if(state_!=State::Waiting||now<next_)return false;
        if(now>deadline_){state_=State::Refused;return false;}next_=now+500;
        Snapshot snapshot;bool ready=false;
        if(!access.Observe(snapshot,ready)){state_=State::Refused;return false;}
        if(!ready)return false;
        std::int32_t remaining=0;if(!Reconcile(request_.seconds,snapshot.elapsed,snapshot.remaining,remaining)){state_=State::Refused;return false;}
        state_=State::Applying;const auto result=access.Apply(snapshot);
        state_=result==Write::Changed?State::Applied:result==Write::Refused?State::Refused:State::Indeterminate;
        return state_==State::Applied;
    }
    State CurrentState()const{return state_;}
    // Same ApplyPart path already used by NativeHumanJasonOptionsRuntime:
    // round-time and remaining-time each get their OWN fresh callback proof.
    // Never replay a partial/ambiguous transaction; retain its failure state.
    template<class Access> bool StepParts(Access& access,std::uint64_t now){
        if(partCalling_||(state_!=State::Waiting&&state_!=State::Applying)||now<next_)return false;
        const bool partial=state_==State::Applying;
        const auto refuse=[&](unsigned why){partFailure=why;state_=partial?State::Indeterminate:State::Refused;return false;};
        if(now>deadline_)return refuse(1);
        next_=now+500;Snapshot snapshot;bool ready=false;
        if(!access.Observe(snapshot,ready))return refuse(2);
        if(!ready)return partial?refuse(3):false;
        std::int32_t remaining=0;
        if(!Reconcile(request_.seconds,snapshot.elapsed,snapshot.remaining,remaining))return refuse(4);
        if(partial&&!(snapshot.phase==partPhase_))return refuse(5);
        state_=State::Applying;partCalling_=true;
        const auto result=access.ApplyPart(snapshot,partial);partCalling_=false;
        if(result==Write::Indeterminate){partFailure=6;state_=State::Indeterminate;return false;}
        if(result!=Write::Changed)return refuse(7);
        partPhase_=snapshot.phase;state_=partial?State::Applied:State::Applying;return partial;
    }
    const Request& Committed()const{return request_;}
};
}
