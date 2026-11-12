#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <climits>
namespace cfgurination {
// These three native functions all begin with exactly the five-byte
// mov [rsp+8/10],rbx instruction. This matches the existing bundled MinHook x64
// relocation:5 original bytes,14-byte absolute continuation,14-byte relay.
// Unsupported prologues/foreign hooks refuse; no nearest-pattern inference.
struct OwnedHook {
    std::uintptr_t module=0,target=0,trampoline=0,relay=0,detour=0;
    unsigned bytes=0;
    std::array<unsigned char,512> entry{};
    std::array<unsigned char,19> continuation{};
    std::array<unsigned char,14> jump{};
};
inline std::array<unsigned char,14> HookJump(std::uintptr_t target){
    std::array<unsigned char,14> out{0xFF,0x25,0,0,0,0};std::memcpy(out.data()+6,&target,8);return out;
}
template<class R,std::size_t N>bool PrepareOwnedHook(R& r,std::uintptr_t module,std::uintptr_t target,
    std::uintptr_t trampoline,std::uintptr_t detour,const std::array<unsigned char,N>& expected,OwnedHook& out){
    static_assert(N>=5&&N<=512);out={};OwnedHook h{};
    if(!module||target<=module||target>UINTPTR_MAX-N||!trampoline||trampoline>UINTPTR_MAX-33||!detour||
        expected[0]!=0x48||expected[1]!=0x89||expected[2]!=0x5C||expected[3]!=0x24||
        (expected[4]!=0x08&&expected[4]!=0x10))return false;
    std::array<unsigned char,N> actual{};
    if(!r.Read(target,actual)||actual!=expected||!r.Read(trampoline,h.continuation))return false;
    const auto back=HookJump(target+5);
    if(std::memcmp(h.continuation.data(),expected.data(),5)||std::memcmp(h.continuation.data()+5,back.data(),14))return false;
    h.module=module;h.target=target;h.trampoline=trampoline;h.relay=trampoline+19;h.detour=detour;h.bytes=N;h.jump=HookJump(detour);
    std::array<unsigned char,14> relay{};
    if(!r.Read(h.relay,relay)||relay!=h.jump)return false;
    const auto distance=static_cast<std::int64_t>(h.relay)-static_cast<std::int64_t>(target+5);
    if(distance<INT32_MIN||distance>INT32_MAX)return false;
    std::memcpy(h.entry.data(),expected.data(),N);h.entry[0]=0xE9;
    const auto displacement=static_cast<std::int32_t>(distance);std::memcpy(h.entry.data()+1,&displacement,4);out=h;return true;
}
template<class R>bool ProveOwnedHook(R& r,std::uintptr_t module,const OwnedHook& h){
    if(!h.module||module!=h.module||!h.target||!h.trampoline||!h.relay||!h.detour||h.bytes<5||h.bytes>h.entry.size())return false;
    std::array<unsigned char,512> entry{};std::array<unsigned char,19> continuation{};std::array<unsigned char,14> relay{};
    return r.ReadBytes(h.target,entry.data(),h.bytes)&&!std::memcmp(entry.data(),h.entry.data(),h.bytes)&&
        r.Read(h.trampoline,continuation)&&continuation==h.continuation&&r.Read(h.relay,relay)&&relay==h.jump;
}
}
