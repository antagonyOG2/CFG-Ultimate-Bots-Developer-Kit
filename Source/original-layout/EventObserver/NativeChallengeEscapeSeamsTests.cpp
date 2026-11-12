#define CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeActorsTests.cpp"
#undef CFG_ESCAPE_ACTORS_FIXTURE_ONLY
#include "NativeChallengeEscapeSeams.hpp"
#include <limits>
namespace seams=cfgchallengeescape::seams;
namespace boundary=cfgchallengeescape::boundary;
struct SeamFixture : ActorFixture {
    ChallengeReader reader;
    scope::Runtime authority;
    scope::Witness witness;
    bool bound=false;
    SeamFixture() {
        static_cast<StageReader&>(reader)=r; reader.cap=524288;
        authority.Arm(Menu(),1,true,0); bound=authority.Observe(reader,42,100,witness);
        act::life::SlotObservation level; act::life::ReadSlot(reader,witness.registry,Level,level);
        reader.Set(witness.registry.data+24ULL*static_cast<unsigned>(level.index)+16,int{});
        reader.count=0;
    }
    seams::Arguments Args(boundary::Seam seam,bool car=false) const {
        seams::Arguments a; a.actor=car ? Car : Npc; a.self=Volume; a.component=Sphere;
        a.otherComponent=car ? CarRoot : Root; a.volume=Volume; a.controller=Ai;
        if(seam==boundary::Seam::CharacterStart) a.self=Npc;
        if(seam==boundary::Seam::Completion) a.self=witness.Mode();
        if(seam==boundary::Seam::VehicleStart) a.self=Car;
        return a;
    }
    bool Validate(boundary::Seam seam,const seams::Arguments& a,boundary::Identity* out=nullptr) {
        const auto current=[&]{return authority.Current(reader,42,witness);};
        return seams::Validate(reader,Module,witness,seam,a,current,out);
    }
};
int main() {
    unsigned checks=0,failures=0,peak=0;
    const auto check=[&](bool okay,const char* label) {++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const std::array<boundary::Seam,5> kinds{{boundary::Seam::Overlap,boundary::Seam::CharacterStart,
        boundary::Seam::Presentation,boundary::Seam::Completion,boundary::Seam::VehicleStart}};
    for(const auto seam:kinds) {
        SeamFixture f; auto a=f.Args(seam); boundary::Identity identity;
        check(f.bound && f.Validate(seam,a,&identity),"five real native seams admit positive sources with serial-zero Level borrow");
        check(identity.Positive() && identity.address==seams::ActorAddress(seam,a),"native actor identity emitted only after repeatable admission");
        check(f.Validate(seam,a),"same native arguments repeat their whole current scope/source proof");
        peak=(std::max)(peak,f.reader.count);
        a.self=f.Root; identity={f.Npc,99,99};
        check(!f.Validate(seam,a,&identity) && !identity.Positive(),"wrong native self refuses and clears output identity");
    }
    for(unsigned fault=0;fault<10;++fault) {
        SeamFixture f; auto a=f.Args(boundary::Seam::Overlap);
        if(fault==0) a.component=0;
        if(fault==1) a.component=f.Root;
        if(fault==2) a.otherComponent=f.Movement;
        if(fault==3) a.otherComponent=f.CarRoot;
        if(fault==4) a.actor=f.Ai;
        if(fault==5) f.reader.Set(f.Sphere+0x20,f.Car);
        if(fault==6) f.reader.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
        if(fault==7) f.reader.Set(f.Npc+0x110,std::uint8_t{1});
        if(fault==8) f.reader.Set(f.ActorArray,f.Car);
        if(fault==9) f.reader.Set(f.Npc+0x20,f.World);
        check(!f.Validate(boundary::Seam::Overlap,a),"wrong overlap component/actor/owner/role/foreignworld refuses");
    }
    {SeamFixture f; auto a=f.Args(boundary::Seam::Overlap);a.otherComponent=0;a.bodyIndex=-123;a.sweep=true;
     check(f.Validate(boundary::Seam::Overlap,a),"optional other component and opaque overlap values do not invent action authority");}
    {SeamFixture f; auto a=f.Args(boundary::Seam::Overlap,true);
     check(f.Validate(boundary::Seam::Overlap,a),"actual car root can be an admitted overlap component");}
    for(const auto seam:kinds) {
        SeamFixture f; auto a=f.Args(seam);
        f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
        check(!f.Validate(seam,a),"all seams require fresh offline scope rather than argument identity alone");
    }
    for(const auto seam:kinds) {
        SeamFixture f; auto a=f.Args(seam);
        f.reader.Set(f.Mode+cfgprobe::ObjectClass,f.ActorType);
        check(!f.Validate(seam,a),"ordinary native game mode never admits any escape seam");
    }
    {SeamFixture f; auto a=f.Args(boundary::Seam::CharacterStart);
     a.exitPath=UINTPTR_MAX;a.delay=std::numeric_limits<float>::quiet_NaN();a.actor=f.Car;
     check(f.Validate(boundary::Seam::CharacterStart,a),"character start uses actual self and keeps spline/delay opaque");
     f.reader.Set(f.Npc+0x3A0,std::uintptr_t{});
     check(f.Validate(boundary::Seam::CharacterStart,a),"source admission does not promote or reject unpossessed AI history");}
    {SeamFixture f; auto a=f.Args(boundary::Seam::Completion);
     f.reader.Set(f.Npc+0x3A0,std::uintptr_t{});a.controller=f.Human;
     check(f.Validate(boundary::Seam::Completion,a),"historical unpossessed counselor completion ignores native RDX current-controller mismatch");
     a.actor=f.Passenger;a.controller=0;
     check(f.Validate(boundary::Seam::Completion,a),"completion admits actual R8 passenger as current-world counselor");
     a.actor=f.Car;check(!f.Validate(boundary::Seam::Completion,a),"completion R8 must be native counselor rather than car");}
    {SeamFixture f;auto a=f.Args(boundary::Seam::Presentation);a.actor=f.Passenger;
     check(f.Validate(boundary::Seam::Presentation,a),"presentation accepts actual current-world passenger counselor");
     a.self=f.Car;check(!f.Validate(boundary::Seam::Presentation,a),"presentation owner must be actual positive escape volume");}
    {SeamFixture f;auto a=f.Args(boundary::Seam::VehicleStart);a.exitPath=UINTPTR_MAX;a.delay=-1000;
     check(f.Validate(boundary::Seam::VehicleStart,a),"vehicle seam source validates without native calls or seat-policy inference");
     a.volume=f.Car;check(!f.Validate(boundary::Seam::VehicleStart,a),"vehicle start requires positive native escape volume");
     a=f.Args(boundary::Seam::VehicleStart);a.self=f.Npc;
     check(!f.Validate(boundary::Seam::VehicleStart,a),"vehicle start self cannot be a native counselor");}
    for(const auto seam:kinds) {
        SeamFixture f;auto a=f.Args(seam);act::life::SlotObservation slot;
        act::life::ReadSlot(f.reader,f.witness.registry,seams::ActorAddress(seam,a),slot);
        f.reader.Set(f.witness.registry.data+24ULL*static_cast<unsigned>(slot.index)+16,int{});
        check(!f.Validate(seam,a),"serial-unassigned actor never inherits Level borrow exemption");
    }
    {SeamFixture f;auto a=f.Args(boundary::Seam::Overlap);bool current=true;
     const auto refuse=[&]{return current;};current=false;
     check(!seams::Validate(f.reader,f.Module,f.witness,boundary::Seam::Overlap,a,refuse),"caller scope refusal fails before source promotion");}
    {SeamFixture f;auto a=f.Args(boundary::Seam::Overlap);
     check(!f.Validate(static_cast<boundary::Seam>(255),a),"unknown seam enumeration never gains admission");}
    {SeamFixture f;auto a=f.Args(boundary::Seam::Overlap);f.reader.cap=1;
     check(!f.Validate(boundary::Seam::Overlap,a),"finite reader exhaustion refuses without native side effects");}
    check(peak<524288,"whole repeated native seam proof remains within dedicated callback read ceiling");
    std::printf("NativeChallengeEscapeSeams: %u checks, %u failures; peak repeated reads=%u/524288. Owned fixtures, no native calls.\n",checks,failures,peak);
    return failures ? 1 : 0;
}
