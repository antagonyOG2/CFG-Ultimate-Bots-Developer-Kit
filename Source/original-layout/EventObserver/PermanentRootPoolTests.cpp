#include "PermanentRootPool.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Registry {
    cfggcretain::ReferencerList list{0x10000,1,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{0x20000};
    unsigned snapshots=0,adds=0,failSnapshot=0;bool reentrantCollect=false;
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept {
        if(++snapshots==failSnapshot)return false;out=list;values=entries;return true;
    }
    bool Register(std::uintptr_t owner)noexcept {
        ++adds;entries[list.count++]=owner;
        if(reentrantCollect){auto& o=*reinterpret_cast<cfggcretain::Owner*>(owner);
            CHECK(o.state.load()==cfggcretain::State::Registering);
            o.CollectWith([](std::uintptr_t*)noexcept {});}
        return true;
    }
};
template<std::size_t N>struct Reader {
    cfgretain::PermanentRootPool<N>& pool;
    unsigned reads=0,failAt=0,clearAt=0;
    template<class T>bool Read(std::uintptr_t at,T& out)noexcept {
        if(++reads==failAt)return false;
        if(reads==clearAt)reinterpret_cast<cfggcretain::Owner*>(pool.Address(0))->slots[0]=0;
        for(unsigned i=0;i<pool.Count();++i){const auto base=pool.Address(i);
            if(at>=base&&at-base<=sizeof(cfggcretain::Owner)&&sizeof(T)<=sizeof(cfggcretain::Owner)-(at-base)){
                std::memcpy(&out,reinterpret_cast<const void*>(at),sizeof(T));return true;
            }}return false;
    }
};
}
int main(){
    {cfgretain::PermanentRootPool<2> pool;Registry registry;cfggcprobe::Report report;Reader<2> reader{pool};
     CHECK(!pool.Current(reader)&&!pool.Contains(0x40000));
     CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,2>{0x40000,0x50000},report));
     CHECK(pool.Count()==1&&pool.References(0)==2&&pool.Contains(0x40000)&&pool.CurrentReference(reader,0x40000)&&pool.Current(reader));
     const auto first=pool.Address(0);report={};
     CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,3>{0x50000,0x60000,0},report));
     CHECK(pool.Count()==2&&registry.adds==2&&pool.Address(0)==first&&pool.References(1)==1&&pool.Current(reader));
     report.reads=42;pool.RecordAccess(report);
     unsigned calls,reads,callbacks,completed,submissions,cleared,destroyed;
     pool.Counters(calls,reads,callbacks,completed,submissions,cleared,destroyed);
     CHECK(calls==2&&reads==42&&!callbacks&&!destroyed);
     report={};CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,3>{0x40000,0x50000,0x60000},report));
     CHECK(pool.Count()==2&&registry.adds==2&&pool.Current(reader));
     CHECK(!pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x70000},report));
     CHECK(pool.Poisoned()&&pool.Count()==2&&registry.adds==2&&!pool.Current(reader)&&!pool.Contains(0x40000));}
    {cfgretain::PermanentRootPool<2> pool;Registry registry;cfggcprobe::Report report;registry.reentrantCollect=true;
     CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,2>{0x40000,0x50000},report));
     Reader<2> reader{pool};CHECK(pool.Current(reader));
     auto& owner=*reinterpret_cast<cfggcretain::Owner*>(pool.Address(0));
     CHECK(owner.callbacks.load()==1&&owner.completedCallbacks.load()==1&&owner.submissions.load()==2);
     owner.slots[0]=0;CHECK(!owner.clearedSlots.load()&&!pool.Current(reader)&&!pool.CurrentReference(reader,0x40000));}
    {cfgretain::PermanentRootPool<2> pool;Registry registry;cfggcprobe::Report report;registry.failSnapshot=2;
     CHECK(!pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x40000},report));
     CHECK(pool.Poisoned()&&pool.Count()==1&&registry.adds==1);
     const auto address=pool.Address(0);report={};
     CHECK(!pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x50000},report)&&pool.Address(0)==address&&registry.adds==1);}
    {cfgretain::PermanentRootPool<2> pool;Registry registry;cfggcprobe::Report report;
     CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x40000},report));
     Reader<2> reader{pool};reader.failAt=2;CHECK(!pool.Current(reader));
     Reader<2> changed{pool};changed.clearAt=2;CHECK(!pool.Current(changed));}
    {cfgretain::PermanentRootPool<2> pool;Registry registry;cfggcprobe::Report report;
     CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x40000},report));
     Reader<2> reader{pool};auto& owner=*reinterpret_cast<cfggcretain::Owner*>(pool.Address(0));
     owner.callbacks.store(1);owner.submissions.store(1);CHECK(!pool.Current(reader));
     owner.completedCallbacks.store(1);CHECK(pool.Current(reader));
     owner.destroyRequests.store(1);CHECK(!pool.Current(reader));}
    {cfgretain::PermanentRootPool<32> pool;Registry registry;cfggcprobe::Report report;
     std::array<std::uintptr_t,32> roots{};
     for(unsigned group=0;group<32;++group){
      for(unsigned slot=0;slot<roots.size();++slot)roots[slot]=0x100000+group*0x10000+slot*0x100;
      CHECK(pool.AddLocked(registry,roots,report));
     }
     Reader<32> reader{pool};CHECK(pool.Count()==32&&registry.adds==32&&pool.Current(reader));
     CHECK(reader.reads==64); // two whole immutable-owner reads per group
     for(unsigned group=0;group<32;++group){
      auto& owner=*reinterpret_cast<cfggcretain::Owner*>(pool.Address(group));
      const auto saved=owner.slots.back();owner.slots.back()=0;
      Reader<32> cleared{pool};CHECK(!pool.Current(cleared));owner.slots.back()=saved; // fixture only
     }
     Reader<32> restored{pool};CHECK(pool.Current(restored)&&restored.reads==64);
     CHECK(!pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x900000},report));
     CHECK(pool.Poisoned()&&pool.Count()==32&&registry.adds==32);
     std::printf("Full immutable root pool: 1024 references, 32 owners, 64 reads/check; capacity overflow refuses without publication\n");}
    std::printf("Permanent staged root pool: %u failures; synthetic registry/collector only\n",failures);
    return failures?1:0;
}
