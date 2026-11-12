#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
namespace cfginventory {
// CFG's native ILLInventoryComponent API, not the donor Blueprint SpawnItem.
// AddItem has no return value; success requires a later ownership/roster check.
struct FindParameters {std::uintptr_t itemClass=0,result=0;};
struct AddParameters {std::uintptr_t item=0;};
static_assert(sizeof(FindParameters)==16&&offsetof(FindParameters,result)==8,"CFG FindItem native frame");
static_assert(sizeof(AddParameters)==8&&offsetof(AddParameters,item)==0,"CFG AddItem native frame");
static_assert(std::is_trivially_copyable<FindParameters>::value&&std::is_trivially_copyable<AddParameters>::value,"No dynamic parameter cleanup");
}
