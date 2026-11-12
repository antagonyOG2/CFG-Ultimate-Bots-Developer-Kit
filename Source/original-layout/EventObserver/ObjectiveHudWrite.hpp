#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
namespace cfgobjective {
// Native pointer-cache publication only, behind fresh owner/roster proofs.
// Unlike script operand replacement, an empty cache is expected and valid.
// Atomic compare/exchange never overwrites an entry changed by the engine.
inline bool PublishEmptyPointer(std::uintptr_t address,std::uintptr_t replacement) noexcept {
 if(address<0x10000||address>0x00007FFFFFFFFFF7ULL||(address&7)||!replacement)return false;
 MEMORY_BASIC_INFORMATION memory{};
 if(VirtualQuery(reinterpret_cast<const void*>(address),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
    memory.Type!=MEM_PRIVATE||(memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
    (memory.Protect&0xFF)!=PAGE_READWRITE)return false;
 const auto start=reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
 if(address<start||memory.RegionSize<8||address-start>memory.RegionSize-8)return false;
 __try {
  return InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(address),
      reinterpret_cast<void*>(replacement),nullptr)==nullptr;
 } __except(EXCEPTION_EXECUTE_HANDLER) {return false;}
}
// Only use behind Runtime's owner-thread/offline/metadata gate. This does not
// change protection or allocate. Script operands are commonly unaligned.
// False after the write is indeterminate: the caller retains its original record.
inline bool CompareWrite(std::uintptr_t address,std::uintptr_t expected,std::uintptr_t replacement) noexcept {
 if(address<0x10000||address>0x00007FFFFFFFFFF7ULL||!expected||!replacement)return false;
 MEMORY_BASIC_INFORMATION memory{};
 if(VirtualQuery(reinterpret_cast<const void*>(address),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
    memory.Type!=MEM_PRIVATE||(memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
    (memory.Protect&0xFF)!=PAGE_READWRITE)return false;
 const auto start=reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
 if(address<start||memory.RegionSize<8||address-start>memory.RegionSize-8)return false;
 std::uintptr_t before=0,after=0;SIZE_T done=0;
 if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),&before,8,&done)||done!=8||before!=expected)return false;
 done=0;if(!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&replacement,8,&done)||done!=8)return false;
 done=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),&after,8,&done)&&done==8&&after==replacement;
}
}
