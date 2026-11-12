#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include "../Core/WorldRootRetirement.hpp"
namespace cfgretain {
// Slot bookkeeping only. Caller MUST hold its SynchronizedRootOwner's
// publication lock and independently verified engine window for EVERY method.
// No registration, lookup, GC authorization or native gameplay call occurs here.
// This is not yet connected to production root admission.
template<unsigned Capacity=32>class RootSlotLedger {
    static_assert(Capacity>0&&Capacity<=32);
public:
    struct Handle {
        const void* owner=nullptr;unsigned slot=Capacity;std::uint64_t generation=0;
        bool operator==(const Handle& b)const noexcept{return owner==b.owner&&slot==b.slot&&generation==b.generation;}
    };
    struct Reservation {const void* owner=nullptr;std::uint64_t id=0;unsigned count=0;};
private:
    std::array<std::uintptr_t,Capacity>& slots_;
    std::array<std::uintptr_t,Capacity> expected_{};
    std::array<std::uint64_t,Capacity> generations_{};
    std::array<std::uint64_t,Capacity> collectedGenerations_{};
    std::array<std::uint64_t,Capacity> reserved_{};
    std::uint64_t sequence_=0;bool poisoned_=false;
    bool Bump(unsigned slot)noexcept{
        if(generations_[slot]==(std::numeric_limits<std::uint64_t>::max)()){poisoned_=true;return false;}
        ++generations_[slot];return true;
    }
    static bool Pointer(std::uintptr_t value)noexcept{return value>=0x0/*CFG3.5 address withheld*/&&value<=0x00007FFFFFFFFFFFULL&&!(value&7u);}
public:
    explicit RootSlotLedger(std::array<std::uintptr_t,Capacity>& slots)noexcept:slots_(slots){
        for(auto value:slots_)if(value)poisoned_=true; // never adopt unaccounted raw addresses
    }
    RootSlotLedger(const RootSlotLedger&)=delete;
    RootSlotLedger& operator=(const RootSlotLedger&)=delete;
    bool Poisoned()const noexcept{return poisoned_;}
    bool Refresh()noexcept{
        if(poisoned_)return false;
        for(unsigned i=0;i<Capacity;++i){
            if(slots_[i]&&slots_[i]!=expected_[i]){poisoned_=true;return false;}
            if(!slots_[i]&&expected_[i]){collectedGenerations_[i]=generations_[i];if(!Bump(i))return false;expected_[i]=0;}
        }
        return true;
    }
    unsigned Live()const noexcept{unsigned count=0;for(auto value:expected_)count+=value?1u:0u;return count;}
    // Completed-menu recovery only. All ACTUAL collector slots are re-proved
    // under the owner's publication lock. Never trust poisoned expected values.
    template<class Classify>bool CanRebuildAtMenu(Classify classify)const noexcept{
        if(sequence_==(std::numeric_limits<std::uint64_t>::max)())return false;
        for(unsigned i=0;i<Capacity;++i){
            if(reserved_[i]||generations_[i]==(std::numeric_limits<std::uint64_t>::max)())return false;
            if(slots_[i]&&(!Pointer(slots_[i])||classify(slots_[i])==MapReference::Unproven))return false;
        }
        return true;
    }
    template<class Classify>bool RebuildAtMenu(Classify classify,unsigned& released)noexcept{
        released=0;if(!CanRebuildAtMenu(classify))return false;
        std::array<MapReference,Capacity> kinds{};
        for(unsigned i=0;i<Capacity;++i)if(slots_[i]){
            kinds[i]=classify(slots_[i]);if(kinds[i]==MapReference::Unproven)return false;
        }
        for(unsigned i=0;i<Capacity;++i){
            ++generations_[i];collectedGenerations_[i]=0; // invalidate EVERY earlier handle, including metadata
            if(slots_[i]&&kinds[i]==MapReference::OldMap){slots_[i]=0;++released;}
            expected_[i]=slots_[i];
        }
        poisoned_=false;return true;
    }
    template<class Classify>bool RetireMapReferences(Classify classify,unsigned& released)noexcept{
        released=0;if(!Refresh())return false;
        for(const auto reservation:reserved_)if(reservation)return false; // no in-flight native outputs
        std::array<bool,Capacity> retire{};
        for(unsigned i=0;i<Capacity;++i)if(expected_[i]){
            const auto kind=classify(expected_[i]);if(kind==MapReference::Unproven)return false;
            retire[i]=kind==MapReference::OldMap;
        }
        // Classification completes BEFORE mutation; caller keeps the engine
        // window and owner's local publication lock through the entire call.
        for(unsigned i=0;i<Capacity;++i)if(retire[i]){
            if(!Release({this,i,generations_[i]})){poisoned_=true;return false;}++released;
        }
        return true;
    }
    bool Reserve(unsigned maximumOutputs,Reservation& out)noexcept{
        out={};if(!maximumOutputs||maximumOutputs>Capacity||!Refresh())return false;
        unsigned free=0;for(unsigned i=0;i<Capacity;++i)free+=!expected_[i]&&!reserved_[i]?1u:0u;
        if(free<maximumOutputs||sequence_==(std::numeric_limits<std::uint64_t>::max)())return false;
        const auto id=++sequence_;unsigned selected=0;
        for(unsigned i=0;i<Capacity&&selected<maximumOutputs;++i)if(!expected_[i]&&!reserved_[i]){reserved_[i]=id;++selected;}
        out={this,id,maximumOutputs};return true;
    }
    bool Cancel(Reservation& token)noexcept{
        if(token.owner!=this||!token.id||!token.count)return false;unsigned count=0;
        for(const auto id:reserved_)count+=id==token.id?1u:0u;
        if(count!=token.count)return false;
        for(auto& id:reserved_)if(id==token.id)id=0;
        token={};return true;
    }
    template<unsigned N>bool Commit(Reservation& token,const std::array<std::uintptr_t,N>& acquired,
        std::array<Handle,N>& handles,unsigned actualCount=N)noexcept{
        handles={};if(token.owner!=this||!token.id||actualCount>N||actualCount>token.count||!Refresh())return false;
        unsigned count=0;for(const auto id:reserved_)count+=id==token.id?1u:0u;
        if(count!=token.count)return false;
        // Validate EVERYTHING before mutating one slot. 'acquired' must come
        // from authoritative current engine observations, not stale caches.
        for(unsigned i=0;i<actualCount;++i)if(acquired[i]&&!Pointer(acquired[i]))return false;
        for(unsigned i=0;i<Capacity;++i)if(reserved_[i]==token.id&&
            generations_[i]==(std::numeric_limits<std::uint64_t>::max)())return false;
        for(unsigned n=0;n<actualCount;++n){const auto value=acquired[n];if(!value)continue;
            unsigned slot=Capacity;
            // Deduplicate only WITHIN this acquisition. A separate native
            // transaction gets its own slot/handle even for the same UObject,
            // so releasing temporary inputs cannot release a retained result.
            for(unsigned i=0;i<n;++i)if(acquired[i]==value){slot=handles[i].slot;break;}
            if(slot==Capacity){
                for(unsigned i=0;i<Capacity;++i)if(reserved_[i]==token.id&&!expected_[i]){slot=i;break;}
                if(slot==Capacity){poisoned_=true;return false;} // impossible under validated reservation
                if(!Bump(slot))return false;
                collectedGenerations_[slot]=0;
                slots_[slot]=expected_[slot]=value;
            }
            handles[n]={this,slot,generations_[slot]};
        }
        return Cancel(token);
    }
    bool Resolve(Handle handle,std::uintptr_t& out)noexcept{
        out=0;if(handle.owner!=this||!Refresh()||handle.slot>=Capacity||!handle.generation||
           generations_[handle.slot]!=handle.generation||!expected_[handle.slot])return false;
        out=expected_[handle.slot];return true;
    }
    bool Release(Handle handle)noexcept{
        std::uintptr_t value=0;if(!Resolve(handle,value)||!Bump(handle.slot))return false;
        // Never restore a collector-cleared pointer. Releasing is a separate
        // explicit operation inside the same proved publication lock/window.
        slots_[handle.slot]=expected_[handle.slot]=0;collectedGenerations_[handle.slot]=0;return true;
    }
    bool ReleaseAcquired(Handle handle)noexcept{
        if(handle.owner!=this||!handle.generation||handle.slot>=Capacity||!Refresh())return false;
        if(generations_[handle.slot]==handle.generation)return Release(handle);
        // Refresh advances exactly once when the collector clears this claim.
        // A reallocated/released slot has another generation and is rejected.
        if(handle.generation!=(std::numeric_limits<std::uint64_t>::max)()&&
            collectedGenerations_[handle.slot]==handle.generation&&generations_[handle.slot]==handle.generation+1&&
            !expected_[handle.slot]&&!reserved_[handle.slot]){collectedGenerations_[handle.slot]=0;return true;}
        return false;
    }
};
}
