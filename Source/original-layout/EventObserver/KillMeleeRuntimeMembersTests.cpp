#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#undef CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "KillRuntime.hpp"
#include "NativeOfflineSettings.hpp"
struct MeleeFixture:SpectatorFixture {
    cfgkill::Runtime bundleKill;
    cfgprobe::lifetime::SlotObservation bundleKillActorSlot{},bundleKillInteractSlot{},bundleKillComponentSlot{};
    cfgkill::Context scenario{};unsigned writes=0;bool living=true,writeOkay=true;
    bool KnownLiving(std::uintptr_t pawn){return living&&pawn==r_.bot;}
    cfgofflinesettings::Write ExchangeByte(std::uintptr_t address,unsigned char expected,unsigned char desired){
        unsigned char before=2;++writes;
        if(!writeOkay||!r_.Read(address,before)||before!=expected)return cfgofflinesettings::Write::Refused;
        r_.Put(address,desired);return cfgofflinesettings::Write::Changed;
    }
#include "KillMeleeRuntimeMembers.hpp"
    MeleeFixture(){
        r_.cap=100000;r_.reads=0;const auto type=r_.Object(),killType=r_.Object();
        r_.Put(type+cfgprobe::ClassSuper,std::uintptr_t{});
        r_.Put(plan.module+cfgkill::disk::SCInteractComponentReflection,type);
        r_.Put(plan.module+cfgkill::disk::SCInteractComponentConstructor,type);
        scenario={r_.Object(),r_.Object(type),r_.Object(killType),false};
        Serial(scenario.actor);Serial(scenario.interactable);Serial(scenario.killComponent);
        cfgspawn::ObjectSlot(r_,context.objectArray,scenario.actor,bundleKillActorSlot);
        cfgspawn::ObjectSlot(r_,context.objectArray,scenario.interactable,bundleKillInteractSlot);
        cfgspawn::ObjectSlot(r_,context.objectArray,scenario.killComponent,bundleKillComponentSlot);
        r_.Put(scenario.interactable+0x20,scenario.actor);r_.Put(scenario.interactable+0xE0,scenario.actor);
        r_.Put(scenario.killComponent+0x20,scenario.actor);r_.Put(scenario.killComponent+0xE0,scenario.actor);
        r_.Put(scenario.actor+cfgkill::disk::InteractComponent,scenario.interactable);
        r_.Put(scenario.actor+cfgkill::disk::ContextKillComponent,scenario.killComponent);
        r_.Put(bundleJason.pawn.object+cfgkill::disk::JasonKillObject,scenario.actor);
        r_.Put(scenario.interactable+cfgkill::disk::bCanAttackToInteract,std::uint8_t{});
        r_.Put(scenario.killComponent+cfgkill::disk::bCanAttackToInteract,std::uint8_t{0xA7}); // never a CFG property on this type
        r_.Put(scenario.killComponent+cfgkill::disk::bIsKillActive,std::uint8_t{});
        r_.Put(plan.module+cfgkill::disk::bCanAttackToInteractSetter,cfgkill::disk::bCanAttackToInteractSetterCode);
        bundleKill.policy.context=scenario.actor;bundleKill.policy.finisher=r_.bot;r_.reads=512;r_.cap=1536;
    }
};
int main(){unsigned peak=0;
    {MeleeFixture f;CHECK(f.ArmFinishingStrike(f.scenario,f.r_.bot)&&f.writes==1);
     unsigned char value=0;CHECK(f.r_.Read(f.scenario.interactable+cfgkill::disk::bCanAttackToInteract,value)&&value==1);
     CHECK(f.r_.Read(f.scenario.killComponent+cfgkill::disk::bCanAttackToInteract,value)&&value==0xA7);
     CHECK(f.ArmFinishingStrike(f.scenario,f.r_.bot)&&f.writes==1);peak=f.r_.reads;}
    for(unsigned fault=0;fault<10;++fault){MeleeFixture f;
     if(fault==0)f.scenario.active=true;
     if(fault==1)f.living=false;
     if(fault==2)f.r_.Put(f.scenario.interactable+cfgprobe::ObjectClass,std::uintptr_t{});
     if(fault==3)f.r_.Put(f.scenario.killComponent+cfgkill::disk::bIsKillActive,std::uint8_t{1});
     if(fault==4)f.r_.Put(f.scenario.interactable+0xE0,f.plan.pawn);
     if(fault==5)f.r_.Put(f.scenario.actor+cfgkill::disk::InteractComponent,f.plan.pawn);
     if(fault==6)f.r_.Put(f.plan.module+cfgkill::disk::bCanAttackToInteractSetter,std::uint8_t{});
     if(fault==7)f.r_.Put(f.scenario.interactable+cfgkill::disk::bCanAttackToInteract,std::uint8_t{2});
     if(fault==8)f.r_.cap=f.r_.reads+5;
     if(fault==9)f.r_.Put(f.bundleJason.pawn.object+cfgkill::disk::JasonKillObject,std::uintptr_t{});
     CHECK(!f.ArmFinishingStrike(f.scenario,f.r_.bot)&&!f.writes);
    }
    {MeleeFixture f;f.writeOkay=false;CHECK(!f.ArmFinishingStrike(f.scenario,f.r_.bot)&&f.writes==1);}
    std::printf("Included CFG melee-finish arming: %d failures peak%u/1536 incl512 prior reads; no game access.\n",failures,peak);
    return failures?1:0;
}
