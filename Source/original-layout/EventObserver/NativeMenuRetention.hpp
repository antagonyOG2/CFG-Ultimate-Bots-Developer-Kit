#pragma once
#include <cstdint>
#include "../Core/J24FeatureFlags.hpp"
namespace cfgmenu::rows {
// Compatibility boundary for the EXISTING native menu dispatcher. No native
// authority is inferred from an image pin, metadata observation or owner thread.
// The callback provider must supply a live independently validated lease and
// retain exact inputs/results. It owns the context for the entire scope.
struct NativeRetentionHooks {
    void* context=nullptr;
    bool (*current)(void*,std::uintptr_t)=nullptr;
    bool (*inputs)(void*,std::uintptr_t,std::uintptr_t,std::uintptr_t,void*)=nullptr;
    bool (*outputs)(void*,std::uintptr_t,std::uintptr_t,void*)=nullptr;
    bool (*factoryInput)(void*,std::uintptr_t,std::uintptr_t,std::uintptr_t)=nullptr;
    bool (*factoryOutput)(void*,std::uintptr_t,std::uintptr_t)=nullptr;
    void (*uncertain)(void*)=nullptr;
    void (*cancelUnpublished)(void*)=nullptr;
    bool (*enterNative)(void*)=nullptr;
    // Only a previously classified Challenges Push/Pop may supply this. All
    // other calls still use exact pre/post Current, unchanged.
    bool (*currentForOutputs)(void*,std::uintptr_t,std::uintptr_t,void*)=nullptr;
    bool Complete()const noexcept{return context&&current&&inputs&&outputs&&factoryInput&&factoryOutput&&uncertain&&cancelUnpublished&&enterNative;}
};
template<class Tag> class NativeRetentionScopeBase {
    inline static thread_local NativeRetentionScopeBase* active_=nullptr;
    inline static thread_local unsigned inputFailure_=0;
    inline static thread_local unsigned outputFailure_=0; // 0=none, 1=pre, 2=hook, 3=post
    NativeRetentionHooks hooks_{};std::uintptr_t module_=0;bool owns_=false,uncertain_=false;
    static NativeRetentionScopeBase* Match(std::uintptr_t module)noexcept{
        return active_&&active_->owns_&&active_->module_==module&&!active_->uncertain_?active_:nullptr;
    }
public:
    NativeRetentionScopeBase(std::uintptr_t module,const NativeRetentionHooks& hooks,bool admitted)noexcept:
        hooks_(hooks),module_(module){
        if(admitted&&module&&hooks.Complete()&&!active_){owns_=true;active_=this;}
    }
    ~NativeRetentionScopeBase(){if(owns_){hooks_.cancelUnpublished(hooks_.context);active_=nullptr;}}
    NativeRetentionScopeBase(const NativeRetentionScopeBase&)=delete;
    NativeRetentionScopeBase& operator=(const NativeRetentionScopeBase&)=delete;
    bool Entered()const noexcept{return owns_;}
    static bool Occupied()noexcept{return active_!=nullptr;}
    static unsigned InputFailure()noexcept{return inputFailure_;}
    static void ResetInputFailure()noexcept{inputFailure_=0;}
    static void ResetOutputFailure()noexcept{outputFailure_=0;}
    static unsigned OutputFailure()noexcept{return outputFailure_;}
    static bool Current(std::uintptr_t module)noexcept{
        const auto scope=Match(module);return scope&&scope->hooks_.current(scope->hooks_.context,module);
    }
    static void Uncertain(std::uintptr_t module)noexcept{
        const auto scope=Match(module);if(!scope)return;
        scope->uncertain_=true;scope->hooks_.uncertain(scope->hooks_.context);
    }
    static void CancelUnpublished(std::uintptr_t module)noexcept{
        const auto scope=Match(module);if(scope)scope->hooks_.cancelUnpublished(scope->hooks_.context);
    }
    static bool Inputs(std::uintptr_t module,std::uintptr_t object,std::uintptr_t function,void* packet)noexcept{
        inputFailure_=0;const auto scope=Match(module);
        if(!scope){inputFailure_=1;return false;}
        if(!Current(module)){inputFailure_=2;CancelUnpublished(module);return false;}
        if(!scope->hooks_.inputs(scope->hooks_.context,module,object,function,packet)){inputFailure_=3;CancelUnpublished(module);return false;}
        if(!Current(module)){inputFailure_=4;CancelUnpublished(module);return false;}
        return true;
    }
    static bool Outputs(std::uintptr_t module,std::uintptr_t function,void* packet)noexcept{
        const auto scope=Match(module);outputFailure_=0;
        if(!scope){outputFailure_=1;Uncertain(module);return false;}
        // Only an allowlisted, single-use challenge transition can reprove the
        // NEW stack before the provider has updated its binding. Never admit
        // unrelated functions by observing a different top widget.
        const bool pre=CFG_J25_CHALLENGE_PUSH_OUTPUTS&&scope->hooks_.currentForOutputs?
            scope->hooks_.currentForOutputs(scope->hooks_.context,module,function,packet):Current(module);
        if(!pre){outputFailure_=1;Uncertain(module);return false;}
        if(!scope->hooks_.outputs(scope->hooks_.context,module,function,packet)){
            outputFailure_=2;Uncertain(module);return false;
        }
        if(!Current(module)){outputFailure_=3;Uncertain(module);return false;}
        return true;
    }
    static bool FactoryInput(std::uintptr_t module,std::uintptr_t address,std::uintptr_t owner)noexcept{
        const auto scope=Match(module);
        if(scope&&owner&&Current(module)&&scope->hooks_.factoryInput(scope->hooks_.context,module,address,owner)&&Current(module))return true;
        CancelUnpublished(module);return false;
    }
    static bool FactoryOutput(std::uintptr_t module,std::uintptr_t result)noexcept{
        const auto scope=Match(module);
        if(scope&&result&&Current(module)&&scope->hooks_.factoryOutput(scope->hooks_.context,module,result)&&Current(module))return true;
        Uncertain(module);return false;
    }
    static bool EnterNative(std::uintptr_t module)noexcept{
        const auto scope=Match(module);
        if(scope&&Current(module)&&scope->hooks_.enterNative(scope->hooks_.context))return true;
        CancelUnpublished(module);return false;
    }
};
struct FrontendRetentionTag;struct HostedLobbyRetentionTag;
using NativeRetentionScope=NativeRetentionScopeBase<FrontendRetentionTag>;
using HostedLobbyRetentionScope=NativeRetentionScopeBase<HostedLobbyRetentionTag>;
// Preserve legacy/mock Engine interfaces. The production retained dispatcher
// implements FactoryOwned; untyped Factory cannot bypass its owner requirement.
template<class Engine>auto FactoryWithOwner(Engine& engine,std::uintptr_t address,std::uintptr_t owner,
    std::uintptr_t& output,int)->decltype(engine.FactoryOwned(address,owner,output)){
    return engine.FactoryOwned(address,owner,output);
}
template<class Engine>bool FactoryWithOwner(Engine& engine,std::uintptr_t address,std::uintptr_t,
    std::uintptr_t& output,long){return engine.Factory(address,output);}
template<class Engine>bool FactoryWithOwner(Engine& engine,std::uintptr_t address,std::uintptr_t owner,std::uintptr_t& output){
    return FactoryWithOwner(engine,address,owner,output,0);
}
}
