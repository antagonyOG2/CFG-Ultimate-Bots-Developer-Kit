#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "PendingSandboxStart.hpp"
namespace cfgmenu {
// Low-level operation only; provides no object/property/hook authority.
// Callers must prove the exact current owned field before AND after exchange.
inline sandboxstart::WriteResult ExchangeOwnedNullablePointer(std::uintptr_t address,std::uintptr_t expected,std::uintptr_t replacement)noexcept{
    if(!cfgprobe::ObjectPointer(address)||(expected&&!cfgprobe::ObjectPointer(expected))||(replacement&&!cfgprobe::ObjectPointer(replacement)))return sandboxstart::WriteResult::Refused;
    MEMORY_BASIC_INFORMATION memory{};
    if(VirtualQuery(reinterpret_cast<const void*>(address),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
        (memory.Protect!=PAGE_READWRITE&&memory.Protect!=PAGE_WRITECOPY)||memory.Type!=MEM_PRIVATE||
        address<reinterpret_cast<std::uintptr_t>(memory.BaseAddress)||
        address-reinterpret_cast<std::uintptr_t>(memory.BaseAddress)>memory.RegionSize||
        memory.RegionSize-(address-reinterpret_cast<std::uintptr_t>(memory.BaseAddress))<8)return sandboxstart::WriteResult::Refused;
    __try{
        const auto actual=InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(address),
            reinterpret_cast<void*>(replacement),reinterpret_cast<void*>(expected));
        return actual==reinterpret_cast<void*>(expected)?sandboxstart::WriteResult::Changed:sandboxstart::WriteResult::Refused;
    }__except(EXCEPTION_EXECUTE_HANDLER){return sandboxstart::WriteResult::Indeterminate;}
}
inline sandboxstart::WriteResult ExchangeOwnedPointer(std::uintptr_t address,std::uintptr_t expected,std::uintptr_t replacement)noexcept{
    if(!cfgprobe::ObjectPointer(expected)||!cfgprobe::ObjectPointer(replacement))return sandboxstart::WriteResult::Refused;
    return ExchangeOwnedNullablePointer(address,expected,replacement);
}
}
