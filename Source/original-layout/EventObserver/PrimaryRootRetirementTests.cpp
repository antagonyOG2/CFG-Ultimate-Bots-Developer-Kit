#define main OriginalPoolTests
#include "PermanentRootPoolTests.cpp"
#undef main
struct RemovalRegistry:Registry {
    unsigned removes=0;bool failRemove=false,keepRegistered=false,swap=false,corruptOther=false;
    bool Unregister(std::uintptr_t owner)noexcept{
        ++removes;if(failRemove)return false;if(keepRegistered)return true;
        for(int i=0;i<list.count;++i)if(entries[i]==owner){
            --list.count;
            if(swap)entries[i]=entries[list.count];else for(int j=i;j<list.count;++j)entries[j]=entries[j+1];
            entries[list.count]=0;if(corruptOther)entries[0]=0xBAD00;return true;
        }return false;
    }
};
#ifndef CFG_PRIMARY_RETIREMENT_MAIN
#define CFG_PRIMARY_RETIREMENT_MAIN main
#endif
int CFG_PRIMARY_RETIREMENT_MAIN(){OriginalPoolTests();
    for(bool swap:{false,true}){
        cfgretain::PermanentRootPool<2> pool;RemovalRegistry registry;registry.swap=swap;cfggcprobe::Report report;
        for(unsigned cycle=0;cycle<4;++cycle){
            CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,2>{0x40000+cycle*0x10000,0x50000+cycle*0x10000},report));
            CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x90000+cycle*0x10000},report));
            CHECK(pool.ReleaseAllLocked(registry,report)&&report.removed==2&&report.calls==2);
            CHECK(!pool.Count()&&!pool.Poisoned()&&registry.list.count==1&&registry.entries[0]==0x20000);
        }
    }
    for(unsigned fault=0;fault<6;++fault){
        cfgretain::PermanentRootPool<2> pool;RemovalRegistry registry;cfggcprobe::Report report;
        CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x40000},report));
        if(fault==0)registry.failRemove=true;if(fault==1)registry.keepRegistered=true;
        if(fault==2)registry.failSnapshot=registry.snapshots+2;if(fault==3)registry.corruptOther=true;
        if(fault==4)reinterpret_cast<cfggcretain::Owner*>(pool.Address(0))->slots[0]=0;
        if(fault==5)reinterpret_cast<cfggcretain::Owner*>(pool.Address(0))->slots[0]=0x60000;
        const auto address=pool.Address(0);CHECK(!pool.ReleaseAllLocked(registry,report));
        CHECK(pool.Count()==1&&pool.Poisoned());const auto calls=registry.removes;
        CHECK(!pool.ReleaseAllLocked(registry,report)&&registry.removes==calls);
        CHECK(!pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x80000},report)&&pool.Address(0)==address);
    }
    for(bool all:{false,true}){
        cfgretain::PermanentRootPool<2> pool;RemovalRegistry registry;cfggcprobe::Report report;
        CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,3>{0x40000,0x50000,0x60000},report));
        auto& owner=*reinterpret_cast<cfggcretain::Owner*>(pool.Address(0));
        owner.CollectWith([&](std::uintptr_t* slot)noexcept{if(all||*slot!=0x40000)*slot=0;});
        CHECK(owner.clearedSlots.load()==(all?3u:2u));
        CHECK(pool.ReleaseAllLocked(registry,report)&&report.removed==1&&!pool.Poisoned()&&!pool.Count());
        CHECK(pool.AddLocked(registry,std::array<std::uintptr_t,1>{0x70000},report));
    }
    std::printf("Completed-match root retirement: %u failures; verified remove/reuse and native-removal ambiguity refuses without replay.\n",failures);
    return failures?1:0;
}
