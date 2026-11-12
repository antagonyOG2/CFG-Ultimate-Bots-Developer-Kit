#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "HostedSpectatorHookCalls.hpp"
#include "../RuntimeProbe/HostedSpectator35.generated.hpp"
#include "OwnedNativeDetour.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include <array>
#include <cstdio>
#include <cstring>

namespace {
unsigned checks=0,failures=0,selectorCalls=0,canCalls=0,decisions=0;
bool forceCanReentry=false;
unsigned admissionOriginalCalls=0,contextFactoryCalls=0,authorizationCalls=0,proofCalls=0;
bool proofAllows=true;
bool ownershipAllows=true;
void Check(bool value,const char* label){++checks;if(!value){++failures;std::printf("FAIL %s\n",label);}}
thread_local bool selectorBusy=false,canBusy=false;
void* nestedResult=reinterpret_cast<void*>(0x3333);
bool CanOriginal(void*,void*,void*);
void* SelectorOriginal(void*,std::int32_t direction){++selectorCalls;
    if(direction==2)return cfgh216hooks::SelectorCall(selectorBusy,&SelectorOriginal,reinterpret_cast<void*>(1),3,
        [](void* stock,std::int32_t){(void)stock;++decisions;return reinterpret_cast<void*>(0x9999);});
    (void)cfgh216hooks::CanSpectateCall(canBusy,&CanOriginal,reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),reinterpret_cast<void*>(3),
        [](bool stock,void*,void*,void*){++decisions;return !stock;});
    return nestedResult;
}
bool CanOriginal(void*,void*,void*){++canCalls;
    if(forceCanReentry&&canCalls==1)return cfgh216hooks::CanSpectateCall(canBusy,&CanOriginal,nullptr,nullptr,nullptr,
        [](bool stock,void*,void*,void*){++decisions;return !stock;});
    return false;
}
struct Memory:cfgownednative::CurrentProcess {
    std::uintptr_t target=0,image=0;
    bool Image(std::uintptr_t at,std::uintptr_t module,std::size_t n)noexcept{return at==target&&module==image&&n==16;}
};
void __fastcall Detour(void*,std::int32_t){}
void* AdmissionOriginal(void*,std::int32_t){++admissionOriginalCalls;return reinterpret_cast<void*>(0x5555);}
}
int main(){
    {selectorCalls=canCalls=decisions=0;forceCanReentry=true;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&SelectorOriginal,
        reinterpret_cast<void*>(1),1,[](void* stock,std::int32_t){(void)stock;++decisions;return reinterpret_cast<void*>(0x4444);});
     Check(result==reinterpret_cast<void*>(0x4444)&&selectorCalls==1&&canCalls==2&&decisions==2,"selector original once, then policy; separate CanSpectate guard admits original nested call");}
    {selectorCalls=canCalls=decisions=0;forceCanReentry=false;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&SelectorOriginal,
        reinterpret_cast<void*>(1),2,[](void* stock,std::int32_t){(void)stock;++decisions;return reinterpret_cast<void*>(0x4444);});
     Check(result==reinterpret_cast<void*>(0x4444)&&selectorCalls==2&&decisions==2,"selector reentry calls original and skips nested policy");}
    {canCalls=decisions=0;forceCanReentry=true;const auto result=cfgh216hooks::CanSpectateCall(canBusy,&CanOriginal,nullptr,nullptr,nullptr,
        [](bool stock,void*,void*,void*){++decisions;return !stock;});
     Check(result&&canCalls==2&&decisions==1,"CanSpectate reentry remains stock-only; outer policy runs once");}
    // This is the exact shared post-original admission function used by the
    // production selector and CanSpectate wrappers. Independent native queries
    // have no BundleCallbackRootScope.
    {const bool bundleRootLease=false;admissionOriginalCalls=contextFactoryCalls=authorizationCalls=proofCalls=decisions=0;proofAllows=true;ownershipAllows=true;
     const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::OwnerThreadAfterOriginal(stock,7,7,
          [&](std::uint32_t current,std::uint32_t owner,void* ownedStock){++contextFactoryCalls;
            return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(ownedStock,true,true,current,owner,true,
                [&]{++authorizationCalls;return ownershipAllows;},
                [&](int& proof){++proofCalls;proof=1;return proofAllows;},
                [&](void*,const int&){++decisions;return reinterpret_cast<void*>(0x6666);});});});
     Check(!bundleRootLease&&result==reinterpret_cast<void*>(0x6666)&&admissionOriginalCalls==1&&contextFactoryCalls==1&&authorizationCalls==1&&proofCalls==1&&decisions==1,
        "current independent callback admits read-only proof without callback root lease");}
    {admissionOriginalCalls=authorizationCalls=proofCalls=0;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(stock,false,true,7,7,true,
            [&]{++authorizationCalls;return true;},
            [&](int&){++proofCalls;return true;},[&](void*,const int&){return reinterpret_cast<void*>(0x6666);});});
     Check(result==reinterpret_cast<void*>(0x5555)&&admissionOriginalCalls==1&&authorizationCalls==0&&proofCalls==0,"retired generation passes stock without observation");}
    {admissionOriginalCalls=contextFactoryCalls=authorizationCalls=proofCalls=0;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::OwnerThreadAfterOriginal(stock,8,7,
            [&](std::uint32_t current,std::uint32_t owner,void* ownedStock){++contextFactoryCalls;
                return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(ownedStock,true,true,current,owner,true,
                    [&]{++authorizationCalls;return true;},
                    [&](int&){++proofCalls;return true;},[&](void*,const int&){return reinterpret_cast<void*>(0x6666);});});});
     Check(result==reinterpret_cast<void*>(0x5555)&&admissionOriginalCalls==1&&contextFactoryCalls==0&&authorizationCalls==0&&proofCalls==0,
        "foreign thread calls original once then skips request/context and observer");}
    {admissionOriginalCalls=authorizationCalls=proofCalls=0;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(stock,true,false,7,7,true,
            [&]{++authorizationCalls;return true;},
            [&](int&){++proofCalls;return true;},[&](void*,const int&){return reinterpret_cast<void*>(0x6666);});});
     Check(result==reinterpret_cast<void*>(0x5555)&&admissionOriginalCalls==1&&authorizationCalls==0&&proofCalls==0,"missing owned hook continuation admission passes stock");}
    {admissionOriginalCalls=authorizationCalls=proofCalls=0;ownershipAllows=false;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(stock,true,true,7,7,true,
            [&]{++authorizationCalls;return ownershipAllows;},
            [&](int&){++proofCalls;return true;},[&](void*,const int&){return reinterpret_cast<void*>(0x6666);});});
     Check(result==reinterpret_cast<void*>(0x5555)&&admissionOriginalCalls==1&&authorizationCalls==1&&proofCalls==0,"unowned/stale native continuation passes stock");}
    {admissionOriginalCalls=authorizationCalls=proofCalls=0;ownershipAllows=true;proofAllows=false;const auto result=cfgh216hooks::SelectorCall(selectorBusy,&AdmissionOriginal,nullptr,1,
        [&](void* stock,std::int32_t){return cfgh216hooks::ReadOnlyAfterOriginal<void*,int>(stock,true,true,7,7,true,
            [&]{++authorizationCalls;return ownershipAllows;},
            [&](int&){++proofCalls;return proofAllows;},[&](void*,const int&){return reinterpret_cast<void*>(0x6666);});});
     Check(result==reinterpret_cast<void*>(0x5555)&&admissionOriginalCalls==1&&authorizationCalls==1&&proofCalls==1,"veto or fresh graph refusal passes stock after one proof attempt");}
    // Exact two hook prologues are admitted by the owned registry only when the
    // real MinHook continuation has the required stolen bytes and relay layout.
    auto* code=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));
    Check(code!=nullptr,"owned fixture allocation");if(!code)return 1;
    std::array<unsigned char,16> expected=cfghosted::spectator35disk::SelectorEntry;
    std::memcpy(code,expected.data(),expected.size());
    FlushInstructionCache(GetCurrentProcess(),code,16);
    Memory memory;memory.target=reinterpret_cast<std::uintptr_t>(code);memory.image=memory.target-0x1000;
    cfgownednative::Registry registry;void* original=nullptr;
    Check(MH_Initialize()==MH_OK,"MinHook initialized");
    Check(MH_CreateHook(code,reinterpret_cast<void*>(&Detour),&original)==MH_OK,"selector hook created");
    Check(registry.Prepare(memory,memory.image,memory.target,expected,reinterpret_cast<std::uintptr_t>(original),
        reinterpret_cast<std::uintptr_t>(&Detour),5),"exact selector five-byte stolen prefix and continuation accepted");
    Check(MH_EnableHook(code)==MH_OK&&registry.Enable(memory,memory.target),"owned selector relay registered");
    Check(registry.Resolve(memory,memory.image,memory.target,expected)==reinterpret_cast<std::uintptr_t>(original),"only registered trampoline resolves");
    registry.Disable(memory.target);Check(registry.Resolve(memory,memory.image,memory.target,expected)==0,"disabled patch refuses trampoline");
    Check(MH_DisableHook(code)==MH_OK,"fixture hook disabled");Check(MH_RemoveHook(code)==MH_OK,"fixture hook removed");
    Check(MH_Uninitialize()==MH_OK,"MinHook uninitialized");VirtualFree(code,0,MEM_RELEASE);
    std::printf("H216 spectator hook gates: %u checks, %u failures. No game access.\n",checks,failures);return failures?1:0;
}
