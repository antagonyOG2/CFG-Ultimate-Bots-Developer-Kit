#pragma once
#include <cstdint>
namespace cfgobjective {
// Only read-only acquisition can retry. Any publication attempt, successful or
// ambiguous, consumes the slot. Native car caches may warm after gameplay birth.
struct CacheAdmission {
    unsigned index=0,attempts=0;std::uint64_t since=0,next=0;bool begun=false;
    bool Due(std::uint64_t now)const{return index<2&&now>=next;}
    void Begin(std::uint64_t now){if(!begun){begun=true;since=now;}}
    void Advance(){++index;attempts=0;since=next=0;begun=false;}
    bool Acquired(std::uint64_t now,bool publishable){
        Begin(now);++attempts;
        if(publishable||attempts>=21||now<since||now-since>=20000){Advance();return true;}
        next=now>UINT64_MAX-1000?UINT64_MAX:now+1000;return false;
    }
    void BudgetDeferred(std::uint64_t now){Begin(now);if(now<since||now-since>=20000){Advance();return;}
        next=now>UINT64_MAX-1000?UINT64_MAX:now+1000;}
};
// Separate fresh callback for revalidation/CAS/readback. Read-only deferrals
// can retry within this finite window; an actual CAS is never replayed.
struct PublicationWindow {
    bool pending=false,attempted=false;unsigned index=0;
    std::uint64_t since=0,next=0;
    void Begin(unsigned slot,std::uint64_t now){pending=true;attempted=false;index=slot;since=now;next=now>UINT64_MAX-25?UINT64_MAX:now+25;}
    bool Due(std::uint64_t now)const{return pending&&now>=next;}
    bool Expired(std::uint64_t now)const{return now<since||now-since>=20000;}
    bool MarkAttempt(){if(!pending||attempted)return false;attempted=true;return true;}
    bool Defer(std::uint64_t now){if(attempted||Expired(now)){pending=false;return false;}next=now>UINT64_MAX-1000?UINT64_MAX:now+1000;return true;}
    void Finish(){pending=false;}
};
}
