#include "RootSlotLedger.hpp"
#include <cstdio>
namespace {unsigned failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(false)
}
int main(){using Ledger=cfgretain::RootSlotLedger<4>;
    std::array<std::uintptr_t,4> slots{};Ledger ledger(slots);
    Ledger::Reservation token;std::array<Ledger::Handle,1> first{},next{};std::uintptr_t value=0;
    CHECK(!ledger.Reserve(0,token)&&!token.id);
    CHECK(ledger.Reserve(1,token)&&ledger.Commit(token,std::array<std::uintptr_t,1>{0x10000},first));
    CHECK(ledger.Resolve(first[0],value)&&value==0x10000&&ledger.Live()==1);
    {std::array<std::uintptr_t,4> foreignSlots{};Ledger foreign(foreignSlots);Ledger::Reservation foreignToken;
     std::array<Ledger::Handle,1> foreignHandles{};
     CHECK(foreign.Reserve(1,foreignToken)&&!ledger.Cancel(foreignToken));
     CHECK(!ledger.Commit(foreignToken,std::array<std::uintptr_t,1>{0x30000},foreignHandles));
     CHECK(foreign.Commit(foreignToken,std::array<std::uintptr_t,1>{0x10000},foreignHandles));
     CHECK(!ledger.Resolve(foreignHandles[0],value)&&!foreign.Resolve(first[0],value));}
    slots[first[0].slot]=0;CHECK(!ledger.Resolve(first[0],value)&&!value&&!ledger.Live());
    CHECK(ledger.Reserve(1,token)&&ledger.Commit(token,std::array<std::uintptr_t,1>{0x10000},next));
    CHECK(!(first[0]==next[0])&&!ledger.Resolve(first[0],value));
    CHECK(ledger.Resolve(next[0],value)&&value==0x10000);
    CHECK(ledger.Release(next[0])&&!ledger.Release(next[0]));
    Ledger::Handle stale{};
    for(unsigned i=0;i<10000;++i){
        CHECK(ledger.Reserve(1,token));
        CHECK(ledger.Commit(token,std::array<std::uintptr_t,1>{0x20000+8u*i},next));
        CHECK(!ledger.Resolve(stale,value)&&ledger.Resolve(next[0],value)&&value==0x20000+8u*i);
        stale=next[0];CHECK(ledger.Release(next[0])&&!ledger.Live());
    }
    {Ledger::Reservation full;CHECK(ledger.Reserve(4,full));
     CHECK(!ledger.Reserve(1,token)&&!token.id&&!ledger.Live());
     auto malformed=full;malformed.count=3;CHECK(!ledger.Cancel(malformed));
     CHECK(!ledger.Reserve(1,token)); // malformed cancellation cannot free another live reservation
     auto copied=full;CHECK(ledger.Cancel(full)&&!ledger.Cancel(copied));}
    {std::array<Ledger::Handle,2> handles{};
     CHECK(ledger.Reserve(2,token));const auto before=slots;
     CHECK(!ledger.Commit(token,std::array<std::uintptr_t,2>{0x40000,0x40001},handles)&&slots==before);
     CHECK(ledger.Cancel(token));}
    {std::array<Ledger::Handle,1> a{},b{};
     CHECK(ledger.Reserve(1,token)&&ledger.Commit(token,std::array<std::uintptr_t,1>{0x45000},a));
     CHECK(ledger.Reserve(1,token)&&ledger.Commit(token,std::array<std::uintptr_t,1>{0x45000},b));
     CHECK(!(a[0]==b[0])&&ledger.Release(a[0])&&!ledger.Resolve(a[0],value));
     CHECK(ledger.Resolve(b[0],value)&&value==0x45000&&ledger.Release(b[0]));}
    {std::array<Ledger::Handle,3> handles{};
     CHECK(ledger.Reserve(3,token)&&ledger.Commit(token,std::array<std::uintptr_t,3>{0x50000,0x50000,0},handles));
     CHECK(ledger.Live()==1&&handles[0]==handles[1]&&!ledger.Resolve(handles[2],value));
     slots[handles[0].slot]=0;CHECK(!ledger.Resolve(handles[1],value)&&!ledger.Live());}
    {std::array<Ledger::Handle,2> handles{};
     CHECK(ledger.Reserve(2,token)&&ledger.Commit(token,std::array<std::uintptr_t,2>{0x58000,0x59000},handles));
     slots[handles[0].slot]=0;
     CHECK(ledger.ReleaseAcquired(handles[0])&&!ledger.ReleaseAcquired(handles[0]));
     CHECK(ledger.Release(handles[1])&&!ledger.ReleaseAcquired(handles[1]));}
    {std::array<Ledger::Handle,2> handles{};
     CHECK(ledger.Reserve(2,token)&&ledger.Commit(token,std::array<std::uintptr_t,2>{0x60000,0x70000},handles));
     slots[handles[0].slot]=0;
     CHECK(!ledger.Resolve(handles[0],value)&&ledger.Resolve(handles[1],value)&&value==0x70000);
     slots[handles[1].slot]=0x80000;
     CHECK(!ledger.Resolve(handles[1],value)&&ledger.Poisoned());}
    std::printf("Root slot generations/retirement/address-reuse/10k-singletons/output-reservation/partial-refusal: %u failures; OWNED slots only, not engine approval\n",failures);
    return failures?1:0;
}
