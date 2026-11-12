#pragma once
#include "SpawnResultObservation.hpp"
namespace cfghelperbirth {
// A native return is captured immediately, before yielding. This is NOT a
// usable helper until the existing reciprocal controller/brain/scope proof
// succeeds on its own callback. Never resolve an unjournaled pointer later.
struct PendingCounselorBirth {
    cfgprobe::lifetime::SlotObservation pawn{};
    std::uintptr_t world=0,pawnClass=0;
    cfgspawn::Vec3 requested{};
    std::uint64_t deadline=0;
    bool pending=false;
    static constexpr unsigned CaptureReserve=64;
    template<class Reader> bool Capture(Reader& r,const cfgobserver::Plan& plan,
        std::uintptr_t returned,std::uintptr_t expectedClass,cfgspawn::Vec3 location,std::uint64_t now){
        if(pending||!returned||!expectedClass||!cfgspawn::Finite(location)||returned==plan.pawn)return false;
        cfgprobe::lifetime::ArrayHeader registry{},again{};
        cfgprobe::lifetime::SlotObservation first{},last{};
        std::uintptr_t type=0;
        if(!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfgspawn::ObjectSlot(r,registry,returned,first)||!first.HasAssignedSerial()||
           !r.Read(returned+cfgprobe::ObjectClass,type)||type!=expectedClass||
           !cfgspawn::ObjectSlot(r,registry,returned,last)||!(last==first)||
           !cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,again)||!(again==registry))return false;
        pawn=first;world=plan.world;pawnClass=expectedClass;requested=location;deadline=now+3000;pending=true;return true;
    }
    template<class Reader> bool Current(Reader& r,const cfgobserver::Plan& plan,std::uint64_t now)const{
        cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation fresh{};
        std::uintptr_t type=0;
        return pending&&plan.world==world&&now<=deadline&&
            cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgspawn::ObjectSlot(r,registry,pawn.object,fresh)&&fresh==pawn&&
            r.Read(pawn.object+cfgprobe::ObjectClass,type)&&type==pawnClass;
    }
};
}
