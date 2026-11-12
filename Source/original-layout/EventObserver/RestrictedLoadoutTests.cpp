#include "NativeRestrictedLoadout.hpp"
#include <map>
#include <cstdio>
using namespace cfgloadout::restricted;
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;std::uintptr_t missing=0;
    template<class T> void Set(std::uintptr_t at,const T& value){const auto* p=reinterpret_cast<const unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    template<class T> bool Read(std::uintptr_t at,T& value){++reads;auto* p=reinterpret_cast<unsigned char*>(&value);for(unsigned i=0;i<sizeof(T);++i){auto f=bytes.find(at+i);if(f==bytes.end()){missing=at+i;return false;}p[i]=f->second;}return true;}
};
struct Host {
    bool current=true,returns=true;unsigned calls=0,resolutions=0;std::uintptr_t resolved=0;
    bool Current(const cfgofflinesettings::Request&){return current;}
    bool InvokeGrant(std::uintptr_t,std::uintptr_t,std::uintptr_t){++calls;return returns;}
    bool ResolveLoadedClass(const cfgofflinesettings::Request&,std::uintptr_t,const cfgprimary::SoftClassBytes&,std::uintptr_t& out){++resolutions;out=resolved;return resolved!=0;}
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,World=0x200000,Pawn=0x210000,Mode=0x220000,
        ModeClass=0x240000,Sandbox=0x250000,Counselor=0x260000,Item=0x270000,Table=0x280000,
        Names=0x290000,Chunk=0x2A0000,Entries=0x2B0000,Loaded=0x300000,Defaults=0x310000,
        Small=0x320000,Special=0x330000,Inventory=0x340000,Package=0x350000,Vtable=0x360000,
        Created=0x370000,ListData=0x380000;
    Memory memory;Host host;Request request{{1,World,Pawn,100,101,0},Choice::PocketKnife};Context context{Module,Mode};
    NativeAccess<Memory,Host> access{memory,host,request,context};
    void Name(unsigned i,const char* value){std::array<unsigned char,128> data{};for(unsigned at=0;value[at];++at)data[at]=static_cast<unsigned char>(value[at]);
        memory.Set(Chunk+8*i,Entries+0x100*i);memory.Set(Entries+0x100*i,std::uint32_t{});memory.Set(Entries+0x100*i+cfgnames::disk::EntryText,data);}
    void Slot(unsigned i,std::uintptr_t object){memory.Set(object+0xC,static_cast<int>(i));memory.Set(Table+24*i,object);memory.Set(Table+24*i+16,static_cast<int>(100+i));}
    Fixture(){using namespace cfgprobe;
        const auto pair=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t type){memory.Set(Module+a,type);memory.Set(Module+b,type);};
        pair(itemdisk::SCGameModeClass,itemdisk::SCGameModeConstructor,ModeClass);
        pair(cfgofflinesettings::disk::SCGameMode_SandboxClass,cfgofflinesettings::disk::SCGameMode_SandboxConstructor,Sandbox);
        pair(itemdisk::SCCounselorCharacterClass,itemdisk::SCCounselorCharacterConstructor,Counselor);
        pair(itemdisk::SCItemClass,itemdisk::SCItemConstructor,Item);
        const auto& inv=cfgfeature::disk::Classes[cfgfeature::disk::Inventory];pair(inv.reflection,inv.constructor,Inventory);
        memory.Set(Mode+ObjectClass,Sandbox);memory.Set(Sandbox+ClassSuper,ModeClass);memory.Set(ModeClass+ClassSuper,std::uintptr_t{});
        memory.Set(Pawn+ObjectClass,Counselor);memory.Set(Counselor+ClassSuper,std::uintptr_t{});
        memory.Set(Loaded+ClassSuper,Item);memory.Set(Item+ClassSuper,std::uintptr_t{});
        memory.Set(Small+ObjectClass,Inventory);memory.Set(Special+ObjectClass,Inventory);memory.Set(Inventory+ClassSuper,std::uintptr_t{});
        memory.Set(World+WorldNetDriver,std::uintptr_t{});memory.Set(World+WorldGameMode,Mode);
        memory.Set(Module+lifetime::ObjectArrayRva+0x10,Table);memory.Set(Module+lifetime::ObjectArrayRva+0x1C,int{7});
        const std::array<std::uintptr_t,7> objects{{World,Pawn,Mode,Loaded,Defaults,Special,Small}};for(unsigned i=0;i<objects.size();++i)Slot(i,objects[i]);
        cfgprimary::SoftClassBytes resource{};const int index=3,serial=103;const cfgnames::Words path{2,0};
        std::memcpy(resource.data(),&index,4);std::memcpy(resource.data()+4,&serial,4);std::memcpy(resource.data()+16,&path,8);
        memory.Set(Mode+itemdisk::PocketKnifeClass,resource);memory.Set(Mode+itemdisk::MedSprayClass,resource);memory.Set(Mode+itemdisk::MapClass,resource);
        memory.Set(Module+cfgnames::disk::Global,Names);struct Counts {int entries,chunks;};memory.Set(Names+cfgnames::disk::ElementCount,Counts{3,1});memory.Set(Names,Chunk);
        Name(0,"OwnedItem_C");Name(1,"/Game/Owned/Item");Name(2,"/Game/Owned/Item.OwnedItem_C");
        memory.Set(Loaded+cfgnames::disk::ObjectName,cfgnames::Words{0,0});memory.Set(Loaded+0x20,Package);
        memory.Set(Package+cfgnames::disk::ObjectName,cfgnames::Words{1,0});memory.Set(Package+0x20,std::uintptr_t{});
        memory.Set(Loaded+itemdisk::ClassDefaultObject,Defaults);memory.Set(Defaults+ObjectClass,Loaded);
        memory.Set(Defaults+cfgloadout::restricteddisk::IsLarge,std::uint8_t{});memory.Set(Defaults+cfgloadout::restricteddisk::IsSpecial,std::uint8_t{});
        memory.Set(Pawn+cfgfeature::disk::SmallInventory,Small);memory.Set(Pawn+cfgfeature::disk::SpecialInventory,Special);
        memory.Set(Small+0x20,Pawn);memory.Set(Special+0x20,Pawn);
        memory.Set(Pawn+itemdisk::PickingItem,std::uintptr_t{});memory.Set(Pawn+cfgloadout::restricteddisk::TempWeapon,std::uintptr_t{});
        memory.Set(Pawn+cfgloadout::restricteddisk::MaxSmallItems,int{3});
        memory.Set(Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{});memory.Set(Special+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{});
        memory.Set(Pawn,Vtable);memory.Set(Vtable+cfgloadout::restricteddisk::AddOrSwapSlot,Module+cfgloadout::restricteddisk::CounselorAddOrSwap);
        memory.Set(Module+cfgloadout::restricteddisk::CounselorAddOrSwap,cfgloadout::restricteddisk::CounselorAddOrSwapCode);
        memory.Set(Module+itemdisk::GiveStartingItem,itemdisk::GiveStartingItemCode);
    }
    void Added(bool special=false){const auto inv=special?Special:Small;Slot(7,Created);memory.Set(Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{8});
        memory.Set(inv+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{ListData,1,1});memory.Set(ListData,Created);
        memory.Set(Created+cfgprobe::ObjectClass,Loaded);memory.Set(Created+cfgloadout::restricteddisk::OwningCharacter,Pawn);
        memory.Set(Created+cfgfeature::disk::OuterInventory,inv);memory.Set(Created+cfgloadout::restricteddisk::IsLarge,std::uint8_t{});
        memory.Set(Created+cfgloadout::restricteddisk::IsSpecial,static_cast<std::uint8_t>(special));}
};
int main(){unsigned tests=0,failures=0;auto check=[&](bool x,const char* name){++tests;if(!x){++failures;std::printf("FAIL %s\n",name);}};
    {Fixture f;Runtime runtime;check(runtime.Begin(f.request,0),"resource wait begins");
        for(unsigned t=3000;t<=30000;t+=250)check(runtime.AwaitResource(t),"pending resource does not consume attempts");
        check(runtime.Service(f.access,30250)&&f.host.calls==0,"loaded resource prepares after original18s deadline");
        check(runtime.Service(f.access,31250)&&f.host.calls==1,"loaded resource grants in fresh phase");
        check(!runtime.AwaitResource(31500)&&!runtime.FailResource(),"dispatched grant cannot be reopened by loader");
        f.Added();check(runtime.Service(f.access,32250)&&runtime.CurrentState()==State::Verified,"delayed grant verified");}
    {Fixture f;Runtime runtime;check(runtime.Begin(f.request,0)&&runtime.FailResource(),"load timeout terminal");
        check(!runtime.AwaitResource(30000)&&!runtime.Service(f.access,30000)&&!f.host.calls,"no grant after load timeout");}
    for(bool manualMode:{false,true})for(unsigned fault=0;fault<5;++fault){Fixture f;
        cfgprimary::SoftClassBytes cold{};f.memory.Read(f.Mode+itemdisk::PocketKnifeClass,cold);
        const int empty=-1;std::memcpy(cold.data(),&empty,4);std::memset(cold.data()+4,0,4);
        f.memory.Set(f.Mode+itemdisk::PocketKnifeClass,cold);f.host.resolved=f.Loaded;
        // The manual offline branch accepts the same native Sandbox fixture.
        NativeAccess<Memory,Host> manual(f.memory,f.host,f.request,{f.Module,f.Mode,manualMode});
        if(fault==1)f.host.resolved=0;
        if(fault==2)f.Name(2,"/Game/Other/Item.OwnedItem_C");
        Evidence before;
        if(fault==1||fault==2){check(!manual.Observe(before,true)&&!f.host.calls,"unresolved or wrong full path refused");continue;}
        check(manual.Observe(before,true)&&before.resolved&&f.host.resolutions==1,"cold soft class resolved once");
        if(fault==3)f.memory.Set(f.Table+3*24+16,int{999});
        if(fault==4)f.memory.Set(f.Table+4*24+16,int{999});
        if(fault>=3){check(!manual.Grant(before)&&!f.host.calls&&f.host.resolutions==1,"resolved class/CDO evidence revalidated before grant");continue;}
        check(manual.Grant(before)&&f.host.calls==1&&f.host.resolutions==1,"grant uses supplied resolved evidence");
        f.Added();std::uintptr_t created=0;check(manual.Verify(before,&created)&&created&&f.host.resolutions==1,"return verified and new item captured without warmed mode cache");
        cfgprimary::SoftClassBytes unchanged{};f.memory.Read(f.Mode+itemdisk::PocketKnifeClass,unchanged);
        check(unchanged==cold,"original forty byte soft entry preserved");
        std::printf("Resolved inventory capture reads=%u\n",manual.lastReads);
    }
    for(unsigned stale=0;stale<3;++stale){Fixture f;cfgprimary::SoftClassBytes packet{};
        f.memory.Read(f.Mode+itemdisk::PocketKnifeClass,packet);
        const int index=stale==0?999:3,serial=stale==1?999:103;
        std::memcpy(packet.data(),&index,4);std::memcpy(packet.data()+4,&serial,4);
        f.memory.Set(f.Mode+itemdisk::PocketKnifeClass,packet);f.host.resolved=f.Loaded;
        if(stale==2)f.memory.Set(f.Table+3*24,std::uintptr_t{});
        Evidence e;
        if(stale==2)check(!f.access.Observe(e,true)&&!f.host.calls,"resolver cannot rescue missing class slot");
        else check(f.access.Observe(e,true)&&e.resolved&&f.host.resolutions==1,"stale weak cache uses exact native path resolver");
    }
    for(auto mode:{cfgprobe::OfflineBotsMode,cfgprobe::ChallengesMode}){
        Fixture f;constexpr std::uintptr_t other=0x390000;
        const auto& binding=cfgprobe::NativeClasses[mode];
        f.memory.Set(f.Module+binding.reflectionCacheRva,other);f.memory.Set(f.Module+binding.constructorCacheRva,other);
        f.memory.Set(other+cfgprobe::ClassSuper,f.ModeClass);f.memory.Set(f.Mode+cfgprobe::ObjectClass,other);
        const auto& sandbox=cfgprobe::NativeClasses[cfgprobe::SandboxMode];
        f.memory.Set(f.Module+sandbox.reflectionCacheRva,std::uintptr_t{});f.memory.Set(f.Module+sandbox.constructorCacheRva,std::uintptr_t{});
        Evidence e;
        check(!f.access.Observe(e,false),"startup-only adapter does not adopt different offline mode");
        NativeAccess<Memory,Host> manual(f.memory,f.host,f.request,{f.Module,f.Mode,true});
        check(manual.Observe(e,false),"manual tools accept verified offline mode without requiring Sandbox cache");
        f.memory.Set(f.World+cfgprobe::WorldNetDriver,f.Small);
        check(!manual.Observe(e,false)&&!f.host.calls,"manual inventory still refuses network world");
    }
    for(bool special:{false,true}){Fixture f;f.memory.Set(f.Defaults+cfgloadout::restricteddisk::IsSpecial,static_cast<std::uint8_t>(special));Runtime runtime;
        check(runtime.Begin(f.request,0)&&!runtime.Service(f.access,2999),"delay");
        check(runtime.Service(f.access,3000)&&f.host.calls==0&&runtime.CurrentState()==State::Waiting,"prepare without native dispatch");
        check(runtime.Service(f.access,4000)&&f.host.calls==1&&runtime.CurrentState()==State::Dispatched,"one native helper dispatch in fresh phase");
        if(f.host.calls!=1)std::printf("Missing=%llX reads=%u\n",static_cast<unsigned long long>(f.memory.missing),f.access.lastReads);
        f.Added(special);check(runtime.Service(f.access,5000)&&runtime.CurrentState()==State::Verified,"new owned item verified");
        check(!runtime.Service(f.access,6000)&&f.host.calls==1,"no second grant");
        std::printf("Restricted capture reads=%u\n",f.access.lastReads);}
    for(unsigned fault=0;fault<12;++fault){Fixture f;Evidence e;
        if(fault==0)f.host.current=false;
        if(fault==1)f.memory.Set(f.World+cfgprobe::WorldNetDriver,f.Small);
        if(fault==2)f.memory.Set(f.Pawn+itemdisk::PickingItem,f.Created);
        if(fault==3)f.memory.Set(f.Pawn+cfgloadout::restricteddisk::TempWeapon,f.Created);
        if(fault==4)f.memory.Set(f.Defaults+cfgloadout::restricteddisk::IsLarge,std::uint8_t{1});
        if(fault==5)f.memory.Set(f.Pawn+cfgloadout::restricteddisk::MaxSmallItems,int{0});
        if(fault==6)f.memory.Set(f.Small+0x20,f.Mode);
        if(fault==7)f.memory.Set(f.Table+3*24+16,int{999});
        if(fault==8)f.Name(2,"/Game/Other/Item.OwnedItem_C");
        if(fault==9)f.memory.Set(f.Vtable+cfgloadout::restricteddisk::AddOrSwapSlot,f.Module+0x1234);
        if(fault==10)f.memory.Set(f.Module+itemdisk::GiveStartingItem,std::uint8_t{});
        if(fault==11)f.memory.Set(f.Table+16,int{999});
        check(!f.access.Observe(e,true)&&!f.host.calls,"unsafe readiness refused");
        if(fault==2)check(f.access.lastReason==RefusalReason::PickupBusy,"pickup busy has a distinct stage-5 reason");
        if(fault==3)check(f.access.lastReason==RefusalReason::TemporaryWeaponBusy,"temporary weapon busy has a distinct stage-5 reason");
        if(fault==4)check(f.access.lastReason==RefusalReason::LargeItem,"invalid large-item default has a distinct reason");
        if(fault==5)check(f.access.lastReason==RefusalReason::InvalidCapacity,"invalid small-item capacity has a distinct reason");
        if(fault==6)check(f.access.lastReason==RefusalReason::SmallInventoryOwner,"wrong small-inventory owner has a distinct reason");}
    for(unsigned fault=0;fault<7;++fault){Fixture f;Evidence e;
        cfgspawn::PlayerArray list{f.ListData,1,1};RefusalReason expected=RefusalReason::None;
        switch(fault){
        case 0:list.count=9;list.capacity=9;expected=RefusalReason::ListCount;break;
        case 1:list.capacity=0;expected=RefusalReason::ListCapacity;break;
        case 2:list.data=0;expected=RefusalReason::ListData;break;
        case 3:expected=RefusalReason::ListEntryRead;break;
        case 4:f.memory.Set(f.ListData,std::uintptr_t{});expected=RefusalReason::ListNullEntry;break;
        case 5:f.memory.Set(f.ListData,std::uintptr_t{1});expected=RefusalReason::ListInvalidEntry;break;
        default:list.count=list.capacity=2;f.memory.Set(f.ListData,f.Created);f.memory.Set(f.ListData+8,f.Created);expected=RefusalReason::ListDuplicateEntry;break;
        }
        f.memory.Set(f.Special+cfgfeature::disk::InventoryItems,list);
        if(fault==4){check(f.access.Observe(e,true)&&!f.host.calls,
            "native null special slot is stable absence; no dereference, write or capacity bypass");continue;}
        check(!f.access.Observe(e,true)&&f.access.lastReason==expected&&!f.host.calls,
            "special inventory refusal leaf is diagnostic only; no grant or weakened guard");
    }
    for(unsigned fault=0;fault<5;++fault){Fixture f;Evidence before;check(f.access.Observe(before,true),"verification baseline");f.Added();
        if(fault==0)f.memory.Set(f.Created+cfgloadout::restricteddisk::OwningCharacter,f.Mode);
        if(fault==1)f.memory.Set(f.Created+cfgfeature::disk::OuterInventory,f.Special);
        if(fault==2)f.memory.Set(f.Created+cfgprobe::ObjectClass,f.Item);
        if(fault==3)f.memory.Set(f.Created+cfgloadout::restricteddisk::IsLarge,std::uint8_t{1});
        if(fault==4)f.memory.Set(f.Table+7*24,f.Pawn);
        std::uintptr_t created=1;check(!f.access.Verify(before,&created)&&!created,"unowned/mismatched result refused and no output exposed");}
    {Fixture f;Runtime runtime;runtime.Begin(f.request,0);f.host.returns=false;runtime.Service(f.access,3000);runtime.Service(f.access,4000);
        check(runtime.CurrentState()==State::Indeterminate&&!runtime.Service(f.access,5000)&&f.host.calls==1,"ambiguous native return never retried");}
    {Fixture f;f.request.choice=Choice::None;Runtime runtime;check(runtime.Begin(f.request,0)&&runtime.CurrentState()==State::Preserved&&!runtime.Service(f.access,4000)&&!f.memory.reads,"None zero work");}
    {Fixture f;f.Added();f.memory.Set(f.Pawn+cfgloadout::restricteddisk::MaxSmallItems,int{1});Evidence e;
        check(!f.access.Observe(e,true)&&!f.host.calls,"full small inventory cannot enter swap branch");}
    {Fixture f;f.memory.Set(f.Defaults+cfgloadout::restricteddisk::IsSpecial,std::uint8_t{1});f.Added(true);Evidence e;
        check(!f.access.Observe(e,true)&&!f.host.calls,"occupied special inventory preserved");}
    {Fixture f;f.Added();Evidence e;check(f.access.Observe(e,true)&&e.lists[1].header.count==1,"spare small slot with owned old item accepted");
        f.memory.Set(f.Created+cfgloadout::restricteddisk::OwningCharacter,f.Mode);check(!f.access.Observe(e,true),"foreign old inventory item refused");}
    {Fixture f;Evidence e;check(f.access.Observe(e,true),"stale grant snapshot captured");f.memory.Set(f.Pawn+itemdisk::PickingItem,f.Created);
        check(!f.access.Grant(e)&&!f.host.calls,"pickup became busy before grant");}
    {Fixture f;Runtime runtime;runtime.Begin(f.request,0);check(!runtime.Service(f.access,18001)&&runtime.CurrentState()==State::Unavailable&&!f.host.calls,"absolute startup deadline");}
    for(unsigned fault=0;fault<5;++fault){Fixture f;constexpr std::uintptr_t nativeMode=0x400000;
        const auto family=cfgmenu::offlinelifecycledisk::OfflineBotsClass;
        f.memory.Set(f.Module+family,nativeMode);f.memory.Set(f.Module+family+8,nativeMode);
        f.memory.Set(nativeMode+cfgprobe::ClassSuper,f.ModeClass);f.memory.Set(f.Mode+cfgprobe::ObjectClass,nativeMode);
        f.memory.Set(f.Mode,f.Module+cfgmenu::offlinelifecycledisk::OfflineBotsVtable);
        f.context.nativeOffline=fault!=1;
        if(fault==2)f.memory.Set(f.Mode+cfgprobe::ObjectClass,f.Sandbox);
        if(fault==3)f.memory.Set(f.Mode,f.Vtable);
        NativeAccess<Memory,Host> native(f.memory,f.host,f.request,f.context);Evidence evidence;
        check(native.Observe(evidence,true)==(fault==0),"restricted loadout explicit exact native family");
        if(!fault){check(native.Grant(evidence)&&f.host.calls==1,"native family grant dispatched once");f.Added();check(native.Verify(evidence),"native family created inventory ownership verified");}
        else check(!f.host.calls,"wrong family never grants loadout");
    }
    std::printf("Restricted loadout owned fixtures: %u checks, %u failures. No native/game access.\n",tests,failures);return failures?1:0;
}
