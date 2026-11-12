#pragma once
#include "SynchronizedRootOwner.hpp"
#include "GcRegistryProbe.hpp"
namespace cfgretain {
// Register a fixed EMPTY resident owner ONCE before root publication. No native
// allocation is allowed: existing backing storage and spare capacity are proved
// while the caller holds BOTH engine/hash and list locks. Slots remain resident
// even when confirmation fails; this function never removes or recycles owners.
template<class Access>bool RegisterResidentRootOwnerLocked(Access& access,SynchronizedRootOwner& owner,
    cfggcprobe::Report& report)noexcept{
    report={};report.result=cfggcprobe::Result::Refused;report.stage=5;
    const auto own=reinterpret_cast<std::uintptr_t>(&owner.native);report.owner=own;
    cfggcretain::ReferencerList before{},after{};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{},fresh{};
    if(!access.ProvenSlotWindow()||!owner.module||owner.unknownSubmit.load()||
       owner.native.vtable!=&SynchronizedRootVtable||
       !access.Snapshot(before,entries)||before.count<0||before.count>=cfggcprobe::MaximumOwners||
       before.capacity<=before.count||!before.data)return false;
    report.data=before.data;report.count=before.count;report.capacity=before.capacity;
    for(int i=0;i<before.count;++i)if(entries[static_cast<unsigned>(i)]==own)return false;
    if(!owner.native.PrepareEmptyRegistryProbe()||!owner.native.BeginRegistration())return false;
    report.stage=6;++report.calls;
    if(!access.Register(own)){
        owner.native.Registered(false);report.result=cfggcprobe::Result::Indeterminate;return false;
    }
    report.stage=7;
    const bool observed=access.ProvenSlotWindow()&&access.Snapshot(after,fresh)&&
        after.data==before.data&&after.capacity==before.capacity&&after.count==before.count+1&&
        !std::memcmp(entries.data(),fresh.data(),static_cast<std::size_t>(before.count)*sizeof(std::uintptr_t))&&
        fresh[static_cast<unsigned>(before.count)]==own;
    report.added=observed?1u:0u;
    if(!owner.native.Registered(observed)||owner.unknownSubmit.load()){
        report.result=cfggcprobe::Result::Indeterminate;return false;
    }
    report.stage=8;report.permanent=1;report.result=cfggcprobe::Result::Passed;return true;
}
}
