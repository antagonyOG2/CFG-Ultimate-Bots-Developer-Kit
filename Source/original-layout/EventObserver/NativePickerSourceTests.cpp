#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/NativePickerSourceObservation.hpp"
#include "../RuntimeProbe/PickerSourceDiagnostic.hpp"
#include "NativeFrontendEpochSource.hpp"
#include <fstream>
#include <regex>
int main(int argc,char** argv){using namespace cfgmenu;using namespace pickersource;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const auto textName=[](cfgnames::Text& out,std::u16string_view value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());};
    const char* capture=argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-16530-15103-Roots.txt";
    std::ifstream input(capture,std::ios::binary);const std::string text((std::istreambuf_iterator<char>(input)),{});
    const std::regex expression("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=([^ ]+) Flags=([A-F0-9]+) ParameterBytes=([0-9]+) StorageBytes=([0-9]+) ScriptBytes=([0-9]+) NativeExecutor=0x([A-F0-9]+) DestructorLink=0x([A-F0-9]+)[\\s\\S]*?ScriptHexBegin\\r?\\n([A-F0-9\\r\\n]+)\\r?\\nScriptHexEnd");
    const auto parse=[&](const char* name,capture::GraphSnapshot& graph){
        for(auto iterator=std::sregex_iterator(text.begin(),text.end(),expression);iterator!=std::sregex_iterator();++iterator){const auto& m=*iterator;if(m[3].str()!=name)continue;
            graph={};graph.function.object=std::stoull(m[1].str(),nullptr,16);graph.function.owner=std::stoull(m[2].str(),nullptr,16);
            const std::string label=name;textName(graph.function.label,std::u16string(label.begin(),label.end()));textName(graph.function.kind,u"Function");
            graph.flags=static_cast<std::uint32_t>(std::stoul(m[4].str(),nullptr,16));graph.parameterBytes=static_cast<std::uint16_t>(std::stoul(m[5].str()));
            graph.storageBytes=std::stoi(m[6].str());graph.script.count=graph.script.capacity=std::stoi(m[7].str());graph.nativeExecutor=std::stoull(m[8].str(),nullptr,16);graph.destructorLink=std::stoull(m[9].str(),nullptr,16);
            auto hex=m[10].str();hex.erase(std::remove(hex.begin(),hex.end(),'\r'),hex.end());hex.erase(std::remove(hex.begin(),hex.end(),'\n'),hex.end());
            if(hex.size()!=2*static_cast<unsigned>(graph.script.count)||static_cast<unsigned>(graph.script.count)>graph.bytes.size())return false;
            for(unsigned i=0;i<static_cast<unsigned>(graph.script.count);++i)graph.bytes[i]=static_cast<unsigned char>(std::stoul(hex.substr(2*i,2),nullptr,16));return true;}return false;};
    auto main=std::make_unique<capture::GraphSnapshot>(),custom=std::make_unique<capture::GraphSnapshot>();
    check(parse("ExecuteUbergraph_MainMenuWidget",*main)&&parse("ExecuteUbergraph_CustomizeMenuWidget",*custom),"actual complete main/customize graph fixtures");
    if(!main->function.object||!custom->function.object)return 1;
    const auto capturedModule=main->nativeExecutor-startdisk::ScriptExecutor;std::uintptr_t customize=0,counselor=0,jason=0;
    const auto capturedNative=Pointer(*main,MainCustomize-8);
    check(MainShape(*main,main->function.owner,capturedModule)&&CustomizeShape(*custom,custom->function.owner,capturedModule),"actual graph8161/1526 storage1072/137 exact native shape");
    check(Edge(*main,MainCustomize,capturedNative,customize)&&customize==custom->function.owner,"actual serialized Main->Customize native edge");
    check(Edge(*custom,CustomizeCounselor,capturedNative,counselor)&&Edge(*custom,CustomizeJason,capturedNative,jason)&&counselor!=jason,"actual Customize->Counselor/Jason direct edges");
    for(unsigned fault=0;fault<6;++fault){auto graph=*custom;std::uintptr_t result=1;
        if(fault==0)graph.bytes[CustomizeCounselor-9]=0x90;if(fault==1)graph.bytes[CustomizeCounselor-8]^=1;
        if(fault==2)graph.bytes[CustomizeCounselor]=0x76;if(fault==3)graph.bytes[CustomizeCounselor+9]=0xA5;
        if(fault==4)graph.bytes[CustomizeCounselor+10]=0;if(fault==5)graph.script.count=CustomizeCounselor+9;
        check(!Edge(graph,CustomizeCounselor,capturedNative,result)&&!result,"undecoded/mutated edge refuses without pointer output");}
    // Relocate ONLY graph metadata and the proved edge operands into an owned
    // engine model. All historical captured pointers remain inert test bytes.
    NativeFixture fixture;auto& f=fixture.fixture;auto& r=f.r;
    constexpr std::uintptr_t root=0x2300000,customType=0x2310000,counselorType=0x2320000,jasonType=0x2330000,
        menu=0x2340000,stackOwner=0x2350000,stackType=0x2360000,meta=0x2370000,native=0x2380000;
    constexpr std::uintptr_t mainFunction=0x2390000,customFunction=0x23A0000,mainScript=0x2400000,customScript=0x2410000;
    f.Name(MenuFixture::Type,"OfflinePlayMenuWidget_C");f.Name(meta,"WidgetBlueprintGeneratedClass");fixture.Root(menu);fixture.Root(stackOwner);
    r.Set(MenuFixture::Type+cfgprobe::ClassSuper,menu);r.Set(stackType+cfgprobe::ClassSuper,stackOwner);r.Set(NativeFixture::Stack+cfgprobe::ObjectClass,stackType);
    r.Set(MenuFixture::Base+parentdisk::Owner,stackOwner);r.Set(MenuFixture::Base+parentdisk::Constructor,stackOwner);
    r.Set(MenuFixture::Base+parentdisk::PreviousExecutor,parentdisk::PreviousCode);r.Set(MenuFixture::Base+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
    r.Set(MenuFixture::Base+pickersourcedisk::ObjectSlot,MenuFixture::Base+pickersourcedisk::ObjectExecutor);
    r.Set(MenuFixture::Base+pickersourcedisk::FinalSlot,MenuFixture::Base+pickersourcedisk::FinalExecutor);
    r.Set(MenuFixture::Base+pickersourcedisk::ObjectExecutor,pickersourcedisk::ObjectCode);r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,pickersourcedisk::FinalCode);
    const std::array<std::uintptr_t,4> classes={root,customType,counselorType,jasonType};
    const std::array<const char*,4> classNames={"MainMenuWidget_C","CustomizeMenuWidget_C","Counselor_Menu_C","Jason_Select_Widget_C"};
    unsigned rootSlot=0;
    for(unsigned i=0;i<classes.size();++i){f.Name(classes[i],classNames[i]);r.Set(classes[i]+cfgprobe::ObjectClass,meta);r.Set(classes[i]+cfgprobe::ClassSuper,menu);
        if(i==0)rootSlot=fixture.slotCount;fixture.Register(classes[i],i==0?5586:0);}
    r.Set(NativeFixture::Stack+execdisk::StackArray,active::StackArray{NativeFixture::Records,2,2});
    r.Set(NativeFixture::Records+parentdisk::RecordClass,root);r.Set(NativeFixture::Records+execdisk::RecordWidget,std::uintptr_t{0});
    r.Set(NativeFixture::Records+execdisk::RecordBytes+parentdisk::RecordClass,MenuFixture::Type);
    r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,MenuFixture::Widget);
    f.Field(native,menu,MenuFixture::FunctionClass,"PlayTransitionOut",0);fixture.Register(native);
    r.Set(MenuFixture::Base+transitiondisk::Owner,menu);r.Set(MenuFixture::Base+transitiondisk::Constructor,menu);r.Set(MenuFixture::Base+transitiondisk::FunctionCache,native);
    r.Set(native+0xB0,MenuFixture::Base+transitiondisk::Executor);r.Set(MenuFixture::Base+transitiondisk::Executor,transitiondisk::Code);
    r.Set(native+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});r.Set(native+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});r.Set(native+cfgreflection::disk::StorageBytes,8);
    const auto transplant=[&](capture::GraphSnapshot& graph,std::uintptr_t owner,std::uintptr_t function,std::uintptr_t script){
        const auto name=NativeFixture::Narrow(graph.function.label.View());f.Field(function,owner,MenuFixture::FunctionClass,name.c_str(),0);fixture.Register(function);
        r.Set(owner+cfgreflection::disk::Children,function);graph.function.object=function;graph.function.owner=owner;graph.script.data=script;
        r.Set(function+cfgreflection::disk::Children,function+0x100);r.Set(function+cfgreflection::disk::FunctionFlags,graph.flags);
        r.Set(function+cfgreflection::arraydisk::FunctionParameterBytes,graph.parameterBytes);r.Set(function+cfgreflection::disk::StorageBytes,graph.storageBytes);
        r.Set(function+0xB0,MenuFixture::Base+startdisk::ScriptExecutor);r.Set(function+0x68,graph.destructorLink?function+0x100:std::uintptr_t{0});r.Set(function+scriptdisk::Script,graph.script);
        f.Field(function+0x100,function,MenuFixture::IntProperty,"EntryPoint",0);f.Property(function+0x100,4,0);
        r.Set(function+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294ULL});
    };
    auto relocatedMain=*main,relocatedCustom=*custom;transplant(relocatedMain,root,mainFunction,mainScript);transplant(relocatedCustom,customType,customFunction,customScript);
    std::memcpy(relocatedMain.bytes.data()+MainCustomize-8,&native,8);std::memcpy(relocatedMain.bytes.data()+MainCustomize+1,&customType,8);
    std::memcpy(relocatedCustom.bytes.data()+CustomizeCounselor-8,&native,8);std::memcpy(relocatedCustom.bytes.data()+CustomizeCounselor+1,&counselorType,8);
    std::memcpy(relocatedCustom.bytes.data()+CustomizeJason-8,&native,8);std::memcpy(relocatedCustom.bytes.data()+CustomizeJason+1,&jasonType,8);
    r.Set(mainScript,relocatedMain.bytes);r.Set(customScript,relocatedCustom.bytes);
    OptionsReader reader{r};active::Context context;check(active::Observe(reader,MenuFixture::Base,context),"owned active firstOfflinePlay native context");
    auto resources=std::make_unique<Resources>();unsigned reads=0,stage=0;
    check(Observe(reader,context,42,true,*resources,&reads,&stage),"positive actual retained root source with NULL widget");
    check(pickerdiagnostic::NamedClass(reader,context,root,u"MainMenuWidget_C")&&
        !pickerdiagnostic::NamedClass(reader,context,root,u"CustomizeMenuWidget_C"),"failure evidence preserves exact named class/slot gates");
    auto diagnosticGraph=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic graphDetail;
    check(capture::Graph(reader,context.module,resources->mainGraph.function,*diagnosticGraph,graphDetail)&&
        diagnosticGraph->bytes==resources->mainGraph.bytes,"failure evidence uses stable graph snapshot, not guessed operands");
    check(reads<ReadLimit&&resources->root.widget==0&&resources->root.menuClass==root&&resources->counselor==counselorType&&resources->jason==jasonType,"exact class-only source outputs and bounded cold reads");
    check(resources->classes[0].HasAssignedSerial()&&!resources->classes[1].HasAssignedSerial()&&!resources->classes[2].HasAssignedSerial()&&!resources->classes[3].HasAssignedSerial(),"zero metadata serials retained through native authored chain, not assigned");
    check(Retained(reader,context,*resources,42)&&!Retained(reader,context,*resources,43),"current chain lease only actual native owner thread");
    {constexpr std::uintptr_t controllerType=0x2490000;r.Set(context.controller+cfgprobe::ObjectClass,controllerType);fixture.Root(controllerType);
     r.Set(MenuFixture::Base+controllergate::MenuClass,controllerType);r.Set(MenuFixture::Base+controllergate::MenuConstructor,controllerType);
     const auto cell=NativeFixture::Registry+24*static_cast<unsigned>(context.identities[0].index)+16;
     const auto oldSerial=context.identities[0].serial;r.Set(cell,oldSerial+1);active::Context replay;
     check(active::Observe(reader,MenuFixture::Base,replay),"positive rebuilt/recycled frontend world");
     auto rebound=std::make_unique<Resources>();
     check(!Retained(reader,replay,*resources,42),"ordinary old-world lease remains strict");
     check(epochsource::Rebind(reader,replay,*resources,*rebound)&&rebound->source==replay,"explicit epoch adapter verifies retained graphs under new positive native owner");
     {constexpr std::uintptr_t freshEntryType=0x24B0000,freshEntryWidget=0x24C0000;
      f.Name(freshEntryType,"OfflinePlayMenuWidget_C");fixture.Root(freshEntryType);fixture.Register(freshEntryWidget,7100);
      r.Set(freshEntryType+cfgprobe::ClassSuper,menu);r.Set(freshEntryType+cfgprobe::ObjectClass,meta);
      r.Set(freshEntryWidget+cfgprobe::ObjectClass,freshEntryType);
      r.Set(NativeFixture::Records+execdisk::RecordBytes, freshEntryType);
      r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,freshEntryWidget);
      active::Context recreated;auto cold=std::make_unique<Resources>();
      check(active::Observe(reader,MenuFixture::Base,recreated)&&recreated.type==freshEntryType,"new native OfflinePlay Blueprint class, not merely a new widget");
      check(!epochsource::Rebind(reader,recreated,*resources,*rebound),"old class lease never blesses recreated Blueprint class");
      check(Observe(reader,recreated,42,true,*cold)&&cold->source.type==freshEntryType&&Retained(reader,recreated,*cold,42),"fresh cold authored capture uses new entry class and repeated native owner proof");
      r.Set(NativeFixture::Records+execdisk::RecordBytes,context.type);
      r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,context.widget);
     }
     r.Set(NativeFixture::Stack+execdisk::StackArray,active::StackArray{NativeFixture::Records,3,3});
     r.Set(NativeFixture::Records+execdisk::RecordBytes,root);
     r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,std::uintptr_t{});
     r.Set(NativeFixture::Records+2*execdisk::RecordBytes,context.type);
     r.Set(NativeFixture::Records+2*execdisk::RecordBytes+execdisk::RecordWidget,context.widget);
     active::Context deeper;check(active::Observe(reader,MenuFixture::Base,deeper),"rebuilt three-record frontend sampled");
     check(epochsource::Rebind(reader,deeper,*resources,*rebound)&&rebound->root.index==1,"exact immediate MainMenu parent instead of hardcoded depth2");
     r.Set(NativeFixture::Records+execdisk::RecordBytes,customType);
     check(!epochsource::Rebind(reader,deeper,*resources,*rebound),"foreign immediate parent cannot inherit route");
     r.Set(NativeFixture::Stack+execdisk::StackArray,context.array);
     r.Set(NativeFixture::Records+execdisk::RecordBytes,context.type);
     r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,context.widget);
     auto foreign=replay;foreign.player+=0x1000;check(!epochsource::Rebind(reader,foreign,*resources,*rebound),"foreign local player cannot inherit epoch");
     r.Set(NativeFixture::World+cfgprobe::WorldNetDriver,std::uintptr_t{0xEE0000});check(!epochsource::Rebind(reader,replay,*resources,*rebound),"online epoch refused");
     r.Set(NativeFixture::World+cfgprobe::WorldNetDriver,std::uintptr_t{});r.Set(cell,oldSerial);
    }
    auto refused=std::make_unique<Resources>();unsigned refusedReads=0;
    check(!Observe(reader,context,42,false,*refused,&refusedReads)&&!refused->counselor,"unverified image publishes no class source");
    f.Name(MenuFixture::Type,"OfflineBotsSettingsMenuWidget_C");check(!Observe(reader,context,42,true,*refused)&&!refused->counselor,"stock settings not first-entry source workaround");f.Name(MenuFixture::Type,"OfflinePlayMenuWidget_C");
    r.Set(MenuFixture::Base+pickersourcedisk::ObjectSlot,MenuFixture::Base+pickersourcedisk::FinalExecutor);check(!Retained(reader,context,*resources,42),"wrong ObjectConst native slot refuses");r.Set(MenuFixture::Base+pickersourcedisk::ObjectSlot,MenuFixture::Base+pickersourcedisk::ObjectExecutor);
    r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,std::uint8_t{0});check(!Retained(reader,context,*resources,42),"changed FinalFunction native code refuses");r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,pickersourcedisk::FinalCode);
    r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor+19,std::uint8_t{0});check(!Retained(reader,context,*resources,42),"entire FinalFunction E9 rel32 is pinned, including byte20");r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,pickersourcedisk::FinalCode);
    r.Set(MenuFixture::Base+parentdisk::PreviousHelper,std::uint8_t{0});check(!Retained(reader,context,*resources,42),"unproved stack class record leaf refuses");r.Set(MenuFixture::Base+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
    r.Set(NativeFixture::Records+parentdisk::RecordClass,customType);check(!Retained(reader,context,*resources,42),"native root record replacement revokes entire chain");r.Set(NativeFixture::Records+parentdisk::RecordClass,root);
    r.Set(NativeFixture::Registry+24*rootSlot+16,0);check(!Retained(reader,context,*resources,42)&&!Observe(reader,context,42,true,*refused)&&!refused->counselor,"lost positive retained root identity refuses");r.Set(NativeFixture::Registry+24*rootSlot+16,5586);
    auto damaged=relocatedMain.bytes;damaged[MainCustomize+1]^=1;r.Set(mainScript,damaged);check(!Retained(reader,context,*resources,42),"main serialized Customize replacement revokes lease");r.Set(mainScript,relocatedMain.bytes);
    damaged=relocatedCustom.bytes;damaged[CustomizeCounselor-8]^=1;r.Set(customScript,damaged);check(!Retained(reader,context,*resources,42),"customized native function operand replacement refuses");r.Set(customScript,relocatedCustom.bytes);
    r.Set(counselorType+cfgprobe::ClassSuper,std::uintptr_t{0});check(!Retained(reader,context,*resources,42),"Counselor label alone cannot replace native menu ancestry");r.Set(counselorType+cfgprobe::ClassSuper,menu);
    f.Name(jasonType,"Jason_Select_Widget_C_0");check(!Retained(reader,context,*resources,42),"invented picker class alias refuses");f.Name(jasonType,"Jason_Select_Widget_C");
    f.Name(meta,"BlueprintGeneratedClass");check(!Retained(reader,context,*resources,42),"non-widget class metadata refuses");f.Name(meta,"WidgetBlueprintGeneratedClass");
    r.Set(native+cfgreflection::disk::FunctionFlags,std::uint32_t{0});check(!Retained(reader,context,*resources,42),"transition function metadata/ABI changes refuse");r.Set(native+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});
    r.Set(menu+cfgreflection::disk::Children,native);
    r.Set(MenuFixture::Base+transitiondisk::FunctionCache,std::uintptr_t{0});
    check(Observe(reader,context,42,true,*refused)&&refused->transition==native,"zero factory cache resolves exact owned native reflected function without initialization");
    check(Retained(reader,context,*resources,42),"retained zero-cache function uses direct metadata, no repeated discovery");
    r.Set(MenuFixture::Base+transitiondisk::FunctionCache,native+0x100);
    check(!Retained(reader,context,*resources,42),"nonzero conflicting cache cannot substitute for retained native function");
    r.Set(MenuFixture::Base+transitiondisk::FunctionCache,native);
    check(Retained(reader,context,*resources,42),"restored fixture retains exact proof, not game rollback");
    for(const auto owner:std::array<std::pair<unsigned,int>,3>{{{0,100},{1,101},{4,103}}}){
        r.Set(NativeFixture::Registry+24*owner.first+16,owner.second+1);active::Context fresh;
        check(active::Observe(reader,MenuFixture::Base,fresh)&&!Retained(reader,fresh,*resources,42),"fresh active context cannot bless reused world/controller/HUD source identity");
        r.Set(NativeFixture::Registry+24*owner.first+16,owner.second);}
    constexpr std::uintptr_t nextWidget=0x24A0000;fixture.Register(nextWidget,7000);r.Set(nextWidget+cfgprobe::ObjectClass,counselorType);
    r.Set(NativeFixture::Records+execdisk::RecordBytes+parentdisk::RecordClass,counselorType);
    r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,nextWidget);
    r.Set(NativeFixture::Registry+24*2+16,0); // OfflinePlay widget no longer supplies any lease.
    active::Context pushed;
    check(active::Observe(reader,MenuFixture::Base,pushed)&&pushed.widget==nextWidget&&Retained(reader,pushed,*resources,42),
        "new native active menu allowed; source root is retained CLASS, not dead OfflinePlay widget");
    r.Set(NativeFixture::Records+parentdisk::RecordClass,customType);check(!Retained(reader,pushed,*resources,42),"new picker owner still cannot retain replaced native root class");
    std::printf("Native picker source owned cold reads=%u cap=%u stage=%u\n",reads,ReadLimit,stage);
    std::printf("CFG retained main/customize/counselor/Jason source: %s (%u checks, %u failures). Owned captured fixtures; no game access/calls/publication.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
