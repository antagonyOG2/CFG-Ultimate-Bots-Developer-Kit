#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <bcrypt.h>
#include <cstdio>
#include <vector>
#include <string>
#include "MenuClassCache.hpp"
#include "SnapshotBindings.hpp"
#include "SpawnBindings.generated.hpp"
#include "MenuOptionsObservation.hpp"
#include "MenuNativeCaches.generated.hpp"
#include "NativeMapSelectionObservation.hpp"
#include "FrontendHudObservation.hpp"
#include "NativeProfileSelectionObservation.hpp"
#include "MenuNativeEvidence.hpp"
#include "SettingsStartObservation.hpp"
#include "NativeFrontendCatalogues.hpp"
#include <memory>
namespace {
constexpr wchar_t Path[]=L"G:\\iknowimiconic\\CampForestGreenLauncher\\Games\\cfg-live\\CampForestGreen\\Binaries\\Win64\\SummerCamp-Win64-Shipping.exe";
struct Handle {HANDLE value=nullptr;explicit Handle(HANDLE h=nullptr):value(h){}~Handle(){if(value&&value!=INVALID_HANDLE_VALUE)CloseHandle(value);}Handle(const Handle&)=delete;Handle& operator=(const Handle&)=delete;};
class Reader {
    HANDLE process_;ULONGLONG start_=GetTickCount64();
public:
    unsigned reads=0,cap=750000;bool exhausted=false;
    explicit Reader(HANDLE p):process_(p){}
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t bytes){
        if(reads>=cap||GetTickCount64()-start_>30000){exhausted=true;return false;}
        ++reads;if(bytes>65536||!output||!cfgprobe::lifetime::Extent(address,bytes))return false;
        SIZE_T done=0;return ReadProcessMemory(process_,reinterpret_cast<const void*>(address),output,bytes,&done)&&done==bytes;
    }
    template<class T>bool Read(std::uintptr_t address,T& out){T value{};if(!ReadBytes(address,&value,sizeof(value)))return false;out=value;return true;}
};
bool ProcessPath(HANDLE process){wchar_t name[32768]{};DWORD size=32768;return QueryFullProcessImageNameW(process,0,name,&size)&&!_wcsicmp(name,Path);}
bool Hash(HANDLE file){
    LARGE_INTEGER size{};if(file==INVALID_HANDLE_VALUE||!GetFileSizeEx(file,&size)||size.QuadPart!=cfgprobe::ExecutableBytes)return false;
    std::vector<unsigned char> input(static_cast<std::size_t>(size.QuadPart));DWORD read=0;
    if(!ReadFile(file,input.data(),static_cast<DWORD>(input.size()),&read,nullptr)||read!=input.size())return false;
    BCRYPT_ALG_HANDLE alg=nullptr;if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;
    std::array<unsigned char,32> digest{};const auto status=BCryptHash(alg,nullptr,0,input.data(),static_cast<ULONG>(input.size()),digest.data(),32);
    BCryptCloseAlgorithmProvider(alg,0);if(status<0)return false;constexpr char hex[]="0123456789ABCDEF";
    for(unsigned i=0;i<32;++i)if(hex[digest[i]>>4]!=cfgprobe::ExecutableSha256[2*i]||hex[digest[i]&15]!=cfgprobe::ExecutableSha256[2*i+1])return false;
    return true;
}
DWORD Find(){
    Handle list(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0));if(list.value==INVALID_HANDLE_VALUE)return 0;
    PROCESSENTRY32W item{};item.dwSize=sizeof(item);if(!Process32FirstW(list.value,&item))return 0;
    DWORD pid=0;unsigned count=0,seen=0;
    do{if(++seen>65536)return 0;if(_wcsicmp(item.szExeFile,L"SummerCamp-Win64-Shipping.exe"))continue;
        Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,item.th32ProcessID));
        if(process.value&&ProcessPath(process.value)){pid=item.th32ProcessID;++count;}
    }while(Process32NextW(list.value,&item));
    return GetLastError()==ERROR_NO_MORE_FILES&&count==1?pid:0;
}
bool Module(HANDLE process,Reader& reader,std::uintptr_t& base){
    base=0;if(!ProcessPath(process))return false;std::array<HMODULE,512> modules{};DWORD bytes=0;
    if(!EnumProcessModulesEx(process,modules.data(),sizeof(modules),&bytes,LIST_MODULES_64BIT)||bytes<sizeof(HMODULE)||bytes>sizeof(modules))return false;
    wchar_t path[32768]{};if(!GetModuleFileNameExW(process,modules[0],path,32768)||_wcsicmp(path,Path))return false;
    const auto candidate=reinterpret_cast<std::uintptr_t>(modules[0]);IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS64 nt{};
    if(!reader.Read(candidate,dos)||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<64||dos.e_lfanew>4096||
       !reader.Read(candidate+static_cast<unsigned>(dos.e_lfanew),nt)||nt.Signature!=IMAGE_NT_SIGNATURE||nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64||
       nt.FileHeader.TimeDateStamp!=cfgprobe::ExecutableTimestamp||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC||nt.OptionalHeader.SizeOfImage!=cfgprobe::ExecutableImageBytes)return false;
    base=candidate;return true;
}
struct Context {std::uintptr_t engine=0,viewport=0,world=0,net=0;bool operator==(const Context& b)const{return engine==b.engine&&viewport==b.viewport&&world==b.world&&net==b.net;}};
bool ReadContext(Reader& r,std::uintptr_t base,Context& c){
    return r.Read(base+cfgprobe::EngineGlobalCandidate,c.engine)&&cfgprobe::ObjectPointer(c.engine)&&
        r.Read(c.engine+cfgprobe::EngineGameViewport,c.viewport)&&cfgprobe::ObjectPointer(c.viewport)&&
        r.Read(c.viewport+cfgprobe::ViewportWorld,c.world)&&cfgprobe::ObjectPointer(c.world)&&
        r.Read(c.world+cfgprobe::WorldNetDriver,c.net)&&!c.net;
}
void Name(const cfgnames::Text& name){for(auto c:name.View())std::putchar(c>=32&&c<127?static_cast<char>(c):'?');}
template<std::size_t Fields,std::size_t Bytes> void Function(const cfgmenu::capture::FunctionEvidenceStorage<Fields,Bytes>& f){
    std::printf("Function=0x%llX Owner=0x%llX Serial=%d Name=",static_cast<unsigned long long>(f.function.object),static_cast<unsigned long long>(f.function.owner),f.slot.serial);Name(f.function.label);
    std::printf(" Flags=%08X ParameterBytes=%u StorageBytes=%d ScriptBytes=%d NativeExecutor=0x%llX DestructorLink=0x%llX\n",f.flags,f.parameterBytes,f.storageBytes,f.script.count,static_cast<unsigned long long>(f.nativeExecutor),static_cast<unsigned long long>(f.destructorLink));
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];std::printf(" Parameter=0x%llX Name=",static_cast<unsigned long long>(p.field.object));Name(p.field.label);std::printf(" Type=");Name(p.field.kind);
        std::printf(" Offset=%d ElementBytes=%d Dimension=%d Flags=%016llX\n",p.offset,p.elementBytes,p.dimension,static_cast<unsigned long long>(p.flags));}
    std::puts("ScriptHexBegin");for(int i=0;i<f.script.count;++i){std::printf("%02X",f.bytes[static_cast<unsigned>(i)]);if(i%32==31)std::putchar('\n');}std::puts("\nScriptHexEnd");
}
void Property(const char* role,const cfgreflection::Property& p){
    std::printf(" %s=0x%llX Owner=0x%llX Type=0x%llX Name=",role,static_cast<unsigned long long>(p.field.object),static_cast<unsigned long long>(p.field.owner),static_cast<unsigned long long>(p.field.type));Name(p.field.label);
    std::printf(" Kind=");Name(p.field.kind);std::printf(" Offset=%d ElementBytes=%d Dimension=%d Flags=%016llX\n",p.offset,p.elementBytes,p.dimension,static_cast<unsigned long long>(p.flags));
}
struct Candidate {std::uintptr_t object=0,type=0,outer=0,defaults=0;std::uint32_t flags=0;cfgprobe::lifetime::SlotObservation slot{},classSlot{};cfgnames::Words name{};};
bool Identity(Reader& r,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t object,Candidate& c){
    c={};c.object=object;
    return cfgprobe::ObjectPointer(object)&&r.Read(object+cfgprobe::ObjectClass,c.type)&&cfgprobe::ObjectPointer(c.type)&&
        r.Read(object+0x20,c.outer)&&r.Read(object+8,c.flags)&&r.Read(object+cfgnames::disk::ObjectName,c.name)&&r.Read(c.type+cfgspawn::ClassDefaultObject,c.defaults)&&
        cfgprobe::lifetime::ReadSlot(r,registry,object,c.slot)&&cfgprobe::lifetime::ReadSlot(r,registry,c.type,c.classSlot);
}
bool Same(const Candidate& a,const Candidate& b){return a.object==b.object&&a.type==b.type&&a.outer==b.outer&&a.defaults==b.defaults&&a.flags==b.flags&&a.slot==b.slot&&a.classSlot==b.classSlot&&a.name==b.name;}
#include "SummaryReadOnlyCapture.inl"
int Capture(HANDLE process,std::uintptr_t base,Reader& r,bool currentOnly,bool selectionsOnly,bool startOnly,bool cataloguesOnly){
    if(selectionsOnly||cataloguesOnly)r.cap=(startOnly||cataloguesOnly)?16384:4096;
    using namespace cfgprobe::lifetime;ArrayHeader registry;Context context;
    if(!ReadContext(r,base,context)||!ReadArrayHeader(r,base+ObjectArrayRva,registry)||registry.count>262144)return 5;
    cfgmenu::active::Context activeMenu;cfgmenu::maps::Selection mapSelection;
    const bool activeObserved=cfgmenu::active::Observe(r,base,activeMenu);
    cfgmenu::maps::Diagnostic mapDiagnostic;
    const bool mapObserved=activeObserved&&cfgmenu::maps::Observe(r,activeMenu,activeMenu.thread,true,mapSelection,nullptr,&mapDiagnostic);
    cfgmenu::profiles::Selection profileSelection;unsigned profileReads=0;cfgmenu::profiles::Diagnostic profileDiagnostic;
    const bool profileObserved=activeObserved&&cfgmenu::profiles::ObserveSnapshot(r,activeMenu,activeMenu.thread,true,profileSelection,&profileReads,&profileDiagnostic);
    cfgnames::Observation<Reader> names(r,base);cfgmenu::capture::MenuClassCache classes;std::vector<Candidate> found;bool discoveryComplete=true;unsigned failures=0;
    if(currentOnly){
        cfgnames::Text label;Candidate current;
        if(!activeObserved||!names.Object(activeMenu.type,label)||label.View()!=u"OfflineBotsSettingsMenuWidget_C"||
            !Identity(r,registry,activeMenu.widget,current)||current.type!=activeMenu.type||
            !(current.slot==activeMenu.identities[2]))return 5;
        found.push_back(current);std::puts("ActiveSettingsOnly=yes RegistryDiscoveryPasses=0 NoGlobalObjectScan=yes");
    }
    if(cataloguesOnly){
        auto modes=std::make_unique<cfgmenu::catalogues::Modes>();auto characters=std::make_unique<cfgmenu::catalogues::Characters>();
        unsigned modeReads=0,characterReads=0;
        const bool modesObserved=mapObserved&&cfgmenu::catalogues::ObserveModes(r,activeMenu,true,*modes,&modeReads);
        const bool charactersObserved=activeObserved&&cfgmenu::catalogues::ObserveCharacters(r,activeMenu,true,*characters,&characterReads);
        std::printf("ShippingModesObserved=%s Reads=%u Count=%u ReadCapacityEntries=no\n",modesObserved?"yes":"no",modeReads,modes->count);
        for(unsigned i=0;i<modes->count;++i){const auto& entry=modes->entries[i];
            std::printf(" Mode[%u] Class=0x%llX Name=",i,static_cast<unsigned long long>(entry.type));Name(entry.name);
            std::printf(" Alias=");for(const auto c:entry.alias.View())std::putchar(c>=32&&c<127?static_cast<char>(c):'?');
            std::printf(" SupportedMaps=%d SupportsSelectedMap=%s\n",entry.supported.count,entry.supportsSelectedMap?"yes":"no");
        }
        std::printf("CharacterCataloguesObserved=%s Reads=%u Counselors=%d Jasons=%d OpaqueBytes=40 CatalogueIsNotOwnership=yes\n",
            charactersObserved?"yes":"no",characterReads,characters->counselors.header.count,characters->jasons.header.count);
        std::printf(" CharacterCatalogueSource=0x%llX SourceType=0x%llX NativeGameModeDefaults=%s DefaultsAreNotCookedRoster=yes\n",
            static_cast<unsigned long long>(characters->source),static_cast<unsigned long long>(characters->type),characters->nativeDefaults?"yes":"no");
        struct Spec{const char16_t* name;std::uintptr_t owner,constructor,cache,executor;};
        using namespace cfgmenu::cataloguedisk;
        const std::array<Spec,4> specs{{
            {u"RequestCounselorClass",SCPlayerStateClass,SCPlayerStateConstructor,RequestCounselorClassCache,RequestCounselorClassExecutor},
            {u"RequestKillerClass",SCPlayerStateClass,SCPlayerStateConstructor,RequestKillerClassCache,RequestKillerClassExecutor},
            {u"GetCurrentCharacterClass",SCCharacterWidgetClass,SCCharacterWidgetConstructor,GetCurrentCharacterClassCache,GetCurrentCharacterClassExecutor},
            {u"SetCurrentCharacterClass",SCCharacterWidgetClass,SCCharacterWidgetConstructor,SetCurrentCharacterClassCache,SetCurrentCharacterClassExecutor}
        }};
        unsigned functionsObserved=0;
        for(const auto& spec:specs){std::uintptr_t owner=0,constructor=0,function=0,fresh=0;
            if(!r.Read(base+spec.owner,owner)||!r.Read(base+spec.constructor,constructor)||
                !r.Read(base+spec.cache,function))return 7;
            std::printf(" NativeSelectionCache Name=");for(const auto* c=spec.name;*c;++c)std::putchar(static_cast<char>(*c));
            std::printf(" Owner=0x%llX Function=0x%llX\n",static_cast<unsigned long long>(owner),static_cast<unsigned long long>(function));
            if(!cfgprobe::MatchingClassCache(owner,constructor)||!cfgprobe::ObjectPointer(function)){std::puts(" CacheNotConstructed=yes NoFactoryCalled=yes");continue;}
            cfgreflection::Observation<Reader> reflection(r,base);cfgreflection::Field field;
            auto evidence=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
            if(!reflection.ReadField(function,owner,field)||!cfgmenu::capture::KnownFunction(r,base,field,spec.name,*evidence)||
                evidence->nativeExecutor!=base+spec.executor||evidence->parameterBytes!=40||evidence->storageBytes!=40||evidence->script.count||
                !r.Read(base+spec.owner,fresh)||fresh!=owner||!r.Read(base+spec.constructor,fresh)||fresh!=constructor||
                !r.Read(base+spec.cache,fresh)||fresh!=function)return 7;
            Function(*evidence);++functionsObserved;
        }
        if(!cfgmenu::active::Current(r,activeMenu,activeMenu.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
        std::printf("CatalogueOnly=yes FunctionsObserved=%u SnapshotIsNotAtomic=yes NativeCallsApproved=no ReadCalls=%u\n",functionsObserved,r.reads);
        std::puts("NoWritesHooksLoadsOrNativeCalls=yes NoGlobalObjectScan=yes NoSoftClassPointerDecoding=yes");
        return modesObserved&&charactersObserved?0:6;
    }
    if(selectionsOnly){
        // Narrow repeat: no graphs, menu-class inventory or registry discovery.
        // Capture the refused raw header before changing any capacity policy.
        cfgmenu::startbinding::Binding startBinding;unsigned startReads=0;bool startObserved=false;
        if(startOnly){auto scratch=std::make_unique<cfgmenu::startbinding::Workspace>();
            startObserved=cfgmenu::startbinding::Observe(r,activeMenu,true,*scratch,startBinding,&startReads);
            std::printf("SettingsStartBindingObserved=%s Reads=%u NoSharedDispatcherOrInterpreterChanges=yes\n",startObserved?"yes":"no",startReads);
            for(const auto* f:{&scratch->start,&scratch->construct,&scratch->graph}){
                cfgmenu::startbinding::FastCall fast;const bool okay=cfgmenu::startbinding::ReadFastCall(r,f->function.object,fast);
                std::printf(" SettingsFunction=0x%llX Name=",static_cast<unsigned long long>(f->function.object));Name(f->function.label);
                std::printf(" Flags=%08X Parameters=%u Storage=%d ScriptBytes=%d Executor=0x%llX FastMetadataRead=%s FastGraph=0x%llX FastEntry=%d\n",
                    f->flags,f->parameterBytes,f->storageBytes,f->script.count,static_cast<unsigned long long>(f->nativeExecutor),okay?"yes":"no",
                    static_cast<unsigned long long>(fast.graph),fast.entry);
            }
            if(startObserved&&!cfgmenu::startbinding::Current(r,startBinding,activeMenu.thread,
                base+cfgmenu::startdisk::ScriptExecutor,base+cfgmenu::startdisk::ScriptExecutor))return 7;
        }
        cfgmenu::profiles::Selection final;
        if(profileObserved&&(!cfgmenu::profiles::ObserveSnapshot(r,activeMenu,activeMenu.thread,true,final)||
            final.save!=profileSelection.save||final.saveType!=profileSelection.saveType||final.defaults!=profileSelection.defaults||
            final.counselor!=profileSelection.counselor||final.jason!=profileSelection.jason||!(final.cache==profileSelection.cache)||
            !(final.identity==profileSelection.identity)))return 7;
        if(mapDiagnostic.shippingRead){cfgmenu::maps::Array fresh;
            if(!r.Read(mapDiagnostic.registryDefaults+cfgmenu::selectiondisk::ShippingMapList,fresh)||!(fresh==mapDiagnostic.shipping))return 7;}
        if(mapObserved){cfgmenu::maps::Selection fresh;
            if(!cfgmenu::maps::Observe(r,activeMenu,activeMenu.thread,true,fresh)||fresh.map!=mapSelection.map||fresh.mode!=mapSelection.mode||
                fresh.settingsClass!=mapSelection.settingsClass||fresh.registryClass!=mapSelection.registryClass||fresh.registryDefaults!=mapSelection.registryDefaults||
                fresh.modeDefaults!=mapSelection.modeDefaults||!(fresh.maps==mapSelection.maps)||!(fresh.supported==mapSelection.supported)||
                fresh.count!=mapSelection.count||fresh.selectedIndex!=mapSelection.selectedIndex||fresh.entries!=mapSelection.entries)return 7;}
        if(!cfgmenu::active::Current(r,activeMenu,activeMenu.thread)||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
        std::printf("SelectionOnly=yes ReflectionGraphCapture=%s ReadLimit=%u ModuleBase=0x%llX\n",startOnly?"three_owned_settings_functions_only":"no",r.cap,static_cast<unsigned long long>(base));
        std::printf("MapObserved=%s Failure=%s HeaderRead=%s Data=0x%llX Count=%d Capacity=%d CountPolicy=%u ReadCapacityEntries=no\n",
            mapObserved?"yes":"no",cfgmenu::maps::Name(mapDiagnostic.failure),mapDiagnostic.shippingRead?"yes":"no",
            static_cast<unsigned long long>(mapDiagnostic.shipping.data),mapDiagnostic.shipping.count,mapDiagnostic.shipping.capacity,cfgmenu::maps::MaximumMaps);
        std::printf("ProfileObserved=%s Failure=%s SaveSerial=%d Counselor=0x%llX Jason=0x%llX Reads=%u\n",profileObserved?"yes":"no",
            profileDiagnostic.stage,profileSelection.identity.serial,static_cast<unsigned long long>(profileSelection.counselor),
            static_cast<unsigned long long>(profileSelection.jason),profileReads);
        std::printf("ProfileRaw Save=0x%llX Type=0x%llX DefaultsRead=%s Defaults=0x%llX CounselorRead=%s Counselor=0x%llX JasonRead=%s Jason=0x%llX\n",
            static_cast<unsigned long long>(profileDiagnostic.save),static_cast<unsigned long long>(profileDiagnostic.saveType),
            profileDiagnostic.defaultsRead?"yes":"no",static_cast<unsigned long long>(profileDiagnostic.defaults),
            profileDiagnostic.counselorRead?"yes":"no",static_cast<unsigned long long>(profileDiagnostic.counselor),
            profileDiagnostic.jasonRead?"yes":"no",static_cast<unsigned long long>(profileDiagnostic.jason));
        if(profileObserved){std::printf("SavedCounselorChoiceUnset=%s SavedJasonChoiceUnset=%s UnsetIsNotSpawnAuthority=yes\n",
                profileSelection.counselor?"no":"yes",profileSelection.jason?"no":"yes");
            for(const auto pick:{profileSelection.counselor,profileSelection.jason}){if(!pick)continue;cfgnames::Text label;
            if(!names.Object(pick,label))return 7;std::printf(" SelectedClass=0x%llX Name=",static_cast<unsigned long long>(pick));Name(label);std::putchar('\n');}}
        std::printf("SamplesConsistent=yes ReadCalls=%u BudgetHit=%s SnapshotIsNotAtomic=yes\n",r.reads,r.exhausted?"yes":"no");
        std::puts("ProfileSaveLifetimePinned=no NativeCallsApproved=no NoWritesHooksLoadsOrNativeCalls=yes");return startOnly&&!startObserved?6:0;
    }
    const auto refuse=[&](const char* reason){
        std::printf("DiscoveryRefused=%s Candidates=%zu ClassCacheHits=%u ClassCacheMisses=%u ClassCacheEvictions=%u\n",reason,found.size(),classes.hits,classes.misses,classes.evictions);return 6;
    };
    std::array<unsigned char,24*256> block{};
    for(int begin=0;!currentOnly&&begin<registry.count;begin+=256){
        const auto count=(registry.count-begin<256)?registry.count-begin:256;
        if(!r.ReadBytes(registry.data+static_cast<std::uintptr_t>(begin)*24,block.data(),static_cast<unsigned>(count)*24))return refuse("registry_block_unreadable_or_budget");
        for(int i=0;i<count;++i){std::uintptr_t object=0,type=0;std::memcpy(&object,block.data()+24*i,8);if(!cfgprobe::ObjectPointer(object))continue;
            if(!r.Read(object+cfgprobe::ObjectClass,type)||!cfgprobe::ObjectPointer(type)){discoveryComplete=false;++failures;continue;}
            bool menu=false;if(!classes.Lookup(type,menu)){
                cfgnames::Text label;
                if(!names.Object(type,label)){discoveryComplete=false;++failures;continue;}
                menu=cfgmenu::observation::Classify(label.View())!=cfgmenu::observation::Route::Unknown;classes.Store(type,menu);
            }
            if(!menu)continue;if(found.size()==16)return refuse("menu_candidate_limit");
            Candidate c;
            if(!Identity(r,registry,object,c)||c.type!=type||c.slot.index!=begin+i){
                discoveryComplete=false;++failures;std::puts("MenuCandidateRefused=identity_changed_or_unreadable");continue;
            }
            cfgnames::Text label;if(!names.Object(object,label))return refuse("candidate_name_unreadable_or_budget");
            if(label.View().substr(0,9)==u"Default__")std::puts("CandidateNameHasDefaultPrefix=yes; native_CDO_flag_not_verified; not_active_screen_proof");
            found.push_back(c);
        }
    }
    std::printf("DiscoveryPassComplete=%s RegistrySlots=%d Candidates=%zu ClassCacheEntries=4096 ClassCacheHits=%u ClassCacheMisses=%u ClassCacheEvictions=%u\n",currentOnly?"not_requested":"yes",registry.count,found.size(),classes.hits,classes.misses,classes.evictions);
    std::printf("ModuleBase=0x%llX\n",static_cast<unsigned long long>(base));
    std::vector<cfgmenu::capture::DiagnosticFunctionEvidence> functions;std::vector<Candidate> links;
    const auto captureFunction=[&](std::uintptr_t type,std::u16string_view name){
        cfgmenu::capture::DiagnosticFunctionEvidence f;if(!cfgmenu::capture::Function(r,base,type,name,f))return false;
        for(const auto& old:functions)if(old.function.object==f.function.object)return true;
        if(functions.size()==64)return false;functions.push_back(f);return true;
    };
    cfgmenu::hud::Binding hudBinding;unsigned hudReads=0;
    std::uintptr_t hudEvent=0;
    // Exact active HUD only. Capturing its event supplies binding evidence;
    // it does not prove that the callback recurs or authorize native service.
    const bool hudPathObserved=activeObserved&&cfgmenu::hud::Observe(r,activeMenu,true,hudBinding,&hudReads);
    if(hudPathObserved&&captureFunction(hudBinding.hudType,u"ReceiveDrawHUD"))hudEvent=hudBinding.event;
    struct NativeCache {std::uintptr_t classCache=0,functionCache=0,type=0,function=0;};std::vector<NativeCache> nativeCaches;
    for(const auto& binding:cfgmenu::nativedisk::Bindings){
        NativeCache cache{base+binding.classCache,base+binding.functionCache};cfgnames::Text name;
        cfgmenu::capture::DiagnosticFunctionEvidence f;
        const bool namedOwner=r.Read(cache.classCache,cache.type)&&cfgprobe::ObjectPointer(cache.type)&&names.Object(cache.type,name)&&name.View()==binding.owner;
        const bool cacheRead=namedOwner&&r.Read(cache.functionCache,cache.function);
        const auto logicalBytes=binding.name==u"RemoveChild"?cfgmenu::native::ParameterExtent(cfgmenu::execdisk::RemoveChild,cfgmenu::execdisk::RemoveChildParameters):
            binding.name==u"ShowLoadingAndTravel"?cfgmenu::native::ParameterExtent(cfgmenu::execdisk::ShowLoadingAndTravel,cfgmenu::execdisk::ShowLoadingAndTravelParameters):binding.parameterBytes;
        const bool ok=cacheRead&&cfgprobe::ObjectPointer(cache.function)&&
            cfgmenu::capture::Function(r,base,cache.type,binding.name,f)&&f.function.object==cache.function&&
            f.parameterBytes==logicalBytes&&f.storageBytes==static_cast<int>(binding.parameterBytes);
        std::printf("NativeCacheName=");for(auto c:binding.name)std::putchar(static_cast<char>(c));std::printf(" ExactNamedCacheMatch=%s NativeCallApproved=no\n",ok?"yes":"no");
        if(cacheRead)nativeCaches.push_back(cache);
        if(ok){if(!captureFunction(cache.type,binding.name))return 6;}
        else if(namedOwner&&captureFunction(cache.type,binding.name)){
            // Named class-owned metadata is useful even before its lazy native
            // cache is populated. This does NOT warm/call the factory or claim
            // a cache match. Metadata is independently rechecked at the end.
            std::printf("NamedFunctionWithoutCacheApproval=");for(auto c:binding.name)std::putchar(static_cast<char>(c));std::putchar('\n');
        }
    }
    for(const auto& c:found){
        cfgmenu::observation::Report schema;cfgmenu::observation::SetupWidget<Reader> observer(r,base,true);
        const bool schemaOk=observer.Capture(c.object,schema);
        std::printf("Widget=0x%llX Class=0x%llX Outer=0x%llX WidgetSerial=%d ClassSerial=%d StrictSchema=%s ActiveScreenProven=no\n",static_cast<unsigned long long>(c.object),static_cast<unsigned long long>(c.type),static_cast<unsigned long long>(c.outer),c.slot.serial,c.classSlot.serial,schemaOk?"yes":"no");
        std::printf("NativeClassDefaultObjectMatch=%s RawObjectFlags=%08X ZeroSerialIsUnassigned=%s\n",c.defaults==c.object?"yes":"no",c.flags,(!c.slot.serial||!c.classSlot.serial)?"yes":"no");
        auto outer=c.outer;std::array<std::uintptr_t,8> visited{};unsigned depth=0;
        for(;outer&&depth<visited.size();++depth){
            bool duplicate=false;for(unsigned i=0;i<depth;++i)if(visited[i]==outer)duplicate=true;
            if(duplicate){std::puts("OuterChainRefused=cycle");break;}visited[depth]=outer;
            Candidate link;cfgnames::Text label,typeName;if(!Identity(r,registry,outer,link)||!names.Object(outer,label)||!names.Object(link.type,typeName)){std::puts("OuterChainRefused=unreadable");break;}
            links.push_back(link);std::printf(" Outer[%u]=0x%llX Serial=%d ClassSerial=%d Name=",depth,static_cast<unsigned long long>(outer),link.slot.serial,link.classSlot.serial);Name(label);std::printf(" Class=");Name(typeName);std::putchar('\n');outer=link.outer;
        }
        if(outer&&depth==visited.size())std::puts("OuterChainTruncated=yes");
        for(const auto name:{u"Construct",u"OnClicked_Start"})captureFunction(c.type,name);
        cfgnames::Text className;if(names.Object(c.type,className)){
            auto label=className.View();if(label.size()>2&&label.substr(label.size()-2)==u"_C"){
                std::u16string ubergraph=u"ExecuteUbergraph_";ubergraph.append(label.data(),label.size()-2);
                if(!captureFunction(c.type,ubergraph))std::puts("UbergraphCaptureRefused=missing_or_unreadable_or_limit");
            }
        }
        std::uintptr_t donor=0;cfgreflection::Observation<Reader> reflection(r,base);
        if(reflection.ReadObjectMember(c.object,c.type,u"DifficultyComboBox",donor)&&donor){
            Candidate link;if(Identity(r,registry,donor,link)){
                links.push_back(link);std::printf("Donor=0x%llX Class=0x%llX Outer=0x%llX Serial=%d ClassName=",static_cast<unsigned long long>(donor),static_cast<unsigned long long>(link.type),static_cast<unsigned long long>(link.outer),link.slot.serial);
                cfgnames::Text donorName;if(names.Object(link.type,donorName))Name(donorName);std::putchar('\n');
                if(!captureFunction(link.type,u"SetOptions"))std::puts("SetOptionsCaptureRefused=yes");
                // Read the same native donor panel/label path used in the base
                // implementation. Capture schema only; no calls or reparenting.
                const auto related=[&](std::uintptr_t object,std::uintptr_t type,std::u16string_view member,const char* label){
                    std::uintptr_t value=0;Candidate node;
                    if(!reflection.ReadObjectMember(object,type,member,value)||!value||!Identity(r,registry,value,node))return Candidate{};
                    links.push_back(node);std::printf("Donor%s=0x%llX Class=0x%llX Owner=0x%llX Serial=%d\n",label,static_cast<unsigned long long>(value),static_cast<unsigned long long>(node.type),static_cast<unsigned long long>(node.outer),node.slot.serial);return node;
                };
                const auto slot=related(donor,link.type,u"Slot","Slot");if(slot.object)related(slot.object,slot.type,u"Parent","Panel");
                related(donor,link.type,u"TitleText","TitleText");
                captureFunction(link.type,u"Construct");
                if(names.Object(link.type,donorName)){
                    auto label=donorName.View();if(label.size()>2&&label.substr(label.size()-2)==u"_C"){
                        std::u16string ubergraph=u"ExecuteUbergraph_";ubergraph.append(label.data(),label.size()-2);captureFunction(link.type,ubergraph);
                    }
                }
            }else std::puts("DonorCaptureRefused=identity_changed_or_unreadable");
        }
        if(schemaOk&&schema.stringArrayShape)std::printf("ItemsInner=0x%llX Outer=0x%llX Type=0x%llX ElementBytes=%d Offset=%d Flags=%016llX\n",static_cast<unsigned long long>(schema.options.inner.field),static_cast<unsigned long long>(schema.options.inner.owner),static_cast<unsigned long long>(schema.options.inner.type),schema.options.inner.bytes,schema.options.inner.offset,static_cast<unsigned long long>(schema.options.inner.flags));
    }
    struct CopyRecord {std::uintptr_t function=0;bool script=false,dependencies=false;cfgmenu::capture::OptionsEvidence evidence{};};
    std::vector<CopyRecord> copies;
    for(const auto& f:functions)if(f.function.label.View()==u"SetOptions"){
        CopyRecord copy;copy.function=f.function.object;cfgmenu::capture::OptionsOperands operands;
        copy.script=cfgmenu::capture::OptionsScript(f,operands);
        if(copy.script)copy.dependencies=cfgmenu::capture::OptionsDependencies(r,base,f,copy.evidence);copies.push_back(copy);
    }
    ArrayHeader finalRegistry;Context finalContext;
    if(!ReadContext(r,base,finalContext)||!(finalContext==context)||!ReadArrayHeader(r,base+ObjectArrayRva,finalRegistry)||!(finalRegistry==registry))return 7;
    for(const auto& c:found){Candidate final;if(!Identity(r,registry,c.object,final)||!Same(c,final))return 7;}
    for(const auto& c:links){Candidate final;if(!Identity(r,registry,c.object,final)||!Same(c,final))return 7;}
    for(const auto& cache:nativeCaches){std::uintptr_t type=0,function=0;
        if(!r.Read(cache.classCache,type)||type!=cache.type||!r.Read(cache.functionCache,function)||function!=cache.function)return 7;
    }
    for(const auto& old:functions){cfgmenu::capture::DiagnosticFunctionEvidence final;
        if(!cfgmenu::capture::Function(r,base,old.function.owner,old.function.label.View(),final)||final.function.object!=old.function.object||
           final.function.type!=old.function.type||!(final.slot==old.slot)||!(final.script==old.script)||final.bytes!=old.bytes||
           final.flags!=old.flags||final.parameterBytes!=old.parameterBytes||final.storageBytes!=old.storageBytes||
           final.nativeExecutor!=old.nativeExecutor||final.destructorLink!=old.destructorLink||final.fieldsCount!=old.fieldsCount)return 7;
        for(unsigned i=0;i<old.fieldsCount;++i)if(!cfgmenu::capture::SameProperty(final.fields[i],old.fields[i]))return 7;
    }
    for(const auto& copy:copies)if(copy.dependencies){
        for(const auto& f:functions)if(f.function.object==copy.function){cfgmenu::capture::OptionsEvidence final;
            if(!cfgmenu::capture::OptionsDependencies(r,base,f,final)||
               !cfgmenu::capture::SameProperty(final.destinationItems,copy.evidence.destinationItems)||
               !cfgmenu::capture::SameProperty(final.destinationSelected,copy.evidence.destinationSelected)||
               !cfgmenu::capture::SameProperty(final.destinationInner,copy.evidence.destinationInner)||
               !cfgmenu::capture::SameProperty(final.sourceInner,copy.evidence.sourceInner))return 7;
        }
    }
    if(!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    if(activeObserved&&!cfgmenu::active::Current(r,activeMenu,activeMenu.thread))return 7;
    if(mapObserved){cfgmenu::maps::Selection final;
        if(!cfgmenu::maps::Observe(r,activeMenu,activeMenu.thread,true,final)||final.registryClass!=mapSelection.registryClass||
            final.registryDefaults!=mapSelection.registryDefaults||final.map!=mapSelection.map||final.mode!=mapSelection.mode||
            final.settingsClass!=mapSelection.settingsClass||!(final.maps==mapSelection.maps)||final.count!=mapSelection.count||
            final.entries!=mapSelection.entries||final.modeDefaults!=mapSelection.modeDefaults||!(final.supported==mapSelection.supported))return 7;
    }
    if(profileObserved){cfgmenu::profiles::Selection final;
        if(!cfgmenu::profiles::ObserveSnapshot(r,activeMenu,activeMenu.thread,true,final)||final.save!=profileSelection.save||
            final.saveType!=profileSelection.saveType||final.defaults!=profileSelection.defaults||
            final.counselor!=profileSelection.counselor||final.jason!=profileSelection.jason||
            !(final.cache==profileSelection.cache)||!(final.identity==profileSelection.identity))return 7;
    }
    std::printf("CaptureSampleStable=yes DiscoveryComplete=%s DiscoveryReadFailures=%u CandidateCount=%zu ReadCalls=%u\n",discoveryComplete?"yes":"no",failures,found.size(),r.reads);
    for(const auto& f:functions){cfgmenu::startbinding::FastCall fast;
        if(!cfgmenu::startbinding::ReadFastCall(r,f.function.object,fast))return 7;
        std::printf("FunctionFastMetadata=0x%llX FastGraph=0x%llX FastEntry=%d\n",static_cast<unsigned long long>(f.function.object),
            static_cast<unsigned long long>(fast.graph),fast.entry);Function(f);}
    for(const auto& copy:copies){std::printf("OptionsCopyFunction=0x%llX ExactTwoAssignmentScript=%s CopyDependenciesStable=%s NativeCallApproved=no\n",static_cast<unsigned long long>(copy.function),copy.script?"yes":"no",copy.dependencies?"yes":"no");
        if(copy.dependencies){Property("DestinationItems",copy.evidence.destinationItems);Property("DestinationStringInner",copy.evidence.destinationInner);Property("ParameterStringInner",copy.evidence.sourceInner);Property("DestinationSelected",copy.evidence.destinationSelected);}
    }
    if(hudEvent&&!cfgmenu::hud::Current(r,hudBinding,activeMenu.thread))return 7;
    std::printf("ActiveTopOfStackOwnerObserved=%s ActiveWidget=0x%llX NativeThreadId=%u\n",activeObserved?"yes":"no",static_cast<unsigned long long>(activeMenu.widget),activeMenu.thread);
    if(activeObserved){cfgnames::Text label;if(!names.Object(activeMenu.type,label))return 7;std::printf("ActiveMenuClass=");Name(label);std::putchar('\n');}
    std::printf("ActiveNativeMapSelectionObserved=%s SelectedMap=0x%llX NativeMapCount=%u\n",mapObserved?"yes":"no",static_cast<unsigned long long>(mapSelection.map),mapSelection.count);
    std::printf("NativeMapObservationFailure=%s ReadCalls=%u CachePair=%u PartialValuesAreSelectionAuthority=no\n",
        cfgmenu::maps::Name(mapDiagnostic.failure),mapDiagnostic.reads,mapDiagnostic.cachePair);
    for(unsigned i=0;i<mapDiagnostic.caches.size();i+=2)std::printf(" MapNativeCachePair[%u] Reflection=0x%llX Constructor=0x%llX\n",
        i/2,static_cast<unsigned long long>(mapDiagnostic.caches[i]),static_cast<unsigned long long>(mapDiagnostic.caches[i+1]));
    std::printf(" MapPartialRegistry=0x%llX Defaults=0x%llX PendingMap=0x%llX PendingMode=0x%llX PendingSettings=0x%llX\n",
        static_cast<unsigned long long>(mapDiagnostic.registryClass),static_cast<unsigned long long>(mapDiagnostic.registryDefaults),
        static_cast<unsigned long long>(mapDiagnostic.map),static_cast<unsigned long long>(mapDiagnostic.mode),static_cast<unsigned long long>(mapDiagnostic.settings));
    std::printf(" ShippingHeaderRead=%s Data=0x%llX Count=%d Capacity=%d PolicyCountLimit=%u ReadCapacityEntries=no\n",
        mapDiagnostic.shippingRead?"yes":"no",static_cast<unsigned long long>(mapDiagnostic.shipping.data),
        mapDiagnostic.shipping.count,mapDiagnostic.shipping.capacity,cfgmenu::maps::MaximumMaps);
    std::printf(" SupportedHeaderRead=%s Data=0x%llX Count=%d Capacity=%d\n",mapDiagnostic.supportedRead?"yes":"no",
        static_cast<unsigned long long>(mapDiagnostic.supported.data),mapDiagnostic.supported.count,mapDiagnostic.supported.capacity);
    std::printf("ActiveHudDrawEventObserved=%s Hud=0x%llX Event=0x%llX LiveRecurrenceProven=no\n",hudEvent?"yes":"no",static_cast<unsigned long long>(activeMenu.hud),static_cast<unsigned long long>(hudEvent));
    std::printf("ExactHudFrameValidated=%s HudPreparationReads=%u\n",hudEvent?"yes":"no",hudReads);
    std::printf("OwnedProfileSelectionObserved=%s ProfilePreparationReads=%u Save=0x%llX Serial=%d CounselorClass=0x%llX JasonClass=0x%llX ProfileWritesApproved=no\n",
        profileObserved?"yes":"no",profileReads,static_cast<unsigned long long>(profileSelection.save),profileSelection.identity.serial,
        static_cast<unsigned long long>(profileSelection.counselor),static_cast<unsigned long long>(profileSelection.jason));
    std::printf("ProfileObservationFailure=%s PartialSave=0x%llX Serial=%d MatchingSaves=%u CacheCount=%d PartialValuesAreSelectionAuthority=no\n",
        profileDiagnostic.stage,static_cast<unsigned long long>(profileDiagnostic.save),profileDiagnostic.serial,
        profileDiagnostic.matches,profileDiagnostic.cache.count);
    std::puts("ProfileSaveLifetimePinned=no\nProfileRetainedCallAuthority=no\nFrontendExecutionApproved=no\nNativeCallsApproved=no\nZeroSerialMetadataIsUnassigned=yes\nNoWritesHooksLoadsOrNativeCalls=yes\nSnapshotIsNotAtomic=yes");
    return found.empty()?8:discoveryComplete?0:9;
}
}
int wmain(int argc,wchar_t** argv){
    if(argc==2&&!std::wcscmp(argv[1],L"--self-test")){
        Reader r(GetCurrentProcess());std::uintptr_t base=0,value=0;alignas(8) std::uintptr_t fixture=42;
        bool ok=!Module(GetCurrentProcess(),r,base)&&r.reads==0&&r.Read(reinterpret_cast<std::uintptr_t>(&fixture),value)&&value==42&&!r.Read(0,value);
        alignas(8) std::array<unsigned char,256> object{},type{};alignas(8) std::array<unsigned char,48> slots{};
        const auto objectAddress=reinterpret_cast<std::uintptr_t>(object.data()),classAddress=reinterpret_cast<std::uintptr_t>(type.data());
        std::memcpy(object.data()+0x10,&classAddress,8);std::memcpy(type.data()+cfgspawn::ClassDefaultObject,&objectAddress,8);
        const int classIndex=1;std::memcpy(type.data()+0xC,&classIndex,4);std::memcpy(slots.data(),&objectAddress,8);std::memcpy(slots.data()+24,&classAddress,8);
        cfgprobe::lifetime::ArrayHeader registry{reinterpret_cast<std::uintptr_t>(slots.data()),2};Candidate a,b;
        ok=ok&&Identity(r,registry,objectAddress,a)&&!a.slot.serial&&!a.classSlot.serial&&a.defaults==objectAddress&&Identity(r,registry,objectAddress,b)&&Same(a,b);
        object[8]=1;ok=ok&&Identity(r,registry,objectAddress,b)&&!Same(a,b);
        r.reads=750000;value=99;ok=ok&&!r.Read(reinterpret_cast<std::uintptr_t>(&fixture),value)&&value==99&&r.exhausted;
        std::printf("Frontend capture own-process refusal/read test %s; no game accessed\n",ok?"PASS":"FAIL");return ok?0:1;
    }
    const bool summaryOnly=argc==2&&!std::wcscmp(argv[1],L"--capture-summary");
    const bool startOnly=argc==2&&!std::wcscmp(argv[1],L"--capture-start");
    const bool cataloguesOnly=argc==2&&!std::wcscmp(argv[1],L"--capture-catalogues");
    const bool selectionsOnly=startOnly||(argc==2&&!std::wcscmp(argv[1],L"--capture-selections"));
    const bool currentOnly=cataloguesOnly||selectionsOnly||(argc==2&&!std::wcscmp(argv[1],L"--capture-current"));
    if(argc!=2||(!summaryOnly&&!currentOnly&&std::wcscmp(argv[1],L"--capture"))){std::puts("Use --capture-summary, --capture-catalogues, --capture-start, --capture-selections, --capture-current, --capture or --self-test. Fixed CFG path only; one-shot readonly diagnostic, never elevate.");return 1;}
    Handle file(CreateFileW(Path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
    if(!Hash(file.value)){std::puts("Exact CFG disk hash refused before process access");return 3;}
    const DWORD pid=Find();if(!pid){std::puts("Exactly one fixed-path CFG process required; no game launched");return 2;}
    Handle process(OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid));
    if(!process.value){std::puts("Normal read access refused; stop, no escalation");return 4;}
    Reader reader(process.value);std::uintptr_t base=0;if(!Module(process.value,reader,base))return 3;
    if(summaryOnly){const auto result=CaptureSummary(process.value,base,reader);std::printf("Result=%d ReadCalls=%u BudgetHit=%s\n",result,reader.reads,reader.exhausted?"yes":"no");return result;}
    std::puts(currentOnly?"READONLY CURRENT SETTINGS DIAGNOSTIC: owned menu only; no registry discovery. Not a UI install.":
        "READONLY MENU DIAGNOSTIC: bounded registry discovery; provisional records require final stable marker. Not a UI install.");
    const int result=Capture(process.value,base,reader,currentOnly,selectionsOnly,startOnly,cataloguesOnly);std::printf("Result=%d ReadCalls=%u BudgetHit=%s\n",result,reader.reads,reader.exhausted?"yes":"no");return result;
}
