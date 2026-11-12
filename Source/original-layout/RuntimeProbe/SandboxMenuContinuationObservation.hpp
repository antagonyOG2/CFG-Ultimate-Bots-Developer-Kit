#pragma once
#include "MenuPanelChildrenObservation.hpp"
#include "PendingModeMemberObservation.hpp"
#include "MenuScriptObservation.hpp"
#include "MenuPush.generated.hpp"
#include "SettingsStart.generated.hpp"
#include "OfflineEntrySettingsObservation.hpp"
#include "../Core/SandboxMenuContinuation.hpp"
#include <memory>
namespace cfgmenu::mapcontinuation {
constexpr std::u16string_view ClickName=u"BndEvt__btnButton_K2Node_ComponentBoundEvent_92_OnButtonClickedEvent__DelegateSignature";
struct Binding {
    active::Context context{};std::uintptr_t settings=0,panel=0,buttonType=0,initialMap=0,initialMode=0;
    cfgprobe::lifetime::SlotObservation settingsIdentity{};
    panelcapture::Snapshot children{};
    cfgreflection::Property slots{},parent{},content{},map{},mode{},before{},pending{};
    std::array<std::uintptr_t,panelcapture::ChildLimit> maps{},modes{};
    continuation::Plan plan{};
    const entrysettings::Resources* source=nullptr; // Process-owned, immutable authored resource observation.
};
template<class Reader> bool Member(Reader& r,const active::Context& c,std::uintptr_t type,std::u16string_view name,
    std::u16string_view kind,int bytes,int offset,cfgreflection::Property& out,bool hierarchy=true){
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgreflection::Field field;cfgreflection::Property p;
    if(!reflection.Find(type,name,field,hierarchy)||field.kind.View()!=kind||!capture::RawProperty(r,field,p)||
       p.dimension!=1||p.elementBytes!=bytes||p.offset!=offset)return false;out=p;return true;
}
template<class Reader> bool SameMember(Reader& r,std::uintptr_t module,const cfgreflection::Property& p){
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field field;cfgreflection::Property fresh;
    return reflection.ReadField(p.field.object,p.field.owner,field)&&field.type==p.field.type&&field.next==p.field.next&&field.name==p.field.name&&
        capture::RawProperty(r,field,fresh)&&pendingmember::Same(p,fresh);
}
template<class Reader> bool LoadedSettings(Reader& r,const active::Context& c,std::uintptr_t settings,cfgprobe::lifetime::SlotObservation& identity,
    const entrysettings::Resources* source=nullptr){
    cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text label;std::uintptr_t native=0,ctor=0;
    return names.Object(settings,label)&&label.View()==u"OfflineBotsSettingsMenuWidget_C"&&
        r.Read(c.module+pushdisk::MenuClass,native)&&r.Read(c.module+pushdisk::MenuConstructor,ctor)&&cfgprobe::MatchingClassCache(native,ctor)&&
        maps::Subclass(r,settings,native)&&cfgprobe::lifetime::ReadSlot(r,c.registry,settings,identity)&&
        (identity.HasAssignedSerial()||(source&&source->settings==settings&&identity==source->settingsIdentity&&entrysettings::Retained(r,c,*source)));
}
template<class Reader> bool Current(Reader& r,const Binding& b,std::uint32_t thread){
    if(thread!=b.context.thread||!active::Current(r,b.context,thread))return false;
    cfgprobe::lifetime::SlotObservation identity;std::uintptr_t map=0,mode=0;
    if(!LoadedSettings(r,b.context,b.settings,identity,b.source)||!(identity==b.settingsIdentity)||
       !r.Read(b.context.mode+selectiondisk::PendingOfflineMap,map)||map!=b.initialMap||
       !r.Read(b.context.mode+selectiondisk::PendingOfflineMode,mode)||mode!=b.initialMode)return false;
    for(const auto* p:{&b.slots,&b.parent,&b.content,&b.map,&b.mode,&b.before,&b.pending})if(!SameMember(r,b.context.module,*p))return false;
    cfgreflection::Observation<Reader> reflection(r,b.context.module);std::uintptr_t panel=0;
    if(!reflection.ReadObjectMember(b.context.widget,b.context.type,u"MapListVerticalBox",panel)||panel!=b.panel)return false;
    panelcapture::Snapshot fresh;
    if(!panelcapture::Observe(r,b.panel,static_cast<unsigned>(b.slots.offset),static_cast<unsigned>(b.parent.offset),static_cast<unsigned>(b.content.offset),fresh)||
       !(fresh.slots==b.children.slots))return false;
    for(int i=0;i<fresh.slots.count;++i){const auto index=static_cast<unsigned>(i);const auto& button=fresh.children[index];std::uintptr_t value=0;
        if(!(button==b.children.children[index])||!r.Read(button.widget+static_cast<unsigned>(b.map.offset),value)||value!=b.maps[index]||
           !r.Read(button.widget+static_cast<unsigned>(b.mode.offset),value)||value!=b.modes[index])return false;
    }
    return true;
}
// Existing loaded settings class from an actual owned stock settings screen;
// no string-based load, registry search, mode switch, human repair or API call.
// Supports only shipping Sandbox buttons; native developer/test maps stay stock.
template<class Reader> bool Observe(Reader& r,const active::Context& c,std::uintptr_t settings,std::uint32_t thread,Binding& out,unsigned* stage=nullptr,
    const entrysettings::Resources* source=nullptr){
    out={};Binding b;b.context=c;b.settings=settings;b.source=source;cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text label;
    if(stage)*stage=1;
    if(thread!=c.thread||!active::Current(r,c,thread)||!names.Object(c.type,label)||label.View()!=u"PickSandboxMapMenuWidget_C"||
       !LoadedSettings(r,c,settings,b.settingsIdentity,source))return false;
    std::uintptr_t native=0,ctor=0,modeType=0,panelType=0,slot=0,slotType=0;
    cfgreflection::Observation<Reader> reflection(r,c.module);
    if(stage)*stage=2;
    if(!r.Read(c.module+selectiondisk::SCGame_MenuClass,native)||!r.Read(c.module+selectiondisk::SCGame_MenuConstructor,ctor)||
       !cfgprobe::MatchingClassCache(native,ctor)||!r.Read(c.mode+cfgprobe::ObjectClass,modeType)||!maps::Subclass(r,modeType,native)||
       !Member(r,c,native,u"PendingOfflineSettingsWidget",u"ClassProperty",8,0x528,b.pending,false)||b.pending.flags!=0x0/*CFG3.5 address withheld*/40000204ULL||
       !r.Read(c.mode+selectiondisk::PendingOfflineMap,b.initialMap)||!r.Read(c.mode+selectiondisk::PendingOfflineMode,b.initialMode)||
       !reflection.ReadObjectMember(c.widget,c.type,u"MapListVerticalBox",b.panel)||!b.panel||!r.Read(b.panel+cfgprobe::ObjectClass,panelType)||
       !Member(r,c,panelType,u"Slots",u"ArrayProperty",16,0xF8,b.slots)||!r.Read(b.panel+0xF8,b.children.slots)||!b.children.slots.Valid()||
       !r.Read(b.children.slots.data,slot)||!cfgprobe::ObjectPointer(slot)||!r.Read(slot+cfgprobe::ObjectClass,slotType)||
       !Member(r,c,slotType,u"Parent",u"ObjectProperty",8,0x28,b.parent)||!Member(r,c,slotType,u"Content",u"ObjectProperty",8,0x30,b.content)||
       !panelcapture::Observe(r,b.panel,0xF8,0x28,0x30,b.children))return false;
    b.buttonType=b.children.children[0].type;
    if(stage)*stage=3;
    if(!names.Object(b.buttonType,label)||label.View()!=u"WideMenuMapButton_C"||
       !Member(r,c,b.buttonType,u"MapDefinitionClass",u"ClassProperty",8,0x530,b.map,false)||b.map.flags!=0x0/*CFG3.5 address withheld*/40000205ULL||
       !Member(r,c,b.buttonType,u"ModeDefinitionClass",u"ClassProperty",8,0x538,b.mode,false)||b.mode.flags!=0x0/*CFG3.5 address withheld*/40000205ULL||
       !Member(r,c,b.buttonType,u"MenuBeforeLoadingClass",u"ClassProperty",8,0x540,b.before,false)||b.before.flags!=0x0/*CFG3.5 address withheld*/40010205ULL)return false;
    auto click=std::make_unique<capture::DiagnosticFunctionEvidence>();auto graph=std::make_unique<capture::DiagnosticFunctionEvidence>();
    if(stage)*stage=4;
    if(!capture::Function(r,c.module,b.buttonType,ClickName,*click)||click->function.owner!=b.buttonType||click->flags!=0x08000000||
       click->parameterBytes||click->storageBytes||click->fieldsCount||click->destructorLink||click->script.count!=22||
       click->nativeExecutor!=c.module+startdisk::ScriptExecutor||click->bytes[0]!=0x90||click->bytes[13]!=0x5A||
       click->bytes[18]!=0x97||click->bytes[19]!=0xA5||click->bytes[20]!=0x39||click->bytes[21]!=0xE5||
       !capture::Function(r,c.module,b.buttonType,u"ExecuteUbergraph_WideMenuMapButton",*graph)||graph->function.owner!=b.buttonType||
       graph->flags||graph->parameterBytes!=4||graph->storageBytes!=73||graph->script.count!=1196||
       graph->nativeExecutor!=c.module+startdisk::ScriptExecutor)return false;
    std::int32_t entry=0;std::memcpy(&entry,click->bytes.data()+14,4);if(entry!=0x72)return false;
    if(stage)*stage=5;
    std::uint32_t noMenu=0;std::memcpy(&noMenu,graph->bytes.data()+0x91,4);
    if(graph->bytes[0x90]!=0x6F||noMenu!=0x1CE||graph->bytes[0x1CD]!=0x87||graph->bytes[0x23C]!=0xF5)return false;
    for(const auto& operand:std::array<std::pair<unsigned,std::uintptr_t>,4>{{{0x86,b.before.field.object},{0x135,b.mode.field.object},{0x15E,b.map.field.object},{0x1C4,b.before.field.object}}}){
        std::uintptr_t value=0;std::memcpy(&value,graph->bytes.data()+operand.first,8);if(value!=operand.second)return false;
    }
    // Native shipping catalogue membership only; native button/Start keeps
    // responsibility for package validation and normal/snow weather travel.
    std::uintptr_t registry=0,defaults=0,mapNative=0,modeNative=0,registryNative=0;
    if(stage)*stage=6;
    const auto cache=[&](std::uintptr_t a,std::uintptr_t z,std::uintptr_t& value){std::uintptr_t other=0;return r.Read(c.module+a,value)&&r.Read(c.module+z,other)&&cfgprobe::MatchingClassCache(value,other);};
    if(!cache(selectiondisk::SCMapDefinitionClass,selectiondisk::SCMapDefinitionConstructor,mapNative)||
       !cache(selectiondisk::SCModeDefinitionClass,selectiondisk::SCModeDefinitionConstructor,modeNative)||
       !cache(selectiondisk::SCMapRegistryClass,selectiondisk::SCMapRegistryConstructor,registryNative)||
       !r.Read(c.gameInstance+selectiondisk::MapRegistryClass,registry)||!maps::Subclass(r,registry,registryNative)||
       !r.Read(registry+cfgspawn::ClassDefaultObject,defaults)||!cfgprobe::ObjectPointer(defaults))return false;
    maps::Array shipping,supported;std::array<std::uintptr_t,64> shippingMaps{},supportedMaps{};std::uintptr_t sandbox=0,sandboxDefaults=0;
    if(!r.Read(defaults+selectiondisk::ShippingMapList,shipping)||!shipping.Sane(64))return false;
    for(int i=0;i<shipping.count;++i)if(!r.Read(shipping.data+8*static_cast<unsigned>(i),shippingMaps[static_cast<unsigned>(i)]))return false;
    if(stage)*stage=7;
    for(int i=0;i<b.children.slots.count;++i){const auto index=static_cast<unsigned>(i);const auto& button=b.children.children[index];std::uintptr_t previous=0;
        if(button.type!=b.buttonType||!r.Read(button.widget+0x530,b.maps[index])||!maps::Subclass(r,b.maps[index],mapNative)||
           !r.Read(button.widget+0x538,b.modes[index])||!maps::Subclass(r,b.modes[index],modeNative)||
           !r.Read(button.widget+0x540,previous)||previous)return false;
        if(!sandbox){sandbox=b.modes[index];catalogues::StringValue alias;
            if(!names.Object(sandbox,label)||label.View()!=u"ModeDef_Sandbox_C"||!r.Read(sandbox+cfgspawn::ClassDefaultObject,sandboxDefaults)||
               !cfgprobe::ObjectPointer(sandboxDefaults)||!catalogues::String(r,sandboxDefaults+selectiondisk::Alias,alias)||alias.View()!=u"snbx"||
               !r.Read(sandboxDefaults+selectiondisk::SupportedMaps,supported)||!supported.Sane(64))return false;
            for(int j=0;j<supported.count;++j)if(!r.Read(supported.data+8*static_cast<unsigned>(j),supportedMaps[static_cast<unsigned>(j)]))return false;
        }
        if(b.modes[index]!=sandbox)return false;bool inShipping=false,inSupported=false;
        for(int j=0;j<shipping.count;++j)inShipping|=shippingMaps[static_cast<unsigned>(j)]==b.maps[index];
        for(int j=0;j<supported.count;++j)inSupported|=supportedMaps[static_cast<unsigned>(j)]==b.maps[index];
        if(inShipping&&inSupported)b.plan.cells[++b.plan.count]={button.widget+0x540,0};
    }
    if(!b.plan.count||b.plan.count>=continuation::MaximumCells)return false;
    if(stage)*stage=8;
    b.plan.settingsClass=settings;b.plan.cells[0].address=c.mode+0x528;
    if(!r.Read(b.plan.cells[0].address,b.plan.cells[0].previous)||
       (b.plan.cells[0].previous&&!cfgprobe::ObjectPointer(b.plan.cells[0].previous)))return false;++b.plan.count;
    if(!Current(r,b,thread))return false;out=b;return true;
}
}
