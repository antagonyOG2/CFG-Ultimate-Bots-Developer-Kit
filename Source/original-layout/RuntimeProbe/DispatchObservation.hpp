#pragma once
#include "ObjectLifetime.hpp"

namespace cfgprobe::dispatch {
// Corroborated disk evidence only; no native function binding/call authority.
constexpr std::uintptr_t ThreadFlagRva=cfgcompat::cfg35::dispatch::ThreadFlagRva;
constexpr std::uintptr_t ThreadIdRva=cfgcompat::cfg35::dispatch::ThreadIdRva;
constexpr std::uintptr_t EventSlotBytes=cfgcompat::cfg35::dispatch::ReflectedEventSlotBytes;
constexpr std::uintptr_t NativeActorDispatcherRva=cfgcompat::cfg35::dispatch::NativeActorDispatcherRva;
struct Observation {
    std::uint8_t flag=0;
    std::uint32_t threadId=0;
    std::uintptr_t vtable=0, target=0;
    bool operator==(const Observation& other) const noexcept {
        return flag==other.flag && threadId==other.threadId &&
            vtable==other.vtable && target==other.target;
    }
};
template<class Reader> bool Observe(Reader& reader,std::uintptr_t module,
    std::uintptr_t pawn,Observation& output) {
    output={}; Observation candidate;
    if (!ObjectPointer(module) || !lifetime::Extent(module,ThreadIdRva+sizeof(candidate.threadId)) ||
        !ObjectPointer(pawn) || !reader.Read(module+ThreadFlagRva,candidate.flag) ||
        candidate.flag!=1 || !reader.Read(module+ThreadIdRva,candidate.threadId) ||
        !candidate.threadId || !reader.Read(pawn,candidate.vtable) ||
        !ObjectPointer(candidate.vtable) || !lifetime::Extent(candidate.vtable,EventSlotBytes+sizeof(candidate.target)) ||
        !reader.Read(candidate.vtable+EventSlotBytes,candidate.target) ||
        !lifetime::Extent(candidate.target,1)) return false;
    output=candidate; return true;
}
template<class Reader> bool Revalidate(Reader& reader,std::uintptr_t module,
    std::uintptr_t pawn,const Observation& observed) {
    Observation fresh;
    return Observe(reader,module,pawn,fresh) && fresh==observed;
}
// Membership and executable-region evidence are checked separately by Windows.
// Even a successful observation is NOT a callback or permission to call target.

namespace diagnostic {
// Read-only reverse-engineering observation. These field offsets and the
// virtual slot were recovered from the exact CFG 3.5 dispatcher at RVA
// 0x00602140; this is NOT a production binding or call authorization.
constexpr std::uintptr_t RegistryRva=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t ArrayData=0x40,ArrayCount=0x50,EntryBytes=0x40;
constexpr std::uintptr_t EntryCallbackObject=0x20,EntryEnabled=0x30,VtableSlot=0x68;
constexpr std::size_t MaximumCallbacks=64;
struct Callback {
    std::uint32_t enabled=0;
    std::uintptr_t object=0,vtable=0,target=0;
    bool operator==(const Callback& other)const noexcept {
        return enabled==other.enabled&&object==other.object&&vtable==other.vtable&&target==other.target;
    }
};
struct Registry {
    std::uintptr_t data=0;
    std::int32_t count=0;
    std::uint32_t dispatchDepth=0;
    std::array<Callback,MaximumCallbacks> callbacks{};
    bool operator==(const Registry& other)const noexcept {
        if(data!=other.data||count!=other.count||dispatchDepth!=other.dispatchDepth)return false;
        for(std::int32_t i=0;i<count;++i)
            if(!(callbacks[static_cast<std::size_t>(i)]==other.callbacks[static_cast<std::size_t>(i)]))return false;
        return true;
    }
    unsigned ActiveCount()const noexcept {
        unsigned active=0;
        for(std::int32_t i=0;i<count;++i)
            active+=callbacks[static_cast<std::size_t>(i)].enabled!=0?1u:0u;
        return active;
    }
};
template<class Reader> bool ObserveRegistry(Reader& reader,std::uintptr_t module,Registry& output) {
    output={}; Registry candidate;
    if(!ObjectPointer(module)||!lifetime::Extent(module+RegistryRva,ArrayCount+sizeof(candidate.count))||
        !reader.Read(module+RegistryRva+ArrayData,candidate.data)||
        !reader.Read(module+RegistryRva+ArrayCount,candidate.count)||
        !reader.Read(module+RegistryRva+0x64,candidate.dispatchDepth)||
        candidate.count<0||static_cast<std::size_t>(candidate.count)>MaximumCallbacks)return false;
    if(candidate.count>0&&(!ObjectPointer(candidate.data)||
        !lifetime::Extent(candidate.data,static_cast<std::uintptr_t>(candidate.count)*EntryBytes)))return false;
    for(std::int32_t i=0;i<candidate.count;++i) {
        auto& item=candidate.callbacks[static_cast<std::size_t>(i)];
        const auto entry=candidate.data+static_cast<std::uintptr_t>(i)*EntryBytes;
        if(!reader.Read(entry+EntryEnabled,item.enabled))return false;
        if(!item.enabled)continue;
        if(!reader.Read(entry+EntryCallbackObject,item.object)||!ObjectPointer(item.object)||
            !reader.Read(item.object,item.vtable)||!ObjectPointer(item.vtable)||
            !lifetime::Extent(item.vtable,VtableSlot+sizeof(item.target))||
            !reader.Read(item.vtable+VtableSlot,item.target)||!lifetime::Extent(item.target,1))return false;
    }
    output=candidate;return true;
}
template<class Reader> bool RevalidateRegistry(Reader& reader,std::uintptr_t module,
    const Registry& observed,Registry* endpoint=nullptr) {
    Registry fresh;
    if(!ObserveRegistry(reader,module,fresh)||!(fresh==observed))return false;
    if(endpoint)*endpoint=fresh;
    return true;
}
} // namespace diagnostic
} // namespace cfgprobe::dispatch
