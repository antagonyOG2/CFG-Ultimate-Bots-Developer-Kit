#pragma once
#include "../RuntimeProbe/GcRetentionBindings.generated.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cfggcretain {
constexpr std::size_t Capacity=32;
enum class State:unsigned { Empty,Prepared,Registering,Registered,Removing,Removed,Abandoned };
struct Owner;
struct Vtable {
    Owner* (__fastcall* destroy)(Owner*,unsigned) noexcept;
    void (__fastcall* collect)(Owner*,void*) noexcept;
};
extern const Vtable NativeVtable;

// Must be module-owned and resident through process exit. Slots are initialized
// before registration and may only be inspected/changed outside collection
// under the separately verified engine synchronization window. The collector
// is allowed to NULL a slot; expected values are comparison-only, never restored.
struct Owner {
    const Vtable* vtable=&NativeVtable;
    std::array<std::uintptr_t,Capacity> slots{},expected{};
    std::atomic<unsigned> callbacks{0},completedCallbacks{0},submissions{0},clearedSlots{0},destroyRequests{0};
    std::atomic<State> state{State::Empty};

    // Only for the isolated, same-window register/remove ABI test. No UObject
    // references are exposed and this does not admit the gameplay spawn path.
    bool PrepareEmptyRegistryProbe() noexcept {
        for(std::size_t i=0;i<Capacity;++i)if(slots[i]||expected[i])return false;
        auto before=State::Empty;
        return state.compare_exchange_strong(before,State::Prepared);
    }

    template<std::size_t N>bool Prepare(const std::array<std::uintptr_t,N>& input) noexcept {
        if(state.load()!=State::Empty)return false;
        std::array<std::uintptr_t,Capacity> candidate{};std::size_t used=0;
        for(const auto value:input){
            if(!value)continue;
            if(value<0x10000||value>0x00007FFFFFFFFFFFULL||(value&7u))return false;
            bool duplicate=false;for(std::size_t i=0;i<used;++i)duplicate|=candidate[i]==value;
            if(duplicate)continue;
            if(used==Capacity)return false;
            candidate[used++]=value;
        }
        if(!used)return false;
        expected=slots=candidate;state.store(State::Prepared);return true;
    }
    bool BeginRegistration() noexcept {
        auto before=State::Prepared;return state.compare_exchange_strong(before,State::Registering);
    }
    bool Registered(bool membershipConfirmed) noexcept {
        auto before=State::Registering;
        return state.compare_exchange_strong(before,membershipConfirmed?State::Registered:State::Abandoned)&&membershipConfirmed;
    }
    bool BeginRemoval() noexcept {
        auto before=State::Registered;return state.compare_exchange_strong(before,State::Removing);
    }
    bool Removed(bool absenceConfirmed) noexcept {
        auto before=State::Removing;
        return state.compare_exchange_strong(before,absenceConfirmed?State::Removed:State::Abandoned)&&absenceConfirmed;
    }
    bool SlotsUnchanged() const noexcept {
        // Caller must hold the verified publication/removal window.
        return slots==expected;
    }
    template<class Submit>void CollectWith(Submit&& submit) noexcept {
        callbacks.fetch_add(1,std::memory_order_relaxed);
        // Ignore registration state: GC can enter after native registration
        // before its caller has published the successful membership result.
        for(auto& slot:slots)if(slot){
            const auto before=slot;
            submit(&slot);submissions.fetch_add(1,std::memory_order_relaxed);
            if(before&&!slot)clearedSlots.fetch_add(1,std::memory_order_relaxed);
        }
        completedCallbacks.fetch_add(1,std::memory_order_release);
    }
};
static_assert(std::is_standard_layout_v<Owner> && offsetof(Owner,vtable)==0);
static_assert(sizeof(Vtable)==16 && offsetof(Vtable,collect)==8);
static_assert(std::atomic<unsigned>::is_always_lock_free);
static_assert(sizeof(std::atomic<unsigned>)==sizeof(unsigned) && sizeof(std::atomic<State>)==sizeof(State));

inline Owner* __fastcall ResidentDestroy(Owner* owner,unsigned) noexcept {
    // Native registration/removal never destroys the reference owner. An
    // unexpected deleting-destructor request poisons reuse but cannot free
    // module-owned storage or leave a dangling registered stack pointer.
    owner->destroyRequests.fetch_add(1,std::memory_order_relaxed);
    owner->state.store(State::Abandoned);return owner;
}
inline void __fastcall NativeCollect(Owner* owner,void* collector) noexcept {
    using Submit=void(__fastcall*)(void*,std::uintptr_t*,void*,void*);
    const auto table=*static_cast<std::uintptr_t**>(collector);
    const auto submit=reinterpret_cast<Submit>(table[disk::profile::CollectorSubmitSlot/sizeof(std::uintptr_t)]);
    // This runs DURING collection. Never gate it on LookupSafe/GC-clear, log,
    // allocate, or call gameplay. The engine supplies the collector interface.
    owner->CollectWith([&](std::uintptr_t* slot) noexcept {submit(collector,slot,nullptr,nullptr);});
}
inline const Vtable NativeVtable{ResidentDestroy,NativeCollect};
} // namespace cfggcretain
