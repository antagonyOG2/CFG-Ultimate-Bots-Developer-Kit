#pragma once
#include "NativeJasonControllerTickObservation.hpp"
#include "OwnedNativeDetour.hpp"
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
using NativeJasonControllerTickOriginal=void(__fastcall*)(void*,float);
NativeJasonControllerTickOriginal bundleNativeJasonControllerTickOriginal=nullptr;
std::uintptr_t bundleNativeJasonControllerTickTarget=0;
thread_local unsigned bundleNativeJasonTickDepth=0;
thread_local bool bundleNativeJasonTickUsedBase=false;
thread_local bool bundleNativeJasonWaterKnown=false,bundleNativeJasonWater=true;
bool bundleNativeJasonControllerTickEnabled=false;
using DonorJasonHuntService=void(*)(const cfgjasontick::Owner&)noexcept;
DonorJasonHuntService bundleNativeJasonDonorService=nullptr;
bool DonorNativeDriverReady(std::uintptr_t controller);
void __fastcall BundleNativeJasonControllerTick(void*,float);
bool BundleNativeJasonControllerTickCodeCurrent(std::uintptr_t module)noexcept{
    using namespace cfgcompat::cfg35::j1a;
    return bundleNativeJasonControllerTickEnabled&&bundleNativeJasonControllerTickOriginal&&
        bundleNativeJasonControllerTickTarget==module+KillerControllerTick&&
        cfgownednative::Resolve(module,module+KillerControllerTick,KillerControllerTickCode)==
            reinterpret_cast<std::uintptr_t>(bundleNativeJasonControllerTickOriginal);
}
bool PrepareNativeJasonControllerTick(std::uintptr_t module){
    using namespace cfgcompat::cfg35::j1a;
    if(!module||GetCurrentThreadId()!=plan.threadId||!bundleNativeJasonDonorService)return false;
    const auto target=module+KillerControllerTick;
    if(bundleNativeJasonControllerTickTarget){
        if(bundleNativeJasonControllerTickTarget!=target||!bundleNativeJasonControllerTickOriginal)return false;
        if(bundleNativeJasonControllerTickEnabled)return BundleNativeJasonControllerTickCodeCurrent(module);
        Reader retry;retry.cap=64;cfgownednative::CurrentProcess process;
        if(!cfgbundle::Code(retry,module,KillerControllerTick,KillerControllerTickCode))return false;
        if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK&&cfgownednative::resident.Enable(process,target)){
            bundleNativeJasonControllerTickEnabled=true;return true;
        }
        MH_DisableHook(reinterpret_cast<void*>(target));return false;
    }
    Reader reader;reader.cap=64;
    if(!cfgbundle::Code(reader,module,KillerControllerTick,KillerControllerTickCode))return false;
    void* original=nullptr;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeJasonControllerTick),&original)!=MH_OK)return false;
    cfgownednative::CurrentProcess process;
    // Actual MinHook steals push rbx(2)+sub rsp,20h(4), then emits its
    // continuation. The next mov rbx,rcx stays at original+6. No relocation.
    if(!cfgownednative::resident.Prepare(process,module,target,KillerControllerTickCode,
        reinterpret_cast<std::uintptr_t>(original),reinterpret_cast<std::uintptr_t>(&BundleNativeJasonControllerTick),6)){
        MH_RemoveHook(reinterpret_cast<void*>(target));return false;
    }
    bundleNativeJasonControllerTickOriginal=reinterpret_cast<NativeJasonControllerTickOriginal>(original);
    bundleNativeJasonControllerTickTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK&&cfgownednative::resident.Enable(process,target)){
        bundleNativeJasonControllerTickEnabled=true;return true;
    }
    MH_DisableHook(reinterpret_cast<void*>(target));cfgownednative::resident.Disable(target);
    return false; // no foreign/partially verified trampoline becomes authority
}
inline bool NativeJasonControllerTickInFlight()noexcept{return bundleNativeJasonTickDepth!=0;}
#else
inline bool NativeJasonControllerTickInFlight()noexcept{return false;}
#endif
