#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "NativePickerArrivalObservation.hpp"
#include "PickerPrecreationFixture.hpp"
struct ArrivalReader {
    OptionsReader inner;unsigned reads=0,limit=32768;bool exhausted=false;
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t size){if(reads==limit){exhausted=true;return false;}++reads;return inner.ReadBytes(at,out,size);}
    template<class T> bool Read(std::uintptr_t at,T& out){return ReadBytes(at,&out,sizeof(out));}
};
// Actual prepared-picker fixture plus an owned authored entry/map chain. No
// process addresses, native calls, identity allocation, or callback publication.
struct ArrivalFixture:PickerPreparedFixture {
    static constexpr std::uintptr_t Map=0x5100000,Settings=0x5110000,Pending=0x5120000,Mode=0x5130000,MapMember=0x5140000,
        Local=0x5150000,EntryGraph=0x5160000,MapGraph=0x5170000,EntryCode=0x5180000,MapCode=0x5190000,Widget=0x51A0000;
    std::unique_ptr<cfgmenu::entrysettings::Resources> entry=std::make_unique<cfgmenu::entrysettings::Resources>();
    std::unique_ptr<cfgmenu::pickerprepare::Prepared> prepared=std::make_unique<cfgmenu::pickerprepare::Prepared>();
    cfgmenu::pickerprepare::Executors expected{};cfgmenu::pickerprepare::Receiver receiver{};cfgmenu::active::Context actual{};
    bool ready=false;
    ArrivalFixture(const char* root,const char* counselor,const char* jason):PickerPreparedFixture(root,counselor,jason){
        using namespace cfgmenu;using namespace cfgprobe;using M=MenuFixture;auto& f=fixture;auto& r=f.r;OptionsReader reader{r};if(!initialized)return;
        r.Set(M::Base+pushdisk::MenuClass,Menu);r.Set(M::Base+pushdisk::MenuConstructor,Menu);
        for(const auto type:{Map,Settings}){r.Set(type+ClassSuper,Menu);Register(type);}
        f.Name(Map,"PickOfflineBotsMapMenuWidget_C");f.Name(Settings,"OfflineBotsSettingsMenuWidget_C");
        const auto property=[&](std::uintptr_t at,std::uintptr_t ownerType,std::uintptr_t kind,const char* name,int offset,std::uint64_t flags){
            f.Field(at,ownerType,kind,name,0);f.Property(at,8,offset);r.Set(at+cfgreflection::disk::PropertyFlags,flags);};
        property(Pending,Frontend,ClassKind,"PendingOfflineSettingsWidget",0x528,0x001C001040000204ULL);
        property(Mode,Frontend,ClassKind,"PendingOfflineMode",0x520,0x001C001040000204ULL);
        property(MapMember,Frontend,ClassKind,"PendingOfflineMap",0x518,0x001C001040000204ULL);
        property(Local,MapGraph,M::ObjectProperty,"K2Node_DynamicCast_AsSCGame_Menu2",184,0x0008001040000200ULL);
        const auto graph=[&](std::uintptr_t at,std::uintptr_t type,const char* name,std::uint32_t flags,int storage,std::uintptr_t code,int bytes,std::uintptr_t cleanup){
            f.Field(at,type,M::FunctionClass,name,0);Register(at);r.Set(type+cfgreflection::disk::Children,at);
            r.Set(at+cfgreflection::disk::Children,std::uintptr_t{});r.Set(at+cfgreflection::disk::FunctionFlags,flags);
            r.Set(at+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});r.Set(at+cfgreflection::disk::StorageBytes,storage);
            r.Set(at+0xB0,M::Base+startdisk::ScriptExecutor);r.Set(at+0x68,cleanup);r.Set(at+scriptdisk::Script,capture::ScriptHeader{code,bytes,bytes});};
        graph(EntryGraph,M::Type,"ExecuteUbergraph_OfflinePlayMenuWidget",0x00800000,113,EntryCode,1944,0);
        graph(MapGraph,Map,"ExecuteUbergraph_PickOfflineBotsMapMenuWidget",0,193,MapCode,1867,Pending);
        const auto pointer=[](auto& bytes,unsigned at,std::uintptr_t value){std::memcpy(bytes.data()+at,&value,8);};
        std::array<unsigned char,1944> entryBytes{};entryBytes[0x3E8]=0x97;entryBytes[0x3E9]=0x87;entryBytes[0x3EA]=0xF5;
        pointer(entryBytes,0x3EB,Transition);entryBytes[0x3F3]=0x89;pointer(entryBytes,0x3F4,Map);entryBytes[0x3FC]=0x97;entryBytes[0x3FD]=0x87;r.Set(EntryCode,entryBytes);
        std::array<unsigned char,1867> mapBytes{};mapBytes[0x108]=0x0D;mapBytes[0x109]=0xC6;mapBytes[0x10A]=0x76;mapBytes[0x113]=9;
        pointer(mapBytes,0x117,Pending);mapBytes[0x11F]=0xDB;pointer(mapBytes,0x120,Pending);mapBytes[0x128]=0x89;pointer(mapBytes,0x129,Settings);mapBytes[0x131]=0x87;
        const std::array<unsigned,3> starts={0x5D2,0x5F3,0x614};const std::array<std::uintptr_t,3> members={Mode,MapMember,Pending};
        for(unsigned i=0;i<starts.size();++i){const auto at=starts[i];mapBytes[at]=0x0D;mapBytes[at+1]=0xC6;mapBytes[at+2]=0x76;
            pointer(mapBytes,at+3,Local);mapBytes[at+11]=9;pointer(mapBytes,at+15,members[i]);mapBytes[at+23]=0xDB;pointer(mapBytes,at+24,members[i]);mapBytes[at+32]=0x7B;}
        r.Set(MapCode,mapBytes);Register(Widget,9981);r.Set(Widget+ObjectClass,Counselor);r.Set(Widget,WidgetVtable);
        if(!active::Observe(reader,M::Base,owner)||!entrysettings::Observe(reader,owner,42,true,*entry)||
           !pickerprepare::Prepare(reader,owner,*source,pickerprepare::Kind::Counselor,42,true,*prepared))return;
        expected=pickerprepare::Original(*prepared);receiver.widget=Widget;receiver.type=Counselor;receiver.owningController=owner.identities[1];
        if(!lifetime::ReadSlot(reader,owner.registry,Widget,receiver.identity))return;
        r.Set(Stack+execdisk::StackArray,active::StackArray{Records,3,3});r.Set(Records+2*execdisk::RecordBytes,Counselor);
        r.Set(Records+2*execdisk::RecordBytes+execdisk::RecordWidget,Widget);
        ready=active::Observe(reader,M::Base,actual);
    }
};
int main(int argc,char** argv){using namespace cfgmenu;using F=ArrivalFixture;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    if(argc!=4)return 2;auto f=std::make_unique<F>(argv[1],argv[2],argv[3]);check(f->ready,"composed authored entry and prepared picker fixture");if(!f->ready)return 1;
    auto& memory=f->fixture.r;pickerarrival::Diagnostic diagnostic;
    const auto observe=[&](auto& reader){return pickerarrival::Current(reader,f->actual,*f->prepared,f->expected,f->receiver,*f->entry,42,&diagnostic);};
    ArrivalReader single{{memory}};check(observe(single)&&diagnostic.stage==0,"concrete native counselor arrival succeeds");
    ArrivalReader old{{memory}};
    const auto duplicated=[&](auto& reader){return pickerprepare::Current(reader,f->actual,*f->prepared,42,f->expected)&&
        entrysettings::Retained(reader,f->actual,*f->entry)&&pickerprepare::OwnedReceiverCurrent(reader,f->actual,*f->prepared,f->expected,f->receiver,42);};
    check(duplicated(old)&&old.reads>single.reads,"old composition repeats complete prepared metadata proof");
    ArrivalReader boundary{{memory},0,single.reads};check(observe(boundary)&&!boundary.exhausted,"single complete proof passes exact aggregate boundary");
    ArrivalReader oldBoundary{{memory},0,single.reads};check(!duplicated(oldBoundary)&&oldBoundary.exhausted,"duplicate proof refuses the same aggregate boundary");
    std::printf("Concrete arrival read counts single=%u duplicated=%u saved=%u cap=16384; boundary regression cap=%u (not evidence of live exhaustion).\n",single.reads,old.reads,old.reads-single.reads,single.reads);
    for(unsigned fault=0;fault<8;++fault){const auto savedReceiver=f->receiver;const auto savedExpected=f->expected;const auto savedActual=f->actual;
        if(fault==0)f->receiver.widget+=0x1000;
        if(fault==1)f->receiver.identity.serial++;
        if(fault==2)f->receiver.owningController.serial++;
        if(fault==3)f->expected.values[1]++;
        if(fault==4)memory.Set(F::Records,PickerPreparedFixture::Customize);
        if(fault==5)memory.Set(F::Mode+cfgreflection::disk::PropertyOffset,0x528);
        if(fault==6)f->actual.thread=43;
        if(fault==7)f->actual.type=PickerPreparedFixture::Jason;
        ArrivalReader reader{{memory}};check(!observe(reader)&&diagnostic.stage!=0,"ownership/metadata/source/schema/thread/foreign type refuses");
        if(fault==5)check(diagnostic.stage==4&&diagnostic.entryStage==7&&diagnostic.reset.stage==4,"exact retained entry reset substage propagated without extra reads");
        f->receiver=savedReceiver;f->expected=savedExpected;f->actual=savedActual;
        memory.Set(F::Records,PickerPreparedFixture::RootType);memory.Set(F::Mode+cfgreflection::disk::PropertyOffset,0x520);
    }
    for(const auto type:{F::Map,F::Settings}){memory.Set(F::Widget+cfgprobe::ObjectClass,type);memory.Set(F::Records+2*execdisk::RecordBytes,type);
        OptionsReader reader{memory};check(active::Observe(reader,MenuFixture::Base,f->actual),"fresh actual map/settings context");
        ArrivalReader counted{{memory}};check(observe(counted)&&diagnostic.stage==0,"map/settings retain exact prepared metadata and authored entry proof");
        std::printf("Arrival type=%llX reads=%u metadata=%u source=%u\n",static_cast<unsigned long long>(type),counted.reads,diagnostic.metadataStage,diagnostic.sourceStage);
        auto expected=f->expected;f->expected.values[2]++;ArrivalReader bad{{memory}};
        check(!observe(bad)&&diagnostic.stage==3&&diagnostic.metadataStage==12,"map/settings cannot omit exact executor proof");f->expected=expected;
        auto staged=std::make_unique<pickerprepare::Prepared>(*f->prepared);
        auto entry=std::make_unique<entrysettings::Resources>(*f->entry);
        ArrivalReader transaction{{memory}};
        check(pickerarrival::Initialize(transaction,f->actual,*staged,f->expected,f->receiver,*entry,42,&diagnostic)&&diagnostic.stage==0,
            "single composed arrival refreshes and proves picker plus authored entry before commit");
        check(transaction.reads<16384,"arrival transaction remains within unchanged shared read cap");
        ArrivalReader exact{{memory},0,transaction.reads};
        check(pickerarrival::Initialize(exact,f->actual,*staged,f->expected,f->receiver,*entry,42,&diagnostic)&&!exact.exhausted,
            "complete composed arrival passes its exact aggregate boundary");
        check(!pickerarrival::Current(exact,f->actual,*staged,f->expected,f->receiver,*entry,42,&diagnostic)&&exact.exhausted,
            "removed duplicate cannot fit that same aggregate boundary; no live budget cause claim");
        ArrivalReader guardedStart{{memory}};
        check(pickerarrival::Current(guardedStart,f->actual,*staged,f->expected,f->receiver,*entry,42,&diagnostic),
            "later actual Start needs a new independent full source proof");
        const auto identity=staged->source.classes[1];
        memory.Set(NativeFixture::Registry+24*static_cast<std::uintptr_t>(identity.index)+16,9001);
        // Native first weak-serial assignment is internal metadata, not an actor
        // lease. Stage a copy; a downstream schema failure must commit nothing.
        memory.Set(F::Mode+cfgreflection::disk::PropertyOffset,0x528);
        auto untouched=std::make_unique<pickerprepare::Prepared>(*f->prepared);
        auto unchanged=std::make_unique<entrysettings::Resources>(*f->entry);
        ArrivalReader rejected{{memory}};
        check(!pickerarrival::Initialize(rejected,f->actual,*untouched,f->expected,f->receiver,*unchanged,42,&diagnostic)&&
            diagnostic.stage==4&&diagnostic.entryStage==7&&diagnostic.reset.stage==4,
            "entry schema failure records leaf and refuses staged arrival");
        check(untouched->source.classes[1].serial==f->prepared->source.classes[1].serial&&unchanged->settingsIdentity==f->entry->settingsIdentity,
            "failed second half cannot publish refreshed first-half metadata");
        memory.Set(F::Mode+cfgreflection::disk::PropertyOffset,0x520);
        ArrivalReader assigned{{memory}};
        check(pickerarrival::Initialize(assigned,f->actual,*untouched,f->expected,f->receiver,*unchanged,42,&diagnostic)&&
            untouched->source.classes[1].serial==9001,
            "actual first metadata serial is adopted only by a complete successful arrival");
        memory.Set(NativeFixture::Registry+24*static_cast<std::uintptr_t>(identity.index)+16,9002);
        ArrivalReader recycled{{memory}};
        check(!pickerarrival::Current(recycled,f->actual,*untouched,f->expected,f->receiver,*unchanged,42,&diagnostic)&&
            diagnostic.stage==3&&diagnostic.metadataStage==1&&diagnostic.sourceStage==11,
            "changed assigned source serial never reuses arrival authority at Start; exact failure propagated");
        ArrivalReader refused{{memory}};
        check(!pickerarrival::Initialize(refused,f->actual,*untouched,f->expected,f->receiver,*unchanged,42,&diagnostic)&&
            diagnostic.stage==3&&diagnostic.refreshStage==2,
            "arrival cannot renew a previously assigned source identity");
        memory.Set(NativeFixture::Registry+24*static_cast<std::uintptr_t>(identity.index)+16,identity.serial);
        auto executor=f->expected;executor.values[2]++;
        ArrivalReader executorFailure{{memory}};
        check(!pickerarrival::Initialize(executorFailure,f->actual,*staged,executor,f->receiver,*entry,42,&diagnostic)&&
            diagnostic.stage==3&&diagnostic.metadataStage==12,
            "single transaction retains exact published executor checks and their leaf diagnostics");
        std::printf("Staged arrival type=%llX reads=%u cap16384; duplicate old total=%u. No native calls.\n",
            static_cast<unsigned long long>(type),transaction.reads,transaction.reads+counted.reads);
    }
    std::printf("CFG concrete native picker arrival: %s (%u checks, %u failures). Owned memory only; no game access.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
