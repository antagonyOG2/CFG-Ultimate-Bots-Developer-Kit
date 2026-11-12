#pragma once
#include "ActiveMenuObservation.hpp"
#include "SetupSelections.generated.hpp"
#include "SpawnBindings.generated.hpp"
namespace cfgmenu::maps {
// Physical CFG capture2859-27298 has50 shipping entries (Num50/Max56).
// Map catalogue capacity is separate from save-cache and UI option limits.
constexpr unsigned MaximumMaps=64,ReadLimit=2048;
enum class Failure {None,ImageOrOwner,CacheRead,CacheIdentity,InstanceAncestry,ModeAncestry,
    RegistryClass,RegistryDefaults,RegistryDefaultsType,ShippingArray,DevelopmentArray,EditorArray,ModeDefaults,SupportedArray,
    PendingMap,PendingMode,PendingSettings,MapEntry,CatalogueCapacity,SelectedMapAbsent,
    SampleChanged,CacheChanged,OwnerChanged};
inline const char* Name(Failure value){
    switch(value){
    case Failure::None:return "none";case Failure::ImageOrOwner:return "image_or_current_owner";
    case Failure::CacheRead:return "native_class_cache_read";case Failure::CacheIdentity:return "native_class_cache_identity";
    case Failure::InstanceAncestry:return "game_instance_ancestry";case Failure::ModeAncestry:return "frontend_mode_ancestry";
    case Failure::RegistryClass:return "map_registry_class";case Failure::RegistryDefaults:return "map_registry_defaults";
    case Failure::RegistryDefaultsType:return "map_registry_defaults_type";case Failure::ShippingArray:return "shipping_array";
    case Failure::DevelopmentArray:return "development_array";case Failure::EditorArray:return "editor_array";
    case Failure::ModeDefaults:return "mode_definition_defaults";case Failure::SupportedArray:return "mode_supported_maps";
    case Failure::PendingMap:return "pending_map";case Failure::PendingMode:return "pending_mode";
    case Failure::PendingSettings:return "pending_settings_class";case Failure::MapEntry:return "native_map_entry";
    case Failure::CatalogueCapacity:return "catalogue_capacity";case Failure::SelectedMapAbsent:return "selected_map_absent";
    case Failure::SampleChanged:return "sample_changed";case Failure::CacheChanged:return "native_cache_changed";
    case Failure::OwnerChanged:return "current_owner_changed";
    }return "unknown";
}
// Refusal evidence only. Partial observations never become a usable Selection.
struct Array {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const Array& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
    // Native Max is reserved allocation capacity, not the number of entries.
    // Bound the WORK by Num; do not reject a two-entry array merely because it
    // reserved more than256 slots. Never read uninitialized capacity entries.
    bool Sane(unsigned countLimit=32)const{return count>=0&&count<=static_cast<int>(countLimit)&&capacity>=count&&
        (!capacity?!data:cfgprobe::ObjectPointer(data)&&
            cfgprobe::lifetime::Extent(data,static_cast<std::uintptr_t>(count?count:1)*8));}
};
struct Diagnostic {Failure failure=Failure::ImageOrOwner;unsigned reads=0,cachePair=0;
    std::array<std::uintptr_t,10> caches{};std::uintptr_t registryClass=0,registryDefaults=0,map=0,mode=0,settings=0;
    Array shipping{},supported{};bool shippingRead=false,supportedRead=false;
};
struct Selection {
    std::uintptr_t registryClass=0,registryDefaults=0,map=0,mode=0,settingsClass=0;
    std::uintptr_t modeDefaults=0;Array maps{},supported{};
    std::array<std::uintptr_t,MaximumMaps> entries{};unsigned count=0,selectedIndex=0;
};
template<class Reader> class Bounded {
    Reader& r_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):r_(r){}
    template<class T>bool Read(std::uintptr_t address,T& out){if(reads>=ReadLimit)return false;++reads;return r_.Read(address,out);}
};
template<class Reader> bool Subclass(Reader& r,std::uintptr_t child,std::uintptr_t expected){
    if(!cfgprobe::ObjectPointer(child)||!cfgprobe::ObjectPointer(expected))return false;
    std::array<std::uintptr_t,32> classes{},parents{};unsigned count=0;bool found=false;
    while(child){if(!cfgprobe::ObjectPointer(child)||count==classes.size())return false;
        for(unsigned i=0;i<count;++i)if(classes[i]==child)return false;
        classes[count]=child;found|=child==expected;
        if(!r.Read(child+cfgprobe::ClassSuper,parents[count]))return false;child=parents[count++];
    }
    for(unsigned i=0;i<count;++i){std::uintptr_t fresh=0;if(!r.Read(classes[i]+cfgprobe::ClassSuper,fresh)||fresh!=parents[i])return false;}
    return found;
}
// Supplied ACTIVE frontend only. Native MapRegistryClass -> its own CDO ->
// native SHIPPING list intersected with pending SCModeDefinition.SupportedMaps,
// in SupportedMaps order, matching native GetMapListByMode39A000. No dev/editor
// lists or global asset discovery. This observes structural membership ONLY;
// it does not replace CFG's additional native ValidateMap package checks or
// advertise these entries as independently travel-eligible. Only a map ALREADY
// selected by CFG's active picker is accepted.
// This is selection
// observation, not permission to create a map/mode or dispatch travel.
template<class Reader> bool Observe(Reader& reader,const active::Context& context,std::uint32_t callerThread,
    bool exactImageVerified,Selection& output,unsigned* consumed=nullptr,Diagnostic* diagnostic=nullptr){
    output={};Bounded<Reader> r(reader);
    Diagnostic detail;
    const auto finish=[&](bool result){if(consumed)*consumed=r.reads;detail.reads=r.reads;
        if(result)detail.failure=Failure::None;if(diagnostic)*diagnostic=detail;return result;};
    if(!exactImageVerified||!active::Current(r,context,callerThread))return finish(false);
    using namespace selectiondisk;std::array<std::uintptr_t,10> caches{};
    const std::array<std::uintptr_t,10> at{{SCGame_MenuClass,SCGame_MenuConstructor,SCGameInstanceClass,SCGameInstanceConstructor,
        SCMapRegistryClass,SCMapRegistryConstructor,SCMapDefinitionClass,SCMapDefinitionConstructor,SCModeDefinitionClass,SCModeDefinitionConstructor}};
    detail.failure=Failure::CacheRead;
    for(unsigned i=0;i<at.size();++i){if(!r.Read(context.module+at[i],caches[i]))return finish(false);detail.caches[i]=caches[i];}
    detail.failure=Failure::CacheIdentity;
    for(unsigned i=0;i<caches.size();i+=2){detail.cachePair=i/2;if(!cfgprobe::MatchingClassCache(caches[i],caches[i+1]))return finish(false);}
    std::uintptr_t instanceType=0,modeType=0;
    detail.failure=Failure::InstanceAncestry;
    if(!r.Read(context.gameInstance+cfgprobe::ObjectClass,instanceType)||!Subclass(r,instanceType,caches[2]))return finish(false);
    detail.failure=Failure::ModeAncestry;
    if(!r.Read(context.mode+cfgprobe::ObjectClass,modeType)||!Subclass(r,modeType,caches[0]))return finish(false);
    Selection selected;
    const auto sample=[&](Selection& value){
        detail.failure=Failure::RegistryClass;
        if(!r.Read(context.gameInstance+MapRegistryClass,value.registryClass))return false;detail.registryClass=value.registryClass;
        if(!Subclass(r,value.registryClass,caches[4]))return false;
        detail.failure=Failure::RegistryDefaults;
        if(!r.Read(value.registryClass+cfgspawn::ClassDefaultObject,value.registryDefaults))return false;detail.registryDefaults=value.registryDefaults;
        if(!cfgprobe::ObjectPointer(value.registryDefaults))return false;
        std::uintptr_t defaultsType=0;
        detail.failure=Failure::RegistryDefaultsType;
        if(!r.Read(value.registryDefaults+cfgprobe::ObjectClass,defaultsType)||defaultsType!=value.registryClass)return false;
        detail.failure=Failure::ShippingArray;
        detail.shippingRead=r.Read(value.registryDefaults+ShippingMapList,value.maps);detail.shipping=value.maps;
        if(!detail.shippingRead||!value.maps.Sane(MaximumMaps))return false;
        detail.failure=Failure::PendingMap;
        if(!r.Read(context.mode+PendingOfflineMap,value.map))return false;detail.map=value.map;if(!cfgprobe::ObjectPointer(value.map))return false;
        detail.failure=Failure::PendingMode;
        if(!r.Read(context.mode+PendingOfflineMode,value.mode))return false;detail.mode=value.mode;if(!cfgprobe::ObjectPointer(value.mode))return false;
        detail.failure=Failure::PendingSettings;
        if(!r.Read(context.mode+PendingOfflineSettingsWidget,value.settingsClass))return false;detail.settings=value.settingsClass;
        if(value.settingsClass!=context.type)return false;
        detail.failure=Failure::ModeDefaults;
        if(!Subclass(r,value.mode,caches[8])||!r.Read(value.mode+cfgspawn::ClassDefaultObject,value.modeDefaults)||
            !cfgprobe::ObjectPointer(value.modeDefaults)||!r.Read(value.modeDefaults+cfgprobe::ObjectClass,defaultsType)||defaultsType!=value.mode)return false;
        detail.failure=Failure::SupportedArray;
        detail.supportedRead=r.Read(value.modeDefaults+SupportedMaps,value.supported);detail.supported=value.supported;
        if(!detail.supportedRead||!value.supported.Sane(MaximumMaps))return false;
        std::array<std::uintptr_t,MaximumMaps> shipping{},supported{};
        for(unsigned i=0;i<static_cast<unsigned>(value.maps.count);++i){detail.failure=Failure::MapEntry;
            if(!r.Read(value.maps.data+8*i,shipping[i])||!Subclass(r,shipping[i],caches[6]))return false;}
        unsigned matches=0;
        for(unsigned i=0;i<static_cast<unsigned>(value.supported.count);++i){std::uintptr_t entry=0;
            detail.failure=Failure::MapEntry;
            if(!r.Read(value.supported.data+8*i,entry)||!Subclass(r,entry,caches[6]))return false;supported[i]=entry;
            bool inShipping=false;for(unsigned j=0;j<static_cast<unsigned>(value.maps.count);++j)inShipping|=shipping[j]==entry;
            if(!inShipping)continue;
            bool duplicate=false;for(unsigned j=0;j<value.count;++j)duplicate|=value.entries[j]==entry;
            if(duplicate)return false; // An ambiguous native mode list is not collapsed into a fabricated index.
            detail.failure=Failure::CatalogueCapacity;if(value.count==value.entries.size())return false;
            value.entries[value.count]=entry;
            if(entry==value.map){value.selectedIndex=value.count;++matches;}++value.count;
        }
        // Recheck every source entry, including the ones excluded from the
        // intersection. A stable array header alone does not prove its values.
        detail.failure=Failure::SampleChanged;std::uintptr_t fresh=0;
        for(unsigned i=0;i<static_cast<unsigned>(value.maps.count);++i)if(!r.Read(value.maps.data+8*i,fresh)||fresh!=shipping[i])return false;
        for(unsigned i=0;i<static_cast<unsigned>(value.supported.count);++i)if(!r.Read(value.supported.data+8*i,fresh)||fresh!=supported[i])return false;
        detail.failure=Failure::SelectedMapAbsent;return matches==1;
    };
    Selection again;
    if(!sample(selected)||!sample(again))return finish(false);detail.failure=Failure::SampleChanged;
    if(selected.registryClass!=again.registryClass||selected.registryDefaults!=again.registryDefaults||
        selected.map!=again.map||selected.mode!=again.mode||selected.settingsClass!=again.settingsClass||!(selected.maps==again.maps)||
        selected.modeDefaults!=again.modeDefaults||!(selected.supported==again.supported)||selected.count!=again.count||
        selected.entries!=again.entries||selected.selectedIndex!=again.selectedIndex)return finish(false);
    detail.failure=Failure::CacheChanged;
    for(unsigned i=0;i<at.size();++i){std::uintptr_t fresh=0;if(!r.Read(context.module+at[i],fresh)||fresh!=caches[i])return finish(false);}
    detail.failure=Failure::OwnerChanged;if(!active::Current(r,context,callerThread))return finish(false);
    output=selected;return finish(true);
}
}
