#pragma once
#include "NativeCounselorPromotion.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
namespace cfgmenu::offlinelifecycle {
// Current-process, game-thread native bridge only. Does not install callbacks,
// select a mode or grant route admission. The host must retain fresh ownership.
class Calls {
    promotion::Context owner_{};bool faulted_=false,introCalled_=false,matchCalled_=false,restartCalled_=false;
    template<std::size_t N> bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& bytes){
        return promotion::detail::Code(owner_.module,owner_.module+rva,bytes);
    }
    static bool One(std::uintptr_t function,std::uintptr_t object)noexcept{
        __try{reinterpret_cast<void(__fastcall*)(void*)>(function)(reinterpret_cast<void*>(object));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
public:
    explicit Calls(const promotion::Context& owner):owner_(owner){}
    template<class Reader> bool Current(Reader& r){
        using namespace cfgprobe;namespace d=offlinelifecycledisk;
        if(faulted_||!owner_.exactImageVerified||!owner_.thread||GetCurrentThreadId()!=owner_.thread||
           owner_.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
        const std::array<std::uintptr_t,4> objects{{owner_.world,owner_.mode,owner_.controller,owner_.playerState}};
        for(unsigned i=0;i<objects.size();++i){
            if(!owner_.identities[i].HasAssignedSerial()||owner_.identities[i].object!=objects[i])return false;
            for(unsigned j=0;j<i;++j)if(objects[i]==objects[j]||owner_.identities[i].index==owner_.identities[j].index)return false;
        }
        const auto same=[&](std::uintptr_t at,std::uintptr_t expected){std::uintptr_t got=0;return r.Read(at,got)&&got==expected;};
        cfgprobe::lifetime::ArrayHeader fresh{};
        if(!cfgprobe::lifetime::ReadArrayHeader(r,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh)||
           !cfgprobe::lifetime::RevalidateBatch(r,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh,owner_.identities))return false;
        owner_.registry=fresh;
        std::uintptr_t klass=0,constructor=0,defaults=0;DWORD thread=0;unsigned char gameThread=0;
        if(!r.Read(owner_.module+dispatch::ThreadIdRva,thread)||thread!=owner_.thread||
           !r.Read(owner_.module+dispatch::ThreadFlagRva,gameThread)||gameThread!=1||
           !lifetime::RevalidateBatch(r,owner_.module+lifetime::ObjectArrayRva,owner_.registry,owner_.identities)||
           !same(owner_.module+EngineGlobalCandidate,owner_.engine)||!same(owner_.engine+EngineGameViewport,owner_.viewport)||
           !same(owner_.viewport+ViewportWorld,owner_.world)||!same(owner_.world+WorldNetDriver,0)||
           !same(owner_.world+WorldGameMode,owner_.mode)||!same(owner_.world+WorldGameInstance,owner_.gameInstance)||
           !same(owner_.gameInstance+GameInstanceLocalPlayers,owner_.localPlayers)||!same(owner_.localPlayers,owner_.player)||
           !same(owner_.player+PlayerController,owner_.controller)||!same(owner_.controller+ControllerPlayer,owner_.player)||
           !same(owner_.controller+0x388,owner_.playerState)||!same(owner_.playerState+0x88,owner_.controller)||
           !r.Read(owner_.module+d::OfflineBotsClass,klass)||!r.Read(owner_.module+d::OfflineBotsClass+8,constructor)||
           !MatchingClassCache(klass,constructor)||!same(owner_.mode+ObjectClass,klass)||
           !r.Read(klass+0xF8,defaults)||!ObjectPointer(defaults)||defaults==owner_.mode||
           !same(owner_.mode,owner_.module+d::OfflineBotsVtable))return false;
        int count=0,capacity=0;
        return r.Read(owner_.gameInstance+GameInstanceLocalPlayers+8,count)&&count==1&&
            r.Read(owner_.gameInstance+GameInstanceLocalPlayers+12,capacity)&&capacity>=1&&capacity<=1024;
    }
    template<class Reader> bool ParentIntro(Reader& r){
        namespace d=offlinelifecycledisk;
        if(introCalled_||!Current(r)||!Code(d::OfflineIntro958,d::OfflineIntro958Code))return false;
        // Mark before native entry. An exception/recursive callback must not replay it.
        introCalled_=true;
        if(!startupretain::Invoke(owner_.module,startupretain::Call::ParentIntro,promotion::RetainedInputs(owner_),
            [&](){return One(owner_.module+d::OfflineIntro958,owner_.mode);})){faulted_=true;return false;}
        return Current(r);
    }
    template<class Reader> bool RestartSelectedHuman(Reader& r){
        namespace d=offlinelifecycledisk;
        std::uintptr_t pawn=0,acknowledged=0;promotion::Packet active{};
        if(restartCalled_||!owner_.pickedResolutionVerified||!cfgprobe::ObjectPointer(owner_.selectedClass)||!Current(r)||
           !r.Read(owner_.controller+cfgprobe::ControllerPawn,pawn)||pawn||
           !r.Read(owner_.controller+cfgprobe::ControllerAcknowledgedPawn,acknowledged)||acknowledged||
           !r.Read(owner_.playerState+birthdisk::ActiveCharacterClass,active)||
           std::memcmp(active.bytes.data()+16,owner_.picked.bytes.data()+16,8)!=0||
           !Code(d::OfflineBotsRestartPlayer,d::OfflineBotsRestartPlayerCode)||
           !Code(d::ParentRestartPlayer,d::ParentRestartPlayerCode))return false;
        restartCalled_=true;
        std::array<std::uintptr_t,1> outputs{};
        if(!startupretain::Invoke(owner_.module,startupretain::Call::RestartHuman,promotion::RetainedInputs(owner_),outputs,[&](){
            promotion::CurrentProcessReader completion;
            return promotion::detail::Two(owner_.module+d::OfflineBotsRestartPlayer,owner_.mode,reinterpret_cast<void*>(owner_.controller))&&
                completion.Read(owner_.controller+cfgprobe::ControllerPawn,outputs[0]);})){faulted_=true;return false;}
        // The caller still must validate resulting counselor class, controller,
        // acknowledged pawn and HUD before publishing successful human birth.
        return Current(r);
    }
    template<class Reader> bool ParentMatchStart(Reader& r){
        namespace d=offlinelifecycledisk;
        if(!introCalled_||matchCalled_||!Current(r)||!Code(d::OfflineMatchStarted818,d::OfflineMatchStartedCode))return false;
        matchCalled_=true;
        if(!startupretain::Invoke(owner_.module,startupretain::Call::ParentMatch,promotion::RetainedInputs(owner_),
            [&](){return One(owner_.module+d::OfflineMatchStarted818,owner_.mode);})){faulted_=true;return false;}
        return Current(r);
    }
};
}
