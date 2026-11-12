#pragma once
#include "../RuntimeProbe/NativeClassPathChoice.hpp"
#include "NativeCounselorPromotion.hpp"
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
#include "../Core/ResidentCounselorSelection.hpp"
namespace cfghostedpacket {
#ifndef CFG_H243_COUNSELOR
#define CFG_H243_COUNSELOR 1 // donor catalogue match of the displaced human's own PickedCounselorClass (OfflineSetup.cpp:10230-10259)
#endif
struct Diagnostic {unsigned reason=0,kind=0,index=0;std::uintptr_t address=0;
 // H243: kind-2 index is the matched CounselorCharacterClasses entry (donor "preferred-matched"), NOT a fallback slot.
 // matchState: 0=not attempted 1=matched 2=unmatched 3=catalogue unreadable
 unsigned matchState=0,catalogueCount=0;};
struct Source {std::uintptr_t address=0,owner=0;unsigned kind=0,index=0;cfgspawn::PlayerArray catalogue{};cfgmenu::promotion::Packet packet{};};
// The 40-byte soft class stores FName asset path at16, optional FString SUBPATH
// at24. A zero subpath is normal, not an absent counselor class (H25 stage14/6).
template<class R>bool Read(R& r,std::uintptr_t module,std::uintptr_t at,cfgmenu::promotion::Packet& out,Diagnostic& d){
 d.address=at;d.reason=1;cfgmenu::promotion::Packet p,again;
 if(!r.Read(at,p))return false;
 cfgnames::Words name{};std::memcpy(&name,p.bytes.data()+16,8);
 d.reason=2;if(name.index<=0||name.number)return false;
 d.reason=3;for(unsigned i=24;i<40;++i)if(p.bytes[i])return false;
 cfgnames::Observation<R> names(r,module);cfgnames::PathText path;
 d.reason=4;if(!names.Decode(name,path))return false;
 const auto text=path.View();const auto dot=text.rfind(u'.');
 d.reason=5;if(text.substr(0,6)!=u"/Game/"||dot==text.npos||dot+1>=text.size()||
    text.substr(text.size()-2)!=u"_C"||!cfgresidentcounselor::Ordinary(text.substr(dot+1)))return false;
 d.reason=6;if(!r.Read(at,again)||p.bytes!=again.bytes)return false;
 out=p;d.reason=0;return true;
}
template<class R>bool Current(R& r,std::uintptr_t module,const Source& s,Diagnostic& d){
 d.kind=s.kind;d.index=s.index;cfgmenu::promotion::Packet packet;
 if(!s.owner||!s.address||!Read(r,module,s.address,packet,d)||packet.bytes!=s.packet.bytes){if(!d.reason)d.reason=7;return false;}
 if(s.kind==1){cfgspawn::PlayerArray header{};d.reason=8;
  if(!r.Read(s.owner+cfgmenu::cataloguedisk::CounselorCharacterClasses,header)||header.data!=s.catalogue.data||
     header.count!=s.catalogue.count||header.capacity!=s.catalogue.capacity||s.index>=static_cast<unsigned>(header.count)||s.address!=header.data+40ULL*s.index)return false;
 }else if(s.kind!=2||s.address!=s.owner+cfgmenu::birthdisk::PickedCounselorClass){d.reason=9;return false;}
 d.reason=0;return true;
}
// Donor OfflineSetup.cpp:10230-10259: read the displaced human's OWN PickedCounselorClass, then match it
// against mode CounselorCharacterClasses. Match is by the soft-class FName (same asset path identity the donor
// compares as text). Unmatched / unreadable catalogue keeps the already-proven preference packet (no new refusal).
template<class R>void MatchCatalogue(R& r,std::uintptr_t module,std::uintptr_t mode,Source& s,Diagnostic& d){
 d.matchState=0;d.catalogueCount=0;
#if CFG_H243_COUNSELOR
 if(!mode)return;
 cfgspawn::PlayerArray header{};
 if(!r.Read(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,header)||!cfgprimary::SaneCatalogue(header)||header.count<=0){d.matchState=3;s.index=d.index=0xFFFEu;return;}
 d.catalogueCount=static_cast<unsigned>(header.count);d.matchState=2;s.index=d.index=0xFFFFu;
 for(unsigned i=0;i<static_cast<unsigned>(header.count);++i){
  cfgmenu::promotion::Packet entry;Diagnostic scratch;
  if(!Read(r,module,header.data+40ULL*i,entry,scratch))continue;
  if(std::memcmp(entry.bytes.data()+16,s.packet.bytes.data()+16,8)==0){s.index=i;d.index=i;d.matchState=1;return;}
 }
#else
 (void)r;(void)module;(void)mode;(void)s;
#endif
}
template<class R>bool Preferred(R& r,std::uintptr_t module,std::uintptr_t state,Source& out,Diagnostic& d,std::uintptr_t mode=0){
 Source s;s.kind=2;s.owner=state;s.address=state+cfgmenu::birthdisk::PickedCounselorClass;
 if(!state||!Read(r,module,s.address,s.packet,d)||!Current(r,module,s,d))return false;
#if CFG_H243_COUNSELOR
 MatchCatalogue(r,module,mode,s,d);
#endif
 out=s;return true;
}
template<class R,std::size_t N>bool Choose(R& r,std::uintptr_t module,std::uintptr_t mode,
 const std::array<std::uintptr_t,N>& states,unsigned count,unsigned seed,Source& out,Diagnostic& d){
 out={};d={};cfgspawn::PlayerArray header{};
 if(r.Read(mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,header)&&cfgprimary::SaneCatalogue(header)&&header.count>0){
  const auto size=static_cast<unsigned>(header.count),start=seed%size;
  for(unsigned i=0;i<size;++i){Source s;s.kind=1;s.index=(start+i)%size;s.owner=mode;s.catalogue=header;s.address=header.data+40ULL*s.index;
   d.kind=s.kind;d.index=s.index;
   if(!Read(r,module,s.address,s.packet,d))continue;
   if(!Current(r,module,s,d))return false;out=s;return true;
  }
 }
 // A current registered preference is a fallback, never required in 2-human
 // lobbies. It uses the same FName packet proof, not invented FString storage.
 for(unsigned i=0;i<count&&i<N;++i){if(!states[i])continue;Source s;s.kind=2;s.index=i;s.owner=states[i];s.address=s.owner+cfgmenu::birthdisk::PickedCounselorClass;
  d.kind=s.kind;d.index=s.index;if(Read(r,module,s.address,s.packet,d)&&Current(r,module,s,d)){out=s;return true;}}
 if(!d.reason)d.reason=10;return false;
}
}
