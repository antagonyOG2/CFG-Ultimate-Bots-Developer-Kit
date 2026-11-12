#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "NativeOfflineEntryRuntime.hpp"
struct EntryFixture:HudFixture {
    static constexpr std::uintptr_t MenuNative=0x3100000,TransitionNative=0x3110000,Transition=0x3120000,
        Pending=0x3130000,ClassKind=0x3140000,Map=0x3150000,Settings=0x3160000,
        EntryGraph=0x3170000,MapGraph=0x3180000,EntryCode=0x3190000,MapCode=0x31A0000,
        Sandbox=0x31B0000,Jason=0x31C0000,ButtonType=0x31D0000,SandboxField=0x31E0000,JasonField=0x31F0000,
        TitleMember=0x3200000,SandboxClick=0x3210000,JasonClick=0x3220000,SandboxCode=0x3230000,JasonCode=0x3240000,
        SandboxMap=0x3250000,PendingMode=0x3260000,PendingMap=0x3270000,ResetLocal=0x3280000;
    EntryFixture(){using namespace cfgmenu;using namespace cfgprobe;using M=MenuFixture;auto& f=fixture;auto& r=f.r;
        r.Set(Records,M::Type);
        f.Name(M::Type,"OfflinePlayMenuWidget_C");f.Name(Map,"PickOfflineBotsMapMenuWidget_C");f.Name(Settings,"OfflineBotsSettingsMenuWidget_C");
        f.Name(SandboxMap,"PickSandboxMapMenuWidget_C");f.Name(ClassKind,"ClassProperty");
        Root(TransitionNative);r.Set(MenuNative+ClassSuper,TransitionNative);r.Set(M::Type+ClassSuper,MenuNative);
        for(const auto object:{Map,Settings,SandboxMap}){r.Set(object+ClassSuper,MenuNative);Register(object);}
        r.Set(M::Base+pushdisk::MenuClass,MenuNative);r.Set(M::Base+pushdisk::MenuConstructor,MenuNative);
        r.Set(M::Base+transitiondisk::Owner,TransitionNative);r.Set(M::Base+transitiondisk::Constructor,TransitionNative);
        r.Set(M::Base+transitiondisk::FunctionCache,Transition);r.Set(M::Base+transitiondisk::Executor,transitiondisk::Code);
        f.Field(Transition,TransitionNative,M::FunctionClass,"PlayTransitionOut",0);Register(Transition);
        r.Set(TransitionNative+cfgreflection::disk::Children,Transition);
        r.Set(Transition+0xB0,M::Base+transitiondisk::Executor);r.Set(Transition+cfgreflection::disk::FunctionFlags,std::uint32_t{0x04020409});
        r.Set(Transition+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});r.Set(Transition+cfgreflection::disk::StorageBytes,8);
        f.Field(Pending,Frontend,ClassKind,"PendingOfflineSettingsWidget",0);f.Property(Pending,8,0x528);
        r.Set(Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x001C001040000204});r.Set(Frontend+cfgreflection::disk::Children,Pending);
        f.Field(PendingMode,Frontend,ClassKind,"PendingOfflineMode",Pending);f.Property(PendingMode,8,0x520);
        f.Field(PendingMap,Frontend,ClassKind,"PendingOfflineMap",PendingMode);f.Property(PendingMap,8,0x518);
        for(const auto property:{PendingMode,PendingMap})r.Set(property+cfgreflection::disk::PropertyFlags,std::uint64_t{0x001C001040000204});
        ObjectMember(ResetLocal,MapGraph,"K2Node_DynamicCast_AsSCGame_Menu2",0,184);
        r.Set(ResetLocal+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000200});
        const auto setFunction=[&](std::uintptr_t object,std::uintptr_t owner,const char* name,std::uintptr_t next,unsigned flags,int parameters,int storage,std::uintptr_t code,int bytes,std::uintptr_t cleanup){
            f.Field(object,owner,M::FunctionClass,name,next);Register(object);
            r.Set(object+cfgreflection::disk::Children,std::uintptr_t{0});r.Set(object+cfgreflection::disk::FunctionFlags,flags);
            r.Set(object+cfgreflection::disk::StorageBytes,storage);r.Set(object+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(parameters));
            r.Set(object+0xB0,M::Base+startdisk::ScriptExecutor);r.Set(object+0x68,cleanup);r.Set(object+scriptdisk::Script,capture::ScriptHeader{code,bytes,bytes});
            r.Set(object+startdisk::FastGraph,std::uintptr_t{0});r.Set(object+startdisk::FastEntry,std::int32_t{0});};
        setFunction(EntryGraph,M::Type,"ExecuteUbergraph_OfflinePlayMenuWidget",SandboxField,0x00800000,4,113,EntryCode,1944,0);
        setFunction(MapGraph,Map,"ExecuteUbergraph_PickOfflineBotsMapMenuWidget",0,0,4,193,MapCode,1867,Pending);
        r.Set(M::Type+cfgreflection::disk::Children,EntryGraph);r.Set(Map+cfgreflection::disk::Children,MapGraph);
        std::array<unsigned char,1944> graph{};const auto pointer=[&](auto& bytes,unsigned at,std::uintptr_t value){std::memcpy(bytes.data()+at,&value,8);};
        graph[0x3E8]=0x97;graph[0x3E9]=0x87;graph[0x3EA]=0xF5;pointer(graph,0x3EB,Transition);
        graph[0x3F3]=0x89;pointer(graph,0x3F4,Map);graph[0x3FC]=0x97;graph[0x3FD]=0x87;
        graph[0xDB]=0xF5;pointer(graph,0xDC,Transition);graph[0xE4]=0x89;pointer(graph,0xE5,SandboxMap);graph[0xED]=0x97;graph[0xEE]=0x87;r.Set(EntryCode,graph);
        std::array<unsigned char,1867> mapGraph{};mapGraph[0x108]=0x0D;mapGraph[0x109]=0xC6;mapGraph[0x10A]=0x76;mapGraph[0x113]=9;
        pointer(mapGraph,0x117,Pending);mapGraph[0x11F]=0xDB;pointer(mapGraph,0x120,Pending);mapGraph[0x128]=0x89;pointer(mapGraph,0x129,Settings);mapGraph[0x131]=0x87;
        constexpr std::array<unsigned,3> resetStarts={0x5D2,0x5F3,0x614};
        constexpr std::array<std::uintptr_t,3> resetMembers={PendingMode,PendingMap,Pending};
        for(unsigned i=0;i<resetStarts.size();++i){const auto at=resetStarts[i];
            mapGraph[at]=0x0D;mapGraph[at+1]=0xC6;mapGraph[at+2]=0x76;pointer(mapGraph,at+3,ResetLocal);
            mapGraph[at+11]=9;pointer(mapGraph,at+15,resetMembers[i]);mapGraph[at+23]=0xDB;
            pointer(mapGraph,at+24,resetMembers[i]);mapGraph[at+32]=0x7B;}
        r.Set(MapCode,mapGraph);
        const auto member=[&](std::uintptr_t at,std::uintptr_t owner,const char* name,std::uintptr_t next,int offset){ObjectMember(at,owner,name,next,offset);
            r.Set(at+cfgreflection::disk::PropertyFlags,std::uint64_t{0x000A0010C008020C});};
        member(SandboxField,M::Type,"SandboxButton",JasonField,0x420);member(JasonField,M::Type,"OfflineBotsButton",SandboxClick,0x418);
        member(TitleMember,ButtonType,"TitleText",0,0x400);r.Set(ButtonType+cfgreflection::disk::Children,TitleMember);f.Name(ButtonType,"WideMenuTextButton_C");
        r.Set(M::Widget+0x420,Sandbox);r.Set(M::Widget+0x418,Jason);r.Set(Sandbox+ObjectClass,ButtonType);r.Set(Jason+ObjectClass,ButtonType);
        r.Set(Sandbox+0x400,Title);r.Set(Jason+0x400,Value);r.Set(Title+ObjectClass,TextType);r.Set(Value+ObjectClass,TextType);
        Register(Sandbox,501);Register(Jason,502);
        setFunction(SandboxClick,M::Type,NativeFixture::Narrow(entry::SandboxClick).c_str(),JasonClick,0x08000000,0,0,SandboxCode,22,0);
        setFunction(JasonClick,M::Type,NativeFixture::Narrow(entry::JasonClick).c_str(),0,0x08000000,0,0,JasonCode,22,0);
        cfgnames::Words graphName{};r.Read(EntryGraph+24,graphName);
        for(const auto pair:std::array<std::pair<std::uintptr_t,int>,2>{{{SandboxCode,0xDB},{JasonCode,0x3EA}}}){
            std::array<unsigned char,22> click{};click[0]=0x90;std::memcpy(click.data()+1,&graphName.index,4);std::memcpy(click.data()+5,&graphName.index,4);
            click[13]=0x5A;std::memcpy(click.data()+14,&pair.second,4);click[18]=0x97;click[19]=0xA5;click[20]=0x39;click[21]=0xE5;r.Set(pair.first,click);}
    }
};
struct StartupBudgetReader {
    OptionsReader reader;unsigned limit=256,reads=81;bool budgetHit=false;
    bool ReadBytes(std::uintptr_t address,void* out,std::size_t size){
        if(reads>=limit){budgetHit=true;return false;}
        ++reads;return reader.ReadBytes(address,out,size);
    }
    template<class T> bool Read(std::uintptr_t address,T& out){
        T value{};if(!ReadBytes(address,&value,sizeof(value)))return false;
        out=value;return true;
    }
};
struct PendingSchemaChangingReader {
    OptionsReader reader;unsigned flagsReads=0;
    template<class T> bool Read(std::uintptr_t address,T& out){
        const bool okay=reader.Read(address,out);
        if(okay&&address==EntryFixture::Pending+cfgreflection::disk::PropertyFlags&&++flagsReads==1)
            reader.r.Set(address,std::uint64_t{0x001C001040000205});
        return okay;
    }
    bool ReadBytes(std::uintptr_t address,void* out,std::size_t size){return reader.ReadBytes(address,out,size);}
};
struct LabelFailureEngine:ModelEngine {
    unsigned fault=0;
    explicit LabelFailureEngine(NativeFixture& f):ModelEngine{f}{}
    bool Invoke(std::uintptr_t target,std::uintptr_t function,void* packet){
        if(target==NativeFixture::Value&&function==model.textFunction&&fault==17)return false;
        const bool okay=ModelEngine::Invoke(target,function,packet);
        if(target==NativeFixture::Value&&function==model.textFunction&&fault==18)
            model.fixture.r.Set(MenuFixture::Widget+0x418,std::uintptr_t{0});
        return okay;
    }
};
#ifndef CFGMENU_ENTRY_FIXTURE_ONLY
int main(){using namespace cfgmenu;using F=EntryFixture;using M=MenuFixture;unsigned failures=0;
    const auto check=[&](bool value,const char* name){if(!value){++failures;std::printf("FAIL %s\n",name);}};
    {auto f=std::make_unique<F>();OptionsReader preparation{f->fixture.r};active::Context context;entry::Binding binding;
        auto source=std::make_unique<entrysettings::Resources>();
        check(active::Observe(preparation,M::Base,context)&&entrysettings::Observe(preparation,context,42,true,*source)&&entry::Observe(preparation,*source,binding),"paired-label source fixture");
        StartupBudgetReader reader{{f->fixture.r},32768,0,false};ModelEngine engine{*f};reflection::RetainedMenuText text;
        entry::Labels<StartupBudgetReader,ModelEngine> labels(reader,engine,text,binding);unsigned peak=0;
        for(unsigned i=0;i<3;++i){reader.reads=0;const auto calls=engine.calls;
            check(labels.Service(i*25,2)&&labels.Stage()==2*(i+1),"two validated label stages share one real callback");
            peak=(std::max)(peak,reader.reads);check(!reader.budgetHit&&reader.reads<=32768&&engine.calls-calls<=2,"paired label shares read cap and two-call bound");}
        check(labels.Ready()&&engine.texts==2,"both donor labels ready in three callbacks instead of six");
        const auto calls=engine.calls,reads=reader.reads;
        check(!labels.Service(75,2)&&engine.calls==calls&&reader.reads==reads,"ready label performs no idle work");
        std::printf("Paired menu labels maxReads=%u/32768 callbacks=3 nativeCallsPerCallback<=2\n",peak);
    }
    {struct Budget {unsigned cap=32768,reads=16385;} budget;
        check(!setupslice::MayContinue(budget),"insufficient shared reserve defers coalescing");
        budget.reads=16384;check(setupslice::MayContinue(budget),"exact shared reserve permits bounded next phase");
        auto fixture=std::make_unique<F>();OptionsReader unknown{fixture->fixture.r};
        check(!setupslice::MayContinue(unknown),"unknown reader budget retains one phase");
    }
    for(const unsigned failure:{12u,13u,17u,18u}){
        auto f=std::make_unique<F>();OptionsReader reader{f->fixture.r};active::Context context;
        auto source=std::make_unique<entrysettings::Resources>();entry::Binding binding;
        check(active::Observe(reader,M::Base,context)&&entrysettings::Observe(reader,context,42,true,*source)&&
            entry::Observe(reader,*source,binding),"label failure fixture source");
        LabelFailureEngine engine(*f);auto text=std::make_unique<reflection::RetainedMenuText>();
        entry::Labels<OptionsReader,LabelFailureEngine> labels(reader,engine,*text,binding);
        for(unsigned i=0;i<5;++i)check(labels.Service(25*i),"prepare final Jason label");
        engine.fault=failure;
        if(failure==12)f->fixture.r.Set(M::Base+execdisk::SetText.cache,std::uintptr_t{0});
        if(failure==13)f->fixture.r.Set(F::Value+cfgprobe::ObjectClass,F::ButtonType);
        check(!labels.Service(125)&&labels.Refused()&&labels.Stage()==5&&labels.Failure()==failure,
            "final-label failure reports exact guard without replay");
        const auto calls=engine.calls;check(!labels.Service(150)&&engine.calls==calls,"refused label never replays native operation");
    }
    {auto f=std::make_unique<F>();auto& memory=f->fixture.r;OptionsReader reader{memory};active::Context sourceContext;
        check(active::Observe(reader,M::Base,sourceContext),"native arrival source context");
        auto original=std::make_unique<entrysettings::Resources>();
        check(entrysettings::Observe(reader,sourceContext,42,true,*original),"native arrival original authored chain");
        constexpr std::uintptr_t mapWidget=0x3600000,settingsWidget=0x3610000;
        f->Register(mapWidget,9600);memory.Set(mapWidget+cfgprobe::ObjectClass,F::Map);
        f->Register(settingsWidget,9601);memory.Set(settingsWidget+cfgprobe::ObjectClass,F::Settings);
        memory.Set(F::Stack+execdisk::StackArray,active::StackArray{F::Records,2,2});
        memory.Set(F::Records+execdisk::RecordBytes,F::Map);memory.Set(F::Records+execdisk::RecordBytes+execdisk::RecordWidget,mapWidget);
        memory.Set(F::Registry+24*original->mapIdentity.index+16,9700);active::Context mapContext;
        check(active::Observe(reader,M::Base,mapContext),"actual positive new map menu context");
        unsigned stage=0;check(!entrysettings::Retained(reader,mapContext,*original,&stage)&&stage==8,"native map serial initialization reproduces strict old arrival refusal");
        auto staged=std::make_unique<entrysettings::Resources>(*original);
        check(entrysettings::FirstNativeArrival(reader,mapContext,*staged,&stage)&&stage==0&&staged->mapIdentity.serial==9700&&original->mapIdentity.serial==0,
            "first actual map arrival revalidates unchanged authored source and captures native-assigned ID in copy");
        memory.Set(F::Records+execdisk::RecordBytes,F::Settings);memory.Set(F::Records+execdisk::RecordBytes+execdisk::RecordWidget,settingsWidget);
        memory.Set(F::Registry+24*original->settingsIdentity.index+16,9701);active::Context settingsContext;
        check(active::Observe(reader,M::Base,settingsContext)&&entrysettings::FirstNativeArrival(reader,settingsContext,*staged,&stage)&&
            staged->settingsIdentity.serial==9701&&entrysettings::Retained(reader,settingsContext,*staged),"native settings arrival captures its first assigned ID without losing map lease");
        memory.Set(F::Registry+24*staged->mapIdentity.index+16,9702);
        check(!entrysettings::FirstNativeArrival(reader,settingsContext,*staged),"assigned map identity replacement cannot be recaptured");
        memory.Set(F::Registry+24*staged->mapIdentity.index+16,9700);
        const auto next=staged->pendingMode.field.next;
        memory.Set(staged->pendingMode.field.object+cfgreflection::disk::Next,std::uintptr_t{0});
        check(entrysettings::FirstNativeArrival(reader,settingsContext,*staged,&stage),"same authored mode schema survives unrelated native sibling-link change across callbacks");
        memory.Set(staged->pendingMode.field.object+cfgreflection::disk::PropertyOffset,staged->pendingMode.offset+8);
        check(!entrysettings::FirstNativeArrival(reader,settingsContext,*staged,&stage)&&stage==7&&staged->reset.stage==4&&staged->reset.property.failure==3,
            "changed mode offset still refuses with exact property diagnostic");
        memory.Set(staged->pendingMode.field.object+cfgreflection::disk::PropertyOffset,staged->pendingMode.offset);
        memory.Set(staged->pendingMode.field.object+cfgreflection::disk::Next,next);
        const auto settingsNext=staged->pending.field.next;
        memory.Set(staged->pending.field.object+cfgreflection::disk::Next,F::PendingMode);
        check(entrysettings::Retained(reader,settingsContext,*staged,&stage)&&stage==0,
            "direct authored settings descriptor survives unrelated sibling-link change across callbacks");
        memory.Set(staged->pending.field.object+cfgreflection::disk::PropertyFlags,staged->pending.flags^1ULL);
        check(!entrysettings::Retained(reader,settingsContext,*staged,&stage)&&stage==6,
            "settings descriptor schema mutation still refuses at exact retained leaf");
        memory.Set(staged->pending.field.object+cfgreflection::disk::PropertyFlags,staged->pending.flags);
        memory.Set(staged->pending.field.object+cfgreflection::disk::Next,settingsNext);
        memory.Set(F::Records,F::Map);
        check(!entrysettings::FirstNativeArrival(reader,settingsContext,*staged,&stage)&&stage==3,"lost source class record still refuses initialized metadata");
    }
    {auto f=std::make_unique<F>();OptionsReader contextReader{f->fixture.r};active::Context context;
        check(active::Observe(contextReader,M::Base,context),"startup budget fixture context");
        // CapturePlan consumes 81 reads before source verification.
        // Exercise the composed outer cap, not just the component's local cap.
        for(const unsigned limit:{256u,32768u}){
            StartupBudgetReader reader{{f->fixture.r},limit,81,false};
            auto source=std::make_unique<entrysettings::Resources>();unsigned reads=0,stage=0;
            const bool okay=entrysettings::Observe(reader,context,42,true,*source,&reads,&stage);
            if(limit==256)check(!okay&&reader.reads==256&&reader.budgetHit&&reads==176&&!source->settings,
                "legacy aggregate cap reproduces exact SourceReads176 refusal without publication");
            else check(okay&&!reader.budgetHit&&reader.reads==81+reads&&reads<16384&&source->settings==F::Settings,
                "explicit frontend aggregate cap completes source while preserving component limit");
            std::printf("ComposedStartupBudget limit=%u total=%u source=%u hit=%u result=%s\n",
                limit,reader.reads,reads,reader.budgetHit?1u:0u,okay?"accepted":"refused");
        }
    }
    {auto f=std::make_unique<F>();PendingSchemaChangingReader reader{{f->fixture.r}};active::Context context;
        check(active::Observe(reader,M::Base,context),"changing descriptor context");
        cfgreflection::Property property;pendingmember::PropertyDiagnostic detail;
        check(!pendingmember::PropertyAt(reader,context,F::Pending,F::Frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",8,0x528,property,&detail)&&
            detail.failure==4&&!property.field.object,"direct descriptor refuses flags changing during its double observation");}
    for(unsigned fault=0;fault<16;++fault){auto f=std::make_unique<F>();auto& memory=f->fixture.r;
        OptionsReader reader{memory};active::Context context;check(active::Observe(reader,M::Base,context),"reset triple fixture context");
        memory.Set(F::Frontend+cfgreflection::disk::Children,std::uintptr_t{0});
        if(fault==1)memory.Set(F::MapCode+0x5D2,std::uint8_t{0});
        if(fault==2)memory.Set(F::MapCode+0x60A,std::uint8_t{0});
        if(fault==3)memory.Set(F::MapCode+0x634,std::uint8_t{0});
        if(fault==4)memory.Set(F::MapCode+0x602,F::PendingMode);
        if(fault==5){memory.Set(F::MapCode+0x602,F::PendingMode);memory.Set(F::MapCode+0x60B,F::PendingMode);}
        if(fault==6)memory.Set(F::MapCode+0x5F6,F::ResetLocal+0x800);
        if(fault==7){memory.Set(F::MapCode+0x623,F::PendingMode);memory.Set(F::MapCode+0x62C,F::PendingMode);}
        if(fault==8)f->fixture.Name(F::PendingMode,"OtherMode");
        if(fault==9)memory.Set(F::PendingMap+cfgreflection::disk::PropertyOffset,0x520);
        if(fault==10)memory.Set(F::PendingMode+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0014000000000004});
        if(fault==11)memory.Set(F::PendingMode+0x20,F::MenuNative);
        if(fault==12)memory.Set(F::ResetLocal+0x20,F::EntryGraph);
        if(fault==13)memory.Set(F::ResetLocal+cfgreflection::disk::PropertyOffset,16);
        if(fault==14)memory.Set(F::PendingMap+cfgprobe::ObjectClass,M::ObjectProperty);
        if(fault==15)memory.Set(F::MapCode+0x5DE,std::uint8_t{1});
        auto source=std::make_unique<entrysettings::Resources>();unsigned stage=0;
        const bool okay=entrysettings::Observe(reader,context,42,true,*source,nullptr,&stage);
        check(okay==(fault==0),"exact reset triple and typed descriptors are required before publication");
        if(fault)check(stage==25&&!source->pendingMode.field.object,"malformed reset reports stage25 without partial mode authority");
        else {check(source->pendingMode.field.object==F::PendingMode&&source->pendingMap.field.object==F::PendingMap,"authored mode/map identities extracted exactly");
            memory.Set(F::PendingMode+cfgreflection::disk::PropertyFlags,source->pendingMode.flags^1ULL);
            check(!entrysettings::Retained(reader,context,*source),"retained mode descriptor is revalidated with the source");
            memory.Set(F::PendingMode+cfgreflection::disk::PropertyFlags,source->pendingMode.flags);
            memory.Set(F::PendingMap+cfgreflection::disk::PropertyOffset,0x520);
            check(!entrysettings::Retained(reader,context,*source),"retained map descriptor schema is revalidated");
            memory.Set(F::PendingMap+cfgreflection::disk::PropertyOffset,0x518);
            memory.Set(F::ResetLocal+0x20,F::EntryGraph);
            check(!entrysettings::Retained(reader,context,*source),"retained reset local must still belong to the map graph");
            memory.Set(F::ResetLocal+0x20,F::MapGraph);
            memory.Set(F::MapCode+0x5EA,F::PendingMap);
            check(!entrysettings::Retained(reader,context,*source),"retained reset graph operand changes revoke source");}
    }
    for(unsigned fault=0;fault<12;++fault){auto f=std::make_unique<F>();auto& memory=f->fixture.r;
        OptionsReader reader{memory};active::Context context;
        check(active::Observe(reader,M::Base,context),"authored descriptor regression context");
        // Native class child discovery can be unavailable while the loaded
        // authored graph retains its exact native member operand.
        memory.Set(F::Frontend+cfgreflection::disk::Children,std::uintptr_t{0});
        if(fault==1)memory.Set(F::Frontend+cfgreflection::disk::Children,std::uintptr_t{1});
        if(fault==2)memory.Set(F::Pending+cfgreflection::disk::Next,F::Pending+0x800); // Unrelated, unreadable next field.
        if(fault==3)f->fixture.Name(F::Pending,"PendingOfflineMode");
        if(fault==4)memory.Set(F::Pending+0x20,F::MenuNative);
        if(fault==5)memory.Set(F::Pending+cfgprobe::ObjectClass,M::FunctionClass);
        if(fault==6)memory.Set(F::Pending+cfgreflection::disk::PropertyDimensions,2);
        if(fault==7)memory.Set(F::Pending+cfgreflection::disk::PropertyElementSize,16);
        if(fault==8)memory.Set(F::Pending+cfgreflection::disk::PropertyOffset,0x520);
        if(fault==9)memory.Set(F::Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0014000000000004});
        if(fault==10)memory.Set(F::MapCode+0x120,F::Pending+0x800);
        if(fault==11)memory.Set(F::MapCode+0x117,std::uintptr_t{0});
        cfgreflection::Property searched;
        check(!pendingmember::Property(reader,context,F::Frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",8,0x528,searched),
            "generic property discovery still refuses unavailable native child list");
        auto source=std::make_unique<entrysettings::Resources>();unsigned stage=0,reads=0;
        const bool okay=entrysettings::Observe(reader,context,42,true,*source,&reads,&stage);
        check(okay==(fault<=2),"authored native member bypasses only unrelated list traversal, never its exact schema");
        if(okay){check(source->pending.field.object==F::Pending&&source->pending.field.owner==F::Frontend&&
                source->pending.flags==0x001C001040000204ULL,"direct pending binding preserves native identity and augmented flags");
            check(entrysettings::Retained(reader,context,*source),"authored source retains unchanged graph with unavailable native list");
            memory.Set(F::Pending+cfgreflection::disk::PropertyFlags,source->pending.flags^1ULL);
            check(!entrysettings::Retained(reader,context,*source),"retained authored descriptor flags are revalidated directly");}
        else check(!source->settings&&!source->pending.field.object,"rejected authored member publishes no source");
    }
    for(unsigned fault=0;fault<14;++fault){auto f=std::make_unique<F>();OptionsReader reader{f->fixture.r};active::Context context;
        check(active::Observe(reader,M::Base,context),"actual owned first entry context");
        if(fault==1)f->fixture.r.Set(F::Transition+0xB0,M::Base+pushdisk::Executor); // Prior incorrect PushMenu assumption must refuse.
        if(fault==2)f->fixture.r.Set(F::EntryCode+0x3EB,f->textFunction);
        if(fault==3)f->fixture.Name(F::Map,"SP_LobbyWidget_C");
        if(fault==4)f->fixture.r.Set(F::MapCode+0x129,F::SandboxMap);
        if(fault==9)f->fixture.r.Set(M::Base+transitiondisk::FunctionCache,std::uintptr_t{0});
        if(fault==10)f->fixture.r.Set(M::Base+transitiondisk::FunctionCache,F::Transition+0x100);
        if(fault==11)f->fixture.Name(F::Transition,"PlayTransitionOutWrong");
        if(fault==12)f->fixture.r.Set(F::Pending+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0014000000000004});
        if(fault==13)f->fixture.r.Set(F::Pending+cfgreflection::disk::PropertyOffset,0x520);
        auto resource=std::make_unique<entrysettings::Resources>();unsigned reads=0,stage=0;
        nativetransition::Diagnostic detail;
        pendingmember::PropertyDiagnostic propertyDetail;
        const bool source=entrysettings::Observe(reader,context,42,true,*resource,&reads,&stage,&detail,&propertyDetail);
        check(source==(fault==0||(fault>=5&&fault<=9)),"exact authored source and transition identity");
        if(fault==12)check(stage==22,"unaugmented disk flags are refused at specific flags stage");
        if(fault==13)check(stage==21,"wrong native pending property has specific descriptor stage");
        if(fault==12)check(propertyDetail.failure==0&&propertyDetail.flags==0x0014000000000004,"flags diagnostic reports observed value, not approved output");
        if(fault==13)check(propertyDetail.failure==3&&propertyDetail.offset==0x520,"schema diagnostic distinguishes wrong property offset");
        if(fault==10||fault==11)check(stage==24&&detail.failure,"native function refusal names exact resource substage");
        if(!source)continue;
        if(fault==9){
            check(detail.cached==0&&resource->push==F::Transition,"cold cache uses exact native reflected function, not initialization");
            std::uintptr_t current=0;
            check(nativetransition::Observe(reader,M::Base,F::TransitionNative,current,resource->push)&&current==resource->push,"cold retained function remains directly verified");
            f->fixture.r.Set(M::Base+transitiondisk::FunctionCache,F::Transition+0x100);
            check(!nativetransition::Observe(reader,M::Base,F::TransitionNative,current,resource->push)&&!current,"later mismatched nonzero cache revokes cold binding");
            f->fixture.r.Set(M::Base+transitiondisk::FunctionCache,std::uintptr_t{0});
        }
        if(!fault)std::printf("Owned first-entry source reads=%u (limit16384)\n",reads);
        if(fault==5)f->fixture.r.Set(F::SandboxField+cfgreflection::disk::PropertyOffset,0x418);
        if(fault==6)f->fixture.r.Set(F::SandboxCode+14,std::int32_t{0x3EA});
        if(fault==7)f->fixture.r.Set(F::EntryCode+0xE5,F::Map);
        if(fault==8)f->fixture.r.Set(F::TitleMember+cfgreflection::disk::PropertyOffset,0x408);
        entry::Binding binding;const auto before=f->fixture.r.reads;
        const bool observed=entry::Observe(reader,*resource,binding,&stage);
        check(observed==(fault==0||fault==9),"captured native button/label/click route only");if(!observed)continue;
        std::printf("Owned entry binding reads=%u; paced after separate source preparation\n",f->fixture.r.reads-before);
        const auto currentBefore=f->fixture.r.reads;check(entry::Current(reader,binding,context.thread),"existing entry retained binding");
        std::printf("Owned entry retained-current reads=%u\n",f->fixture.r.reads-currentBefore);
        for(const auto* property:{&binding.sandboxMember,&binding.jasonMember,&binding.titleMember}){
            f->fixture.r.Set(property->field.object+cfgreflection::disk::PropertyFlags,property->flags^1);
            check(!entry::Current(reader,binding,context.thread),"changed retained property flags refused without rediscovery");
            f->fixture.r.Set(property->field.object+cfgreflection::disk::PropertyFlags,property->flags);
            f->fixture.r.Set(property->field.object+cfgreflection::disk::PropertyOffset,property->offset+8);
            check(!entry::Current(reader,binding,context.thread),"changed retained property offset refused without rediscovery");
            f->fixture.r.Set(property->field.object+cfgreflection::disk::PropertyOffset,property->offset);}
        ModelEngine engine{*f};auto text=std::make_unique<reflection::RetainedMenuText>();entry::Labels<OptionsReader,ModelEngine> labels(reader,engine,*text,binding);
        for(unsigned i=0;i<6;++i){const auto calls=engine.calls;check(labels.Service(25*i),"one native text phase");
            check(engine.calls-calls<=1,"at most one native call per entry phase");check(!labels.Service(25*i),"same callback does no catch-up");}
        check(labels.Ready()&&engine.texts==2&&engine.creates==0&&engine.attaches==0,"both existing native labels; no new widgets");
        const auto calls=engine.calls,total=f->fixture.r.reads;
        for(unsigned i=0;i<1000;++i)check(!labels.Service(1000+25*i),"ready entry does no ongoing work");
        check(calls==engine.calls&&total==f->fixture.r.reads,"ready entry zero calls/reads");
        hud::Binding transport;
        check(hud::Observe(reader,context,true,transport),"production entry owns exact native HUD transport");
        ModelEngine viewEngine{*f};auto viewText=std::make_unique<reflection::RetainedMenuText>();
        entry::View<OptionsReader,ModelEngine> view(reader,viewEngine,*viewText,binding,transport);
        const auto unrelatedReads=f->fixture.r.reads;
        for(unsigned i=0;i<100;++i){
            check(!view.After(M::Widget,transport.event,false,true,i*25),"production entry ignores non-HUD receiver");
            check(!view.After(transport.context.hud,transport.event+16,false,true,i*25),"production entry ignores unrelated event");
            check(!view.After(transport.context.hud,transport.event,true,true,i*25),"production entry ignores nested callback");
        }
        check(f->fixture.r.reads==unrelatedReads&&viewEngine.calls==0,"unrelated production callbacks do zero work");
        for(unsigned i=0;i<3;++i)check(view.After(transport.context.hud,transport.event,false,true,3000+25*i),"production HUD recurrence sample");
        check(viewEngine.calls==0&&view.Stage()==0,"three returned HUD samples precede entry changes");
        for(unsigned i=0;i<6;++i){const auto phaseCalls=viewEngine.calls;
            check(view.After(transport.context.hud,transport.event,false,true,3075+25*i),"production native label phase");
            check(viewEngine.calls-phaseCalls<=1,"production entry performs at most one native call per callback");
        }
        check(view.Ready()&&!view.Refused()&&viewEngine.texts==2,"production HUD-gated existing entry labels ready");
        const auto finishedReads=f->fixture.r.reads,finishedCalls=viewEngine.calls;
        for(unsigned i=0;i<100;++i)view.After(transport.context.hud,transport.event,false,true,4000+25*i);
        check(f->fixture.r.reads==finishedReads&&viewEngine.calls==finishedCalls,"production ready entry performs zero ongoing reads/calls");
        check(entrysettings::Retained(reader,context,*resource),"zero-serial classes retained by real native stack source");
        // Actual native Push clears old Widget references while retaining CLASS.
        auto& memory=f->fixture.r;f->Register(F::NewWidget,9500);memory.Set(F::NewWidget+cfgprobe::ObjectClass,F::Map);
        memory.Set(F::Stack+execdisk::StackArray,active::StackArray{F::Records,2,2});
        memory.Set(F::Records+execdisk::RecordWidget,std::uintptr_t{0});
        memory.Set(F::Records+execdisk::RecordBytes,F::Map);memory.Set(F::Records+execdisk::RecordBytes+execdisk::RecordWidget,F::NewWidget);
        memory.Set(M::Widget+cfgprobe::lifetime::ObjectIndexOffset,-1); // Former widget may already be gone.
        active::Context pushed;check(active::Observe(reader,M::Base,pushed),"actual pushed map context after old widget retirement");
        check(entrysettings::Retained(reader,pushed,*resource),"native retained class survives null/retired prior widget, without old-widget dereference");
        memory.Set(F::Records,F::Map);
        check(!entrysettings::Retained(reader,pushed,*resource),"removed/replaced native source CLASS revokes metadata retention");
    }
    std::printf("CFG first Offline Play native entry/source/label integration: %s (%u failures). Owned memory only; full gameplay approval remains pending.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif
