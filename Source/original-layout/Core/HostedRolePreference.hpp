#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace cfghostedrole {
constexpr std::uint8_t CounselorPreference=1,KillerPreference=2;
constexpr bool RejectNonHostWinner(bool roleSelectionEnabled,bool hostJasonRequested,bool winnerIsHost)noexcept{
    return roleSelectionEnabled&&hostJasonRequested&&!winnerIsHost;
}
template<std::size_t N>
bool InvertHostPreference(const std::array<std::uint8_t,N>& before,const std::array<std::uint8_t,N>& human,
    unsigned count,unsigned host,
    std::array<std::uint8_t,N>& after)noexcept{
    after={};
    if(count<2||count>N||host>=count)return false;
    unsigned remote=0;
    for(unsigned i=0;i<count;++i){
        if(!human[i])continue;
        if(before[i]>KillerPreference)return false;
        if(i==host)after[i]=KillerPreference;
        else{after[i]=CounselorPreference;++remote;}
    }
    return human[host]!=0&&remote>0;
}
}

// ---- H2.45 shared layout: one implementation, per-image offsets and byte pins as parameters. ----
#ifndef CFG_H245_HOST_JASON
#define CFG_H245_HOST_JASON 1 // H2.48: on. Local-host takeover now exists (CFG_H247_HOST_TAKEOVER) with pinned local view natives; 3.5-correct host-as-Jason lottery bridge (pins + offsets below)
#endif
#ifndef CFG_H245_HOST_ADOPTION
#define CFG_H245_HOST_ADOPTION 1 // H2.48: on. Local-winner adoption (HostedJasonTakeover Reason::HostSelected path) is enabled
#endif
namespace cfghostedrole {
constexpr unsigned PinPrologue=1,PinPrefOne=2,PinPrefTwo=4,PinPrior=8,PinAll=15;
// RefusalCode values in the report. 1-9 are the pre-existing preflight/apply codes.
constexpr unsigned RefusePinMismatch=10,RefuseOffline=12,RefuseAdoptionUnproven=14;
struct Witness{std::uintptr_t rva;const unsigned char* bytes;std::size_t size;unsigned bit;};
struct Layout{
    std::uintptr_t entry;const unsigned char* prologue;std::size_t prologueSize;
    std::array<Witness,3> witness;unsigned required;
    std::uintptr_t preference,priorName; // PlayerState role byte; GameInstance prior-Jason FString data (count at +8)
};
// Pure pin check: read(rva,out,n) fills n image bytes. Returns the mask of pins that match byte-for-byte.
template<class Read> unsigned VerifyPins(const Layout& l,Read&& read)noexcept{
    unsigned mask=0;unsigned char buf[96]{};
    if(l.prologueSize&&l.prologueSize<=sizeof(buf)&&read(l.entry,buf,l.prologueSize)){
        bool same=true;for(std::size_t i=0;i<l.prologueSize;++i)if(buf[i]!=l.prologue[i])same=false;
        if(same)mask|=PinPrologue;}
    for(const auto& w:l.witness){
        if(!w.bytes||!w.size||w.size>sizeof(buf)||!read(w.rva,buf,w.size))continue;
        bool same=true;for(std::size_t i=0;i<w.size;++i)if(buf[i]!=w.bytes[i])same=false;
        if(same)mask|=w.bit;}
    return mask;
}
constexpr bool PinsAccepted(const Layout& l,unsigned mask)noexcept{return l.required!=0&&(mask&l.required)==l.required;}
// Single decision point for any write: pins, hosted world, adoption gate. Returns 0 to allow, else a refusal code.
constexpr unsigned WriteDecision(const Layout& l,unsigned pinMask,bool netDriverNonNull,bool writesEnabled)noexcept{
    return !PinsAccepted(l,pinMask)?RefusePinMismatch:!netDriverNonNull?RefuseOffline:!writesEnabled?RefuseAdoptionUnproven:0u;
}
// The only writes the donor bridge makes: role byte per human, prior-Jason length zeroed. Returns writes made.
template<class W8,class W32>
unsigned WriteRoles(const Layout& l,const std::uintptr_t* state,const std::uint8_t* after,unsigned count,
    std::uintptr_t gameInstance,W8&& write8,W32&& write32)noexcept{
    unsigned n=0;
    for(unsigned i=0;i<count;++i)if(after[i]){write8(state[i]+l.preference,after[i]);++n;}
    write32(gameInstance+l.priorName+8,0);return n+1;
}
// Donor rc1 (base game) image: entry signature only; offsets proven by the donor source.
inline constexpr unsigned char DonorPrologue[17]{0x48,0x89,0x4C,0x24,0x08,0x55,0x53,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57};
inline constexpr Layout DonorLayout{0x382110,DonorPrologue,sizeof(DonorPrologue),{},PinPrologue,0x868,0x2F0};
// CFG 3.5 (SummerCamp-Win64-Shipping.exe 4FC4BB54...), H244 proof: lottery 0x3DEAE0, witnesses 0x3DF2DA/0x3DF556/0x3DF171 (proof cites 0x3DF17B = the +0x310 load, 10 bytes into this pin; verified byte-for-byte against the image).
inline constexpr unsigned char Cfg35Prologue[90]{
    0x48,0x89,0x4C,0x24,0x08,0x55,0x53,0x56,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x8D,0xAC,0x24,0x00,0xFD,0xFF,0xFF,
    0x48,0x81,0xEC,0x00,0x04,0x00,0x00,0x48,0x8B,0x01,0x45,0x33,0xE4,0x4C,0x8B,0xF9,0x44,0x89,0xA5,0x48,0x03,0x00,0x00,0x41,
    0x8B,0xF4,0xFF,0x90,0x38,0x01,0x00,0x00,0x49,0x8B,0x07,0x49,0x8B,0xCF,0x4C,0x89,0x64,0x24,0x50,0xFF,0x90,0x38,0x01,0x00,
    0x00,0x4C,0x8B,0xA8,0x40,0x01,0x00,0x00,0x4C,0x89,0x6D,0x88,0x4D,0x85,0xED,0x74,0x2C,0xE8};
inline constexpr unsigned char Cfg35PrefOne[7]{0x80,0xBF,0x48,0x09,0x00,0x00,0x01};                // cmp byte [rdi+0x948],1
inline constexpr unsigned char Cfg35PrefTwo[9]{0x0F,0xB6,0x83,0x48,0x09,0x00,0x00,0x3C,0x02};     // movzx eax,[rbx+0x948]; cmp al,2
inline constexpr unsigned char Cfg35Prior[17]{0x41,0x83,0xBD,0x18,0x03,0x00,0x00,0x00,0x74,0x09,0x49,0x8B,0x95,0x10,0x03,0x00,0x00}; // [r13+0x318]==0 ; [r13+0x310]
inline constexpr Layout Cfg35Layout{0x3DEAE0,Cfg35Prologue,sizeof(Cfg35Prologue),
    {{{0x3DF2DA,Cfg35PrefOne,sizeof(Cfg35PrefOne),PinPrefOne},{0x3DF556,Cfg35PrefTwo,sizeof(Cfg35PrefTwo),PinPrefTwo},
      {0x3DF171,Cfg35Prior,sizeof(Cfg35Prior),PinPrior}}},PinAll,0x948,0x310};
#if CFG_H245_HOST_JASON
inline constexpr const Layout& ActiveLayout=Cfg35Layout;
#else
inline constexpr const Layout& ActiveLayout=DonorLayout;
#endif
constexpr bool WritesEnabledByBuild()noexcept{return !CFG_H245_HOST_JASON||CFG_H245_HOST_ADOPTION;}
}
