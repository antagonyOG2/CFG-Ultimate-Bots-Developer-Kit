#pragma once
#include <fstream>
#include <regex>
// OWNED byte-addressable fixture. Captured addresses are inert serialized
// operands, relocated only where the source/ABI proof explicitly identifies one.
struct PickerPreparedFixture:NativeFixture {
    static constexpr std::uintptr_t RootType=0x2300000,Customize=0x2310000,Counselor=0x2320000,Jason=0x2330000,
        Menu=0x2340000,StackOwner=0x2350000,StackType=0x2360000,Meta=0x2370000,Transition=0x2380000,
        BoolKind=0x2390000,ClassKind=0x23A0000,State=0x23B0000,LobbyState=0x23C0000,
        UserWidget=0x23D0000,ControllerClass=0x23E0000,OwnerGetter=0x23F0000,WidgetVtable=0x2400000;
    cfgmenu::active::Context owner{};std::unique_ptr<cfgmenu::pickersource::Resources> source=std::make_unique<cfgmenu::pickersource::Resources>();
    unsigned sourceReads=0;bool initialized=false;
    static std::string File(const char* path){std::ifstream input(path,std::ios::binary);return {(std::istreambuf_iterator<char>(input)),{}};}
    template<class Function> static bool Parse(const std::string& text,std::string_view wanted,Function& function){
        using namespace cfgmenu;const std::regex expression("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=([^ ]+) Flags=([A-F0-9]+) ParameterBytes=([0-9]+) StorageBytes=([0-9]+) ScriptBytes=([0-9]+) NativeExecutor=0x([A-F0-9]+) DestructorLink=0x([A-F0-9]+)[\\s\\S]*?ScriptHexBegin\\r?\\n([A-F0-9\\r\\n]+)\\r?\\nScriptHexEnd");
        const auto label=[](cfgnames::Text& out,const std::string& value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());};
        for(auto it=std::sregex_iterator(text.begin(),text.end(),expression);it!=std::sregex_iterator();++it){const auto& m=*it;if(m[3].str()!=wanted)continue;
            function={};label(function.function.label,m[3].str());label(function.function.kind,"Function");function.flags=static_cast<std::uint32_t>(std::stoul(m[4].str(),nullptr,16));
            function.parameterBytes=static_cast<std::uint16_t>(std::stoul(m[5].str()));function.storageBytes=std::stoi(m[6].str());function.script.count=function.script.capacity=std::stoi(m[7].str());function.destructorLink=std::stoull(m[9].str(),nullptr,16);
            auto hex=m[10].str();hex.erase(std::remove(hex.begin(),hex.end(),'\r'),hex.end());hex.erase(std::remove(hex.begin(),hex.end(),'\n'),hex.end());
            if(hex.size()!=2*static_cast<unsigned>(function.script.count)||static_cast<unsigned>(function.script.count)>function.bytes.size())return false;
            for(unsigned i=0;i<static_cast<unsigned>(function.script.count);++i)function.bytes[i]=static_cast<unsigned char>(std::stoul(hex.substr(2*i,2),nullptr,16));
            const std::regex fields(" Parameter=0x([A-F0-9]+) Name=([^ ]+) Type=([^ ]+) Offset=([0-9]+) ElementBytes=([0-9]+) Dimension=([0-9]+) Flags=([A-F0-9]+)");const auto block=m.str();
            for(auto field=std::sregex_iterator(block.begin(),block.end(),fields);field!=std::sregex_iterator();++field){const auto& row=*field;
                if(function.fields.size()==1&&function.fieldsCount==1)break; // graph-only EntryPoint view, not all locals
                if(function.fieldsCount==function.fields.size())return false;
                auto& p=function.fields[function.fieldsCount++];label(p.field.label,row[2].str());label(p.field.kind,row[3].str());p.offset=std::stoi(row[4].str());p.elementBytes=std::stoi(row[5].str());p.dimension=std::stoi(row[6].str());p.flags=std::stoull(row[7].str(),nullptr,16);}
            return true;}return false;
    }
    template<class Function> void Function(Function& f,std::uintptr_t type,std::uintptr_t at,std::uintptr_t next){using namespace cfgmenu;auto& model=fixture;auto& r=model.r;
        const auto name=Narrow(f.function.label.View());model.Field(at,type,MenuFixture::FunctionClass,name.c_str(),next);Register(at);
        f.function.object=at;f.function.owner=type;r.Read(at+cfgnames::disk::ObjectName,f.function.name);f.nativeExecutor=MenuFixture::Base+startdisk::ScriptExecutor;
        f.script.data=at+0x1000000;f.script.capacity=f.script.count;f.destructorLink=f.destructorLink?at+0x100:0;
        r.Set(at+cfgreflection::disk::FunctionFlags,f.flags);r.Set(at+cfgreflection::arraydisk::FunctionParameterBytes,f.parameterBytes);r.Set(at+cfgreflection::disk::StorageBytes,f.storageBytes);
        r.Set(at+0xB0,f.nativeExecutor);r.Set(at+0x68,f.destructorLink);r.Set(at+scriptdisk::Script,f.script);r.Set(at+startdisk::FastGraph,std::uintptr_t{0});r.Set(at+startdisk::FastEntry,0);
        r.Set(at+cfgreflection::disk::Children,f.fieldsCount?at+0x100:std::uintptr_t{0});
        for(unsigned i=0;i<f.fieldsCount;++i){auto& p=f.fields[i];const auto field=at+0x100+0x100*i;const auto label=Narrow(p.field.label.View());
            const auto kind=p.field.kind.View()==u"IntProperty"?MenuFixture::IntProperty:p.field.kind.View()==u"BoolProperty"?BoolKind:p.field.kind.View()==u"ClassProperty"?ClassKind:MenuFixture::ObjectProperty;
            model.Field(field,at,kind,label.c_str(),i+1<f.fieldsCount?field+0x100:0);model.Property(field,p.elementBytes,p.offset);r.Set(field+cfgreflection::disk::PropertyFlags,p.flags);p.field.object=field;p.field.owner=at;}
    }
    PickerPreparedFixture(const char* rootPath,const char* counselorPath,const char* jasonPath){using namespace cfgmenu;auto& f=fixture;auto& r=f.r;
        f.Name(MenuFixture::Type,"OfflinePlayMenuWidget_C");f.Name(Meta,"WidgetBlueprintGeneratedClass");f.Name(BoolKind,"BoolProperty");f.Name(ClassKind,"ClassProperty");Root(Menu);Root(StackOwner);Root(State);
        r.Set(MenuFixture::Type+cfgprobe::ClassSuper,Menu);r.Set(StackType+cfgprobe::ClassSuper,StackOwner);r.Set(Stack+cfgprobe::ObjectClass,StackType);
        r.Set(MenuFixture::Base+parentdisk::Owner,StackOwner);r.Set(MenuFixture::Base+parentdisk::Constructor,StackOwner);r.Set(MenuFixture::Base+parentdisk::PreviousExecutor,parentdisk::PreviousCode);r.Set(MenuFixture::Base+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
        r.Set(MenuFixture::Base+pickersourcedisk::ObjectSlot,MenuFixture::Base+pickersourcedisk::ObjectExecutor);r.Set(MenuFixture::Base+pickersourcedisk::FinalSlot,MenuFixture::Base+pickersourcedisk::FinalExecutor);
        r.Set(MenuFixture::Base+pickersourcedisk::ObjectExecutor,pickersourcedisk::ObjectCode);r.Set(MenuFixture::Base+pickersourcedisk::FinalExecutor,pickersourcedisk::FinalCode);
        const std::array<std::uintptr_t,4> classes={RootType,Customize,Counselor,Jason};const std::array<const char*,4> names={"MainMenuWidget_C","CustomizeMenuWidget_C","Counselor_Menu_C","Jason_Select_Widget_C"};
        for(unsigned i=0;i<classes.size();++i){f.Name(classes[i],names[i]);r.Set(classes[i]+cfgprobe::ObjectClass,Meta);r.Set(classes[i]+cfgprobe::ClassSuper,Menu);Register(classes[i],i?0:5586);}
        r.Set(Stack+execdisk::StackArray,active::StackArray{Records,2,2});r.Set(Records,RootType);r.Set(Records+execdisk::RecordWidget,std::uintptr_t{0});
        r.Set(Records+execdisk::RecordBytes,MenuFixture::Type);r.Set(Records+execdisk::RecordBytes+execdisk::RecordWidget,MenuFixture::Widget);
        f.Field(Transition,Menu,MenuFixture::FunctionClass,"PlayTransitionOut",0);Register(Transition);
        r.Set(Menu+cfgreflection::disk::Children,Transition);
        r.Set(MenuFixture::Base+transitiondisk::Owner,Menu);r.Set(MenuFixture::Base+transitiondisk::Constructor,Menu);r.Set(MenuFixture::Base+transitiondisk::FunctionCache,Transition);
        r.Set(Transition+0xB0,MenuFixture::Base+transitiondisk::Executor);r.Set(MenuFixture::Base+transitiondisk::Executor,transitiondisk::Code);r.Set(Transition+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});
        r.Set(Transition+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});r.Set(Transition+cfgreflection::disk::StorageBytes,8);
        const auto roots=File(rootPath);auto main=std::make_unique<capture::GraphSnapshot>(),custom=std::make_unique<capture::GraphSnapshot>();
        if(!Parse(roots,"ExecuteUbergraph_MainMenuWidget",*main)||!Parse(roots,"ExecuteUbergraph_CustomizeMenuWidget",*custom))return;
        Function(*main,RootType,0x2500000,0);Function(*custom,Customize,0x2510000,0);r.Set(RootType+cfgreflection::disk::Children,main->function.object);r.Set(Customize+cfgreflection::disk::Children,custom->function.object);
        std::memcpy(main->bytes.data()+pickersource::MainCustomize-8,&Transition,8);std::memcpy(main->bytes.data()+pickersource::MainCustomize+1,&Customize,8);
        std::memcpy(custom->bytes.data()+pickersource::CustomizeCounselor-8,&Transition,8);std::memcpy(custom->bytes.data()+pickersource::CustomizeCounselor+1,&Counselor,8);
        std::memcpy(custom->bytes.data()+pickersource::CustomizeJason-8,&Transition,8);std::memcpy(custom->bytes.data()+pickersource::CustomizeJason+1,&Jason,8);r.Set(main->script.data,main->bytes);r.Set(custom->script.data,custom->bytes);
        r.Set(MenuFixture::Base+cataloguedisk::SCPlayerStateClass,State);r.Set(MenuFixture::Base+cataloguedisk::SCPlayerStateConstructor,State);r.Set(LobbyState+cfgprobe::ClassSuper,State);
        if(!Picker(File(counselorPath),true)){std::printf("Fixture counselor parse failed\n");return;}
        if(!Picker(File(jasonPath),false)){std::printf("Fixture Jason parse failed\n");return;}
        f.Name(UserWidget,"UserWidget");f.Name(ControllerClass,"PlayerController");Root(UserWidget);Root(ControllerClass);
        r.Set(Menu+cfgprobe::ClassSuper,UserWidget);r.Set(Controller+cfgprobe::ObjectClass,ControllerClass);
        r.Set(MenuFixture::Base+widgetowner::ClassCache,UserWidget);r.Set(MenuFixture::Base+widgetowner::ClassConstructor,UserWidget);
        r.Set(MenuFixture::Base+widgetowner::FunctionCache,OwnerGetter);f.Field(OwnerGetter,UserWidget,MenuFixture::FunctionClass,"GetOwningPlayer",0);Register(OwnerGetter);
        r.Set(OwnerGetter+cfgreflection::disk::Children,OwnerGetter+0x100);r.Set(OwnerGetter+cfgreflection::disk::FunctionFlags,std::uint32_t{0x54020409});
        r.Set(OwnerGetter+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});r.Set(OwnerGetter+cfgreflection::disk::StorageBytes,8);
        r.Set(OwnerGetter+0xB0,MenuFixture::Base+widgetowner::Executor);r.Set(OwnerGetter+0x68,std::uintptr_t{0});r.Set(OwnerGetter+scriptdisk::Script,capture::ScriptHeader{});
        f.Field(OwnerGetter+0x100,OwnerGetter,MenuFixture::ObjectProperty,"ReturnValue",0);f.Property(OwnerGetter+0x100,8,0);
        r.Set(OwnerGetter+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000780ULL});
        r.Set(OwnerGetter+0x100,MenuFixture::Base+counselorpickerdisk::ObjectPropertyVtable);r.Set(OwnerGetter+0x170,ControllerClass);
        r.Set(MenuFixture::Base+widgetowner::Executor,widgetowner::ExecutorCode);r.Set(MenuFixture::Base+widgetowner::VirtualGetter,widgetowner::VirtualCode);
        r.Set(WidgetVtable+widgetowner::VirtualSlot,MenuFixture::Base+widgetowner::VirtualGetter);
        r.Set(MenuFixture::Base+counselorpickerdisk::DynamicCastExecutor,counselorpickerdisk::DynamicCastCode);
        r.Set(MenuFixture::Base+counselorpickerdisk::DynamicCastSlot,MenuFixture::Base+counselorpickerdisk::DynamicCastExecutor);
        constexpr std::uintptr_t stateField=0x2410000,stateObject=0x2420000;
        f.Field(stateField,ControllerClass,MenuFixture::ObjectProperty,"PlayerState",0);f.Property(stateField,8,0x388);
        r.Set(stateField+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0020001040000015ULL});
        r.Set(Controller+0x388,stateObject);r.Set(stateObject+cfgprobe::ObjectClass,State);Register(stateObject,8102);
        // Relocate the exact embedded PlayerState field operand from the actual
        // counselor graph; fixture objects are not captured runtime addresses.
        r.Set(0x2600000+0x1000000+0x105C,stateField);
        r.Set(0x2600000+0x1000000+0x1065,stateField);
        OptionsReader reader{r};unsigned stage=0;initialized=active::Observe(reader,MenuFixture::Base,owner)&&pickersource::Observe(reader,owner,42,true,*source,&sourceReads,&stage);
        if(!initialized)std::printf("Fixture source failure stage%u reads%u\n",stage,sourceReads);
    }
    bool Picker(const std::string& text,bool counselor){using namespace cfgmenu;auto& f=fixture;auto& r=f.r;
        const auto type=counselor?Counselor:Jason;const auto first=counselor?std::uintptr_t{0x2600000}:std::uintptr_t{0x2800000};
        auto graph=std::make_unique<capture::GraphSnapshot>();if(!Parse(text,counselor?"ExecuteUbergraph_Counselor_Menu":"ExecuteUbergraph_Jason_Select_Widget",*graph))return false;
        Function(*graph,type,first,first+0x10000);r.Set(type+cfgreflection::disk::Children,first);
        const std::array<const char*,6> names={"Construct","OnClick_Accept","OnPlayTransitionOut","OnClick_Back","OfflineBots_AcceptJason","GetPreviousMenu"};
        const unsigned total=counselor?4:6;
        for(unsigned i=0;i<total;++i){auto fn=std::make_unique<capture::DiagnosticFunctionEvidence>();
            if(i==3){ // Back was observed class-owned, but its wrapper bytes were
                // not printed. This is a SYNTHETIC ABI refusal fixture, not an
                // assertion of its historical graph entry/control-flow.
                fn->function.label.count=fn->function.label.baseCount=12;std::copy_n(u"OnClick_Back",12,fn->function.label.data.begin());
                fn->flags=0x0C000000;fn->script.count=fn->script.capacity=22;fn->bytes[0]=0x90;fn->bytes[13]=0x5A;
                const int entry=counselor?0x2340:0x3BD0;std::memcpy(fn->bytes.data()+14,&entry,4);fn->bytes[18]=0x97;fn->bytes[19]=0xA5;fn->bytes[20]=0x39;fn->bytes[21]=0xE5;
            }else if(!Parse(text,names[i],*fn))return false;
            const auto at=first+0x10000*(i+1);Function(*fn,type,at,at+0x10000);
            if(i!=5){const auto offset=i==2?19U:1U;const auto name=graph->function.name;
                std::memcpy(fn->bytes.data()+offset,&name.index,4);std::memcpy(fn->bytes.data()+offset+4,&name.index,4);std::memcpy(fn->bytes.data()+offset+8,&name.number,4);}
            r.Set(fn->script.data,fn->bytes);
        }
        const auto index=first+0x10000*(total+1),roster=index+0x10000;
        f.Field(index,type,MenuFixture::IntProperty,"CurrentIndex",roster);f.Property(index,4,counselor?1456:1672);r.Set(index+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040010205ULL});
        f.Field(roster,type,MenuFixture::ArrayProperty,counselor?"CharacterClassList":"JasonClassList",counselor?roster+0x10000:0);f.Property(roster,16,counselor?1440:1656);r.Set(roster+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0000000000010205ULL});
        if(counselor){const auto flag=roster+0x10000,lobby=first+0x300,base=first+0x400;
            f.Field(flag,type,BoolKind,"IsFromLobby",0);f.Property(flag,1,1480);r.Set(flag+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0000001040010205ULL});
            f.Field(lobby,first,MenuFixture::ObjectProperty,"K2Node_DynamicCast_AsSCPlayer_State_Lobby",0);f.Property(lobby,8,1368);r.Set(lobby+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000200ULL});
            f.Field(base,first,MenuFixture::ObjectProperty,"K2Node_DynamicCast_AsSCPlayer_State",0);f.Property(base,8,1400);r.Set(base+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000200ULL});
            for(const auto property:{lobby,base})r.Set(property,MenuFixture::Base+counselorpickerdisk::ObjectPropertyVtable);
            r.Set(lobby+counselorpickerdisk::DeclaredObjectClass,LobbyState);r.Set(base+counselorpickerdisk::DeclaredObjectClass,State);
            std::memcpy(graph->bytes.data()+0x103D,&lobby,8);std::memcpy(graph->bytes.data()+0x1E28,&base,8);std::memcpy(graph->bytes.data()+0x1096,&flag,8);
            std::memcpy(graph->bytes.data()+counselorpicker::CastOperandOffset,&LobbyState,8);for(const auto at:{0x1E31,0x229E,0x23C4})std::memcpy(graph->bytes.data()+at,&State,8);
        }
        r.Set(graph->script.data,graph->bytes);return true;
    }
};
