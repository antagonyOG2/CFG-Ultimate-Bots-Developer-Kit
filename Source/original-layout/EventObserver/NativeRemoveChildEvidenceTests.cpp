#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include <memory>
int main(){unsigned failures=0;using namespace cfgmenu;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<8;++fault){auto f=std::make_unique<NativeFixture>();OptionsReader reader{f->fixture.r};
        auto& r=f->fixture.r;const auto fn=f->removeFunction;
        // Recorded live schema, independent of RuntimeFlags(): first return
        // Boolean8/1/0780, then Content0/8/0080280; ParmsSize9, storage16.
        r.Set(fn+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000780ULL});
        r.Set(fn+0x200+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040080280ULL});
        if(fault==1)r.Set(fn+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0010000000000580ULL});
        if(fault==2)r.Set(fn+0x200+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000280ULL});
        if(fault==3)r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{16});
        if(fault==4)r.Set(fn+cfgreflection::disk::StorageBytes,9);
        if(fault==5)r.Set(fn+0x100+cfgreflection::disk::PropertyOffset,0);
        if(fault==6)r.Set(fn+0x68,fn+0x100);
        if(fault==7)r.Set(fn+0xB0,MenuFixture::Base+execdisk::RemoveChild.executor+8);
        capture::FunctionEvidence evidence;
        check(native::CachedFunction(reader,MenuFixture::Base,execdisk::RemoveChild,execdisk::RemoveChildParameters,
            execdisk::RemoveChildCode,evidence)==(fault==0),"live recorded RemoveChild accepts; old flags/ABI/cleanup/executor refuse");
    }
    std::printf("CFG captured RemoveChild schema regression: %s (8 cases,%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
