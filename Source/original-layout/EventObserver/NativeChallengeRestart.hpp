#pragma once
#include "NativeChallengeEscapeScope.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
namespace cfgmenu::challengerestart {
class Runtime {
 active::Context source_{};cfgchallengeescape::scope::Witness last_{};
 bool enabled_=false,seen_=false;std::uint64_t next_=0;
public:
 std::array<unsigned,challengemenu::choice::Count> choices{};unsigned restarts=0,failure=0;
 void Arm(const active::Context& source,const std::array<unsigned,challengemenu::choice::Count>& selected){
  source_=source;choices=selected;last_={};enabled_=true;seen_=false;next_=0;failure=0;
 }
 void Cancel(){enabled_=false;}
 const active::Context& Source()const{return source_;}
 template<class R>bool Observe(R& r,std::uintptr_t callback,std::uint32_t thread,std::uint64_t now,
 cfgchallengeescape::scope::Witness& out){
  out={};if(!enabled_||thread!=source_.thread||now<next_)return false;
  using namespace cfgprobe;std::uintptr_t world=0,mode=0,state=0,controller=0;
  if(!r.Read(source_.viewport+ViewportWorld,world))return false;
  if(world==source_.world){if(seen_)Cancel();return false;}
  if(!r.Read(world+WorldGameMode,mode)||!ObjectPointer(mode))return false;
  if(callback!=mode&&(!r.Read(world+WorldGameState,state)||!r.Read(source_.player+PlayerController,controller)||
     (callback!=state&&callback!=controller)))return false;
  next_=now+100;
  if(seen_&&world==last_.World()&&mode==last_.Mode()){
   lifetime::ArrayHeader array;lifetime::SlotObservation w,m;
   if(!lifetime::ReadArrayHeader(r,source_.module+lifetime::ObjectArrayRva,array)||
      !lifetime::ReadSlot(r,array,world,w)||!lifetime::ReadSlot(r,array,mode,m))return false;
   if(w==last_.owners[0]&&m==last_.owners[1])return false;
  }
  std::uintptr_t type=0,expected=0;
  if(r.Read(mode+ObjectClass,type)&&r.Read(source_.module+cfgchallengeescape::disk::SCGameMode_SPChallenges,expected)&&
     expected&&type!=expected){Cancel();return false;}
  cfgchallengeescape::scope::Runtime proof;cfgchallengeescape::scope::Witness fresh;
  if(!proof.Arm(source_,1,true,now)||!proof.Attempt(r,thread,fresh))return false;
  if(!seen_){last_=fresh;seen_=true;return false;}
  if(!(fresh.map==last_.map)||!(fresh.mission==last_.mission)||!(fresh.missionPackage==last_.missionPackage)){
   failure=1;Cancel();return false;
  }
  if(fresh.owners[0]==last_.owners[0]||fresh.owners[1]==last_.owners[1])return false;
  cfgnames::Observation<R> names(r,source_.module);cfgnames::Text phase;
  if(!names.Decode(fresh.phase,phase)||phase.View()!=u"PreMatchIntro")return false;
  last_=fresh;out=fresh;++restarts;return true;
 }
};
}
