#pragma once
#include "SpawnRuntimeBindings.generated.hpp"
#include <atomic>
#include <cstring>
namespace cfgdispatch {
// One process-pinned, owned UObject event bridge. Never accept a foreign hook.
// MinHook's six stolen push bytes and absolute continuation/relay are verified
// before enable; the remaining original prologue must stay byte-identical.
struct Hook {
 std::uintptr_t module=0,trampoline=0,relay=0,detour=0;
 std::array<unsigned char,19> entry{};
 std::array<unsigned char,20> continuation{};
 std::array<unsigned char,14> jump{};
};
inline Hook owned{};
inline std::atomic<bool> enabled{false};
inline std::array<unsigned char,14> AbsoluteJump(std::uintptr_t target){
 std::array<unsigned char,14> bytes{0xFF,0x25,0,0,0,0};
 std::memcpy(bytes.data()+6,&target,8);return bytes;
}
template<class R> bool Prepare(R& r,std::uintptr_t module,std::uintptr_t trampoline,
 std::uintptr_t detour,Hook& out){
 out={};Hook h;h.module=module;h.trampoline=trampoline;h.detour=detour;
 if(!module||!trampoline||!detour||trampoline>UINTPTR_MAX-34||module>UINTPTR_MAX-cfgspawn::GenericDispatcherRva-19)return false;
 const auto target=module+cfgspawn::GenericDispatcherRva;
 if(!r.Read(target,h.entry)||h.entry!=cfgspawn::GenericDispatcherPrefix||
    !r.Read(trampoline,h.continuation))return false;
 const auto back=AbsoluteJump(target+6);
 if(std::memcmp(h.continuation.data(),h.entry.data(),6)||
    std::memcmp(h.continuation.data()+6,back.data(),back.size()))return false;
 // MinHook appends its x64 relay immediately after the continuation jump.
 h.relay=trampoline+h.continuation.size();h.jump=AbsoluteJump(detour);
 std::array<unsigned char,14> relay{};
 if(!r.Read(h.relay,relay)||relay!=h.jump)return false;
 const auto displacement=static_cast<std::int64_t>(h.relay)-static_cast<std::int64_t>(target+5);
 if(displacement<INT32_MIN||displacement>INT32_MAX)return false;
 const auto relative=static_cast<std::int32_t>(displacement);h.entry[0]=0xE9;
 std::memcpy(h.entry.data()+1,&relative,4);out=h;return true;
}
template<class R> bool Proof(R& r,std::uintptr_t module,const Hook& h){
 std::array<unsigned char,19> entry{};std::array<unsigned char,20> continuation{};
 std::array<unsigned char,14> relay{};
 return module==h.module&&h.trampoline&&h.relay&&h.detour&&
  r.Read(module+cfgspawn::GenericDispatcherRva,entry)&&entry==h.entry&&
  r.Read(h.trampoline,continuation)&&continuation==h.continuation&&
  r.Read(h.relay,relay)&&relay==h.jump;
}
template<class R> bool Proof(R& r,std::uintptr_t module){
 if(enabled.load(std::memory_order_acquire))return Proof(r,module,owned);
 std::array<unsigned char,19> entry{};
 return r.Read(module+cfgspawn::GenericDispatcherRva,entry)&&entry==cfgspawn::GenericDispatcherPrefix;
}
}
