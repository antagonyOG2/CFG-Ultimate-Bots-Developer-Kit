#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeSandboxMenuContinuation.hpp"
struct MapContinuationFixture:NativeFixture {
    static constexpr std::uintptr_t Settings=0x2100000,MenuNative=0x2110000,ClassKind=0x2120000,
        Pending=0x2130000,PanelField=0x2140000,SlotsField=0x2150000,Slots=0x2160000,Button=0x2170000,ButtonType=0x2180000,
        MapField=0x2190000,ModeField=0x21A0000,BeforeField=0x21B0000,Click=0x21C0000,Graph=0x21D0000,Entry=0x21E0000,
        ClickCode=0x21F0000,GraphCode=0x2200000,MapNative=0x2210000,MapA=0x2220000,MapB=0x2230000,DevMap=0x2240000,
        ModeNative=0x2250000,Sandbox=0x2260000,SandboxDefaults=0x2270000,Supported=0x2280000,Alias=0x2290000,
        RegistryNative=0x22A0000,RegistryType=0x22B0000,RegistryDefaults=0x22C0000,Shipping=0x22D0000;
    MapContinuationFixture(){using namespace cfgmenu;using namespace cfgprobe;using M=MenuFixture;auto& f=fixture;auto& r=f.r;
        f.Name(M::Type,"PickSandboxMapMenuWidget_C");f.Name(Settings,"OfflineBotsSettingsMenuWidget_C");f.Name(ClassKind,"ClassProperty");
        Root(MenuNative);r.Set(Settings+ClassSuper,MenuNative);Register(Settings,200);
        r.Set(M::Base+pushdisk::MenuClass,MenuNative);r.Set(M::Base+pushdisk::MenuConstructor,MenuNative);
        f.Field(Pending,Frontend,ClassKind,"PendingOfflineSettingsWidget",0);f.Property(Pending,8,0x528);
        r.Set(Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x001C001040000204});r.Set(Frontend+cfgreflection::disk::Children,Pending);
        r.Set(Mode+0x518,MapA);r.Set(Mode+0x520,Sandbox);r.Set(Mode+0x528,Settings);
        f.Field(PanelField,M::Type,M::ObjectProperty,"MapListVerticalBox",0);f.Property(PanelField,8,0x180);
        r.Set(M::Type+cfgreflection::disk::Children,PanelField);r.Set(M::Widget+0x180,Panel);
        f.Field(SlotsField,PanelType,M::ArrayProperty,"Slots",0);f.Property(SlotsField,16,0xF8);
        r.Set(PanelType+cfgreflection::disk::Children,SlotsField);r.Set(Panel+0xF8,panelcapture::Array{Slots,3,1000000});
        f.Property(ParentField,8,0x28);f.Property(ContentField,8,0x30);
        for(unsigned i=0;i<3;++i){const auto slot=DonorSlot+0x100*i,button=Button+0x100*i;
            r.Set(Slots+8*i,slot);r.Set(slot+ObjectClass,SlotType);r.Set(slot+0x28,Panel);r.Set(slot+0x30,button);
            r.Set(button+ObjectClass,ButtonType);r.Set(button+0x530,i==0?MapA:i==1?MapB:DevMap);r.Set(button+0x538,Sandbox);r.Set(button+0x540,std::uintptr_t{0});
        }
        f.Name(ButtonType,"WideMenuMapButton_C");Root(ButtonType);
        const auto member=[&](std::uintptr_t at,const char* name,unsigned offset,std::uintptr_t next,std::uint64_t flags){
            f.Field(at,ButtonType,ClassKind,name,next);f.Property(at,8,static_cast<int>(offset));r.Set(at+cfgreflection::disk::PropertyFlags,flags);};
        member(MapField,"MapDefinitionClass",0x530,ModeField,0x0009001040000205);member(ModeField,"ModeDefinitionClass",0x538,BeforeField,0x0009001040000205);
        member(BeforeField,"MenuBeforeLoadingClass",0x540,Click,0x0008001040010205);r.Set(ButtonType+cfgreflection::disk::Children,MapField);
        f.Field(Click,ButtonType,M::FunctionClass,"BndEvt__btnButton_K2Node_ComponentBoundEvent_92_OnButtonClickedEvent__DelegateSignature",Graph);Register(Click);
        f.Field(Graph,ButtonType,M::FunctionClass,"ExecuteUbergraph_WideMenuMapButton",0);Register(Graph);
        const auto setFunction=[&](std::uintptr_t fn,unsigned flags,std::uint16_t parameters,int storage,std::uintptr_t head,std::uintptr_t code,int length){
            r.Set(fn+cfgreflection::disk::FunctionFlags,flags);r.Set(fn+cfgreflection::disk::StorageBytes,storage);
            r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,parameters);r.Set(fn+cfgreflection::disk::Children,head);
            r.Set(fn+0x68,std::uintptr_t{0});r.Set(fn+0xB0,M::Base+startdisk::ScriptExecutor);r.Set(fn+scriptdisk::Script,capture::ScriptHeader{code,length,length});};
        setFunction(Click,0x08000000,0,0,0,ClickCode,22);setFunction(Graph,0,4,73,Entry,GraphCode,1196);
        f.Field(Entry,Graph,M::IntProperty,"EntryPoint",0);f.Property(Entry,4,0);r.Set(Entry+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294});
        std::array<unsigned char,22> click{};click[0]=0x90;click[13]=0x5A;const std::int32_t entry=0x72;std::memcpy(click.data()+14,&entry,4);
        click[18]=0x97;click[19]=0xA5;click[20]=0x39;click[21]=0xE5;r.Set(ClickCode,click);
        std::array<unsigned char,1196> graph{};graph[0x90]=0x6F;const std::uint32_t destination=0x1CE;std::memcpy(graph.data()+0x91,&destination,4);
        graph[0x1CD]=0x87;graph[0x23C]=0xF5;
        for(const auto& p:std::array<std::pair<unsigned,std::uintptr_t>,4>{{{0x86,BeforeField},{0x135,ModeField},{0x15E,MapField},{0x1C4,BeforeField}}})std::memcpy(graph.data()+p.first,&p.second,8);r.Set(GraphCode,graph);
        const auto cache=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t type){r.Set(M::Base+a,type);r.Set(M::Base+b,type);Root(type);};
        cache(selectiondisk::SCMapDefinitionClass,selectiondisk::SCMapDefinitionConstructor,MapNative);
        cache(selectiondisk::SCModeDefinitionClass,selectiondisk::SCModeDefinitionConstructor,ModeNative);
        cache(selectiondisk::SCMapRegistryClass,selectiondisk::SCMapRegistryConstructor,RegistryNative);
        for(const auto map:{MapA,MapB,DevMap})r.Set(map+ClassSuper,MapNative);
        f.Name(Sandbox,"ModeDef_Sandbox_C");r.Set(Sandbox+ClassSuper,ModeNative);r.Set(Sandbox+cfgspawn::ClassDefaultObject,SandboxDefaults);
        std::array<char16_t,5> alias{{u's',u'n',u'b',u'x',0}};r.Set(SandboxDefaults+selectiondisk::Alias,catalogues::StringHeader{Alias,5,5});r.Set(Alias,alias);
        r.Set(SandboxDefaults+selectiondisk::SupportedMaps,maps::Array{Supported,3,3});r.Set(Supported,MapA);r.Set(Supported+8,MapB);r.Set(Supported+16,DevMap);
        r.Set(Instance+selectiondisk::MapRegistryClass,RegistryType);r.Set(RegistryType+ClassSuper,RegistryNative);r.Set(RegistryType+cfgspawn::ClassDefaultObject,RegistryDefaults);
        r.Set(RegistryDefaults+selectiondisk::ShippingMapList,maps::Array{Shipping,2,1000000});r.Set(Shipping,MapA);r.Set(Shipping+8,MapB);
    }
};
template class cfgmenu::mapcontinuation::NativeHost<OptionsReader>;
int main(){using namespace cfgmenu;using F=MapContinuationFixture;unsigned failures=0;
    const auto check=[&](bool okay,const char* text){if(!okay){++failures;std::printf("FAIL %s\n",text);}};
    for(unsigned fault=0;fault<15;++fault){auto f=std::make_unique<F>();auto& r=f->fixture.r;OptionsReader reader{r};active::Context context;
        check(active::Observe(reader,MenuFixture::Base,context),"actual current map-menu chain");
        if(fault==1)r.Set(F::Button+0x538,F::MapA);
        if(fault==2)r.Set(F::Button+0x540,F::Settings);
        if(fault==3)r.Set(F::BeforeField+cfgreflection::disk::PropertyOffset,0x538);
        if(fault==4)r.Set(F::BeforeField+cfgreflection::disk::PropertyFlags,std::uint64_t{0});
        if(fault==5)r.Set(F::ClickCode+14,std::int32_t{0x73});
        if(fault==6)r.Set(F::GraphCode+0x86,F::MapField);
        if(fault==7)r.Set(F::GraphCode+0x91,std::uint32_t{0x1CD});
        if(fault==8)r.Set(F::Graph+cfgreflection::disk::StorageBytes,74);
        if(fault==9)r.Set(F::Slots+8,F::DonorSlot);
        if(fault==10)r.Set(F::DonorSlot+0x28,F::Mode);
        if(fault==11)f->fixture.Name(F::Settings,"DifferentSettings_C");
        if(fault==12)f->fixture.Name(MenuFixture::Type,"PickOfflineBotsMapMenuWidget_C");
        if(fault==13)r.Set(F::Supported+8,F::MapA); // MapB unsupported: not advertised as enabled.
        mapcontinuation::Binding binding;const auto before=r.reads;
        const bool okay=mapcontinuation::Observe(reader,context,F::Settings,fault==14?43:42,binding);
        check(okay==(fault==0||fault==13),"actual native route schema/refusal");
        if(!okay){check(!binding.plan.count&&!binding.context.widget,"partial route never published");continue;}
        check(binding.plan.count==(fault==13?2u:3u)&&binding.plan.cells[0].address==F::Mode+0x528,"pending settings and supported SHIPPING buttons only");
        check(binding.plan.cells[1].address==F::Button+0x540,"only instance continuation; no pending mode/map write");
        if(!fault){std::printf("Native map continuation cold reads=%u\n",r.reads-before);
            check(mapcontinuation::Current(reader,binding,42),"prepared current native map scope");
            r.Set(F::Button+0x530,F::MapB);check(!mapcontinuation::Current(reader,binding,42),"changed native map choice refuses stale writes");}
    }
    std::printf("Concrete native Sandbox map continuation observation: %s (%u failures); owned memory, no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
