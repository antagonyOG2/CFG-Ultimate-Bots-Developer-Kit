// Included inside the observer namespace; permanent storage in the pinned DLL.
#include "GcRegistryProbeRuntime.hpp"
cfgretain::PermanentRootPool<32> primaryRoots;
std::uintptr_t primaryRootTickFunction=0,primaryRootTickReturn=0,primaryRootTickActor=0;
unsigned primaryRootChecks=0,primaryRootPreserved=0;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
bool primaryBootstrapRootsOpen=true;
#include "BundleCallbackRootsRuntime.hpp"
#endif
template<class LookupReader>bool PrimaryRootsCurrent(LookupReader& reader)noexcept {
    ++primaryRootChecks;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(bundleRootLease){
        const bool valid=GetCurrentThreadId()==plan.threadId&&bundleRootLease->Current()&&
            cfgprimary::LookupVetoClearForObservation(reader,plan.module);
        primaryRootPreserved=valid?1u:0u;return valid;
    }
#endif
    const bool valid=GetCurrentThreadId()==plan.threadId&&
        cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&primaryRoots.Current(reader)&&
        cfgprimary::LookupVetoClearForObservation(reader,plan.module);
    primaryRootPreserved=valid?1u:0u;return valid;
}
template<std::size_t N>bool RetainPrimaryObjects(Reader& reader,const std::array<std::uintptr_t,N>& objects)noexcept {
    static_assert(N>0&&N<=cfggcretain::Capacity);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(bundleRootLease)return RetainBundleCallbackValues(reader,objects);
    if(!primaryBootstrapRootsOpen)return false;
    // Preserve the live-tested finite original-Jason bootstrap before the
    // completed-callback feature scope takes over. Historical bootstrap roots
    // are not consulted by the recurring feature lease.
#endif
    if(GetCurrentThreadId()!=plan.threadId||!primaryRootTickFunction||primaryRoots.Poisoned())return false;
    bool covered=primaryRoots.Count()!=0;
    for(const auto value:objects)if(value&&!primaryRoots.Contains(value))covered=false;
    if(covered)return PrimaryRootsCurrent(reader);
    if(primaryRoots.Count()&&!PrimaryRootsCurrent(reader))return false;
    auto contextKind=cfgprimary::RetentionContext::Human;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(primaryRootTickActor&&cfgprimary::ScopedLookupEligibility<Reader>::Current(reader,plan.module))
        contextKind=cfgprimary::RetentionContext::ScopedWorld;
    GcRegistryAccess registry(contextKind==cfgprimary::RetentionContext::ScopedWorld?primaryRootTickActor:0);
#else
    GcRegistryAccess registry;
#endif
    gcRegistryReport={};bool retained=false;
    __try {
        if(registry.Begin(primaryRootTickFunction,primaryRootTickReturn)){
            cfgobserver::Plan current{};cfgprobe::lifetime::SlotObservation slot{};
            bool valid=cfgprimary::RefreshRetentionContext(reader,plan,current,contextKind);
            for(const auto value:objects)if(value&&valid)valid=cfgspawn::ObjectSlot(reader,current.objectArray,value,slot);
            if(valid)retained=primaryRoots.AddLocked(registry,objects,gcRegistryReport);
        }
    } __finally {registry.End();}
    primaryRoots.RecordAccess(gcRegistryReport);
    return retained&&PrimaryRootsCurrent(reader);
}
