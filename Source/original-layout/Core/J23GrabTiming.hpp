#pragma once
#include <array>
#include <cstdint>
namespace cfggrabtiming {
struct Sample {std::uint64_t allowedMs=0,availableMs=0;unsigned known=0,slot=7;};
struct Report {
    std::uint64_t holds=0,allowed=0,available=0,allowedTotalMs=0,availableTotalMs=0,allowedMaxMs=0,availableMaxMs=0;
    std::array<std::uint64_t,4> frozenEntered{},frozenReturned{},frozenUncertain{};
    std::array<Sample,16> samples{};unsigned cursor=0,count=0;
};
inline bool Valid(const Report& r){
    if(r.count>r.samples.size()||r.cursor>=r.samples.size()||r.allowed>r.holds||r.available>r.holds||
       r.allowedMaxMs>r.allowedTotalMs||r.availableMaxMs>r.availableTotalMs)return false;
    for(unsigned i=0;i<r.samples.size();++i)if(r.samples[i].known>3||r.samples[i].slot>7)return false;
    for(unsigned i=0;i<4;++i)if(r.frozenReturned[i]>r.frozenEntered[i]||r.frozenUncertain[i]>r.frozenEntered[i]||
        r.frozenReturned[i]>r.frozenEntered[i]-r.frozenUncertain[i])return false;
    return true;
}
class Timing {
    std::uint64_t start_=0;unsigned index_=0;bool active_=false;
public:
    Report report{};
    void End(){active_=false;}
    void Begin(std::uint64_t now){start_=now;active_=true;index_=report.cursor;report.samples[index_]={};
        report.cursor=(report.cursor+1)%16;if(report.count<16)++report.count;++report.holds;}
    void Allowed(std::uint64_t now){if(!active_||now<start_)return;auto& s=report.samples[index_];if(s.known&1)return;
        s.known|=1;s.allowedMs=now-start_;++report.allowed;report.allowedTotalMs+=s.allowedMs;
        if(s.allowedMs>report.allowedMaxMs)report.allowedMaxMs=s.allowedMs;}
    void Available(std::uint64_t now,unsigned slot){if(!active_||now<start_||slot>=7)return;auto& s=report.samples[index_];if(s.known&2)return;
        s.known|=2;s.availableMs=now-start_;s.slot=slot;++report.available;report.availableTotalMs+=s.availableMs;
        if(s.availableMs>report.availableMaxMs)report.availableMaxMs=s.availableMs;}
    void Frozen(unsigned direction,bool entered,bool returned,bool uncertain){if(direction>=4||!entered)return;
        ++report.frozenEntered[direction];report.frozenReturned[direction]+=returned;report.frozenUncertain[direction]+=uncertain;}
};
}
