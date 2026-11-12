#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>

using Dispatch=void(__fastcall*)(void*,void*,void*);
static unsigned invocations=0;
static bool nativeFault=false;
static void NativeStubEntered(){++invocations;if(nativeFault)RaiseException(0xE0473501,0,0,nullptr);}
static void __fastcall DispatchStub(void*,void*,void*) { NativeStubEntered(); }
static bool __fastcall BoolStub(void*) { NativeStubEntered(); return true; }
static void __fastcall InputStub(void*) { NativeStubEntered(); }
static void __fastcall CrowdStub(void*,std::uint8_t) { NativeStubEntered(); }
static Dispatch originalActorDispatcher=DispatchStub;
static struct {std::uintptr_t module=0;DWORD threadId=GetCurrentThreadId();} plan;
namespace cfgbundle {constexpr unsigned CallCap=12;}
namespace cfgspawn {const auto GenericDispatcherRva=reinterpret_cast<std::uintptr_t>(&DispatchStub);}
namespace cfghunter {namespace disk {
const auto Press=reinterpret_cast<std::uintptr_t>(&InputStub),Release=Press;
}}
namespace cfgcrowd {const auto Setter=reinterpret_cast<std::uintptr_t>(&CrowdStub);constexpr std::uint8_t DisabledState=0;}
struct Reader {bool eligible=true;unsigned checks=0;};
namespace cfgprimary {bool LookupSafe(Reader& r,std::uintptr_t){++r.checks;return r.eligible;}}
static struct {struct {std::uintptr_t object=0x10000;}pawn;}bundleJason;
static struct {
    struct {struct {std::uintptr_t object=0x20000;}pawn;}actor;
    struct {bool uncertain=false;}journal;
}bundleAdditional;
struct Access {
    Reader r_{};unsigned calls_=0;bool exception_=false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    bool inputs=true,revoke=false,outputs=true,revokeOutputs=false,uncertain=false;unsigned roots=0,results=0;
    void BundleRetentionFailed(){uncertain=true;}
    bool BundleRetentionEnvironment(){return !uncertain&&GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&cfgprimary::LookupSafe(r_,plan.module);}
    bool RetainBundleActors(std::uintptr_t){++roots;if(revoke)r_.eligible=false;return inputs;}
    bool RetainBundleInvocation(std::uintptr_t actor,std::uintptr_t,void*){return RetainBundleActors(actor);}
    bool RetainBundleResults(std::uintptr_t,void*){++results;if(revokeOutputs)r_.eligible=false;return outputs;}
    bool BeginBundleNative(unsigned=0){return inputs;}
    bool EnterBundleInvocation(){return BeginBundleNative();}
    bool FinishBundleNative(){return outputs;}
#endif
#include "BundleNativeCallRuntimeMembers.hpp"
#include "AdditionalNativeCallRuntimeMembers.hpp"
    bool Call(unsigned operation,bool& output) {
        const auto query=reinterpret_cast<std::uintptr_t>(&BoolStub);
        switch(operation) {
        case 0:return Invoke(0x10000,0x30000,nullptr);
        case 1:return Invoke(0x10000,0x30000,nullptr,false);
        case 2:return NativeBool(query,output);
        case 3:return Input(false);
        case 4:return Input(true);
        case 5:return AdditionalInvoke(0x20000,0x30000,nullptr);
        case 6:return AdditionalInvoke(0x20000,0x30000,nullptr,false);
        case 7:return AdditionalBool(query,output);
        case 8:return AdditionalInput(false);
        case 9:return AdditionalInput(true);
        default:return AdditionalCrowdCall(0x40000);
        }
    }
};
#define CHECK(expression) do{if(!(expression)){std::printf("FAIL line %d: %s\n",__LINE__,#expression);return 1;}}while(false)
int main() {
    for(unsigned operation=0;operation<11;++operation) {
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        {Access faulted;bool output=false;invocations=0;nativeFault=true;
         CHECK(!faulted.Call(operation,output)&&invocations==1&&faulted.calls_==1&&faulted.uncertain);
         nativeFault=false;
         CHECK(!faulted.Call(operation,output)&&invocations==1&&faulted.calls_==1);
         CHECK(!faulted.NativeCallAllowed());bundleAdditional.journal.uncertain=false;}
#endif
        // Refuse at the final boundary, even after a caller's earlier check
        // succeeded and eligibility changed before dispatch.
        for(unsigned fault=0;fault<4;++fault) {
            Access a;bool output=false;invocations=0;
            CHECK(a.NativeCallAllowed());a.r_.checks=0;
            if(fault==0)a.r_.eligible=false;
            if(fault==1)plan.threadId=GetCurrentThreadId()+1;
            if(fault==2)originalActorDispatcher=nullptr;
            if(fault==3)a.calls_=cfgbundle::CallCap;
            const auto before=a.calls_;
            CHECK(!a.Call(operation,output));
            if(operation<2)CHECK(a.bundleInvocationStage_==1);
            CHECK(!invocations&&!output&&a.calls_==before&&!a.exception_&&!bundleAdditional.journal.uncertain);
            CHECK(a.r_.checks==(fault==0?1u:0u));
            plan.threadId=GetCurrentThreadId();originalActorDispatcher=DispatchStub;
        }
        Access a;bool output=false;invocations=0;a.calls_=cfgbundle::CallCap-1;
        CHECK(a.Call(operation,output)&&invocations==1&&a.calls_==cfgbundle::CallCap);
        if(operation<2)CHECK(a.bundleInvocationStage_==8);
        CHECK(!a.Call(operation,output)&&invocations==1);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const bool reflected=operation==0||operation==1||operation==5||operation==6;
        CHECK(a.r_.checks==(reflected?3u:2u));
#else
        CHECK(a.r_.checks==1);
#endif
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        for(unsigned lost=0;lost<2;++lost){Access guarded;bool result=false;invocations=0;
            guarded.inputs=lost!=0;guarded.revoke=lost!=0;
            CHECK(!guarded.Call(operation,result)&&!invocations&&!guarded.calls_&&!result&&guarded.roots==1);
            if(operation<2)CHECK(guarded.bundleInvocationStage_==(lost?3u:2u));
        }
        if(reflected)for(unsigned lost=0;lost<2;++lost){Access guarded;bool result=false;invocations=0;
            guarded.outputs=lost!=0;guarded.revokeOutputs=lost!=0;bundleAdditional.journal.uncertain=false;
            CHECK(!guarded.Call(operation,result)&&invocations==1&&guarded.calls_==1&&guarded.results==1);
            if(operation<2)CHECK(guarded.bundleInvocationStage_==(lost?7u:6u));
            CHECK(guarded.uncertain);
            guarded.r_.eligible=true;guarded.outputs=true;guarded.revokeOutputs=false;
            CHECK(!guarded.Call(operation,result)&&!guarded.NativeCallAllowed()&&invocations==1&&guarded.calls_==1);
            CHECK(bundleAdditional.journal.uncertain==(operation==5||operation==6));
            bundleAdditional.journal.uncertain=false;
        }
#endif
    }
    std::puts("PASS 11 native entry variants reject stale eligibility, wrong thread, missing dispatcher and exhausted budget; permitted calls consume exactly one slot");
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    std::puts("PASS native SEH fault in all 11 gateways poisons retained admission; no subsequent native entry or replay");
#endif
    return 0;
}
