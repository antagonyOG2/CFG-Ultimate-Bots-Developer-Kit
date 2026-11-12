#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <array>
#include <cstdint>
#include <utility>
#include "../Core/J22FeatureFlags.hpp"

namespace cfgmenu::startupretain {
// Exact typed call sites in the EXISTING startup coordinator. These are not a
// general ProcessEvent/HUD capability. Only a separately proved lifecycle
// callback provider can publish this callback-local scope.
enum class Role {Start,Intro,Match,Frontend};
enum class Call {LoadClass,ControllerGetter,ControllerFactory,CopyPath,ConsumeActive,
    DestroyPath,RestartHuman,RestartBot,SpawnController,TravelReady,AssignIntroClass,
    ParentIntro,ParentMatch,OriginalStart,SelectedClassPath,FrontendChallengePageClass};
inline constexpr char16_t ChallengePagePath[]=u"/Game/UI/Menu/FrontEnd/OfflinePlay/SPChallengesSettingsMenuWidget.SPChallengesSettingsMenuWidget_C";
inline bool Allowed(Role role,Call call)noexcept{
    if(role==Role::Frontend)return CFG_J22_CHALLENGES_RETENTION&&call==Call::FrontendChallengePageClass;
    if(role==Role::Start)return call==Call::OriginalStart;
    if(role==Role::Match)return call==Call::ParentMatch||call==Call::CopyPath||
        call==Call::ConsumeActive||call==Call::AssignIntroClass||call==Call::DestroyPath;
    if(role!=Role::Intro)return false;
    switch(call){
    case Call::LoadClass:case Call::ControllerGetter:case Call::ControllerFactory:
    case Call::CopyPath:case Call::ConsumeActive:case Call::DestroyPath:case Call::RestartHuman:
    case Call::RestartBot:case Call::SpawnController:case Call::TravelReady:case Call::AssignIntroClass:
    case Call::ParentIntro:case Call::SelectedClassPath:return true;
    default:return false;
    }
}
struct Hooks {
    void* context=nullptr;
    bool (*current)(void*,std::uintptr_t)=nullptr;
    bool (*retain)(void*,const std::array<std::uintptr_t,32>&,unsigned)=nullptr;
    bool (*before)(void*,Call,const std::array<std::uintptr_t,32>&,unsigned,unsigned)=nullptr;
    bool (*enter)(void*)=nullptr;
    bool (*after)(void*,const std::array<std::uintptr_t,32>&,unsigned)=nullptr;
    void (*cancel)(void*)=nullptr;
    void (*uncertain)(void*)=nullptr;
    bool Valid()const noexcept{return context&&current&&retain&&before&&enter&&after&&cancel&&uncertain;}
};
class Scope {
    inline static thread_local Scope* active_=nullptr;
    Scope* previous_=nullptr;Hooks hooks_{};std::uintptr_t module_=0;DWORD thread_=0;
    Role role_;bool entered_=false,occupied_=false,inflight_=false,faulted_=false;
public:
    Scope(std::uintptr_t module,Role role,const Hooks& hooks,bool admitted)noexcept:
        previous_(active_),hooks_(hooks),module_(module),thread_(GetCurrentThreadId()),role_(role){
        // Even a refused nested scope masks its outer capability. An engine
        // callback reentered during native dispatch must not borrow that lease.
        occupied_=active_!=nullptr;active_=this;
        entered_=!occupied_&&admitted&&module_&&hooks_.Valid()&&hooks_.current(hooks_.context,module_);
    }
    ~Scope(){active_=previous_;}
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
    bool Entered()const noexcept{return entered_;}
    bool Allows(Call call)noexcept{return Current(module_)==this&&Allowed(role_,call);}
    static bool Occupied()noexcept{return active_!=nullptr;}
    static Scope* Current(std::uintptr_t module)noexcept{
        auto* s=active_;return s&&s->entered_&&!s->faulted_&&!s->inflight_&&s->module_==module&&
            GetCurrentThreadId()==s->thread_&&s->hooks_.current(s->hooks_.context,module)?s:nullptr;
    }
    template<std::size_t N>bool Retain(const std::array<std::uintptr_t,N>& inputs)noexcept{
        static_assert(N>0&&N<=32);if(Current(module_)!=this)return false;
        std::array<std::uintptr_t,32> values{};for(unsigned i=0;i<N;++i)values[i]=inputs[i];
        return hooks_.retain(hooks_.context,values,static_cast<unsigned>(N));
    }
    template<std::size_t N,std::size_t M,class Native>bool Run(Call call,
        const std::array<std::uintptr_t,N>& inputs,std::array<std::uintptr_t,M>& outputs,Native&& native)noexcept{
        static_assert(N>0&&N<=32&&M<=32);
        outputs={};if(Current(module_)!=this||!Allowed(role_,call))return false;
        std::array<std::uintptr_t,32> values{};for(unsigned i=0;i<N;++i)values[i]=inputs[i];
        if(!hooks_.before(hooks_.context,call,values,static_cast<unsigned>(N),static_cast<unsigned>(M))){hooks_.cancel(hooks_.context);return false;}
        if(!hooks_.enter(hooks_.context)){hooks_.cancel(hooks_.context);return false;}
        inflight_=true;bool returned=false;
        try{returned=native();}catch(...){returned=false;}
        inflight_=false;
        if(!returned){faulted_=true;hooks_.uncertain(hooks_.context);return false;}
        values={};for(unsigned i=0;i<M;++i)values[i]=outputs[i];
        // Outputs must be rooted BEFORE subsequent validation/allocating calls.
        // The provider uses an independent completion budget, not the spent
        // coordinator's feature reader. Uncertain calls are never replayed.
        if(!hooks_.after(hooks_.context,values,static_cast<unsigned>(M))){
            faulted_=true;hooks_.uncertain(hooks_.context);return false;
        }
        return true;
    }
};
inline bool Current(std::uintptr_t module)noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    return Scope::Current(module)!=nullptr;
#else
    (void)module;return true;
#endif
}
template<std::size_t N>bool Retain(std::uintptr_t module,const std::array<std::uintptr_t,N>& inputs)noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    auto* scope=Scope::Current(module);return scope&&scope->Retain(inputs);
#else
    (void)module;(void)inputs;return true;
#endif
}
template<std::size_t N,std::size_t M,class Native>bool Invoke(std::uintptr_t module,Call call,
    const std::array<std::uintptr_t,N>& inputs,std::array<std::uintptr_t,M>& outputs,Native&& native)noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    auto* scope=Scope::Current(module);return scope&&scope->Run(call,inputs,outputs,std::forward<Native>(native));
#else
    (void)module;(void)call;(void)inputs;(void)outputs;return native();
#endif
}
template<std::size_t N,class Native>bool Invoke(std::uintptr_t module,Call call,
    const std::array<std::uintptr_t,N>& inputs,Native&& native)noexcept{
    std::array<std::uintptr_t,0> outputs{};return Invoke(module,call,inputs,outputs,std::forward<Native>(native));
}
}
