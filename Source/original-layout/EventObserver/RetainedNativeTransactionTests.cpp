#include "RetainedNativeTransaction.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Registry {
    cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
    bool current=true;
    bool ProvenSlotWindow()noexcept{return current;}
    bool NativeEntryUnlocked()const noexcept{return !current;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t address)noexcept{entries[static_cast<unsigned>(list.count++)]=address;return true;}
    void ClearInputs()noexcept{
        for(int i=0;i<list.count;++i){auto& owner=*reinterpret_cast<cfgretain::SynchronizedRootOwner*>(entries[static_cast<unsigned>(i)]);
            owner.CollectWith([](std::uintptr_t* slot)noexcept{if(*slot==0x20000)*slot=0;},true);}
    }
};
}
int main(){using Bank=cfgretain::ResidentRootBank<2>;using Call=cfgretain::RetainedNativeTransaction<Bank>;
    {Bank bank;Registry registry;unsigned nativeCalls=0;
     for(unsigned i=0;i<1000;++i){Call call(bank);
        CHECK(call.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},1));
        CHECK(!call.EnteredNative(registry)); // no allocating call under publication locks
        registry.current=false;CHECK(call.EnteredNative(registry));++nativeCalls;registry.current=true;
        CHECK(call.CommitReturned(registry,std::array<std::uintptr_t,1>{0x30000+8u*i}));
        CHECK(!call.EnteredNative(registry)&&call.ReleaseCompleted(registry)&&call.CurrentState()==Call::State::Complete);
     }
     CHECK(nativeCalls==1000&&bank.Owners()==1&&registry.list.count==1);}
    {Bank bank;Registry registry;Bank::Reservation full;
     CHECK(bank.ReserveOutputs(registry,0x100000,32,full));
     Call call(bank);unsigned nativeCalls=0;
     // Inputs consume the remaining owner; outputs have no reserved slot.
     std::array<std::uintptr_t,32> fullInputs{};for(unsigned i=0;i<32;++i)fullInputs[i]=0x20000+8u*i;
     const bool prepared=call.Prepare(registry,0x100000,fullInputs,1);
     registry.current=false;if(prepared&&call.EnteredNative(registry))++nativeCalls;registry.current=true;
     CHECK(!nativeCalls&&call.CurrentState()==Call::State::Refused&&!call.EnteredNative(registry));
     CHECK(bank.CancelUnpublished(registry,full));}
    for(unsigned fault=0;fault<3;++fault){Bank bank;Registry registry;Call call(bank);unsigned nativeCalls=0;
     CHECK(call.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},1));
     registry.current=false;CHECK(call.EnteredNative(registry));++nativeCalls;registry.current=true;
     if(fault==0)registry.current=false;
     if(fault==1)registry.ClearInputs();
     if(fault==2)call.NativeFailed();
     CHECK(!call.CommitReturned(registry,std::array<std::uintptr_t,1>{0x30000}));
     CHECK(call.CurrentState()==Call::State::Indeterminate&&!call.EnteredNative(registry)&&nativeCalls==1);
     registry.current=true;Call retry(bank);
     CHECK(bank.Poisoned()&&!retry.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},1));
    }
    {Bank bank;Registry registry;Call canceled(bank);
     CHECK(canceled.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},1));
     CHECK(canceled.CancelBeforeNative(registry)&&!canceled.EnteredNative(registry));
     Call empty(bank);CHECK(empty.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},0));
     registry.current=false;CHECK(empty.EnteredNative(registry));registry.current=true;
     CHECK(empty.CommitReturned(registry,std::array<std::uintptr_t,0>{})&&empty.ReleaseCompleted(registry));}
    {Bank bank;Registry registry;Call duplicate(bank);
     CHECK(duplicate.Prepare(registry,0x100000,std::array<std::uintptr_t,2>{0x20000,0x20000},2));
     registry.current=false;CHECK(duplicate.EnteredNative(registry));registry.current=true;
     CHECK(duplicate.CommitReturned(registry,std::array<std::uintptr_t,2>{0x20000,0x20000})&&duplicate.ReleaseCompleted(registry));}
    {Bank bank;Registry registry;Call create(bank);
     CHECK(create.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x20000},1));
     registry.current=false;CHECK(create.EnteredNative(registry));registry.current=true;
     CHECK(create.CommitReturned(registry,std::array<std::uintptr_t,1>{0x40000}));
     std::array<Bank::Handle,32> kept{};unsigned count=0;std::uintptr_t object=0;
     CHECK(create.TakeReturnedOutputs(registry,kept,count)&&count==1&&bank.ResolveRequired(registry,kept[0],object)&&object==0x40000);
     Call attach(bank);CHECK(attach.Prepare(registry,0x100000,std::array<std::uintptr_t,1>{0x40000},0));
     registry.current=false;CHECK(attach.EnteredNative(registry));registry.current=true;
     CHECK(attach.CommitReturned(registry,std::array<std::uintptr_t,0>{})&&attach.ReleaseCompleted(registry));
     CHECK(bank.ResolveRequired(registry,kept[0],object)&&object==0x40000&&bank.Release(registry,kept[0]));}
    {Bank bank;Registry registry;Call bounded(bank);std::array<std::uintptr_t,32> packetRoots{0x20000};
     CHECK(bounded.Prepare(registry,0x100000,packetRoots,1,1));
     registry.current=false;CHECK(bounded.EnteredNative(registry));registry.current=true;
     packetRoots={0x30000};CHECK(bounded.CommitReturned(registry,packetRoots,1)&&bounded.ReleaseCompleted(registry));
     CHECK(bank.Owners()==1);}
    std::printf("Retained native transaction: %u failures; pre-call output capacity, returned roots, cancellation and one-call/no-replay; modeled dispatch ONLY\n",failures);
    return failures?1:0;
}
