#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "OwnedNativeDetour.hpp"
#include "../RuntimeProbe/Compatibility/CfgJasonHunting35.generated.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include <cstdio>
namespace {
struct Object {unsigned ticks=0,pad=0;float delta=0;};
using Tick=void(__fastcall*)(void*,float);
Tick originalTick=nullptr;unsigned detours=0;
void __fastcall Detour(void* object,float delta){++detours;originalTick(object,delta);}
struct OwnedMemory:cfgownednative::CurrentProcess {
    std::uintptr_t target=0,module=0;
    bool Image(std::uintptr_t at,std::uintptr_t image,std::size_t n){
        return at==target&&image==module&&n==153&&Executable(at,n);
    }
};
}
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL native tick owned hook check%u\n",checks);}};
    auto* code=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
    check(code!=nullptr);if(!code)return 1;
    std::array<unsigned char,153> expected{};expected.fill(0xCC);
    // Exact first9bytes of verified33ECA0; remaining OWNED fixture stores the
    // true Win64 second positional float (XMM1), increments receiver and returns.
    const unsigned char body[]={0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B,0xD9,
        0xF3,0x0F,0x11,0x49,0x08,0xFF,0x01,0x48,0x83,0xC4,0x20,0x5B,0xC3};
    for(unsigned i=0;i<9;++i)check(body[i]==cfgcompat::cfg35::j1a::KillerControllerTickCode[i]);
    std::memcpy(expected.data(),body,sizeof(body));std::memcpy(code,expected.data(),expected.size());
    DWORD previous=0;check(VirtualProtect(code,4096,PAGE_EXECUTE_READ,&previous)!=FALSE);
    FlushInstructionCache(GetCurrentProcess(),code,expected.size());
    check(MH_Initialize()==MH_OK);void* original=nullptr;
    check(MH_CreateHook(code,reinterpret_cast<void*>(&Detour),&original)==MH_OK);
    if(!original){MH_Uninitialize();VirtualFree(code,0,MEM_RELEASE);return 1;}
    originalTick=reinterpret_cast<Tick>(original);
    OwnedMemory memory;memory.target=reinterpret_cast<std::uintptr_t>(code);memory.module=memory.target-0x1000;
    cfgownednative::Registry bank;
    check(bank.Prepare(memory,memory.module,memory.target,expected,reinterpret_cast<std::uintptr_t>(original),reinterpret_cast<std::uintptr_t>(&Detour),6));
    cfgownednative::Registry wrong;
    check(!wrong.Prepare(memory,memory.module,memory.target,expected,reinterpret_cast<std::uintptr_t>(original),reinterpret_cast<std::uintptr_t>(&Detour),9));
    check(MH_EnableHook(code)==MH_OK&&bank.Enable(memory,memory.target));
    check(bank.Resolve(memory,memory.module,memory.target,expected)==reinterpret_cast<std::uintptr_t>(original));
    Object object{};reinterpret_cast<Tick>(code)(&object,0.375f);
    check(detours==1&&object.ticks==1&&object.delta==0.375f);
    originalTick(&object,0.625f);check(detours==1&&object.ticks==2&&object.delta==0.625f);
    check(MH_DisableHook(code)==MH_OK);bank.Disable(memory.target);
    check(bank.Resolve(memory,memory.module,memory.target,expected)==memory.target);
    check(MH_RemoveHook(code)==MH_OK&&MH_Uninitialize()==MH_OK);
    check(VirtualFree(code,0,MEM_RELEASE)!=FALSE);
    std::printf("Native Jason Tick real MinHook/stolen6bytes/Win64float forwarding: %u checks,%u failures. OWN PROCESS synthetic code ONLY.\n",checks,failures);
    return failures?1:0;
}
