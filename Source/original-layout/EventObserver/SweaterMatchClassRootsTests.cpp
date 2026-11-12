#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "FreshAbilityClassObservation.hpp"
#include "PermanentRootPool.hpp"
#define main RootSparseResourceMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
struct ClassRegistry {
 cfggcretain::ReferencerList list{0x10000,1,256};std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{0x20000};
 bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values){out=list;values=entries;return true;}
 bool Register(std::uintptr_t owner){entries[list.count++]=owner;return true;}
 bool Unregister(std::uintptr_t owner){for(int i=0;i<list.count;++i)if(entries[i]==owner){--list.count;for(int j=i;j<list.count;++j)entries[j]=entries[j+1];entries[list.count]=0;return true;}return false;}
};
struct RootedClassFixture:Fixture {
 cfgretain::PermanentRootPool<4> roots;
 template<class T>bool Read(std::uintptr_t at,T& out){
  for(unsigned i=0;i<roots.Count();++i){const auto base=roots.Address(i);
   if(at>=base&&at-base<=sizeof(cfggcretain::Owner)&&sizeof(T)<=sizeof(cfggcretain::Owner)-(at-base)){
    if(reads++>=cap)return false;std::memcpy(&out,reinterpret_cast<const void*>(at),sizeof(T));return true;}}
  return Fixture::Read(at,out);
 }
 bool Current(const cfgsweater::ClassLease& saved){auto lease=saved;return lease.Current(*this,objects,module,
  roots.CurrentReference(*this,lease.slot.object)&&roots.CurrentReference(*this,lease.defaultSlot.object));}
};
int main(){RootedClassFixture f;ClassRegistry registry;f.cap=100000;
 const auto native=f.Object(),derived=f.Object(),defaults=f.Object(derived),ability=f.Object(derived);
 const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
 f.Put(f.module+binding.reflection,native);f.Put(f.module+binding.constructor,native);
 f.Put(derived+cfgprobe::ClassSuper,native);f.Put(derived+cfgspawn::ClassDefaultObject,defaults);
 f.Put(f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,f.objects.data);
 f.Put(f.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,f.objects.count);
 int abilityIndex=0;CHECK(f.Read(ability+cfgprobe::lifetime::ObjectIndexOffset,abilityIndex));
 std::uintptr_t reused=0;
 for(unsigned match=0;match<10;++match){f.reads=0;f.Put(ability+cfgprobe::ObjectClass,derived);
  f.Put(f.objects.data+24ULL*static_cast<unsigned>(abilityIndex),ability);f.Put(f.objects.data+24ULL*static_cast<unsigned>(abilityIndex)+16,int{77});
  cfgsweater::ClassLease lease;CHECK(lease.Capture(f,f.objects,f.module,ability)&&lease.anchor==4);
  cfggcprobe::Report report;CHECK(f.roots.AddLocked(registry,std::array<std::uintptr_t,2>{derived,defaults},report));
  if(!reused)reused=f.roots.Address(0);CHECK(f.roots.Address(0)==reused&&f.roots.Count()==1);
  lease.anchor=8;lease.earnedSlot={};lease.earnedOwner={};
  f.Put(ability+cfgprobe::ObjectClass,std::uintptr_t{});f.Put(f.objects.data+24ULL*static_cast<unsigned>(abilityIndex),std::uintptr_t{});
  CHECK(f.Current(lease));CHECK(!lease.Current(f,f.objects,f.module)); // copied bare anchor8 never grants authority
  auto& owner=*reinterpret_cast<cfggcretain::Owner*>(f.roots.Address(0));
  owner.CollectWith([](std::uintptr_t*)noexcept{});CHECK(f.Current(lease)&&owner.submissions.load()==2);
  f.reads=0;f.cap=0;CHECK(!f.Current(lease));f.cap=100000;
  const auto saved=owner.slots[0];owner.slots[0]=0;CHECK(!f.Current(lease));owner.slots[0]=saved;
  int classIndex=0;CHECK(f.Read(derived+cfgprobe::lifetime::ObjectIndexOffset,classIndex));
  f.Put(f.objects.data+24ULL*static_cast<unsigned>(classIndex)+8,std::uint32_t{0x10000000});CHECK(!f.Current(lease));
  f.Put(f.objects.data+24ULL*static_cast<unsigned>(classIndex)+8,std::uint32_t{});
  CHECK(f.roots.ReleaseAllLocked(registry,report)&&!f.roots.Count()&&registry.list.count==1);CHECK(!f.Current(lease));
 }
 std::printf("S11 retained exact class/CDO after native instance retirement: %d failures; ten reused match owners, real collector and removal, no native/game calls.\n",failures);return failures?1:0;
}