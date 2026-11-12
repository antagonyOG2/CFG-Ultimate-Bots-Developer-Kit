#define CFG_H23_LOBBY_PROVIDER_MAIN H23LobbyProviderBaselineMain
#include "H23LobbyProviderTests.cpp"
#undef CFG_H23_LOBBY_PROVIDER_MAIN
#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include <cstring>

namespace {
template<std::size_t N,std::size_t C> std::uintptr_t BindPanelFunction(LobbyFixture& f,
    const cfgmenu::execdisk::Binding& binding,
    const std::array<cfgmenu::execdisk::Parameter,N>& parameters,
    const std::array<unsigned char,C>& code,std::uintptr_t owner,std::uintptr_t function,unsigned index,bool serialZero=false){
    using namespace cfgprobe;using namespace cfgmenu;
    auto& r=f.r;
    r.Set(f.Module+binding.reflection,owner);r.Set(f.Module+binding.constructor,owner);
    r.Set(f.Module+binding.cache,function);
    r.Set(f.Module+binding.executor,code);
    std::uintptr_t functionClass=0x2F00000;
    r.Set(functionClass+ClassSuper,std::uintptr_t{});f.Slot(functionClass,32,132);
    f.Label(functionClass,"Function");
    r.Set(function+ObjectClass,functionClass);r.Set(function+0x20,owner);r.Set(function+8,std::uint32_t{});r.Set(function+lifetime::ObjectIndexOffset,static_cast<int>(33+index));
    r.Set(function+cfgreflection::disk::Next,std::uintptr_t{});
    r.Set(f.Registry+24ULL*(33+index),function);r.Set(f.Registry+24ULL*(33+index)+16,serialZero?0:static_cast<int>(200+index));
    r.Set(f.Registry+24ULL*(33+index)+8,serialZero?std::uint32_t{0x42000000}:std::uint32_t{});
    if(serialZero)r.Set(function+8,std::uint32_t{0x41});
    r.Set(f.Module+lifetime::ObjectArrayRva+28,static_cast<int>(34+index));
    r.Set(function+cfgreflection::disk::StorageBytes,static_cast<int>(binding.frame));
    r.Set(function+cfgreflection::disk::FunctionFlags,binding.flags);
    r.Set(function+cfgreflection::arraydisk::FunctionParameterBytes,
        static_cast<std::uint16_t>(native::ParameterExtent(binding,parameters)));
    r.Set(function+0xB0,f.Module+binding.executor);r.Set(function+0x68,std::uintptr_t{});
    r.Set(function+cfgmenu::scriptdisk::Script,capture::ScriptHeader{});
    std::string functionName;for(auto ch:binding.name)functionName.push_back(static_cast<char>(ch));f.Label(function,functionName.c_str());
    std::uintptr_t head=0;
    for(std::size_t i=0;i<N;++i){const auto& p=parameters[i];const auto field=function+0x100+0x100*i;
        const auto kind= function+0x3000+0x100*i;
        const auto kindName=native::Kind(p.kind);
        std::string kn,pn;for(auto ch:kindName)kn.push_back(static_cast<char>(ch));for(auto ch:p.name)pn.push_back(static_cast<char>(ch));
        f.Label(kind,kn.c_str());f.Label(field,pn.c_str());
        r.Set(field+ObjectClass,kind);r.Set(field+0x20,function);r.Set(field+cfgreflection::disk::Next,i+1<N?field+0x100:std::uintptr_t{});
        r.Set(field+cfgreflection::disk::PropertyDimensions,static_cast<int>(p.dimension));
        r.Set(field+cfgreflection::disk::PropertyElementSize,static_cast<int>(cfgmenu::native::ElementBytes(p.kind)));
        r.Set(field+cfgreflection::disk::PropertyOffset,static_cast<int>(p.offset));
        r.Set(field+cfgreflection::disk::PropertyFlags,cfgmenu::native::RuntimeFlags(binding,p));if(!head)head=field;
    }
    r.Set(function+cfgreflection::disk::Children,head);
    r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
    return function;
}
void PacketBytes(ChallengeReader& memory,const void* packet,std::size_t size){
    const auto at=reinterpret_cast<std::uintptr_t>(packet);const auto* bytes=static_cast<const unsigned char*>(packet);
    for(std::size_t i=0;i<size;++i)memory.Set(at+i,bytes[i]);
}
struct H215HostedRowModel {
    NativeFixture& fixture;ChallengeReader& memory;unsigned calls=0,creates=0,attaches=0,options=0,texts=0,removes=0;
    bool lastOkay=false,lastOutputOkay=false;
    std::map<std::uintptr_t,std::u16string> convertedLabels;
    std::u16string displayedTitle,displayedValue;
    std::uintptr_t module=MenuFixture::Base;
    unsigned Thread()const{return GetCurrentThreadId();}
    void Register(std::uintptr_t object,int serial,std::uint32_t objectFlags=0,std::uint32_t slotFlags=0){
        int count=0;memory.Read(module+cfgprobe::lifetime::ObjectArrayRva+28,count);
        const auto index=static_cast<unsigned>(count);memory.Set(object+cfgprobe::lifetime::ObjectIndexOffset,static_cast<int>(index));memory.Set(object+8,objectFlags);
        memory.Set(fixture.Registry+24ULL*index,object);memory.Set(fixture.Registry+24ULL*index+8,slotFlags);
        memory.Set(fixture.Registry+24ULL*index+16,serial);memory.Set(module+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(index+1));
    }
    bool Invoke(std::uintptr_t target,std::uintptr_t function,void* packet){
        using Scope=cfgmenu::rows::HostedLobbyRetentionScope;using namespace cfgmenu;using namespace cfgmenu::execdisk;
        ++calls;PacketBytes(memory,packet,function==fixture.createFunction?sizeof(reflection::CreateParameters):
            function==fixture.addFunction?sizeof(reflection::AddParameters):function==MenuFixture::Options?20:
            function==fixture.convertFunction?sizeof(reflection::TextConversion):function==fixture.textFunction?sizeof(reflection::OpaqueText):16);
        if(!Scope::Inputs(module,target,function,packet)||!Scope::EnterNative(module)||windowHeld)return false;
        auto& r=memory;bool okay=false;
        if(function==fixture.createFunction){++creates;auto& p=*static_cast<reflection::CreateParameters*>(packet);
            p.result=NativeFixture::NewWidget;r.Set(p.result+cfgprobe::ObjectClass,MenuFixture::DonorType);
            r.Set(p.result+0x180,std::uintptr_t{});r.Set(p.result+0x188,NativeFixture::Title);r.Set(p.result+0x190,NativeFixture::Value);
            r.Set(NativeFixture::Title+cfgprobe::ObjectClass,NativeFixture::TextType);
            r.Set(NativeFixture::Value+cfgprobe::ObjectClass,NativeFixture::TextType);
            Register(NativeFixture::Title,502);Register(NativeFixture::Value,503);
            Register(p.result,501);okay=true;
        }else if(function==fixture.addFunction){++attaches;auto& p=*static_cast<reflection::AddParameters*>(packet);
            if(target==NativeFixture::Panel&&p.content==NativeFixture::NewWidget){
                r.Set(NativeFixture::NewWidget+0x180,NativeFixture::NewSlot);r.Set(NativeFixture::NewSlot+cfgprobe::ObjectClass,NativeFixture::SlotType);
                r.Set(NativeFixture::NewSlot+0x180,NativeFixture::Panel);r.Set(NativeFixture::NewSlot+0x188,NativeFixture::NewWidget);
                r.Set(NativeFixture::NewSlot+cfgpractice::disk::Content,NativeFixture::NewWidget);
                for(const auto offset:{std::uintptr_t{0x200},static_cast<std::uintptr_t>(cfgpractice::disk::Slots)}){
                    cfgmenu::panelcapture::Array slots{};if(!r.Read(NativeFixture::Panel+offset,slots))return false;
                    r.Set(slots.data+8ULL*static_cast<unsigned>(slots.count),NativeFixture::NewSlot);++slots.count;r.Set(NativeFixture::Panel+offset,slots);}
                p.result=NativeFixture::NewSlot;Register(p.result,0,0x48,0x42000000);okay=true;
            }
        }else if(function==fixture.removeFunction){++removes;
            std::uintptr_t child=0;std::memcpy(&child,packet,8);cfgmenu::panelcapture::Array slots{};
            if(target==NativeFixture::Panel&&child==NativeFixture::NewWidget){
                for(const auto offset:{std::uintptr_t{0x200},static_cast<std::uintptr_t>(cfgpractice::disk::Slots)}){
                    if(!r.Read(NativeFixture::Panel+offset,slots))return false;unsigned remain=0;
                    for(int i=0;i<slots.count;++i){std::uintptr_t slot=0;if(!r.Read(slots.data+8ULL*i,slot))return false;
                        if(slot!=NativeFixture::NewSlot)r.Set(slots.data+8ULL*remain++,slot);}
                    slots.count=static_cast<int>(remain);r.Set(NativeFixture::Panel+offset,slots);}
                r.Set(child+0x180,std::uintptr_t{});*reinterpret_cast<std::uint8_t*>(static_cast<unsigned char*>(packet)+8)=1;okay=true;
            }
        }else if(function==MenuFixture::Options){
            ++options;
            reflection::Array input{};std::int32_t selected=-1;std::memcpy(&input,packet,sizeof(input));
            std::memcpy(&selected,static_cast<unsigned char*>(packet)+16,sizeof(selected));
            if(target==NativeFixture::NewWidget&&input.data&&input.count>0&&input.count<=32){
                struct Header{std::uintptr_t data;std::int32_t count,capacity;};const std::uintptr_t data=0x1400000;
                r.Set(target+0x200,Header{data,input.count,input.count});
                r.Set(target+0x210,selected);
                for(int i=0;i<input.count;++i){const auto& source=input.data[i];const auto dest=0x1410000+0x100ULL*static_cast<unsigned>(i);
                    std::array<char16_t,128> chars{};for(int j=0;j<source.count;++j)chars[static_cast<unsigned>(j)]=source.data[j];
                    r.Set(dest,chars);const reflection::String copy{reinterpret_cast<const char16_t*>(dest),source.count,source.count};r.Set(data+16ULL*static_cast<unsigned>(i),copy);}
                okay=true;}
        }else if(function==fixture.convertFunction){
            auto& conversion=*static_cast<reflection::TextConversion*>(packet);
            if(conversion.input.data&&conversion.input.count>0&&conversion.input.count<=128){
                const auto token=0x1500000+0x100ULL*convertedLabels.size();
                convertedLabels[token]=std::u16string(conversion.input.data,static_cast<std::size_t>(conversion.input.count-1));
                const std::uintptr_t result[2]{token,0x1510000};
                std::memcpy(conversion.result.bytes.data(),result,sizeof(result));okay=true;}
        }else if(function==fixture.textFunction){
            ++texts;std::uintptr_t token=0;std::memcpy(&token,packet,sizeof(token));
            const auto label=convertedLabels.find(token);
            okay=label!=convertedLabels.end()&&(target==NativeFixture::Title||target==NativeFixture::Value);
            if(okay)(target==NativeFixture::Title?displayedTitle:displayedValue)=label->second;
        }
        PacketBytes(memory,packet,function==fixture.createFunction?sizeof(reflection::CreateParameters):
            function==fixture.addFunction?sizeof(reflection::AddParameters):function==fixture.convertFunction?sizeof(reflection::TextConversion):16);
        lastOkay=okay;lastOutputOkay=Scope::Outputs(module,function,packet);
        return okay&&lastOutputOkay;
    }
};
struct H215HostedGraphProof {cfghostedlobby::Discovery* discovery=nullptr;cfghostedlobby::RowLayout* layout=nullptr;};
bool H215HostedCurrent(Reader& reader,void* opaque){auto& p=*static_cast<H215HostedGraphProof*>(opaque);
    return p.discovery&&p.layout&&cfghostedlobby::LayoutCurrent(reader,*p.discovery,*p.layout);}
bool H215HostedRowAdapter(){
    using namespace cfgprobe;using namespace cfghostedlobby;using namespace cfgmenu;
    unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* text){++checks;if(!ok){++failures;std::printf("FAIL adapter %s\n",text);}};
    NativeFixture fixture;using F=NativeFixture;using M=MenuFixture;
    constexpr std::uintptr_t GameState=0x700000,PlayerState=0x710000,NetDriver=0x720000,
        GameStateType=0x730000,PlayerStateType=0x740000,NetDriverType=0x750000,ModeType=0x760000,ControllerType=0x770000,
        HudType=0x780000,MenuType=0x790000,StackType=0x7A0000,UserType=0x7B0000,Tree=0x1050000,TreeType=0x7C0000,WidgetType=0x7D0000;
    fixture.fixture.Name(F::Panel,"CustomServerSettings");fixture.fixture.Name(F::PanelType,"VerticalBox");
    fixture.fixture.Name(M::DonorType,"LobbyOptionComboBoxWidget_C");fixture.fixture.Name(TreeType,"WidgetTree");
    fixture.fixture.Name(M::Widget,"PrivateLobby_C");
    constexpr std::uintptr_t SlotParentField=0x1080000,SlotContentField=0x1080100;
    fixture.fixture.r.Set(F::SlotType+cfgreflection::disk::Children,SlotParentField);
    fixture.fixture.Field(SlotParentField,F::SlotType,MenuFixture::ObjectProperty,"Parent",SlotContentField);
    fixture.fixture.Property(SlotParentField,8,0x180);
    fixture.fixture.Field(SlotContentField,F::SlotType,MenuFixture::ObjectProperty,"Content",0);
    fixture.fixture.Property(SlotContentField,8,cfgpractice::disk::Content);
    // Aggregate backing serves many independently capped provider windows.
    ChallengeReader memory;memory.bytes=fixture.fixture.r.bytes;memory.cap=4000000;active=&memory;
    const auto pair=[&](std::uintptr_t rva,std::uintptr_t type){memory.Set(M::Base+rva,std::array<std::uintptr_t,2>{{type,type}});memory.Set(type+ClassSuper,std::uintptr_t{});};
    pair(cfghostedlobby::disk::ControllerReflection,ControllerType);pair(cfghostedlobby::disk::ModeReflection,ModeType);
    pair(cfghostedlobby::disk::HudReflection,HudType);pair(cfghostedlobby::disk::MenuReflection,MenuType);
    pair(cfghostedlobby::disk::StateReflection,GameStateType);pair(cfghostedlobby::disk::PlayerStateReflection,PlayerStateType);
    pair(cfghosted::disk35::NetDriverReflection,NetDriverType);pair(cfgpractice::disk::UserWidgetReflection,UserType);
    // This runtime RVA aliases the native RemoveChild owner-class cache in the fixture.
    // PanelType derives from RemoveOwner, so keep the exact native binding owner here.
    pair(cfgpractice::disk::PanelWidgetReflection,F::RemoveOwner);pair(cfgpractice::disk::PanelSlotReflection,F::SlotType);
    pair(cfgpractice::disk::WidgetTreeReflection,TreeType);pair(cfgpractice::disk::WidgetReflection,WidgetType);
    memory.Set(WidgetType+ClassSuper,std::uintptr_t{});
    memory.Set(ControllerType+ClassSuper,WidgetType);memory.Set(HudType+ClassSuper,WidgetType);memory.Set(MenuType+ClassSuper,UserType);
    memory.Set(UserType+ClassSuper,WidgetType);memory.Set(F::Mode+ObjectClass,ModeType);memory.Set(F::Controller+ObjectClass,ControllerType);memory.Set(F::HUD+ObjectClass,HudType);
    memory.Set(M::Widget+ObjectClass,MenuType);memory.Set(GameState+ObjectClass,GameStateType);memory.Set(PlayerState+ObjectClass,PlayerStateType);
    memory.Set(NetDriver+ObjectClass,NetDriverType);memory.Set(F::Stack+ObjectClass,StackType);memory.Set(Tree+ObjectClass,TreeType);
    memory.Set(F::World+WorldGameMode,Fixture::Mode);memory.Set(F::World+WorldGameState,GameState);
    memory.Set(F::World+WorldNetDriver,NetDriver);memory.Set(NetDriver+cfghosted::disk35::DriverWorld,F::World);
    memory.Set(NetDriver+cfghosted::disk35::ServerConnection,std::uintptr_t{});
    memory.Set(GameState+0x20,F::World);memory.Set(PlayerState+cfghostedlobby::disk::HostByte,std::uint8_t{1});
    memory.Set(PlayerState+0x88,F::Controller);memory.Set(F::Controller+cfgescape::disk::PlayerState,PlayerState);
    memory.Set(F::Controller+cfghosted::disk35::ControllerPlayer,F::Player);memory.Set(F::Player+PlayerController,F::Controller);
    memory.Set(F::Controller+0x20,F::World);
    memory.Set(Fixture::Mode+ObjectClass,ModeType);memory.Set(Fixture::Mode+cfghosted::disk35::ActorRole,std::uint8_t{3});memory.Set(F::Controller+cfghosted::disk35::ActorRole,std::uint8_t{3});
    memory.Set(F::HUD+0x368,F::Controller);memory.Set(F::Stack+0x20,F::HUD);
    memory.Set(M::Widget+cfgpractice::disk::WidgetTree,Tree);memory.Set(Tree+cfgpractice::disk::RootWidget,F::Panel);
    memory.Set(F::Panel+ObjectClass,F::PanelType);memory.Set(F::Panel+8,std::uint32_t{0xA80048});
    memory.Set(M::Donor+ObjectClass,M::DonorType);memory.Set(F::DonorSlot+0x180,F::Panel);memory.Set(F::DonorSlot+0x188,M::Donor);
    memory.Set(F::DonorSlot+cfgpractice::disk::Content,M::Donor);
    // Strong host/current identities and the authored native two-node layout share the NativeFixture registry.
    int objectCount=0;memory.Read(M::Base+cfgprobe::lifetime::ObjectArrayRva+28,objectCount);
    const auto reg=[&](std::uintptr_t obj,int serial){const auto index=static_cast<unsigned>(objectCount++);
        memory.Set(obj+cfgprobe::lifetime::ObjectIndexOffset,static_cast<int>(index));memory.Set(obj+8,std::uint32_t{});memory.Set(F::Registry+24ULL*index,obj);
        memory.Set(F::Registry+24ULL*index+8,std::uint32_t{});memory.Set(F::Registry+24ULL*index+16,serial);};
    for(auto [obj,serial]:std::array<std::pair<std::uintptr_t,int>,20>{{{Fixture::Mode,301},{F::Stack,302},{F::Panel,0},{M::Donor,303},
        {F::DonorSlot,315},{M::DonorType,304},{F::PanelType,305},{Tree,306},{GameState,307},{PlayerState,308},{NetDriver,309},{ModeType,310},
        {ControllerType,311},{HudType,312},{MenuType,313},{UserType,314},{F::CreateOwner,0},{F::CreateOwner+0x800,0},{StackType,316},{TreeType,317}}})reg(obj,serial);
    for(unsigned i=0;i<static_cast<unsigned>(objectCount);++i){std::uintptr_t obj=0;memory.Read(F::Registry+24ULL*i,obj);
        memory.Set(F::Registry+24ULL*i+8,std::uint32_t{});memory.Set(obj+8,std::uint32_t{});}
    memory.Set(F::Panel+8,std::uint32_t{0xA80048});
    memory.Set(M::Base+cfgcompat::cfg35::menuStack::OwnerRva,StackType);memory.Set(M::Base+cfgcompat::cfg35::menuStack::ConstructorRva,StackType);
    memory.Set(F::RemoveOwner+ClassSuper,UserType);memory.Set(F::PanelType+ClassSuper,F::RemoveOwner);memory.Set(M::DonorType+ClassSuper,UserType);
    std::uint32_t cdoFlags=0x31,synthetic=0x40000000;int cdoIndex=-1;memory.Read(F::CreateOwner+0x800+cfgprobe::lifetime::ObjectIndexOffset,cdoIndex);
    memory.Set(F::CreateOwner+0x800+8,cdoFlags);memory.Set(F::Registry+24ULL*static_cast<unsigned>(cdoIndex)+8,synthetic);
    reg(F::ConvertOwner,0);reg(F::ConvertOwner+0x800,0);reg(F::TextType,317);
    memory.Set(F::ConvertOwner+0x800+8,cdoFlags);
    memory.Set(M::Base+cfgprobe::lifetime::ObjectArrayRva+28,objectCount);
    memory.Set(M::Options+8,std::uint32_t{0x280001}); // Exact live SetOptions serial-zero flags.
    // Preserve the function UObjects' exact generated metadata while making instance calls positive-slot eligible.
    int addIndex=-1;memory.Read(fixture.addFunction+cfgprobe::lifetime::ObjectIndexOffset,addIndex);memory.Set(F::Registry+24ULL*static_cast<unsigned>(addIndex)+16,401);
    memory.Set(fixture.addFunction+8,std::uint32_t{});int functionCount=objectCount;
    memory.Set(fixture.removeFunction+cfgprobe::lifetime::ObjectIndexOffset,functionCount);memory.Set(F::Registry+24ULL*static_cast<unsigned>(functionCount),fixture.removeFunction);
    memory.Set(F::Registry+24ULL*static_cast<unsigned>(functionCount)+8,std::uint32_t{});memory.Set(F::Registry+24ULL*static_cast<unsigned>(functionCount)+16,402);
    memory.Set(M::Base+cfgprobe::lifetime::ObjectArrayRva+28,static_cast<int>(objectCount));
    ++functionCount;memory.Set(M::Base+cfgprobe::lifetime::ObjectArrayRva+28,functionCount);
    memory.Set(M::Base+cfgprobe::lifetime::ObjectArrayRva+16,F::Registry);
    memory.Set(F::Panel+cfgpractice::disk::Slots,cfgpracticeobs::WidgetNode::List{F::PanelSlots,1,64});
    memory.Set(F::PanelSlots,F::DonorSlot);memory.Set(F::SlotType+ClassSuper,std::uintptr_t{});
    memory.Set(F::PanelType+ClassSuper,F::RemoveOwner);memory.Set(M::DonorType+ClassSuper,UserType);
    memory.Set(F::PanelType+cfgreflection::disk::Children,F::SlotsField); // exact reflected Slots array from NativeFixture
    const auto thread=GetCurrentThreadId();memory.Set(M::Base+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{1});memory.Set(M::Base+cfgprobe::dispatch::ThreadIdRva,thread);
    memory.Set(M::Base+cfgprimary::SerializationFlag,std::uint8_t{});memory.Set(M::Base+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
    memory.Set(M::Base+cfghostedlobby::disk::GetCurrentMenu,cfghostedlobby::disk::GetCurrentMenuCode);
    memory.Set(M::Base+cfghostedlobby::disk::TextToString,cfghostedlobby::disk::TextToStringCode);
    Reader reader;reader.cap=131072;Context context;Failure why;cfghostedlobby::ControllerTrace trace{};
    const bool observed=Observe(reader,M::Base,thread,context,why,&trace);
    if(!observed||!context.listenHost)std::printf("hosted adapter Observe failure=%u thread=%u world=%p mode=%p state=%p controller=%p hud=%p stack=%p menu=%p host=%u\n",
        static_cast<unsigned>(why),context.thread,reinterpret_cast<void*>(context.world),reinterpret_cast<void*>(context.mode),
        reinterpret_cast<void*>(context.gameState),reinterpret_cast<void*>(context.controller),reinterpret_cast<void*>(context.hud),
        reinterpret_cast<void*>(context.stack),reinterpret_cast<void*>(context.menu),context.hostFlag);
    if(!observed){std::uintptr_t back=0,controllerClass=0,objectClass=0;memory.Read(F::Controller+cfghosted::disk35::ControllerPlayer,back);
        memory.Read(M::Base+disk::ControllerReflection,controllerClass);memory.Read(F::Controller+ObjectClass,objectClass);
        std::printf("hosted controller trace stage=%u player=%p controller=%p class=%p backlink=%p direct=%p classCache=%p objectClass=%p\n",trace.stage,
        reinterpret_cast<void*>(trace.player),reinterpret_cast<void*>(trace.controller),reinterpret_cast<void*>(trace.type),reinterpret_cast<void*>(trace.backlink),
        reinterpret_cast<void*>(back),reinterpret_cast<void*>(controllerClass),reinterpret_cast<void*>(objectClass));}
    if(!observed) {std::uintptr_t mode=0,instance=0,type=0,cache=0,ctor=0;memory.Read(F::World+WorldGameMode,mode);memory.Read(F::World+WorldGameInstance,instance);
        memory.Read(mode+ObjectClass,type);memory.Read(M::Base+disk::ModeReflection,cache);memory.Read(M::Base+disk::ModeReflection+sizeof(std::uintptr_t),ctor);
        int index=-1,serial=-1;memory.Read(mode+cfgprobe::lifetime::ObjectIndexOffset,index);memory.Read(F::Registry+24ULL*static_cast<unsigned>(index)+16,serial);
        cfgprobe::lifetime::ArrayHeader header{};cfgprobe::lifetime::SlotObservation slot{};cfghostedlobbystatic::Failure slotWhy{};
        cfgprobe::lifetime::ReadArrayHeader(reader,M::Base+cfgprobe::lifetime::ObjectArrayRva,header);
        const bool hasSlot=cfghostedlobbystatic::Slot(reader,header,mode,false,slot,slotWhy);
        std::printf("hosted adapter world values mode=%p instance=%p modeType=%p cache=%p ctor=%p count=%d index=%d serial=%d slot=%d slotWhy=%u\n",reinterpret_cast<void*>(mode),
            reinterpret_cast<void*>(instance),reinterpret_cast<void*>(type),reinterpret_cast<void*>(cache),reinterpret_cast<void*>(ctor),objectCount,index,serial,hasSlot,slotWhy.reason);}
    check(observed&&context.listenHost,"real Observe confirms hosted NativeFixture context");
    Discovery discovery;discovery.context=context;discovery.graph.count=2;discovery.graph.hudWidget=context.identities[4];discovery.graph.world=context.world;
    int panelIndexForGraph=-1,donorIndexForGraph=-1;memory.Read(F::Panel+cfgprobe::lifetime::ObjectIndexOffset,panelIndexForGraph);
    memory.Read(M::Donor+cfgprobe::lifetime::ObjectIndexOffset,donorIndexForGraph);
    auto& root=discovery.graph.nodes[0];root.widget={F::Panel,panelIndexForGraph,0};root.parent=UINT32_MAX;root.tree=Tree;
    auto& anchor=discovery.graph.nodes[1];anchor.widget={M::Donor,donorIndexForGraph,303};anchor.parent=0;anchor.kind=2;anchor.index=0;
    anchor.slot=F::DonorSlot;anchor.list={F::PanelSlots,1,64};
    RowLayout layout{context.world,context.menu,M::DonorType,F::Panel,M::Donor,1,1};
    const bool layoutNow=LayoutCurrent(reader,discovery,layout);
    if(!layoutNow){Context fresh;Failure freshWhy;const bool freshOk=Observe(reader,M::Base,thread,fresh,freshWhy);
        cfgprobe::lifetime::SlotObservation rootSlot{},anchorSlot{},slotIdentity{};const bool rootClear=cfgpracticeobs::Clear(reader,context.registry,F::Panel,rootSlot);
        const bool anchorClear=cfgpracticeobs::Clear(reader,context.registry,M::Donor,anchorSlot);std::uintptr_t anchorType=0,rootTree=0,treeRoot=0,outer=0;
        memory.Read(M::Donor+ObjectClass,anchorType);memory.Read(M::Widget+cfgpractice::disk::WidgetTree,rootTree);memory.Read(Tree+cfgpractice::disk::RootWidget,treeRoot);
        memory.Read(F::Controller+0x20,outer);cfgpracticeobs::WidgetNode::List list{},listAgain{};std::uintptr_t slotValue=0,content=0;
        memory.Read(F::Panel+cfgpractice::disk::Slots,list);memory.Read(F::PanelSlots,slotValue);
        const bool panelTyped=cfgpracticeobs::Typed(reader,context.registry,M::Base,F::Panel,cfgpractice::disk::PanelWidgetReflection);
        const bool slotTyped=cfgpracticeobs::Typed(reader,context.registry,M::Base,F::DonorSlot,cfgpractice::disk::PanelSlotReflection);
        const bool contentRead=memory.Read(F::DonorSlot+cfgpractice::disk::Content,content);memory.Read(F::Panel+cfgpractice::disk::Slots,listAgain);
        std::printf("layout diag observe=%d same=%d owned=%d rootClear=%d anchorClear=%d panelTyped=%d slotTyped=%d list=%d/%d/%p slot=%p/%p content=%p/%p headerAgain=%d type=%p rowclass=%p tree=%p/%p outer=%p world=%p after=%u index=%u\n",
          freshOk,Same(context,fresh),cfghosted::native::WorldOwned(reader,context.controller,context.world),rootClear,anchorClear,
          panelTyped,slotTyped,list.count,list.capacity,reinterpret_cast<void*>(list.data),reinterpret_cast<void*>(slotValue),reinterpret_cast<void*>(F::DonorSlot),
          reinterpret_cast<void*>(content),reinterpret_cast<void*>(M::Donor),listAgain==list,
          reinterpret_cast<void*>(anchorType),reinterpret_cast<void*>(layout.rowClass),reinterpret_cast<void*>(rootTree),reinterpret_cast<void*>(treeRoot),
          reinterpret_cast<void*>(outer),reinterpret_cast<void*>(context.world),layout.after,discovery.graph.nodes[1].index);
        (void)contentRead;(void)slotIdentity;}
    check(layoutNow,"actual LayoutCurrent proves NativeFixture panel/anchor graph");
    static const auto current=[](Reader& r,void* p){auto& proof=*static_cast<H215HostedGraphProof*>(p);return LayoutCurrent(r,*proof.discovery,*proof.layout);};
    H215HostedGraphProof graphProof{&discovery,&layout};callbackComplete=true;windowHeld=false;
    unsigned providerInputs=0,providerInputFailures=0,creates=0,attaches=0,removes=0;
    {
    HostedLobbyRootProvider provider(discovery,layout,Fixture::Mode,0x810000,0x820000);
    rows::HostedLobbyRetentionScope scope(M::Base,provider.Hooks(),provider.Admitted());
    if(!scope.Entered())std::printf("adapter provider refusal stage=%u reason=%u value=%p current=%d callback=%d\n",provider.Stage(),
        provider.Failure().reason,reinterpret_cast<void*>(provider.Failure().value),H215HostedCurrent(reader,&graphProof),callbackComplete);
    check(scope.Entered(),"actual provider grants retained host callback");
    H215HostedRowModel engine{fixture,memory};reflection::RetainedMenuText text;
    rows::NativeAccess<Reader,H215HostedRowModel> access(reader,engine,text);
    active::Context nativeContext;nativeContext.module=context.module;nativeContext.thread=context.thread;
    nativeContext.world=context.world;nativeContext.controller=context.controller;nativeContext.widget=context.menu;nativeContext.type=context.type;
    const bool prepared=access.PrepareHosted(nativeContext,1,M::Donor,F::Panel,&graphProof,current);
    if(!prepared){capture::FunctionEvidence optionsFunction;capture::OptionsEvidence optionCopy;const bool optionFn=capture::Function(reader,M::Base,M::DonorType,u"SetOptions",optionsFunction);
        const bool optionDeps=optionFn&&capture::OptionsDependencies(reader,M::Base,optionsFunction,optionCopy);cfgnames::Observation<Reader> names(reader,M::Base);cfgnames::Text donorName;
        const bool nameOk=names.Object(M::DonorType,donorName);std::uintptr_t donorClass=0;reader.Read(M::Donor+ObjectClass,donorClass);
        std::printf("adapter prepare fail current=%d scope=%d classMatch=%d name=%d optionsFn=%d deps=%d currentReason=%u\n",
          current(reader,&graphProof),rows::HostedLobbyRetentionScope::Current(M::Base),donorClass==M::DonorType,nameOk,optionFn,optionDeps,access.currentReason);}
    check(prepared,"actual NativeAccess PrepareHosted uses shared retained graph");
    std::uintptr_t row=0;bool attached=false;
    const bool appended=prepared&&access.AppendHostedAfter(1,row,attached);
    if(!appended)std::printf("adapter append stage=%u invokeFailure=%u creates=%u attaches=%u providerStage=%u providerReason=%u providerInputs=%u/%u windowHeld=%d\n",
        access.operationStage,access.invokeReason,engine.creates,engine.attaches,provider.Stage(),provider.Failure().reason,
        provider.InputAttempts(),provider.InputFailures(),windowHeld);
    check(appended&&attached&&row==F::NewWidget&&engine.creates==1&&engine.attaches==1,"one actual Create+AddChild completes in same retained callback");
    cfgpracticeobs::WidgetNode::List after{};memory.Read(F::Panel+cfgpractice::disk::Slots,after);
    check(after.count==2&&memory.bytes.size()>0,"native forward Slots edge includes appended owned slot");
    rows::Spec spec;spec.id=cfgmenu::RowId::Difficulty;spec.title=u"Hosted Jason";spec.count=2;spec.selected=1;
    spec.labels[0]=u"Off";spec.labels[1]=u"On";
    const bool copiedOptions=access.CopyOptions(row,spec);
    if(!copiedOptions)std::printf("H219 SetOptions diagnostic stage=%u operation=%u provider=%u/%u input=%u/%u calls=%u options=%u engine=%d/%d current=%d\n",
        access.operationStage,access.currentReason,provider.Stage(),provider.Failure().reason,provider.InputAttempts(),provider.InputFailures(),engine.calls,
        engine.options,engine.lastOkay,engine.lastOutputOkay,access.Current(1));
    check(copiedOptions,"serial-zero SetOptions proves exact script/dependencies and copies choices");
    unsigned selected=0;check(access.Index(row,selected)&&selected==1,"hosted row selection reads copied native value");
    reflection::RetainedMenuText::Request converted;
    check(access.Text(spec.title,converted)&&access.SetText(row,u"TitleText",converted),"hosted title conversion and retained SetText");
    const bool valueConverted=access.Text(spec.labels[selected],converted);
    const bool valueSet=valueConverted&&access.SetText(row,u"ValueText",converted);
    if(!valueSet)std::printf("H219 value text converted=%d provider=%u/%u role=%u value=%p engine=%d/%d texts=%u calls=%u\n",
        valueConverted,provider.Stage(),provider.Failure().reason,provider.Role(),reinterpret_cast<void*>(provider.Failure().value),engine.lastOkay,engine.lastOutputOkay,engine.texts,engine.calls);
    check(valueSet&&engine.texts==2&&engine.displayedTitle==spec.title&&engine.displayedValue==spec.labels[selected],
        "hosted selected value conversion and retained SetText");
    unsigned char changed=0;if(memory.Read(OptionsFixture::ScriptData+10,changed)){
        memory.Set(OptionsFixture::ScriptData+10,static_cast<unsigned char>(changed^1));const auto callsBefore=engine.calls;
        check(!access.CopyOptions(row,spec)&&engine.calls==callsBefore,"mutated SetOptions byte refuses before invoke");
        check(!rows::HostedLobbyRetentionScope::Current(M::Base),"registered row function reproof rejects changed bytecode in a later window");
        memory.Set(OptionsFixture::ScriptData+10,changed);
        check(rows::HostedLobbyRetentionScope::Current(M::Base),"restored owned fixture permits safe cleanup");}
    int panelIndex=-1,panelSerial=-1;memory.Read(F::Panel+cfgprobe::lifetime::ObjectIndexOffset,panelIndex);
    memory.Read(F::Registry+24ULL*static_cast<unsigned>(panelIndex)+16,panelSerial);
    check(panelSerial==0,"authored panel remains serial zero");
    const bool removed=access.RemoveOwned(row);
    if(!removed){capture::FunctionEvidence removeEvidence;MenuWarmDiagnostic removeDiagnostic{};
        const bool removeCached=native::CachedFunction(reader,M::Base,execdisk::RemoveChild,execdisk::RemoveChildParameters,
            execdisk::RemoveChildCode,removeEvidence,&removeDiagnostic);
        std::uintptr_t panelClass=0,removeOwnerCache=0,removeConstructorCache=0,removeFunctionCache=0;memory.Read(F::Panel+ObjectClass,panelClass);
        memory.Read(M::Base+execdisk::RemoveChild.reflection,removeOwnerCache);memory.Read(M::Base+execdisk::RemoveChild.constructor,removeConstructorCache);
        memory.Read(M::Base+execdisk::RemoveChild.cache,removeFunctionCache);
        cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation functionSlot{};
        cfgprobe::lifetime::ReadArrayHeader(reader,M::Base+cfgprobe::lifetime::ObjectArrayRva,registry);
        int functionIndex=-1;memory.Read(fixture.removeFunction+cfgprobe::lifetime::ObjectIndexOffset,functionIndex);
        const bool registered=cfgprobe::lifetime::ReadSlot(reader,registry,fixture.removeFunction,functionSlot);
        std::uint32_t internal=~0u,flags=~0u;int serial=-1;
        if(registered){reader.Read(registry.data+24ULL*static_cast<unsigned>(functionSlot.index)+8,internal);
            reader.Read(registry.data+24ULL*static_cast<unsigned>(functionSlot.index)+16,serial);
            reader.Read(fixture.removeFunction+8,flags);}
        const bool methodOwner=removeOwnerCache&&cfgspawn::ObjectIs(reader,F::Panel,removeOwnerCache);
        const auto priorCurrentReason=access.currentReason;const bool stillCurrent=access.Current(1);
        std::printf("adapter RemoveOwned refusal current=%d currentReason=%u/%u operationStage=%u invokeFailure=%u cached=%d evidence=%u frame=%u targetType=%d panelClass=%p ownerCache=%p ctorCache=%p functionCache=%p function=%p index=%d/%d serial=%d internal=%08X flags=%08X provider=%u/%u inputs=%u/%u removes=%u\n",
            stillCurrent,priorCurrentReason,access.currentReason,access.operationStage,access.invokeReason,removeCached,removeDiagnostic.evidence,removeDiagnostic.frame,methodOwner,
            reinterpret_cast<void*>(panelClass),reinterpret_cast<void*>(removeOwnerCache),reinterpret_cast<void*>(removeConstructorCache),reinterpret_cast<void*>(removeFunctionCache),
            reinterpret_cast<void*>(fixture.removeFunction),functionIndex,registered?functionSlot.index:-1,serial,internal,flags,provider.Stage(),provider.Failure().reason,
            provider.InputAttempts(),provider.InputFailures(),engine.removes);
    }
    check(removed&&engine.removes==1,"exact RemoveChild rollback removes the owned row");
    memory.Read(F::Panel+cfgpractice::disk::Slots,after);check(after.count==1,"rollback preserves donor anchor forward edge");
    providerInputs=provider.InputAttempts();providerInputFailures=provider.InputFailures();
    creates=engine.creates;attaches=engine.attaches;removes=engine.removes;
    }
    check(!windowHeld,"provider and retained-scope destructors close the callback window");
    check(hostedLobbyRootBank.Healthy(),"provider teardown leaves the hosted root bank healthy");
    std::array<std::uintptr_t,32*32> remainingRoots{};unsigned remainingRootCount=0;
    const bool rootsObserved=hostedLobbyRootBank.ObserveSlots(remainingRoots,remainingRootCount);
    bool noLeakedRoots=rootsObserved;
    for(unsigned i=0;i<remainingRootCount;++i)noLeakedRoots=noLeakedRoots&&!remainingRoots[i];
    check(noLeakedRoots,"provider teardown clears every retained root claim");
    std::printf("H215 adapter reader budget: backing=%u/%u adapter=%u/%u providerContext/Window=32768 providerOutputs=16384 inputs=%u/%u; create=%u attach=%u remove=%u; teardown held=%d bankHealthy=%d bankOwners=%u rootsObserved=%d rootSlots=%u noLeakedRoots=%d\n",
        memory.count,memory.cap,reader.reads,reader.cap,providerInputs,providerInputFailures,creates,attaches,removes,
        windowHeld,hostedLobbyRootBank.Healthy(),hostedLobbyRootBank.Owners(),rootsObserved,remainingRootCount,noLeakedRoots);
    active=nullptr;return failures==0;
}
}

int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* what){++checks;if(!okay){++failures;std::printf("FAIL %s\n",what);}};
    using namespace cfgprobe;using namespace cfgmenu;using namespace cfghostedlobby;
    LobbyFixture f;f.Slot(f.World,0,100);f.Slot(f.hud.widget.object,20,120);f.Slot(f.Child,17,131);f.Slot(f.Root,16,116);f.Slot(f.UserType,31,132);
    f.r.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,GetCurrentThreadId());
    f.r.Set(f.Root+8,std::uint32_t{0xA80048});f.r.Set(f.Registry+24ULL*16+16,int{});
    for(auto& node:f.graph.nodes){if(node.widget.object==f.Child)node.widget.serial=131;
        if(node.widget.object==f.Root)node.widget.serial=0;
        if(node.widget.object==f.hud.widget.object)node.widget.serial=120;}
    f.Label(f.Root,"CustomServerSettings");f.Label(f.PanelType,"VerticalBox");
    f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
    f.Add(f.SlotType,"Parent","ObjectProperty",0x180,1,8);f.Add(f.SlotType,"Content","ObjectProperty",cfgpractice::disk::Content,1,8);
    f.r.Set(f.Root+ObjectClass,f.PanelType);f.r.Set(f.Module+cfgpractice::disk::PanelWidgetReflection,f.PanelType);
    f.r.Set(f.Module+cfgpractice::disk::PanelWidgetConstructor,f.PanelType);
    f.r.Set(f.Child+ObjectClass,f.UserType);f.r.Set(f.Controller+0x20,f.World);
    constexpr std::uintptr_t AddFunction=0x2E00000,RemoveFunction=0x2E08000,AddedSlot=0x2E10000;
    const auto fn=BindPanelFunction(f,execdisk::AddChildToVerticalBox,execdisk::AddChildToVerticalBoxParameters,
        execdisk::AddChildToVerticalBoxCode,f.PanelType,AddFunction,0,true);
    const auto removeFn=BindPanelFunction(f,execdisk::RemoveChild,execdisk::RemoveChildParameters,
        execdisk::RemoveChildCode,f.PanelType,RemoveFunction,1,true);
    f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{36});
    f.r.Set(f.Module+execdisk::Create.cache,std::uintptr_t{0x2E20000});
    f.r.Set(f.Module+execdisk::Conv_StringToText.cache,std::uintptr_t{0x2E30000});
    f.Slot(AddedSlot,35,0);f.r.Set(AddedSlot+8,std::uint32_t{0x48});f.r.Set(f.Registry+24ULL*35+8,std::uint32_t{0x42000000});
    f.r.Set(AddedSlot+ObjectClass,f.SlotType);f.r.Set(AddedSlot+0x180,f.Root);
    f.r.Set(AddedSlot+cfgpractice::disk::Content,f.Child);
    f.r.Set(f.Root+cfgpractice::disk::Slots,cfgpracticeobs::WidgetNode::List{f.Slots,2,2});
    f.r.Set(f.Slots+8,AddedSlot);f.r.Set(f.Child+cfgpractice::disk::Content,AddedSlot);

    ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=1000000;::active=&memory;
    Context context;Failure why;check(Observe(memory,f.Module,GetCurrentThreadId(),context,why)&&context.listenHost,"real Observe confirms listen-host context");
    Discovery discovery;Snapshot snapshot;check(discovery.Begin(memory,context,snapshot),"actual discovery begins");
    discovery.graph=f.graph;discovery.context=context;
    RowLayout layout{f.World,context.menu,f.UserType,f.Root,f.Child,1,1};
    check(LayoutCurrent(memory,discovery,layout),"real LayoutCurrent proves authored panel and donor anchor");

    cfgprobe::lifetime::ArrayHeader outputRegistry{};
    check(cfgprobe::lifetime::ReadArrayHeader(memory,f.Module+cfgprobe::lifetime::ObjectArrayRva,outputRegistry),"output registry current");
    const auto outputCurrent=[&](){return cfghostedlobbyoutput::Current(memory,f.Module,outputRegistry,f.Root,AddedSlot,f.Child);};
    check(outputCurrent(),"serial-zero output has exact typed parent/content/forward membership");
    memory.Set(AddedSlot+0x180,f.Root+0x100);check(!outputCurrent(),"foreign parent output refused");memory.Set(AddedSlot+0x180,f.Root);
    memory.Set(AddedSlot+cfgpractice::disk::Content,f.Root);check(!outputCurrent(),"wrong content output refused");memory.Set(AddedSlot+cfgpractice::disk::Content,f.Child);
    memory.Set(f.Slots+8,f.SlotObject);check(!outputCurrent(),"detached output refused");memory.Set(f.Slots+8,AddedSlot);
    memory.Set(f.Slots,AddedSlot);check(!outputCurrent(),"duplicate output membership refused");memory.Set(f.Slots,f.SlotObject);
    memory.Set(AddedSlot+ObjectClass,f.UserType);check(!outputCurrent(),"wrong output class refused");memory.Set(AddedSlot+ObjectClass,f.SlotType);

    reflection::AddParameters packet{};packet.content=f.Child;packet.result=AddedSlot;PacketBytes(memory,&packet,sizeof(packet));
    capture::FunctionEvidence functionEvidence;MenuWarmDiagnostic functionDiagnostic{};
    const bool functionReady=native::CachedFunction(memory,f.Module,execdisk::AddChildToVerticalBox,
        execdisk::AddChildToVerticalBoxParameters,execdisk::AddChildToVerticalBoxCode,functionEvidence,&functionDiagnostic);
    check(functionReady,"exact native AddChild function metadata fixture");
    const auto thread=GetCurrentThreadId();f.r.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,thread);
    callbackComplete=true;
    {
        HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
        rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
        const bool wrongFunction=rows::HostedLobbyRetentionScope::Inputs(f.Module,f.Root,AddFunction+0x1000,&packet);
        check(scope.Entered()&&!wrongFunction,"same serial-zero panel refuses a different target UFunction");
        check(provider.Failure().reason==4&&provider.Role()==2,"wrong-function refusal is the exact positive-slot gate");
        check(!rows::HostedLobbyRetentionScope::EnterNative(f.Module),"wrong target proof cannot enter native code");
    }
    {
        HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
        rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
        check(scope.Entered(),"real provider admits captured host callback");
        const bool accepted=rows::HostedLobbyRetentionScope::Inputs(f.Module,f.Root,fn,&packet);
        if(!accepted){const auto& d=provider.Failure();std::printf("target refusal stage=%u reason=%u role=%u value=%p serial=%d internal=%u flags=%u\n",
            provider.Stage(),d.reason,provider.Role(),reinterpret_cast<void*>(d.value),d.slot.serial,d.internal,d.flags);}
        check(accepted,"serial-zero authored panel is admitted as exact AddChild target");
        int serial=-1;f.r.Read(f.Registry+24ULL*16+16,serial);check(serial==0,"panel serial remains zero");
        check(rows::HostedLobbyRetentionScope::EnterNative(f.Module)&&!windowHeld,"native entry follows registration window close");
        const bool addOutput=rows::HostedLobbyRetentionScope::Outputs(f.Module,fn,&packet);
        if(!addOutput)std::printf("serial0 add output fail stage=%u reason=%u role=%u value=%p\n",provider.Stage(),provider.Failure().reason,provider.Role(),reinterpret_cast<void*>(provider.Failure().value));
        check(addOutput,"AddChild output completes retained call");
        check(provider.Failure().reason==0,"provider has no failed proof");
        memory.Set(AddedSlot+0x180,f.Root+0x100);check(!rows::HostedLobbyRetentionScope::Current(f.Module),"retained slot parent mutation refuses next window");memory.Set(AddedSlot+0x180,f.Root);
        memory.Set(AddedSlot+cfgpractice::disk::Content,f.Root);check(!rows::HostedLobbyRetentionScope::Current(f.Module),"retained slot content mutation refuses next window");memory.Set(AddedSlot+cfgpractice::disk::Content,f.Child);
        memory.Set(AddedSlot+ObjectClass,f.UserType);check(!rows::HostedLobbyRetentionScope::Current(f.Module),"retained slot class mutation refuses next window");memory.Set(AddedSlot+ObjectClass,f.SlotType);
        memory.Set(f.Slots+8,f.SlotObject);check(!rows::HostedLobbyRetentionScope::Current(f.Module),"retained slot detach refuses next window");memory.Set(f.Slots+8,AddedSlot);
    }
    // The authored AddChild UFunction itself can have serial zero too. Its
    // exact cache/owner/parameter/code proof and the strong panel witness must
    // admit this one target; arbitrary serial-zero functions remain rejected.
    {
        HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
        rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
        check(scope.Entered()&&rows::HostedLobbyRetentionScope::Inputs(f.Module,f.Root,fn,&packet),
            "exact serial-zero AddChild function is retained with authored panel");
        check(rows::HostedLobbyRetentionScope::EnterNative(f.Module)&&rows::HostedLobbyRetentionScope::Outputs(f.Module,fn,&packet),
            "serial-zero AddChild function completes the real retention adapter");
    }
    {
        std::uintptr_t child=f.Child;PacketBytes(memory,&child,sizeof(child));
        HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
        rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
        check(scope.Entered()&&rows::HostedLobbyRetentionScope::Inputs(f.Module,f.Root,removeFn,&child),
            "exact serial-zero RemoveChild function has symmetric authored-panel proof");
        check(rows::HostedLobbyRetentionScope::EnterNative(f.Module)&&rows::HostedLobbyRetentionScope::Outputs(f.Module,removeFn,&child),
            "serial-zero RemoveChild function completes the real retention adapter");
    }
    {
        PacketBytes(memory,&packet,sizeof(packet));
        HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
        rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
        check(scope.Entered()&&rows::HostedLobbyRetentionScope::Inputs(f.Module,f.Root,fn,&packet),"exact function initially retained");
        memory.Set(f.Module+execdisk::AddChildToVerticalBox.cache,AddFunction+0x1000);
        check(!rows::HostedLobbyRetentionScope::Current(f.Module),"changed native function cache invalidates retained context before dispatch");
        memory.Set(f.Module+execdisk::AddChildToVerticalBox.cache,AddFunction);
    }
    check(!windowHeld&&hostedLobbyRootBank.Healthy(),"collector windows close and dedicated root bank stays healthy");
    ::active=nullptr;
    check(H215HostedRowAdapter(),"actual NativeAccess hosted Create/Attach/Rollback adapter");
    std::printf("H215 real hosted provider AddChild target integration: %u checks, %u failures; no game/native calls.\n",checks,failures);
    return failures?1:0;
}
