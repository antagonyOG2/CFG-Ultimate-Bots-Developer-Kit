#include "TrampolinePreparation.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include <cstdio>

int main(){
    // Exercise the real allocators in this test process, never in CFG.
    auto* code=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
    if(!code)return 1;
    for(unsigned offset: {0u,128u,256u}){
        std::memcpy(code+offset,cfgobserver::ExpectedPrefix.data(),cfgobserver::ExpectedPrefix.size());
        code[offset+cfgobserver::ExpectedPrefix.size()]=0xC3;
    }
    code[512]=0xC3;
    DWORD old=0;
    if(!VirtualProtect(code,4096,PAGE_EXECUTE_READ,&old)||!FlushInstructionCache(GetCurrentProcess(),code,4096))return 2;
    cfgobserver::Prepared observer;
    if(!cfgobserver::Prepare(code,code+512,observer))return 3;
    MEMORY_BASIC_INFORMATION sealed{};
    if(!VirtualQuery(reinterpret_cast<void*>(observer.trampoline),&sealed,sizeof(sealed))||sealed.Protect!=PAGE_EXECUTE_READ)return 4;
    if(MH_Initialize()!=MH_OK)return 5;
    for(unsigned offset: {128u,256u}){
        void* original=nullptr;
        if(MH_CreateHook(code+offset,code+512,&original)!=MH_OK||!original)return 6;
        MEMORY_BASIC_INFORMATION pool{};
        if(!VirtualQuery(original,&pool,sizeof(pool))||pool.AllocationBase==sealed.AllocationBase)return 7;
        if(MH_QueueEnableHook(code+offset)!=MH_OK)return 8;
    }
    if(MH_ApplyQueued()!=MH_OK)return 9;
    if(MH_DisableHook(MH_ALL_HOOKS)!=MH_OK||MH_Uninitialize()!=MH_OK)return 10;
    MEMORY_BASIC_INFORMATION after{};
    if(!VirtualQuery(reinterpret_cast<void*>(observer.trampoline),&after,sizeof(after))||after.Protect!=PAGE_EXECUTE_READ||
       !cfgobserver::AbsoluteJump(reinterpret_cast<unsigned char*>(observer.trampoline)+5,reinterpret_cast<std::uintptr_t>(code)+5))return 11;
    std::puts("PASS: sealed observer pool remains RX across two real MinHook create/enable/disable operations");
    return 0;
}
