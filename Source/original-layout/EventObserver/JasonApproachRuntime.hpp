#include "../Core/J21cFeatureFlags.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include "../Core/J24SpawnRefusal.hpp"
#include "../Core/PrimaryStartupLeasePolicy.hpp"
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
class SpawnAccess;
void J24RecordWorldForRetirement(SpawnAccess&)noexcept;
void J24WorldInitialized()noexcept;
#endif
// Included only in the isolated owner-thread Jason approach prototype.
#ifdef CFG_RETAINED_PRIMARY_JASON
#include "PrimaryJasonRetentionRuntime.hpp"
#endif
template<class R>bool JasonLookupSafe(R& reader,std::uintptr_t module) {
#ifdef CFG_RETAINED_PRIMARY_JASON
    return cfgcompat::cfg35::gcRetention::PermanentOwnerCallbackObserved&&
        cfgprimary::LookupVetoClearForObservation(reader,module);
#else
    return cfgprimary::LookupSafe(reader,module);
#endif
}
cfgspawn::OneJasonApproach spawnRuntime;
struct JasonAssets {
    cfgspawn::ResourceReport resources{};
    std::uintptr_t gameState=0,killerBase=0;
    std::array<std::uintptr_t,4> classes{},functions{};
    std::array<cfgprobe::lifetime::SlotObservation,4> classSlots{},functionSlots{};
    std::array<cfgprobe::lifetime::SlotObservation,7> slots{};
    cfgprobe::lifetime::SlotObservation helperClassSlot{};
#ifdef CFG_PRIMARY_JASON
    cfgprimary::Report primary{};
    cfgspawn::TemplateObservation killerTemplate{};
    std::uintptr_t kismetClass=0,kismetDefault=0,conversionFunction=0;
    cfgprobe::lifetime::SlotObservation kismetClassSlot{},kismetDefaultSlot{},conversionSlot{};
#endif
} spawnAssets;
#ifdef CFG_RETAINED_PRIMARY_JASON
cfgjason::NewActorEvidence primaryNavigationEvidence{};
std::uint64_t primaryNavigationNext=0,primaryNavigationStarted=0;
unsigned primaryNavigationSamples=0,primaryNavigationMoves=0;
bool primaryNavigationStopped=false;
cfgcabin::Runtime primaryCabins;
cfgpursuit::Progress primaryProgress;
cfgpursuit::Ring primaryRing;
unsigned primaryCabinRootBatch=0,primaryRecoveryProjections=0,primaryRecoveryTeleports=0,primaryRecoveryFailure=0;
std::uint64_t primaryRecoveryNext=0,primaryMoveNext=0;
#endif
#ifdef CFG_RETAINED_PRIMARY_JASON
bool EnsurePrimaryRoots(Reader& reader)noexcept {
    const auto& r=spawnAssets.resources;
    const std::array<std::uintptr_t,25> input{plan.pawn,plan.world,plan.mode,plan.controller,spawnAssets.gameState,
        r.local.pawnClass,r.local.defaults,r.helperDefault,r.spawnFunction,
        r.classes[0],r.classes[1],r.classes[2],r.classes[3],r.classes[4],r.classes[5],r.classes[6],r.classes[7],
        spawnAssets.classes[0],spawnAssets.classes[1],spawnAssets.classes[2],spawnAssets.classes[3],
        spawnAssets.killerBase,spawnAssets.kismetClass,spawnAssets.kismetDefault,primaryRootTickFunction};
    return RetainPrimaryObjects(reader,input);
}
void ReportPrimaryRoots(cfgspawn::SpawnReport& output)noexcept {
    primaryRoots.Counters(output.retentionCalls,output.retentionReads,output.retentionCallbacks,output.retentionCompleted,
        output.retentionSubmissions,output.retentionCleared,output.retentionDestroyed);
    output.retentionOwners=primaryRoots.Count();output.retentionPoisoned=primaryRoots.Poisoned()?1u:0u;
    output.retentionChecks=primaryRootChecks;output.retentionPreserved=primaryRootPreserved;
    output.retentionStage=gcRegistryReport.stage;output.retentionResult=static_cast<unsigned>(gcRegistryReport.result);
    output.retentionFailure=gcRegistryReport.gateFailure;output.retentionReferences=0;
    for(unsigned i=0;i<primaryRoots.Count();++i){
        output.retentionOwnerAddresses[i]=primaryRoots.Address(i);output.retentionOwnerReferences[i]=primaryRoots.References(i);
        output.retentionOwnerFirsts[i]=primaryRoots.First(i);output.retentionReferences+=primaryRoots.References(i);
    }
    output.retentionOwner=primaryRoots.Address(0);
}
#endif
#ifdef CFG_PRIMARY_JASON
cfgkillerstart::Observation primaryStartObservation;
cfgkillerstart::Bootstrap primaryStartBootstrap;
std::uint64_t primaryStartAt=0,primaryStartNext=0;
class PrimaryStartHost {
    Reader& reader_;
public:
    explicit PrimaryStartHost(Reader& reader):reader_(reader){}
    bool Current(){
        cfgobserver::Plan current;
        return GetCurrentThreadId()==plan.threadId&&JasonLookupSafe(reader_,plan.module)&&
#ifdef CFG_RETAINED_PRIMARY_JASON
            PrimaryRootsCurrent(reader_)&&
#endif
            cfgprimary::RefreshKnownObjectStorage(reader_,plan,current);
    }
    bool Caches(std::uintptr_t& reflection,std::uintptr_t& constructor){
        return reader_.Read(plan.module+cfgkillerstart::disk::ClassCache,reflection)&&
            reader_.Read(plan.module+cfgkillerstart::disk::ConstructorCache,constructor)&&
            (!reflection||cfgprobe::ObjectPointer(reflection))&&(!constructor||cfgprobe::ObjectPointer(constructor));
    }
    bool InitializeClass(bool getter,std::uintptr_t& result)noexcept{
        result=0;if(!Current())return false;
        const auto entry=plan.module+(getter?cfgkillerstart::disk::ClassGetter:cfgkillerstart::disk::ClassFactory);
        MEMORY_BASIC_INFORMATION region{};const auto length=getter?162u:47u;
        if(VirtualQuery(reinterpret_cast<void*>(entry),&region,sizeof(region))!=sizeof(region)||region.State!=MEM_COMMIT||
            region.Type!=MEM_IMAGE||reinterpret_cast<std::uintptr_t>(region.AllocationBase)!=plan.module||
            (region.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
            !(region.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))||
            entry<reinterpret_cast<std::uintptr_t>(region.BaseAddress)||
            entry-reinterpret_cast<std::uintptr_t>(region.BaseAddress)>region.RegionSize||
            length>region.RegionSize-(entry-reinterpret_cast<std::uintptr_t>(region.BaseAddress)))return false;
        std::array<unsigned char,162> actualGetter{};std::array<unsigned char,47> actualFactory{};
        if(getter){if(!reader_.Read(entry,actualGetter)||actualGetter!=cfgkillerstart::disk::GetterCode)return false;}
        else if(!reader_.Read(entry,actualFactory)||actualFactory!=cfgkillerstart::disk::FactoryCode)return false;
        __try{result=reinterpret_cast<std::uintptr_t(*)()>(entry)();
            return cfgprobe::ObjectPointer(result)
#ifdef CFG_RETAINED_PRIMARY_JASON
                &&RetainPrimaryObjects(reader_,std::array<std::uintptr_t,1>{result})
#endif
                ;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
};
bool PrimaryFlagRegions() noexcept {
    for (const auto rva:{cfgprimary::SerializationFlag,cfgprimary::GarbageCollectionCounter}) {
        const auto at=plan.module+rva; MEMORY_BASIC_INFORMATION region{};
        if (VirtualQuery(reinterpret_cast<void*>(at),&region,sizeof(region))!=sizeof(region) ||
            region.State!=MEM_COMMIT || region.Type!=MEM_IMAGE ||
            reinterpret_cast<std::uintptr_t>(region.AllocationBase)!=plan.module ||
            (region.Protect&(PAGE_GUARD|PAGE_NOACCESS))) return false;
        const auto start=reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        if (at<start || at-start>region.RegionSize || sizeof(std::uint32_t)>region.RegionSize-(at-start)) return false;
    }
    return true;
}
#endif
const cfgspawn::TemplateObservation& JasonTemplate() {
#ifdef CFG_PRIMARY_JASON
    return spawnAssets.killerTemplate;
#else
    return spawnAssets.resources.local;
#endif
}
template<class R>bool J24SlotSame(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t object,
    const cfgprobe::lifetime::SlotObservation& captured,cfgj24::Refusal* why,cfgj24::ResourceCheck check,unsigned index=0){
    cfgprobe::lifetime::SlotObservation fresh{};const bool readable=cfgspawn::ObjectSlot(r,array,object,fresh);
    if(readable&&fresh==captured)return true;
    return why?why->FailSlot(check,index,captured,fresh,readable):false;
}
std::uint64_t j24WorldAssetResets=0;
template<class R> bool JasonStaticResources(R& r,const cfgprobe::lifetime::ArrayHeader& array,bool functions,cfgj24::Refusal* why=nullptr) {
    using C=cfgj24::ResourceCheck;
    const auto fail=[&](C check,unsigned index=0){return why?why->Fail(check,index):false;};
    const auto& resource=spawnAssets.resources;
    for (unsigned i=0;i<3;++i)
        if(!J24SlotSame(r,array,spawnAssets.slots[i].object,spawnAssets.slots[i],why,C::PawnSlot,i))return false;
    if(!spawnAssets.slots[5].HasAssignedSerial())return fail(C::StateSlot);
    if(!J24SlotSame(r,array,spawnAssets.slots[5].object,spawnAssets.slots[5],why,C::StateSlot))return false;
    if(!cfgobserver::Same(r,resource.counselor.pawn+0x3A0,resource.counselor.controller))return fail(C::PawnController);
    if(!cfgobserver::Same(r,resource.counselor.controller+0x370,resource.counselor.pawn))return fail(C::ControllerPawn);
    if(!cfgobserver::Same(r,resource.local.pawnClass+cfgspawn::ClassDefaultObject,resource.local.defaults))return fail(C::ClassDefault);
    if(!cfgobserver::Same(r,resource.local.defaults+cfgspawn::PawnAIClass,resource.local.defaultAI))return fail(C::DefaultAI);
    if(!cfgspawn::ObserveOwnedNativeHelper(r,plan.module,array,resource.classes[cfgspawn::Helper],resource.helperDefault,
        resource.spawnFunction,spawnAssets.helperClassSlot,spawnAssets.slots[3],spawnAssets.slots[4]))return fail(C::Helper);
    for(unsigned i=0;i<cfgspawn::Classes.size();++i){
        if(!cfgobserver::Same(r,plan.module+cfgspawn::Classes[i].reflection,resource.classes[i]))return fail(C::AICache,i);
        if(!cfgobserver::Same(r,plan.module+cfgspawn::Classes[i].constructor,resource.classes[i]))return fail(C::AIConstructor,i);
    }
    for (unsigned i=0;i<4;++i) {
        if(!cfgobserver::Same(r,plan.module+cfgjason::disk::Classes[i].reflection,spawnAssets.classes[i]))return fail(C::ClassCache,i);
        if(!cfgobserver::Same(r,plan.module+cfgjason::disk::Classes[i].constructor,spawnAssets.classes[i]))return fail(C::ClassConstructor,i);
        if(!J24SlotSame(r,array,spawnAssets.classes[i],spawnAssets.classSlots[i],why,C::ClassSlot,i))return false;
        if(functions){
            if(!cfgobserver::Same(r,plan.module+cfgjason::disk::Functions[i].cache,spawnAssets.functions[i]))return fail(C::FunctionCache,i);
            if(!J24SlotSame(r,array,spawnAssets.functions[i],spawnAssets.functionSlots[i],why,C::FunctionSlot,i))return false;
            if(!cfgobserver::Same(r,spawnAssets.functions[i]+0x20,i==3?resource.classes[cfgspawn::BaseAI]:spawnAssets.classes[i]))return fail(C::FunctionOuter,i);
        }
    }
    return cfgdispatch::Proof(r,plan.module)?true:fail(C::Dispatch);
}
void J24ResetSpawnAssets()noexcept{if(CFG_J24_SPAWN_RESET){spawnAssets={};++j24WorldAssetResets;}}
bool PrepareSpawnResources() noexcept {
    J24ResetSpawnAssets();
    Reader r; r.cap=4096;
#ifdef CFG_RETAINED_PRIMARY_JASON
    cfgobserver::Plan fresh{};
    if(!JasonLookupSafe(r,plan.module)||!cfgprimary::RefreshKnownObjectStorage(r,plan,fresh))return false;
    plan=fresh;
#endif
    spawnRuntime.report.preparationStep=1;
    std::uintptr_t counselor=0;
    bool valid=r.Read(plan.world+cfgprobe::WorldGameState,spawnAssets.gameState) &&
        r.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselor) &&
        r.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,spawnAssets.killerBase) &&
        cfgspawn::CaptureResources(r,plan.module,plan.objectArray,plan.pawn,
#ifdef CFG_PRIMARY_JASON
            1,
#else
            2,
#endif
            spawnAssets.gameState,counselor,spawnAssets.killerBase,spawnAssets.resources);
#ifdef CFG_PRIMARY_JASON
    std::uintptr_t sandbox=0;
    const auto modeFamily=
#ifdef CFG_FRONTEND_BUNDLE
        nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Ready?cfgprobe::OfflineBotsMode:
#endif
        cfgprobe::SandboxMode;
    valid=valid && r.Read(plan.module+cfgprobe::NativeClasses[modeFamily].reflectionCacheRva,sandbox) &&
        cfgprimary::Capture(r,plan.module,plan.objectArray,plan.mode,spawnAssets.gameState,sandbox,plan.pawn,counselor,
            spawnAssets.killerBase,spawnAssets.resources.classes[cfgspawn::PlayerState],spawnAssets.resources.classes[cfgspawn::Tree],spawnAssets.primary) &&
        spawnAssets.primary.EmptyPrimary() && spawnAssets.primary.catalogue.count>0 &&
#ifdef CFG_GAMEPLAY_BUNDLE
        // Configured bundle resolves ONE explicitly selected entry, so extra
        // CFG Jasons do not increase native call count. Keep the legacy search
        // prototype's18-conversion budget unchanged below.
        spawnAssets.primary.catalogue.count<=64;
#else
        spawnAssets.primary.catalogue.count<=18;
#endif
    // The immutable human counselor is the navigation target, never a bot or
    // possession candidate. The donor-slot machinery continues to guard it.
    spawnAssets.resources.counselor=spawnAssets.resources.local;
    spawnAssets.resources.counselor.controller=plan.controller;
#endif
    const auto& resource=spawnAssets.resources;
    spawnRuntime.report.preparationStep=2;
#ifdef CFG_PRIMARY_JASON
    valid=valid && resource.classMask==255 && resource.local.kind==1 && resource.local.aiKind==1 &&
        resource.local.defaultTree==0 && resource.local.instanceTree==0 && resource.counselor.pawn==plan.pawn;
    std::uintptr_t kismetReflection=0,kismetConstructor=0,kismetValue=0;
    std::array<unsigned char,47> conversionPrefix{};
    std::array<unsigned char,22> serialPrefix{};
    std::array<unsigned char,12> gcPrefix{};
    valid=valid && r.Read(plan.module+cfgprimary::disk::KismetReflection,kismetReflection) &&
        r.Read(plan.module+cfgprimary::disk::KismetConstructor,kismetConstructor) && kismetReflection && kismetReflection==kismetConstructor;
    spawnAssets.kismetClass=kismetReflection;
    valid=valid && cfgspawn::ObjectSlot(r,plan.objectArray,kismetReflection,spawnAssets.kismetClassSlot) &&
        r.Read(kismetReflection+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault) &&
        cfgspawn::ObjectSlot(r,plan.objectArray,spawnAssets.kismetDefault,spawnAssets.kismetDefaultSlot) &&
        r.Read(spawnAssets.kismetDefault+cfgprobe::ObjectClass,kismetValue) && kismetValue==kismetReflection &&
        r.Read(plan.module+cfgprimary::disk::Conversion.factory,conversionPrefix) && conversionPrefix==cfgprimary::disk::Conversion.prefix &&
        r.Read(plan.module+cfgprimary::SerializationGuardCode,serialPrefix) && serialPrefix==cfgprimary::SerializationGuardPrefix &&
        r.Read(plan.module+cfgprimary::GarbageCollectionGuardCode,gcPrefix) && gcPrefix==cfgprimary::GarbageCollectionGuardPrefix &&
        PrimaryFlagRegions() && JasonLookupSafe(r,plan.module);
#else
    valid=valid && resource.classMask==255 && resource.local.aiKind==3 && resource.local.defaultAI==resource.classes[cfgspawn::BaseAI] &&
        resource.local.defaultTree==0 && resource.local.instanceTree==0 && resource.counselor.pawn &&
        resource.counselor.controllerAIKind==1 && resource.counselor.brainTyped;
#endif
    const std::array<std::uintptr_t,7> objects{plan.pawn,resource.local.pawnClass,resource.local.defaults,
        resource.helperDefault,resource.spawnFunction,resource.counselor.pawn,0};
    for (unsigned i=0;valid && i<6;++i) {
        spawnRuntime.report.assetIndex=i;
        valid=
#ifdef CFG_PRIMARY_JASON
            cfgspawn::ObjectSlot(r,plan.objectArray,objects[i],spawnAssets.slots[i]) &&
#else
            cfgspawn::ReadSpawnAssetSlot(r,plan.objectArray,objects[i],i,spawnAssets.slots[i]) &&
#endif
            (i!=5 || spawnAssets.slots[i].HasAssignedSerial());
        spawnRuntime.report.assetSerials[i]=spawnAssets.slots[i].serial;
    }
    valid=valid && cfgspawn::ObjectSlot(r,plan.objectArray,resource.classes[cfgspawn::Helper],spawnAssets.helperClassSlot);
    spawnRuntime.report.preparationStep=3;
    for (unsigned i=0;valid && i<4;++i) {
        std::uintptr_t a=0,b=0;
        valid=r.Read(plan.module+cfgjason::disk::Classes[i].reflection,a) &&
            r.Read(plan.module+cfgjason::disk::Classes[i].constructor,b) && a && a==b &&
            cfgspawn::ObjectSlot(r,plan.objectArray,a,spawnAssets.classSlots[i]);
        spawnAssets.classes[i]=a;
        // Lazy reflected function initialization may allocate; defer ALL of it
        // to the one requested owner-thread service, before actor mutation.
        std::array<unsigned char,47> prefix{};
        valid=valid && r.Read(plan.module+cfgjason::disk::Functions[i].factory,prefix) && prefix==cfgjason::disk::Functions[i].prefix;
    }
    if (valid) spawnRuntime.report.preparationStep=4;
    valid=valid && JasonStaticResources(r,plan.objectArray,false) && cfgobserver::ContextUnchanged(r,plan);
    if (valid) spawnRuntime.report.preparationStep=5;
    spawnRuntime.report.error=r.error;
    return spawnRuntime.Configure({plan.world,plan.pawn,plan.objects[0].serial,plan.objects[3].serial},valid,r.reads,
        resource.counselor.pawn,
#ifdef CFG_PRIMARY_JASON
        0);
#else
        resource.local.pawnClass);
#endif
}
class SpawnAccess {
    Reader r_; cfgobserver::Plan current_=plan; cfgjason::NewActorEvidence evidence_{};
    unsigned calls_=0,writes_=0; bool exception_=false,functionsReady_=false;
    cfgj24::Refusal refusal_{};unsigned refusalScope_=1;
#ifdef CFG_PRIMARY_JASON
    unsigned conversions_=0,catalogueIndex_=0xFFFFFFFF;
    bool explicitClassResolved_=false;
    bool budgetRefused_=false;
    cfgprobe::lifetime::ArrayHeader ownerContextHeader_{};
    cfgcrowd::Observation crowdBefore_{},crowdAfter_{};
    unsigned crowdCalls_=0;
    std::uintptr_t primarySpawned_=0;
#ifdef CFG_FRONTEND_BUNDLE
    bool nativeLifecycle_=NativeOfflineWorld();
    unsigned nativeCreationStage_=0;
    cfgnativejason::Fault nativeFault_{};
#endif
    bool PrimaryRegistry() {
        return JasonLookupSafe(r_,plan.module) &&
            cfgprimary::RegistryMatches(r_,current_.objectArray,spawnAssets.primary.stateSlot,
                primarySpawned_,plan.controller,spawnAssets.resources.classes[cfgspawn::PlayerState]);
    }
    bool PrimaryAssets(bool conversion=false) {
        using C=cfgj24::ResourceCheck;auto* why=CFG_J24_SPAWN_RESET?&refusal_:nullptr;
        const auto fail=[&](C check){return why?why->Fail(check):false;};
        if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::KismetReflection,spawnAssets.kismetClass))return fail(C::KismetCache);
        if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::KismetConstructor,spawnAssets.kismetClass))return fail(C::KismetConstructor);
        if(!J24SlotSame(r_,current_.objectArray,spawnAssets.kismetClass,spawnAssets.kismetClassSlot,why,C::KismetClassSlot))return false;
        if(!cfgobserver::Same(r_,spawnAssets.kismetClass+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault))return fail(C::KismetDefault);
        if(!J24SlotSame(r_,current_.objectArray,spawnAssets.kismetDefault,spawnAssets.kismetDefaultSlot,why,C::KismetDefaultSlot))return false;
        if(!cfgobserver::Same(r_,spawnAssets.kismetDefault+cfgprobe::ObjectClass,spawnAssets.kismetClass))return fail(C::KismetType);
        if(conversion){
            if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::Conversion.cache,spawnAssets.conversionFunction))return fail(C::ConversionCache);
            if(!J24SlotSame(r_,current_.objectArray,spawnAssets.conversionFunction,spawnAssets.conversionSlot,why,C::ConversionSlot))return false;
            if(!cfgobserver::Same(r_,spawnAssets.conversionFunction+0x20,spawnAssets.kismetClass))return fail(C::ResolvedOuter);
        }
        if (catalogueIndex_!=0xFFFFFFFF||explicitClassResolved_) {
            cfgspawn::TemplateObservation fresh{};
            const auto& t=JasonTemplate();
            bool selectedCurrent=false;
#ifdef CFG_GAMEPLAY_BUNDLE
            if(explicitClassResolved_)selectedCurrent=cfgselectedjason::Current(r_,plan.module,current_.objectArray,spawnAssets.killerBase,bundleConfiguration.selectedClass);
            else
#endif
                selectedCurrent=cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,catalogueIndex_);
            if(!selectedCurrent)return fail(C::SelectedEntry);
            if(!cfgprimary::ObserveResolvedTemplate(r_,current_.objectArray,t.pawnClass,spawnAssets.killerBase,
                spawnAssets.resources.classes[cfgspawn::BaseAI],fresh))return fail(C::SelectedTemplate);
            if(!(fresh.classSlot==t.classSlot))return why?why->FailSlot(C::SelectedClassSlot,0,t.classSlot,fresh.classSlot,true):false;
            if(!(fresh.defaultSlot==t.defaultSlot))return why?why->FailSlot(C::SelectedDefaultSlot,0,t.defaultSlot,fresh.defaultSlot,true):false;
            return true;
        }
        return true;
    }
#endif
    bool Refresh() {
        cfgprobe::lifetime::ArrayHeader fresh;
        if (!cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,fresh)) return false;
        current_.objectArray=fresh; return ContextMatches();
    }
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* params,bool actor=true) noexcept {
        if (calls_>=cfgspawn::MaximumNativeCalls || !OwnerThread() ||
            !JasonLookupSafe(r_,plan.module) || !originalActorDispatcher) return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{object,function}))return false;
#endif
        ++calls_;
        __try {
            const auto dispatch=actor?originalActorDispatcher:reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva);
            dispatch(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),params); return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { exception_=true; return false; }
    }
    bool Factory(unsigned i,std::uintptr_t& out) noexcept {
        using C=cfgj24::ResourceCheck;
        std::array<unsigned char,47> prefix{};
        if(i>=4)return refusal_.Fail(C::FactoryIndex,i);
        if(calls_>=cfgspawn::MaximumNativeCalls)return refusal_.Fail(C::CallCap,i);
        if(!OwnerThread())return refusal_.Fail(C::OwnerThread,i);
        if(!ContextMatches())return refusal_.Fail(C::Context,i);
        if(!ResourcesMatch())return false;
        if(!r_.Read(plan.module+cfgjason::disk::Functions[i].factory,prefix)||prefix!=cfgjason::disk::Functions[i].prefix)return refusal_.Fail(C::FactoryCode,i);
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!PrimaryRootsCurrent(r_))return refusal_.Fail(C::RootScope,i);
#endif
        ++calls_;
        __try {
            out=reinterpret_cast<std::uintptr_t(*)()>(plan.module+cfgjason::disk::Functions[i].factory)(); return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { exception_=true; return refusal_.Fail(C::NativeException,i); }
    }
    bool Pawn() {
        const auto& t=JasonTemplate();
        return cfgjason::NewPawn(r_,current_.objectArray,evidence_,plan,spawnRuntime.report.sourcePawn,t.pawnClass,t.defaults,spawnAssets.killerBase);
    }
public:
    SpawnAccess() { r_.cap=cfgspawn::MaximumServiceReads; } // one explicit attempt, never a recurring tick budget
    const cfgprobe::lifetime::ArrayHeader& ObjectArray()const noexcept{return current_.objectArray;}
    bool OwnerThread() const { return GetCurrentThreadId()==plan.threadId; }
    bool ReceiveTickEvent(std::uintptr_t function,std::uintptr_t returnPc) {
#ifndef CFG_PRIMARY_JASON
        (void)function;(void)returnPc;return false;
#else
        if (!OwnerThread() || !JasonLookupSafe(r_,plan.module) || !cfgprobe::ObjectPointer(function)) return false;
        if(!cfgtick::Observe(r_,plan.module,plan.pawn,function,returnPc))return false;
        if(!OwnerThread()||!JasonLookupSafe(r_,plan.module))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
        primaryRootTickFunction=function;primaryRootTickReturn=returnPc;primaryRootTickActor=plan.pawn;
#endif
        return true;
#endif
    }
#ifdef CFG_PRIMARY_JASON
    std::uint64_t Now()const{return GetTickCount64();}
#endif
#ifdef CFG_PRIMARY_JASON
    bool PrepareOwnerContext() {
        if (!OwnerThread() || ownerContextHeader_.Sane() || !cfgprimary::RefreshKnownObjectStorage(r_,plan,current_)) return false;
        ownerContextHeader_=current_.objectArray; return true;
    }
#endif
    bool ContextMatches() {
        return OwnerThread() && cfgobserver::ContextUnchanged(r_,current_) &&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)
#ifdef CFG_PRIMARY_JASON
            && PrimaryRegistry()
#endif
            ;
    }
    bool ResourcesMatch() { refusal_.Phase(refusalScope_);return JasonStaticResources(r_,current_.objectArray,functionsReady_,CFG_J24_SPAWN_RESET?&refusal_:nullptr)
#ifdef CFG_PRIMARY_JASON
        && PrimaryAssets(spawnAssets.conversionFunction!=0)
#endif
        ; }
    bool ResolveFunctions() {
        using C=cfgj24::ResourceCheck;
        struct Restore{unsigned& phase;unsigned saved;~Restore(){phase=saved;}}restore{refusalScope_,refusalScope_};
        refusalScope_=2;refusal_.Phase(2);
        for (unsigned i=0;i<4;++i) {
            std::uintptr_t value=0;
            if(!r_.Read(plan.module+cfgjason::disk::Functions[i].cache,value))return refusal_.Fail(C::FunctionCacheRead,i);
            if(!value){if(!Factory(i,value))return refusal_.Fail(C::Factory,i);if(!Refresh())return refusal_.Fail(C::Refresh,i);}
            if(!cfgprobe::ObjectPointer(value))return refusal_.Fail(C::FunctionPointer,i);
            if(!cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[i].cache,value))return refusal_.Fail(C::ResolvedCache,i);
            if(!cfgspawn::ObjectSlot(r_,current_.objectArray,value,spawnAssets.functionSlots[i]))return refusal_.Fail(C::ResolvedSlot,i);
            if(!cfgobserver::Same(r_,value+0x20,i==3?spawnAssets.resources.classes[cfgspawn::BaseAI]:spawnAssets.classes[i]))return refusal_.Fail(C::ResolvedOuter,i);
            spawnAssets.functions[i]=value;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{value}))return refusal_.Fail(C::Retain,i);
#endif
        }
        functionsReady_=true;refusalScope_=3;refusal_.Phase(3);
        if(!ContextMatches())return refusal_.Fail(C::FinalContext);
        return ResourcesMatch();
    }
#ifdef CFG_PRIMARY_JASON
#include "SharedConversionRuntimeMembers.hpp"
    bool ResolvePawnClass(std::uintptr_t& result) {
        result=0;
#ifdef CFG_GAMEPLAY_BUNDLE
        if(bundleConfiguration.selectedClass.Valid()){
            const auto& selected=bundleConfiguration.selectedClass;
            cfgspawn::TemplateObservation candidate{};
            if(InterlockedCompareExchange(&bundleConfigurationState,2,2)!=2||!ContextMatches()||!ResourcesMatch()||
                !cfgbundle::MatchesConfiguration(bundleConfiguration,spawnRuntime.report.identity,spawnAssets.primary,cfgbundle::MaximumBots)||
                !cfgprimary::ObserveSelectedTemplate(r_,plan.module,current_.objectArray,selected,spawnAssets.killerBase,
                    spawnAssets.resources.classes[cfgspawn::BaseAI],candidate))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{candidate.pawnClass,candidate.defaults,candidate.defaultAI})||!Refresh()||
                !cfgselectedjason::Current(r_,plan.module,current_.objectArray,spawnAssets.killerBase,selected))return false;
#endif
            if(CFG_J21C_SHARED_CONVERTER&&(!PrepareSharedConversion()||!Refresh()||
                !cfgselectedjason::Current(r_,plan.module,current_.objectArray,spawnAssets.killerBase,selected)))return false;
            spawnAssets.killerTemplate=candidate;explicitClassResolved_=true;
            if(!ContextMatches()||!ResourcesMatch())return false;
            result=candidate.pawnClass;return true; // no conversion, lookup or invented index
        }
#endif
        if (!PrepareSharedConversion()) return false;
        const auto function=spawnAssets.conversionFunction;
        unsigned first=0,last=static_cast<unsigned>(spawnAssets.primary.catalogue.count);
#ifdef CFG_GAMEPLAY_BUNDLE
        if(InterlockedCompareExchange(&bundleConfigurationState,2,2)!=2||
           !cfgbundle::MatchesConfiguration(bundleConfiguration,spawnRuntime.report.identity,spawnAssets.primary,cfgbundle::MaximumBots))return false;
        first=bundleConfiguration.jasonCatalogueIndex;last=first+1;
#endif
        for (unsigned index=first;index<last;++index) {
            cfgprimary::ConversionParameters packet{};
            if (!ContextMatches() || !PrimaryAssets(true) || !cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,index) ||
                !cfgprimary::BuildConversion(spawnAssets.primary.entries[index],packet) ||
                !PrimaryFlagRegions() || !JasonLookupSafe(r_,plan.module)) return false;
            ++conversions_;
            if (!Invoke(spawnAssets.kismetDefault,function,&packet,false) || !Refresh() || !PrimaryAssets(true) ||
                !cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,index)) return false;
            if (!packet.result) continue; // Resolve-only; never an asset-loading fallback.
            cfgspawn::TemplateObservation candidate{};
            if (!cfgprimary::ObserveResolvedTemplate(r_,current_.objectArray,packet.result,spawnAssets.killerBase,
                spawnAssets.resources.classes[cfgspawn::BaseAI],candidate)) continue;
            spawnAssets.killerTemplate=candidate; catalogueIndex_=index; result=packet.result;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{candidate.pawnClass,candidate.defaults,candidate.defaultAI}))return false;
#endif
            return ContextMatches() && ResourcesMatch();
        }
        return false;
    }
    unsigned ClassConversions() const { return conversions_; }
    unsigned CatalogueIndex() const { return catalogueIndex_; }
    bool CompletionBudget() {
        budgetRefused_=!cfgprimary::CanComplete(r_.reads,r_.cap);
        return !budgetRefused_;
    }
    bool BudgetRefused() const { return budgetRefused_; }
    void NavigationReport(cfgspawn::SpawnReport& output) const {
        output.refusalPhase=refusal_.check?refusal_.phase:0;output.refusalCheck=refusal_.check;output.refusalIndex=refusal_.index;
        output.refusalSlotMask=refusal_.slotMask;output.refusalCapturedSerial=refusal_.capturedSerial;
        output.refusalFreshSerial=refusal_.freshSerial;output.worldAssetResets=j24WorldAssetResets;
#ifdef CFG_FRONTEND_BUNDLE
        output.nativeLifecycle=nativeLifecycle_?1u:0u;output.nativeCreationStage=nativeCreationStage_;
        output.exceptionCode=nativeFault_.code;output.exceptionAddress=nativeFault_.address;
#endif
        output.crowdKind=crowdBefore_.kind; output.crowdFallbackCalls=crowdCalls_;
        output.crowdStateBefore=crowdBefore_.state; output.crowdStateAfter=crowdAfter_.state;
        output.resultPawnSerial=evidence_.pawn.serial; output.resultControllerSerial=evidence_.killerController.serial;
        output.objectCountAtInstall=plan.objectArray.count; output.objectCountAtService=ownerContextHeader_.count;
        output.objectTableChanged=ownerContextHeader_.Sane() && !(plan.objectArray==ownerContextHeader_)?1u:0u;
    }
    bool ConfigureNavigation() {
        const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
        if (!cfgjason::LinkedKiller(r_,current_.objectArray,evidence_,plan,resource.counselor.pawn,t.pawnClass,
            t.defaults,spawnAssets.killerBase,resource.classes[cfgspawn::KillerAI],spawnAssets.classes[3],plan.controller) ||
            !cfgcrowd::Observe(r_,plan.module,current_.objectArray,evidence_.path,evidence_.killerController.object,crowdBefore_) ||
            !cfgcrowd::Idle(crowdBefore_)) return false;
        if (cfgcrowd::OrdinaryReady(crowdBefore_)) { crowdAfter_=crowdBefore_; return true; }
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{evidence_.path.object}))return false;
#endif
        std::array<unsigned char,cfgcrowd::SetterBytes.size()> code{};
        std::array<unsigned char,cfgcrowd::FollowBytes.size()> follow{};
        if (!r_.Read(plan.module+cfgcrowd::Setter,code) || code!=cfgcrowd::SetterBytes ||
            !r_.Read(plan.module+cfgcrowd::CrowdFollow,follow) || follow!=cfgcrowd::FollowBytes ||
            !ContextMatches() || !ResourcesMatch() || calls_>=cfgspawn::MaximumNativeCalls || !OwnerThread() ||
            !cfgcrowd::Observe(r_,plan.module,current_.objectArray,evidence_.path,evidence_.killerController.object,crowdAfter_) ||
            !cfgcrowd::Idle(crowdAfter_) || crowdAfter_.kind!=crowdBefore_.kind ||
            crowdAfter_.state!=crowdBefore_.state) return false;
        ++calls_; ++crowdCalls_;
        __try {
            reinterpret_cast<void(__fastcall*)(void*,std::uint8_t)>(plan.module+cfgcrowd::Setter)(
                reinterpret_cast<void*>(evidence_.path.object),cfgcrowd::DisabledState);
        } __except(EXCEPTION_EXECUTE_HANDLER) { exception_=true; return false; }
        return Refresh() && cfgcrowd::Observe(r_,plan.module,current_.objectArray,evidence_.path,
            evidence_.killerController.object,crowdAfter_) && cfgcrowd::Idle(crowdAfter_) &&
            cfgcrowd::OrdinaryReady(crowdAfter_) && crowdAfter_.kind==crowdBefore_.kind && ResourcesMatch();
    }
    bool PrimaryKiller(std::uintptr_t& value) {
        value=0;
        return r_.Read(spawnAssets.gameState+cfgprimary::CurrentKiller,value);
    }
#endif
    bool Location(cfgspawn::Vec3& out) { out={}; return Invoke(plan.pawn,spawnAssets.functions[2],&out) && cfgspawn::Finite(out); }
#ifdef CFG_PRIMARY_JASON
    bool StartTransform(cfgspawn::Vec3& position,cfgspawn::Vec3& rotation){
        position={};rotation={};PrimaryStartHost host(r_);cfgkillerstart::Marker marker{};
        std::uintptr_t root=0,scene=0,sceneCtor=0,function=0;
        cfgprobe::lifetime::SlotObservation rootSlot{},functionSlot{},fresh{};
        if(!ContextMatches()||!primaryStartObservation.CurrentMarker(r_,host,marker)||
           !r_.Read(marker.actor+cfgposition::disk::ActorRootComponent,root)||!cfgprobe::ObjectPointer(root)||
           !r_.Read(plan.module+cfgposition::disk::SceneClassCache,scene)||
           !r_.Read(plan.module+cfgposition::disk::SceneConstructorCache,sceneCtor)||
           !cfgprobe::MatchingClassCache(scene,sceneCtor)||!cfgspawn::ObjectIs(r_,root,scene)||
           !cfgspawn::ObjectSlot(r_,current_.objectArray,root,rootSlot)||
           !cfgobserver::Same(r_,root+0x20,marker.actor)||!cfgobserver::Same(r_,root+0xE0,marker.actor)||
           !cfgobserver::Same(r_,plan.module+cfgrotation::disk::ActorClass,spawnAssets.classes[2])||
           !cfgobserver::Same(r_,plan.module+cfgrotation::disk::ActorConstructor,spawnAssets.classes[2])||
           !r_.Read(plan.module+cfgrotation::disk::FunctionFactory,rotationFactoryCode_)||rotationFactoryCode_!=cfgrotation::disk::FactoryCode||
           !r_.Read(plan.module+cfgrotation::disk::Thunk,rotationThunkCode_)||rotationThunkCode_!=cfgrotation::disk::ThunkCode||
           !r_.Read(plan.module+cfgrotation::disk::FunctionCache,function))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,4>{marker.actor,marker.level,root,scene}))return false;
#endif
        if(!function){
            if(calls_>=cfgspawn::MaximumNativeCalls||!host.Current())return false;++calls_;
            if(!RotationFactory(function)||!Refresh())return false;
        }
        std::uint16_t parameterBytes=0;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{function}))return false;
#endif
        if(!cfgprobe::ObjectPointer(function)||!cfgspawn::ObjectSlot(r_,current_.objectArray,function,functionSlot)||
           !r_.Read(function+0x8E,parameterBytes)||parameterBytes!=12||
           !cfgobserver::Same(r_,plan.module+cfgrotation::disk::FunctionCache,function)||
           !cfgobserver::Same(r_,function+0x20,spawnAssets.classes[2])||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgrotation::disk::Thunk)||
           !primaryStartObservation.CurrentMarker(r_,host,marker)||
           !cfgobserver::Same(r_,marker.actor+cfgposition::disk::ActorRootComponent,root)||
           !cfgspawn::ObjectSlot(r_,current_.objectArray,root,fresh)||!(fresh==rootSlot)||
           !cfgobserver::Same(r_,root+0x20,marker.actor)||!cfgobserver::Same(r_,root+0xE0,marker.actor)||
           !Invoke(marker.actor,spawnAssets.functions[2],&position)||!cfgspawn::Finite(position)||
           !primaryStartObservation.CurrentMarker(r_,host,marker)||
           !cfgobserver::Same(r_,marker.actor+cfgposition::disk::ActorRootComponent,root)||
           !cfgspawn::ObjectSlot(r_,current_.objectArray,root,fresh)||!(fresh==rootSlot)||
           !cfgobserver::Same(r_,root+0x20,marker.actor)||!cfgobserver::Same(r_,root+0xE0,marker.actor)||
           !cfgspawn::ObjectSlot(r_,current_.objectArray,function,fresh)||!(fresh==functionSlot)||
           !Invoke(marker.actor,function,&rotation)||!cfgspawn::Finite(rotation)||
           !cfgobserver::Same(r_,marker.actor+cfgposition::disk::ActorRootComponent,root)||
           !cfgspawn::ObjectSlot(r_,current_.objectArray,root,fresh)||!(fresh==rootSlot)||
           !cfgobserver::Same(r_,root+0x20,marker.actor)||!cfgobserver::Same(r_,root+0xE0,marker.actor)||
           !host.Current())return false;
        spawnRuntime.report.startMarker=marker.actor;return true;
    }
private:
    std::array<unsigned char,cfgrotation::disk::FactoryCode.size()> rotationFactoryCode_{};
    std::array<unsigned char,cfgrotation::disk::ThunkCode.size()> rotationThunkCode_{};
    bool RotationFactory(std::uintptr_t& result)noexcept{
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!PrimaryRootsCurrent(r_))return false;
#endif
        __try{result=reinterpret_cast<std::uintptr_t(*)()>(plan.module+cfgrotation::disk::FunctionFactory)();return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
public:
#endif
    bool Spawn(cfgspawn::SpawnAIParameters& packet) {
#ifdef CFG_FRONTEND_BUNDLE
        if(nativeLifecycle_){
            nativeCreationStage_=1;
            if(calls_>=cfgspawn::MaximumNativeCalls||!OwnerThread()||!ContextMatches()||!ResourcesMatch()||
               packet.world!=plan.world||packet.pawnClass!=JasonTemplate().pawnClass||packet.behaviorTree||
               !cfgnativejason::SpawnCode(r_,plan.module))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{packet.world,packet.pawnClass}))return false;
#endif
            ++calls_;
            const bool returned=cfgnativejason::detail::Spawn(plan.module+cfgmenu::populationdisk::SpawnParametersConstructor,
                plan.module+cfgmenu::populationdisk::SpawnActor,plan.world,packet.pawnClass,&packet.location,&packet.rotation,
                packet.result,nativeFault_);
            if(!returned){exception_=nativeFault_.code!=0;return false;}
            if(packet.result&&packet.result!=plan.pawn)primarySpawned_=packet.result;
            return true;
        }
#endif
        const bool invoked=Invoke(spawnAssets.resources.helperDefault,spawnAssets.resources.spawnFunction,&packet,false);
#ifdef CFG_PRIMARY_JASON
        if (invoked && packet.result && packet.result!=plan.pawn) primarySpawned_=packet.result;
#endif
        return invoked;
    }
    bool VerifyInitial(std::uintptr_t pawn,std::uintptr_t& controller) {
        controller=0; const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
#ifdef CFG_FRONTEND_BUNDLE
        if(nativeLifecycle_){
            nativeCreationStage_=2;
            const bool valid=Refresh()&&cfgspawn::ObjectSlot(r_,current_.objectArray,pawn,evidence_.pawn)&&Pawn()&&
                cfgobserver::Same(r_,pawn+0x3A0,std::uintptr_t{0})&&
                cfgobserver::Same(r_,pawn+cfgspawn::PawnAIClass,t.defaultAI)&&ContextMatches();
#ifdef CFG_RETAINED_PRIMARY_JASON
            return valid&&RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{pawn});
#else
            return valid;
#endif
        }
#endif
        if (!Refresh() || !cfgspawn::ObjectSlot(r_,current_.objectArray,pawn,evidence_.pawn) || !Pawn() ||
            !r_.Read(pawn+0x3A0,controller) || controller==plan.controller || controller==resource.counselor.controller ||
            !cfgspawn::ObjectSlot(r_,current_.objectArray,controller,evidence_.initialController) ||
            !cfgjason::SameSlot(r_,current_.objectArray,evidence_.initialController) ||
            !cfgobserver::Same(r_,controller+cfgprobe::ObjectClass,t.defaultAI) ||
            !cfgobserver::Same(r_,controller+0x370,pawn) ||
            !cfgobserver::Same(r_,pawn+cfgspawn::PawnAIClass,t.defaultAI) || !Pawn() || !ContextMatches()) return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{pawn,controller}))return false;
#endif
        return true;
    }
    bool Unpossess(std::uintptr_t controller) {
#ifdef CFG_FRONTEND_BUNDLE
        // Donor native route never creates the helper's temporary base AI.
        if(nativeLifecycle_)return !controller&&VerifyUnpossessed();
#endif
#ifdef CFG_PRIMARY_JASON
        // Do not detach a controller whose killer player-state ownership we cannot preserve.
        if (!cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKillerState,std::uintptr_t{0})) return false;
#endif
        return controller==evidence_.initialController.object && Pawn() &&
            cfgjason::SameSlot(r_,current_.objectArray,evidence_.initialController) &&
            cfgobserver::Same(r_,controller+0x370,evidence_.pawn.object) &&
            cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,controller) && Invoke(controller,spawnAssets.functions[0],nullptr);
    }
    bool VerifyUnpossessed() {
        const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
#ifdef CFG_FRONTEND_BUNDLE
        if(nativeLifecycle_)return Refresh()&&!evidence_.initialController.object&&Pawn()&&
            cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,std::uintptr_t{0})&&
            cfgobserver::Same(r_,evidence_.pawn.object+cfgspawn::PawnAIClass,t.defaultAI)&&ContextMatches();
#endif
        return Refresh() && cfgjason::Detached(r_,current_.objectArray,evidence_,plan,resource.counselor.pawn,
            t.pawnClass,t.defaults,spawnAssets.killerBase,t.defaultAI,resource.counselor.controller) && ContextMatches();
    }
    bool WriteInstanceClass() {
        if (writes_ || !VerifyUnpossessed()) return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!PrimaryRootsCurrent(r_))return false;
#endif
        const auto at=evidence_.pawn.object+cfgspawn::PawnAIClass; MEMORY_BASIC_INFORMATION region{};
        if (VirtualQuery(reinterpret_cast<void*>(at),&region,sizeof(region))!=sizeof(region) || region.State!=MEM_COMMIT ||
            region.Type!=MEM_PRIVATE || region.Protect!=PAGE_READWRITE) return false;
        const auto start=reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        if (at<start || at-start>region.RegionSize || 8>region.RegionSize-(at-start) || !ContextMatches() || !ResourcesMatch() || !VerifyUnpossessed()) return false;
        ++writes_; SIZE_T written=0; const auto desired=spawnAssets.resources.classes[cfgspawn::KillerAI];
        if (!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&desired,sizeof(desired),&written)) { r_.error=GetLastError(); return false; }
        if (written!=sizeof(desired)) { r_.error=ERROR_PARTIAL_COPY; return false; }
        return cfgobserver::Same(r_,at,desired) && Pawn() && ContextMatches();
    }
    bool DefaultController() {
#ifdef CFG_FRONTEND_BUNDLE
        if(nativeLifecycle_){
            nativeCreationStage_=3;
            const auto klass=spawnAssets.resources.classes[cfgspawn::KillerAI];
            if(calls_+2>cfgspawn::MaximumNativeCalls||!OwnerThread()||!ContextMatches()||!ResourcesMatch()||!Pawn()||
               !cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,std::uintptr_t{0})||
               !cfgobserver::Same(r_,evidence_.pawn.object+cfgspawn::PawnAIClass,klass)||
               !cfgnativejason::SpawnCode(r_,plan.module)||!cfgnativejason::PossessCode(r_,plan.module))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{plan.world,klass,evidence_.pawn.object}))return false;
#endif
            std::uintptr_t controller=0;cfgspawn::Vec3 zero{};++calls_;
            if(!cfgnativejason::detail::Spawn(plan.module+cfgmenu::populationdisk::SpawnParametersConstructor,
                plan.module+cfgmenu::populationdisk::SpawnActor,plan.world,klass,&zero,&zero,controller,nativeFault_)){
                exception_=nativeFault_.code!=0;return false;
            }
            if(!controller||controller==plan.controller||!Refresh()||!Pawn()||
               !cfgspawn::ObjectSlot(r_,current_.objectArray,controller,evidence_.killerController)||
               !evidence_.killerController.HasAssignedSerial()||!cfgobserver::Same(r_,controller+cfgprobe::ObjectClass,klass)||
               !cfgobserver::Same(r_,controller+0x370,std::uintptr_t{0})||
               !cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,std::uintptr_t{0})||
               !ResourcesMatch()||!cfgnativejason::PossessCode(r_,plan.module))return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{controller,evidence_.pawn.object}))return false;
#endif
            nativeCreationStage_=4;++calls_;
            if(!cfgnativejason::detail::Possess(plan.module+cfgnativejason::disk::BasePossess,controller,evidence_.pawn.object,nativeFault_)){
                exception_=nativeFault_.code!=0;return false;
            }
            nativeCreationStage_=5;
            return Refresh()&&Pawn()&&cfgjason::SameSlot(r_,current_.objectArray,evidence_.killerController)&&
                cfgobserver::Same(r_,controller+0x370,evidence_.pawn.object)&&
                cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,controller);
        }
#endif
        return Pawn() && cfgobserver::Same(r_,evidence_.pawn.object+0x3A0,std::uintptr_t{0}) &&
            cfgobserver::Same(r_,evidence_.pawn.object+cfgspawn::PawnAIClass,spawnAssets.resources.classes[cfgspawn::KillerAI]) &&
            Invoke(evidence_.pawn.object,spawnAssets.functions[1],nullptr);
    }
    bool VerifyKiller(std::uintptr_t& controller,std::uintptr_t& path) {
        controller=0; path=0; const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
#ifdef CFG_FRONTEND_BUNDLE
        if(nativeLifecycle_)nativeCreationStage_=6;
#endif
        const bool linked=Refresh() && r_.Read(evidence_.pawn.object+0x3A0,controller) &&
            cfgspawn::ObjectSlot(r_,current_.objectArray,controller,evidence_.killerController) &&
            r_.Read(controller+cfgjason::disk::PathFollowing,path) &&
            cfgspawn::ObjectSlot(r_,current_.objectArray,path,evidence_.path) &&
            cfgjason::LinkedKiller(r_,current_.objectArray,evidence_,plan,resource.counselor.pawn,t.pawnClass,
                t.defaults,spawnAssets.killerBase,resource.classes[cfgspawn::KillerAI],spawnAssets.classes[3],resource.counselor.controller) && ContextMatches();
#ifdef CFG_RETAINED_PRIMARY_JASON
        return linked&&RetainPrimaryObjects(r_,std::array<std::uintptr_t,2>{controller,path});
#else
        return linked;
#endif
    }
    bool Move(cfgspawn::MoveParameters& packet) {
        const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
        return packet.goal==resource.counselor.pawn &&
            cfgjason::LinkedKiller(r_,current_.objectArray,evidence_,plan,resource.counselor.pawn,t.pawnClass,
                t.defaults,spawnAssets.killerBase,resource.classes[cfgspawn::KillerAI],spawnAssets.classes[3],resource.counselor.controller) &&
            Invoke(evidence_.killerController.object,spawnAssets.functions[3],&packet);
    }
    bool VerifyAfterMove() {
        const auto& resource=spawnAssets.resources; const auto& t=JasonTemplate();
        const bool valid=Refresh() &&
#ifdef CFG_RETAINED_PRIMARY_JASON
            PrimaryRootsCurrent(r_)&&
#endif
            cfgjason::LinkedKiller(r_,current_.objectArray,evidence_,plan,resource.counselor.pawn,
            t.pawnClass,t.defaults,spawnAssets.killerBase,resource.classes[cfgspawn::KillerAI],
            spawnAssets.classes[3],resource.counselor.controller) && ContextMatches();
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(valid)primaryNavigationEvidence=evidence_;
#endif
        return valid;
    }
#ifdef CFG_RETAINED_PRIMARY_JASON
    bool recoveryDidWork_=false;
    template<class B>bool RecoveryFunction(const B& binding,std::uintptr_t& function,std::uintptr_t& owner){
        function=owner=0;std::uintptr_t constructor=0;
        std::array<unsigned char,47> code{};cfgprobe::lifetime::SlotObservation slot{};
        if(!Scope()||!r_.Read(plan.module+binding.ownerReflection,owner)||
           !r_.Read(plan.module+binding.ownerConstructor,constructor)||!owner||owner!=constructor||
           !r_.Read(plan.module+binding.factory,code)||code!=binding.code||
           !RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{owner})||
           !r_.Read(plan.module+binding.cache,function))return false;
        if(!function){if(calls_>=cfgspawn::MaximumNativeCalls||!Scope())return false;++calls_;
            if(!RecoveryFactory(binding.factory,function)||!Refresh())return false;}
        std::uint16_t bytes=0;
        return cfgspawn::ObjectSlot(r_,current_.objectArray,function,slot)&&
            cfgobserver::Same(r_,function+0x20,owner)&&cfgobserver::Same(r_,plan.module+binding.cache,function)&&
            r_.Read(function+0x8E,bytes)&&bytes>=binding.nativeBytes&&bytes<=binding.storageBytes&&
            RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{function});
    }
    bool RecoveryFactory(std::uintptr_t factory,std::uintptr_t& function)noexcept{
        __try{function=reinterpret_cast<std::uintptr_t(*)()>(plan.module+factory)();return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    template<std::size_t N>bool RecoveryCode(std::uintptr_t at,const std::array<unsigned char,N>& expected){
        std::array<unsigned char,N> actual{};return r_.Read(plan.module+at,actual)&&actual==expected;
    }
    bool Scope(){return OwnerThread()&&Refresh()&&PrimaryRootsCurrent(r_)&&ResourcesMatch();}
    unsigned PrepareBounds(){
        std::uintptr_t owner=0,function=0;
        return RecoveryFunction(cfgcabin::disk::GetActorBounds,function,owner)&&owner==spawnAssets.classes[2]&&
            cfgobserver::Same(r_,function+0xB0,plan.module+cfgcabin::disk::Executor)&&
            RecoveryCode(cfgcabin::disk::Executor,cfgcabin::disk::ExecutorCode)&&
            RecoveryCode(cfgcabin::disk::Native,cfgcabin::disk::NativeCode)?1u:2u;
    }
    bool ReadBounds(std::uintptr_t actor,cfgcabin::Packet& packet){
        std::uintptr_t owner=0,function=0;
        return RecoveryFunction(cfgcabin::disk::GetActorBounds,function,owner)&&
            RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{actor})&&Invoke(actor,function,&packet)&&Scope();
    }
    bool RecoverNavigation(std::uint64_t now){
        // Reuse donor cabin exclusion and exterior-ring policy, not guessed
        // shack coordinates or unconditional relocation through collision.
        if(primaryCabins.cache.stage!=cfgcabin::Stage::Ready){
            recoveryDidWork_=true;
            if(primaryCabins.cache.stage==cfgcabin::Stage::Refused){primaryRecoveryFailure=1;return false;}
            if(primaryCabins.cache.stage==cfgcabin::Stage::Capture){
                const unsigned total=primaryCabins.cache.count*3;
                if(primaryCabinRootBatch<total){std::array<std::uintptr_t,32> roots{};
                    for(unsigned i=0;i<roots.size()&&primaryCabinRootBatch+i<total;++i){
                        const auto index=primaryCabinRootBatch+i;const auto& source=primaryCabins.sources[index/3];
                        if(!primaryCabins.Current(r_,current_.objectArray,source)){primaryRecoveryFailure=2;return false;}
                        roots[i]=index%3==0?source.actor:index%3==1?source.type:source.level;
                    }
                    if(!RetainPrimaryObjects(r_,roots)){primaryRecoveryFailure=3;return false;}
                    primaryCabinRootBatch+=32;return true;
                }
            }
            primaryCabins.Step(r_,current_.objectArray,plan.module,plan.world,*this);return true;
        }
        cfgspawn::Vec3 here{},there{};
        if(!Invoke(evidence_.pawn.object,spawnAssets.functions[2],&here)||!cfgspawn::Finite(here)||
           !Location(there)||!Scope())return false;
        if(primaryProgress.Due(now))primaryProgress.Observe(plan.pawn,plan.objects[3].serial,here,now);
        if(!primaryProgress.Stalled()||now<primaryRecoveryNext)return true;
        if(!primaryRing.target&&!primaryRing.Begin(plan.pawn,there,now))return true;
        if(!primaryRing.Current(plan.pawn,there)){primaryRing.Refused(now);return true;}
        std::uintptr_t function=0,owner=0;
        if(!primaryRing.projected){
            recoveryDidWork_=true;
            if(!RecoveryFunction(cfgvehicle::disk::Project,function,owner))return false;
            std::uintptr_t cdo=0;
            if(!r_.Read(owner+cfgspawn::ClassDefaultObject,cdo)||!cfgspawn::ObjectIs(r_,cdo,owner)||
               !RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{cdo}))return false;
            cfgvehicle::Projection projection;projection.world=plan.world;projection.point=primaryRing.Candidate();projection.extent={350,350,110};
            if(!Scope()||!Invoke(cdo,function,&projection,false)||!Scope())return false;
            ++primaryRecoveryProjections;
            if(projection.result!=1||!primaryRing.Projected(projection.projected))primaryRing.FailedCandidate(now);
            return true;
        }
        if(!primaryCabins.Allows(r_,plan.world,primaryRing.surface)){primaryRing.FailedCandidate(now);return true;}
        const cfgvehicle::disk::Binding teleport{cfgteleport::disk::FunctionFactory,cfgteleport::disk::FunctionCache,
            cfgteleport::disk::ActorClass,cfgteleport::disk::ActorConstructor,cfgteleport::disk::ParameterBytes,
            cfgteleport::disk::StorageBytes,cfgteleport::disk::FactoryCode};
        if(!RecoveryFunction(teleport,function,owner)||owner!=spawnAssets.classes[2]||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgteleport::disk::Thunk)||
           !RecoveryCode(cfgteleport::disk::Thunk,cfgteleport::disk::ThunkCode)||
           !RecoveryCode(cfgteleport::disk::Implementation,cfgteleport::disk::ImplementationCode)||!Scope())return false;
        cfgpursuit::TeleportPacket packet;packet.location=primaryRing.ActorDestination();
        recoveryDidWork_=true;
        // Spend cooldown before any ambiguous side effect; never replay it.
        primaryRecoveryNext=now+7000;
        if(!Invoke(evidence_.pawn.object,function,&packet))return false;
        if(!packet.result){primaryRing.FailedCandidate(now);return true;}
        primaryRing.Clear();primaryProgress.Clear();
        if(packet.result!=1||!Scope()||!Invoke(evidence_.pawn.object,spawnAssets.functions[2],&here)||!cfgspawn::Finite(here))return false;
        const auto dx=here.x-packet.location.x,dy=here.y-packet.location.y,dz=here.z-packet.location.z;
        if(dx*dx+dy*dy+dz*dz>500.f*500.f)return false;
        ++primaryRecoveryTeleports;return true;
    }
    bool ContinueNavigation() {
        r_.cap=8192;
        evidence_=primaryNavigationEvidence;functionsReady_=true;primarySpawned_=spawnRuntime.report.result;
        cfgcrowd::Observation follower{};cfgcrowd::MovementSample movement{};
        if(!Refresh()||!PrimaryRootsCurrent(r_)||!ResourcesMatch()||!VerifyAfterMove()||
           !cfgcrowd::Observe(r_,plan.module,current_.objectArray,evidence_.path,evidence_.killerController.object,follower)||
           !cfgcrowd::OrdinaryReady(follower)||
           !cfgcrowd::Movement(r_,plan.module,current_.objectArray,evidence_.pawn.object,movement))return false;
        ++primaryNavigationSamples;
        if(!RecoverNavigation(GetTickCount64()))return false;
        if(recoveryDidWork_)return true; // dedicated census/bounds/projection/teleport phase
        // Observe an accepted request on later ticks; do not continually abort
        // an active path. Recover an idle follower using the existing binding.
        const auto now=GetTickCount64();
        if(!cfgcrowd::Idle(follower)||now<primaryMoveNext)return true;
        primaryMoveNext=now+1000;
        cfgspawn::MoveParameters packet;packet.goal=spawnAssets.resources.counselor.pawn;
        if(!ContextMatches()||!ResourcesMatch()||!Move(packet)||!VerifyAfterMove())return false;
        ++primaryNavigationMoves;return packet.result<=2;
    }
#endif
    unsigned Calls() const { return calls_; } unsigned Reads() const { return r_.reads; }
    unsigned Writes() const { return writes_; } DWORD Error() const { return r_.error; }
    bool NativeException() const { return exception_; }
    void Publish(std::uint32_t& generation,std::uint32_t value) {
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!(value&1u))ReportPrimaryRoots(spawnRuntime.report);
#endif
        MemoryBarrier(); InterlockedExchange(reinterpret_cast<volatile LONG*>(&generation),static_cast<LONG>(value));
    }
};
void ServiceSpawn(void* object,void* function,bool wasNested,bool dispatchForwarded,std::uintptr_t returnPc) noexcept {
    const bool owner=GetCurrentThreadId()==plan.threadId;
#if defined(CFG_RETAINED_PRIMARY_JASON) && !defined(CFG_GAMEPLAY_BUNDLE)
    // The standalone compatibility test observes movement after birth. The
    // existing full bundle owns pursuit from this point; never run both loops.
    if(owner&&!wasNested&&dispatchForwarded&&reinterpret_cast<std::uintptr_t>(object)==plan.pawn&&
       !spawnRuntime.Pending()&&spawnRuntime.report.state==static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted)&&
       !primaryNavigationStopped){
        const auto now=GetTickCount64();if(now<primaryNavigationNext)return;
        SpawnAccess navigation;
        if(!navigation.ReceiveTickEvent(reinterpret_cast<std::uintptr_t>(function),returnPc))return;
        if(!primaryNavigationStarted)primaryNavigationStarted=now;
        if(now-primaryNavigationStarted>=180000||primaryNavigationSamples>=900){
            primaryNavigationStopped=true;
            navigation.Publish(spawnRuntime.report.generation,1);
            spawnRuntime.report.navigationStopped=1;
            navigation.Publish(spawnRuntime.report.generation,2);return;
        }
        primaryNavigationNext=now+200;
        const bool okay=navigation.ContinueNavigation();
        if(!okay){primaryNavigationStopped=true;if(!primaryRecoveryFailure)primaryRecoveryFailure=8;}
        navigation.Publish(spawnRuntime.report.generation,1);
        spawnRuntime.report.navigationSamples=primaryNavigationSamples;
        spawnRuntime.report.navigationMoves=primaryNavigationMoves;
        spawnRuntime.report.navigationStopped=primaryNavigationStopped?1u:0u;
        spawnRuntime.report.navigationReads=navigation.Reads();
        spawnRuntime.report.recoveryStage=static_cast<unsigned>(primaryCabins.cache.stage);
        spawnRuntime.report.recoveryFailure=primaryRecoveryFailure?primaryRecoveryFailure:primaryCabins.cache.failure;
        spawnRuntime.report.recoveryProjections=primaryRecoveryProjections;
        spawnRuntime.report.recoveryTeleports=primaryRecoveryTeleports;
        navigation.Publish(spawnRuntime.report.generation,2);
        return;
    }
#endif
    if (!cfgspawn::SpawnServiceCandidate(owner,wasNested,dispatchForwarded,spawnRuntime.Pending()) ||
        reinterpret_cast<std::uintptr_t>(object)!=plan.pawn) return;
    SpawnAccess access;
    if (!access.ReceiveTickEvent(reinterpret_cast<std::uintptr_t>(function),returnPc) ||
        !cfgspawn::SpawnServiceWindow(owner,wasNested,dispatchForwarded,true,spawnRuntime.Pending())) return;
#ifdef CFG_RETAINED_PRIMARY_JASON
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    // A hosted callback closes the one-time bootstrap. A subsequent offline
    // birth must use its OWN returned-tick lease, not reopen that bootstrap.
    // ServiceQueuedGameplay supplies that same verified capability below.
    if(!cfgstartuplease::CanRetain(primaryBootstrapRootsOpen,bundleRootLease!=nullptr))return;
#endif
    {Reader roots;roots.cap=cfgspawn::MaximumServiceReads;
     if(!EnsurePrimaryRoots(roots)){spawnRuntime.RefuseStartMarker(access);return;}}
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    if(CFG_J24_FAILED_START_RETIRE)J24RecordWorldForRetirement(access);
#endif
#endif
#ifdef CFG_PRIMARY_JASON
    // One cold, owned-level search only while this explicit startup is pending.
    // Normal gameplay never scans levels or actors for its start marker again.
    const auto now=GetTickCount64();
    if(primaryStartObservation.CurrentState()!=cfgkillerstart::State::Ready){
        if(now<primaryStartNext)return;primaryStartNext=now+25;
        Reader reader;reader.cap=cfgspawn::MaximumServiceReads;PrimaryStartHost host(reader);
        bool okay=false;
        try{
            if(primaryStartBootstrap.state!=cfgkillerstart::BootstrapState::Ready){
                const auto warm=primaryStartBootstrap.Step(host);
                InterlockedExchange(reinterpret_cast<volatile LONG*>(&spawnRuntime.report.generation),1);
                spawnRuntime.report.startClassWarmState=static_cast<unsigned>(warm);
                spawnRuntime.report.startClassWarmCalls=primaryStartBootstrap.calls;
                host.Caches(spawnRuntime.report.startClassCache,spawnRuntime.report.startConstructorCache);
#ifdef CFG_RETAINED_PRIMARY_JASON
                ReportPrimaryRoots(spawnRuntime.report);
#endif
                MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&spawnRuntime.report.generation),2);
                if(warm==cfgkillerstart::BootstrapState::Pending)return;
                if(warm==cfgkillerstart::BootstrapState::Refused){spawnRuntime.RefuseStartMarker(access);return;}
            }
#ifdef CFG_RETAINED_PRIMARY_JASON
            {std::uintptr_t reflection=0,constructor=0;
             if(!host.Caches(reflection,constructor)||!reflection||reflection!=constructor||
                !RetainPrimaryObjects(reader,std::array<std::uintptr_t,1>{reflection})){
                 spawnRuntime.RefuseStartMarker(access);return;}}
#endif
            if(!primaryStartAt){primaryStartAt=now;okay=primaryStartObservation.Begin(reader,host,plan.module,plan.world);}
            // Pawn callbacks can arrive only every ~270ms during startup. The
            // old60s/16-actor window expired mid-verification on a valid map.
            // Keep bounded slices, plus both wall-clock and total-work limits.
            else if(now-primaryStartAt<180000&&spawnRuntime.report.startMarkerSteps<4096)
                okay=primaryStartObservation.Step(reader,host,32);
        }catch(...){okay=false;}
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&spawnRuntime.report.generation),1);
        spawnRuntime.report.startMarkerState=static_cast<unsigned>(primaryStartObservation.CurrentState());
        spawnRuntime.report.startMarkerFailure=static_cast<unsigned>(primaryStartObservation.LastFailure());
        spawnRuntime.report.startMarkerRestarts=primaryStartObservation.Restarts();
        spawnRuntime.report.startMarkerAppendUpdates=primaryStartObservation.AppendUpdates();
        spawnRuntime.report.startMarkerChangedSource=primaryStartObservation.ChangedSource();
        spawnRuntime.report.startMarkerChangedLevel=primaryStartObservation.ChangedLevel();
        ++spawnRuntime.report.startMarkerSteps;
        spawnRuntime.report.startMarkerMaxReads=(std::max)(spawnRuntime.report.startMarkerMaxReads,reader.reads);
#ifdef CFG_RETAINED_PRIMARY_JASON
        ReportPrimaryRoots(spawnRuntime.report);
#endif
        MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&spawnRuntime.report.generation),2);
        if(!okay){spawnRuntime.RefuseStartMarker(access);return;}
        if(primaryStartObservation.CurrentState()!=cfgkillerstart::State::Ready)return;
    }
#endif
    spawnRuntime.Service(access);
}
