#include "GcReferenceOwner.hpp"
#include <cstdio>

namespace {
unsigned failures=0,calls=0;
cfggcretain::Owner* activeOwner=nullptr;
#define CHECK(c) do{if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(false)
void __fastcall SubmitStub(void*,std::uintptr_t* slot,void* owner,void* property){
    CHECK(!owner&&!property);++calls;
    // The native registry can reach the owner before its registering call
    // publishes the successful membership result. Keep the callback usable in
    // that reentrant window, and mutate the exact caller-owned slot address.
    CHECK(activeOwner&&activeOwner->state.load()==cfggcretain::State::Registering);
    if(*slot==0x20000)*slot=0;
}
}
int main(){
    using namespace cfggcretain;
    Owner owner;
    CHECK(!owner.Prepare(std::array<std::uintptr_t,1>{0}));
    CHECK(!owner.Prepare(std::array<std::uintptr_t,2>{0x10000,0x10001})&&!owner.slots[0]);
    CHECK(owner.Prepare(std::array<std::uintptr_t,4>{0x10000,0x20000,0x10000,0}));
    CHECK(!owner.Prepare(std::array<std::uintptr_t,1>{0x30000}));
    CHECK(owner.slots[0]==0x10000&&owner.slots[1]==0x20000&&!owner.slots[2]);
    CHECK(owner.BeginRegistration()&&!owner.BeginRegistration());
    // A native collector may reach the owner before registration returns.
    activeOwner=&owner;
    std::array<std::uintptr_t,8> table{};table[7]=reinterpret_cast<std::uintptr_t>(&SubmitStub);
    auto* pointer=table.data();owner.vtable->collect(&owner,&pointer);
    CHECK(calls==2&&owner.callbacks.load()==1&&owner.completedCallbacks.load()==1&&owner.submissions.load()==2&&owner.clearedSlots.load()==1);
    CHECK(!owner.slots[1]&&!owner.SlotsUnchanged()); // nulled pointer stays null
    // A later collection receives only still-live slots; it cannot resurrect
    // the cleared input from the diagnostic expected-value copy.
    owner.vtable->collect(&owner,&pointer);
    CHECK(calls==3&&owner.callbacks.load()==2&&owner.completedCallbacks.load()==2&&owner.submissions.load()==3&&owner.clearedSlots.load()==1);
    CHECK(!owner.slots[1]&&owner.slots[0]==0x10000);
    activeOwner=nullptr;
    CHECK(owner.Registered(true)&&!owner.Registered(true));
    CHECK(owner.BeginRemoval()&&owner.Removed(true));
    CHECK(owner.state.load()==State::Removed&&!owner.BeginRegistration());
    CHECK(owner.vtable->destroy(&owner,1)==&owner&&owner.destroyRequests.load()==1);
    CHECK(owner.state.load()==State::Abandoned&&owner.slots[0]==0x10000);
    Owner ambiguous;CHECK(ambiguous.Prepare(std::array<std::uintptr_t,1>{0x10000}));
    CHECK(ambiguous.BeginRegistration()&&!ambiguous.Registered(false));
    CHECK(ambiguous.state.load()==State::Abandoned&&!ambiguous.BeginRegistration());
    Owner cleanup;CHECK(cleanup.Prepare(std::array<std::uintptr_t,1>{0x10000}));
    CHECK(cleanup.BeginRegistration()&&cleanup.Registered(true)&&cleanup.BeginRemoval());
    CHECK(!cleanup.Removed(false)&&cleanup.state.load()==State::Abandoned);
    CHECK(cleanup.slots[0]==0x10000); // uncertain cleanup retains resident slots
    Owner overflow;std::array<std::uintptr_t,Capacity+1> many{};
    for(unsigned i=0;i<many.size();++i)many[i]=0x10000+8u*i;
    CHECK(!overflow.Prepare(many)&&overflow.state.load()==State::Empty&&!overflow.slots[0]);
    std::printf("GC reference-owner ABI/mutable slots/one-shot cleanup: %u failures; synthetic collector only\n",failures);
    return failures?1:0;
}
