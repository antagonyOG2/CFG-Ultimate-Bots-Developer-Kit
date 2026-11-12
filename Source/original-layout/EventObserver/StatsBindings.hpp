#pragma once
#include "../Core/CounselorStatsPolicy.hpp"
#include "OfflineContextObservation.hpp"
namespace cfgstats {
// Exact CFG native Stats property and GetStatAlpha disk chain, not donor offsets.
constexpr auto StatOffset=cfgcompat::cfg35::toolsStats::Offset,DescriptorRva=cfgcompat::cfg35::toolsStats::DescriptorRva,NameRva=cfgcompat::cfg35::toolsStats::NameRva;
constexpr auto IndexedLoadRva=cfgcompat::cfg35::toolsStats::IndexedLoadRva;
template<class Reader> bool SchemaMatches(Reader& r,std::uintptr_t module) {
    std::array<unsigned char,8> code{}; std::array<char,6> name{};
    std::uint32_t kind=1,dimension=0,offset=0; std::uintptr_t namePointer=0;
    return r.Read(module+IndexedLoadRva,code) && code==std::array<unsigned char,8>{0x0F,0xB6,0x8C,0x28,0xF4,0x13,0,0} &&
        r.Read(module+DescriptorRva,kind) && kind==0 &&
        r.Read(module+DescriptorRva+8,namePointer) && namePointer==module+NameRva &&
        r.Read(namePointer,name) && name==std::array<char,6>{'S','t','a','t','s',0} &&
        r.Read(module+DescriptorRva+32,dimension) && dimension==7 &&
        r.Read(module+DescriptorRva+48,offset) && offset==StatOffset;
}
constexpr std::uint32_t Magic=0x43465354, Version=1;
enum class Command : std::uint32_t { Enable=1, Restore=2 };
enum class State : std::uint32_t { Idle, Applied, Restored, AlreadyMaximum, NoOverride, Refused, Indeterminate };
enum class Reason : std::uint32_t { None, Context, Read, Values, Changed, Region, Write, Readback, Limit };
struct Request {
    std::uint32_t magic=Magic,version=Version,bytes=0,command=0;
    cfgport::stats::Identity identity{};
};
struct Report {
    std::uint32_t magic=Magic,version=Version,bytes=sizeof(Report),generation=0;
    std::uint32_t state=static_cast<std::uint32_t>(State::Idle),reason=0,operations=0,command=0;
    std::uint32_t nativeReads=0,error=0,writes=0,active=0;
    cfgport::stats::Identity identity{};
    cfgport::stats::Values before{},after{};
};
inline bool ValidReport(const Report& r) noexcept {
    if (r.magic!=Magic || r.version!=Version || r.bytes!=sizeof(Report) || (r.generation&1) ||
        r.operations>16 || r.generation!=r.operations*2 || r.nativeReads>256 || r.writes>1 || r.active>1 ||
        !r.identity.Valid() || r.state>static_cast<std::uint32_t>(State::Indeterminate) ||
        r.reason>static_cast<std::uint32_t>(Reason::Limit)) return false;
    if (r.state==static_cast<std::uint32_t>(State::Idle)) return !r.operations && !r.command && !r.writes && !r.active;
    if (!r.operations || (r.command!=1 && r.command!=2)) return false;
    if (r.state==static_cast<std::uint32_t>(State::Applied)) return r.command==1 && r.active && r.writes==1 &&
        r.reason==0 && cfgport::stats::Valid(r.before) && r.after==cfgport::stats::Maximum();
    if (r.state==static_cast<std::uint32_t>(State::Restored)) return r.command==2 && !r.active && r.writes==1 &&
        r.reason==0 && r.before==cfgport::stats::Maximum() && cfgport::stats::Valid(r.after);
    return true; // Other diagnostic states never grant a gameplay operation.
}
inline bool SameReport(const Report& a,const Report& b) noexcept {
    return ValidReport(a) && ValidReport(b) && a.generation==b.generation && a.state==b.state && a.reason==b.reason &&
        a.operations==b.operations && a.command==b.command && a.nativeReads==b.nativeReads && a.error==b.error &&
        a.writes==b.writes && a.active==b.active && a.identity==b.identity && a.before==b.before && a.after==b.after;
}
}
