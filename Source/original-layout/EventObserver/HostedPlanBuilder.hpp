#pragma once
#include "HostedTakeoverObservation.hpp"
#include "PrimaryContextObservation.hpp"
// H1: the ONE place a hosted plan + its HostedBinding are built, from a READY listen-host frame. The kind (Hosted)
// is decided here, once. The binding is accepted only if the exact RetainedHostedWorld proof the driver will repeat
// passes on the freshly built plan. Pure observation; no native call, no write, no retention.
namespace cfgprimary {
// Independent local-pawn effect plan. Remote-Jason takeover eligibility is
// deliberately NOT fabricated for an ordinary host F3 request.
template<class R>bool BuildHostedLocalPlan(R& r,const cfghosted::listen35::Context& host,
    cfgobserver::Plan& out,HostedBinding& binding){
    using namespace cfgprobe;out={};binding={};const auto& s=host.session;const auto& a=s.admission;
    std::uintptr_t pawn=0,type=0;cfghosted::native::Slot identity;
    if(!r.Read(host.local[2].object+ControllerPawn,pawn))return false;
    bool typed=false;for(const auto kind:{Counselor,Killer})if(cfghosted::native::Class(r,a.module,NativeClasses[kind].reflectionCacheRva,NativeClasses[kind].constructorCacheRva,type)&&
        cfghosted::native::Typed(r,s.table,pawn,type,identity)){typed=true;break;}
    if(!typed||
       !cfghosted::native::Same(r,pawn+cfgescape::disk::Controller,host.local[2].object)||!cfghosted::native::WorldOwned(r,pawn,a.world))return false;
    cfgobserver::Plan p;p.bytes=sizeof(p);p.threadId=a.ownerThread;p.module=a.module;p.engine=a.engine;p.viewport=a.viewport;
    p.world=a.world;p.mode=s.Mode();p.controller=host.local[2].object;p.pawn=pawn;
    p.player=host.local[1].object;p.gameInstance=host.local[0].object;p.localPlayerData=host.players.data;
    p.objects={{s.identities[0],s.identities[2],host.local[2],identity}};
    HostedBinding b;b.active=true;b.plan=p;b.netDriver=s.Driver();b.driver=s.identities[1];b.huntClass=s.classes[2];
    if(!lifetime::ReadArrayHeader(r,p.module+lifetime::ObjectArrayRva,p.objectArray)||!dispatch::Observe(r,p.module,p.pawn,p.dispatch)||
       !cfgobserver::SanePlan(p)||!r.Read(p.mode+ObjectClass,b.modeClass))return false;
    b.plan=p;cfgobserver::Plan fresh;if(!RetainedHostedWorld(r,p,b,fresh))return false;
    out=p;binding=b;return true;
}
template<class R> bool BuildHostedPlan(R& r,const cfghosted::takeover35::Frame& f,cfgobserver::Plan& out,HostedBinding& binding){
    using namespace cfgprobe;
    out={};binding={};
    if((f.eligibility!=cfghosted::takeover35::Eligibility::Ready&&f.eligibility!=cfghosted::takeover35::Eligibility::HostSelected)||!f.localPawn.HasAssignedSerial()||!f.host.local[2].HasAssignedSerial())return false;
    const bool local=f.human.object==f.host.local[2].object;
    if((f.eligibility==cfghosted::takeover35::Eligibility::HostSelected)!=local||
       (local&&f.localPawn.object!=f.killer.object)||(!local&&f.localPawn.object==f.killer.object))return false;
    const auto& session=f.host.session;const auto& a=session.admission;
    cfgobserver::Plan p;p.bytes=sizeof(p);p.threadId=a.ownerThread;p.module=a.module;p.engine=a.engine;p.viewport=a.viewport;
    p.world=a.world;p.mode=session.Mode();p.controller=f.host.local[2].object;p.pawn=f.localPawn.object;
    p.player=f.host.local[1].object;p.gameInstance=f.host.local[0].object;p.localPlayerData=f.host.players.data;
    p.objects={{session.identities[0],session.identities[2],f.host.local[2],f.localPawn}};
    std::uintptr_t modeClass=0;
    if(!lifetime::ReadArrayHeader(r,p.module+lifetime::ObjectArrayRva,p.objectArray)||
       !dispatch::Observe(r,p.module,p.pawn,p.dispatch)||!cfgobserver::SanePlan(p)||
       !r.Read(p.mode+ObjectClass,modeClass)||!ObjectPointer(modeClass))return false;
    HostedBinding candidate;candidate.active=true;candidate.plan=p;candidate.netDriver=session.Driver();
    candidate.driver=session.identities[1];candidate.modeClass=modeClass;candidate.huntClass=session.classes[2];
    cfgobserver::Plan fresh{};
    if(!RetainedHostedWorld(r,p,candidate,fresh))return false;
    out=p;binding=candidate;return true;
}
}
