#include "UrinationNativeHooks.hpp"
#include <array>
#include <thread>
#include <cstdio>
namespace {
unsigned failures=0,ctorCalls=0,delegateCalls=0,destroyCalls=0;
bool nativeReturn=false,nativeFault=false,destroyObserved=false,requireDestroy=false;
std::uintptr_t lastAction=0,lastPath=0,lastLatent=0,lastDelegate=0,lastValue=0;unsigned lastFlags=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
void* __fastcall Constructor(void* a,void* b,void* c){++ctorCalls;lastAction=reinterpret_cast<std::uintptr_t>(a);
    lastPath=reinterpret_cast<std::uintptr_t>(b);lastLatent=reinterpret_cast<std::uintptr_t>(c);
    if(nativeFault)RaiseException(0xE0000011,0,0,nullptr);return a;}
bool __fastcall Delegate(void* a,void* b){++delegateCalls;lastDelegate=reinterpret_cast<std::uintptr_t>(a);lastValue=reinterpret_cast<std::uintptr_t>(b);return nativeReturn;}
void* __fastcall Destructor(void* a,unsigned flags){++destroyCalls;lastFlags=flags;
    if(requireDestroy)CHECK(destroyObserved);return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(a)+8);}
template<unsigned Tag>struct Host {
    bool pinned=true,requesting=true,fault=false,reenter=false;
    unsigned constructs=0,completes=0,destroys=0,faults=0;
    std::uintptr_t ctorPc=0,delegatePc=0,delegateHook=0;
    bool PinnedBuild(std::uintptr_t module){return pinned&&module==0x100000;}
    bool Requesting(){return requesting;}
    void Constructed(std::uintptr_t a,std::uintptr_t returned,std::uintptr_t path,std::uintptr_t latent,std::uintptr_t pc){
        ++constructs;ctorPc=pc;CHECK(a==returned&&a==lastAction&&path==lastPath&&latent==lastLatent);
        if(fault)RaiseException(0xE0000012,0,0,nullptr);
    }
    void Completing(std::uintptr_t delegate,std::uintptr_t value,std::uintptr_t pc){++completes;delegatePc=pc;
        if(reenter){reenter=false;reinterpret_cast<bool(__fastcall*)(void*,void*)>(delegateHook)(reinterpret_cast<void*>(delegate),reinterpret_cast<void*>(value));}
        if(fault)RaiseException(0xE0000013,0,0,nullptr);
    }
    void Destroying(std::uintptr_t){++destroys;destroyObserved=true;}
    void ObservationFault(){++faults;}
};
struct Api {
    unsigned codes=0,creates=0,enables=0,owned=0,failCode=0,failCreate=0,failEnable=0,failOwned=0;
    std::array<std::uintptr_t,3> hooks{};
    template<std::size_t N>bool Code(std::uintptr_t,const std::array<unsigned char,N>&){return ++codes!=failCode;}
    // Forwarding fixture supplies no actual engine hook proof. Separate byte
    // tests exercise PrepareOwnedHook; Ready alone cannot pass runtime Proof.
    template<std::size_t N>bool PrepareOwned(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,
        const std::array<unsigned char,N>&,cfgurination::OwnedHook& out){out={};return ++owned!=failOwned;}
    bool Create(std::uintptr_t target,std::uintptr_t hook,void*& original){const auto index=creates++;if(creates==failCreate)return false;
        hooks[index]=hook;
        if(target==0x100000+cfgurination::disk::ActionConstructor)original=reinterpret_cast<void*>(&Constructor);
        else if(target==0x100000+cfgurination::disk::LoadedDelegate)original=reinterpret_cast<void*>(&Delegate);
        else if(target==0x100000+cfgurination::disk::ActionDestructor)original=reinterpret_cast<void*>(&Destructor);
        else{CHECK(false);return false;}return true;
    }
    bool Enable(std::uintptr_t){return ++enables!=failEnable;}
};
using Ctor=void*(__fastcall*)(void*,void*,void*);using Load=bool(__fastcall*)(void*,void*);using Destroy=void*(__fastcall*)(void*,unsigned);
bool CallNativeFault(Ctor fn){__try{fn(reinterpret_cast<void*>(0x200000),reinterpret_cast<void*>(0x300000),reinterpret_cast<void*>(0x400000));}
    __except(EXCEPTION_EXECUTE_HANDLER){return true;}return false;}
template<unsigned Tag>void FailureCase(unsigned code,unsigned create,unsigned enable,unsigned owned=0){Host<Tag> host;Api api;
    api.failCode=code;api.failCreate=create;api.failEnable=enable;api.failOwned=owned;
    using Hooks=cfgurination::NativeHooks<Host<Tag>>;
    CHECK(!Hooks::Install(host,api,0x100000,GetCurrentThreadId())&&!Hooks::Ready());
    if(code)CHECK(api.creates==0&&api.enables==0);
    if(create)CHECK(api.enables==0);
    if(owned)CHECK(api.enables==0);
    if(enable){const auto before=ctorCalls;auto ctor=reinterpret_cast<Ctor>(api.hooks[0]);
        CHECK(ctor(reinterpret_cast<void*>(0x200000),nullptr,nullptr)==reinterpret_cast<void*>(0x200000));
        CHECK(ctorCalls==before+1&&host.constructs==0);}
    if(create||enable||owned){const auto count=api.creates;CHECK(!Hooks::Install(host,api,0x100000,GetCurrentThreadId()));CHECK(api.creates==count);}
}
struct RejectReader {template<class T>bool Read(std::uintptr_t,T&){return false;}bool ReadBytes(std::uintptr_t,void*,std::size_t){return false;}};
}
int main(){
    Host<0> host;Api api;using Hooks=cfgurination::NativeHooks<Host<0>>;
    CHECK(Hooks::Install(host,api,0x100000,GetCurrentThreadId())&&Hooks::Ready()&&api.codes==3&&api.creates==3&&api.enables==3&&api.owned==3);
    CHECK(Hooks::Install(host,api,0x100000,GetCurrentThreadId())&&api.creates==3);
    CHECK(!Hooks::Install(host,api,0x110000,GetCurrentThreadId()));
    RejectReader noProof;CHECK(!Hooks::Proof(noProof,0x100000)); // Ready is not native-boundary authority
    const auto ctor=reinterpret_cast<Ctor>(api.hooks[0]);const auto delegate=reinterpret_cast<Load>(api.hooks[1]);const auto destroy=reinterpret_cast<Destroy>(api.hooks[2]);host.delegateHook=api.hooks[1];
    auto a=reinterpret_cast<void*>(0x200000),b=reinterpret_cast<void*>(0x300000),c=reinterpret_cast<void*>(0x400000);
    CHECK(ctor(a,b,c)==a&&ctorCalls==1&&host.constructs==1&&host.ctorPc!=0);
    host.requesting=false;CHECK(ctor(a,b,c)==a&&ctorCalls==2&&host.constructs==1);host.requesting=true;
    nativeReturn=false;CHECK(!delegate(a,b)&&delegateCalls==1&&host.completes==1&&lastDelegate==0x200000&&lastValue==0x300000);
    nativeReturn=true;CHECK(delegate(a,b)&&delegateCalls==2&&host.completes==2&&host.delegatePc!=0);
    host.reenter=true;const auto before=delegateCalls;CHECK(delegate(a,b));CHECK(delegateCalls==before+2&&host.completes==3); // each native invocation forwards once; nested observation suppressed
    host.fault=true;CHECK(ctor(a,b,c)==a&&host.faults==1);CHECK(delegate(a,b)&&host.faults==2);host.fault=false;
    nativeFault=true;const auto observed=host.constructs;CHECK(CallNativeFault(ctor)&&host.constructs==observed);nativeFault=false;
    destroyObserved=false;requireDestroy=true;CHECK(destroy(a,7)==reinterpret_cast<void*>(0x200008)&&host.destroys==1&&lastFlags==7);requireDestroy=false;
    const auto oldConstructs=host.constructs,oldCompletes=host.completes,oldDestroys=host.destroys;
    std::thread foreign([&]{CHECK(ctor(a,b,c)==a);CHECK(delegate(a,b));CHECK(destroy(a,3)==reinterpret_cast<void*>(0x200008));});foreign.join();
    CHECK(host.constructs==oldConstructs&&host.completes==oldCompletes&&host.destroys==oldDestroys);
    FailureCase<1>(1,0,0);FailureCase<2>(2,0,0);FailureCase<3>(3,0,0);
    FailureCase<4>(0,1,0);FailureCase<5>(0,2,0);FailureCase<6>(0,3,0);
    FailureCase<7>(0,0,1);FailureCase<8>(0,0,2);FailureCase<9>(0,0,3);
    FailureCase<11>(0,0,0,1);FailureCase<12>(0,0,0,2);FailureCase<13>(0,0,0,3);
    {Host<10> invalid;Api invalidApi;invalid.pinned=false;CHECK(!cfgurination::NativeHooks<Host<10>>::Install(invalid,invalidApi,0x100000,GetCurrentThreadId())&&invalidApi.codes==0);}
    std::printf("Actual urination native hook adapters: %u failures; original arguments/results/SEH and once-forwarding, no foreign-thread observation, reentry/fault cleanup, destruction-before-free,3body preflight and partial-install no-replay. API stubs only; no game/hooks installed.\n",failures);
    return failures?1:0;
}
