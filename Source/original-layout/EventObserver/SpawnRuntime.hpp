// Included inside the observer implementation's anonymous namespace, after its
// pinned plan, native forwarding target and bounded Reader definitions.
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
#include "GcRegistryProbeRuntime.hpp"
#endif
cfgspawn::OneCounselorSpawn spawnRuntime;
bool SpawnRuntimeLookupSafe(Reader& reader,std::uintptr_t module) noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
    // Read-only admission during preparation. Gameplay calls additionally
    // require the native tick owner, registered immutable roots and exact
    // context/resource checks below. Do not enable the shared lookup gate.
    return cfgcompat::cfg35::gcRetention::PermanentOwnerCallbackObserved&&
        cfgprimary::LookupVetoClearForObservation(reader,module);
#else
    return cfgprimary::LookupSafe(reader,module);
#endif
}
struct SpawnAssets {
    cfgspawn::ResourceReport resources{};
    std::uintptr_t gameState=0,tree=0,locationFunction=0,actorClass=0;
    std::array<cfgprobe::lifetime::SlotObservation,7> slots{};
    cfgprobe::lifetime::SlotObservation helperClassSlot{},actorClassSlot{};
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
    std::array<std::uintptr_t,cfggcretain::Capacity> roots{};
#endif
} spawnAssets;
bool SpawnContextCurrent(Reader& reader) noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
    cfgobserver::Plan fresh{};
    if(!cfgprimary::RefreshKnownObjectStorage(reader,plan,fresh))return false;
    plan=fresh;return true;
#else
    return cfgobserver::ContextUnchanged(reader,plan);
#endif
}
bool PrepareSpawnResources() noexcept {
    Reader r; r.cap=4096;
    spawnRuntime.report.preparationStep=1;
    if (!SpawnRuntimeLookupSafe(r,plan.module) || !SpawnContextCurrent(r)) {
        spawnRuntime.report.error=r.error;
        return false;
    }
    cfgspawn::ResourceProgress progress;
    std::uintptr_t counselor=0,killer=0;
    bool valid=r.Read(plan.world+cfgprobe::WorldGameState,spawnAssets.gameState) &&
        r.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselor) &&
        r.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,killer) &&
        cfgspawn::CaptureResources(r,plan.module,plan.objectArray,plan.pawn,2,spawnAssets.gameState,counselor,killer,spawnAssets.resources,&progress);
    const auto& resource=spawnAssets.resources;
    spawnRuntime.report.preparationStep=2;
    valid=valid && resource.classMask==255 && resource.counselor.pawn &&
        resource.counselor.aiKind==1 && resource.counselor.controllerAIKind==1 && resource.counselor.brainTyped &&
        resource.counselor.defaultAI==resource.classes[cfgspawn::CounselorAI] &&
        r.Read(plan.mode+cfgspawn::ModeCounselorTree,spawnAssets.tree) &&
        cfgspawn::ObjectIs(r,spawnAssets.tree,resource.classes[cfgspawn::Tree]) &&
        r.Read(plan.module+cfgspawn::LocationFunctionCache,spawnAssets.locationFunction) &&
        r.Read(plan.module+cfgspawn::ActorClassReflectionCache,spawnAssets.actorClass);
    std::uintptr_t owner=0;
    valid=valid && r.Read(spawnAssets.locationFunction+0x20,owner) && owner==spawnAssets.actorClass;
    const std::array<std::uintptr_t,7> objects{resource.counselor.pawn,resource.counselor.pawnClass,
        resource.counselor.defaults,resource.helperDefault,resource.spawnFunction,spawnAssets.tree,spawnAssets.locationFunction};
    if (valid) spawnRuntime.report.preparationStep=3;
    for (unsigned i=0;valid && i<objects.size();++i) {
        spawnRuntime.report.assetIndex=i;
        valid=cfgspawn::ReadSpawnAssetSlot(r,plan.objectArray,objects[i],i,spawnAssets.slots[i]);
        spawnRuntime.report.assetSerials[i]=spawnAssets.slots[i].serial;
    }
    valid=valid && cfgspawn::ObjectSlot(r,plan.objectArray,resource.classes[cfgspawn::Helper],spawnAssets.helperClassSlot) &&
        cfgspawn::ObserveOwnedNativeHelper(r,plan.module,plan.objectArray,resource.classes[cfgspawn::Helper],
            resource.helperDefault,resource.spawnFunction,spawnAssets.helperClassSlot,spawnAssets.slots[cfgspawn::HelperDefaultAssetIndex],spawnAssets.slots[4]) &&
        cfgspawn::ObjectSlot(r,plan.objectArray,spawnAssets.actorClass,spawnAssets.actorClassSlot) &&
        cfgspawn::ObserveModeTreeAndLocation(r,plan.module,plan.mode,plan.objectArray,spawnAssets.tree,resource.classes[cfgspawn::Tree],
            spawnAssets.locationFunction,spawnAssets.actorClass,spawnAssets.slots[5],spawnAssets.slots[6],spawnAssets.actorClassSlot);
    if (valid) spawnRuntime.report.preparationStep=4;
    valid=valid && cfgdispatch::Proof(r,plan.module) &&
        SpawnContextCurrent(r);
    if (valid) spawnRuntime.report.preparationStep=5;
    const cfgspawn::Identity identity{plan.world,plan.pawn,plan.objects[0].serial,plan.objects[3].serial};
    spawnRuntime.report.error=r.error;
    return spawnRuntime.Configure(identity,valid,r.reads,resource.counselor.pawn,resource.counselor.pawnClass,spawnAssets.tree);
}
class SpawnAccess {
    Reader reader_; unsigned calls_=0; bool exception_=false;
    std::uintptr_t tickFunction_=0,tickReturnPc_=0;
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* params,bool actor) noexcept {
        if (calls_>=2 || GetCurrentThreadId()!=plan.threadId || !originalActorDispatcher ||
            !SpawnRuntimeLookupSafe(reader_,plan.module) || !InputsRetained()) return false;
        ++calls_;
        __try {
            const auto dispatch=actor?originalActorDispatcher:reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva);
            dispatch(reinterpret_cast<void*>(object),reinterpret_cast<void*>(function),params);
            return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { exception_=true; return false; }
    }
public:
    SpawnAccess() noexcept { reader_.cap=cfgspawn::MaximumCounselorServiceReads; }
    bool OwnerThread() noexcept { return GetCurrentThreadId()==plan.threadId; }
    bool ReceiveTickEvent(std::uintptr_t function,std::uintptr_t returnPc) noexcept {
        if (!SpawnRuntimeLookupSafe(reader_,plan.module) || !OwnerThread() || !cfgprobe::ObjectPointer(function)) return false;
        if(!cfgtick::Observe(reader_,plan.module,plan.pawn,function,returnPc))return false;
        if(!SpawnRuntimeLookupSafe(reader_,plan.module)||!OwnerThread())return false;
        tickFunction_=function;tickReturnPc_=returnPc;return true;
    }
    bool ContextMatches() noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
        if(!SpawnRuntimeLookupSafe(reader_,plan.module)||!OwnerThread()||
           !SpawnContextCurrent(reader_))return false;
        return cfgobserver::Same(reader_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState);
#else
        return SpawnRuntimeLookupSafe(reader_,plan.module) && OwnerThread() && cfgobserver::ContextUnchanged(reader_,plan) &&
            cfgobserver::Same(reader_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState);
#endif
    }
    bool RetainInputs() noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
        if(!OwnerThread()||!tickFunction_||gcRegistryOwner.state.load()!=cfggcretain::State::Empty)return false;
        GcRegistryAccess registry;bool registered=false;
        __try {
            if(registry.Begin(tickFunction_,tickReturnPc_)&&ContextMatches()&&ResourcesMatch()){
                const auto& resource=spawnAssets.resources;
                const std::array<std::uintptr_t,21> objects{plan.pawn,plan.world,plan.mode,plan.controller,spawnAssets.gameState,
                    resource.counselor.pawn,resource.counselor.pawnClass,resource.counselor.defaults,resource.helperDefault,
                    resource.spawnFunction,spawnAssets.tree,spawnAssets.locationFunction,spawnAssets.actorClass,
                    resource.classes[0],resource.classes[1],resource.classes[2],resource.classes[3],
                    resource.classes[4],resource.classes[5],resource.classes[6],resource.classes[7]};
                bool valid=true;cfgprobe::lifetime::SlotObservation slot;
                for(const auto object:objects)if(!cfgspawn::ObjectSlot(reader_,plan.objectArray,object,slot)){valid=false;break;}
                if(valid){
                    cfggcprobe::RegisterPermanentInputsLocked(registry,gcRegistryOwner,gcRegistryReport,objects);
                    registered=gcRegistryReport.result==cfggcprobe::Result::Passed&&gcRegistryReport.permanent;
                    if(registered)spawnAssets.roots=gcRegistryOwner.expected; // application-immutable diagnostic copy
                }
            }
        } __finally {registry.End();}
        PublishRetention();
        return registered&&InputsRetained();
#else
        return false;
#endif
    }
    bool InputsRetained() noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
        auto& r=spawnRuntime.report;++r.retentionChecks;r.retentionPreserved=0;r.retentionFailure=1;
        if(!OwnerThread()||!SpawnRuntimeLookupSafe(reader_,plan.module)||!gcRegistryReport.permanent)return false;
        cfggcprobe::OwnerObservation observed;cfggcprobe::OwnerObservationFailure failure{};
        r.retentionFailure=2;
        if(!cfggcprobe::CaptureOwnerObservation(reader_,reinterpret_cast<std::uintptr_t>(&gcRegistryOwner),observed,failure))return false;
        r.retentionFailure=3;
        if(!observed.InputsPreserved(gcRegistryReport.references,plan.pawn)||
           observed.slots!=spawnAssets.roots||observed.expected!=spawnAssets.roots)return false;
        r.retentionFailure=4;
        if(!SpawnRuntimeLookupSafe(reader_,plan.module))return false;
        r.retentionPreserved=1;r.retentionFailure=0;return true;
#else
        return false;
#endif
    }
    void PublishRetention() noexcept {
#ifdef CFG_RETAINED_COUNSELOR_SPAWN
        auto& r=spawnRuntime.report;
        r.retentionCalls=gcRegistryReport.calls;r.retentionReads=gcRegistryReport.reads;r.retentionStage=gcRegistryReport.stage;
        r.retentionResult=static_cast<unsigned>(gcRegistryReport.result);r.retentionReferences=gcRegistryReport.references;
        r.retentionOwner=gcRegistryReport.owner;r.retentionCallbacks=gcRegistryOwner.callbacks.load();
        r.retentionCompleted=gcRegistryOwner.completedCallbacks.load();r.retentionSubmissions=gcRegistryOwner.submissions.load();
        r.retentionCleared=gcRegistryOwner.clearedSlots.load();r.retentionDestroyed=gcRegistryOwner.destroyRequests.load();
#endif
    }
    bool ResourcesMatch() noexcept {
        if (!SpawnRuntimeLookupSafe(reader_,plan.module)) return false;
        const auto& resource=spawnAssets.resources;
        cfgprobe::lifetime::SlotObservation fresh;
        for (unsigned i=0;i<cfgspawn::HelperDefaultAssetIndex;++i) {
            const auto& slot=spawnAssets.slots[i];
            if (!cfgspawn::ReadSpawnAssetSlot(reader_,plan.objectArray,slot.object,i,fresh) || !(fresh==slot)) return false;
        }
        if (!cfgspawn::ObserveOwnedNativeHelper(reader_,plan.module,plan.objectArray,resource.classes[cfgspawn::Helper],
            resource.helperDefault,resource.spawnFunction,spawnAssets.helperClassSlot,spawnAssets.slots[cfgspawn::HelperDefaultAssetIndex],spawnAssets.slots[4]) ||
            !cfgspawn::ObserveModeTreeAndLocation(reader_,plan.module,plan.mode,plan.objectArray,spawnAssets.tree,resource.classes[cfgspawn::Tree],
                spawnAssets.locationFunction,spawnAssets.actorClass,spawnAssets.slots[5],spawnAssets.slots[6],spawnAssets.actorClassSlot)) return false;
        if (!cfgobserver::Same(reader_,resource.counselor.pawn+cfgprobe::ObjectClass,resource.counselor.pawnClass) ||
            !cfgobserver::Same(reader_,resource.counselor.pawnClass+cfgspawn::ClassDefaultObject,resource.counselor.defaults) ||
            !cfgobserver::Same(reader_,resource.counselor.defaults+cfgspawn::PawnAIClass,resource.classes[cfgspawn::CounselorAI])) return false;
        for (unsigned i=0;i<cfgspawn::Classes.size();++i) {
            if (i==cfgspawn::Helper || i==cfgspawn::Tree) continue; // checked twice by ownership guards above
            if (!cfgobserver::Same(reader_,plan.module+cfgspawn::Classes[i].reflection,resource.classes[i]) ||
                !cfgobserver::Same(reader_,plan.module+cfgspawn::Classes[i].constructor,resource.classes[i])) return false;
        }
        return cfgdispatch::Proof(reader_,plan.module);
    }
    bool Location(cfgspawn::Vec3& out) noexcept {
        out={};
        return Invoke(plan.pawn,spawnAssets.locationFunction,&out,true) && cfgspawn::Finite(out);
    }
    bool Spawn(cfgspawn::SpawnAIParameters& packet) noexcept {
        return Invoke(spawnAssets.resources.helperDefault,spawnAssets.resources.spawnFunction,&packet,false);
    }
    bool VerifyResult(std::uintptr_t pawn,std::uintptr_t& controller) noexcept {
        controller=0; std::uintptr_t nativeCounselor=0;
        return ContextMatches() && InputsRetained() &&
            reader_.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,nativeCounselor) &&
            cfgspawn::ObserveCounselorSpawnResult(reader_,plan,spawnAssets.gameState,spawnAssets.resources.counselor.pawnClass,
                nativeCounselor,spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],pawn,controller);
    }
    unsigned Calls() const noexcept { return calls_; }
    unsigned Reads() const noexcept { return reader_.reads; }
    DWORD Error() const noexcept { return reader_.error; }
    bool NativeException() const noexcept { return exception_; }
    void Publish(std::uint32_t& generation,std::uint32_t value) noexcept {
        if(!(value&1u))PublishRetention();
        MemoryBarrier(); InterlockedExchange(reinterpret_cast<volatile LONG*>(&generation),static_cast<LONG>(value));
    }
};
void ServiceSpawn(void* object,void* function,bool wasNested,bool dispatchForwarded,std::uintptr_t returnPc) noexcept {
    const bool owner=GetCurrentThreadId()==plan.threadId;
    if (!cfgspawn::SpawnServiceCandidate(owner,wasNested,dispatchForwarded,spawnRuntime.Pending()) ||
        reinterpret_cast<std::uintptr_t>(object)!=plan.pawn) return;
    SpawnAccess access;
    if (!access.ReceiveTickEvent(reinterpret_cast<std::uintptr_t>(function),returnPc) ||
        !cfgspawn::SpawnServiceWindow(owner,wasNested,dispatchForwarded,true,spawnRuntime.Pending())) return;
    spawnRuntime.Service(access);
}
