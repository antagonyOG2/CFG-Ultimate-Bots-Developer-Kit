#pragma once
#include "StartupEntryBindings.generated.hpp"
#include "OfflineLifecycleBindings.generated.hpp"
#include "../EventObserver/NativeStartupRetention.hpp"
#include "ObjectLifetime.hpp"
namespace cfgmenu::startupentry {
// Admission of the SOURCE of a single synchronous native callback only.
// The host separately proves fresh world/owner links and retains dependencies.
// Recheck state at entry, not after a native callee legitimately advances it.
template<class Reader>bool Origin(Reader& r,std::uintptr_t module,std::uintptr_t mode,
    startupretain::Role role,std::uintptr_t pc,std::uint32_t thread,std::uint32_t ownerThread,
    bool hooksEnabled,unsigned& stage){
    namespace p=startupentrydisk::profile;namespace d=offlinelifecycledisk;
    stage=1;
    if(!hooksEnabled||!thread||thread!=ownerThread||!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(mode)||
       (role!=startupretain::Role::Intro&&role!=startupretain::Role::Match)||
       pc!=module+(role==startupretain::Role::Intro?p::IntroReturnPc:p::MatchReturnPc))return false;
    const auto code=[&](std::uintptr_t at,const auto& expected){auto actual=expected;
        return r.ReadBytes(module+at,actual.data(),actual.size())&&actual==expected;};
    stage=2;if(!code(p::IntroDispatcher,startupentrydisk::IntroDispatcherCode)||
        !code(p::MatchBridge,startupentrydisk::MatchBridgeCode)||!code(p::MatchDispatcher,startupentrydisk::MatchDispatcherCode))return false;
    stage=3;std::uintptr_t vt=0,dispatch=0,intro=0,match=0;
    if(!r.Read(mode,vt)||vt!=module+d::OfflineBotsVtable||
       !r.Read(vt+0x808,dispatch)||dispatch!=module+p::IntroDispatcher||
       !r.Read(vt+d::IntroSlot,intro)||intro!=module+d::OfflineBotsIntro958||
       !r.Read(vt+d::MatchStartedSlot,match)||match!=module+d::OfflineBotsMatchStarted818)return false;
    stage=4;std::uint64_t state=0,expected=0,again=0;
    if(!r.Read(mode+p::ModeState,state)||!r.Read(module+(role==startupretain::Role::Intro?p::IntroState:p::MatchState),expected)||
       !expected||state!=expected||!r.Read(mode+p::ModeState,again)||again!=state)return false;
    stage=0;return true;
}
}
