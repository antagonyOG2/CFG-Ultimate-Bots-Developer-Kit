#define main RestrictedLoadoutRegressionMain
#include "RestrictedLoadoutTests.cpp"
#undef main
#include "HeldWeaponObservation.hpp"
struct ChangingWeaponReader {
    Fixture& f;unsigned fault=0,ownerReads=0,weaponSerialReads=0,pawnSerialReads=0,classReads=0;
    template<class T> bool Read(std::uintptr_t at,T& out){
        if(at==f.Created+cfgloadout::restricteddisk::OwningCharacter&&++ownerReads==2&&fault==7)
            f.memory.Set(at,std::uintptr_t{0x400000});
        if(at==f.Table+24*7+16&&++weaponSerialReads==2&&fault==8)f.memory.Set(at,int{999});
        if(at==f.Table+24+16&&++pawnSerialReads==2&&fault==9)f.memory.Set(at,int{999});
        if(at==f.Created+cfgprobe::ObjectClass&&++classReads==2&&fault==10)f.memory.Set(at,f.Counselor);
        return f.memory.Read(at,out);
    }
};
int main(){unsigned failures=0;
    for(unsigned fault=0;fault<=10;++fault){Fixture f;f.Added();
        f.memory.Set(f.Table+24+8,std::uint32_t{});f.memory.Set(f.Table+24*7+8,std::uint32_t{});
        if(fault==1)f.memory.Set(f.Table+24*7+16,int{});
        if(fault==2)f.memory.Set(f.Created+cfgloadout::restricteddisk::OwningCharacter,std::uintptr_t{0x400000});
        if(fault==3)f.memory.Set(f.Table+24+16,int{});
        if(fault==4)f.memory.Set(f.Loaded+cfgprobe::ClassSuper,std::uintptr_t{});
        if(fault==5)f.memory.Set(f.Table+24*7,std::uintptr_t{0x400000});
        if(fault==6)f.memory.Set(f.Table+24*7+8,std::uint32_t{0x20000000});
        ChangingWeaponReader reader{f,fault};
        const bool result=cfgkill::OwnedHeldWeapon(reader,{f.Table,8},f.Pawn,f.Created,f.Item);
        if(result!=(fault<=1)){++failures;std::printf("FAIL held-weapon fault%u\n",fault);}
    }
    std::printf("HeldWeaponTests: 11 checks, %u failures. No game access.\n",failures);return failures?1:0;
}
