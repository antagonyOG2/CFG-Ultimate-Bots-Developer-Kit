#pragma once
#include "ObserverProtocol.hpp"
namespace cfgobserver {
inline const char* PreparationStageName(std::uint32_t stage) noexcept {
    static constexpr const char* names[]{"None","Input","Build","Thread","ModeClass","ControllerClass",
        "PawnClass","ContextBefore","Trampoline","ContextAfter","Capture","ModulePin","EmoteBindings","SpawnBindings"};
    static_assert(sizeof(names)/sizeof(names[0])==static_cast<unsigned>(PreparationStage::SpawnBindings)+1,"Stage names complete");
    return stage<sizeof(names)/sizeof(names[0])?names[stage]:"Unknown";
}
// Failure consumption is diagnostic ONLY. It must never authorize installation.
inline bool PreparationFailure(const Report& r) noexcept {
    const auto status=static_cast<Status>(r.status); const auto stage=static_cast<PreparationStage>(r.preparationStage);
    const bool category=(status==Status::RefusedPlan && (stage==PreparationStage::Input || stage==PreparationStage::Build)) ||
        (status==Status::RefusedIdentity && ((stage>=PreparationStage::Thread && stage<=PreparationStage::ContextBefore) ||
            (stage>=PreparationStage::ContextAfter && stage<=LastPreparationStage))) ||
        (status==Status::RefusedInstrumentation && stage==PreparationStage::Trampoline);
    const bool contextStage=stage==PreparationStage::ContextBefore||stage==PreparationStage::ContextAfter;
    return category && r.magic==Magic && r.version==Version && r.bytes==sizeof(Report) &&
        r.preparationReads<=
#ifdef CFG_FRONTEND_BUNDLE
        FrontendPreparationReadLimit
#else
        256
#endif
        && r.preparationContextFailure<=16 && (!r.preparationContextFailure||contextStage) &&
        !r.publishedCount && !r.hookInstalled && !r.stopReason && !r.threadId &&
        !r.pawn && !r.target && !r.trampoline && !r.relay && !r.detour && !r.dllPinned &&
        r.desiredPatch==std::array<unsigned char,5>{};
}
inline bool SamePreparationFailure(const Report& a,const Report& b) noexcept {
    return PreparationFailure(a) && PreparationFailure(b) && a.status==b.status &&
        a.preparationStage==b.preparationStage && a.preparationReads==b.preparationReads &&
        a.preparationError==b.preparationError && a.preparationContextFailure==b.preparationContextFailure &&
        a.hookStatus==b.hookStatus;
}
inline bool PublishedReport(const Report& r) noexcept {
    if (r.magic!=Magic || r.version!=Version || r.bytes!=sizeof(Report) ||
        r.publishedCount>cfgport::callbacks::Capacity || r.preparationStage || r.preparationError ||
        r.preparationContextFailure) return false;
    const auto status=static_cast<Status>(r.status);
    const auto reason=static_cast<cfgport::callbacks::Stop>(r.stopReason);
    if (status!=Status::Prepared && status!=Status::Capturing && status!=Status::Complete && status!=Status::Stopped) return false;
    if (!cfgprobe::ObjectPointer(r.pawn) || !r.threadId || !r.target || !r.trampoline || !r.relay || !r.detour) return false;
    if (status==Status::Prepared && (r.publishedCount || r.hookInstalled || reason!=cfgport::callbacks::Stop::None)) return false;
    if (status==Status::Capturing && (!r.publishedCount || !r.hookInstalled || reason!=cfgport::callbacks::Stop::None)) return false;
    if (status==Status::Complete && (r.publishedCount!=cfgport::callbacks::Capacity || !r.hookInstalled || reason!=cfgport::callbacks::Stop::Budget)) return false;
    if (status==Status::Stopped && (!r.hookInstalled ||
        (reason!=cfgport::callbacks::Stop::IdentityChanged && reason!=cfgport::callbacks::Stop::ClockChanged && reason!=cfgport::callbacks::Stop::Timeout))) return false;
    for (std::size_t i=0;i<r.publishedCount;++i) {
        const auto& e=r.events[i];
        if (e.threadId!=r.threadId || e.sequence!=i+1 || e.elapsedMs>cfgport::callbacks::MaximumElapsedMs ||
            (!i && e.elapsedMs) || (i && e.elapsedMs<r.events[i-1].elapsedMs)) return false;
    }
    return true;
}
// External RPM snapshots only: take two fixed Report reads. Never dereference
// CfgObserverReport() concurrently in process. Events are immutable after the
// count is published. Stop/status may lag; mismatched/incomplete metadata refuses
// consumption. This does not authorize game calls or prove native GC eligibility.
inline bool SamePublishedPrefix(const Report& a,const Report& b) noexcept {
    if (!PublishedReport(a) || !PublishedReport(b) || a.status!=b.status ||
        a.publishedCount!=b.publishedCount || a.stopReason!=b.stopReason || a.threadId!=b.threadId ||
        a.hookInstalled!=b.hookInstalled || a.pawn!=b.pawn || a.target!=b.target ||
        a.trampoline!=b.trampoline || a.relay!=b.relay || a.detour!=b.detour ||
        a.desiredPatch!=b.desiredPatch || a.preparationReads!=b.preparationReads || a.dllPinned!=b.dllPinned ||
        a.preparationContextFailure!=b.preparationContextFailure) return false;
    for (std::size_t i=0;i<a.publishedCount;++i) {
        const auto& x=a.events[i]; const auto& y=b.events[i];
        if (x.function!=y.function || x.parameters!=y.parameters || x.elapsedMs!=y.elapsedMs ||
            x.threadId!=y.threadId || x.sequence!=y.sequence) return false;
    }
    return true;
}
} // namespace cfgobserver
