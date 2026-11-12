#pragma once
#include "NativeOfflineOwnerObservation.hpp"
namespace cfgmenu::offlinerestart {
// Names are content identifiers only. Every use also proves a fresh standalone
// native owner; a saved name never grants authority over a departed actor.
template<class R> bool Package(R& r,const promotion::Context& c,cfgnames::Text& out){
 out={};std::uintptr_t outer=0,again=0;cfgnames::Observation<R> names(r,c.module);
 cfgnames::Text first,second;
 if(!offlineowner::Current(r,c,c.thread)||!r.Read(c.world+0x20,outer)||
    !names.Object(outer,first)||first.View().find(u"/Game/Maps/")!=0||
    !r.Read(c.world+0x20,again)||again!=outer||!names.Object(outer,second)||
    first.View()!=second.View()||!offlineowner::Current(r,c,c.thread))return false;
 out=first;return true;
}
template<class R> bool Observe(R& r,const freshgame::Source& source,const promotion::Context& prior,
 const cfgnames::Text& package,std::uintptr_t receiver,std::uint32_t thread,promotion::Context& out){
 out={};if(!source.nativeOffline||package.View().empty()||!prior.identities[0].HasAssignedSerial()||
  !prior.identities[1].HasAssignedSerial()||thread!=source.thread)return false;
 promotion::Context fresh;cfgnames::Text actual;
 if(!offlineowner::Observe(r,source,receiver,thread,true,fresh)||
    fresh.identities[0]==prior.identities[0]||fresh.identities[1]==prior.identities[1]||
    !Package(r,fresh,actual)||actual.View()!=package.View()||
    !offlineowner::Current(r,fresh,thread))return false;
 out=fresh;return true;
}
}
