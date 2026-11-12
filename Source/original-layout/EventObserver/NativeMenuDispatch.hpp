#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/SpawnRuntimeBindings.generated.hpp"
#include "../RuntimeProbe/DispatchObservation.hpp"
#include "../RuntimeProbe/OwnedGenericDispatcher.hpp"
#include "NativeMenuRetention.hpp"
namespace cfgmenu::rows {
// Actual Windows adapter, not a worker/injector. Only the frontend callback
// provider may create one after ExactBuild has hashed and pinned the image.
// NativeAccess supplies target/function/owner/packet checks around each call.
struct WindowsDispatchPlatform {
    static DWORD Thread()noexcept{return GetCurrentThreadId();}
    static std::uintptr_t Module()noexcept{return reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));}
    static bool Read(std::uintptr_t at,void* out,std::size_t bytes)noexcept{SIZE_T read=0;
        return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(at),out,bytes,&read)&&read==bytes;}
    static void Invoke(std::uintptr_t address,std::uintptr_t object,std::uintptr_t function,void* packet){
        reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(address)(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),packet);}
    static std::uintptr_t Factory(std::uintptr_t address){return reinterpret_cast<std::uintptr_t(__fastcall*)()>(address)();}
};
// The production platform is unchanged Win32 memory/SEH/native dispatch. An owned
// fixture can supply memory and native stubs while exercising this SAME pipeline.
template<class Platform=WindowsDispatchPlatform,class Retention=NativeRetentionScope>class NativeDispatchBase {
public:
    enum class Failure:unsigned {None,Thread,Faulted,Unverified,CurrentScope,Object,Guard,Inputs,Reproof,Entry,NativeException,Outputs};
private:
    Failure failure_=Failure::None;
    bool Refuse(Failure why)noexcept{failure_=why;return false;}
    struct LocalReader {template<class T>bool Read(std::uintptr_t at,T& out){return Platform::Read(at,&out,sizeof(out));}};
    std::uintptr_t module_=0;DWORD ownerThread_=0;unsigned calls_=0;bool exactBuild_=false,faulted_=false;
    bool Guard()noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!Retention::Current(module_))return false;
#endif
        if(!exactBuild_||faulted_||Platform::Thread()!=ownerThread_||calls_==4||
            module_!=Platform::Module())return false;
        LocalReader proof;
        DWORD current=0;std::uint8_t flag=0;
        return Platform::Read(module_+cfgprobe::dispatch::ThreadIdRva,&current,sizeof(current))&&current==ownerThread_&&
            Platform::Read(module_+cfgprobe::dispatch::ThreadFlagRva,&flag,sizeof(flag))&&flag==1&&
            cfgdispatch::Proof(proof,module_);
    }
public:
    NativeDispatchBase(std::uintptr_t module,DWORD thread,bool exactImageVerified):module_(module),ownerThread_(thread),exactBuild_(exactImageVerified){}
    DWORD Thread()const noexcept{return Platform::Thread();}
    bool BeginPhase()noexcept{
        failure_=Failure::None;
        if(Platform::Thread()!=ownerThread_)return Refuse(Failure::Thread);
        if(faulted_)return Refuse(Failure::Faulted);
        if(!exactBuild_)return Refuse(Failure::Unverified);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!Retention::Current(module_))return Refuse(Failure::CurrentScope);
#endif
        calls_=0;return true;
    }
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* packet)noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        Retention::ResetInputFailure();
        Retention::ResetOutputFailure();
#endif
        if(!cfgprobe::ObjectPointer(object)||!cfgprobe::ObjectPointer(function)||!packet)return Refuse(Failure::Object);
        if(!Guard())return Refuse(Failure::Guard);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!Retention::Inputs(module_,object,function,packet))return Refuse(Failure::Inputs);
        if(!Guard()){Retention::CancelUnpublished(module_);return Refuse(Failure::Reproof);}
        if(!Retention::EnterNative(module_))return Refuse(Failure::Entry);
#endif
        ++calls_;
        __try{
            Platform::Invoke(module_+cfgspawn::GenericDispatcherRva,object,function,packet);
        }__except(EXCEPTION_EXECUTE_HANDLER){faulted_=true;failure_=Failure::NativeException;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            Retention::Uncertain(module_);
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!Retention::Outputs(module_,function,packet)){faulted_=true;return Refuse(Failure::Outputs);}
#endif
        return true;
    }
    // Lazy native UFunction factories are normal reflection construction, not
    // alternate loading. Caller must pin the entire 47-byte factory first and
    // recheck its native owner/current screen. Never used on a refused cache.
    bool Factory(std::uintptr_t address,std::uintptr_t& out)noexcept{return FactoryOwned(address,0,out);}
    bool FactoryOwned(std::uintptr_t address,std::uintptr_t owner,std::uintptr_t& out)noexcept{
        out=0;if(!Guard()||address<module_||address-module_>=cfgprobe::ExecutableImageBytes)return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!cfgprobe::ObjectPointer(owner)||!Retention::FactoryInput(module_,address,owner))return false;
        if(!Guard()){Retention::CancelUnpublished(module_);return Refuse(Failure::Reproof);}
        if(!Retention::EnterNative(module_))return Refuse(Failure::Entry);
#else
        (void)owner;
#endif
        ++calls_;
        __try{out=Platform::Factory(address);}
        __except(EXCEPTION_EXECUTE_HANDLER){faulted_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            Retention::Uncertain(module_);
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!Retention::FactoryOutput(module_,out)){faulted_=true;out=0;return false;}
#endif
        return cfgprobe::ObjectPointer(out);
    }
    Failure LastFailure()const noexcept{return failure_;}
    unsigned Calls()const noexcept{return calls_;}
    bool Faulted()const noexcept{return faulted_;}
};
using NativeDispatch=NativeDispatchBase<>;
using HostedLobbyNativeDispatch=NativeDispatchBase<WindowsDispatchPlatform,HostedLobbyRetentionScope>;
}
