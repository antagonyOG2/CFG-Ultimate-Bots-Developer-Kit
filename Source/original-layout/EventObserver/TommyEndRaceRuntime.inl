// Included in the private frontend namespace. No new game thread or timer.
using NativeEndOriginal=void(__fastcall*)(void*);
NativeEndOriginal nativeEndOriginal=nullptr;
std::uintptr_t nativeEndHookTarget=0;
bool nativeEndHookEnabled=false;
void __fastcall NativeEndTommyGuard(void* receiver){
    const auto caller=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
    const auto& status=nativeOfflineStartup.Status();
    if(GetCurrentThreadId()==observerOwnerThread&&status.stage==cfgmenu::offlinestartup::Stage::Ready&&
       reinterpret_cast<std::uintptr_t>(receiver)==status.mode&&
       caller==nativeOfflineStartup.Owner().module+cfgtommyend::disk::TimerReturn){
        Reader r;r.cap=4096;unsigned reason=0;std::uintptr_t state=0;bool protect=false;
        __try{protect=r.Read(status.world+cfgprobe::WorldGameState,state)&&
            cfgtommyend::Protect(r,nativeOfflineStartup.Owner(),state,status.mode,caller,GetCurrentThreadId(),reason);}
        __except(EXCEPTION_EXECUTE_HANDLER){protect=false;}
        cfgclosedtrace::Mark(protect?"tommy-timer-end-veto":"tommy-timer-end-forward",status.mode,reason);
        if(protect)return;
    }
    // Legitimate timeout, Jason death, other modes, unowned worlds, uncertain
    // observations and every other caller forward exactly once, unchanged.
    if(nativeEndOriginal)nativeEndOriginal(receiver);
}
bool PrepareNativeTommyEndGuard(std::uintptr_t module){
    if(nativeEndHookTarget)return nativeEndHookEnabled&&nativeEndHookTarget==module+cfgtommyend::disk::EndMatch;
    Reader r;r.cap=32;
    if(!cfgbundle::Code(r,module,cfgescape::disk::EndMatchNative,cfgescape::disk::EndMatchNativeCode)||
       !cfgbundle::Code(r,module,cfgtommyend::disk::Timer,cfgtommyend::disk::TimerCode))return false;
    void* original=nullptr;const auto target=module+cfgtommyend::disk::EndMatch;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&NativeEndTommyGuard),&original)!=MH_OK)return false;
    nativeEndOriginal=reinterpret_cast<NativeEndOriginal>(original);nativeEndHookTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){
        std::array<unsigned char,682> observed{};
        if(r.Read(target,observed)&&observed[0]==0xE9&&
           std::equal(observed.begin()+5,observed.end(),cfgescape::disk::EndMatchNativeCode.begin()+5)){
            cfgtommyend::guardedCode=observed;cfgtommyend::guardedModule=module;
            nativeEndHookEnabled=true;return true;
        }
    }
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
