#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <intrin.h>
#include <atomic>
#include "UrinationActionObservation.hpp"
#include "UrinationOwnedHookObservation.hpp"
namespace cfgurination {
// Thin port of the donor's native constructor/completion bridges, plus action
// destruction invalidation. Host owns exact request/retention validation.
// No UObject/native-call authority is created by installing these observers.
template<class Host>class NativeHooks {
    using Constructor=void*(__fastcall*)(void*,void*,void*);
    using Delegate=bool(__fastcall*)(void*,void*);
    using Destructor=void*(__fastcall*)(void*,unsigned);
    inline static Host* host_=nullptr;
    inline static Constructor constructor_=nullptr;
    inline static Delegate delegate_=nullptr;
    inline static Destructor destructor_=nullptr;
    inline static std::atomic<bool> ready_{false};
    inline static std::uint32_t thread_=0;
    inline static std::uintptr_t module_=0;
    inline static bool attempted_=false;
    inline static std::array<OwnedHook,3> proofs_{};
    inline static thread_local bool observing_=false;
    static bool Owned()noexcept{return ready_.load(std::memory_order_acquire)&&GetCurrentThreadId()==thread_&&!observing_;}
    static void* __fastcall Construct(void* action,void* path,void* latent){
        const auto caller=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
        void* result=constructor_(action,path,latent); // preserve native exceptions and exactly one original call
        if(!Owned()||!host_->Requesting())return result;
        observing_=true;
        __try{
            __try{host_->Constructed(reinterpret_cast<std::uintptr_t>(action),reinterpret_cast<std::uintptr_t>(result),
                reinterpret_cast<std::uintptr_t>(path),reinterpret_cast<std::uintptr_t>(latent),caller);}
            __except(EXCEPTION_EXECUTE_HANDLER){host_->ObservationFault();}
        }__finally{observing_=false;}
        return result;
    }
    static bool __fastcall Loaded(void* delegate,void* value){
        const auto caller=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
        if(Owned()){
            observing_=true;
            __try{
                __try{host_->Completing(reinterpret_cast<std::uintptr_t>(delegate),reinterpret_cast<std::uintptr_t>(value),caller);}
                __except(EXCEPTION_EXECUTE_HANDLER){host_->ObservationFault();}
            }__finally{observing_=false;}
        }
        // Host MUST close its registry window before returning here. Native
        // forwarding/return value remain authoritative even on rejected output.
        return delegate_(delegate,value);
    }
    static void* __fastcall Destroy(void* action,unsigned flags){
        if(Owned()){
            observing_=true;
            __try{
                __try{host_->Destroying(reinterpret_cast<std::uintptr_t>(action));}
                __except(EXCEPTION_EXECUTE_HANDLER){host_->ObservationFault();}
            }__finally{observing_=false;}
        }
        return destructor_(action,flags); // caller may free action; never read it afterwards
    }
public:
    static bool Ready()noexcept{return ready_.load(std::memory_order_acquire);}
    template<class R>static bool Proof(R& r,std::uintptr_t module){
        return Ready()&&module_==module&&ProveOwnedHook(r,module,proofs_[0])&&
            ProveOwnedHook(r,module,proofs_[1])&&ProveOwnedHook(r,module,proofs_[2]);
    }
    // Existing MinHook owner supplies Code/Create/Enable. Code checks all three
    // pinned bodies BEFORE changing one. Created state stays resident on partial
    // failure; do not remove a possibly entered trampoline or retry installation.
    template<class Api>static bool Install(Host& host,Api& api,std::uintptr_t module,std::uint32_t thread){
        if(attempted_)return Ready()&&host_==&host&&thread_==thread&&module_==module;
        if(!cfgprobe::ObjectPointer(module)||module>UINTPTR_MAX-disk::LoadedDelegate||!thread||
            GetCurrentThreadId()!=thread||!host.PinnedBuild(module)||
            !api.Code(module+disk::ActionConstructor,disk::ActionConstructorCode)||
            !api.Code(module+disk::LoadedDelegate,disk::LoadedDelegateCode)||
            !api.Code(module+disk::ActionDestructor,disk::ActionDestructorCode))return false;
        attempted_=true;host_=&host;thread_=thread;module_=module;
        void* ctor=nullptr;void* loaded=nullptr;void* destroyed=nullptr;
        if(!api.Create(module+disk::ActionConstructor,reinterpret_cast<std::uintptr_t>(&Construct),ctor)||!ctor)return false;
        constructor_=reinterpret_cast<Constructor>(ctor);
        if(!api.Create(module+disk::LoadedDelegate,reinterpret_cast<std::uintptr_t>(&Loaded),loaded)||!loaded)return false;
        delegate_=reinterpret_cast<Delegate>(loaded);
        if(!api.Create(module+disk::ActionDestructor,reinterpret_cast<std::uintptr_t>(&Destroy),destroyed)||!destroyed)return false;
        destructor_=reinterpret_cast<Destructor>(destroyed);
        if(!api.PrepareOwned(module,module+disk::ActionConstructor,reinterpret_cast<std::uintptr_t>(ctor),
                reinterpret_cast<std::uintptr_t>(&Construct),disk::ActionConstructorCode,proofs_[0])||
            !api.PrepareOwned(module,module+disk::LoadedDelegate,reinterpret_cast<std::uintptr_t>(loaded),
                reinterpret_cast<std::uintptr_t>(&Loaded),disk::LoadedDelegateCode,proofs_[1])||
            !api.PrepareOwned(module,module+disk::ActionDestructor,reinterpret_cast<std::uintptr_t>(destroyed),
                reinterpret_cast<std::uintptr_t>(&Destroy),disk::ActionDestructorCode,proofs_[2]))return false;
        if(!api.Enable(module+disk::ActionConstructor)||!api.Enable(module+disk::LoadedDelegate)||
            !api.Enable(module+disk::ActionDestructor))return false;
        ready_.store(true,std::memory_order_release);return true;
    }
};
}
