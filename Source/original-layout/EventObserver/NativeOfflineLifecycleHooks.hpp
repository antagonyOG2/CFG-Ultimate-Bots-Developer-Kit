#pragma once
#include "NativeCounselorPromotion.hpp"
#include "TrampolinePreparation.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
namespace cfgmenu::offlinehooks {
using Handler=void(__fastcall*)(void*);
enum class State {Empty,Preparing,Prepared,Enabling,Enabled,DisabledResident,Refused,Quarantined};
struct Request {
    std::uintptr_t module=0,handlerImage=0;
    Handler intro=nullptr,match=nullptr;
    bool exactImageVerified=false;
    // Caller asserts it is outside DllMain/loader lock and serializes access
    // to THIS linked MinHook instance (including all queue/apply operations).
    bool outsideLoaderLock=false,exclusiveBackend=false;
};
class WindowsBackend {
    Request request_{};
    static bool ImageCode(std::uintptr_t at,std::uintptr_t image){
        MEMORY_BASIC_INFORMATION m{};
        return at&&image&&VirtualQuery(reinterpret_cast<void*>(at),&m,sizeof(m))==sizeof(m)&&
            m.State==MEM_COMMIT&&m.Type==MEM_IMAGE&&reinterpret_cast<std::uintptr_t>(m.AllocationBase)==image&&
            (m.Protect==PAGE_EXECUTE_READ||m.Protect==PAGE_EXECUTE_WRITECOPY);
    }
public:
    bool Ready(const Request& r){
        if(!r.exactImageVerified||!r.outsideLoaderLock||!r.exclusiveBackend||
           r.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||!r.handlerImage||r.handlerImage==r.module||
           !ImageCode(reinterpret_cast<std::uintptr_t>(r.intro),r.handlerImage)||
           !ImageCode(reinterpret_cast<std::uintptr_t>(r.match),r.handlerImage)||r.intro==r.match)return false;
        request_=r;return Pristine();
    }
    bool Pristine(){namespace d=offlinelifecycledisk;
        return promotion::detail::Code(request_.module,request_.module+d::OfflineBotsIntro958,d::OfflineBotsIntro958Code)&&
            promotion::detail::Code(request_.module,request_.module+d::OfflineBotsMatchStarted818,d::OfflineBotsMatchStartedCode);
    }
    bool Pin(){
        HMODULE resident=nullptr;
        return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(request_.intro),&resident)&&reinterpret_cast<std::uintptr_t>(resident)==request_.handlerImage;
    }
    int Initialize(){return static_cast<int>(MH_Initialize());}
    int Create(std::uintptr_t target,Handler handler,Handler& original){
        void* trampoline=nullptr;const auto status=MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(handler),&trampoline);
        if(status==MH_OK)original=reinterpret_cast<Handler>(trampoline);
        return static_cast<int>(status);
    }
    bool Trampoline(Handler fn){return fn&&cfgobserver::ExecutableRange(reinterpret_cast<void*>(fn),1);}
    int Queue(std::uintptr_t target,bool enabled){return static_cast<int>(enabled?MH_QueueEnableHook(reinterpret_cast<void*>(target)):MH_QueueDisableHook(reinterpret_cast<void*>(target)));}
    int Apply(){return static_cast<int>(MH_ApplyQueued());}
    int Remove(std::uintptr_t target){return static_cast<int>(MH_RemoveHook(reinterpret_cast<void*>(target)));}
};
// The only two targets are the disk-proven OfflineBots958/818 native bodies.
// No630 interception, route claim, game call, or default installation. The
// owner must publish both Originals AFTER Prepare and BEFORE explicit Enable.
// Objects and callback DLL must remain resident after trampoline publication.
template<class Backend=WindowsBackend> class Pair {
    Backend& backend_;Request request_{};Handler introOriginal_=nullptr,matchOriginal_=nullptr;
    State state_=State::Empty;int lastError_=MH_OK;bool createdIntro_=false,createdMatch_=false,enableAttempted_=false;
    std::uintptr_t Intro()const{return request_.module+offlinelifecycledisk::OfflineBotsIntro958;}
    std::uintptr_t Match()const{return request_.module+offlinelifecycledisk::OfflineBotsMatchStarted818;}
    bool RefuseBeforePublication(){
        bool removed=true;
        if(createdMatch_){const int s=backend_.Remove(Match());if(s!=MH_OK){lastError_=s;removed=false;}}
        if(createdIntro_){const int s=backend_.Remove(Intro());if(s!=MH_OK){lastError_=s;removed=false;}}
        if(removed){createdIntro_=createdMatch_=false;introOriginal_=matchOriginal_=nullptr;state_=State::Refused;}
        else state_=State::Quarantined;
        return false;
    }
    bool CancelQueues(){
        const int a=backend_.Queue(Intro(),false),b=backend_.Queue(Match(),false);
        if(a!=MH_OK||b!=MH_OK){lastError_=a!=MH_OK?a:b;state_=State::Quarantined;return false;}
        return true;
    }
public:
    explicit Pair(Backend& backend):backend_(backend){}
    Pair(const Pair&)=delete;Pair& operator=(const Pair&)=delete;
    // Intentionally no destructor cleanup/MH_Uninitialize/FreeLibrary: another
    // thread may still execute an original trampoline after a failed apply.
    State Status()const{return state_;}int LastError()const{return lastError_;}
    bool Prepared()const{return state_==State::Prepared||state_==State::Enabled||state_==State::DisabledResident;}
    bool Enabled()const{return state_==State::Enabled;}
    bool Quarantined()const{return state_==State::Quarantined;}
    Handler OriginalIntro()const{return introOriginal_;}Handler OriginalMatch()const{return matchOriginal_;}
    bool Prepare(const Request& request){
        if(state_!=State::Empty)return false;state_=State::Preparing;request_=request;
        if(!backend_.Ready(request_)||!backend_.Pin()){state_=State::Refused;return false;}
        lastError_=backend_.Initialize();
        if(lastError_!=MH_OK&&lastError_!=MH_ERROR_ALREADY_INITIALIZED){state_=State::Refused;return false;}
        if(!backend_.Pristine()){state_=State::Refused;return false;}
        lastError_=backend_.Create(Intro(),request_.intro,introOriginal_);
        if(lastError_!=MH_OK)return RefuseBeforePublication();createdIntro_=true;
        if(!backend_.Trampoline(introOriginal_))return RefuseBeforePublication();
        lastError_=backend_.Create(Match(),request_.match,matchOriginal_);
        if(lastError_!=MH_OK)return RefuseBeforePublication();createdMatch_=true;
        if(!backend_.Trampoline(matchOriginal_)||!backend_.Pristine())return RefuseBeforePublication();
        state_=State::Prepared;return true;
    }
    bool Enable(){
        if(state_!=State::Prepared||enableAttempted_)return false;enableAttempted_=true;
        if(!backend_.Pristine()){state_=State::DisabledResident;return false;}
        state_=State::Enabling;
        lastError_=backend_.Queue(Intro(),true);
        if(lastError_!=MH_OK){if(CancelQueues())state_=State::DisabledResident;return false;}
        lastError_=backend_.Queue(Match(),true);
        if(lastError_!=MH_OK){if(CancelQueues())state_=State::DisabledResident;return false;}
        // One MinHook transaction/suspension for the pair. MinHook can fail
        // after a partial apply; that case requires a paired disable attempt.
        lastError_=backend_.Apply();
        if(lastError_==MH_OK){state_=State::Enabled;return true;}
        if(!CancelQueues())return false;
        const int rollback=backend_.Apply();
        if(rollback==MH_OK)state_=State::DisabledResident;
        else{lastError_=rollback;state_=State::Quarantined;}
        return false;
    }
};
}
