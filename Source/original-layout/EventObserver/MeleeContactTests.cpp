#define main PriorKillTests
#include "KillRuntimeTests.cpp"
#undef main
#include <map>
#include <cstring>
struct ContactReader {
 std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,cap=512;bool fail=false;
 template<class T>void Put(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);
  for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
 template<class T>bool Read(std::uintptr_t at,T& value){if(fail||++reads>cap)return false;
  auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){const auto f=bytes.find(at+i);if(f==bytes.end())return false;p[i]=f->second;}return true;}
};
struct ContactHost:Host {bool current=true;unsigned contactChecks=0;
 bool RequiresFinishingStrike()const{return true;}
 bool MeleeContactCurrent(const cfgkillcontact::Proof&){++contactChecks;return current;}
};
static cfgkillcontact::Proof Contact(const Runtime& r){cfgkillcontact::Proof p;
 p.armed=true;p.identity=r.policy.identity;p.context=r.policy.context;p.usedAt=r.policy.usedAt;p.acceptedAt=r.policy.acceptedAt;
 p.source={};p.source.object=r.policy.finisher;p.weapon=r.policy.weapon;
 p.after.list={0x2000,1,4};p.after.actors[0]=r.policy.identity.jason;return p;}
int main(){PriorKillTests();unsigned checks=0;const auto check=[&](bool okay){++checks;assert(okay);};
 {ContactHost h;auto r=Armed(h);check(HitNow(r,h));h.context={10,11,12,false};check(r.Tick(h,102)==Step::ContextReady);
  cfgkillcontact::Tracker tracker;auto p=Contact(r);p.before=p.after;
  check(!tracker.Consider(p)); // first baseline can ALREADY contain the first kneeling hit
  p=Contact(r);p.before=p.after;check(!tracker.Consider(p)); // repeated old list is not new contact
  p=Contact(r);p.before={};check(!tracker.Consider(p)); // native next-attack reset observed
  p=Contact(r);p.before=p.after;check(tracker.Consider(p)&&r.ObserveMeleeContact(h,p,103));
  p=Contact(r);p.before=p.after;check(!tracker.Consider(p));tracker.Reset();
  check(!tracker.have&&!tracker.next);
  p=Contact(r);p.before={};check(!tracker.Consider(p));
  p=Contact(r);p.before=p.after;++p.usedAt;check(!tracker.Consider(p)); // no cross-use reuse
 }
 {cfgkillcontact::Contacts before,after;before.list={0x2000,1,4};before.actors[0]=99;
  after.list={0x3000,2,4};after.actors[0]=99;after.actors[1]=2;
  check(cfgkillcontact::AddedTarget(before,after,2,10)); // reallocated buffer, preserved native prefix
  after.actors[0]=98;check(!cfgkillcontact::AddedTarget(before,after,2,10));
  after.actors[0]=99;after.actors[1]=88;check(!cfgkillcontact::AddedTarget(before,after,2,10));
  before.actors[0]=2;after.actors[0]=2;after.actors[1]=10;
  check(!cfgkillcontact::AddedTarget(before,after,2,10)); // old Jason in same attack cannot finish again
  before.list.count=65;check(!cfgkillcontact::AddedTarget(before,after,2,10));}
 {ContactReader r;cfgkillcontact::Contacts c;const cfgkillcontact::List list{0x2000,2,4};
  r.Put(0x1000,list);r.Put(0x2000,std::uintptr_t{2});r.Put(0x2008,std::uintptr_t{10});
  check(cfgkillcontact::Read(r,0x1000,c)&&c.actors[0]==2&&c.actors[1]==10);
  r.Put(0x2008,std::uintptr_t{2});check(!cfgkillcontact::Read(r,0x1000,c));
  r.Put(0x1000,cfgkillcontact::List{0x2000,-1,4});check(!cfgkillcontact::Read(r,0x1000,c));
  r.Put(0x1000,cfgkillcontact::List{0x2000,2,65});check(!cfgkillcontact::Read(r,0x1000,c));
  r.Put(0x1000,cfgkillcontact::List{0x2001,2,4});check(!cfgkillcontact::Read(r,0x1000,c));}
 for(unsigned fault=0;fault<20;++fault){ContactHost h;auto r=Armed(h);check(HitNow(r,h));h.context={10,11,12,false};
  check(r.Tick(h,102)==Step::ContextReady&&!h.interactions);auto p=Contact(r);auto now=std::uint64_t{103};
  if(fault==1)p.armed=false;if(fault==2)p.identity.world=20;if(fault==3)p.context=20;
  if(fault==4)++p.usedAt;if(fault==5)++p.acceptedAt;if(fault==6)p.source.object=44;
  if(fault==7)p.weapon=66;if(fault==8)p.after.list.count=0;if(fault==9)p.after.actors[0]=99;
  if(fault==10){p.before=p.after;} // old native contact is not a second strike
  if(fault==11)h.squared=40001;if(fault==12)h.mask=false;if(fault==13)h.kind=Weapon::Other;
  if(fault==14)h.context.active=true;if(fault==15)h.current=false;if(fault==16)h.living=false;
  if(fault==17)now=r.policy.acceptedAt;if(fault==18)now=r.policy.until;
  if(fault==19)p.after.actors[0]=10; // exact owned death context also receives native blocking contact
  const auto until=r.policy.until;const bool accepted=r.ObserveMeleeContact(h,p,now);
  check(accepted==(fault==0||fault==19));check(r.policy.until==until&&!h.interactions);
  if(accepted){check(!r.ObserveMeleeContact(h,p,now+1));check(r.Tick(h,now+2)==Step::InteractionDispatched&&h.interactions==1);}
 }
 std::printf("Native melee contact delta/kill guards: %u checks PASS; no damage fabricated, frozen timing unchanged.\n",checks);return 0;}
