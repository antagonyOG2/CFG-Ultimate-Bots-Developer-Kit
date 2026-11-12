#pragma once
#include "GcRetentionBindings.generated.hpp"
#include "ObjectLifetime.hpp"
namespace cfggcretain {
struct ReferencerList {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const ReferencerList& other)const noexcept{
        return data==other.data&&count==other.count&&capacity==other.capacity;
    }
};
struct Observation {
    std::uintptr_t object=0,type=0;ReferencerList owners{};
    cfgprobe::lifetime::SlotObservation objectSlot{},classSlot{};
};
// Acquire the stable, typed endpoint without sampling its mutable owner list.
// The caller holds the engine/hash lock; the list is sampled only after its
// independently identified recursive lock is acquired.
template<class R>bool ObserveReferencerEndpoint(R& r,std::uintptr_t module,Observation& out){
    out={};Observation candidate;namespace p=cfgcompat::cfg35::gcRetention;
    cfgprobe::lifetime::ArrayHeader registry{},end{};std::uintptr_t type=0,again=0;
    if(!r.Read(module+p::ReferencerGlobalRva,candidate.object)||!cfgprobe::ObjectPointer(candidate.object)||
       !r.Read(module+p::ReferencerClassCacheRva,candidate.type)||!cfgprobe::ObjectPointer(candidate.type)||
       !r.Read(candidate.object+cfgprobe::ObjectClass,type)||type!=candidate.type||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgprobe::lifetime::ReadSlot(r,registry,candidate.object,candidate.objectSlot)||
       !cfgprobe::lifetime::ReadSlot(r,registry,candidate.type,candidate.classSlot)||
       !r.Read(module+p::ReferencerGlobalRva,again)||again!=candidate.object||
       !r.Read(module+p::ReferencerClassCacheRva,again)||again!=candidate.type||
       !r.Read(candidate.object+cfgprobe::ObjectClass,type)||type!=candidate.type||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,end)||!(registry==end))return false;
    cfgprobe::lifetime::SlotObservation fresh{};
    if(!cfgprobe::lifetime::ReadSlot(r,end,candidate.object,fresh)||!(fresh==candidate.objectSlot)||
       !cfgprobe::lifetime::ReadSlot(r,end,candidate.type,fresh)||!(fresh==candidate.classSlot))return false;
    out=candidate;return true;
}
template<class R>bool ObserveReferencer(R& r,std::uintptr_t module,Observation& out){
    out={};Observation candidate;
    namespace p=cfgcompat::cfg35::gcRetention;
    cfgprobe::lifetime::ArrayHeader registry{},end{};
    std::uintptr_t actualClass=0,again=0;ReferencerList list{};
    if(!r.Read(module+p::ReferencerGlobalRva,candidate.object)||!cfgprobe::ObjectPointer(candidate.object)||
       !r.Read(module+p::ReferencerClassCacheRva,candidate.type)||!cfgprobe::ObjectPointer(candidate.type)||
       !r.Read(candidate.object+cfgprobe::ObjectClass,actualClass)||actualClass!=candidate.type||
       !r.Read(candidate.object+p::OwnerArray,candidate.owners))return false;
    const auto& a=candidate.owners;
    if(a.count<0||a.capacity<a.count||a.capacity>65536||
       ((a.count||a.capacity)&&!cfgprobe::ObjectPointer(a.data)))return false;
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgprobe::lifetime::ReadSlot(r,registry,candidate.object,candidate.objectSlot)||
       !cfgprobe::lifetime::ReadSlot(r,registry,candidate.type,candidate.classSlot)||
       !r.Read(module+p::ReferencerGlobalRva,again)||again!=candidate.object||
       !r.Read(module+p::ReferencerClassCacheRva,again)||again!=candidate.type||
       !r.Read(candidate.object+p::OwnerArray,list)||!(list==a)||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,end)||!(registry==end))return false;
    cfgprobe::lifetime::SlotObservation fresh{};
    if(!cfgprobe::lifetime::ReadSlot(r,end,candidate.object,fresh)||!(fresh==candidate.objectSlot)||
       !cfgprobe::lifetime::ReadSlot(r,end,candidate.type,fresh)||!(fresh==candidate.classSlot))return false;
    out=candidate;return true; // Endpoint identity only; no registration/call approval.
}
enum class ReferencerWindowFailure:unsigned {None,Endpoint,ListLock,LockedObservation,ChangedEndpoint};
// Caller releases any acquired list lock on EVERY exit. No retry/native work
// here. Full original list/registry validation is retained, under the list lock.
template<class R,class Lock>bool ObserveReferencerWindow(R& r,std::uintptr_t module,Lock&& lock,
    Observation& out,ReferencerWindowFailure& failure){
    out={};failure=ReferencerWindowFailure::Endpoint;Observation endpoint,locked;
    if(!ObserveReferencerEndpoint(r,module,endpoint))return false;
    failure=ReferencerWindowFailure::ListLock;if(!lock(endpoint))return false;
    failure=ReferencerWindowFailure::LockedObservation;if(!ObserveReferencer(r,module,locked))return false;
    failure=ReferencerWindowFailure::ChangedEndpoint;
    if(locked.object!=endpoint.object||locked.type!=endpoint.type||!(locked.objectSlot==endpoint.objectSlot)||!(locked.classSlot==endpoint.classSlot))return false;
    out=locked;failure=ReferencerWindowFailure::None;return true;
}
}
