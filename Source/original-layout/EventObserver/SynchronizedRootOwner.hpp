#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "GcReferenceOwner.hpp"
#include "../RuntimeProbe/CollectorSubmitBindings.generated.hpp"
#include <cstring>
namespace cfgretain {
struct SynchronizedRootOwner;
extern const cfggcretain::Vtable SynchronizedRootVtable;
// Resident storage only. This owner is not registered by this header and does
// not authorize a publication window. Existing immutable spawn owners remain
// unchanged. No lock may be held across a native allocating gameplay call.
struct SynchronizedRootOwner {
    cfggcretain::Owner native;
    SRWLOCK gate=SRWLOCK_INIT;
    std::uintptr_t module=0;
    std::atomic<unsigned> unknownSubmit{0};
    std::atomic<std::uintptr_t> lastSubmit{0};
    SynchronizedRootOwner()noexcept{native.vtable=&SynchronizedRootVtable;}
    SynchronizedRootOwner(const SynchronizedRootOwner&)=delete;
    SynchronizedRootOwner& operator=(const SynchronizedRootOwner&)=delete;
    template<class Window,class Publish>bool TryPublication(Window& window,Publish&& publish)noexcept{
        if(!window.ProvenSlotWindow()||unknownSubmit.load(std::memory_order_acquire)||
           !TryAcquireSRWLockExclusive(&gate))return false;
        bool okay=false;
        __try{
            // No allocation/native dispatch here. A late owner callback and
            // publication serialize on THIS lock; the global engine lock alone
            // does not imply that its queued workers have joined.
            if(window.ProvenSlotWindow()&&!unknownSubmit.load(std::memory_order_acquire)&&
               native.state.load()==cfggcretain::State::Registered&&
               !native.destroyRequests.load()&&native.callbacks.load()==native.completedCallbacks.load())
                okay=publish(native);
        }__finally{ReleaseSRWLockExclusive(&gate);}
        return okay;
    }
    template<class Submit>void CollectWith(Submit&& submit,bool exactSynchronousHandler)noexcept{
        AcquireSRWLockExclusive(&gate);
        __try{
            // Unknown implementations still receive the original references;
            // they are never silently omitted. But their slot storage becomes
            // immutable forever, since their address-retention rules are unknown.
            if(!exactSynchronousHandler)unknownSubmit.fetch_add(1,std::memory_order_release);
            native.CollectWith(submit);
        }__finally{ReleaseSRWLockExclusive(&gate);}
    }
    bool KnownSubmit(std::uintptr_t target)const noexcept{
        if(!module)return false;
        if(target==module+cfgcollectorsubmit::Parallel)
            return !std::memcmp(reinterpret_cast<const void*>(target),cfgcollectorsubmit::ParallelCode.data(),cfgcollectorsubmit::ParallelCode.size());
        if(target==module+cfgcollectorsubmit::Serial)
            return !std::memcmp(reinterpret_cast<const void*>(target),cfgcollectorsubmit::SerialCode.data(),cfgcollectorsubmit::SerialCode.size());
        return false;
    }
};
static_assert(std::is_standard_layout_v<SynchronizedRootOwner> && offsetof(SynchronizedRootOwner,native)==0);
inline cfggcretain::Owner* __fastcall SynchronizedDestroy(cfggcretain::Owner* native,unsigned flags)noexcept{
    return cfggcretain::ResidentDestroy(native,flags);
}
inline void __fastcall SynchronizedCollect(cfggcretain::Owner* native,void* collector)noexcept{
    auto& owner=*reinterpret_cast<SynchronizedRootOwner*>(native);
    const auto table=*static_cast<std::uintptr_t**>(collector);
    const auto target=table[cfggcretain::disk::profile::CollectorSubmitSlot/sizeof(std::uintptr_t)];
    owner.lastSubmit.store(target,std::memory_order_release);
    using Submit=void(__fastcall*)(void*,std::uintptr_t*,void*,void*);
    const auto submit=reinterpret_cast<Submit>(target);
    owner.CollectWith([&](std::uintptr_t* slot)noexcept{submit(collector,slot,nullptr,nullptr);},owner.KnownSubmit(target));
}
inline const cfggcretain::Vtable SynchronizedRootVtable{SynchronizedDestroy,SynchronizedCollect};
}
