#define CFG_STARTING_FIXTURE_ONLY
#include "StartingPlayerObservationTests.cpp"
#include "LoadedCounselorResolver.hpp"
namespace lc=cfgmenu::loadedcounselor;
struct LoadedFixture:StartingFixture {
    static constexpr std::uintptr_t ClassMeta=0x800000,BlueprintMeta=0x810000,PackageMeta=0x820000,Package=0x830000,PathHolder=0x840000;
    LoadedFixture(){
        reader.Set(Registry+4*24,Selected);reader.Set(Registry+4*24+16,int{104});reader.Set(Registry+4*24+8,std::uint32_t{0});
        reader.Set(Selected+cfgprobe::lifetime::ObjectIndexOffset,int{4});reader.Set(Module+cfgprobe::lifetime::ObjectArrayRva+28,int{5});
        reader.Set(Module+cfgmenu::loadeddisk::NativeClassCache,ClassMeta);reader.Set(Module+cfgmenu::loadeddisk::NativePackageCache,PackageMeta);
        reader.Set(Selected+cfgprobe::ObjectClass,BlueprintMeta);reader.Set(BlueprintMeta+cfgprobe::ClassSuper,ClassMeta);reader.Set(ClassMeta+cfgprobe::ClassSuper,std::uintptr_t{0});
        reader.Set(Package+cfgprobe::ObjectClass,PackageMeta);reader.Set(Package+0x20,std::uintptr_t{0});reader.Set(Selected+0x20,Package);
        Name(Selected,"Athlete_C");Name(Package,"/Game/Characters/Counselors/Athlete");Name(PathHolder,"/Game/Characters/Counselors/Athlete.Athlete_C");
        cfgnames::Words path{};reader.Read(PathHolder+cfgnames::disk::ObjectName,path);
        const int index=4,serial=104;std::memcpy(pick.bytes.data(),&index,4);std::memcpy(pick.bytes.data()+4,&serial,4);std::memcpy(pick.bytes.data()+16,&path,8);
        reader.Set(State+cfgmenu::birthdisk::PickedCounselorClass,pick);
    }
};
struct Environment {
    DWORD thread=42;bool ready=true;unsigned calls=0;std::function<void()> during;
    DWORD Thread()const{return thread;}
    bool Ready(const cfgmenu::promotion::Context&){++calls;if(during)during();return ready;}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* what){++checks;if(!ok){++failures;std::printf("FAIL %s\n",what);}};
    for(unsigned fault=0;fault<26;++fault){LoadedFixture f;sp::Observation observed;check(sp::Observe(f.reader,f.event,observed),"fresh owner before resolver");
        Environment env;using Typed=lc::Resolver<OwnedByteReader,Environment>;Typed resolver(f.reader,env,observed.owner);
        if(fault==1)env.thread=43;
        if(fault==2)env.ready=false;
        if(fault==3){std::memset(f.pick.bytes.data()+16,0,8);f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick);}
        if(fault==4){const int v=-1;std::memcpy(f.pick.bytes.data(),&v,4);f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick);}
        if(fault==5){const int v=0;std::memcpy(f.pick.bytes.data()+4,&v,4);f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick);}
        if(fault==6)f.reader.Set(f.Registry+4*24+16,int{105});
        if(fault==7)f.reader.Set(f.Registry+4*24+8,std::uint32_t{0x10000000});
        if(fault==8)f.reader.Set(f.Registry+4*24+8,std::uint32_t{0x20000000});
        if(fault==9)f.reader.Set(f.Selected+cfgprobe::lifetime::ObjectIndexOffset,int{3});
        if(fault==10)f.reader.Set(f.Selected+cfgprobe::ObjectClass,f.PCClass);
        if(fault==11)f.reader.Set(f.Selected+cfgprobe::ClassSuper,f.Killer);
        if(fault==12)f.Name(f.Selected,"Different_C");
        if(fault==13)f.Name(f.Package,"/Game/Other/Athlete");
        if(fault==14)f.reader.Set(f.Package+cfgprobe::ObjectClass,f.BlueprintMeta);
        if(fault==15)f.reader.Set(f.Package+0x20,f.ClassMeta);
        if(fault==16){const std::uintptr_t v=0x999000;std::memcpy(f.pick.bytes.data()+24,&v,8);f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick);}
        if(fault==17)f.reader.Set(f.Module+cfgmenu::loadeddisk::NativeClassCache,std::uintptr_t{0});
        if(fault==18)f.reader.Set(f.Module+cfgmenu::loadeddisk::NativePackageCache,std::uintptr_t{0});
        if(fault==19)f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,cfgmenu::promotion::Packet{});
        if(fault==20)f.reader.Set(f.Registry+24+16,int{999});
        if(fault==21)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==22){const int v=5;std::memcpy(f.pick.bytes.data(),&v,4);f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick);}
        if(fault==23)f.reader.Set(f.BlueprintMeta+cfgprobe::ClassSuper,f.BlueprintMeta);
        if(fault==24)f.Name(f.Package,"/Script/SomePackage");
        std::uintptr_t selected=999;const auto source=f.State+cfgmenu::birthdisk::PickedCounselorClass+(fault==25?8:0);
        const bool okay=resolver.ResolveLoaded(source,f.pick,selected);
        check(okay==(fault==0),"strict loaded cache/type/path/owner result");check(fault?!selected:selected==f.Selected,"no class substitution or partial output");
        check(resolver.Reads()<=lc::MaximumReads,"bounded known weak slot; no global census");
        if(fault==0){check(resolver.LastReason()==lc::Reason::Ready,"ready reason is explicit");
            // Fails closed until PickedCounselorClass runtime flags are proved for 3.5.
            cfgmenu::promotion::Context prepared;const bool produced=sp::ResolvePicked(f.reader,observed.owner,42,resolver,prepared);
            check(cfgmenu::startingdisk::PickedFlagsVerified?produced&&prepared.selectedClass==f.Selected:!produced&&!prepared.selectedClass,
                "real producer uses compatible loaded-only resolver only after picked runtime flags are proved");std::printf("Loaded-only counselor reads=%u\n",resolver.Reads());}
        if(fault==3)check(resolver.LastReason()==lc::Reason::EmptyPick,"null pick is not resolved");
        if(fault==4||fault==5)check(resolver.LastReason()==lc::Reason::Uncached,"cold weak cache explicit refusal");
    }
    for(unsigned mutation=0;mutation<5;++mutation){LoadedFixture f;sp::Observation o;check(sp::Observe(f.reader,f.event,o),"mutation fixture owns context");Environment env;
        lc::Resolver<OwnedByteReader,Environment> r(f.reader,env,o.owner);
        if(mutation==0){f.reader.changeAt=f.Selected+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Registry+4*24+16;f.reader.newPointer=999;}
        if(mutation==1){f.reader.changeAt=f.Package+cfgnames::disk::ObjectName;f.reader.changeSlot=f.State+cfgmenu::birthdisk::PickedCounselorClass;f.reader.newPointer=0;}
        if(mutation==2){f.reader.changeAt=f.Package+cfgnames::disk::ObjectName;f.reader.changeSlot=f.World+cfgprobe::WorldGameMode;f.reader.newPointer=f.State;}
        if(mutation==3){f.reader.changeAt=f.Package+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Registry+4*24+8;f.reader.newPointer=0x10000000;}
        if(mutation==4){f.reader.changeAt=f.Package+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Module+cfgmenu::loadeddisk::NativeClassCache;f.reader.newPointer=f.BlueprintMeta;}
        std::uintptr_t selected=999;check(!r.ResolveLoaded(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick,selected)&&!selected,"in-call identity/path/owner changes refuse");
    }
    {LoadedFixture f;sp::Observation o;check(sp::Observe(f.reader,f.event,o),"reentry fixture owns context");Environment env;lc::Resolver<OwnedByteReader,Environment> r(f.reader,env,o.owner);
        env.during=[&](){std::uintptr_t nested=999;check(!r.ResolveLoaded(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick,nested)&&!nested,"recursive resolver refuses without lookup");};
        std::uintptr_t selected=0;check(r.ResolveLoaded(f.State+cfgmenu::birthdisk::PickedCounselorClass,f.pick,selected)&&selected==f.Selected,"read-only outer resolution remains exact");}
    {cfgmenu::promotion::Context c{};lc::WindowsLoadedCounselorResolver concrete(c);cfgmenu::promotion::Packet packet;std::uintptr_t out=999;
        check(!concrete.ResolveLoaded(0,packet,out)&&!out,"concrete Windows adapter rejects unbound non-CFG context without calls");}
    std::printf("CFG loaded counselor resolver: %s (%u checks, %u failures). Owned read-only fixtures; no game/native calls or loads.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
