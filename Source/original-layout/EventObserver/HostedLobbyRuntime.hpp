#if CFG_H21_LOBBY_DISCOVERY
cfghostedlobby::Report hostedLobbyReport;
struct HostedLobbyPublishScope {
    HostedLobbyPublishScope(){InterlockedIncrement(reinterpret_cast<volatile LONG*>(&hostedLobbyReport.generation));}
    ~HostedLobbyPublishScope(){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&hostedLobbyReport.generation));}
};
cfghostedlobby::Discovery hostedLobbyDiscovery;
cfghostedlobby::StallGuard hostedLobbyStall; // H2.47 CFG_H247_LOBBY_REARM
std::uint64_t hostedLobbyNext=0;
#if CFG_H21_LOBBY_ROWS
cfghostedlobby::RowHooks hostedLobbyRowHooks; // empty until the dedicated host-lobby provider is proved
cfghostedlobby::Placement hostedLobbyPlacement;
#include "HostedLobbyRowsRuntime.hpp"
#endif
void ServiceHostedLobby()noexcept{
    const auto now=GetTickCount64();if(now<hostedLobbyNext||GetCurrentThreadId()!=observerOwnerThread)return;
    hostedLobbyNext=now+100;Reader r;r.cap=8192;
    auto& lobbyReport=hostedLobbyReport;InterlockedIncrement(reinterpret_cast<volatile LONG*>(&lobbyReport.generation));
    ++lobbyReport.samples;cfghostedlobby::Context current;cfghostedlobby::Failure why;
    if(cfghostedlobby::Observe(r,plan.module,observerOwnerThread,current,why,&lobbyReport.controller)){
        const bool matchLive=HostedRoutineActive()||hostedAdoptionActive;
        const bool same=!hostedLobbyStall.TakeRearm(matchLive)&&(hostedLobbyDiscovery.graph.count||(!current.listenHost&&lobbyReport.histories))&&hostedLobbyDiscovery.context.world==current.world&&hostedLobbyDiscovery.context.menu==current.menu&&hostedLobbyDiscovery.context.listenHost==current.listenHost&&hostedLobbyDiscovery.context.identities[4]==current.identities[4];
        if(!same){
            unsigned at=lobbyReport.histories<cfghostedlobby::HistoryLimit?lobbyReport.histories++:(lobbyReport.current+1)%cfghostedlobby::HistoryLimit;
            lobbyReport.current=at;if(!hostedLobbyDiscovery.Begin(r,current,lobbyReport.history[at]))why=cfghostedlobby::Failure::Tree;
        }
        if(why==cfghostedlobby::Failure::None){
            auto& snapshot=lobbyReport.history[lobbyReport.current];
            // Each node needs its own bounded current/path proof. Spending the
            // observation reader on several deep nodes stranded partial trees.
            for(unsigned i=0;i<4&&!snapshot.complete;++i){
                Reader step;step.cap=32768;
                if(!hostedLobbyDiscovery.Step(step,snapshot,lobbyReport)){why=cfghostedlobby::Failure::Changed;hostedLobbyStall.StepFailed(snapshot.complete!=0,snapshot.team.node,matchLive);break;}
                hostedLobbyStall.Progressed();
                lobbyReport.peakReads=(std::max)(lobbyReport.peakReads,step.reads);
            }
            if(CFG_H261_CHAT_MEASURE&&snapshot.complete&&!hostedLobbyDiscovery.measured&&hostedLobbyDiscovery.context.listenHost){
                hostedLobbyDiscovery.measured=true;Reader measure;measure.cap=65536;hostedLobbyDiscovery.Measure(measure); // one-shot, read-only
            }
#if CFG_H21_LOBBY_ROWS
            // Native rows run only in their independently proved returned callback.
#endif
        }
    }
    lobbyReport.failure=static_cast<unsigned>(why);lobbyReport.peakReads=(std::max)(lobbyReport.peakReads,r.reads);
    MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&lobbyReport.generation));
}
#endif
