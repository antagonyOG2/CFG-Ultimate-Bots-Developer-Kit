#include "SynchronizedRootOwner.hpp"
#include "RootSlotLedger.hpp"
#include <thread>
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
struct Window {
    bool current=true,revoke=false;unsigned checks=0;
    bool ProvenSlotWindow()noexcept{return ++checks==2&&revoke?false:current;}
};
void Prepare(cfgretain::SynchronizedRootOwner& owner){
    CHECK(owner.native.Prepare(std::array<std::uintptr_t,2>{0x10000,0x20000}));
    CHECK(owner.native.BeginRegistration()&&owner.native.Registered(true));
}
bool FaultCollector(cfgretain::SynchronizedRootOwner& owner)noexcept{
    __try{owner.CollectWith([](std::uintptr_t*)noexcept{RaiseException(0xE0473502,0,0,nullptr);},true);}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    return true;
}
}
int main(){
    using cfgretain::SynchronizedRootOwner;
    {SynchronizedRootOwner owner;Window window;unsigned writes=0;
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
     Prepare(owner);window.current=false;
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
     window={};window.revoke=true;
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
     window={};CHECK(owner.TryPublication(window,[&](cfggcretain::Owner& native)noexcept{
        ++writes;native.slots[1]=0x30000;return true;})&&writes==1);
     owner.CollectWith([](std::uintptr_t* slot)noexcept{if(*slot==0x30000)*slot=0;},true);
     CHECK(!owner.native.slots[1]&&owner.native.slots[0]==0x10000&&owner.native.clearedSlots.load()==1);
     CHECK(owner.TryPublication(window,[&](cfggcretain::Owner& native)noexcept{
        ++writes;native.slots[1]=0x40000;return true;})&&writes==2);
    }
    {SynchronizedRootOwner owner;Prepare(owner);Window window;
     std::atomic<bool> entered{false},release{false};
     std::thread collector([&](){owner.CollectWith([&](std::uintptr_t*)noexcept{
        entered.store(true,std::memory_order_release);
        while(!release.load(std::memory_order_acquire))std::this_thread::yield();},true);});
     while(!entered.load(std::memory_order_acquire))std::this_thread::yield();
     unsigned writes=0;CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
     release.store(true,std::memory_order_release);collector.join();
     CHECK(owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&writes==1);
    }
    {SynchronizedRootOwner owner;Prepare(owner);Window window;std::uintptr_t* delayed=nullptr;
     owner.CollectWith([&](std::uintptr_t* slot)noexcept{if(*slot==0x20000)delayed=slot;},false);
     CHECK(delayed&&owner.unknownSubmit.load()==1);unsigned writes=0;
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
     *delayed=0; // An UNKNOWN native handler retains the original slot address.
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes&&!owner.native.slots[1]);
     owner.CollectWith([](std::uintptr_t*)noexcept{},true);
     CHECK(!owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{++writes;return true;})&&!writes);
    }
    {SynchronizedRootOwner owner;Prepare(owner);Window window;
     owner.native.callbacks.store(1);CHECK(!owner.TryPublication(window,[](cfggcretain::Owner&)noexcept{return true;}));
     owner.native.completedCallbacks.store(1);owner.native.vtable->destroy(&owner.native,0);
     CHECK(!owner.TryPublication(window,[](cfggcretain::Owner&)noexcept{return true;}));
     CHECK(!owner.KnownSubmit(0x7600F0)&&!owner.KnownSubmit(0x7607F0));
    }
    {SynchronizedRootOwner owner;Window window;
     CHECK(owner.native.PrepareEmptyRegistryProbe()&&owner.native.BeginRegistration()&&owner.native.Registered(true));
     using Ledger=cfgretain::RootSlotLedger<32>;Ledger ledger(owner.native.slots);
     Ledger::Handle previous{};std::uintptr_t value=0;
     for(unsigned i=0;i<10000;++i){
        CHECK(owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{
            Ledger::Reservation reservation;std::array<Ledger::Handle,1> result{};
            if(!ledger.Reserve(1,reservation)||!ledger.Commit(reservation,std::array<std::uintptr_t,1>{0x30000+8u*i},result))return false;
            if(ledger.Resolve(previous,value)||!ledger.Resolve(result[0],value)||value!=0x30000+8u*i)return false;
            previous=result[0];return true;
        }));
        owner.CollectWith([](std::uintptr_t* slot)noexcept{*slot=0;},true);
        CHECK(owner.TryPublication(window,[&](cfggcretain::Owner&)noexcept{return !ledger.Resolve(previous,value)&&!ledger.Live();}));
     }
     CHECK(owner.native.callbacks.load()==10000&&owner.native.completedCallbacks.load()==10000&&
        owner.native.submissions.load()==10000&&owner.native.clearedSlots.load()==10000&&!owner.unknownSubmit.load());
    }
    {SynchronizedRootOwner owner;Prepare(owner);Window window;CHECK(!FaultCollector(owner));
     CHECK(owner.native.callbacks.load()==1&&!owner.native.completedCallbacks.load());
     const bool unlocked=TryAcquireSRWLockExclusive(&owner.gate)!=0;
     CHECK(unlocked);if(unlocked)ReleaseSRWLockExclusive(&owner.gate);
     CHECK(!owner.TryPublication(window,[](cfggcretain::Owner&)noexcept{return true;}));}
    std::printf("Synchronized owner callback/publication/late-unknown-clear/partial/destroyed tests: %u failures; synthetic callbacks, NO engine window or runtime approval\n",failures);
    return failures?1:0;
}
