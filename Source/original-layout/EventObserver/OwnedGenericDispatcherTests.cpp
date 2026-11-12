#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../RuntimeProbe/OwnedGenericDispatcher.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include "OwnedByteReader.hpp"
#include <cstdio>
using Dispatch=void(__fastcall*)(void*,void*,void*);
static Dispatch original=nullptr;static unsigned forwards=0;
static void __fastcall Detour(void* object,void* function,void* parameters){++forwards;original(object,function,parameters);}
struct LocalReader {template<class T>bool Read(std::uintptr_t at,T& out){SIZE_T read=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&out,sizeof(out),&read)&&read==sizeof(out);}};
int main(){
 auto* code=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));if(!code)return 1;
 const unsigned char tail[]{0x48,0x81,0xC4,0xF0,0,0,0,0x41,0x5F,0x41,0x5E,0x41,0x5D,0x41,0x5C,0x5F,0x5E,0x5D,0xC3};
 std::memcpy(code,cfgspawn::GenericDispatcherPrefix.data(),19);std::memcpy(code+19,tail,sizeof(tail));
 DWORD old=0;if(!VirtualProtect(code,4096,PAGE_EXECUTE_READ,&old)||!FlushInstructionCache(GetCurrentProcess(),code,4096))return 2;
 if(MH_Initialize()!=MH_OK)return 3;void* trampoline=nullptr;
 if(MH_CreateHook(code,reinterpret_cast<void*>(&Detour),&trampoline)!=MH_OK)return 4;
 LocalReader reader;cfgdispatch::Hook h;const auto module=reinterpret_cast<std::uintptr_t>(code)-cfgspawn::GenericDispatcherRva;
 if(!cfgdispatch::Prepare(reader,module,reinterpret_cast<std::uintptr_t>(trampoline),reinterpret_cast<std::uintptr_t>(&Detour),h))return 5;
 original=reinterpret_cast<Dispatch>(trampoline);
 if(MH_EnableHook(code)!=MH_OK||!cfgdispatch::Proof(reader,module,h))return 6;
 reinterpret_cast<Dispatch>(code)(nullptr,nullptr,nullptr);if(forwards!=1)return 7;
 OwnedByteReader sparse;sparse.Set(module+cfgspawn::GenericDispatcherRva,h.entry);sparse.Set(h.trampoline,h.continuation);sparse.Set(h.relay,h.jump);
 unsigned failures=0;
 if(cfgdispatch::enabled.load())++failures;
 for(unsigned fault=0;fault<4;++fault){auto r=sparse;
  if(fault==1)r.Set(module+cfgspawn::GenericDispatcherRva,std::uint8_t{});
  if(fault==2)r.Set(h.trampoline,std::uint8_t{});
  if(fault==3)r.Set(h.relay,std::uint8_t{});
  if(cfgdispatch::Proof(r,module,h)!=(fault==0))++failures;}
 if(cfgdispatch::Proof(sparse,module+8,h))++failures;
 if(MH_DisableHook(code)!=MH_OK||!cfgdispatch::Proof(reader,module))return 8;
 MH_RemoveHook(code);MH_Uninitialize();VirtualFree(code,0,MEM_RELEASE);
 std::printf("Owned UObject dispatcher: actual MinHook6-byte continuation/relay PASS, native forward once, %u foreign/stale proof failures; own test process only\n",failures);
 return failures?9:0;
}
