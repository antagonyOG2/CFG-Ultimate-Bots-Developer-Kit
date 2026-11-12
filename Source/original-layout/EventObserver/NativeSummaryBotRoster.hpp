#pragma once
#include "NativeSummaryObservation.hpp"
#include "OwnedCounselorTemplate.hpp"
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
namespace cfgsummarybots {
struct Entry {cfgprobe::lifetime::SlotObservation state{};std::uintptr_t controller=0;};
struct Roster {
    std::array<Entry,32> entries{};
    std::uintptr_t display=0;cfgprobe::lifetime::SlotObservation displaySlot{},hud{},root{};
    unsigned overrides=0,refused=0;
    void Reset(){*this={};}
    bool Contains(std::uintptr_t state)const{for(const auto& e:entries)if(e.state.object==state)return true;return false;}
};
// Retain a real PlayerState, not its mortal pawn. Native participant membership
// is the strong reference after death/escape; serial0 is not manufactured.
template<class R,class H> bool Participant(R& r,H& h,std::uintptr_t gameState,
    std::uintptr_t state,const cfgprobe::lifetime::SlotObservation& expected,unsigned* source=nullptr){
    cfgsummary::List players{};cfgprobe::lifetime::SlotObservation slot{};
    if(!state||expected.object!=state||!h.Scope()||!r.Read(gameState+cfgsummary::disk::Players,players)||!players.Sane())return false;
    unsigned index=32,matches=0;std::uint32_t flags=0;std::uint8_t excluded=0;
    for(int i=0;i<players.count;++i){std::uintptr_t value=0;if(!r.Read(players.data+8ULL*i,value))return false;
        if(value==state){index=static_cast<unsigned>(i);++matches;}}
    if(matches!=1||!h.Slot(state,slot)||!cfgbundle::OwnedClassSlotMatches(expected,slot)||
       !h.StateType(state)||!r.Read(state+0x394,flags)||
       !((flags&0x34)==4||(CFG_J22_END_SUMMARY&&(flags&0x34)==0&&(flags&8)&&
            cfgobserver::Same(r,gameState+cfgprimary::CurrentKillerState,state)))||
       !r.Read(state+0x13C,excluded)||(excluded&4)!=0||
       !cfgsummary::SameList(r,gameState+cfgsummary::disk::Players,players)||
       !cfgobserver::Same(r,players.data+8ULL*index,state)||!h.Scope())return false;
    if(source)*source=index;return true;
}
template<class H> bool Owned(H& h,const cfgprobe::lifetime::SlotObservation& expected){
    cfgprobe::lifetime::SlotObservation fresh{};
    return expected.HasAssignedSerial()&&h.Slot(expected.object,fresh)&&
        cfgbundle::OwnedClassSlotMatches(expected,fresh);
}
template<class R,class H> bool EligibleDisplay(R& r,H& h,const Roster& s,std::uintptr_t display,
    std::uintptr_t controller,std::uintptr_t gameState,std::uintptr_t state,const cfgstatussurface::Binding* status=nullptr,std::uintptr_t module=0){
    std::uintptr_t hud=0,root=0;std::uint8_t defer=0;
    if(!s.Contains(state)||!s.display||display!=s.display||!h.Scope()||
       !Owned(h,s.displaySlot)||!Owned(h,s.hud)||!Owned(h,s.root)||
       !r.Read(controller+cfgsweater::huddisk::ControllerHUD,hud)||hud!=s.hud.object||
       !(status?(status->display.object==display&&status->root==s.root&&cfgstatussurface::Current(r,h,module,controller,*status)):
         r.Read(hud+cfgsummary::disk::Scoreboard,root)&&root==s.root.object&&cfgobserver::Same(r,root+cfgsummary::disk::Display,display))||
       !r.Read(display+0x3D8,defer)||defer!=0)return false;
    for(const auto& entry:s.entries)if(entry.state.object==state)
        return Participant(r,h,gameState,state,entry.state);
    return false;
}

template<class R,class H> bool Eligible(R& r,H& h,const Roster& s,std::uintptr_t query,
    std::uintptr_t controller,std::uintptr_t gameState,std::uintptr_t state,const cfgstatussurface::Binding* status=nullptr,std::uintptr_t module=0){
    std::uintptr_t display=0;return r.Read(query,display)&&EligibleDisplay(r,h,s,display,controller,gameState,state,status,module);
}

// Only the presentation getter on a real, currently owned registered row.
// This never changes cached travel-ready/role/outcome bytes in PlayerState.
template<class R,class H> bool RowReady(R& r,H& h,const Roster& s,std::uintptr_t row,
    std::uintptr_t module,std::uintptr_t controller,std::uintptr_t gameState,const cfgstatussurface::Binding* status=nullptr){
    std::uintptr_t state=0,display=0,rowClass=0;cfgprobe::lifetime::SlotObservation identity{},again{};
    cfgsummary::List rows{},registered{};
    if(!h.Scope()||!h.Slot(row,identity)||!identity.HasAssignedSerial()||
       !r.Read(row+0x20,display)||!r.Read(row+cfgsummary::disk::RowPlayerState,state)||
       !EligibleDisplay(r,h,s,display,controller,gameState,state,status,module)||
       !r.Read(display+cfgsummary::disk::RowClass,rowClass)||!cfgobserver::Same(r,row+cfgprobe::ObjectClass,rowClass)||
       !cfgsummary::Named(r,module,row,u"ScoreboardPlayerListItem_C")||
       !r.Read(display+cfgsummary::disk::Rows,rows)||!rows.Sane()||
       !r.Read(display+cfgsummary::disk::Registered,registered)||!registered.Sane())return false;
    unsigned rowMatches=0,stateMatches=0;
    for(int i=0;i<rows.count;++i){std::uintptr_t value=0;if(!r.Read(rows.data+8ULL*i,value))return false;if(value==row)++rowMatches;}
    for(int i=0;i<registered.count;++i){std::uintptr_t value=0;if(!r.Read(registered.data+8ULL*i,value))return false;if(value==state)++stateMatches;}
    return rowMatches==1&&stateMatches==1&&h.Slot(row,again)&&identity==again&&
        cfgobserver::Same(r,row+0x20,display)&&cfgobserver::Same(r,row+cfgsummary::disk::RowPlayerState,state)&&
        cfgsummary::SameList(r,display+cfgsummary::disk::Rows,rows)&&
        cfgsummary::SameList(r,display+cfgsummary::disk::Registered,registered)&&h.Scope();
}
template<class R,class H> bool Admit(R& r,H& h,Roster& s,std::uintptr_t module,std::uintptr_t query,
    std::uintptr_t controller,std::uintptr_t gameState,std::uintptr_t state){
    if(Eligible(r,h,s,query,controller,gameState,state))return true;
    // Native HUD reconstruction at match end may replace the display after
    // gameplay scheduling has stopped. Re-prove its CURRENT owned chain here;
    // never register rows or dispatch a native action from this predicate.
    if(!s.Contains(state))return false;
    for(const auto& e:s.entries)if(e.state.object==state){unsigned source=32;cfgsummary::Proof proof{};
        if(!Participant(r,h,gameState,state,e.state,&source)||
           !cfgsummary::Observe(r,h,module,controller,gameState,state,source,proof)||
           !cfgobserver::Same(r,query,proof.display.object))return false;
        s.display=proof.display.object;s.displaySlot=proof.display;s.hud=proof.hud;s.root=proof.root;
        return Eligible(r,h,s,query,controller,gameState,state);
    }
    return false;
}
}
