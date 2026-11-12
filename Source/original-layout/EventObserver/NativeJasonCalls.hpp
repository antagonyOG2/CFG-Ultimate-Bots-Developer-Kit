#pragma once
#include "NativeCounselorPromotion.hpp"
#include "../RuntimeProbe/NativePopulationBindings.generated.hpp"
#include "../RuntimeProbe/NativeJasonBindings.generated.hpp"
#include "../Core/NativeSpawnParameters.hpp"

namespace cfgnativejason {
struct Fault { unsigned code=0;std::uintptr_t address=0; };
inline int Capture(EXCEPTION_POINTERS* info,Fault& fault)noexcept{
    if(info&&info->ExceptionRecord){fault.code=info->ExceptionRecord->ExceptionCode;
        fault.address=reinterpret_cast<std::uintptr_t>(info->ExceptionRecord->ExceptionAddress);}
    return EXCEPTION_EXECUTE_HANDLER;
}
namespace detail {
// Same actor-first sequence as the donor's SpawnLifecycleActor. The caller
// proves fresh world/class/thread/registry ownership before each native entry.
inline bool Spawn(std::uintptr_t constructor,std::uintptr_t function,std::uintptr_t world,
    std::uintptr_t klass,const cfgspawn::Vec3* location,const cfgspawn::Vec3* rotation,
    std::uintptr_t& result,Fault& fault,std::uint8_t collision=2,std::uintptr_t owner=0)noexcept{
    result=0;
    __try{
        alignas(16) unsigned char frame[cfgmenu::populationdisk::SpawnParameterBytes]{};
        if(reinterpret_cast<void*(__fastcall*)(void*)>(constructor)(frame)!=frame)return false;
        // Null owner; donor AdjustIfPossibleButAlwaysSpawn at the authored start.
        if(owner)std::memcpy(frame+cfgmenu::populationdisk::SpawnOwnerOffset,&owner,sizeof(owner)); // H2.58 donor F1 pawn owner = GameMode; 0 for every other caller
        frame[cfgmenu::populationdisk::SpawnCollisionOffset]=collision;
        result=reinterpret_cast<std::uintptr_t(__fastcall*)(void*,void*,const void*,const void*,const void*)>(function)(
            reinterpret_cast<void*>(world),reinterpret_cast<void*>(klass),location,rotation,frame);
        return true;
    }__except(Capture(GetExceptionInformation(),fault)){return false;}
}
inline bool Possess(std::uintptr_t function,std::uintptr_t controller,std::uintptr_t pawn,Fault& fault)noexcept{
    __try{
        reinterpret_cast<void(__fastcall*)(void*,void*)>(function)(reinterpret_cast<void*>(controller),reinterpret_cast<void*>(pawn));
        return true;
    }__except(Capture(GetExceptionInformation(),fault)){return false;}
}
}
template<class Reader> bool SpawnCode(Reader& r,std::uintptr_t module){
    namespace d=cfgmenu::populationdisk;
    std::array<unsigned char,d::SpawnParametersConstructorCode.size()> ctor{};
    std::array<unsigned char,d::SpawnActorCode.size()> spawn{};
    return r.Read(module+d::SpawnParametersConstructor,ctor)&&ctor==d::SpawnParametersConstructorCode&&
        r.Read(module+d::SpawnActor,spawn)&&spawn==d::SpawnActorCode;
}
template<class Reader> bool PossessCode(Reader& r,std::uintptr_t module){
    std::array<unsigned char,disk::BasePossessCode.size()> code{};
    return r.Read(module+disk::BasePossess,code)&&code==disk::BasePossessCode;
}
}
