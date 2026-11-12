#include "GcRegistryProbe.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(c) do{if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(false)
struct Fake {
    cfggcretain::ReferencerList list{0x10000,2,16};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{0x20000,0x30000};
    unsigned snapshots=0,adds=0,removes=0,failSnapshot=0;
    bool failAdd=false,failRemove=false,damage=false,relocate=false;
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{
        ++snapshots;if(snapshots==failSnapshot)return false;out=list;values=entries;return true;
    }
    bool Register(std::uintptr_t owner)noexcept{
        ++adds;if(failAdd)return false;entries[list.count++]=owner;
        if(relocate)list.data+=0x1000;return true;
    }
    bool Unregister(std::uintptr_t)noexcept{
        ++removes;if(failRemove)return false;entries[--list.count]=0;
        if(damage)entries[0]+=8;return true;
    }
};
struct OwnerReader {
    cfggcretain::Owner& owner;
    unsigned reads=0,failAt=0,clearAt=0;
    template<class T>bool Read(std::uintptr_t address,T& out)noexcept {
        ++reads;if(reads==failAt)return false;
        if(reads==clearAt)owner.slots[0]=0; // emulate a delayed collector clear
        const auto base=reinterpret_cast<std::uintptr_t>(&owner);
        if(address<base||address-base>sizeof(owner)||sizeof(T)>sizeof(owner)-(address-base))return false;
        std::memcpy(&out,reinterpret_cast<const void*>(address),sizeof(T));return true;
    }
};
}
int main(){
    using namespace cfggcprobe;
    CHECK(Candidate(true,false,true,true,true,false));
    CHECK(!Candidate(false,false,true,true,true,false));
    CHECK(!Candidate(true,true,true,true,true,false));
    CHECK(!Candidate(true,false,false,true,true,false));
    CHECK(!Candidate(true,false,true,false,true,false));
    CHECK(!Candidate(true,false,true,true,false,false));
    CHECK(!Candidate(true,false,true,true,true,true));
    {Fake a;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);
     CHECK(r.result==Result::Passed&&r.calls==2&&r.added&&r.removed&&a.adds==1&&a.removes==1);
     CHECK(o.state.load()==cfggcretain::State::Removed&&r.callbacks==0);}
    {Fake a;a.list.capacity=2;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Refused&&!r.calls);}
    {Fake a;a.list.count=MaximumOwners;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(!r.calls);}
    {Fake a;cfggcretain::Owner o;a.entries[0]=reinterpret_cast<std::uintptr_t>(&o);Report r;RoundTripLocked(a,o,r);CHECK(!r.calls);}
    {Fake a;a.failAdd=true;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Indeterminate&&r.calls==1&&!a.removes);}
    {Fake a;a.failSnapshot=2;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);
     CHECK(r.result==Result::Indeterminate&&!r.added&&r.removed&&a.removes==1);}
    {Fake a;a.failRemove=true;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Indeterminate&&r.calls==2&&!r.removed);}
    {Fake a;a.damage=true;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Indeterminate&&!r.removed);}
    {Fake a;a.relocate=true;cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Indeterminate&&!r.added);}
    {Fake a;cfggcretain::Owner o;o.slots[0]=0x10000;Report r;RoundTripLocked(a,o,r);CHECK(!r.calls);}
    {Fake a;a.list.count=MaximumOwners-1;a.list.capacity=MaximumOwners;
     cfggcretain::Owner o;Report r;RoundTripLocked(a,o,r);CHECK(r.result==Result::Passed&&r.calls==2);}
    {Fake a;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     CHECK(r.result==Result::Passed&&r.calls==1&&r.added==1&&!r.removed&&r.permanent&&r.references==1&&r.pawn==0x40000);
     CHECK(o.state.load()==cfggcretain::State::Registered&&o.slots[0]==0x40000&&a.entries[a.list.count-1]==reinterpret_cast<std::uintptr_t>(&o));}
    {Fake a;a.list.capacity=2;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     CHECK(r.result==Result::Refused&&!r.calls&&o.state.load()==cfggcretain::State::Empty);}
    {Fake a;a.failSnapshot=2;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     CHECK(r.result==Result::Indeterminate&&r.calls==1&&!r.removed&&!r.permanent);
     CHECK(o.state.load()==cfggcretain::State::Abandoned&&o.slots[0]==0x40000&&a.list.count==3&&a.removes==0);}
    {Fake a;a.failAdd=true;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     CHECK(r.result==Result::Indeterminate&&r.calls==1&&!a.removes&&o.state.load()==cfggcretain::State::Abandoned);}
    {Fake a;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0);
     CHECK(r.result==Result::Refused&&!r.calls&&o.state.load()==cfggcretain::State::Empty);}
    {Fake a;cfggcretain::Owner o;Report r;
     RegisterPermanentInputsLocked(a,o,r,std::array<std::uintptr_t,4>{0x40000,0x50000,0x40000,0x60000});
     CHECK(r.result==Result::Passed&&r.calls==1&&r.references==3&&r.permanent&&a.removes==0);
     o.CollectWith([](std::uintptr_t*)noexcept {});
     CHECK(o.callbacks.load()==1&&o.completedCallbacks.load()==1&&o.submissions.load()==3&&o.SlotsUnchanged());
     OwnerReader reader{o};OwnerObservation observed;OwnerObservationFailure failure;
     CHECK(CaptureOwnerObservation(reader,reinterpret_cast<std::uintptr_t>(&o),observed,failure)&&observed.InputsPreserved(3,0x40000));
     CHECK(!observed.InputsPreserved(4,0x40000)&&!observed.InputsPreserved(3,0x50000));}
    {Fake a;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     const auto address=reinterpret_cast<std::uintptr_t>(&o);
     OwnerObservation observed;OwnerObservationFailure failure;OwnerReader reader{o};
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&reader.reads==2);
     CHECK(failure==OwnerObservationFailure::None&&observed.RootPreserved(r.pawn)&&!observed.CollectionObserved(r.pawn));
     o.CollectWith([&](std::uintptr_t* slot)noexcept {CHECK(slot==&o.slots[0]&&*slot==r.pawn);});
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&observed.CollectionObserved(r.pawn));
     CHECK(!observed.CollectionObserved(0x50000));
     // A callback can finish before a deferred native write clears its slot.
     // Counters remain successful: the actual slot read must reject retention.
     o.slots[0]=0;
     CHECK(!o.clearedSlots.load()&&o.callbacks.load()==o.completedCallbacks.load());
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&!observed.RootPreserved(r.pawn)&&!observed.CollectionObserved(r.pawn));}
    {Fake a;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     const auto address=reinterpret_cast<std::uintptr_t>(&o);
     o.callbacks.store(1);o.submissions.store(1); // callback has not returned
     OwnerObservation observed;OwnerObservationFailure failure;OwnerReader reader{o};
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&!observed.CollectionObserved(r.pawn));
     o.completedCallbacks.store(1);o.destroyRequests.store(1);
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&!observed.CollectionObserved(r.pawn));
     o.destroyRequests.store(0);o.state.store(cfggcretain::State::Abandoned);
     CHECK(CaptureOwnerObservation(reader,address,observed,failure)&&!observed.CollectionObserved(r.pawn));}
    {Fake a;cfggcretain::Owner o;Report r;RegisterPermanentPawnLocked(a,o,r,0x40000);
     const auto address=reinterpret_cast<std::uintptr_t>(&o);
     OwnerObservation observed;OwnerObservationFailure failure;OwnerReader reader{o};
     reader.failAt=1;
     CHECK(!CaptureOwnerObservation(reader,address,observed,failure)&&failure==OwnerObservationFailure::Read);
     OwnerReader changed{o};changed.clearAt=2;
     CHECK(!CaptureOwnerObservation(changed,address,observed,failure)&&failure==OwnerObservationFailure::Changed);
     CHECK(!CaptureOwnerObservation(changed,0,observed,failure)&&failure==OwnerObservationFailure::InvalidInput);}
    std::printf("GC registry/permanent root/actual-slot and callback-completion cases: %u failures; synthetic only\n",failures);
    return failures?1:0;
}
