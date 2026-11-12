#pragma once
#include <cstdint>
namespace cfgholdrecovery {
enum class Phase {Observe,Cancel,Unlock,Confirm,Refused};
struct Policy {
    std::uintptr_t victim=0,car=0,lock=0,released=0;
    std::int32_t serial=0,carSerial=0,releasedSerial=0;
    std::uint64_t since=0,inputAt=0,next=0,attemptAt=0,quarantineUntil=0;
    Phase phase=Phase::Observe;unsigned cancels=0,confirmed=0,refusals=0;
    bool Observe(std::uintptr_t v,std::int32_t s,std::uintptr_t c,std::int32_t cs,std::uint64_t now){
        if(!v||s<=0||(c&&cs<=0))return false;
        if(v!=victim||s!=serial||c!=car||cs!=carSerial||now<since){
            victim=v;serial=s;car=c;carSerial=cs;since=now;inputAt=attemptAt=0;lock=0;phase=Phase::Observe;
        }
        return true;
    }
    void Input(std::uintptr_t v,std::uint64_t now){if(v==victim&&!inputAt)inputAt=now;}
    bool Stale(std::uint64_t now,bool seated,bool canKill,bool noSlots,bool nativeKilling,bool terminal)const{
        if(!victim||phase!=Phase::Observe||nativeKilling||terminal||now<since||
           now-since<(seated?5500u:2500u)||(inputAt&&(now<inputAt||now-inputAt<5000)))return false;
        return seated||!canKill||noSlots||inputAt!=0;
    }
    void Attempt(std::uintptr_t component,std::uint64_t now){lock=component;attemptAt=now;phase=Phase::Unlock;++cancels;next=now+100;}
    void Refuse(){phase=Phase::Refused;++refusals;}
    void Released(std::uint64_t now){released=victim;releasedSerial=serial;quarantineUntil=now+1000;
        ++confirmed;victim=car=lock=0;serial=carSerial=0;since=inputAt=attemptAt=0;phase=Phase::Observe;}
    bool Allowed(std::uintptr_t pawn,std::int32_t s,std::uint64_t now)const{
        return pawn!=released||s!=releasedSerial||now>=quarantineUntil;
    }
};
}
