#pragma once
#include <array>
#include <cstdint>
#include "../Core/KillPolicy.hpp"
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgkillcontact {
constexpr unsigned MaximumContacts=64;
struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
 bool operator==(const List& x)const{return data==x.data&&count==x.count&&capacity==x.capacity;}};
struct Contacts {List list{};std::array<std::uintptr_t,MaximumContacts> actors{};};
struct Proof {
 cfgkill::Identity identity{};cfgprobe::lifetime::SlotObservation source{};
 std::uintptr_t function=0,context=0,weapon=0;
 std::uint64_t usedAt=0,acceptedAt=0;Contacts before{},after{};unsigned beforeReads=0;bool armed=false;
};
template<class Reader>bool Read(Reader& r,std::uintptr_t address,Contacts& out){
 out={};List again{};
 if(!r.Read(address,out.list)||out.list.count<0||out.list.capacity<out.list.count||
    out.list.capacity>static_cast<int>(MaximumContacts)||(out.list.count&&!out.list.data)||
    (out.list.data&&out.list.data%8))return false;
 for(int i=0;i<out.list.count;++i){
  auto& actor=out.actors[static_cast<unsigned>(i)];
  if(!r.Read(out.list.data+8*static_cast<unsigned>(i),actor)||!actor)return false;
  for(int j=0;j<i;++j)if(actor==out.actors[static_cast<unsigned>(j)])return false;
 }
 if(!r.Read(address,again)||!(again==out.list))return false;
 // A changing prefix must not create a synthetic before/after contact.
 for(int i=0;i<out.list.count;++i){std::uintptr_t actor=0;
  if(!r.Read(out.list.data+8*static_cast<unsigned>(i),actor)||actor!=out.actors[static_cast<unsigned>(i)])return false;}
 return true;
}
inline bool AddedTarget(const Contacts& before,const Contacts& after,std::uintptr_t jason,std::uintptr_t context){
 if(!jason||!context||before.list.count<0||after.list.count<=before.list.count||
    after.list.count>static_cast<int>(MaximumContacts))return false;
 for(int i=0;i<before.list.count;++i){const auto n=static_cast<unsigned>(i);
  if(before.actors[n]!=after.actors[n]||before.actors[n]==jason||before.actors[n]==context)return false;}
 for(int i=before.list.count;i<after.list.count;++i){const auto actor=after.actors[static_cast<unsigned>(i)];
  if(actor==jason||actor==context)return true;}
 return false; // empty swing, old contact and unrelated actor are not finishing input
}
inline bool Same(const Contacts& a,const Contacts& b){
 if(!(a.list==b.list))return false;
 for(int i=0;i<a.list.count;++i)if(a.actors[static_cast<unsigned>(i)]!=b.actors[static_cast<unsigned>(i)])return false;
 return true;
}
// Stock Tick calls AttackTrace DIRECTLY. Sample its unique native contact list
// across owned finisher callbacks too; a reflected AttackTrace event is optional.
// First sample is baseline ONLY. A missed clear/growth safely loses a hit rather
// than manufacturing a strike from an old identical list.
struct Tracker {
 Proof prior{};bool have=false;std::uint64_t next=0;
 void Reset(){*this={};}
 bool Consider(Proof& current){
  const auto latest=current.before;
  const bool same=have&&current.identity==prior.identity&&current.source==prior.source&&
   current.context==prior.context&&current.weapon==prior.weapon&&current.usedAt==prior.usedAt&&current.acceptedAt==prior.acceptedAt;
  const bool added=same&&AddedTarget(prior.before,latest,current.identity.jason,current.context);
  const auto previous=prior.before;prior=current;have=true;
  if(added){current.before=previous;current.after=latest;}
  return added;
 }
};
}
