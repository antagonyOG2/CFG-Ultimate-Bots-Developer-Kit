#pragma once
#include <array>
#include <cstdint>
#include "ClassIdentity.hpp"
namespace cfgretainframe {
enum class Kind { Unsupported,Value,Object };
enum class Phase { Inputs,Outputs,OutputCapacity };
struct Property {
    std::uintptr_t next=0;std::uint64_t flags=0;
    unsigned offset=0,bytes=0,dimension=0;Kind kind=Kind::Unsupported;
    bool operator==(const Property& b)const noexcept{return next==b.next&&flags==b.flags&&offset==b.offset&&
        bytes==b.bytes&&dimension==b.dimension&&kind==b.kind;}
};
// Read-only compatibility inspection, never call authorization. Host metadata
// must describe the exact retained UFunction. Unknown input types fail closed;
// do not reinterpret arbitrary scalar/structure bytes as UObject pointers.
template<class Host>bool Observe(Host& host,std::uintptr_t function,std::uintptr_t packet,
    std::array<std::uintptr_t,32>& roots,Phase phase=Phase::Inputs,unsigned* maximumOutputRoots=nullptr) {
    if(maximumOutputRoots)*maximumOutputRoots=0;
    roots={};unsigned frame=0;std::uintptr_t head=0;
    if(phase!=Phase::Inputs&&phase!=Phase::Outputs&&phase!=Phase::OutputCapacity)return false;
    if(!host.Frame(function,frame,head)||frame>4096||(!packet&&frame&&phase!=Phase::OutputCapacity))return false;
    std::array<std::uintptr_t,64> seen{};unsigned count=0,used=0,maximum=0;
    auto child=head;
    while(child){
        if(count==seen.size())return false;
        for(unsigned i=0;i<count;++i)if(seen[i]==child)return false;
        seen[count++]=child;Property p{},again{};
        if(!host.PropertyAt(child,function,p))return false;
        if(p.flags&0x80u){
            if(!p.dimension||p.dimension>32||!p.bytes||p.offset>frame||p.bytes>(frame-p.offset)/p.dimension)return false;
            // Never read an uninitialized pure output before dispatch. Observe
            // output/ref+out objects immediately AFTER dispatch, before another
            // allocating/native call. Unknown output containers also refuse.
            const bool output=(p.flags&0x100u)&&!(p.flags&0x08000000u);
            const bool selected=phase==Phase::Inputs?!output:(p.flags&(0x100u|0x400u))!=0;
            if(selected){
                if(p.kind==Kind::Unsupported)return false;
                if(p.kind==Kind::Object){
                    if(p.bytes!=sizeof(std::uintptr_t))return false;
                    if(phase==Phase::OutputCapacity){
                        // Count worst-case distinct outputs BEFORE side effects.
                        // Never read a return slot that the engine has not set.
                        if(p.dimension>roots.size()-maximum)return false;
                        maximum+=p.dimension;
                    }else for(unsigned i=0;i<p.dimension;++i){std::uintptr_t value=0;
                        if(!host.PointerAt(packet+p.offset+i*p.bytes,value))return false;
                        if(!value)continue;if(!cfgprobe::ObjectPointer(value))return false;
                        bool duplicate=false;for(unsigned j=0;j<used;++j)duplicate|=roots[j]==value;
                        if(!duplicate){if(used==roots.size())return false;roots[used++]=value;}
                    }
                }
            }
        }
        if(!host.PropertyAt(child,function,again)||!(p==again))return false;
        child=p.next;
    }
    unsigned afterBytes=0;std::uintptr_t afterHead=0;
    if(!host.Frame(function,afterBytes,afterHead)||frame!=afterBytes||head!=afterHead)return false;
    if(maximumOutputRoots)*maximumOutputRoots=maximum;
    return true;
}
template<class Host>bool MaximumOutputRoots(Host& host,std::uintptr_t function,unsigned& maximum){
    std::array<std::uintptr_t,32> unused{};
    return Observe(host,function,0,unused,Phase::OutputCapacity,&maximum);
}
}
