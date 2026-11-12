#include "ResidentRootBank.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Registry {
    cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
    unsigned calls=0;bool current=true;
    bool ProvenSlotWindow()noexcept{return current;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t address)noexcept{++calls;entries[static_cast<unsigned>(list.count++)]=address;return true;}
    void Collect(bool clear)noexcept{
        for(int i=0;i<list.count;++i){auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(entries[static_cast<unsigned>(i)]);
            owner.CollectWith([&](std::uintptr_t* slot)noexcept{if(clear)*slot=0;},true);}
    }
};
}
int main(){using Bank=cfgretain::ResidentRootBank<2>;Bank bank;Registry registry;
    {Bank observed;Registry own;Bank::Reservation reservation;std::array<Bank::Handle,2> handles{};
     CHECK(observed.ReserveOutputs(own,0x100000,2,reservation));
     CHECK(observed.CommitFreshReturns(own,reservation,std::array<std::uintptr_t,2>{0x20000,0x30000},handles));
     const auto nativeCalls=own.calls;std::array<std::uintptr_t,64> values{};unsigned count=0;
     CHECK(observed.ObserveSlots(values,count)&&count==32&&values[0]==0x20000&&values[1]==0x30000&&own.calls==nativeCalls);
     CHECK(observed.ObserveSlots(values,count)&&values[0]==0x20000&&values[1]==0x30000&&own.calls==nativeCalls);
     own.Collect(true);CHECK(observed.ObserveSlots(values,count)&&count==32&&!values[0]&&!values[1]&&own.calls==nativeCalls);
     CHECK(observed.ReleaseAcquiredMany(own,handles,2));}
    Bank::Handle old{};std::uintptr_t object=0;
    for(unsigned i=0;i<10000;++i){Bank::Reservation reserve;std::array<Bank::Handle,1> returns{};
        CHECK(bank.ReserveOutputs(registry,0x100000,1,reserve));
        CHECK(bank.CommitFreshReturns(registry,reserve,std::array<std::uintptr_t,1>{0x20000+8u*i},returns));
        CHECK(bank.ResolveRequired(registry,returns[0],object)&&object==0x20000+8u*i);
        CHECK(!bank.ResolveRequired(registry,old,object));old=returns[0];
        registry.Collect(true);CHECK(!bank.ResolveRequired(registry,old,object));
    }
    CHECK(bank.Owners()==1&&registry.calls==1&&registry.list.count==1&&bank.Healthy());
    {Bank::Reservation first,second,overflow;CHECK(bank.ReserveOutputs(registry,0x100000,32,first));
     CHECK(bank.ReserveOutputs(registry,0x100000,32,second));const auto calls=registry.calls;
     CHECK(!bank.ReserveOutputs(registry,0x100000,1,overflow)&&registry.calls==calls&&!overflow.slots.id);
     CHECK(bank.CancelUnpublished(registry,first)&&bank.CancelUnpublished(registry,second));}
    {Bank other;Bank::Reservation foreign;std::array<Bank::Handle,1> handles{};
     CHECK(bank.ReserveOutputs(registry,0x100000,1,foreign));
     CHECK(!other.CommitFreshReturns(registry,foreign,std::array<std::uintptr_t,1>{0x30000},handles));
     CHECK(bank.CancelUnpublished(registry,foreign));}
    registry.current=false;Bank::Reservation denied;const auto calls=registry.calls;
    CHECK(!bank.ReserveOutputs(registry,0x100000,1,denied)&&registry.calls==calls);
    CHECK(!bank.ResolveRequired(registry,old,object));registry.current=true;
    {Bank::Reservation reserve;CHECK(!bank.ReserveOutputs(registry,0x200000,1,reserve));}
    {auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(registry.entries[0]);
     const auto callbacks=owner.native.callbacks.load();owner.native.callbacks.store(callbacks+1);
     CHECK(!bank.Healthy());Bank::Reservation reserve;
     CHECK(!bank.ReserveOutputs(registry,0x100000,1,reserve));
     owner.native.completedCallbacks.store(callbacks+1);CHECK(bank.Healthy());}
    {auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(registry.entries[0]);
     owner.CollectWith([](std::uintptr_t*)noexcept{},false);CHECK(!bank.Healthy());
     Bank::Reservation reserve;CHECK(!bank.ReserveOutputs(registry,0x100000,1,reserve));}
    std::printf("Fixed resident root bank: %u failures; 10k native-return simulations/one owner/finite output reservations, NO game calls\n",failures);
    return failures?1:0;
}
