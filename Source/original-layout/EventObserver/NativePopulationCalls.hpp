#pragma once
#include "NativeCounselorPromotion.hpp"
#include "OwnedNativeDetour.hpp"
#include "../RuntimeProbe/NativePopulationBindings.generated.hpp"

namespace cfgmenu::population {
struct SpawnResult {
    std::uintptr_t controller=0;
    bool entered=false,completed=false,current=false;
};
namespace detail {
inline bool GetClass(std::uintptr_t function,std::uintptr_t& result)noexcept{
    result=0;
    __try {result=reinterpret_cast<std::uintptr_t(__fastcall*)()>(function)();return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// Small SEH leaf: no nontrivial automatic objects. Do not reuse this primitive
// without the caller's fresh route ownership and code verification below.
inline bool Spawn(std::uintptr_t constructor,std::uintptr_t spawn,std::uintptr_t world,
                  std::uintptr_t klass,std::uintptr_t mode,std::uintptr_t& result) noexcept {
    result=0;
    __try {
        alignas(16) unsigned char frame[populationdisk::SpawnParameterBytes]{};
        if(reinterpret_cast<void*(__fastcall*)(void*)>(constructor)(frame)!=frame)return false;
        std::memcpy(frame+populationdisk::SpawnOwnerOffset,&mode,sizeof(mode));
        frame[populationdisk::SpawnCollisionOffset]=populationdisk::ControllerCollisionMode;
        result=reinterpret_cast<std::uintptr_t(__fastcall*)(void*,void*,const void*,const void*,const void*)>(spawn)(
            reinterpret_cast<void*>(world),reinterpret_cast<void*>(klass),nullptr,nullptr,frame);
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool Travel(std::uintptr_t function,std::uintptr_t playerState,bool& ready) noexcept {
    ready=false;
    __try {ready=reinterpret_cast<bool(__fastcall*)(void*,bool)>(function)(reinterpret_cast<void*>(playerState),true);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
}
// Call once before roster population. Native factories may append/reallocate
// the object registry; Current must refresh that header and preserve owner serials.
template<class Reader,class Current> std::uintptr_t WarmCounselorController(Reader& reader,std::uintptr_t module,
    DWORD thread,Current&& current,bool exactImageVerified){
    namespace d=populationdisk;
    const auto guard=[&](){return exactImageVerified&&thread&&GetCurrentThreadId()==thread&&
        module==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))&&current();};
    std::uintptr_t reflection=0,klass=0;
    const auto read=[&](){return reader.Read(module+d::CounselorControllerReflection,reflection)&&
        reader.Read(module+d::CounselorControllerClassCache,klass)&&
        (!reflection||cfgprobe::ObjectPointer(reflection))&&(!klass||cfgprobe::ObjectPointer(klass));};
    if(!guard()||!read())return 0;
    if(cfgprobe::MatchingClassCache(reflection,klass))return klass;
    if(reflection&&klass)return 0;
    if(!promotion::detail::Code(module,module+d::CounselorControllerClassGetter,d::CounselorControllerClassGetterCode)||
       !promotion::detail::Code(module,module+d::CounselorControllerClassFactory,d::CounselorControllerClassFactoryCode))return 0;
    if(!klass){
        const auto oldReflection=reflection;std::uintptr_t made=0;
        std::array<std::uintptr_t,1> outputs{};
        if(!guard()||!startupretain::Invoke(module,startupretain::Call::ControllerGetter,std::array<std::uintptr_t,1>{oldReflection},outputs,[&](){
               const bool returned=detail::GetClass(module+d::CounselorControllerClassGetter,made);outputs[0]=made;return returned;})||!guard()||!read()||
           !cfgprobe::ObjectPointer(made)||klass!=made||(oldReflection&&reflection!=oldReflection))return 0;
    }
    if(!reflection){
        const auto oldClass=klass;std::uintptr_t made=0;
        std::array<std::uintptr_t,1> outputs{};
        if(!guard()||!startupretain::Invoke(module,startupretain::Call::ControllerFactory,std::array<std::uintptr_t,1>{oldClass},outputs,[&](){
               const bool returned=detail::GetClass(module+d::CounselorControllerClassFactory,made);outputs[0]=made;return returned;})||!guard()||!read()||
           !cfgprobe::ObjectPointer(made)||reflection!=made||klass!=oldClass)return 0;
    }
    const auto expected=klass;
    return guard()&&read()&&cfgprobe::MatchingClassCache(reflection,klass)&&klass==expected?klass:0;
}
// One instance per roster slot. The synchronous host's Current callback must
// revalidate exact OfflineBots mode, standalone world, callback thread, fresh
// world/mode identity serials, route latch and counselor-controller class identity.
// A returned controller is provisional until the host captures/revalidates its
// serial, verifies native PlayerState ownership, assigns the roster soft class,
// restarts it, verifies pawn ownership/class, then accounts for NumBots once.
// This adapter never makes roster choices or writes NumBots.
class Calls {
    std::uintptr_t module_=0,world_=0,mode_=0;
    DWORD thread_=0;
    bool exact_=false,spawnEntered_=false,travelEntered_=false,faulted_=false;unsigned travelRefusal_=0;
    template<std::size_t N> bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& bytes) const noexcept {
        return promotion::detail::Code(module_,module_+rva,bytes);
    }
    bool Guard() const noexcept {
        return exact_&&!faulted_&&thread_&&GetCurrentThreadId()==thread_&&
            module_==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))&&
            cfgprobe::ObjectPointer(world_)&&cfgprobe::ObjectPointer(mode_)&&world_!=mode_;
    }
public:
    Calls(std::uintptr_t module,std::uintptr_t world,std::uintptr_t mode,DWORD thread,bool exactImageVerified)
        :module_(module),world_(world),mode_(mode),thread_(thread),exact_(exactImageVerified){}
    template<class Reader,class Current> SpawnResult SpawnController(Reader& reader,Current&& current){
        namespace d=populationdisk;
        SpawnResult out{};std::uintptr_t reflection=0,klass=0;
        if(spawnEntered_||!Guard()||!current()||
           !reader.Read(module_+d::CounselorControllerReflection,reflection)||
           !reader.Read(module_+d::CounselorControllerClassCache,klass)||
           !cfgprobe::MatchingClassCache(reflection,klass)||
           !Code(d::SpawnParametersConstructor,d::SpawnParametersConstructorCode)||
           !Code(d::SpawnActor,d::SpawnActorCode))return out;
        // Mark before entry; null, exception, or changed context never replay.
        spawnEntered_=true;out.entered=true;
        std::array<std::uintptr_t,2> outputs{};
        out.completed=startupretain::Invoke(module_,startupretain::Call::SpawnController,
            std::array<std::uintptr_t,3>{world_,klass,mode_},outputs,[&](){
                const bool returned=detail::Spawn(module_+d::SpawnParametersConstructor,module_+d::SpawnActor,world_,klass,mode_,out.controller);
                outputs[0]=out.controller;
                // Spawned controller owns a newborn PlayerState which can still
                // have serial0. Capture and retain it before the next call.
                promotion::CurrentProcessReader completion;
                return returned&&(!out.controller||completion.Read(out.controller+0x388,outputs[1]));});
        if(!out.completed){faulted_=true;return out;}
        std::uintptr_t afterReflection=0,afterClass=0;
        out.current=Guard()&&current()&&reader.Read(module_+d::CounselorControllerReflection,afterReflection)&&
            reader.Read(module_+d::CounselorControllerClassCache,afterClass)&&afterReflection==reflection&&afterClass==klass;
        if(!out.current)faulted_=true;
        return out;
    }
    // CurrentPlayerState must additionally prove this is the spawned controller's
    // same callback-local controller-owned state (latching a serial once the
    // engine assigns one), with a positive controller identity. False readiness is a
    // native result, distinct from a bridge/ownership failure.
    template<class CurrentPlayerState> bool HasFullyTraveled(std::uintptr_t playerState,CurrentPlayerState&& current,bool& ready){
        namespace d=populationdisk;ready=false;
        // Diagnostic only (PID15068 second match: Failure413, last call RestartBot).
        // 1=entry-state 2=guard 3=state-pointer 4=bot-proof 5=code 6=native-or-after 7(caller)=post-proof 8=no-scope
        travelRefusal_=!spawnEntered_||travelEntered_?1u:!Guard()?2u:!cfgprobe::ObjectPointer(playerState)?3u:0u;
        if(travelRefusal_)return false;
        if(!current()){travelRefusal_=4;return false;}
        const auto function=cfgownednative::Resolve(module_,module_+d::HasFullyTraveled,d::HasFullyTraveledCode);
        if(!function){travelRefusal_=5;return false;}
        travelEntered_=true;const bool scoped=startupretain::Current(module_);
        if(!startupretain::Invoke(module_,startupretain::Call::TravelReady,
               std::array<std::uintptr_t,3>{world_,mode_,playerState},[&](){
                   // Re-prove immediately before entry. Never run the UI detour
                   // on this native population readiness call.
                   return cfgownednative::Resolve(module_,module_+d::HasFullyTraveled,d::HasFullyTraveledCode)==function&&
                       detail::Travel(function,playerState,ready);})||!Guard()||!current()){
            travelRefusal_=scoped?6u:8u;faulted_=true;ready=false;return false; // 8=no current startup scope before entry
        }
        return true;
    }
    unsigned TravelRefusal()const noexcept{return travelRefusal_;}
};
}
