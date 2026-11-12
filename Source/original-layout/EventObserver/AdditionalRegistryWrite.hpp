#pragma once
#include <windows.h>
#include <cstdint>
namespace cfgadditionalwrite {
// F1 registry fields ONLY, after fresh world + positively journaled extra
// attribution. Unlike script operands, an attributed null state is legitimate.
// Never use this to infer ownership, repair a null pawn, or change protection.
inline bool Exchange(std::uintptr_t address,std::uintptr_t expected,std::uintptr_t replacement)noexcept{
    if(address<0x10000||address>0x00007FFFFFFFFFF7ULL||(address&7))return false;
    MEMORY_BASIC_INFORMATION memory{};
    if(VirtualQuery(reinterpret_cast<const void*>(address),&memory,sizeof(memory))!=sizeof(memory)||
       memory.State!=MEM_COMMIT||memory.Type!=MEM_PRIVATE||(memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
       (memory.Protect&0xFF)!=PAGE_READWRITE)return false;
    const auto start=reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
    if(address<start||memory.RegionSize<8||address-start>memory.RegionSize-8)return false;
    __try{return InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(address),
        reinterpret_cast<void*>(replacement),reinterpret_cast<void*>(expected))==reinterpret_cast<void*>(expected);}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
}
