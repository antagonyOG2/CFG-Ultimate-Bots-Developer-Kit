#define main ExistingActiveOwnerTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "../RuntimeProbe/NativeProfileSelectionObservation.hpp"
struct ProfileFixture : Fixture {
    static constexpr std::uintptr_t PlayerType=0x600000,SaveType=0x610000,OtherType=0x620000,
        Save=0x630000,OtherSave=0x640000,Cache=0x650000,Counselor=0x660000,Jason=0x670000,SecondSave=0x680000,
        Defaults=0x690000,NativeCounselor=0x6A0000,NativeJason=0x6B0000;
    ProfileFixture(){using namespace cfgmenu::selectiondisk;using namespace cfgprobe;
        reader.Set(Module+ILLLocalPlayerClass,PlayerType);reader.Set(Module+ILLLocalPlayerConstructor,PlayerType);
        reader.Set(Module+SCCharacterSelectionsSaveGameClass,SaveType);reader.Set(Module+SCCharacterSelectionsSaveGameConstructor,SaveType);
        reader.Set(Player+ObjectClass,PlayerType);reader.Set(PlayerType+ClassSuper,std::uintptr_t{0});
        reader.Set(Controller+0x3D0,Player);
        reader.Set(Player+SettingsSaveGameCache,cfgmenu::maps::Array{Cache,2,4});reader.Set(Cache,OtherSave);reader.Set(Cache+8,Save);
        reader.Set(OtherSave+ObjectClass,OtherType);reader.Set(OtherType+ClassSuper,std::uintptr_t{0});
        reader.Set(Save+ObjectClass,SaveType);reader.Set(SaveType+ClassSuper,std::uintptr_t{0});
        reader.Set(SaveType+cfgspawn::ClassDefaultObject,Defaults);
        reader.Set(Save+CounselorPick,Counselor);reader.Set(Save+KillerPick,Jason);
        reader.Set(Counselor+ClassSuper,NativeCounselor);reader.Set(NativeCounselor+ClassSuper,std::uintptr_t{0});
        reader.Set(Jason+ClassSuper,NativeJason);reader.Set(NativeJason+ClassSuper,std::uintptr_t{0});
        for(const auto index:{cfgprobe::Counselor,cfgprobe::Killer}){const auto value=index==cfgprobe::Counselor?NativeCounselor:NativeJason;
            reader.Set(Module+NativeClasses[index].reflectionCacheRva,value);reader.Set(Module+NativeClasses[index].constructorCacheRva,value);}
        reader.Set(Module+lifetime::ObjectArrayRva+28,5);reader.Set(Save+lifetime::ObjectIndexOffset,4);
        reader.Set(Registry+24*4,Save);reader.Set(Registry+24*4+16,104);
    }
};
int main(){using namespace cfgmenu;unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    struct OwnerThread {unsigned thread=42;unsigned Thread()const{return thread;}};
    for(unsigned fault=0;fault<9;++fault){ProfileFixture f;active::Context current;OwnerThread engine;
        check(active::Observe(f.reader,Fixture::Module,current),"event-local fixture owner");using F=ProfileFixture;
        f.reader.Set(F::Registry+24*4+16,0); // Actual CFG save has an unassigned serial.
        if(fault==1)engine.thread=41;
        if(fault==2)f.reader.Set(F::SaveType+cfgspawn::ClassDefaultObject,F::Save);
        if(fault==3)f.reader.Set(F::Counselor+cfgprobe::ClassSuper,F::NativeJason);
        if(fault==4)f.reader.Set(F::Module+cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,F::NativeCounselor);
        if(fault==5){f.reader.changeAt=F::Save+selectiondisk::KillerPick;f.reader.changeSlot=F::Cache+8;f.reader.newPointer=F::OtherSave;}
        if(fault==6)f.reader.Set(F::Registry+24*4,F::OtherSave);
        if(fault==7)f.reader.Set(F::Registry+24*2+16,0);
        if(fault==8)f.reader.Set(F::Controller+0x3D0,F::OtherSave);
        profiles::Values values;unsigned consumed=0;
        check(profiles::EventLocalValues(f.reader,engine,current,true,values,&consumed)==(fault==0),
            "zero-serial event-local values need typed classes, current owner, stable cache and slot; not a retained save");
        check(consumed<=profiles::ReadLimit,"event-local aggregate hard budget");
        if(!fault){check(values.counselor==F::Counselor&&values.jason==F::Jason,"publish only owned typed class values");
            std::printf("Event-local profile values reads=%u Serial0 remains unassigned\n",consumed);
            profiles::Selection diagnostic;check(profiles::ObserveSnapshot(f.reader,current,42,true,diagnostic)&&!diagnostic.identity.HasAssignedSerial(),
                "read-only snapshot preserves unassigned serial without granting retained identity");
            check(!profiles::Observe(f.reader,current,42,true,diagnostic)&&!diagnostic.save,"retained positive-serial contract not weakened");
        }else check(!values.counselor&&!values.jason,"no defaults or partial values on event-local refusal");
        if(fault==1)check(!consumed,"wrong-thread event does zero reads");
    }
    for(unsigned fault=0;fault<11;++fault){ProfileFixture f;active::Context current;
        check(active::Observe(f.reader,Fixture::Module,current),"supplied current frontend only");using F=ProfileFixture;using namespace selectiondisk;
        if(fault==1)f.reader.Set(F::Controller+0x3D0,F::OtherSave);
        if(fault==2)f.reader.Set(F::Cache+8,F::OtherSave);
        if(fault==3)f.reader.Set(F::Cache,F::Save);
        if(fault==4)f.reader.Set(F::Registry+24*4+16,0);
        if(fault==5)f.reader.Set(F::Save+KillerPick,std::uintptr_t{0});
        if(fault==6)f.reader.Set(F::Player+SettingsSaveGameCache,maps::Array{F::Cache,33,33});
        if(fault==7)f.reader.Set(F::OtherType+cfgprobe::ClassSuper,F::OtherType);
        if(fault==8){f.reader.changeAt=F::Save+KillerPick;f.reader.changeSlot=F::Cache+8;f.reader.newPointer=F::OtherSave;}
        profiles::Selection selected;unsigned consumed=0;
        const auto okay=profiles::Observe(f.reader,current,fault==9?41u:42u,fault!=10,selected,&consumed);
        check(okay==(fault==0),"reverse ownership/type/ambiguity/stale/choice mutation refusal");check(consumed<=profiles::ReadLimit,"finite menu-only cache budget");
        if(okay){check(selected.save==F::Save&&selected.counselor==F::Counselor&&selected.jason==F::Jason,"actual player's save picks; no default fallback");
            std::printf("Owned profile selection reads=%u\n",consumed);}
        else check(!selected.save&&!selected.counselor&&!selected.jason,"no stale/default profile published");
    }
    for(unsigned variant=0;variant<4;++variant){ProfileFixture f;active::Context current;OwnerThread engine;using F=ProfileFixture;
        check(active::Observe(f.reader,Fixture::Module,current),"nullable saved-choice fixture");
        f.reader.Set(F::Registry+24*4+16,0);
        f.reader.Set(F::Save+selectiondisk::CounselorPick,std::uintptr_t{0});
        if(variant==1)f.reader.Set(F::Save+selectiondisk::KillerPick,std::uintptr_t{0});
        if(variant==2)f.reader.Set(F::SaveType+cfgspawn::ClassDefaultObject,std::uintptr_t{0});
        if(variant==3)f.reader.Set(F::Save+selectiondisk::CounselorPick,std::uintptr_t{3});
        profiles::Selection snapshot;profiles::Diagnostic evidence;
        check(profiles::ObserveSnapshot(f.reader,current,42,true,snapshot,nullptr,&evidence)==(variant!=3),
            "constructor-null class choices and lazy CDO observable; malformed nonzero choice rejected");
        if(variant!=3){check(!snapshot.counselor&&snapshot.jason==(variant==1?0:F::Jason)&&evidence.counselorRead&&evidence.jasonRead,
                "both nullable fields sampled independently, no fabricated character fallback");
            profiles::Values values;check(!profiles::EventLocalValues(f.reader,engine,current,true,values)&&!values.counselor&&!values.jason,
                "unset snapshot does not become usable spawn values");}
    }
    std::printf("CFG owned local-player profile selection: %s (%u failures). No game access/census/load.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
