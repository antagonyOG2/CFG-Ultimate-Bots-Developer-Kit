#pragma once
#include "HostedTakeoverObservation.hpp"
#include "HostedH1Report.hpp"

// H1-OBSERVE: read-only owner-thread sampler. One call performs the staged listen-host proof and
// the killer/roster/ownership census (Codex's HostedTakeoverObservation) and fills a Report. It makes
// NO native call and NO write. Passing is sampled identity, not GC retention or call approval.
namespace cfgh1 {
#include "HostedContextDiagnostics.inl"
// Stage logic only; the caller supplies the admission (engine chain read by the DLL on the owner thread).
template<class R> Stage Sample(R& source,const cfghosted::native::Admission& a,std::uint32_t thread,Report& rep,
    cfghosted::takeover35::Frame& frame,bool& frameValid){
    using namespace cfghosted;
    frameValid=false;frame={};
    rep.probeBits=0;rep.world=a.world;rep.netDriver=0;rep.matchInProgress=0;rep.context={};
    std::uintptr_t driver=0;
    if(!source.Read(a.world+cfgprobe::WorldNetDriver,driver))return Stage::WorldUnreadable;
    rep.probeBits|=ProbeWorld;
    if(!driver)return Stage::NoNetDriver;
    rep.probeBits|=ProbeNetDriver;rep.netDriver=driver;
    std::uintptr_t driverWorld=0,connection=1,mode=0,state=0;
    if(source.Read(driver+disk35::DriverWorld,driverWorld)&&driverWorld==a.world)rep.probeBits|=ProbeDriverWorld;
    if(source.Read(driver+disk35::ServerConnection,connection)&&!connection)rep.probeBits|=ProbeListenHost;
    if(!(rep.probeBits&ProbeDriverWorld)||!(rep.probeBits&ProbeListenHost))return Stage::ClientOrForeign;
    if(source.Read(a.world+cfgprobe::WorldGameMode,mode)&&cfgprobe::ObjectPointer(mode)){
        rep.probeBits|=ProbeModePresent;rep.mode=mode;
        std::uint8_t role=0;
        if(source.Read(mode+disk35::ActorRole,role)&&role==disk::AuthorityRole)rep.probeBits|=ProbeModeAuthority;
        std::uintptr_t first=0,second=0;cfgprobe::Ancestry ancestry;
        if(source.Read(a.module+disk35::HuntReflection,first)&&source.Read(a.module+disk35::HuntConstructor,second)&&
           cfgprobe::MatchingClassCache(first,second)&&cfgprobe::CaptureAncestry(source,mode,ancestry)&&ancestry.Contains(first))
            rep.probeBits|=ProbeModeHunt;
    }
    if(source.Read(a.world+cfgprobe::WorldGameState,state)&&cfgprobe::ObjectPointer(state))rep.probeBits|=ProbeStatePresent;
    unsigned reads=0;takeover35::Frame captured;
    if(takeover35::Capture(source,a,thread,captured,&reads)){
        rep.lastFrameReads=reads;frame=captured;frameValid=true;
        rep.probeBits|=ProbeLocalController|ProbeLocalAuthority;
        rep.matchInProgress=1;
        rep.roster=static_cast<std::int32_t>(captured.count);rep.numBots=captured.bots;
        rep.botFill=captured.botFill;rep.counselorTree=captured.counselorTree?1u:0u;
        rep.explicitPick=captured.explicitPick;rep.explicitPickPresent=captured.explicitPick?1u:0u;
        rep.explicitPickIsKiller=captured.explicitPick&&captured.explicitPick==captured.humanState.object?1u:0u;
        rep.eligibility=static_cast<std::uint32_t>(captured.eligibility);
        rep.mode=captured.host.session.Mode();rep.localController=captured.host.local[2].object;
        rep.killerPawn=captured.killer.object;rep.killerController=captured.human.object;rep.killerState=captured.humanState.object;
        rep.localPawn=captured.localPawn.object;
        switch(captured.eligibility){
        case takeover35::Eligibility::NoKiller:rep.killerKind=static_cast<std::uint32_t>(Kill::None);break;
        case takeover35::Eligibility::HostSelected:rep.killerKind=static_cast<std::uint32_t>(Kill::LocalHost);break;
        case takeover35::Eligibility::Ready:rep.killerKind=static_cast<std::uint32_t>(Kill::RemoteHuman);break;
        default:rep.killerKind=static_cast<std::uint32_t>(Kill::Unproven);break;}
        return Stage::FramePassed;
    }
    rep.lastFrameReads=reads;
    // Which half refused? The listen-host context alone is cheaper and answers it.
    listen35::Context context;unsigned contextReads=0;
    native::Trace trace;const bool context_ok=listen35::Capture(source,a,thread,context,&contextReads,&trace);
    if(!context_ok)Describe(source,a,trace,rep.context);
    rep.lastContextReads=contextReads;
    if(!context_ok)return Stage::ContextRefused;
    rep.probeBits|=ProbeLocalController|ProbeLocalAuthority;rep.matchInProgress=1;rep.mode=context.session.Mode();
    rep.localController=context.local[2].object;
    return Stage::FrameRefused;
}
// Stability gate for the (armed) takeover: the SAME remote-selected Jason must be observed READY across
// consecutive frames for a minimum wall interval before any native effect is even considered.
struct Stability {
    std::uint64_t firstAt=0,lastAt=0;unsigned frames=0;
    std::uintptr_t world=0,mode=0,pawn=0,controller=0,state=0,local=0,localPawn=0;std::int32_t roster=-1,bots=-1;
    void Reset() noexcept { *this=Stability{}; }
    // Returns true when the identical READY observation has persisted >= minimumMs over >= minimumFrames.
    bool Observe(const Report& r,std::uint64_t now,std::uint64_t minimumMs,unsigned minimumFrames) noexcept {
        const bool ready=r.stage==static_cast<std::uint32_t>(Stage::FramePassed)&&
            (r.eligibility==static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::Ready)||
             r.eligibility==static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::HostSelected)||
             r.eligibility==static_cast<std::uint32_t>(cfghosted::takeover35::Eligibility::HostPreborn));
        if(!ready){Reset();return false;}
        const bool same=frames&&world==r.world&&mode==r.mode&&pawn==r.killerPawn&&controller==r.killerController&&state==r.killerState&&
            local==r.localController&&localPawn==r.localPawn&&roster==r.roster&&bots==r.numBots;
        if(!same){Reset();firstAt=now;world=r.world;mode=r.mode;pawn=r.killerPawn;controller=r.killerController;state=r.killerState;
            local=r.localController;localPawn=r.localPawn;roster=r.roster;bots=r.numBots;frames=0;}
        lastAt=now;++frames;
        return frames>=minimumFrames&&now>=firstAt&&now-firstAt>=minimumMs;
    }
};
}
