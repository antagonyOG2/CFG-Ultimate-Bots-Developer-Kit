#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#include "NativeJasonControllerTickObservation.hpp"
#include "DonorCounselorContextObservation.hpp"
#define main PriorCounselorResultMain
#include "SpawnResultTests.cpp"
#undef main
struct TickFixture:Fixture {
    cfgjasontick::Owner owner{};
    const std::uintptr_t killer=0x40040000,killerClass=0x50040000;
    TickFixture(){
        using namespace cfgprobe;cap=1024;
        Put(plan.module+cfgprimary::SerializationFlag,std::uint8_t{0});
        Put(plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{0});
        owner.source=plan;owner.pawnClass=pawnClass;owner.defaults=0x50050000;
        owner.killerBase=counselorClass;owner.killerAI=killerClass;owner.pathClass=brainClass;
        Slot(killer,42,503);Slot(brain,43,504);
        auto arrayHeader=cfgprobe::lifetime::ArrayHeader{array,64};
        cfgspawn::ObjectSlot(*this,arrayHeader,pawn,owner.actor.pawn);
        cfgspawn::ObjectSlot(*this,arrayHeader,controller,owner.actor.initialController);
        cfgspawn::ObjectSlot(*this,arrayHeader,killer,owner.actor.killerController);
        cfgspawn::ObjectSlot(*this,arrayHeader,brain,owner.actor.path);
        Put(killer+ObjectClass,killerClass);Put(killerClass+ClassSuper,aiClass);
        Put(pawn+cfgspawn::PawnAIClass,killerClass);Put(pawn+0x3A0,killer);Put(killer+0x370,pawn);
        Put(killer+0x410,brain);Put(brain+0x20,killer);Put(brain+cfgjason::disk::ComponentOwner,killer);
        const std::uintptr_t level=0x40060000,levelClass=0x50060000;
        Slot(level,44,505);Slot(levelClass,45,0);
        Put(level+ObjectClass,levelClass);Put(levelClass+ClassSuper,std::uintptr_t{0});
        Put(plan.module+cfgkill::disk::LevelReflection,levelClass);Put(plan.module+cfgkill::disk::LevelConstructor,levelClass);
        Put(pawn+0x20,level);Put(killer+0x20,level);Put(level+cfgkill::disk::OwningWorld,plan.world);
        const auto& binding=cfgcompat::cfg35::ai::Classes[3];
        Put(plan.module+binding.reflection,killerClass);Put(plan.module+binding.constructor,killerClass);
        reads=0;
    }
    bool Verify(std::uint32_t thread=77){reads=0;return cfgjasontick::ObserveOwner(*this,owner,thread);}
};
#ifndef CFG35_NATIVE_TICK_FIXTURE_ONLY
int main(){const int earlier=PriorCounselorResultMain();unsigned checks=0;
    const auto check=[&](bool value){++checks;if(!value)std::printf("Native Tick check %u failed\n",checks);CHECK(value);};
    // SPChallenges uses the human's possessed Jason, not an owned AI receiver.
// Test the actual ObserveOwner exclusion, independently of a mode label.
for(unsigned human=0;human<2;++human){TickFixture f;
    if(human==0)f.owner.actor.pawn=f.owner.source.objects[3];
    else f.owner.actor.killerController=f.owner.source.objects[2];
    check(!f.Verify());}
TickFixture good;check(good.Verify());const auto readCount=good.reads;check(readCount<256);
    for(unsigned fault=0;fault<17;++fault){TickFixture f;
        switch(fault){
        case 0:++f.owner.source.threadId;break;
        case 1:f.Put(f.plan.viewport+cfgprobe::ViewportWorld,std::uintptr_t{0x777000});break;
        case 2:f.Put(f.plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{0x777000});break;
        case 3:f.Put(f.array+40*24+16,std::int32_t{999});break;
        case 4:f.Put(f.array+42*24+16,std::int32_t{999});break;
        case 5:f.Put(f.pawn+0x3A0,f.controller);break;
        case 6:f.Put(f.killer+0x370,f.plan.pawn);break;
        case 7:f.Put(f.brain+0x20,f.plan.controller);break;
        case 8:f.Put(f.plan.module+cfgprimary::SerializationFlag,std::uint8_t{1});break;
        case 9:f.Put(f.plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});break;
        case 10:f.Put(f.plan.module+cfgcompat::cfg35::ai::Classes[3].constructor,f.aiClass);break;
        case 11:f.owner.actor.killerController.serial=0;break;
        case 12:f.Put(f.array+0*24+16,std::int32_t{999});break;
        case 13:f.Put(f.array+1*24+8,std::uint32_t{0x20000000});break;
        case 14:f.Put(std::uintptr_t{0x40060000}+cfgkill::disk::OwningWorld,std::uintptr_t{0x777000});break;
        case 15:f.Put(f.killer+0x20,f.plan.world);break;
        case 16:f.Put(f.plan.module+cfgkill::disk::LevelConstructor,std::uintptr_t{0x777000});break;
        }
        check(!f.Verify());
    }
    {TickFixture f;check(!f.Verify(78));check(!f.Verify(0));}
    for(unsigned cap=0;cap<readCount;++cap){TickFixture f;f.cap=cap;check(!f.Verify());}
    {TickFixture f;f.Slot(f.brain,43,0);f.owner.actor.path.serial=0;check(f.Verify());
        f.Put(f.brain+cfgjason::disk::ComponentOwner,f.plan.controller);check(!f.Verify());}
    {TickFixture f;f.trigger=f.array+42*24+16;f.mutation=f.trigger;f.replacement=999;
        check(!f.Verify());} // changed controller serial during observation
    unsigned contextChecks=0;
    for(unsigned value:{0u,1u,2u,255u}){TickFixture f;using namespace cfgcompat::cfg35::j1a;
        f.Put(f.plan.module+ContextKillSetter,ContextKillSetterCode);
        f.Put(f.pawn+CounselorContextKill,static_cast<std::uint8_t>(value));
        // Adjacent death-animation flag deliberately has the opposite value.
        f.Put(f.pawn+CounselorContextKill+1,static_cast<std::uint8_t>(value==0));
        bool active=false;const bool okay=cfgdonorhunt::ObserveCounselorContextKill(f,
            cfgprobe::lifetime::ArrayHeader{f.array,64},f.plan.module,f.owner.actor.pawn,f.counselorClass,active);
        check(okay==(value<2)&&(value>=2||active==(value==1)));++contextChecks;
    }
    for(unsigned fault=0;fault<4;++fault){TickFixture f;using namespace cfgcompat::cfg35::j1a;
        f.Put(f.plan.module+ContextKillSetter,ContextKillSetterCode);f.Put(f.pawn+CounselorContextKill,std::uint8_t{0});
        if(fault==0)f.owner.actor.pawn.serial=0;
        else if(fault==1)f.Put(f.plan.module+ContextKillSetter,std::uint8_t{0x90});
        else if(fault==2)f.Put(f.pawn+cfgprobe::ObjectClass,std::uintptr_t{0});
        else f.Put(f.array+40*24+16,std::int32_t{999});
        bool active=true;check(!cfgdonorhunt::ObserveCounselorContextKill(f,
            cfgprobe::lifetime::ArrayHeader{f.array,64},f.plan.module,f.owner.actor.pawn,f.counselorClass,active)&&!active);++contextChecks;
    }
    std::printf("Named counselor context-kill observation: %u checks; actual1070,adjacent1071 excluded,invalid bool/slot/type/code refused.\n",contextChecks);
    std::printf("Native Jason controller current-owner observation: %u checks, %u reads positive, %d failures; actual world/slot/type/reciprocal/path/GC predicates.\n",checks,readCount,failures);
    return earlier||failures?1:0;
}
#endif
