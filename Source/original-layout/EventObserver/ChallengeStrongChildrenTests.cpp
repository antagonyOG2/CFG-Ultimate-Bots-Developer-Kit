#include "../Core/ChallengeIntroDispatchPolicy.hpp"
#include "../Core/ChallengeStrongChildrenPolicy.hpp"
#include <map>
#include <cassert>
#include <cstdio>
struct Reader {std::map<std::uintptr_t,std::uint32_t> mem;template<class T>bool Read(std::uintptr_t a,T& v){auto i=mem.find(a);if(i==mem.end())return false;v=static_cast<T>(i->second);return true;}};
struct OwnerHost {
 Reader r;unsigned calls=0;bool linkWorld=true,linkSettings=true,linkBack=true,indices=true,worldPositive=true;
 bool CurrentIntro(const cfgmenu::challengeintro::DispatchRequest& q){
  return q.Valid()&&worldPositive&&indices&&linkWorld&&linkSettings&&linkBack&&
   cfgmenu::challengeintro::ChildrenFlagsClear(r,0x10000,q.levelIndex,q.settingsIndex);
 }
 bool InvokeRetainedIntro(const cfgmenu::challengeintro::DispatchRequest&){++calls;return true;}
};
int main(){using namespace cfgmenu::challengeintro;
 DispatchRequest q{9,1,2,3,4,5,1,0,0,7,1100,4,7,true};
 OwnerHost h;h.r.mem[0x10000+24*4+8]=0;h.r.mem[0x10000+24*7+8]=0;
 assert(q.Valid()==static_cast<bool>(CFG_J33_INTRO_STRONG_CHILDREN));
 if(!CFG_J33_INTRO_STRONG_CHILDREN){DispatchReceipt receipt;assert(!receipt.Post(q,100));std::puts("J33 OFF refuses zero-serial children PASS");return 0;}
 {DispatchReceipt d;assert(d.Post(q,100));assert(d.Service(h,101)&&h.calls==1);assert(!d.Service(h,102)&&h.calls==1);}
 for(unsigned fault=0;fault<7;++fault){OwnerHost bad=h;bad.calls=0;
  if(fault==0)bad.linkWorld=false;
  if(fault==1)bad.linkSettings=false;
  if(fault==2)bad.linkBack=false;
  if(fault==3)bad.indices=false;
  if(fault==4)bad.worldPositive=false;
  if(fault==5)bad.r.mem[0x10000+24*4+8]=0x10000000;
  if(fault==6)bad.r.mem[0x10000+24*7+8]=0x20000000;
  DispatchReceipt d;assert(d.Post(q,100));assert(!d.Service(bad,101)&&bad.calls==0);assert(!d.Service(bad,102));
 }
 {OwnerHost changed=h;changed.calls=0;DispatchReceipt d;assert(d.Post(q,100));changed.linkSettings=false;
  assert(!d.Service(changed,101)&&changed.calls==0);}
 std::puts("J33 strong owner policy/GC/index/one-shot PASS; simulated retained host");
}
