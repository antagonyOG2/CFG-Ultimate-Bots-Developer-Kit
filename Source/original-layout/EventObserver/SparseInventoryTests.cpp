#define main OriginalRestrictedLoadoutSuiteMain
#include "RestrictedLoadoutTests.cpp"
#undef main
#include <cassert>
int main(){unsigned checks=0;const auto check=[&](bool x){++checks;assert(x);};
    for(bool specialHole:{false,true}){
        Fixture f;Evidence before;const auto data=Fixture::ListData+0x80;
        f.memory.Set((specialHole?Fixture::Special:Fixture::Small)+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{data,1,1});
        f.memory.Set(data,std::uintptr_t{});
        check(f.access.Observe(before,true)&&!f.host.calls);
        check(f.access.Grant(before)&&f.host.calls==1);
        f.Added();
        if(!specialHole){f.memory.Set(Fixture::ListData,std::uintptr_t{});f.memory.Set(Fixture::ListData+8,Fixture::Created);
            f.memory.Set(Fixture::Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{Fixture::ListData,2,2});}
        std::uintptr_t created=0;check(f.access.Verify(before,&created)&&created==Fixture::Created);
    }
    {Fixture f;Evidence e;const auto data=Fixture::ListData;
        f.memory.Set(Fixture::Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{data,3,3});
        for(unsigned i=0;i<3;++i)f.memory.Set(data+8*i,std::uintptr_t{});
        check(!f.access.Observe(e,true)&&!f.host.calls); // holes NEVER bypass native count/capacity or authorize swapping
    }
    std::printf("KN1 actual restricted native access sparse special/small roster, append/ownership verification and full refusal: %u checks PASS.\n",checks);
}
