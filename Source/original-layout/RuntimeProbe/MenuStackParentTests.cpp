#include "MenuStackParentObservation.hpp"
#include <map>
#include <cstring>
#include <cstdio>
struct ParentReader {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,occurrences=0;std::uintptr_t changeAt=0;
    template<class T>void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    template<class T>bool Read(std::uintptr_t at,T& out){return ReadBytes(at,&out,sizeof(out));}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t count){++reads;
        if(at==changeAt&&++occurrences==2)Set(at,std::uintptr_t{0xA00000});
        auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<count;++i){const auto found=bytes.find(at+i);if(found==bytes.end())return false;p[i]=found->second;}return true;}
};
int main(){using namespace cfgmenu;unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    static_assert(parentdisk::Owner==0x30451D0&&parentdisk::Constructor==0x30451D8,"Verified CFG 3.5 named menu stack");
    static_assert(parentdisk::PreviousExecutor==0x2E5C50&&parentdisk::PreviousHelper==0x2B3D40,"Verified current previous-class path");
    for(unsigned fault=0;fault<8;++fault){ParentReader r;active::Context c;c.stack=0x100000;c.array={0x200000,2,4};
        r.Set(c.stack+execdisk::StackArray,c.array);r.Set(c.array.data,std::uintptr_t{0x300000});
        r.Set(c.array.data+execdisk::RecordWidget,std::uintptr_t{});
        if(fault==1)r.Set(c.array.data+execdisk::RecordWidget,std::uintptr_t{0x400000});
        if(fault==2)r.Set(c.array.data,std::uintptr_t{});
        if(fault==3)r.Set(c.array.data+execdisk::RecordWidget,std::uintptr_t{1});
        if(fault==4)r.Set(c.stack+execdisk::StackArray,active::StackArray{0x200000,3,4});
        if(fault==5)c.array.capacity=65;
        if(fault==6)r.changeAt=c.array.data;
        parentcapture::Record record;
        const bool okay=parentcapture::Observe(r,c,fault==7?1:0,record);
        check(okay==(fault==0||fault==1));
        if(okay){check(record.menuClass==0x300000&&record.index==0);check(r.reads==6);
            check(parentcapture::Current(r,c,record));r.Set(c.array.data,std::uintptr_t{0x500000});check(!parentcapture::Current(r,c,record));}
        else check(!record.menuClass);
    }
    ParentReader code;constexpr std::uintptr_t module=0x600000;
    code.Set(module+parentdisk::PreviousExecutor,parentdisk::PreviousCode);code.Set(module+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
    check(parentcapture::NativeCode(code,module));code.Set(module+parentdisk::PreviousHelper,static_cast<unsigned char>(0));check(!parentcapture::NativeCode(code,module));
    code.Set(module+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
    auto stale=parentdisk::PreviousCode;stale[36]=0x78;stale[37]=0xEC;
    code.Set(module+parentdisk::PreviousExecutor,stale);check(!parentcapture::NativeCode(code,module));
    ParentReader oldOnly;oldOnly.Set(module+0x2E48A0,parentdisk::PreviousCode);oldOnly.Set(module+0x2B3540,parentdisk::PreviousLeaf);
    check(!parentcapture::NativeCode(oldOnly,module)); // Old addresses cannot approve the new image.
    std::printf("Native retained parent class/nullable widget: %s (%u checks; no native calls/creation).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
