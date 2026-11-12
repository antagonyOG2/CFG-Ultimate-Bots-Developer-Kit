#pragma once
#include <atomic>
#include "../EventObserver/SpawnBindings.hpp"
namespace cfgspawn {
struct MoveParameters {
    std::uintptr_t goal=0;
    float radius=125;
    std::uint8_t stopOnOverlap=1,usePathfinding=1,canStrafe=0,pad=0;
    std::uintptr_t filterClass=0;
    std::uint8_t allowPartial=1,result=255,padding[6]{};
};
static_assert(sizeof(MoveParameters)==32 && offsetof(MoveParameters,goal)==0 &&
    offsetof(MoveParameters,radius)==8 && offsetof(MoveParameters,stopOnOverlap)==12 &&
    offsetof(MoveParameters,usePathfinding)==13 && offsetof(MoveParameters,canStrafe)==14 &&
    offsetof(MoveParameters,filterClass)==16 && offsetof(MoveParameters,allowPartial)==24 &&
    offsetof(MoveParameters,result)==25,"CFG MoveToActor native frame");
inline bool BuildJasonPacket(std::uintptr_t world,std::uintptr_t pawnClass,Vec3 location,SpawnAIParameters& out) {
    // Distinct packet builder: null behavior tree is intentional ONLY here.
    // Do not weaken the proven counselor builder's tree requirement.
    if (!BuildParameters(world,pawnClass,pawnClass,location,{},out)) return false;
    out.behaviorTree=0; return true;
}
class OneJasonApproach {
    std::atomic<unsigned> pending_{0};
    std::atomic<bool> ready_{false};
#ifdef CFG_PRIMARY_JASON
    std::uint64_t retryAt_=0;
#endif
public:
    SpawnReport report{};
    bool EpochIdle()const noexcept{return !(report.generation&1u)&&pending_.load(std::memory_order_acquire)!=1;}
    // Owner-thread only, AFTER positive new frontend/world retirement proof.
    // A service in flight or partially queued external request blocks reset.
    bool ResetEpoch()noexcept{
        if(!EpochIdle())return false;ready_.store(false,std::memory_order_release);
        pending_.store(0,std::memory_order_release);report={};
#ifdef CFG_PRIMARY_JASON
        retryAt_=0;
#endif
        return true;
    }
    bool Configure(Identity identity,bool resources,unsigned reads,std::uintptr_t target,std::uintptr_t pawnClass) {
        report.identity=identity; report.preparationReads=reads; report.sourcePawn=target; report.pawnClass=pawnClass;
        const bool valid=identity.Valid() && resources && reads<=4096 && cfgprobe::ObjectPointer(target) &&
#ifdef CFG_PRIMARY_JASON
            target==identity.pawn && pawnClass==0;
#else
            target!=identity.pawn && cfgprobe::ObjectPointer(pawnClass);
#endif
        report.state=static_cast<unsigned>(valid?SpawnState::Ready:SpawnState::RefusedPreparation);
        ready_.store(valid,std::memory_order_release); return valid;
    }
    bool Submit(const SpawnRequest& request) {
        if (!ready_.load(std::memory_order_acquire) || request.magic!=SpawnMagic || request.version!=SpawnVersion ||
            request.bytes!=sizeof(request) || request.reserved || !(request.identity==report.identity)) return false;
        unsigned expected=0;
        if (!pending_.compare_exchange_strong(expected,1,std::memory_order_acq_rel)) return false;
        pending_.store(2,std::memory_order_release); return true;
    }
    bool Pending() const { return pending_.load(std::memory_order_acquire)==2; }
#ifdef CFG_PRIMARY_JASON
    template<class Access> void RefuseStartMarker(Access& a){
        if(!a.OwnerThread()||report.attempts)return;
        unsigned expected=2;
        if(!pending_.compare_exchange_strong(expected,3,std::memory_order_acq_rel))return;
        ready_.store(false,std::memory_order_release);a.Publish(report.generation,1);
        report.state=static_cast<unsigned>(SpawnState::StartMarkerFailed);
        report.error=a.Error();a.Publish(report.generation,2);
    }
#endif
    template<class Access> void Service(Access& a) {
        if(!a.OwnerThread())return;
#ifdef CFG_PRIMARY_JASON
        if(retryAt_&&a.Now()<retryAt_)return;
#endif
        unsigned expected=2;
        if (!a.OwnerThread() || !pending_.compare_exchange_strong(expected,3,std::memory_order_acq_rel)) return;
        ready_.store(false,std::memory_order_release);
        a.Publish(report.generation,1);
#ifdef CFG_PRIMARY_JASON
        ++report.attempts;report.retryPending=0;retryAt_=0;
#else
        report.attempts=1;
#endif
        Run(a);
#ifdef CFG_PRIMARY_JASON
        report.classConversions=a.ClassConversions(); report.catalogueIndex=a.CatalogueIndex();
        report.primaryKillerObserved=a.PrimaryKiller(report.primaryCurrentKiller)?1u:0u;
        report.budgetRefused=a.BudgetRefused()?1u:0u;
        a.NavigationReport(report);
#endif
        report.serviceReads=a.Reads(); report.nativeCalls=a.Calls(); report.error=a.Error(); report.instanceWrites=a.Writes();
        if (a.NativeException()) {
#ifdef CFG_PRIMARY_JASON
            report.exceptionStage=report.state;
#endif
            report.state=static_cast<unsigned>(SpawnState::NativeException);
        }
#ifdef CFG_PRIMARY_JASON
        // Only a completed native NULL return can try another position. No
        // exception, ambiguous call or nonnull/unverified actor is retried.
        if(report.state==static_cast<unsigned>(SpawnState::SpawnReturnedNull)&&
           report.nullSpawns==report.attempts&&report.attempts<MaximumPrimaryPlacements){
            report.retryPending=1;retryAt_=a.Now()+400;
            pending_.store(2,std::memory_order_release);
        }
#endif
        a.Publish(report.generation,2);
    }
private:
    template<class Access> void Run(Access& a) {
        report.state=static_cast<unsigned>(SpawnState::RefusedContext);
#ifdef CFG_PRIMARY_JASON
        if (!a.PrepareOwnerContext()) return;
#endif
        if (!a.ContextMatches()) return;
        report.state=static_cast<unsigned>(SpawnState::RefusedResources);
        if (!a.ResourcesMatch() || !a.ResolveFunctions()) return;
#ifdef CFG_PRIMARY_JASON
        report.state=static_cast<unsigned>(SpawnState::ClassResolutionFailed);
        if (!a.ResolvePawnClass(report.pawnClass)) return;
#endif
        report.state=static_cast<unsigned>(SpawnState::LocationFailed);
        if (!a.ContextMatches() ||
#ifdef CFG_PRIMARY_JASON
            !a.StartTransform(report.location,report.rotation)
#else
            !a.Location(report.location)
#endif
            ) return;
#ifdef CFG_PRIMARY_JASON
        // Same authored native start, including facing, on every bounded retry.
        // Never fall back to the human, a guessed shack origin or unchecked offset.
        report.placement=report.attempts-1;
#else
        report.location.x+=400;
#endif
        SpawnAIParameters packet;
        report.state=static_cast<unsigned>(SpawnState::PacketRejected);
        if (!BuildJasonPacket(report.identity.world,report.pawnClass,report.location,packet)) return;
#ifdef CFG_PRIMARY_JASON
        packet.rotation=report.rotation;
#endif
        report.state=static_cast<unsigned>(SpawnState::PreSpawnChanged);
        if (!a.ContextMatches() || !a.ResourcesMatch()) return;
#ifdef CFG_PRIMARY_JASON
        if (!a.CompletionBudget()) return;
#endif
        report.state=static_cast<unsigned>(SpawnState::SpawnReturnedNull);
        if (!a.Spawn(packet)) return;
        report.result=packet.result;
        if(!report.result){
#ifdef CFG_PRIMARY_JASON
            ++report.nullSpawns;
#endif
            return;
        }
        report.state=static_cast<unsigned>(SpawnState::ResultUnverified);
        if (report.result==report.identity.pawn || report.result==report.sourcePawn ||
            !a.VerifyInitial(report.result,report.initialController)) return;
        report.state=static_cast<unsigned>(SpawnState::UnpossessFailed);
        if (!a.ContextMatches() || !a.ResourcesMatch() || !a.Unpossess(report.initialController)) return;
        report.state=static_cast<unsigned>(SpawnState::UnpossessUnverified);
        if (!a.VerifyUnpossessed()) return;
        report.state=static_cast<unsigned>(SpawnState::ClassWriteRefused);
        if (!a.ContextMatches() || !a.ResourcesMatch()) return;
        if (!a.WriteInstanceClass()) {
            if (a.Writes()) report.state=static_cast<unsigned>(SpawnState::InstanceWriteIndeterminate);
            return;
        }
        report.state=static_cast<unsigned>(SpawnState::DefaultControllerFailed);
        if (!a.ContextMatches() || !a.ResourcesMatch() || !a.DefaultController()) return;
        report.state=static_cast<unsigned>(SpawnState::KillerControllerUnverified);
        if (!a.VerifyKiller(report.resultController,report.pathComponent)) return;
#ifdef CFG_PRIMARY_JASON
        report.state=static_cast<unsigned>(SpawnState::NavigationReadinessFailed);
        if (!a.ContextMatches() || !a.ResourcesMatch() || !a.ConfigureNavigation()) return;
#endif
        report.state=static_cast<unsigned>(SpawnState::MoveFailed);
        MoveParameters move; move.goal=report.sourcePawn;
        if (!a.ContextMatches() || !a.ResourcesMatch() || !a.Move(move)) return;
        report.moveResult=move.result;
        report.state=static_cast<unsigned>(SpawnState::MoveResultUnverified);
        if (!a.VerifyAfterMove()) return;
        report.state=static_cast<unsigned>(SpawnState::MoveFailed);
        if (move.result<=2
#ifndef CFG_PRIMARY_JASON
            &&move.result!=0
#endif
            ) report.state=static_cast<unsigned>(SpawnState::MoveSubmitted);
        // Primary registration/controller/path identity is already verified.
        // A completed first Failed-path result0 must not suppress helper startup:
        // the gameplay scheduler owns bounded retry/door/Morph recovery. An
        // exception or unknown native result still cannot start the bundle.
    }
};
} // namespace cfgspawn
