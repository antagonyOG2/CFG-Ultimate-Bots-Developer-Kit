#pragma once
#include "../Core/MenuReentryPolicy.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include "../RuntimeProbe/ChallengeMenuCalls.generated.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "SpawnResultObservation.hpp"
#include "../RuntimeProbe/FrontendHudObservation.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
namespace cfgmenu::challengetransition {
// Cleanup of an already-entered, normally returned Start, in the SAME lexical
// completed-HUD activation. This is not permission for any subsequent input.
template<class R>bool ReturnedStartCurrent(R& r,const hud::Binding& b,std::uintptr_t receiver,
    std::uintptr_t function,std::uintptr_t pc,std::uint32_t thread){
    const auto& c=b.context;std::uint32_t owner=0;std::uint8_t flag=0;std::uintptr_t value=0;int count=0,capacity=0;
    cfgprobe::lifetime::ArrayHeader registry{};
    if(thread!=c.thread||receiver!=c.hud||function!=b.event||pc!=c.module+transportdisk::ReturnPc||
       !cfgprimary::LookupVetoClearForObservation(r,c.module)||
       !r.Read(c.module+cfgprobe::dispatch::ThreadIdRva,owner)||owner!=thread||
       !r.Read(c.module+cfgprobe::dispatch::ThreadFlagRva,flag)||flag!=1||
       !r.Read(c.module+cfgprobe::EngineGlobalCandidate,value)||value!=c.engine||
       !r.Read(c.engine+cfgprobe::EngineGameViewport,value)||value!=c.viewport||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers,value)||value!=c.players||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+8,count)||count!=1||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||
       !r.Read(c.players,value)||value!=c.player||
       !cfgprobe::lifetime::ReadArrayHeader(r,c.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
    for(const auto& old:c.identities){cfgprobe::lifetime::SlotObservation slot{};
        if(!old.HasAssignedSerial()||!cfgspawn::ObjectSlot(r,registry,old.object,slot)||!(slot==old))return false;}
    return hud::Path(r,b);
}
template<class R>unsigned Classify(R& r,const active::Context& c,std::uintptr_t actor,std::uintptr_t function){
    if(actor!=c.widget)return 0;
    cfgnames::Observation<R> names(r,c.module);cfgnames::Text label;
    if(!names.Object(c.type,label)||(label.View()!=u"SP_LobbyWidget_C"&&label.View()!=u"SPChallengesSettingsMenuWidget_C"))return 0;
    for(unsigned kind=1;kind<=2;++kind){const auto& b=kind==1?challengecalls::PushMenu:challengecalls::PopMenu;
        std::uintptr_t fn=0,owner=0,ctor=0,outer=0,executor=0;
        if(!r.Read(c.module+b.cache,fn)||fn!=function)continue;
        if(!r.Read(c.module+b.owner,owner)||!r.Read(c.module+b.ctor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor)||
           !r.Read(function+0x20,outer)||outer!=owner||!r.Read(function+0xB0,executor)||executor!=c.module+b.executor)return 0;
        if(kind==1){std::array<unsigned char,challengecalls::PushMenuCode.size()> code{};
            if(!r.ReadBytes(executor,code.data(),code.size())||code!=challengecalls::PushMenuCode)return 0;}
        else{std::array<unsigned char,challengecalls::PopMenuCode.size()> code{};
            if(!r.ReadBytes(executor,code.data(),code.size())||code!=challengecalls::PopMenuCode)return 0;}
        return kind;
    }return 0;
}
template<class R>bool Observe(R& r,const active::Context& source,unsigned kind,std::uintptr_t expectedType,
    std::uintptr_t returnedWidget,active::Context& fresh){
    cfgprobe::lifetime::SlotObservation retained{};std::uintptr_t type=0;
    if((kind!=1&&kind!=2)||!active::Observe(r,source.module,fresh)||!reentry::Infrastructure(source,fresh)||
       !cfgspawn::ObjectSlot(r,fresh.registry,source.widget,retained)||!(retained==source.identities[2])||
       !r.Read(source.widget+cfgprobe::ObjectClass,type)||type!=source.type)return false;
    if(kind==1)return fresh.array.count==source.array.count+1&&fresh.widget==returnedWidget&&fresh.type==expectedType;
    if(!CFG_J25_CHALLENGE_PUSH_OUTPUTS)return source.array.count>1&&fresh.array.count==source.array.count-1;
    if(source.array.count<=1||fresh.array.count!=source.array.count-1||fresh.type==source.type)return false;
    cfgnames::Observation<R> names(r,source.module);cfgnames::Text label;
    return names.Object(fresh.type,label)&&label.View()==u"SP_LobbyWidget_C";
}
}
