#pragma once
#include "CounselorTerminalObservation.hpp"
#include "../RuntimeProbe/SummaryBindings.generated.hpp"
#include "../RuntimeProbe/SweaterHudBindings.generated.hpp"
#include "NativeStatusSurface.hpp"
namespace cfgsummary {
struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
    bool Sane()const{return count>=0&&count<=32&&capacity>=count&&capacity<=64&&(!count||cfgprobe::ObjectPointer(data));}
};
struct Proof {
    cfgprobe::lifetime::SlotObservation hud{},root{},display{},state{};
    std::uintptr_t rowClass=0,table=0,stateOwner=0;unsigned sourceIndex=0;
    List participants{},registered{},rows{};bool present=false,rowPresent=false;
};
struct Runtime {std::uint64_t next=0;unsigned cursor=0,attempts=0,confirmed=0,refused=0;
    unsigned endingTraces=0;
    std::uintptr_t widget=0;std::int32_t widgetSerial=0;
    std::array<std::uintptr_t,16> attempted{};std::array<bool,16> confirmedBefore{};unsigned count=0;
    unsigned Index(std::uintptr_t state)const{for(unsigned i=0;i<count;++i)if(attempted[i]==state)return i;return count;}
    bool MayRegister(std::uintptr_t state,bool registered,bool row)const{
        if(registered||row)return false;const auto i=Index(state);
        return i<count?confirmedBefore[i]:count<attempted.size();
    }
    void Attempt(std::uintptr_t state){auto i=Index(state);if(i==count&&count<attempted.size())attempted[count++]=state;
        if(i<count)confirmedBefore[i]=false;}
    void Confirm(std::uintptr_t state){const auto i=Index(state);if(i<count)confirmedBefore[i]=true;}
};
template<class R> bool SameList(R& r,std::uintptr_t at,const List& list){List fresh;return r.Read(at,fresh)&&fresh==list;}
template<class R> bool Named(R& r,std::uintptr_t module,std::uintptr_t object,std::u16string_view expected){
    std::uintptr_t type=0;cfgnames::Observation<R> names(r,module);cfgnames::Text name;
    return r.Read(object+cfgprobe::ObjectClass,type)&&names.Object(type,name)&&name.View()==expected;
}
template<class R,class H> bool Observe(R& r,H& h,std::uintptr_t module,std::uintptr_t controller,
    std::uintptr_t gameState,std::uintptr_t state,unsigned sourceIndex,Proof& output,const cfgstatussurface::Binding* status=nullptr){
    output={};Proof p;std::uintptr_t hudType=0,listType=0,itemType=0,stateType=0,root=0,display=0,hud=0,rowDefault=0;
    cfgprobe::lifetime::SlotObservation rowTypeSlot{},rowDefaultSlot{};cfgnames::Text rowName;cfgnames::Observation<R> names(r,module);
    using namespace cfgsweater::huddisk;
    if(!h.Scope()||!h.NativeClass(SCInGameHUDReflection,SCInGameHUDConstructor,hudType)||
       !h.NativeClass(disk::ListClass,disk::ListConstructor,listType)||
       !h.NativeClass(disk::ItemClass,disk::ItemConstructor,itemType)||
       !h.NativeClass(cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateType)||
       !r.Read(controller+ControllerHUD,hud)||!h.Slot(hud,p.hud)||!p.hud.HasAssignedSerial()||!cfgspawn::ObjectIs(r,hud,hudType)||
       !cfgobserver::Same(r,hud+0x368,controller))return false;
    if(status){
        if(status->hud.object!=hud||!cfgstatussurface::Current(r,h,module,controller,*status))return false;
        p.root=status->root;p.display=status->display;root=p.root.object;display=p.display.object;
    }else if(!r.Read(hud+disk::Scoreboard,root)||!h.Slot(root,p.root)||!p.root.HasAssignedSerial()||
       !Named(r,module,root,u"ScoreboardWidget_C")||!r.Read(root+disk::Display,display)||
       !h.Slot(display,p.display)||!p.display.HasAssignedSerial())return false;
    if(
       !Named(r,module,display,u"ScoreboardDisplayWidget_C")||!cfgspawn::ObjectIs(r,display,listType)||
       !r.Read(display,p.table)||!cfgobserver::Same(r,p.table+disk::RegisterSlot,module+disk::Register)||
       !cfgbundle::Code(r,module,disk::Register,disk::RegisterCode)||
       !r.Read(display+disk::RowClass,p.rowClass)||!cfgspawn::ClassContains(r,p.rowClass,itemType)||
       !h.Slot(p.rowClass,rowTypeSlot)||!names.Object(p.rowClass,rowName)||rowName.View()!=u"ScoreboardPlayerListItem_C"||
       !r.Read(p.rowClass+cfgspawn::ClassDefaultObject,rowDefault)||!h.Slot(rowDefault,rowDefaultSlot)||
       !cfgobserver::Same(r,rowDefault+cfgprobe::ObjectClass,p.rowClass)||
       !r.Read(gameState+disk::Players,p.participants)||!p.participants.Sane()||sourceIndex>=static_cast<unsigned>(p.participants.count)||
       !cfgobserver::Same(r,p.participants.data+8ULL*sourceIndex,state)||!h.Slot(state,p.state)||
       !cfgspawn::ObjectIs(r,state,stateType)||!r.Read(state+0x88,p.stateOwner)||
       !r.Read(display+disk::Registered,p.registered)||!p.registered.Sane()||
       !r.Read(display+disk::Rows,p.rows)||!p.rows.Sane())return false;
    // Serial0 PlayerState is allowed ONLY through this fresh native strong
    // array member and the host's tracked counselor/controller proof.
    for(int i=0;i<p.registered.count;++i){std::uintptr_t existing=0;
        if(!r.Read(p.registered.data+8ULL*i,existing))return false;
        if(existing==state)p.present=true;
    }
    for(int i=0;i<p.rows.count;++i){std::uintptr_t row=0,existing=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!r.Read(p.rows.data+8ULL*i,row)||!h.Slot(row,slot)||!slot.HasAssignedSerial()||
           !cfgobserver::Same(r,row+cfgprobe::ObjectClass,p.rowClass)||!cfgobserver::Same(r,row+0x20,display)||
           !r.Read(row+disk::RowPlayerState,existing)||!cfgobserver::Same(r,p.rows.data+8ULL*i,row))return false;
        if(existing==state)p.rowPresent=true;
    }
    if(!SameList(r,gameState+disk::Players,p.participants)||!SameList(r,display+disk::Registered,p.registered)||
       !SameList(r,display+disk::Rows,p.rows)||
       !cfgobserver::Same(r,controller+ControllerHUD,hud)||
       !(status?cfgstatussurface::Current(r,h,module,controller,*status):
           cfgobserver::Same(r,hud+disk::Scoreboard,root)&&cfgobserver::Same(r,root+disk::Display,display))||
       !cfgobserver::Same(r,display+disk::RowClass,p.rowClass)||
       !cfgobserver::Same(r,display,p.table)||!cfgobserver::Same(r,p.table+disk::RegisterSlot,module+disk::Register)||!h.Scope())return false;
    p.sourceIndex=sourceIndex;output=p;return true;
}
}
