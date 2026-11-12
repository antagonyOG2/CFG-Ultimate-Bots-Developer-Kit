#pragma once
#include <atomic>
#include "../EventObserver/SpawnBindings.hpp"
namespace cfgspawn {
// One explicitly requested attempt in one pinned session. No retries, worker,
// actor scan, hotkey poll, CDO mutation or human possession operation.
class OneCounselorSpawn {
    std::atomic<unsigned> pending_{0}; //0not requested,1publishing,2queued,3consumed
    std::atomic<bool> ready_{false};
public:
    SpawnReport report{};
    bool Configure(Identity identity,bool resources,unsigned reads,std::uintptr_t source,
        std::uintptr_t pawnClass,std::uintptr_t tree) noexcept {
        report.identity=identity; report.preparationReads=reads;
        report.sourcePawn=source; report.pawnClass=pawnClass; report.tree=tree;
        const bool valid=identity.Valid() && resources && reads<=4096 &&
            cfgprobe::ObjectPointer(source) && source!=identity.pawn && cfgprobe::ObjectPointer(pawnClass) && cfgprobe::ObjectPointer(tree);
        report.state=static_cast<unsigned>(valid?SpawnState::Ready:SpawnState::RefusedPreparation);
        ready_.store(valid,std::memory_order_release); return valid;
    }
    bool Submit(const SpawnRequest& request) noexcept {
        if (!ready_.load(std::memory_order_acquire) || request.magic!=SpawnMagic || request.version!=SpawnVersion ||
            request.bytes!=sizeof(request) || request.reserved || !(request.identity==report.identity)) return false;
        unsigned expected=0;
        if (!pending_.compare_exchange_strong(expected,1,std::memory_order_acq_rel)) return false;
        pending_.store(2,std::memory_order_release); return true;
    }
    bool Pending() const noexcept { return pending_.load(std::memory_order_acquire)==2; }
    template<class Access> void Service(Access& access) noexcept {
        unsigned expected=2;
        if (!access.OwnerThread() || !pending_.compare_exchange_strong(expected,3,std::memory_order_acq_rel)) return;
        ready_.store(false,std::memory_order_release);
        access.Publish(report.generation,1); report.attempts=1;
        report.state=static_cast<unsigned>(SpawnState::RefusedContext);
        if (access.ContextMatches()) {
            report.state=static_cast<unsigned>(SpawnState::RefusedResources);
            if (access.ResourcesMatch() && access.RetainInputs()) {
                report.state=static_cast<unsigned>(SpawnState::LocationFailed);
                if (access.Location(report.location)) {
                    report.location.x+=180.0f; // engine collision handling remains enabled
                    SpawnAIParameters packet;
                    report.state=static_cast<unsigned>(SpawnState::PacketRejected);
                    if (BuildParameters(report.identity.world,report.pawnClass,report.tree,report.location,{},packet)) {
                        report.state=static_cast<unsigned>(SpawnState::PreSpawnChanged);
                        if (access.ContextMatches() && access.ResourcesMatch() && access.InputsRetained()) {
                            report.state=static_cast<unsigned>(SpawnState::SpawnReturnedNull);
                            if (access.Spawn(packet)) {
                                report.result=packet.result;
                                if (packet.result) {
                                    report.state=static_cast<unsigned>(SpawnState::ResultUnverified);
                                    if (access.InputsRetained() && packet.result!=report.identity.pawn && packet.result!=report.sourcePawn &&
                                        access.VerifyResult(packet.result,report.resultController))
                                        report.state=static_cast<unsigned>(SpawnState::LinkedNativeCounselor);
                                }
                            }
                        }
                    }
                }
            }
        }
        report.nativeCalls=access.Calls(); report.serviceReads=access.Reads(); report.error=access.Error();
        if (access.NativeException()) report.state=static_cast<unsigned>(SpawnState::NativeException);
        access.Publish(report.generation,2);
    }
};
} // namespace cfgspawn
