#include "../Core/DoorRecoveryPolicy.hpp"
#include "SelectedDoorObservation.hpp"
#define main CfgExistingDoorFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
int main(){
    static_assert(cfgdoor::disk::ComponentOwner==cfgcompat::cfg35::jasonApproach::ComponentOwner,
        "Selected door must share the verified ActorComponent.GetOwner profile");
    {cfgspawn::Vec3 goal{};
     CHECK(cfgdoor::Approach({0,0,100},{250,0,120},goal)&&goal.x==150&&goal.z==100);
     CHECK(cfgdoor::Approach({0,0,100},{-250,0,120},goal)&&goal.x==-150);
     CHECK(!cfgdoor::Approach({0,0,100},{180,0,100},goal));
     CHECK(!cfgdoor::Approach({0,0,100},{501,0,100},goal));
     CHECK(!cfgdoor::Approach({0,0,100},{250,0,351},goal));
     CHECK(!cfgdoor::Approach({NAN,0,0},{250,0,0},goal));}
    {cfgdoor::Recovery repeated;repeated.Stopped(1,100);repeated.Attempted(400);
     CHECK(!repeated.SelectedDue(2200,1,200.f*200.f));
     CHECK(!repeated.SelectedDue(10000,1,200.f*200.f));
     CHECK(repeated.SelectedDue(30400,1,200.f*200.f));
     CHECK(repeated.SelectedDue(30400,2,200.f*200.f));}
    {cfgdoor::BreakState door;
        CHECK(!door.CanRequestNative(1)); // unlocked attackable door must open, not break
        CHECK(door.CanRequestNativeOpen(1)&&!door.CanRequestNativeOpen(0)&&!door.CanRequestNativeOpen(2));
        door.barricaded=1;CHECK(door.CanRequestNative(1)&&!door.CanRequestNativeOpen(1));door.barricaded=0;
        door.locked=1;CHECK(door.CanRequestNative(1));
        CHECK(!door.CanRequestNativeOpen(1));
        CHECK(!door.CanRequestNative(0)&&!door.CanRequestNative(2));
        door.open=1;CHECK(!door.CanRequestNative(1)&&!door.CanRequestNativeOpen(1));door.open=0;
        door.destroyed=1;CHECK(!door.CanRequestNative(1));door.destroyed=0;
        door.wants=1;CHECK(door.CanRequestNative(1)); // captured PID13316: stale intent, no native owner/target
        door.wants=2;CHECK(!door.CanRequestNative(1));door.wants=0;
        door.target=1;CHECK(!door.CanRequestNative(1));door.target=0;
        door.killer=1;CHECK(!door.CanRequestNative(1));}
    {cfgdoor::Recovery decisive;CHECK(decisive.SelectedDue(0,1,250.f*250.f));
        decisive.Stopped(1,0);CHECK(!decisive.SelectedDue(24,1,250.f*250.f));
        CHECK(decisive.SelectedDue(25,1,250.f*250.f));decisive.Faced(300);
        CHECK(!decisive.SelectedDue(399,1,250.f*250.f)&&decisive.SelectedDue(400,1,250.f*250.f));
        decisive.Attempted(400);CHECK(!decisive.SelectedDue(30399,1,250.f*250.f));
        CHECK(decisive.SelectedDue(30400,1,250.f*250.f));
        decisive.Stopped(1,30400);CHECK(!decisive.SelectedDue(30800,2,400.f*400.f)&&!decisive.phase);
        CHECK(!decisive.SelectedDue(30900,2,NAN));}
    {cfgdoor::Recovery retry;retry.Stopped(1,0);retry.Attempted(25);retry.Unconfirmed(25);
        CHECK(!retry.SelectedDue(2024,1,250.f*250.f)&&retry.SelectedDue(2025,1,250.f*250.f));
        retry.NativeBreak(1,2025);const auto next=retry.next;retry.Unconfirmed(3000);
        CHECK(retry.breaking&&retry.next==next);}
    {cfgdoor::Recovery stalled;stalled.Stopped(1,100);stalled.Faced(400);
        CHECK(!stalled.DecisionExpired(3099)&&stalled.DecisionExpired(3100));
        stalled.YieldDecision(3100);CHECK(!stalled.phase&&!stalled.door&&!stalled.ContinuationDue(6100));
        CHECK(!stalled.SelectedDue(6099,1,200.f*200.f)&&stalled.SelectedDue(6100,1,200.f*200.f));
        stalled.NativeBreak(1,6200);CHECK(!stalled.DecisionExpired(10000)&&stalled.breaking);}
    cfgdoor::Recovery recovery;
    CHECK(recovery.Sample(0,{0,0,0},1000.f*1000.f)&&!recovery.blocked);
    CHECK(!recovery.Sample(1000,{0,0,0},1000.f*1000.f));
    CHECK(recovery.Sample(2000,{5,0,0},1000.f*1000.f)&&recovery.Due(2000));
    recovery.Attempted(2000);CHECK(!recovery.Due(9999)&&recovery.Due(10000));
    recovery.Traversal(3400);CHECK(!recovery.Due(6400));
    CHECK(recovery.Sample(4000,{100,0,0},1000.f*1000.f)&&!recovery.blocked);
    CHECK(recovery.Sample(6000,{100,0,0},100.f*100.f)&&!recovery.blocked);
    CHECK(!recovery.Sample(8000,{NAN,0,0},1000.f*1000.f));
    {cfgdoor::Recovery nearDoor;CHECK(nearDoor.Sample(0,{0,0,0},150.f*150.f));
        CHECK(nearDoor.Sample(2000,{50,0,0},150.f*150.f)&&nearDoor.Due(2000));
        CHECK(nearDoor.Sample(4000,{50,0,0},100.f*100.f)&&!nearDoor.blocked);}
    Fixture f;
    const auto componentClass=f.Object(),doorClass=f.Object(),component=f.Object(componentClass),door=f.Object(doorClass),manager=f.Object();
    // Strong, positive identities for the actor and selected component.
    for(const auto object:{component,door}){
        std::int32_t index=0;CHECK(f.Read(object+0xC,index));
        f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*24+16,std::int32_t{20+index});
    }
    f.Put(f.pawn+cfghunter::disk::Manager,manager);f.Put(manager+cfgdoor::disk::Best,component);
    // Actual PID18572 regression: manager E28 is NOT component ownership.
    // Literal native GetOwner E0 and the unrelated zero E28 are intentional
    // independent fixture evidence, not a mirror of the selected-door constant.
    f.Put(component+0xE0,door);f.Put(component+0xE28,std::uintptr_t{});f.Put(component+0x20,door);
    const auto observe=[&](Fixture value){value.reads=0;cfgdoor::Selected selected{};
        return cfgdoor::ObserveSelected(value,value.objects,value.pawn,doorClass,componentClass,selected);};
    CHECK(observe(f));
    {auto bad=f;bad.Put(component+0xE0,std::uintptr_t{});bad.Put(component+0xE28,door);CHECK(!observe(bad));}
    {auto b=f;const auto type=b.Object(),bar=b.Object(type);std::int32_t index=0;CHECK(b.Read(bar+0xC,index));
     b.Put(b.objects.data+static_cast<std::uintptr_t>(index)*24+16,std::int32_t{33});
     b.Put(door+cfgdoor::breakdisk::LinkedBarricade,bar);b.Put(bar+cfgdoor::breakdisk::LinkedDoor,door);
     b.Put(bar+cfgdoor::breakdisk::BarricadeEnabled,std::uint8_t{});b.Put(bar+0x3C1,std::uint8_t{1});
     cfgdoor::Selected selected;CHECK(cfgdoor::ObserveSelected(b,b.objects,b.pawn,doorClass,componentClass,selected));
     std::uint8_t enabled=2;CHECK(cfgdoor::ObserveBarricade(b,b.objects,selected,type,enabled)&&!enabled);
     b.Put(bar+cfgdoor::breakdisk::BarricadeEnabled,std::uint8_t{1});
     CHECK(cfgdoor::ObserveBarricade(b,b.objects,selected,type,enabled)&&enabled==1);
     auto foreign=b;foreign.Put(bar+cfgdoor::breakdisk::LinkedDoor,f.pawn);CHECK(!cfgdoor::ObserveBarricade(foreign,foreign.objects,selected,type,enabled));
     auto stale=b;stale.Put(stale.objects.data+static_cast<std::uintptr_t>(index)*24+16,std::int32_t{});
     CHECK(!cfgdoor::ObserveBarricade(stale,stale.objects,selected,type,enabled));
     auto changedBar=b;changedBar.trigger=door+cfgdoor::breakdisk::LinkedBarricade;changedBar.triggerAt=2;
     changedBar.mutation=changedBar.trigger;changedBar.newPointer=f.pawn;
     CHECK(!cfgdoor::ObserveBarricade(changedBar,changedBar.objects,selected,type,enabled));}
    {auto anchorFixture=f;const auto sceneClass=anchorFixture.Object();
     const auto anchor=anchorFixture.Object(componentClass);
     // Component inherits the verified SceneComponent type; the actor origin
     // is deliberately different from the native breakdown-facing anchor.
     anchorFixture.Put(componentClass+cfgprobe::ClassSuper,sceneClass);
     anchorFixture.Put(door+cfgdoor::breakdisk::InteractComponent,anchor);
     anchorFixture.Put(anchor+cfgdoor::disk::ComponentOwner,door);anchorFixture.Put(anchor+0x20,door);
     anchorFixture.Put(anchor+cfgposition::disk::SceneWorldTranslation,cfgspawn::Vec3{800,900,100});
     cfgdoor::Selected selected;CHECK(cfgdoor::ObserveSelected(anchorFixture,anchorFixture.objects,anchorFixture.pawn,doorClass,componentClass,selected));
     cfgspawn::Vec3 point;CHECK(cfgdoor::ObserveAnchor(anchorFixture,anchorFixture.objects,selected,componentClass,sceneClass,point));
     CHECK(point.x==800&&point.y==900&&point.z==100);
     anchorFixture.Put(component+cfgposition::disk::SceneWorldTranslation,cfgspawn::Vec3{100,200,100});
     CHECK(cfgdoor::ObserveSelectedAnchor(anchorFixture,anchorFixture.objects,selected,sceneClass,point));
     CHECK(point.x==100&&point.y==200); // unlocked Press faces selected component, not498 break anchor
     auto foreign=anchorFixture;foreign.Put(anchor+0x20,foreign.pawn);
     CHECK(!cfgdoor::ObserveAnchor(foreign,foreign.objects,selected,componentClass,sceneClass,point));
     auto moving=anchorFixture;moving.trigger=door+cfgdoor::breakdisk::InteractComponent;moving.triggerAt=2;
     moving.mutation=moving.trigger;moving.newPointer=component;
     CHECK(!cfgdoor::ObserveAnchor(moving,moving.objects,selected,componentClass,sceneClass,point));}
    {cfgdoor::Selected selected;CHECK(cfgdoor::ObserveSelected(f,f.objects,f.pawn,doorClass,componentClass,selected));
        CHECK(cfgdoor::NativeSelectionMatches(f,selected));}
    {cfgdoor::Selected selected;CHECK(cfgdoor::ObserveSelected(f,f.objects,f.pawn,doorClass,componentClass,selected));
     f.Put(f.pawn+cfgdoor::breakdisk::Target,door);f.Put(f.pawn+cfgdoor::breakdisk::Wants,std::uint8_t{1});
     f.Put(door+cfgdoor::breakdisk::Locked,std::uint8_t{1});f.Put(door+cfgdoor::breakdisk::Destroyed,std::uint8_t{0});
     f.Put(door+cfgdoor::breakdisk::DoorKiller,f.pawn);cfgdoor::BreakState state;
     std::uintptr_t nativeTarget=0;CHECK(cfgdoor::ObserveBreakTarget(f,f.pawn,nativeTarget)&&nativeTarget==door);
     auto completed=f;completed.Put(f.pawn+cfgdoor::breakdisk::Target,std::uintptr_t{});
     CHECK(cfgdoor::ObserveBreakTarget(completed,completed.pawn,nativeTarget)&&!nativeTarget);
     auto movingTarget=f;movingTarget.trigger=f.pawn+cfgdoor::breakdisk::Target;movingTarget.triggerAt=2;
     movingTarget.mutation=movingTarget.trigger;movingTarget.newPointer=f.bot;
     CHECK(!cfgdoor::ObserveBreakTarget(movingTarget,movingTarget.pawn,nativeTarget));
     CHECK(cfgdoor::ObserveBreak(f,f.objects,f.pawn,selected,doorClass,state)&&state.Owned(f.pawn,door));
     cfgdoor::Recovery native;native.NativeBreak(door,100);CHECK(native.breaking&&!native.ContinuationDue(259));
     CHECK(native.ContinuationDue(260));native.ObserveBreak(260);CHECK(native.breaking&&!native.ContinuationDue(419));
     native.EndBreak(420,true);CHECK(!native.breaking&&native.traversalUntil==3420);
     auto invalid=f;invalid.Put(door+cfgdoor::breakdisk::Locked,std::uint8_t{2});
     CHECK(!cfgdoor::ObserveBreak(invalid,invalid.objects,invalid.pawn,selected,doorClass,state));
     auto changedBreak=f;changedBreak.trigger=f.pawn+cfgdoor::breakdisk::Target;changedBreak.triggerAt=2;
     changedBreak.mutation=door+cfgdoor::breakdisk::DoorKiller;changedBreak.newPointer=f.bot;
     CHECK(!cfgdoor::ObserveBreak(changedBreak,changedBreak.objects,changedBreak.pawn,selected,doorClass,state));}
    auto zeroComponent=f;std::int32_t componentIndex=0;CHECK(f.Read(component+0xC,componentIndex));
    zeroComponent.Put(f.objects.data+static_cast<std::uintptr_t>(componentIndex)*24+16,std::int32_t{0});CHECK(observe(zeroComponent));
    for(unsigned n=0;n<7;++n){auto bad=f;
        if(n==0)bad.Put(component+cfgdoor::disk::ComponentOwner,f.pawn);
        if(n==1)bad.Put(component+0x20,f.pawn);
        if(n==2)bad.Put(door+0x10,f.killerBase);
        if(n==3)bad.Put(component+0x10,f.killerBase);
        if(n==4)bad.Put(manager+cfgdoor::disk::Best,std::uintptr_t{0});
        if(n==5)bad.Put(door+cfgdoor::disk::Open,std::uint8_t{1});
        if(n==6)bad.Put(door+cfgdoor::disk::Open,std::uint8_t{2});
        CHECK(!observe(bad));
    }
    auto changed=f;changed.trigger=manager+cfgdoor::disk::Best;changed.mutation=changed.trigger;
    changed.newPointer=f.bot;changed.triggerAt=2;CHECK(!observe(changed));
    auto fallback=f;fallback.Put(manager+cfgdoor::disk::Best,std::uintptr_t{0});fallback.Put(manager+cfgdoor::disk::Fallback,component);CHECK(observe(fallback));
    auto competing=f;const auto nonDoor=competing.Object(),other=competing.Object(componentClass);
    competing.Put(other+cfgdoor::disk::ComponentOwner,nonDoor);competing.Put(other+0x20,nonDoor);
    competing.Put(manager+cfgdoor::disk::Best,other);competing.Put(manager+cfgdoor::disk::Fallback,component);CHECK(observe(competing));
    {cfgdoor::Selected selected;CHECK(cfgdoor::ObserveSelected(competing,competing.objects,competing.pawn,doorClass,componentClass,selected));
        CHECK(!cfgdoor::NativeSelectionMatches(competing,selected));}
    auto bounded=f;bounded.cap=5;CHECK(!observe(bounded));
    std::printf("CFG selected-door ownership/obstruction pacing tests: %s (%d failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
