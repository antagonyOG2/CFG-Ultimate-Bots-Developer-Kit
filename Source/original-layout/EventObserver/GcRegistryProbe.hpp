#pragma once
#include "GcReferenceOwner.hpp"
#include "../RuntimeProbe/GcReferencerObservation.hpp"
#include <cstring>
namespace cfggcprobe {
constexpr unsigned MaximumOwners=512;
constexpr bool Candidate(bool ownerThread,bool nested,bool returned,bool ownedPawn,
    bool nativeTickOrigin,bool published)noexcept{
    return ownerThread&&!nested&&returned&&ownedPawn&&nativeTickOrigin&&!published;
}
enum class Result:std::uint32_t { Waiting,Refused,Passed,Indeterminate };
struct Report {
    std::uint32_t bytes=sizeof(Report),version=5,published=0;
    Result result=Result::Waiting;
    std::uint32_t stage=0,reads=0,calls=0,error=0,added=0,removed=0,callbacks=0;
    std::uint32_t contextFailure=0,gateFailure=0,tickFailure=0,tickReads=0,references=0,permanent=0;
    std::uintptr_t referencer=0,owner=0,data=0,pawn=0;
    std::int32_t count=0,capacity=0;
};
static_assert(std::is_trivially_copyable_v<Report> && sizeof(Report)<4096);

// The callback counters alone cannot detect a collector that clears a slot
// after Submit returns. Observe actual resident slots as well, without writing
// them or assuming that a stable diagnostic sample proves worker quiescence.
struct OwnerObservation {
    std::uintptr_t vtable=0;
    std::array<std::uintptr_t,cfggcretain::Capacity> slots{},expected{};
    unsigned callbacks=0,completed=0,submissions=0,cleared=0,destroyRequests=0;
    cfggcretain::State state=cfggcretain::State::Empty;
    bool operator==(const OwnerObservation& other)const noexcept {
        return vtable==other.vtable&&slots==other.slots&&expected==other.expected&&
            callbacks==other.callbacks&&completed==other.completed&&submissions==other.submissions&&
            cleared==other.cleared&&destroyRequests==other.destroyRequests&&state==other.state;
    }
    bool RootPreserved(std::uintptr_t pawn)const noexcept {
        if(!pawn||slots[0]!=pawn||expected[0]!=pawn)return false;
        for(std::size_t i=1;i<slots.size();++i)if(slots[i]||expected[i])return false;
        return true;
    }
    bool CollectionObserved(std::uintptr_t pawn)const noexcept {
        return RootPreserved(pawn)&&InputsPreserved(1,pawn)&&callbacks>0;
    }
    bool InputsPreserved(unsigned references,std::uintptr_t first)const noexcept {
        unsigned count=0;for(const auto value:expected)count+=value?1u:0u;
        return references>0&&references==count&&first&&expected[0]==first&&slots==expected&&
            state==cfggcretain::State::Registered&&!destroyRequests&&!cleared&&callbacks==completed&&
            submissions==static_cast<std::uint64_t>(completed)*references;
    }
};
enum class OwnerObservationFailure:unsigned {None,InvalidInput,Read,Changed};
template<class Reader>bool ReadOwnerObservation(Reader& reader,std::uintptr_t owner,OwnerObservation& out)noexcept {
    using O=cfggcretain::Owner;
    std::array<unsigned char,sizeof(O)> bytes{};
    if(!reader.Read(owner,bytes))return false;
    // Copy raw observed bytes into plain fields, never copy/assign an Owner
    // containing atomics. Two bounded whole-owner reads also avoid a native
    // call guard scaling to eighteen remote reads per immutable root group.
    std::memcpy(&out.vtable,bytes.data()+offsetof(O,vtable),sizeof(out.vtable));
    std::memcpy(&out.slots,bytes.data()+offsetof(O,slots),sizeof(out.slots));
    std::memcpy(&out.expected,bytes.data()+offsetof(O,expected),sizeof(out.expected));
    std::memcpy(&out.callbacks,bytes.data()+offsetof(O,callbacks),sizeof(out.callbacks));
    std::memcpy(&out.completed,bytes.data()+offsetof(O,completedCallbacks),sizeof(out.completed));
    std::memcpy(&out.submissions,bytes.data()+offsetof(O,submissions),sizeof(out.submissions));
    std::memcpy(&out.cleared,bytes.data()+offsetof(O,clearedSlots),sizeof(out.cleared));
    std::memcpy(&out.destroyRequests,bytes.data()+offsetof(O,destroyRequests),sizeof(out.destroyRequests));
    std::memcpy(&out.state,bytes.data()+offsetof(O,state),sizeof(out.state));return true;
}
template<class Reader>bool CaptureOwnerObservation(Reader& reader,std::uintptr_t owner,OwnerObservation& out,
    OwnerObservationFailure& failure)noexcept {
    out={};failure=OwnerObservationFailure::InvalidInput;
    if(!cfgprobe::ObjectPointer(owner))return false;
    OwnerObservation first{},second{};failure=OwnerObservationFailure::Read;
    if(!ReadOwnerObservation(reader,owner,first)||!ReadOwnerObservation(reader,owner,second))return false;
    failure=OwnerObservationFailure::Changed;
    if(!(first==second))return false;
    out=second;failure=OwnerObservationFailure::None;return true;
}

// Caller holds BOTH native collector/hash and registry-list locks for this
// entire operation. No borrowed UObject references survive an unlock. The spare
// capacity check excludes AddUnique's ONLY allocating branch. This tests the
// register/remove ABI, not GC retention, callback semantics, or actor spawning.
template<class Access>void RoundTripLocked(Access& access,cfggcretain::Owner& owner,Report& report) noexcept {
    cfggcretain::ReferencerList before{},after{};
    std::array<std::uintptr_t,MaximumOwners> entries{},fresh{};
    const auto own=reinterpret_cast<std::uintptr_t>(&owner);
    report.owner=own;report.result=Result::Refused;report.stage=5;
    if(!access.Snapshot(before,entries)||before.count<0||before.count>=MaximumOwners||
       before.capacity<=before.count||!before.data)return;
    report.data=before.data;report.count=before.count;report.capacity=before.capacity;
    for(int i=0;i<before.count;++i)if(entries[i]==own)return;
    if(!owner.PrepareEmptyRegistryProbe()||!owner.BeginRegistration())return;
    report.stage=6;++report.calls;
    if(!access.Register(own)){owner.Registered(false);report.result=Result::Indeterminate;return;}
    // Even if confirmation fails, remove this known empty resident owner once.
    // Never leave a successful native add resident merely because readback failed.
    const bool observed=access.Snapshot(after,fresh)&&after.data==before.data&&
        after.capacity==before.capacity&&after.count==before.count+1&&
        !std::memcmp(entries.data(),fresh.data(),before.count*sizeof(std::uintptr_t))&&
        fresh[before.count]==own;
    report.added=observed?1u:0u;
    owner.Registered(observed);if(observed)owner.BeginRemoval();report.stage=7;++report.calls;
    if(!access.Unregister(own)){owner.Removed(false);report.result=Result::Indeterminate;return;}
    const bool restored=access.Snapshot(after,fresh)&&after==before&&
        !std::memcmp(entries.data(),fresh.data(),before.count*sizeof(std::uintptr_t));
    report.removed=restored?1u:0u;owner.Removed(restored);
    report.callbacks=owner.callbacks.load();
    report.result=observed&&restored&&!report.callbacks?Result::Passed:Result::Indeterminate;
    report.stage=8;
}

// One live callback probe: register a resident owner for an already-live
// counselor pawn and intentionally keep it registered until process exit.
// The slot is prepared before publication and is never changed by this code
// afterward. This exercises the actual owner callback only if normal GC runs;
// it never requests collection or spawns an actor.
template<class Access,std::size_t N>void RegisterPermanentInputsLocked(Access& access,cfggcretain::Owner& owner,
    Report& report,const std::array<std::uintptr_t,N>& inputs) noexcept {
    static_assert(N>0&&N<=cfggcretain::Capacity);
    cfggcretain::ReferencerList before{},after{};
    std::array<std::uintptr_t,MaximumOwners> entries{},fresh{};
    const auto own=reinterpret_cast<std::uintptr_t>(&owner);
    report.owner=own;report.pawn=inputs[0];report.references=0;report.permanent=0;report.result=Result::Refused;report.stage=5;
    if(!inputs[0]||!access.Snapshot(before,entries)||before.count<0||before.count>=MaximumOwners||
       before.capacity<=before.count||!before.data)return;
    report.data=before.data;report.count=before.count;report.capacity=before.capacity;
    for(int i=0;i<before.count;++i)if(entries[static_cast<std::size_t>(i)]==own)return;
    if(!owner.Prepare(inputs)||!owner.BeginRegistration())return;
    for(const auto value:owner.expected)report.references+=value?1u:0u;
    report.stage=6;++report.calls;
    if(!access.Register(own)){
        owner.Registered(false);report.result=Result::Indeterminate;report.stage=8;return;
    }
    report.stage=7;
    const bool observed=access.Snapshot(after,fresh)&&after.data==before.data&&
        after.capacity==before.capacity&&after.count==before.count+1&&
        !std::memcmp(entries.data(),fresh.data(),static_cast<std::size_t>(before.count)*sizeof(std::uintptr_t))&&
        fresh[static_cast<std::size_t>(before.count)]==own;
    report.added=observed?1u:0u;
    if(!owner.Registered(observed)){
        // The owner storage is resident and its slot remains valid even if
        // readback is indeterminate; never unregister or reuse it here.
        report.result=Result::Indeterminate;report.stage=8;return;
    }
    report.permanent=1;report.result=Result::Passed;report.stage=8;
}
template<class Access>void RegisterPermanentPawnLocked(Access& access,cfggcretain::Owner& owner,
    Report& report,std::uintptr_t pawn) noexcept {
    RegisterPermanentInputsLocked(access,owner,report,std::array<std::uintptr_t,1>{pawn});
}
}
