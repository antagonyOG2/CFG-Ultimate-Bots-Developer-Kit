#define main HoldSparseFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "JasonInstanceObservation.hpp"
#include "VehicleOccupancyObservation.hpp"
#include "CounselorTerminalObservation.hpp"
#include "../Core/HeldRecoveryPolicy.hpp"
#include "../Core/GrabCombatPolicy.hpp"
#include "../Core/VehicleActionPolicy.hpp"
#include "../RuntimeProbe/GrabBindings.generated.hpp"
#include "../RuntimeProbe/AiActionBindings.generated.hpp"
namespace cfgclosedtrace {inline void Mark(const char*,std::uintptr_t=0,std::uint64_t=0){}}
struct HoldFixture {
    Fixture r_;cfgobserver::Plan plan{};struct{cfgprobe::lifetime::ArrayHeader objectArray;}context;
    cfgjason::NewActorEvidence bundleJason;cfgholdrecovery::Policy bundleHeldRecovery;cfggrab::Policy bundleGrab;
    cfgvehicleaction::State bundleVehicleActions;cfgvehicle::Pursuit bundleCarPursuit;
    struct{std::uint64_t selectAt=0;}bundleSchedule;
    unsigned bundleCarActionPending=0,bundleRoadMorph=0,calls=0,cancels=0,unlocks=0,retired=0;
    std::uintptr_t bundleCounselorClass=0,fixtureCar=0,fixtureSeat=0,seatType=0,fixtureManager=0;
    bool scope=true,nativeKilling=false,canKill=false,death=false,clearOnCancel=true,seated=false,donorHeld=false;
    bool DonorHeldDecisionInstalled(){return donorHeld;}
    HoldFixture(bool carHold=false):seated(carHold){r_.cap=100000;plan.module=r_.module;context.objectArray=r_.objects;
        bundleCounselorClass=r_.counselorBase;bundleJason.pawn.object=r_.pawn;
        r_.Put(r_.pawn+cfghunter::disk::Held,r_.bot);r_.Put(r_.pawn+cfgaction::disk::ExtractionComponent,std::uintptr_t{});
        r_.Put(r_.bot+cfghunter::disk::Health,100.f);r_.Put(r_.bot+cfgescape::disk::bHasEscaped,std::uint8_t{});
        r_.Put(r_.botState+cfgescape::disk::bDead,std::uint8_t{});r_.Put(r_.botState+cfgescape::disk::bEscaped,std::uint8_t{});
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateReflection,r_.classes[cfgspawn::PlayerState]);
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,r_.classes[cfgspawn::PlayerState]);
        const auto serial=[&](std::uintptr_t object){int index=0;r_.Read(object+0xC,index);r_.Put(r_.objects.data+24ULL*index+16,1);};
        serial(r_.bot);fixtureManager=r_.Object();r_.Put(r_.pawn+cfghunter::disk::Manager,fixtureManager);
        fixtureCar=r_.Object();seatType=r_.Object();fixtureSeat=r_.Object(seatType);serial(fixtureCar);serial(fixtureSeat);
        r_.Put(fixtureSeat+cfgvehicleinteraction::disk::ParentVehicle,fixtureCar);r_.Put(fixtureSeat+cfgvehicleoccupancy::disk::DriverSeat,std::uint8_t{1});
        r_.Put(fixtureSeat+cfgvehicleinteraction::disk::CounselorInSeat,r_.bot);
        r_.Put(r_.bot+cfgvehicleoccupancy::disk::LeavingSeat,std::uint8_t{});
        r_.Put(r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,carHold?fixtureSeat:0);
        r_.Put(fixtureManager+cfgvehicleinteraction::disk::LockedComponent,carHold?fixtureSeat:0);
        r_.Put(0x900000,fixtureSeat);r_.Put(fixtureCar+cfgvehicleinteraction::disk::Seats,cfgspawn::PlayerArray{0x900000,1,1});
        bundleGrab.Observe(r_.bot,1000);
        if(carHold){bundleVehicleActions.Select(fixtureCar,r_.bot);bundleVehicleActions.extractionCommitted=true;bundleVehicleActions.SeatDispatched(1000);}
        r_.cap=1536;r_.reads=0;
    }
    bool Context(){return scope;}bool Jason(){return scope;}
    bool Held(std::uintptr_t& value){return r_.Read(r_.pawn+cfghunter::disk::Held,value);}
    bool Manager(){return scope;}bool HidingSafe(){return scope&&!death;}
    unsigned Reads()const{return r_.reads;}
    bool NativeClass(std::uintptr_t,std::uintptr_t,std::uintptr_t& type){type=seatType;return scope;}
    bool CarSource(std::uintptr_t,std::uintptr_t value){return scope&&value==fixtureCar;}
    bool CarOccupancy(std::uintptr_t value,cfgvehicleoccupancy::Proof& proof){return cfgvehicleoccupancy::Observe(r_,context.objectArray,value,seatType,bundleCounselorClass,proof)!=cfgvehicleoccupancy::Result::Unknown;}
    bool CarOccupancyCurrent(const cfgvehicleoccupancy::Proof& proof){return cfgvehicleoccupancy::Current(r_,context.objectArray,seatType,bundleCounselorClass,proof);}
    void CarRetire(std::uint64_t){++retired;bundleVehicleActions={};}
    template<class B,std::size_t N>bool GrabBool(const B& binding,std::uintptr_t,const std::array<unsigned char,N>&,bool& value){
        ++calls;value=binding.factory==cfggrab::disk::IsGrabKilling.factory?nativeKilling:canKill;return calls<=12;
    }
    template<class B,std::size_t N>bool CarCall(const B& binding,std::uintptr_t,const std::array<unsigned char,N>&,std::uintptr_t,void*){
        ++calls;if(calls>12)return false;
        if(binding.factory==cfgaction::disk::CLIENT_CancelInteractAttempt.factory){++cancels;
            if(clearOnCancel)r_.Put(r_.pawn+cfghunter::disk::Held,std::uintptr_t{});}
        if(binding.factory==cfgaction::disk::CLIENT_UnlockInteraction.factory){++unlocks;r_.Put(fixtureManager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{});}
        return true;
    }
#include "HeldRecoveryRuntimeMembers.hpp"
    void Tick(std::uint64_t now){calls=0;r_.reads=0;HeldRecoveryPhase(now);CHECK(calls<=12&&r_.reads<=1536);}
};
int main(){
    {HoldFixture f;f.Tick(1000);f.Tick(3500);CHECK(f.cancels==1);f.Tick(3750);f.Tick(4000);
     CHECK(f.bundleHeldRecovery.confirmed==1&&f.retired==1&&!f.unlocks);}
    {HoldFixture f(true);f.Tick(1000);f.Tick(6500);CHECK(f.cancels==1);f.Tick(6750);CHECK(f.unlocks==1);
     f.Tick(7000);CHECK(f.bundleHeldRecovery.confirmed==1&&!f.retired&&f.bundleVehicleActions.extractionCommitted&&!f.bundleVehicleActions.seatInputAt);}
    {HoldFixture f(true);f.clearOnCancel=false;f.Tick(1000);f.Tick(6500);f.Tick(6750);f.Tick(9000);f.Tick(12000);
     CHECK(f.cancels==1&&f.bundleHeldRecovery.phase==cfgholdrecovery::Phase::Refused&&!f.retired);}
    for(unsigned fault=0;fault<4;++fault){HoldFixture f;f.Tick(1000);
        if(fault==0)f.death=true;if(fault==1)f.nativeKilling=true;if(fault==2)f.scope=false;
        if(fault==3)f.r_.Put(f.r_.botState+cfgescape::disk::bDead,std::uint8_t{1});
        f.Tick(10000);CHECK(!f.cancels&&!f.unlocks&&!f.retired);}
    {HoldFixture f(true);f.Tick(1000);f.donorHeld=true;
        f.r_.Put(f.r_.bot+cfgvehicleoccupancy::disk::CurrentSeat,std::uintptr_t{});
        f.r_.Put(f.fixtureSeat+cfgvehicleinteraction::disk::CounselorInSeat,std::uintptr_t{});
        f.r_.Put(f.fixtureManager+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{});
        f.Tick(1250);CHECK(f.retired==1&&!f.bundleHeldRecovery.car&&!f.cancels&&!f.unlocks);
    }
    {HoldFixture f;f.donorHeld=true;f.Tick(1000);f.Tick(10000);CHECK(!f.cancels&&!f.unlocks&&!f.retired);}
    std::printf("Included hold-recovery adapter: %d failures; native calls mocked, positive slots and fixtureSeat samples real fixture reads; J2 owns ordinary hold while detached-car retirement remains live.\n",failures);
    return failures?1:0;
}
