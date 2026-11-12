#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "NativeJasonRoutePolicy.hpp"
#include <fstream>
#include <regex>
struct JasonScopeHost {
    bool current=true,owned=true,reference=true;
    bool Current()const{return current;}bool Owned(cfgmenu::jasonpicker::Callback)const{return owned;}
    bool ReferenceCurrent(const cfgmenu::jasonpicker::Reference&)const{return reference;}
};
int main(int argc,char** argv){using namespace cfgmenu;using namespace jasonpicker;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const char* fixturePath=argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-11776-28527-Jason.txt";
    std::ifstream input(fixturePath,std::ios::binary);const std::string text((std::istreambuf_iterator<char>(input)),{});
    const auto label=[](cfgnames::Text& out,std::u16string_view value){out={};out.count=out.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),out.data.begin());};
    const auto widen=[](const std::string& s){return std::u16string(s.begin(),s.end());};
    const std::regex functionExpression("Function=0x([A-F0-9]+) Owner=0x([A-F0-9]+) Serial=0 Name=([^ ]+) Flags=([A-F0-9]+) ParameterBytes=([0-9]+) StorageBytes=([0-9]+) ScriptBytes=([0-9]+) NativeExecutor=0x([A-F0-9]+) DestructorLink=0x([A-F0-9]+)[\\s\\S]*?ScriptHexBegin\\r?\\n([A-F0-9\\r\\n]+)\\r?\\nScriptHexEnd");
    const auto parse=[&](std::u16string_view expected,auto& out){
        for(auto iterator=std::sregex_iterator(text.begin(),text.end(),functionExpression);iterator!=std::sregex_iterator();++iterator){const auto& m=*iterator;
            if(widen(m[3].str())!=expected)continue;out={};out.function.object=std::stoull(m[1].str(),nullptr,16);out.function.owner=std::stoull(m[2].str(),nullptr,16);
            label(out.function.label,expected);label(out.function.kind,u"Function");out.flags=static_cast<std::uint32_t>(std::stoul(m[4].str(),nullptr,16));
            out.parameterBytes=static_cast<std::uint16_t>(std::stoul(m[5].str()));out.storageBytes=std::stoi(m[6].str());out.script.count=out.script.capacity=std::stoi(m[7].str());
            out.nativeExecutor=std::stoull(m[8].str(),nullptr,16);out.destructorLink=std::stoull(m[9].str(),nullptr,16);out.script.data=out.function.object+0x100000;
            auto hex=m[10].str();hex.erase(std::remove(hex.begin(),hex.end(),'\r'),hex.end());hex.erase(std::remove(hex.begin(),hex.end(),'\n'),hex.end());
            if(hex.size()!=2*static_cast<unsigned>(out.script.count)||static_cast<unsigned>(out.script.count)>out.bytes.size())return false;
            for(unsigned i=0;i<static_cast<unsigned>(out.script.count);++i)out.bytes[i]=static_cast<unsigned char>(std::stoul(hex.substr(2*i,2),nullptr,16));
            const std::regex parameterExpression(" Parameter=0x([A-F0-9]+) Name=([^ ]+) Type=([^ ]+) Offset=([0-9]+) ElementBytes=([0-9]+) Dimension=([0-9]+) Flags=([A-F0-9]+)");
            const auto block=m.str();for(auto p=std::sregex_iterator(block.begin(),block.end(),parameterExpression);p!=std::sregex_iterator();++p){const auto& row=*p;
                if(out.fieldsCount==out.fields.size())return false;auto& field=out.fields[out.fieldsCount++];field.field.object=std::stoull(row[1].str(),nullptr,16);field.field.owner=out.function.object;
                label(field.field.label,widen(row[2].str()));label(field.field.kind,widen(row[3].str()));field.offset=std::stoi(row[4].str());field.elementBytes=std::stoi(row[5].str());field.dimension=std::stoi(row[6].str());field.flags=std::stoull(row[7].str(),nullptr,16);}
            return true;}return false;};
    auto graph=std::make_unique<capture::GraphSnapshot>();auto previous=std::make_unique<capture::DiagnosticFunctionEvidence>();
    check(parse(u"ExecuteUbergraph_Jason_Select_Widget",*graph)&&parse(u"GetPreviousMenu",*previous),"complete captured graph/previous fixtures");
    if(!graph->function.object||!previous->function.object)return 1;
    const auto module=graph->nativeExecutor-startdisk::ScriptExecutor;const auto type=graph->function.owner;
    check(GraphShape(*graph,type,module),"captured graph23684/storage3728 native edge shape");check(PreviousShape(*previous,type,module),"captured512 previous native stack two-branch schema");
    check(Pointer(*previous,0xD8)==Pointer(*previous,0x1D1),"actual GetPreviousMenuClass same native reference");
    for(unsigned i=0;i<13;++i){auto changed=*graph;
        if(i==0)--changed.script.count;if(i==1)++changed.storageBytes;if(i==2)changed.bytes[AcceptEntry]=0x0A;if(i==3)changed.bytes[OfflineAcceptEntry+1]^=1;
        if(i==4)changed.bytes[ConstructEntry+1]^=1;if(i==5)changed.bytes[0x39A0]^=1;
        if(i>=6&&i<=8)changed.bytes[TransitionOperands[i-6]]^=1;
        if(i>=9&&i<=11)changed.bytes[DestinationConstants[i-9]]=0x76;if(i==12)changed.flags=0;
        check(!GraphShape(changed,type,module),"changed graph/entry/native alias/destination schema refuses");}
    for(unsigned i=0;i<8;++i){auto changed=*previous;
        if(i==0)--changed.script.count;if(i==1)changed.flags^=1;if(i==2)--changed.fieldsCount;if(i==3)changed.fields[0].flags^=0x80;
        if(i==4)changed.fields[4].elementBytes=8;if(i==5)changed.fields[10].offset=72;if(i==6)changed.bytes[0x1D1]^=1;if(i==7)changed.bytes[0xAD]=0x90;
        check(!PreviousShape(changed,type,module),"changed PreviousMenu return/layout/native reference refuses");}
    for(const auto row:{std::pair<std::u16string_view,int>{u"Construct",ConstructEntry},{u"OnClick_Accept",AcceptEntry},{u"OfflineBots_AcceptJason",OfflineAcceptEntry}}){
        auto function=std::make_unique<capture::DiagnosticFunctionEvidence>();check(parse(row.first,*function),"captured accept/construct wrapper");
        graph->function.name.index=0x35161;check(startbinding::Wrapper(*function,module,type,row.first,row.first==u"Construct"?0x08020808:0x0C000000,row.second)&&WrapperName(*function,graph->function.name),"exact graph FName and entry wrapper");}
    auto transition=std::make_unique<capture::DiagnosticFunctionEvidence>();check(parse(u"OnPlayTransitionOut",*transition),"captured native transition event wrapper");
    check(transition->script.count==40&&transition->fieldsCount==1&&transition->fields[0].field.label.View()==u"TransitioningTo"&&
        transition->fields[0].field.kind.View()==u"ClassProperty"&&WrapperName(*transition,graph->function.name,19),"transition class parameter is event evidence not pre-transition interception");
    // Transplant captured bytecode into an OWNED engine model. Historical game
    // pointers are never dereferenced; only documented serialized pointer/FName
    // operands are relocated to this model's owned functions and classes.
    NativeFixture nativeFixture;auto& model=nativeFixture.fixture;auto& memory=model.r;
    model.Name(MenuFixture::Type,"Jason_Select_Widget_C");
    constexpr std::uintptr_t first=0x2000000,boolKind=0x2100000,classKind=0x2110000,nativeOwner=0x2120000,nativeTransition=0x2130000;
    model.Name(boolKind,"BoolProperty");model.Name(classKind,"ClassProperty");nativeFixture.Root(nativeOwner);
    memory.Set(MenuFixture::Type+cfgprobe::ClassSuper,nativeOwner);memory.Set(MenuFixture::Type+cfgreflection::disk::Children,first);
    auto functionStorage=std::make_unique<std::array<capture::DiagnosticFunctionEvidence,5>>();auto& functions=*functionStorage;
    const std::array<std::u16string_view,5> functionNames={u"Construct",u"OnClick_Accept",u"OfflineBots_AcceptJason",u"OnPlayTransitionOut",u"GetPreviousMenu"};
    for(unsigned i=0;i<functions.size();++i)check(parse(functionNames[i],functions[i]),"owned transplant captured function");
    auto graphModel=*graph;const auto transplant=[&](auto& function,unsigned index){
        const auto at=first+0x10000*index;const auto fnName=NativeFixture::Narrow(function.function.label.View());
        model.Field(at,MenuFixture::Type,MenuFixture::FunctionClass,fnName.c_str(),at+0x10000);nativeFixture.Register(at);
        function.function.object=at;function.function.owner=MenuFixture::Type;memory.Read(at+cfgnames::disk::ObjectName,function.function.name);
        function.script.data=0x3000000+0x10000*index;function.script.capacity=function.script.count;function.nativeExecutor=MenuFixture::Base+startdisk::ScriptExecutor;
        function.destructorLink=function.destructorLink?at+0x100:0;
        memory.Set(at+cfgreflection::disk::FunctionFlags,function.flags);memory.Set(at+cfgreflection::arraydisk::FunctionParameterBytes,function.parameterBytes);
        memory.Set(at+cfgreflection::disk::StorageBytes,function.storageBytes);memory.Set(at+0xB0,function.nativeExecutor);memory.Set(at+0x68,function.destructorLink);
        memory.Set(at+startdisk::FastGraph,std::uintptr_t{0});memory.Set(at+startdisk::FastEntry,0);memory.Set(at+scriptdisk::Script,function.script);
        memory.Set(at+cfgreflection::disk::Children,function.fieldsCount?at+0x100:std::uintptr_t{0});
        for(unsigned p=0;p<function.fieldsCount;++p){auto& value=function.fields[p];const auto field=at+0x100+0x100*p;const auto name=NativeFixture::Narrow(value.field.label.View());
            const auto propertyKind=value.field.kind.View()==u"IntProperty"?MenuFixture::IntProperty:value.field.kind.View()==u"BoolProperty"?boolKind:value.field.kind.View()==u"ClassProperty"?classKind:MenuFixture::ObjectProperty;
            model.Field(field,at,propertyKind,name.c_str(),p+1<function.fieldsCount?field+0x100:0);model.Property(field,value.elementBytes,value.offset);memory.Set(field+cfgreflection::disk::PropertyFlags,value.flags);
            value.field.object=field;value.field.owner=at;}
        return at;};
    transplant(graphModel,0);for(unsigned i=0;i<functions.size();++i)transplant(functions[i],i+1);
    for(unsigned i=0;i<4;++i){const unsigned at=i==3?19:1;const auto words=graphModel.function.name;
        std::memcpy(functions[i].bytes.data()+at,&words.index,4);std::memcpy(functions[i].bytes.data()+at+4,&words.index,4);std::memcpy(functions[i].bytes.data()+at+8,&words.number,4);}
    std::array<std::uintptr_t,3> modelClasses={0x2140000,0x2150000,0x2160000};
    const std::array<const char*,3> modelClassNames={"PickOfflineBotsMapMenuWidget_C","OfflineBotsSettingsMenuWidget_C","MainMenuWidget_C"};
    // Fixture names are deliberately supplied metadata, not asserted names of
    // the historical captured pointees. Unknown/changed names remain Other.
    for(unsigned i=0;i<3;++i){model.Name(modelClasses[i],modelClassNames[i]);memory.Set(modelClasses[i]+cfgprobe::ClassSuper,nativeOwner);nativeFixture.Register(modelClasses[i]);
        std::memcpy(graphModel.bytes.data()+DestinationConstants[i]+1,&modelClasses[i],8);std::memcpy(graphModel.bytes.data()+TransitionOperands[i],&nativeTransition,8);}
    std::memcpy(graphModel.bytes.data()+0x39A0,&nativeTransition,8);
    model.Field(nativeTransition,nativeOwner,MenuFixture::FunctionClass,"PlayTransitionOut",0);nativeFixture.Register(nativeTransition);
    memory.Set(MenuFixture::Base+transitiondisk::FunctionCache,nativeTransition);memory.Set(MenuFixture::Base+transitiondisk::Owner,nativeOwner);
    memory.Set(MenuFixture::Base+transitiondisk::Constructor,nativeOwner);memory.Set(nativeTransition+0xB0,MenuFixture::Base+transitiondisk::Executor);
    memory.Set(MenuFixture::Base+transitiondisk::Executor,transitiondisk::Code);memory.Set(nativeTransition+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});
    memory.Set(nativeTransition+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});memory.Set(nativeTransition+cfgreflection::disk::StorageBytes,8);
    const std::array<const char*,4> previousNames={"GetOwningPlayer","GetHUD","GetMenuStackComp","GetPreviousMenuClass"};
    constexpr std::array<std::array<unsigned,2>,4> previousOffsets={std::array<unsigned,2>{0x0B,0x104},{0x35,0x12E},{0xAE,0x1A7},{0xD8,0x1D1}};
    for(unsigned i=0;i<previousNames.size();++i){const auto at=std::uintptr_t{0x2200000}+i*0x10000;
        model.Field(at,nativeOwner,MenuFixture::FunctionClass,previousNames[i],0);nativeFixture.Register(at);
        for(const auto operand:previousOffsets[i])std::memcpy(functions[4].bytes.data()+operand,&at,8);}
    memory.Set(graphModel.script.data,graphModel.bytes);for(const auto& function:functions)memory.Set(function.script.data,function.bytes);
    const auto indexField=first+0x60000,rosterField=first+0x70000;
    model.Field(indexField,MenuFixture::Type,MenuFixture::IntProperty,"CurrentIndex",rosterField);model.Property(indexField,4,1672);memory.Set(indexField+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040010205ULL});
    model.Field(rosterField,MenuFixture::Type,MenuFixture::ArrayProperty,"JasonClassList",0);model.Property(rosterField,16,1656);memory.Set(rosterField+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0000000000010205ULL});
    OptionsReader modelReader{memory};active::Context modelContext;check(active::Observe(modelReader,MenuFixture::Base,modelContext),"owned positive active Jason context");
    Binding positive;unsigned preparationReads=0;
    check(Observe(modelReader,modelContext,42,true,positive,&preparationReads),"positive full native binding from owned relocated capture");
    check(preparationReads<ReadLimit&&positive.context.widget==MenuFixture::Widget,"bounded preparation publishes only current owned context");
    std::array<Reference,3> named;unsigned referenceReads=0;
    check(LateReferences(modelReader,positive,42,true,named,&referenceReads),"positive bounded native named late destinations");
    check(referenceReads<ReadLimit&&named[0].destination==Destination::OfflineMap&&named[1].destination==Destination::Settings&&named[2].destination==Destination::Other,
        "actual supplied metadata classifies without guessed counselor source");
    auto coldGraph=std::make_unique<capture::GraphSnapshot>();cfgreflection::Observation<OptionsReader> modelReflection(modelReader,MenuFixture::Base);
    cfgreflection::Field graphField;capture::FunctionDiagnostic diagnostic;unsigned graphReferenceReads=0;
    check(modelReflection.ReadField(first,MenuFixture::Type,graphField)&&capture::Graph(modelReader,MenuFixture::Base,graphField,*coldGraph,diagnostic),"owned saved graph-only diagnostic snapshot");
    check(LateGraphReferences(modelReader,modelContext,*coldGraph,42,true,named,&graphReferenceReads)&&named[1].destination==Destination::Settings,
        "graph-only named observer reuses exact captured snapshot");
    auto graphMutation=*coldGraph;graphMutation.bytes[DestinationConstants[1]+1]^=1;
    check(!LateGraphReferences(modelReader,modelContext,graphMutation,42,true,named)&&!named[0].type,"saved/source graph mismatch publishes no guessed destinations");
    model.Name(modelClasses[0],"UnrelatedWidget_C");check(LateReferences(modelReader,positive,42,true,named)&&named[0].destination==Destination::Other,"unknown actual destination stays Other, never an alias");
    auto damaged=graphModel.bytes;damaged[DestinationConstants[1]+1]^=1;memory.Set(graphModel.script.data,damaged);
    check(!LateReferences(modelReader,positive,42,true,named)&&!named[0].type,"changed serialized source clears all resource proof");memory.Set(graphModel.script.data,graphModel.bytes);
    memory.Set(functions[1].script.data,std::uint8_t{0x0A});check(!Current(modelReader,positive,42),"changed accept wrapper refuses current binding");memory.Set(functions[1].script.data,functions[1].bytes);
    memory.Set(nativeTransition+cfgreflection::disk::FunctionFlags,std::uint32_t{0});check(!LateReferences(modelReader,positive,42,true,named)&&!named[0].type,"changed native transition ABI refuses all destinations");
    memory.Set(nativeTransition+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});
    memory.Set(MenuFixture::Base+transitiondisk::Executor,std::uint8_t{0});check(!LateReferences(modelReader,positive,42,true,named)&&!named[0].type,"changed native executor disk code refuses all destinations");
    memory.Set(MenuFixture::Base+transitiondisk::Executor,transitiondisk::Code);
    memory.Set(modelClasses[1]+cfgprobe::ClassSuper,std::uintptr_t{0});check(!LateReferences(modelReader,positive,42,true,named)&&!named[0].type,"named non-menu class cannot become a destination");
    memory.Set(modelClasses[1]+cfgprobe::ClassSuper,nativeOwner);
    model.Name(0x2230000,"UnrelatedFunction");check(!Current(modelReader,positive,42),"PreviousMenu native function names never inferred from alias pointers");
    model.Name(0x2230000,"GetPreviousMenuClass");
    check(!LateReferences(modelReader,positive,43,true,named)&&!named[0].type,"foreign thread never gains reference authority");
    memory.Set(MenuFixture::Base+transitiondisk::FunctionCache,std::uintptr_t{0});check(!LateReferences(modelReader,positive,42,true,named)&&!named[0].type,"missing native transition cache never initialized or called");
    std::printf("Jason owned preparation reads=%u late-reference reads=%u graph-only late reads=%u cap=%u\n",preparationReads,referenceReads,graphReferenceReads,ReadLimit);
    Binding binding;binding.context.widget=0x100000;binding.context.thread=42;binding.graph.field.object=0x110000;binding.graph.script.data=0x120000;
    const std::array<startbinding::Slot*,5> slots={&binding.construct,&binding.accept,&binding.offlineAccept,&binding.transition,&binding.previous};
    constexpr std::array<Callback,5> kinds={Callback::Construct,Callback::Accept,Callback::OfflineAccept,Callback::Transition,Callback::Previous};
    constexpr std::uintptr_t frame=0x150000,locals=0x160000;MenuFixture fixture;OptionsReader reader{fixture.r};Callback kind=Callback::Other;
    fixture.r.Set(frame+startdisk::FrameObject,binding.context.widget);
    for(unsigned i=0;i<slots.size();++i){auto& slot=*slots[i];slot.field.object=0x170000+0x1000*i;slot.script.data=0x180000+0x1000*i;
        fixture.r.Set(frame+startdisk::FrameNode,slot.field.object);fixture.r.Set(frame+startdisk::FrameCurrentNative,slot.field.object);fixture.r.Set(frame+startdisk::FrameCode,slot.script.data);
        check(Frame(reader,binding,binding.context.widget,frame,42,kind)&&kind==kinds[i],"borrowed exact native callback frame");
        check(!Frame(reader,binding,binding.context.widget,frame,43,kind)&&!Frame(reader,binding,binding.context.widget+16,frame,42,kind),"foreign receiver/thread refuses");}
    fixture.r.Set(frame+startdisk::FrameNode,binding.graph.field.object);fixture.r.Set(frame+startdisk::FrameCurrentNative,binding.graph.field.object);
    fixture.r.Set(frame+startdisk::FrameCode,binding.graph.script.data);fixture.r.Set(frame+startdisk::FrameLocals,locals);
    for(const auto entry:{ConstructEntry,AcceptEntry,OfflineAcceptEntry,TransitionEntry,0x123}){fixture.r.Set(locals,entry);check(Frame(reader,binding,binding.context.widget,frame,42,kind)==(entry!=0x123),"only actual captured graph entry borrowed");}
    Reference destination;destination.type=0x200000;destination.destination=Destination::Settings;label(destination.name,u"OfflineBotsSettingsMenuWidget_C");
    for(unsigned fault=0;fault<7;++fault){JasonScopeHost host;NativeAcceptScope scope;unsigned forwards=0;bool during=false;
        if(fault==1)host.current=false;if(fault==2)host.owned=false;if(fault==3)host.reference=false;
        auto selected=destination;if(fault==4)label(selected.name,u"OfflineBotsSettingsMenuWidget_C_0");if(fault==5)selected.destination=Destination::OfflineMap;
        check(scope.Run(host,fault==6?Callback::Construct:Callback::OfflineAccept,[&]()noexcept{++forwards;during=scope.SettingsObserved(host,selected);return true;}),"original native forwarding preserved");
        check(forwards==1&&!scope.Accepting()&&!scope.SettingsObserved(host,destination),"one original forward/no accept lifetime leak");
        check(during==(fault==0),"only synchronous owned native accept settings observation");}
    NativeAcceptScope scope;JasonScopeHost host;unsigned forwards=0;
    check(scope.Run(host,Callback::Accept,[&]()noexcept{++forwards;return scope.Run(host,Callback::OfflineAccept,[&]()noexcept{++forwards;return true;});}),"nested native accept forwarded");
    check(scope.Accepts()==1&&forwards==2&&!scope.Accepting(),"nested accept one transaction/no selection overwrite");
    check(Classify(u"Counselor_Menu_C")==Destination::Counselor&&Classify(u"PickOfflineBotsMapMenuWidget_C")==Destination::OfflineMap&&Classify(u"SandboxMapWidget_C")==Destination::Other,"exact typed destination names/no aliases");
    Binding refused;unsigned reads=0;std::array<Reference,3> references;
    check(!Observe(reader,binding.context,42,false,refused,&reads)&&!refused.context.widget,"unverified image cannot create binding");
    check(!LateReferences(reader,binding,42,false,references,&reads)&&!references[0].type,"unverified image no named late resource proof");
    std::printf("CFG Jason captured graph/previous/borrowed frame/native accept policy: %s (%u checks, %u failures). Read-only owned fixtures; no game access or gameplay approval.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
