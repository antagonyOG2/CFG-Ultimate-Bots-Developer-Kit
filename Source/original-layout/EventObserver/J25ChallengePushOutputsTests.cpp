// Real NativeRetentionScope ordering with a bounded synthetic native stack.
// No Windows process, native dispatch, or game access in this fixture.
#include "NativeMenuRetention.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
namespace {
using cfgmenu::rows::NativeRetentionHooks;using cfgmenu::rows::NativeRetentionScope;
struct Packet{std::uintptr_t type=0;std::uint8_t skip=1;unsigned char pad[7]{};std::uintptr_t result=0;};
static_assert(sizeof(Packet)==24);
struct Mock {
 static constexpr std::uintptr_t Module=0x100000,Source=0x200000,Page=0x300000;
 static constexpr std::uintptr_t LobbyType=0x400000,PageType=0x500000,Push=0x600000,Pop=0x700000,Other=0x800000,Forward=0x900000;
 std::uintptr_t top=Source,type=LobbyType,boundTop=Source,boundType=LobbyType;
 unsigned count=1,boundCount=1,pending=0,attempts=0,inputs=0,outputs=0,postChecks=0,preChecks=0,forwards=0;
 bool liveWorld=true,liveSource=true,hookAccept=true,postMutate=false,uncertain=false,wrongOwner=false,departed=false,tail=false;
 bool Current()const{return !wrongOwner&&(tail?departed:(liveWorld&&top==boundTop&&type==boundType&&count==boundCount));}
 bool Expected(unsigned kind,std::uintptr_t function,void* packet)const{
  if(!liveSource||wrongOwner||pending!=kind||!packet)return false;
  if(kind==3)return function==Forward&&departed&&count==0&&top==0&&boundCount==1&&boundTop==Source;
  if(!liveWorld)return false;
  if(kind==1){auto& p=*static_cast<Packet*>(packet);
   return function==Push&&boundTop==Source&&boundType==LobbyType&&boundCount==1&&count==2&&top==Page&&type==PageType&&
    p.type==PageType&&p.result==Page&&p.skip==1;}
  if(kind==2){return function==Pop&&boundTop==Page&&boundType==PageType&&boundCount==2&&count==1&&top==Source&&type==LobbyType;}
  return false;
 }
 NativeRetentionHooks Hooks(){return {this,
  [](void* p,std::uintptr_t){return static_cast<Mock*>(p)->Current();},
  [](void* p,std::uintptr_t,std::uintptr_t a,std::uintptr_t f,void*){auto& m=*static_cast<Mock*>(p);++m.inputs;
   if(a!=m.top||!m.Current())return false;
   m.pending=f==Push?1:f==Pop?2:f==Forward?3:0;return true;},
  [](void* p,std::uintptr_t,std::uintptr_t f,void* packet){auto& m=*static_cast<Mock*>(p);++m.outputs;
   if(!m.hookAccept||(m.pending&&!m.Expected(m.pending,f,packet))||!m.CurrentForOutputs(f,packet))return false;
   if(m.pending==3)m.tail=true;
   m.boundTop=m.top;m.boundType=m.type;m.boundCount=m.count;m.pending=0;
   if(m.postMutate)m.type=0xBAD000;
   return true;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return true;},
  [](void*,std::uintptr_t,std::uintptr_t){return true;},
  [](void* p){static_cast<Mock*>(p)->uncertain=true;},[](void*){},[](void*){return true;},
  [](void* p,std::uintptr_t,std::uintptr_t f,void* packet){return static_cast<Mock*>(p)->CurrentForOutputs(f,packet);}};}
 bool CurrentForOutputs(std::uintptr_t f,void* packet){++preChecks;
  if(CFG_J25_CHALLENGE_PUSH_OUTPUTS&&pending&&(f==Push||f==Pop||f==Forward))return Expected(pending,f,packet);
  return Current();
 }
 bool Run(std::uintptr_t function,void* packet,bool mutate=true){
  NativeRetentionScope::ResetOutputFailure();
  if(!NativeRetentionScope::Inputs(Module,top,function,packet))return false;
  if(!NativeRetentionScope::EnterNative(Module))return false;
  ++attempts;
  if(mutate){if(function==Push){count=2;top=Page;type=PageType;static_cast<Packet*>(packet)->result=Page;}
   else if(function==Pop){count=1;top=Source;type=LobbyType;}
   else if(function==Forward){count=0;top=0;departed=true;liveWorld=false;}
   else{count=2;top=Page;type=PageType;}}
  return NativeRetentionScope::Outputs(Module,function,packet);
 }
};
}
int main(){unsigned checks=0;auto check=[&](bool ok){++checks;if(!ok){std::printf("FAIL J25 scope check %u\n",checks);std::abort();}};
 {Mock m;Packet push{Mock::PageType};NativeRetentionScope scope(Mock::Module,m.Hooks(),true);check(scope.Entered());
  const bool pushed=m.Run(Mock::Push,&push);check(pushed==bool(CFG_J25_CHALLENGE_PUSH_OUTPUTS));
  if(CFG_J25_CHALLENGE_PUSH_OUTPUTS){check(m.outputs==1&&!m.uncertain&&m.attempts==1&&m.count==2);
   std::vector<unsigned> rows;for(unsigned i=0;i<8;++i)if(m.Current())rows.push_back(i);
   check(rows.size()==8); // UI row construction remains covered separately by production J22 fixture
   unsigned start=0;Packet pop{};if(rows.size()==8){++start;check(m.Run(Mock::Pop,&pop));++m.forwards;}
   check(start==1&&m.forwards==1&&m.count==1&&m.top==Mock::Source&&m.attempts==2&&m.Current());
   Packet forwarded{};check(m.Run(Mock::Forward,&forwarded)&&m.tail&&m.departed&&m.attempts==3&&m.Current());
  }else check(NativeRetentionScope::OutputFailure()==1&&m.uncertain&&m.attempts==1);}
 {Mock m;Packet p{Mock::PageType};NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(!m.Run(Mock::Push,&p,false)&&NativeRetentionScope::OutputFailure()==(CFG_J25_CHALLENGE_PUSH_OUTPUTS?1u:2u)&&m.attempts==1);}
 {Mock m;Packet p{Mock::PageType};NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(NativeRetentionScope::Inputs(Mock::Module,m.top,Mock::Push,&p));
  ++m.attempts;m.count=2;m.top=Mock::Page;m.type=Mock::PageType;p.result=0;
  check(!NativeRetentionScope::Outputs(Mock::Module,Mock::Push,&p)&&NativeRetentionScope::OutputFailure()==1&&m.uncertain);}
 {Mock m;Packet p{Mock::PageType};NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(NativeRetentionScope::Inputs(Mock::Module,m.top,Mock::Push,&p));
  ++m.attempts;m.count=2;m.top=Mock::Page;m.type=Mock::LobbyType;p.result=Mock::Page;
  check(!NativeRetentionScope::Outputs(Mock::Module,Mock::Push,&p)&&NativeRetentionScope::OutputFailure()==1);}
 {Mock m;Packet p{};NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(!m.Run(Mock::Other,&p)&&NativeRetentionScope::OutputFailure()==1&&m.uncertain&&m.attempts==1);}
 {Mock m;Packet p{Mock::PageType};m.hookAccept=false;NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(!m.Run(Mock::Push,&p));check(NativeRetentionScope::OutputFailure()==(CFG_J25_CHALLENGE_PUSH_OUTPUTS?2u:1u));}
 {Mock m;Packet p{Mock::PageType};m.postMutate=true;NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(!m.Run(Mock::Push,&p));check(NativeRetentionScope::OutputFailure()==(CFG_J25_CHALLENGE_PUSH_OUTPUTS?3u:1u));}
 {Mock m;Packet p{Mock::PageType};m.liveWorld=false;NativeRetentionScope scope(Mock::Module,m.Hooks(),true);
  check(!m.Run(Mock::Push,&p)&&m.attempts==0);}
 std::printf("J25 scope transition fixture: %u checks PASS (flag=%d), exact real Inputs/native/Outputs ordering, push/pop/8 rows mock, null/wrong class/foreign/uncertain; no game access.\n",checks,CFG_J25_CHALLENGE_PUSH_OUTPUTS);
}
