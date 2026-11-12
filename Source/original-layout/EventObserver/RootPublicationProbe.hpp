#pragma once
#include "GcRegistryProbe.hpp"
namespace cfgrootprobe {
enum class Stage:unsigned {Waiting,HumanRetained,Swapped,Observed,Refused};
struct Report {
    std::uint32_t bytes=sizeof(Report),version=1,stage=0,reason=0;
    std::uintptr_t module=0,owner=0,world=0,human=0;
    unsigned callbacksAtSwap=0,submissionsAtSwap=0,registrations=0,swaps=0;
    unsigned callbacksAtHumanPublication=0,submissionsAtHumanPublication=0;
};
static_assert(std::is_trivially_copyable_v<Report>);
inline bool Preserved(const Report& r,const cfggcprobe::OwnerObservation& owner,unsigned unknown)noexcept{
    if(r.bytes!=sizeof(r)||r.version!=1||r.stage!=static_cast<unsigned>(Stage::Observed)||r.reason||unknown||
       r.registrations!=1||r.swaps!=1||!r.module||!r.owner||!r.world||!r.human||r.world==r.human||
       r.callbacksAtSwap<=r.callbacksAtHumanPublication||r.submissionsAtSwap<r.submissionsAtHumanPublication||
       r.submissionsAtSwap-r.submissionsAtHumanPublication!=r.callbacksAtSwap-r.callbacksAtHumanPublication||
       owner.state!=cfggcretain::State::Registered||owner.destroyRequests||owner.cleared||
       owner.callbacks!=owner.completed||owner.completed<=r.callbacksAtSwap||
       owner.slots!=owner.expected||owner.slots[0]!=r.world||owner.slots[1]!=r.human||
       static_cast<std::uint64_t>(owner.submissions)!=static_cast<std::uint64_t>(r.submissionsAtSwap)+
           2ULL*(owner.completed-r.callbacksAtSwap))return false;
    for(unsigned i=2;i<owner.slots.size();++i)if(owner.slots[i])return false;
    return true;
}
}
