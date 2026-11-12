#include "UrinationOwnedHookObservation.hpp"
#include "UrinationBindings.generated.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
struct Reader:OwnedByteReader {
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t length){auto* p=static_cast<unsigned char*>(out);
        for(std::size_t i=0;i<length;++i)if(!Read(at+i,p[i]))return false;return true;}
};
template<std::size_t N>void Test(const std::array<unsigned char,N>& code,std::uintptr_t rva){
    using namespace cfgurination;
    constexpr std::uintptr_t module=0x100000,trampoline=0x200000,detour=0x300000;
    const auto target=module+rva;Reader base;base.Set(target,code);
    std::array<unsigned char,19> continuation{};std::memcpy(continuation.data(),code.data(),5);
    const auto back=HookJump(target+5);std::memcpy(continuation.data()+5,back.data(),14);
    base.Set(trampoline,continuation);base.Set(trampoline+19,HookJump(detour));
    OwnedHook proof{};CHECK(PrepareOwnedHook(base,module,target,trampoline,detour,code,proof));
    CHECK(!ProveOwnedHook(base,module,proof)); // not enabled yet
    for(unsigned i=0;i<N;++i)base.Set(target+i,proof.entry[i]);
    CHECK(ProveOwnedHook(base,module,proof));CHECK(!ProveOwnedHook(base,module+8,proof));
    for(unsigned fault=0;fault<4;++fault){Reader r=base;
        if(fault==0)r.Set(target,std::uint8_t{});
        if(fault==1)r.Set(target+N-1,std::uint8_t{});
        if(fault==2)r.Set(trampoline+5,std::uint8_t{});
        if(fault==3)r.Set(trampoline+19,std::uint8_t{});
        CHECK(!ProveOwnedHook(r,module,proof));}
    // A foreign pre-existing detour cannot be adopted during preparation.
    CHECK(!PrepareOwnedHook(base,module,target,trampoline,detour,code,proof)&&!proof.bytes);
}
}
int main(){Test(cfgurination::disk::ActionConstructorCode,cfgurination::disk::ActionConstructor);
    Test(cfgurination::disk::LoadedDelegateCode,cfgurination::disk::LoadedDelegate);
    Test(cfgurination::disk::ActionDestructorCode,cfgurination::disk::ActionDestructor);
    std::printf("Owned urination MinHook relocation/relay/full-body proof: %u failures; exact5 stolen bytes, expected continuation, no foreign-hook adoption or altered native tail. Synthetic bytes only.\n",failures);
    return failures?1:0;
}
