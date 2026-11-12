#pragma once
#include "../RuntimeProbe/SpectatorRuntime.hpp"
#include <array>

// Separate from offline runtime: no hooks, donor offsets or camera dispatch.
// CFG adapter must freshly prove reflected ownership, serials and authority.
namespace cfghosted {
using Identity=cfgspectator::Identity;
using Target=cfgspectator::Target;
struct Context {
    Identity world,netDriver,mode;
    bool requested=false,gameThread=false,authority=false,inProgress=false;
    bool bindingsVerified=false;
};
inline bool Valid(const Context& c){
    return c.requested&&c.gameThread&&c.authority&&c.inProgress&&
        c.bindingsVerified&&c.world&&c.netDriver&&c.mode;
}
inline bool Same(const Context& a,const Context& b){
    return a.world==b.world&&a.netDriver==b.netDriver&&a.mode==b.mode;
}
struct Viewer {
    Identity controller,state;
    bool belongsToWorld=false,remote=false,terminal=false,spectator=false;
};
inline bool Eligible(const Viewer& v){
    return v.controller&&v.state&&v.belongsToWorld&&v.remote&&v.terminal&&v.spectator;
}
class Session {
    Context owner_{};bool armed_=false;std::uint64_t generation_=0;
public:
    bool Arm(const Context& c){
        if(!Valid(c)||(armed_&&!Same(owner_,c)))return false;
        if(!armed_)++generation_;
        owner_=c;armed_=true;return true;
    }
    bool Current(const Context& c)const{return armed_&&Valid(c)&&Same(owner_,c);}
    void Reset(){owner_={};armed_=false;}
    std::uint64_t Generation()const{return generation_;}
    // Adapter rechecks native ownership immediately before dispatch; no solo
    // lifecycle helpers. TickRegisteredAI is restricted to registered AI actors.
    template<class Adapter> bool Tick(Adapter& adapter){
        Context c;
        if(!adapter.ObserveHostedContext(c)||!Current(c)){Reset();return false;}
        return adapter.TickRegisteredAI(c);
    }
};
// Donor HostedViewablePlayerBridge alternates a live stock counselor and AI
// Jason per remote viewer. Caller invokes stock first. Unknown stock unchanged.
class RemoteCycle {
    struct Slot {Identity controller,state;bool jasonNext=false;};
    std::array<Slot,8> slots_{};Context owner_{};bool active_=false;std::uint64_t generation_=0;
    void Forget(Identity controller){for(auto& s:slots_)if(s.controller==controller)s={};}
public:
    void Reset(){slots_={};owner_={};active_=false;}
    void Disconnected(Identity controller){Forget(controller);}
    Identity Select(const Session& session,const Context& context,const Viewer& viewer,
                    const Target& stock,const Target& jason,bool registeredJason){
        if(!session.Current(context)){Reset();return stock.state;}
        if(!active_||!Same(owner_,context)||generation_!=session.Generation()){
            Reset();owner_=context;active_=true;generation_=session.Generation();}
        if(!Eligible(viewer)||!registeredJason||!cfgspectator::Usable(jason)||
           !cfgspectator::Usable(stock)||stock.state==jason.state||stock.pawn==jason.pawn){
            Forget(viewer.controller);return stock.state;
        }
        Slot* found=nullptr;Slot* empty=nullptr;
        for(auto& slot:slots_){
            if(slot.controller.object==viewer.controller.object&&
               (slot.controller!=viewer.controller||slot.state!=viewer.state))slot={};
            if(slot.controller==viewer.controller)found=&slot;
            if(!slot.controller&&!empty)empty=&slot;
        }
        if(!found){found=empty;if(!found)return stock.state;
            found->controller=viewer.controller;found->state=viewer.state;}
        const bool insert=found->jasonNext;found->jasonNext=!insert;
        return insert?jason.state:stock.state;
    }
};
// Original CanSpectate runs first. Exclude only a proven same-world terminal
// counselor, never an unknown entry inferred dead merely from missing pawn.
inline bool CanSpectate(bool stockAllowed,const Session& session,const Context& c,
                        const Viewer& viewer,Identity requested,const Target& jason,
                        bool registeredJason,bool knownTerminalCounselor){
    if(!session.Current(c)||!Eligible(viewer))return stockAllowed;
    if(registeredJason&&cfgspectator::Usable(jason)&&requested==jason.state)return true;
    if(knownTerminalCounselor)return false;
    return stockAllowed;
}
}
