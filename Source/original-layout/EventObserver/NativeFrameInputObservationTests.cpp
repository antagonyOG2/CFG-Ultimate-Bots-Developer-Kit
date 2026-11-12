#include "../RuntimeProbe/NativeFrameInputObservation.hpp"
#include <cstdio>
using cfgretainframe::Kind;
struct Host {
    unsigned bytes=32,reads=0,failAt=0,pointerReads=0;bool mutate=false;
    std::uintptr_t head=0x20000;
    std::array<cfgretainframe::Property,4> fields{{
        {0x20008,0x80,0,8,1,Kind::Object},
        {0x20010,0x80,8,4,1,Kind::Value},
        {0,0x580,16,8,1,Kind::Object},{}}};
    bool Step(){return ++reads!=failAt;}
    bool Frame(std::uintptr_t,unsigned& size,std::uintptr_t& first){size=bytes;first=head;return Step();}
    bool PropertyAt(std::uintptr_t at,std::uintptr_t,cfgretainframe::Property& out){
        if(!Step()||at<0x20000||(at-0x20000)%8||(at-0x20000)/8>=fields.size())return false;
        out=fields[(at-0x20000)/8];if(mutate&&reads==4)++out.offset;return true;
    }
    bool PointerAt(std::uintptr_t at,std::uintptr_t& out){++pointerReads;out=at==0x50010?0x60000:0x40000;return Step();}
};
int main(){unsigned failures=0;
    const auto check=[&](bool value){if(!value)++failures;};
    std::array<std::uintptr_t,32> roots{};Host good;
    check(cfgretainframe::Observe(good,0x10000,0x50000,roots)&&roots[0]==0x40000&&!roots[1]);
    for(unsigned fault=1;fault<=good.reads;++fault){Host h;h.failAt=fault;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.fields[0].kind=Kind::Unsupported;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.fields[0].bytes=4;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.fields[0].dimension=33;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.fields[0].offset=33;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.fields[2].next=h.head;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;h.mutate=true;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;check(!cfgretainframe::Observe(h,0x10000,0,roots));}
    {Host h;h.bytes=0;h.head=0;check(cfgretainframe::Observe(h,0x10000,0,roots)&&!roots[0]);}
    {Host h;h.fields[0].flags=0x180;h.fields[0].kind=Kind::Unsupported;
     check(cfgretainframe::Observe(h,0x10000,0x50000,roots)&&!roots[0]);
     h.reads=0;h.fields[0].flags|=0x08000000;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots));}
    {Host h;check(cfgretainframe::Observe(h,0x10000,0x50000,roots,cfgretainframe::Phase::Outputs)&&roots[0]==0x60000&&!roots[1]);
     for(unsigned fault=1;fault<=h.reads;++fault){Host f;f.failAt=fault;
      check(!cfgretainframe::Observe(f,0x10000,0x50000,roots,cfgretainframe::Phase::Outputs));}}
    {Host h;h.fields[0].flags=0x08000180;
     check(cfgretainframe::Observe(h,0x10000,0x50000,roots,cfgretainframe::Phase::Outputs)&&roots[0]==0x40000&&roots[1]==0x60000&&!roots[2]);}
    {Host h;h.fields[2].kind=Kind::Unsupported;
     check(!cfgretainframe::Observe(h,0x10000,0x50000,roots,cfgretainframe::Phase::Outputs));}
    {Host h;unsigned maximum=99;
     check(cfgretainframe::MaximumOutputRoots(h,0x10000,maximum)&&maximum==1&&!h.pointerReads);
     for(unsigned fault=1;fault<=h.reads;++fault){Host f;f.failAt=fault;maximum=99;
        check(!cfgretainframe::MaximumOutputRoots(f,0x10000,maximum)&&!maximum&&!f.pointerReads);}}
    {Host h;h.bytes=272;h.fields[2].dimension=32;unsigned maximum=99;
     check(cfgretainframe::MaximumOutputRoots(h,0x10000,maximum)&&maximum==32&&!h.pointerReads);
     h.fields[0].flags=0x580;maximum=99;
     check(!cfgretainframe::MaximumOutputRoots(h,0x10000,maximum)&&!maximum&&!h.pointerReads);}
    {Host h;h.fields[2].kind=Kind::Unsupported;unsigned maximum=99;
     check(!cfgretainframe::MaximumOutputRoots(h,0x10000,maximum)&&!maximum&&!h.pointerReads);}
    {Host h;h.mutate=true;unsigned maximum=99;
     check(!cfgretainframe::MaximumOutputRoots(h,0x10000,maximum)&&!maximum&&!h.pointerReads);}
    {Host h;check(!cfgretainframe::Observe(h,0x10000,0x50000,roots,static_cast<cfgretainframe::Phase>(99))&&!h.reads);}
    std::printf("Native frame typed input/output/ref/cycle/extent/read/mutation checks: %u failures; no native calls\n",failures);
    return failures?1:0;
}
