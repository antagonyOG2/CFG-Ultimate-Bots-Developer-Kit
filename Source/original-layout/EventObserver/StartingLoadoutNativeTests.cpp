#define main RestrictedLoadoutRegressionMain
#include "RestrictedLoadoutTests.cpp"
#undef main
#include "NativeStartingLoadout.hpp"
#include "ExpandedLoadoutRuntime.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include "../RuntimeProbe/ObjectPathObservation.hpp"
// Production-shape charging: retained outer Context+LinkedKiller is120 reads
// at state/Jason/controller/path depths5/8/6/3; reserve72 additional reads for
// alive/held/pre-phase guards. Current(owner) costs Context(true)+Counselor
// terminal observation; reserve128 per call (58+61 at depths5/8). Add128 to
// each phase for the deeper native class chains absent from this small fixture,
// plus128 for Runtime::Step's initial Current(owner) before invoking the adapter.
struct ProductionLoadoutReader {
    Memory& memory;unsigned reads=0,peak=0;
    bool Charge(unsigned count){if(count>1536-reads)return false;reads+=count;return true;}
    void Phase(){peak=(std::max)(peak,reads);reads=192+128+128;}
    template<class T> bool Read(std::uintptr_t at,T& value){return Charge(1)&&memory.Read(at,value);}
};
struct ProductionLoadoutHost {
    ProductionLoadoutReader& reader;unsigned calls=0;
    bool Current(const cfgofflinesettings::Request&){return reader.Charge(128);}
    bool InvokeGrant(std::uintptr_t,std::uintptr_t,std::uintptr_t){++calls;return true;}
};
int main(){unsigned failures=0,tests=0;const auto check=[&](bool value,const char* label){++tests;if(!value){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgloadout;
    for(unsigned choice=2;choice<=8;++choice)for(unsigned fault=0;fault<12;++fault){Fixture f;
        const bool weapon=choice>=6;const unsigned selected=weapon?choice-4:choice;
        if(selected>5&&!weapon)continue;
        expanded::Request request{f.request.owner,weapon?Slot::Weapon:Slot::Inventory,selected};
        const auto name=ClassName(request.slot,request.choice);char narrow[128]{};for(unsigned i=0;i<name.size();++i)narrow[i]=static_cast<char>(name[i]);f.Name(0,narrow);
        f.memory.Set(f.Item+cfgprobe::ObjectClass,std::uintptr_t{0x390000});f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
        f.memory.Set(0x390000+cfgprobe::ClassSuper,std::uintptr_t{});
        f.Slot(8,f.Item);f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        f.memory.Set(f.Pawn+restricteddisk::EquippedItem,std::uintptr_t{});
        f.memory.Set(f.Defaults+restricteddisk::IsLarge,static_cast<std::uint8_t>(weapon));
        expanded::NativeAccess<Memory,Host> access(f.memory,f.host,request,f.Module,f.Loaded);expanded::Evidence evidence;
        ProductionLoadoutReader budget{f.memory};ProductionLoadoutHost production{budget};
        expanded::NativeAccess<ProductionLoadoutReader,ProductionLoadoutHost> productionAccess(budget,production,request,f.Module,f.Loaded);
        expanded::Evidence productionEvidence;
        if(fault==1)f.Name(0,"WrongItem_C");
        if(fault==2)f.memory.Set(f.World+cfgprobe::WorldNetDriver,std::uintptr_t{0x400000});
        if(fault==3)f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x400000});
        if(fault==4)f.memory.Set(f.Defaults+restricteddisk::IsLarge,static_cast<std::uint8_t>(!weapon));
        if(fault==9){f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x3A0000});f.memory.Set(0x3A0000+cfgprobe::ClassSuper,std::uintptr_t{0x390000});}
        if(fault>=10){constexpr std::uintptr_t existing=0x3B0000;
            f.memory.Set(f.Pawn+restricteddisk::EquippedItem,existing);f.memory.Set(existing+cfgprobe::ObjectClass,f.Loaded);
            f.memory.Set(existing+restricteddisk::OwningCharacter,fault==11?std::uintptr_t{0x400000}:f.Pawn);
            f.memory.Set(existing+restricteddisk::IsLarge,std::uint8_t{1});f.memory.Set(existing+restricteddisk::IsSpecial,std::uint8_t{});}
        if((fault>=1&&fault<=4)||fault==11){check(!access.Observe(evidence,true)&&!f.host.calls,"invalid discovery or foreign equipped item refused");continue;}
        check(access.Observe(evidence,true),"typed exact loaded class admitted");
        if(!fault){budget.Phase();check(productionAccess.Observe(productionEvidence,true),"production budget prepare");
            budget.Phase();check(productionAccess.Grant(productionEvidence)&&production.calls==1,"production budget grant");}
        if(fault==5){f.memory.Set(f.Table+24*3+16,int{999});check(!access.Grant(evidence)&&!f.host.calls&&!access.grantDispatched,"stale class refused before native call");continue;}
        if(fault==6){f.memory.Set(f.Pawn+restricted::itemdisk::PickingItem,std::uintptr_t{0x400000});check(!access.Grant(evidence)&&!f.host.calls&&!access.grantDispatched,"busy native pickup refused");continue;}
        check(access.Grant(evidence)&&f.host.calls==1,"native grant dispatched once");
        if(fault==7){check(!access.Verify(evidence),"native return is not ownership proof");continue;}
        f.Added();f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        if(weapon){f.memory.Set(f.Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{});f.memory.Set(f.Pawn+restricteddisk::EquippedItem,f.Created);f.memory.Set(f.Created+restricteddisk::IsLarge,std::uint8_t{1});}
        if(fault==8)f.memory.Set(f.Created+restricteddisk::OwningCharacter,std::uintptr_t{0x400000});
        check(access.Verify(evidence)==(fault!=8),"grant outcome requires exact ownership");
        if(!fault){budget.Phase();check(productionAccess.Verify(productionEvidence),"production budget verify");budget.Phase();
            std::printf("Expanded loadout production-shape choice%u peak=%u/1536\n",choice,budget.peak);}
    }
    {Fixture f;f.Name(0,"BP_Firecracker_C");
        f.memory.Set(f.World+cfgprobe::ObjectClass,f.Counselor);
        f.memory.Set(f.Item+cfgprobe::ObjectClass,std::uintptr_t{0x390000});f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
        f.memory.Set(0x390000+cfgprobe::ClassSuper,std::uintptr_t{});f.Slot(8,f.Item);
        f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});f.memory.Set(f.Pawn+restricteddisk::EquippedItem,std::uintptr_t{});
        expanded::Runtime runtime;check(runtime.Begin(f.request.owner,{0,4},0),"expanded runtime begins");
        check(!runtime.Step(f.memory,f.host,f.Module,2999)&&!f.host.calls,"initial delay");
        check(runtime.Step(f.memory,f.host,f.Module,3000)&&!f.host.calls,"bounded discovery separated from mutation");
        check(runtime.Step(f.memory,f.host,f.Module,3025)&&!f.host.calls,"registry fallback discovery");
        check(runtime.Step(f.memory,f.host,f.Module,3050)&&!f.host.calls,"fresh prepared grant evidence");
        check(runtime.Step(f.memory,f.host,f.Module,4050)&&f.host.calls==1,"grant consumed once");
        f.Added();f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        check(runtime.Step(f.memory,f.host,f.Module,5050)&&!runtime.Active()&&runtime.CurrentState()==restricted::State::Verified,"owned grant verified");
        check(!runtime.Step(f.memory,f.host,f.Module,99999)&&f.host.calls==1,"completed slot never repeats");
    }
    {Fixture f;expanded::Runtime expired;check(expired.Begin(f.request.owner,{0,4},0),"finite discovery begins");
        check(!expired.Step(f.memory,f.host,f.Module,90001)&&!expired.Active()&&!f.host.calls,"expired discovery never dispatches");}
    // A stock weapon can arrive after prepare. The exact owner/class lease
    // survives; current native AddOrSwap input becomes the verification base.
    {Fixture f;f.Name(0,"Shotgun_C");
     f.memory.Set(f.Item+cfgprobe::ObjectClass,std::uintptr_t{0x390000});f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
     f.memory.Set(0x390000+cfgprobe::ClassSuper,std::uintptr_t{});f.Slot(8,f.Item);
     f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
     f.memory.Set(f.Defaults+restricteddisk::IsLarge,std::uint8_t{1});
     f.memory.Set(f.Pawn+restricteddisk::EquippedItem,std::uintptr_t{});
     expanded::NativeAccess<Memory,Host> access(f.memory,f.host,{f.request.owner,Slot::Weapon,4},f.Module,f.Loaded);expanded::Evidence before;
     check(access.Observe(before,true),"shotgun prepare before stock weapon");
     const std::uintptr_t stock=0x400000;f.memory.Set(stock+cfgprobe::ObjectClass,f.Loaded);
     f.memory.Set(stock+restricteddisk::OwningCharacter,f.Pawn);f.memory.Set(stock+restricteddisk::IsLarge,std::uint8_t{1});
     f.memory.Set(stock+restricteddisk::IsSpecial,std::uint8_t{});f.memory.Set(f.Pawn+restricteddisk::EquippedItem,stock);
     check(access.Grant(before)&&access.grantDispatched&&f.host.calls==1&&before.equipped==stock,"fresh native swap baseline, one dispatch");
    }
    {cfgbundle::Schedule schedule;check(schedule.Start(0,0),"background scheduler begins");schedule.helpersActive=false;
        schedule.loadoutActive=schedule.backgroundLoadout=true;unsigned hunts=0,vehicles=0,loads=0;
        for(unsigned now=100;now<=90000;now+=100){const auto lane=schedule.Due(now);hunts+=lane==cfgbundle::Lane::Hunt;
            vehicles+=lane==cfgbundle::Lane::Vehicle;loads+=lane==cfgbundle::Lane::Loadout;}
        check(hunts>80&&vehicles>60&&loads>100,"class survey cannot starve hunt or vehicle work");}
    {Fixture f;constexpr std::uintptr_t meta=0x390000,existing=0x3B0000,weapon=0x3C0000;
        f.Name(0,"CounselorTwoHandedAxe_C");f.memory.Set(f.Item+cfgprobe::ObjectClass,meta);f.memory.Set(f.Loaded+cfgprobe::ObjectClass,meta);
        f.memory.Set(meta+cfgprobe::ClassSuper,std::uintptr_t{});f.Slot(8,f.Item);f.Slot(9,existing);
        f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{10});
        f.memory.Set(f.Defaults+restricteddisk::IsLarge,std::uint8_t{1});f.memory.Set(f.Pawn+restricteddisk::EquippedItem,std::uintptr_t{});
        f.memory.Set(f.Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{f.ListData,2,2});
        f.memory.Set(f.ListData,existing);f.memory.Set(f.ListData+8,std::uintptr_t{});
        f.memory.Set(existing+cfgprobe::ObjectClass,f.Loaded);f.memory.Set(existing+restricteddisk::OwningCharacter,f.Pawn);
        f.memory.Set(existing+cfgfeature::disk::OuterInventory,f.Small);
        {f.Name(0,"BP_PocketKnife_C");f.memory.Set(f.Defaults+restricteddisk::IsLarge,std::uint8_t{});
            expanded::Request small{f.request.owner,Slot::Inventory,2};expanded::NativeAccess<Memory,Host> smallAccess(f.memory,f.host,small,f.Module,f.Loaded);
            expanded::Evidence refused;check(!smallAccess.Observe(refused,true)&&!f.host.calls,"large sparse support does not relax small-item grant admission");
            f.Name(0,"CounselorTwoHandedAxe_C");f.memory.Set(f.Defaults+restricteddisk::IsLarge,std::uint8_t{1});}
        expanded::Request request{f.request.owner,Slot::Weapon,3};expanded::NativeAccess<Memory,Host> access(f.memory,f.host,request,f.Module,f.Loaded);
        expanded::Evidence before;
        check(access.Observe(before,true),"convergence large grant accepts an owned sparse bag snapshot");
        check(access.Grant(before)&&f.host.calls==1,"sparse bag preserves native AddOrSwap input proofs");
        f.Slot(10,weapon);f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{11});
        f.memory.Set(f.Pawn+restricteddisk::EquippedItem,weapon);f.memory.Set(weapon+cfgprobe::ObjectClass,f.Loaded);
        f.memory.Set(weapon+restricteddisk::OwningCharacter,f.Pawn);f.memory.Set(weapon+restricteddisk::IsLarge,std::uint8_t{1});
        f.memory.Set(weapon+restricteddisk::IsSpecial,std::uint8_t{});
        check(access.Verify(before),"large weapon verification preserves every sparse inventory cell");
        f.memory.Set(f.ListData+8,existing);
        check(!access.Verify(before),"filling a prior empty cell or duplicate item refuses verification");
    }
    {Fixture f;f.Slot(7,f.Package);f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{8});
        std::array<wchar_t,192> path{};std::array<wchar_t,8> shortPath{};
        check(cfgobjectpath::Observe(f.memory,f.Module,f.Loaded,path)&&std::wstring_view(path.data())==L"/Game/Owned/Item.OwnedItem_C","diagnostic path includes proven package and class");
        check(!cfgobjectpath::Observe(f.memory,f.Module,f.Loaded,shortPath)&&!shortPath[0],"bounded path refuses overflow without publishing a misleading partial name");
        f.memory.Set(f.Package+0x20,f.Loaded);
        check(!cfgobjectpath::Observe(f.memory,f.Module,f.Loaded,path)&&!path[0],"cyclic outers cannot publish a diagnostic path");
    }
    std::printf("StartingLoadoutNativeTests: %u checks, %u failures\n",tests,failures);return failures?1:0;
}
