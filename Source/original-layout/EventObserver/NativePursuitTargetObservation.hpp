#pragma once
#include "CounselorTerminalObservation.hpp"
namespace cfgnativepursuit {
// Native terminal witness is only a discovery hint. Every use repeats the
// bounded owned roster, positive actors, native AI type and possession chain.
struct List {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
struct Proof {
    List list{};unsigned index=0;
    cfgprobe::lifetime::SlotObservation pawn{},controller{};
};
template<class R>bool Observe(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t module,std::uintptr_t gameState,std::uintptr_t counselorType,
    std::uintptr_t aiType,std::uintptr_t jason,unsigned index,std::uintptr_t expected,
    Proof& out,const Proof* retained=nullptr){
    out={};List list{};std::uintptr_t pawn=0,controller=0;cfgescape::State state;
    cfgprobe::lifetime::SlotObservation ps{},cs{},fresh{};
    if(!r.Read(gameState+cfgescape::disk::PlayerCharacterList,list)||list.count<1||list.count>16||
       list.capacity<list.count||list.capacity>32||!cfgprobe::ObjectPointer(list.data)||index>=static_cast<unsigned>(list.count)||
       !r.Read(list.data+8*index,pawn)||!pawn||pawn==jason||pawn!=expected||
       !cfgspawn::ObjectSlot(r,objects,pawn,ps)||!ps.HasAssignedSerial()||
       (retained&&(!(retained->list==list)||retained->index!=index||!(retained->pawn==ps)))||
       !cfgescape::Counselor(r,objects,module,pawn,counselorType,state)||state!=cfgescape::State::Living||
       !r.Read(pawn+cfgescape::disk::Controller,controller)||
       !cfgspawn::ObjectSlot(r,objects,controller,cs)||!cs.HasAssignedSerial()||
       !cfgspawn::ObjectIs(r,controller,aiType)||(retained&&!(retained->controller==cs))||
       !cfgobserver::Same(r,controller+cfgescape::disk::Pawn,pawn)||
       !cfgobserver::Same(r,pawn+cfgescape::disk::Controller,controller)||
       !cfgspawn::ObjectSlot(r,objects,pawn,fresh)||!(fresh==ps)||
       !cfgspawn::ObjectSlot(r,objects,controller,fresh)||!(fresh==cs)||
       !cfgobserver::Same(r,list.data+8*index,pawn)||
       !cfgescape::Same(r,gameState+cfgescape::disk::PlayerCharacterList,list))return false;
    out={list,index,ps,cs};return true;
}
}
