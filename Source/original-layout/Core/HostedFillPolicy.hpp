#pragma once
#include <cstdint>
namespace cfghostedfill {
struct Report {unsigned entries=0,applied=0,refused=0,restored=0,restoreRefused=0;};
template<class Host,class Forward>void Around(Host& host,Forward&& forward,Report& report){
    ++report.entries;typename Host::Ticket ticket{};bool owned=false;
    if(host.Capture(ticket)&&ticket.requested){
        if(ticket.before==1)++report.applied;
        else if(ticket.before==0&&host.Exchange(ticket,0,1)){owned=true;++report.applied;}
        else ++report.refused;
    }else ++report.refused;
    // The engine owns all counselor spawning/replication/roster accounting.
    // This policy never calls a second birth or changes NumBots itself.
    forward();
    if(owned){if(host.Exchange(ticket,1,0))++report.restored;else ++report.restoreRefused;}
}
// Observation only: do not change the stock flag or synthesize replacement births.
template<class Host,class Forward>void AroundStock(Host&,Forward&& forward,Report& report){++report.entries;forward();}
#ifndef CFG_H257_F1_SLOT
#define CFG_H257_F1_SLOT 1 // H2.57 (user-approved): one fewer counselor bot so the F1 Jason always has a slot (live H2.56: roster 8, F1 reason 3)
#endif
// Armed reserve: before adoption the AI Jason and F1 Jason (2), after adoption the F1 Jason (1). Same 6 as the stock startup patch.
inline int PlayerLimit(bool armed,bool adopted){return 8-(armed?(CFG_H257_F1_SLOT?(adopted?1:2):(adopted?0:1)):0);}
inline bool CounselorRoom(int players,bool armed,bool adopted){return players>=1&&players<PlayerLimit(armed,adopted);}
inline int CounselorTarget(int humans,bool armed,bool adopted){const int room=PlayerLimit(armed,adopted)-humans;return room<0?0:room>6?6:room;}
// Suppress stock startup's fixed min(6,8-humans) branch while reserving a Jason
// controller slot. A separate stable-frame helper fills to the capped roster.
// The engine startup still forwards ONCE, and only our unchanged flag value restores.
template<class Host,class Forward>void AroundReserved(Host& host,Forward&& forward,Report& report){
 ++report.entries;typename Host::Ticket ticket{};bool owned=false;
 if(host.Capture(ticket)&&ticket.reserve){
   if(ticket.before==0)++report.applied;
   else if(ticket.before==1&&host.Exchange(ticket,1,0)){owned=true;++report.applied;}
   else ++report.refused;
 }else ++report.refused;
 forward();
 if(owned){if(host.Exchange(ticket,0,1))++report.restored;else ++report.restoreRefused;}
}

}
