#include "BundleObservations.hpp"
#include "OwnedCounselorTemplate.hpp"
#define main CfgFixtureResourceTestMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    {
        Fixture resource;cfgspawn::TemplateObservation t;
        cfgspawn::TypeObservations<Fixture> types(resource);
        CHECK(cfgspawn::ObserveTemplate(resource,types,resource.objects,resource.classes,resource.bot,1,t));
        const auto verify=[&](Fixture value){value.reads=0;return cfgbundle::OwnedCounselorTemplate(value,value.objects,
            value.bot,t,value.counselorBase,value.classes[cfgspawn::CounselorAI]);};
        CHECK(t.classSlot.serial==0&&t.defaultSlot.serial==0&&verify(resource));
        for(unsigned change=0;change<7;++change){
            auto bad=resource;
            if(change==0)bad.Put(bad.bot+cfgprobe::ObjectClass,bad.pawnClass);
            if(change==1)bad.Put(t.pawnClass+cfgspawn::ClassDefaultObject,bad.defaults);
            if(change==2)bad.Put(t.defaults+cfgprobe::ObjectClass,bad.pawnClass);
            if(change==3)bad.Put(t.defaults+cfgspawn::PawnAIClass,bad.classes[cfgspawn::KillerAI]);
            if(change==4)bad.Put(bad.objects.data+static_cast<std::uintptr_t>(t.classSlot.index)*24,bad.botClass+0x4000);
            if(change==5)bad.Put(bad.objects.data+static_cast<std::uintptr_t>(t.defaultSlot.index)*24+8,std::uint32_t{0x20000000});
            if(change==6)bad.Put(t.pawnClass+cfgprobe::ClassSuper,bad.killerBase);
            CHECK(!verify(bad));
        }
        auto mutation=resource;mutation.trigger=t.pawnClass+cfgspawn::ClassDefaultObject;
        mutation.mutation=mutation.trigger;mutation.newPointer=resource.defaults;mutation.triggerAt=2;
        CHECK(!verify(mutation));
        auto assigned=resource;
        const auto classSerialAt=resource.objects.data+static_cast<std::uintptr_t>(t.classSlot.index)*24+16;
        const auto defaultSerialAt=resource.objects.data+static_cast<std::uintptr_t>(t.defaultSlot.index)*24+16;
        assigned.Put(classSerialAt,std::int32_t{77});assigned.Put(defaultSerialAt,std::int32_t{88});
        CHECK(verify(assigned)); // native zero -> positive, with two identical owned samples
        auto changingSerial=resource;changingSerial.trigger=classSerialAt;changingSerial.mutation=classSerialAt;
        changingSerial.newPointer=77;changingSerial.triggerAt=2;CHECK(!verify(changingSerial));
        auto changedDefault=resource;changedDefault.trigger=defaultSerialAt;changedDefault.mutation=defaultSerialAt;
        changedDefault.newPointer=88;changedDefault.triggerAt=2;CHECK(!verify(changedDefault));
        t.classSlot.serial=77;t.defaultSlot.serial=88;CHECK(verify(assigned));
        assigned.Put(classSerialAt,std::int32_t{78});CHECK(!verify(assigned)); // positive IDs never relaxed
        auto bounded=resource;bounded.cap=5;CHECK(!verify(bounded));
    }
    Fixture f;const auto native=f.Object(),manager=f.Object(native);cfgprobe::lifetime::SlotObservation nativeSlot{};
    f.Put(f.module+cfghunter::disk::ManagerReflection,native);f.Put(f.module+cfghunter::disk::ManagerConstructor,native);
    f.Put(f.pawn+cfghunter::disk::Manager,manager);f.Put(manager+0x20,f.pawn);f.Put(manager+0xE0,f.pawn);
    CHECK(cfgspawn::ObjectSlot(f,f.objects,native,nativeSlot));f.reads=0;
    unsigned managerClause=99;
    CHECK(cfgbundle::Manager(f,f.module,f.objects,f.pawn,native,nativeSlot,&managerClause));CHECK(managerClause==0&&f.reads<100);
    auto assigned=f;
    auto startupSlot=nativeSlot;startupSlot.serial=0;
    const auto serialAt=f.objects.data+static_cast<std::uintptr_t>(nativeSlot.index)*24+16;
    assigned.Put(serialAt,std::int32_t{42});assigned.reads=0;
    CHECK(!cfgbundle::Manager(assigned,assigned.module,assigned.objects,assigned.pawn,native,startupSlot,&managerClause));
    CHECK(managerClause==4);
    assigned.reads=0;
    auto currentSlot=startupSlot;
    CHECK(cfgbundle::Manager(assigned,assigned.module,assigned.objects,assigned.pawn,native,startupSlot,&managerClause,&currentSlot));
    CHECK(managerClause==0&&currentSlot.serial==42);
    assigned.Put(serialAt,std::int32_t{43});assigned.reads=0;
    CHECK(!cfgbundle::Manager(assigned,assigned.module,assigned.objects,assigned.pawn,native,currentSlot,&managerClause,&currentSlot));
    CHECK(managerClause==4&&currentSlot.serial==42);
    static_assert(cfghunter::disk::ManagerReflection==0x0304D398&&cfghunter::disk::ManagerConstructor==0x0304D3A0);
    auto stale=f;stale.Put(f.module+cfghunter::disk::ManagerReflection,std::uintptr_t{});
    stale.Put(f.module+0x0304BF58,native);stale.Put(f.module+0x0304BF60,native);stale.reads=0;
    managerClause=0;CHECK(!cfgbundle::Manager(stale,stale.module,stale.objects,stale.pawn,native,nativeSlot,&managerClause));CHECK(managerClause==2);
    const auto checkManager=[&](Fixture value){value.reads=0;return cfgbundle::Manager(value,value.module,value.objects,value.pawn,native,nativeSlot);};
    auto wrong=f;wrong.Put(manager+0xE0,f.bot);CHECK(!checkManager(wrong));
    wrong=f;wrong.Put(manager+0x20,f.bot);CHECK(!checkManager(wrong));
    wrong=f;wrong.Put(manager+0x10,f.classes[cfgspawn::Tree]);CHECK(!checkManager(wrong));
    wrong=f;wrong.Put(f.pawn+cfghunter::disk::Manager,std::uintptr_t{0});CHECK(!checkManager(wrong));
    wrong=f;wrong.cap=5;CHECK(!checkManager(wrong));
    wrong=f;wrong.trigger=f.pawn+cfghunter::disk::Manager;wrong.mutation=wrong.trigger;wrong.newPointer=f.bot;wrong.triggerAt=2;CHECK(!checkManager(wrong));
    bool alive=false;f.reads=0;f.Put(f.pawn+cfghunter::disk::Health,100.f);CHECK(cfgbundle::Alive(f,f.pawn,alive)&&alive);
    f.Put(f.pawn+cfghunter::disk::Health,cfghunter::disk::AliveThreshold);CHECK(cfgbundle::Alive(f,f.pawn,alive)&&!alive);
    f.Put(f.pawn+cfghunter::disk::Health,std::numeric_limits<float>::quiet_NaN());CHECK(!cfgbundle::Alive(f,f.pawn,alive));
    const auto table=std::uintptr_t{0x500000};f.Put(f.pawn,table);
    f.Put(table+cfghunter::disk::PressSlot,f.module+cfghunter::disk::Press);
    f.Put(table+cfghunter::disk::ReleaseSlot,f.module+cfghunter::disk::Release);
    f.Put(table+cfghunter::disk::CanAttackSlot,f.module+cfghunter::disk::CanAttack);
    f.Put(f.module+cfghunter::disk::Press,cfghunter::disk::PressBytes);
    f.Put(f.module+cfghunter::disk::Release,cfghunter::disk::ReleaseBytes);
    f.Put(f.module+cfghunter::disk::CanAttack,cfghunter::disk::CanAttackBytes);
    f.Put(f.module+cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes);
    f.Put(f.module+cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes);f.reads=0;
    CHECK(cfgbundle::NativeBindings(f,f.module,f.pawn));
    wrong=f;wrong.reads=0;wrong.Put(table+cfghunter::disk::CanAttackSlot,std::uintptr_t{});
    CHECK(!cfgbundle::NativeBindings(wrong,wrong.module,wrong.pawn));
    wrong.reads=0;CHECK(cfgbundle::PressBinding(wrong,wrong.module,wrong.pawn));
    wrong=f;wrong.reads=0;wrong.Put(f.module+cfghunter::disk::Press,std::uint8_t{0});
    CHECK(!cfgbundle::NativeBindings(wrong,wrong.module,wrong.pawn));
    wrong.reads=0;CHECK(!cfgbundle::PressBinding(wrong,wrong.module,wrong.pawn));
    // Unrelated press/stun code cannot prevent the paired safe release.
    wrong.reads=0;CHECK(cfgbundle::ReleaseBinding(wrong,wrong.module,wrong.pawn));
    wrong=f;wrong.reads=0;wrong.Put(table+cfghunter::disk::ReleaseSlot,f.module+cfghunter::disk::Press);
    CHECK(!cfgbundle::ReleaseBinding(wrong,wrong.module,wrong.pawn));
    std::printf("CFG bundle production manager/health/code/release guards: %s (%d failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
