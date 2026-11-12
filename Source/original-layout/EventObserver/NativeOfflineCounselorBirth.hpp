#pragma once
#include "NativeOfflineOwnerObservation.hpp"
#include "NativePopulationCalls.hpp"
#include "ControllerOwnedBotState.hpp"
#include "SpawnAssetObservation.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/OfflineSettingsBindings.generated.hpp"
namespace cfgmenu::offlinepopulation {
struct Born {
    unsigned phase=0,travelRefusal=0; // travelRefusal: Calls::TravelRefusal, 7=post-travel bot proof
    std::uintptr_t controller=0,state=0,pawn=0;bool entered=false,ready=false,accounted=false;
    std::array<cfgprobe::lifetime::SlotObservation,3> identities{};
};
// Native controller-first population used by the donor. A preloaded, exact
// selected counselor is supplied by the route; this class never chooses one.
template<class Reader> class Birth {
    Reader& r_;promotion::Context owner_;population::Calls calls_;Born result_{};
    bool attempted_=false;std::uintptr_t selected_=0;promotion::Packet packet_{};
    bool Root(){
        cfgprobe::lifetime::ArrayHeader fresh{};
        if(!cfgprobe::lifetime::ReadArrayHeader(r_,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh)||
           !cfgprobe::lifetime::RevalidateBatch(r_,owner_.module+cfgprobe::lifetime::ObjectArrayRva,fresh,owner_.identities))return false;
        owner_.registry=fresh;return offlineowner::Current(r_,owner_,GetCurrentThreadId());
    }
    bool Bot(bool pawn){
        if(!Root()||!result_.controller||!result_.state)return false;
        cfgprobe::lifetime::SlotObservation slot{};
        if(result_.identities[0].object!=result_.controller||result_.identities[1].object!=result_.state||
           !ControllerOwnedState(r_,owner_,result_.identities[0],result_.identities[1]))return false;
        return !pawn||(result_.pawn&&offlineowner::Same(r_,result_.controller+cfgprobe::ControllerPawn,result_.pawn)&&
            offlineowner::Same(r_,result_.pawn+0x3A0,result_.controller)&&offlineowner::Same(r_,result_.pawn+0x388,result_.state)&&
            offlineowner::Same(r_,result_.pawn+cfgprobe::ObjectClass,selected_)&&
            cfgprobe::lifetime::ReadSlot(r_,owner_.registry,result_.pawn,slot)&&slot==result_.identities[2]);
    }
    bool Account(){
        std::uintptr_t type=0;cfgreflection::Field field;std::int32_t dimension=0,bytes=0,offset=0,count=0;
        if(!Bot(true)||!r_.Read(owner_.module+cfgofflinesettings::disk::GameModeClass,type)||
           !offlineowner::Same(r_,owner_.module+cfgofflinesettings::disk::GameModeConstructor,type)||
           !cfgspawn::ObjectIs(r_,owner_.mode,type))return false;
        cfgreflection::Observation<Reader> reflection(r_,owner_.module);
        if(!reflection.Find(type,u"NumBots",field,false)||field.kind.View()!=u"IntProperty"||
           !r_.Read(field.object+0x30,dimension)||dimension!=1||!r_.Read(field.object+0x34,bytes)||bytes!=4||
           !r_.Read(field.object+0x44,offset)||offset!=populationdisk::NumBots||
           !r_.Read(owner_.mode+populationdisk::NumBots,count)||count<0||count>=31||!Bot(true))return false;
        const auto address=owner_.mode+populationdisk::NumBots;MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||m.Protect!=PAGE_READWRITE)return false;
        if(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(address),count+1,count)!=count)return false;
        result_.accounted=true;return Bot(true);
    }
public:
    Birth(Reader& reader,const promotion::Context& owner):r_(reader),owner_(owner),
        calls_(owner.module,owner.world,owner.mode,owner.thread,owner.exactImageVerified){}
    const Born& Report()const{return result_;}
    bool Run(std::uintptr_t loadedClass,const promotion::Packet& plain){
        result_.phase=1;
        if(attempted_||!Root()||!cfgending::EmptySubpath(plain.bytes)||!cfgprobe::ObjectPointer(loadedClass))return false;
        attempted_=true;selected_=loadedClass;packet_=plain;
        std::uintptr_t counselor=0;
        result_.phase=2;
        if(!r_.Read(owner_.module+birthdisk::SCCounselorCharacterClass,counselor)||
           !offlineowner::Same(r_,owner_.module+birthdisk::SCCounselorCharacterConstructor,counselor)||
           !classchoice::MatchesPath(r_,owner_.module,selected_,counselor,plain.bytes))return false;
        result_.phase=3;const auto created=calls_.SpawnController(r_,[&](){return Root();});
        result_.entered=created.entered;result_.controller=created.controller;
        if(!created.completed||!created.current||!result_.controller||result_.controller==owner_.controller)return false;
        result_.phase=4;if(!r_.Read(result_.controller+0x388,result_.state)||!result_.state||result_.state==owner_.playerState)return false;
        result_.phase=5;if(!cfgprobe::lifetime::ReadSlot(r_,owner_.registry,result_.controller,result_.identities[0])||!result_.identities[0].HasAssignedSerial())return false;
        // Do not reject a normal newborn state solely for its still-unassigned
        // weak serial. Bot() proves the typed, controller-owned state before
        // and after native calls and latches any native serial assignment.
        result_.phase=6;if(!cfgprobe::lifetime::ReadSlot(r_,owner_.registry,result_.state,result_.identities[1]))return false;
        result_.phase=7;if(!Bot(false))return false;
        auto bot=owner_;bot.controller=result_.controller;bot.playerState=result_.state;bot.selectedClass=loadedClass;
        bot.picked=plain;bot.pickedResolutionVerified=true;
        promotion::WindowsCalls native(owner_.module,owner_.thread,true);auto consumed=plain;
        result_.phase=8;if(!native.Consume(bot,consumed)||!Bot(false))return false;
        std::array<std::uintptr_t,1> pawnOutput{};
        result_.phase=9;if(!promotion::detail::Code(owner_.module,owner_.module+offlinelifecycledisk::OfflineBotsRestartPlayer,offlinelifecycledisk::OfflineBotsRestartPlayerCode)||
           !promotion::detail::Code(owner_.module,owner_.module+offlinelifecycledisk::ParentRestartPlayer,offlinelifecycledisk::ParentRestartPlayerCode)||
           !startupretain::Invoke(owner_.module,startupretain::Call::RestartBot,promotion::RetainedInputs(bot),pawnOutput,[&](){
               promotion::CurrentProcessReader completion;
               return promotion::detail::Two(owner_.module+offlinelifecycledisk::OfflineBotsRestartPlayer,owner_.mode,reinterpret_cast<void*>(result_.controller))&&
                   completion.Read(result_.controller+cfgprobe::ControllerPawn,pawnOutput[0]);})||!Bot(false))return false;
        result_.phase=10;if(!r_.Read(result_.controller+cfgprobe::ControllerPawn,result_.pawn)||!result_.pawn)return false;
        result_.phase=11;if(!cfgprobe::lifetime::ReadSlot(r_,owner_.registry,result_.pawn,result_.identities[2])||!result_.identities[2].HasAssignedSerial())return false;
        result_.phase=12;if(!Bot(true))return false;
        bool ready=false;
        result_.phase=13;
        if(!calls_.HasFullyTraveled(result_.state,[&](){return Bot(true);},ready)){result_.travelRefusal=calls_.TravelRefusal();return false;}
        if(!Bot(true)){result_.travelRefusal=7;return false;}
        // Donor treats native readiness as observation, not a second birth request.
        result_.ready=ready;result_.phase=14;if(!Account())return false;result_.phase=15;return true;
    }
};
}
