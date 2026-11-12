#pragma once
#include "CounselorTerminalObservation.hpp"
#include "../RuntimeProbe/PostMatchRecoveryBindings.generated.hpp"
namespace cfgoutro {
constexpr std::uint64_t NativeGraceMs=45000,InactiveGraceMs=3000;
enum class Action {None,Skip,Complete};
struct Recovery {
    bool observed=false,skipped=false,finished=false;std::uint64_t since=0;
    unsigned factoryAttempts=0;
    cfgprobe::lifetime::SlotObservation settings{};
    bool AttemptFactory(unsigned bit){if(finished||(bit!=1&&bit!=2)||(factoryAttempts&bit))return false;
        factoryAttempts|=bit;return true;}
    std::uint64_t Elapsed(std::uint64_t now)const{return observed&&now>=since?now-since:0;}
    Action Next(std::u16string_view phase,std::uint64_t now,bool provenInactive=false){
        if(finished)return Action::None;
        if(phase!=u"PostMatchOutro"){observed=false;since=0;return Action::None;}
        if(!observed){observed=true;since=now;return Action::None;}
        if(now<since||Elapsed(now)<(provenInactive?InactiveGraceMs:NativeGraceMs))return Action::None;
        return skipped?Action::Complete:Action::Skip;
    }
};
struct Owner {
    cfgprobe::lifetime::SlotObservation level{},settings{};
    std::uintptr_t type=0;
};
template<class R>bool ObserveOwner(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t module,std::uintptr_t world,Owner& out,unsigned* diagnostic=nullptr){
    const auto stage=[&](unsigned value){if(diagnostic)*diagnostic=value;};stage(1);
    out={};Owner p;std::uintptr_t level=0,settings=0,levelType=0,ctor=0;
    if(!r.Read(module+disk::LevelClass,levelType)||!r.Read(module+disk::LevelConstructor,ctor)||
       !cfgprobe::MatchingClassCache(levelType,ctor)||
       !r.Read(module+disk::SCWorldSettingsClass,p.type)||!r.Read(module+disk::SCWorldSettingsConstructor,ctor)||
       !cfgprobe::MatchingClassCache(p.type,ctor))return false;
    stage(2);if(!r.Read(world+disk::PersistentLevel,level)||!cfgspawn::ObjectSlot(r,registry,level,p.level)||
       !cfgspawn::ObjectIs(r,level,levelType)||!cfgobserver::Same(r,level+disk::OwningWorld,world))return false;
    stage(3);if(!r.Read(level+disk::WorldSettings,settings)||!cfgspawn::ObjectSlot(r,registry,settings,p.settings)||
       !cfgspawn::ObjectIs(r,settings,p.type)||!cfgobserver::Same(r,settings+0x20,level)||
       !cfgescape::Same(r,settings+0x110,std::uint8_t{3}))return false;
    stage(4);
    cfgprobe::lifetime::SlotObservation fresh{};
    if(!cfgobserver::Same(r,world+disk::PersistentLevel,level)||!cfgobserver::Same(r,level+disk::OwningWorld,world)||
       !cfgobserver::Same(r,level+disk::WorldSettings,settings)||!cfgobserver::Same(r,settings+0x20,level)||
       !cfgescape::Same(r,settings+0x110,std::uint8_t{3})||
       !cfgspawn::ObjectSlot(r,registry,level,fresh)||!(fresh==p.level)||
       !cfgspawn::ObjectSlot(r,registry,settings,fresh)||!(fresh==p.settings))return false;
    out=p;stage(0);return true;
}
// Only an absent sequence or a typed CURRENT player's STOPPED status (0)
// admits the earlier watchdog. Playing, paused/scrubbing/unknown or failed
// reads retain the donor45s grace. This is not a cinematic-completion claim.
template<class R>bool InactiveSequence(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t module,std::uintptr_t world,const Owner& owner){
    std::uintptr_t sequence=0,player=0,type=0,ctor=0,level=0;std::uint8_t status=255;
    cfgprobe::lifetime::SlotObservation actor{},playback{},fresh{};std::uint32_t flags=0;
    if(!r.Read(owner.settings.object+disk::LevelOutroSequence,sequence))return false;
    if(!sequence)return cfgobserver::Same(r,owner.settings.object+disk::LevelOutroSequence,std::uintptr_t{})&&
        cfgspawn::ObjectSlot(r,registry,owner.settings.object,fresh)&&fresh==owner.settings;
    if(!r.Read(module+disk::LevelSequenceActorClass,type)||!r.Read(module+disk::LevelSequenceActorConstructor,ctor)||!cfgprobe::MatchingClassCache(type,ctor)||
       !cfgspawn::ObjectSlot(r,registry,sequence,actor)||!cfgspawn::ObjectIs(r,sequence,type)||
       !r.Read(registry.data+24ULL*static_cast<unsigned>(actor.index)+8,flags)||(flags&0x30000000u)||
       !r.Read(sequence+0x20,level)||level!=owner.level.object||!cfgobserver::Same(r,level+disk::OwningWorld,world)||
       !r.Read(sequence+disk::SequencePlayer,player)||!player||!cfgspawn::ObjectSlot(r,registry,player,playback)||
       !r.Read(registry.data+24ULL*static_cast<unsigned>(playback.index)+8,flags)||(flags&0x30000000u)||
       !r.Read(module+disk::MovieSceneSequencePlayerClass,type)||!r.Read(module+disk::MovieSceneSequencePlayerConstructor,ctor)||!cfgprobe::MatchingClassCache(type,ctor)||
       !cfgspawn::ObjectIs(r,player,type)||!cfgobserver::Same(r,player+0x20,sequence)||
       !r.Read(player+disk::Status,status)||status!=0)return false;
    std::array<unsigned char,11> code{};
    if(!r.Read(module+disk::IsPlayingGetter,code)||code!=disk::IsPlayingGetterCode)return false;
    return cfgescape::Same(r,player+disk::Status,std::uint8_t{})&&cfgobserver::Same(r,player+0x20,sequence)&&
        cfgobserver::Same(r,sequence+disk::SequencePlayer,player)&&cfgobserver::Same(r,sequence+0x20,level)&&
        cfgobserver::Same(r,owner.settings.object+disk::LevelOutroSequence,sequence)&&
        cfgspawn::ObjectSlot(r,registry,sequence,fresh)&&fresh==actor&&cfgspawn::ObjectSlot(r,registry,player,fresh)&&fresh==playback;
}
}
