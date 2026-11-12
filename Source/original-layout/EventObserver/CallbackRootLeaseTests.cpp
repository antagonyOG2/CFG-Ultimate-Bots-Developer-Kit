#include "CallbackRootLease.hpp"
#include <cstdio>
#include <type_traits>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Registry {
    cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};bool held=true;
    bool ProvenSlotWindow()noexcept{return held;}
    bool NativeEntryUnlocked()const noexcept{return !held;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t address)noexcept{entries[static_cast<unsigned>(list.count++)]=address;return true;}
    void Collect(std::uintptr_t clear=0,bool known=true){
        for(int i=0;i<list.count;++i){auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(entries[static_cast<unsigned>(i)]);
            owner.CollectWith([&](std::uintptr_t* slot)noexcept{if(*slot==clear)*slot=0;},known);}
    }
};
}
int main(){using Bank=cfgretain::ResidentRootBank<2>;using Lease=cfgretain::CallbackRootLease<Bank,64>;
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));
     CHECK(lease.PrepareCall(registry,1));registry.held=false;CHECK(lease.EnterNative(registry));
     CHECK(!lease.CompleteReturnedWithoutOutputs()&&lease.NativeEntered());
     CHECK(!lease.CompleteCall(registry,std::array<std::uintptr_t,1>{0x30000})&&bank.Poisoned());}
    using Deferred=Lease::DeferredRetirement;
    static_assert(!std::is_copy_constructible_v<Deferred>&&!std::is_copy_assignable_v<Deferred>);
    {Bank bank;Registry registry;Lease empty(bank,0x100000);Deferred pending;
     CHECK(!empty.TakeIdleClaims(pending)&&!pending.Claims());
     CHECK(empty.Retain(registry,std::array<std::uintptr_t,2>{0x20000,0x30000}));
     auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(registry.entries[0]);const auto slots=owner.native.slots;
     registry.held=false;CHECK(empty.TakeIdleClaims(pending)&&pending.Claims()==2&&!empty.Current()&&!empty.Claims());
     CHECK(owner.native.slots==slots&&bank.Healthy());
     CHECK(!pending.Retire(registry,bank,0x100000)&&pending.Claims()==2&&bank.Healthy());
     registry.held=true;Bank wrong;CHECK(!pending.Retire(registry,wrong,0x100000)&&!pending.Retire(registry,bank,0x110000));
     CHECK(pending.Retire(registry,bank,0x100000)&&!pending.Claims()&&bank.Healthy());
     CHECK(!pending.Retire(registry,bank,0x100000));}
    {Bank bank;Registry registry;Lease source(bank,0x100000),other(bank,0x100000);Deferred pending;
     CHECK(source.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&source.TakeIdleClaims(pending));
     CHECK(other.Retain(registry,std::array<std::uintptr_t,1>{0x30000})&&!other.TakeIdleClaims(pending)&&other.Current());
     CHECK(other.Close(registry)&&pending.Retire(registry,bank,0x100000)&&bank.Healthy());}
    {Bank bank;Registry registry;Lease source(bank,0x100000);Deferred pending;
     CHECK(source.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&source.PrepareCall(registry,0));
     CHECK(!source.TakeIdleClaims(pending));CHECK(source.CancelCall(registry)&&source.TakeIdleClaims(pending));
     registry.Collect(0x20000);CHECK(pending.Retire(registry,bank,0x100000)&&bank.Healthy());}
    {Bank bank;Registry registry;Lease source(bank,0x100000);Deferred pending;
     CHECK(source.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&source.PrepareCall(registry,1));
     CHECK(!source.TakeIdleClaims(pending));registry.held=false;CHECK(source.EnterNative(registry)&&!source.TakeIdleClaims(pending));
     registry.held=true;CHECK(!source.Close(registry)&&bank.Poisoned()&&!source.TakeIdleClaims(pending));}
    {Bank bank;Registry registry;Lease source(bank,0x100000),reuse(bank,0x100000);Deferred pending;
     CHECK(source.Retain(registry,std::array<std::uintptr_t,2>{0x20000,0x30000})&&source.TakeIdleClaims(pending));
     registry.Collect(0x30000);CHECK(reuse.Retain(registry,std::array<std::uintptr_t,1>{0x40000}));
     CHECK(!pending.Retire(registry,bank,0x100000)&&bank.Poisoned()&&pending.Claims()==2);
     CHECK(!pending.Retire(registry,bank,0x100000)); // partially released record NEVER replays
     auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(registry.entries[0]);CHECK(!owner.native.slots[0]);}
    {Bank bank;Registry registry;
     for(unsigned i=0;i<1000;++i){Lease source(bank,0x100000);Deferred pending;
      CHECK(source.Retain(registry,std::array<std::uintptr_t,2>{0x20000,0x30000})&&source.TakeIdleClaims(pending));
      registry.held=false;for(unsigned wait=0;wait<8;++wait)CHECK(!pending.Retire(registry,bank,0x100000)&&pending.Claims()==2&&bank.Healthy());
      registry.held=true;CHECK(pending.Retire(registry,bank,0x100000)&&bank.Healthy());
     }CHECK(bank.Owners()==1);}
    {Bank bank;Registry registry;Lease source(bank,0x100000),reuse(bank,0x100000);Deferred pending;
     std::array<std::uintptr_t,32> first{},second{};for(unsigned i=0;i<32;++i){first[i]=0x20000+8*i;second[i]=0x30000+8*i;}
     CHECK(source.Retain(registry,first)&&source.Retain(registry,second)&&source.TakeIdleClaims(pending)&&bank.Owners()==2);
     registry.Collect(second[0]);CHECK(reuse.Retain(registry,std::array<std::uintptr_t,1>{0x40000}));
     CHECK(!pending.Retire(registry,bank,0x100000)&&bank.Poisoned()&&pending.Claims()==64);
     auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(registry.entries[0]);
     for(const auto value:owner.native.slots)CHECK(!value);
     CHECK(!pending.Retire(registry,bank,0x100000));}
    {Bank bank;Registry registry;Lease source(bank,0x100000);Deferred pending;
     CHECK(source.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));bank.MarkUncertain();
     CHECK(!source.TakeIdleClaims(pending)&&!pending.Claims()&&bank.Poisoned());}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);registry.held=false;
     CHECK(lease.EmptyIdle()&&lease.CloseEmpty()&&!lease.Current()&&bank.Healthy()&&!bank.Owners()&&!registry.list.count);
     CHECK(!lease.CloseEmpty());}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));
     CHECK(!lease.CloseEmpty()&&lease.Claims()==1&&bank.Healthy());CHECK(lease.Close(registry));}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.PrepareCall(registry,0)&&!lease.CloseEmpty());
     CHECK(lease.CancelCall(registry)&&lease.CloseEmpty()&&bank.Healthy());}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.PrepareCall(registry,1)&&!lease.CloseEmpty());CHECK(lease.CancelCall(registry)&&lease.CloseEmpty());}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);CHECK(lease.PrepareCall(registry,0));
     registry.held=false;CHECK(lease.EnterNative(registry)&&!lease.CloseEmpty());registry.held=true;
     CHECK(!lease.Close(registry)&&bank.Poisoned());}
    {Bank bank;Lease lease(bank,0x100000);bank.MarkUncertain();CHECK(!lease.CloseEmpty()&&!bank.Healthy());}
    {Bank bank;Registry registry;
     for(unsigned i=0;i<10000;++i){Lease lease(bank,0x100000);
        CHECK(lease.Retain(registry,std::array<std::uintptr_t,2>{0x20000,0x20000})&&lease.Claims()==1);
        CHECK(lease.RequiredRange(registry,0,1)&&!lease.RequiredRange(registry,1,1));
        CHECK(lease.PrepareCall(registry,1)&&!lease.PrepareCall(registry,1));
        CHECK(!lease.EnterNative(registry));registry.held=false;CHECK(lease.EnterNative(registry));
        CHECK(!lease.Retain(registry,std::array<std::uintptr_t,1>{0x30000}));
        registry.Collect();registry.held=true;
        CHECK(lease.CompleteCall(registry,std::array<std::uintptr_t,1>{0x30000+8u*i}));
        CHECK(lease.PrepareCall(registry,0));registry.held=false;CHECK(lease.EnterNative(registry));registry.held=true;
        CHECK(lease.CompleteCall(registry,std::array<std::uintptr_t,0>{})&&lease.Close(registry));
        CHECK(!lease.Current()&&!lease.Retain(registry,std::array<std::uintptr_t,1>{0x40000}));
     }
     CHECK(bank.Healthy()&&bank.Owners()==1&&registry.list.count==1);}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));registry.Collect(0x20000);
     CHECK(!lease.RequiredRange(registry,0,1));
     CHECK(!lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));
     CHECK(lease.Close(registry)&&bank.Healthy()); // retire, do not resurrect
     Lease next(bank,0x100000);CHECK(next.Retain(registry,std::array<std::uintptr_t,1>{0x30000})&&next.Close(registry));}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&lease.PrepareCall(registry,2));
     CHECK(lease.CancelCall(registry)&&lease.Close(registry)&&bank.Healthy());}
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000})&&lease.PrepareCall(registry,0));
     registry.held=false;CHECK(lease.EnterNative(registry));registry.held=true;
     CHECK(!lease.Close(registry)&&bank.Poisoned()); // missing post-call proof cannot replay
    }
    {Bank bank;Registry registry;Lease lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));registry.Collect(0,false);
     CHECK(!lease.PrepareCall(registry,0)&&!lease.Close(registry)&&bank.Poisoned());}
    {Bank bank;Registry registry;cfgretain::CallbackRootLease<Bank,1> lease(bank,0x100000);
     CHECK(lease.Retain(registry,std::array<std::uintptr_t,1>{0x20000}));
     CHECK(!lease.PrepareCall(registry,1)&&!lease.EnterNative(registry)&&lease.Close(registry));}
    // H2.92: a claim counts as proven only until the next native entry; unknown values, an entered call and a closed lease never do.
    {Bank bank;Registry registry;Lease lease(bank,0x100000);const std::array<std::uintptr_t,1> a{0x20000},b{0x30000};
     const std::array<std::uintptr_t,2> both{0x20000,0x30000};const std::array<std::uintptr_t,1> none{0};
     CHECK(!lease.ProvenSinceLastNative(a)&&!lease.ProvenSinceLastNative(none));
     CHECK(lease.Retain(registry,a)&&lease.ProvenSinceLastNative(a)&&!lease.ProvenSinceLastNative(b)&&!lease.ProvenSinceLastNative(both));
     CHECK(lease.PrepareCall(registry,0));registry.held=false;CHECK(lease.EnterNative(registry)&&!lease.ProvenSinceLastNative(a));
     CHECK(lease.CompleteReturnedWithoutOutputs()&&!lease.ProvenSinceLastNative(a)); // stale after the call returned
     registry.held=true;CHECK(lease.Retain(registry,both)&&lease.ProvenSinceLastNative(both)); // re-resolved and fresh claim, both proven
     CHECK(lease.Close(registry)&&!lease.ProvenSinceLastNative(a));}
    std::printf("Callback-scoped root claims: %u failures; 10,000 callbacks, bounded owners, output reservation, retirement/no resurrection, no replay; model ONLY\n",failures);
    return failures?1:0;
}
