#define main ExistingActiveMenuTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "../RuntimeProbe/NativeMapSelectionObservation.hpp"
struct MapFixture : Fixture {
    static constexpr std::uintptr_t InstanceType=0x600000,FrontendType=0x610000,RegistryType=0x620000,RegistryNative=0x630000,
        Defaults=0x640000,List=0x650000,DefinitionType=0x660000,MapA=0x670000,MapB=0x680000,Settings=0x690000,OfflineMode=0x6A0000,
        ModeDefinition=0x6B0000,ModeDefaults=0x6C0000,Supported=0x6D0000;
    MapFixture(){using namespace cfgmenu::selectiondisk;using namespace cfgprobe;
        const std::array<std::pair<std::uintptr_t,std::uintptr_t>,10> caches{{
            {SCGame_MenuClass,FrontendType},{SCGame_MenuConstructor,FrontendType},{SCGameInstanceClass,InstanceType},{SCGameInstanceConstructor,InstanceType},
            {SCMapRegistryClass,RegistryNative},{SCMapRegistryConstructor,RegistryNative},{SCMapDefinitionClass,DefinitionType},{SCMapDefinitionConstructor,DefinitionType},
            {SCModeDefinitionClass,ModeDefinition},{SCModeDefinitionConstructor,ModeDefinition}}};
        for(const auto& cache:caches)reader.Set(Module+cache.first,cache.second);
        reader.Set(Instance+ObjectClass,InstanceType);reader.Set(InstanceType+ClassSuper,std::uintptr_t{0});
        reader.Set(Mode+ObjectClass,FrontendType);reader.Set(FrontendType+ClassSuper,std::uintptr_t{0});
        reader.Set(Instance+MapRegistryClass,RegistryType);reader.Set(RegistryType+ClassSuper,RegistryNative);reader.Set(RegistryNative+ClassSuper,std::uintptr_t{0});
        reader.Set(RegistryType+cfgspawn::ClassDefaultObject,Defaults);reader.Set(Defaults+ObjectClass,RegistryType);
        reader.Set(Defaults+ShippingMapList,cfgmenu::maps::Array{List,2,8});reader.Set(List,MapA);reader.Set(List+8,MapB);
        reader.Set(Defaults+AdditionalDevelopmentMaps,cfgmenu::maps::Array{});reader.Set(Defaults+AdditionalEditorMaps,cfgmenu::maps::Array{});
        reader.Set(MapA+ClassSuper,DefinitionType);reader.Set(MapB+ClassSuper,DefinitionType);reader.Set(DefinitionType+ClassSuper,std::uintptr_t{0});
        reader.Set(Mode+PendingOfflineMap,MapB);reader.Set(Mode+PendingOfflineMode,OfflineMode);reader.Set(Mode+PendingOfflineSettingsWidget,Type);
        reader.Set(OfflineMode+ClassSuper,ModeDefinition);reader.Set(ModeDefinition+ClassSuper,std::uintptr_t{0});
        reader.Set(OfflineMode+cfgspawn::ClassDefaultObject,ModeDefaults);reader.Set(ModeDefaults+ObjectClass,OfflineMode);
        reader.Set(ModeDefaults+SupportedMaps,cfgmenu::maps::Array{Supported,2,2});reader.Set(Supported,MapB);reader.Set(Supported+8,MapA);
    }
};
#ifndef CFGMENU_MAP_FIXTURE_ONLY
int main(){using namespace cfgmenu;unsigned failures=0;const auto check=[&](bool ok,const char* label){if(!ok){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<22;++fault){MapFixture f;active::Context current;check(active::Observe(f.reader,Fixture::Module,current),"known active frontend fixture");
        using namespace selectiondisk;using F=MapFixture;
        if(fault==1)f.reader.Set(F::List+8,F::MapA);
        if(fault==2)f.reader.Set(F::Mode+PendingOfflineMap,F::Settings);
        if(fault==3)f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,65,65});
        if(fault==4)f.reader.Set(F::Defaults+cfgprobe::ObjectClass,F::RegistryNative);
        if(fault==5)f.reader.Set(F::MapB+cfgprobe::ClassSuper,F::MapB);
        if(fault==6)f.reader.Set(F::Module+SCMapDefinitionConstructor,F::MapB);
        if(fault==7)f.reader.Set(F::Mode+PendingOfflineSettingsWidget,F::Settings);
        if(fault==8){f.reader.changeAt=F::List;f.reader.changeSlot=F::Mode+PendingOfflineMap;f.reader.newPointer=F::MapA;}
        if(fault==11){f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,1,8});f.reader.Set(F::Defaults+AdditionalDevelopmentMaps,maps::Array{F::List+8,1,1});}
        if(fault==12)f.reader.Set(F::Defaults+AdditionalEditorMaps,maps::Array{0,0,1});
        if(fault==13)f.reader.Set(F::ModeDefaults+SupportedMaps,maps::Array{F::Supported+8,1,1});
        if(fault==14)f.reader.Set(F::ModeDefaults+cfgprobe::ObjectClass,F::ModeDefinition);
        if(fault==15)f.reader.Set(F::Supported+8,F::MapB);
        if(fault==16){f.reader.changeAt=F::Supported;f.reader.changeSlot=F::List;f.reader.newPointer=F::MapB;}
        if(fault==17)f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,2,512});
        if(fault==18)f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,2,1});
        if(fault==19)f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,2,-1});
        if(fault==20||fault==21){f.reader.Set(F::Defaults+ShippingMapList,maps::Array{F::List,50,56});
            for(unsigned i=2;i<50;++i){const auto extra=std::uintptr_t{0x710000}+0x100*i;
                f.reader.Set(F::List+8*i,extra);f.reader.Set(extra+cfgprobe::ClassSuper,F::DefinitionType);}
            if(fault==21){f.reader.changeAt=F::Supported;f.reader.changeSlot=F::List+8*49;f.reader.newPointer=F::MapB;}
        }
        maps::Selection result;unsigned consumed=0;maps::Diagnostic diagnostic;
        const auto ok=maps::Observe(f.reader,current,fault==9?41u:42u,fault!=10,result,&consumed,&diagnostic);
        check(ok==(fault==0||fault==12||fault==17||fault==20),"native shipping/mode intersection rejection; handles actual CFG50-entry header");check(consumed<=maps::ReadLimit,"hard read cap, no gameplay registry discovery");
        check(diagnostic.reads==consumed&&(diagnostic.failure==maps::Failure::None)==ok,"specific refusal stage retained without changing selection authority");
        if(fault==6)check(diagnostic.failure==maps::Failure::CacheIdentity&&diagnostic.cachePair==3,
            "pinpoints failing native definition cache, no fallback");
        if(fault==7)check(diagnostic.failure==maps::Failure::PendingSettings&&diagnostic.settings==F::Settings,
            "records wrong settings owner without accepting it");
        if(fault==3)check(diagnostic.failure==maps::Failure::ShippingArray&&diagnostic.shippingRead&&diagnostic.shipping.count==65&&
            diagnostic.shipping.capacity==65&&diagnostic.shipping.data==F::List,"raw read success and header distinguish policy refusal from failed read");
        if(ok){check(result.map==F::MapB&&result.selectedIndex==0&&result.count==2&&result.entries[0]==F::MapB,"actual selected class in native mode-supported order, no first-loaded fallback");
            if(fault==17)check(consumed==238,"spare allocation capacity creates zero extra reads");
            std::printf("Native map catalogue reads=%u\n",consumed);}
        else check(!result.map&&!result.maps.data,"no stale selection published");
    }
    std::printf("CFG active native map catalogue selection tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
#endif
