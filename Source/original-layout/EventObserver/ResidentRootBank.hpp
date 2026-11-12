#pragma once
#include "ResidentRootRegistration.hpp"
#include "RootSlotLedger.hpp"
#include "RootPoisonDiagnostic.hpp"
#include <optional>
namespace cfgretain {
// Compatibility storage, NOT call or object-acquisition authority. All owners
// and ledgers must remain resident through process exit. The caller supplies
// verified current acquisitions; caches retain opaque handles, never a raw
// address masquerading as an old live acquisition. Not selected in runtime yet.
template<unsigned OwnerLimit=32>class ResidentRootBank {
    static_assert(OwnerLimit>0&&OwnerLimit<=32);
    using Ledger=RootSlotLedger<32>;
    struct Entry {SynchronizedRootOwner owner;std::optional<Ledger> ledger;cfggcprobe::Report report;};
    std::array<Entry,OwnerLimit> entries_{};
    unsigned used_=0;bool poisoned_=false;std::uintptr_t module_=0;std::uint64_t recoveryEpoch_=0;
    PoisonObserver poisonObserver_=nullptr;void* poisonContext_=nullptr;
    template<class Access>bool Grow(Access& access,std::uintptr_t module)noexcept{
        if(poisoned_||used_==OwnerLimit||!module||(module_&&module_!=module)||!access.ProvenSlotWindow())return false;
        auto& entry=entries_[used_++];module_=module;entry.owner.module=module;poisoned_=true;
        if(!RegisterResidentRootOwnerLocked(access,entry.owner,entry.report)){
            MarkUncertain({static_cast<unsigned>(PoisonSite::GrowRegistration),0,0,CFG_ROOT_POISON_PC});return false;}
        entry.ledger.emplace(entry.owner.native.slots);poisoned_=entry.ledger->Poisoned();return !poisoned_;
    }
public:
    using Handle=typename Ledger::Handle;
    struct Reservation {const void* bank=nullptr;unsigned group=OwnerLimit;typename Ledger::Reservation slots{};};
    unsigned Owners()const noexcept{return used_;}
    std::uintptr_t Address(unsigned i)const noexcept{return i<used_?reinterpret_cast<std::uintptr_t>(&entries_[i].owner.native):0;}
    bool Poisoned()const noexcept{return poisoned_;}
    std::uint64_t RecoveryEpoch()const noexcept{return recoveryEpoch_;}
    void ObservePoison(PoisonObserver observer,void* context)noexcept{poisonObserver_=observer;poisonContext_=context;}
    void MarkUncertain(PoisonEvidence evidence={static_cast<unsigned>(PoisonSite::Explicit),0,0,0})noexcept{
        if(poisonObserver_)poisonObserver_(poisonContext_,evidence);poisoned_=true;
    } // in-match authority remains quarantined; menu recovery is separate
    template<class Access,class Classify>bool RecoverMapReferencesAtMenu(Access& access,Classify classify,unsigned& released,
        std::uintptr_t* failedOwner=nullptr)noexcept{
        released=0;if(failedOwner)*failedOwner=0;
        if(!access.ProvenSlotWindow()||recoveryEpoch_==UINT64_MAX)return false;
        // Whole-bank preflight before touching slots. Unknown collector code,
        // incomplete callbacks, missing ledger and live output reservations are
        // STILL terminal. Poison alone is not an excuse to bypass these guards.
        for(unsigned i=0;i<used_;++i){auto& entry=entries_[i];if(failedOwner)*failedOwner=Address(i);
            if(!entry.ledger||!entry.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{
                return entry.ledger->CanRebuildAtMenu(classify);
            }))return false;
        }
        poisoned_=true; // a partial rebuild must never grant live-call authority
        for(unsigned i=0;i<used_;++i){auto& entry=entries_[i];unsigned count=0;if(failedOwner)*failedOwner=Address(i);
            if(!entry.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{
                return entry.ledger->RebuildAtMenu(classify,count);
            }))return false;
            released+=count;
        }
        poisoned_=false;
        if(!Healthy()){poisoned_=true;return false;}
        ++recoveryEpoch_;return true;
    }
    template<class Access,class Classify>bool RetireMapReferences(Access& access,Classify classify,unsigned& released,std::uintptr_t* failedOwner=nullptr)noexcept{
        released=0;if(failedOwner)*failedOwner=0;
        if(!Healthy()||!access.ProvenSlotWindow())return false;
        for(unsigned i=0;i<used_;++i){unsigned count=0;auto& entry=entries_[i];
            if(failedOwner)*failedOwner=Address(i);
            if(!entry.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{
                return entry.ledger->RetireMapReferences(classify,count);
            }))return false;
            released+=count;
        }
        return Healthy();
    }
    // Diagnostic snapshot only. Serialize with the collector on each resident
    // owner's LOCAL lock; do not refresh ledgers, acquire claims, register an
    // owner, or confer UObject/native-call authority. Opaque values must still
    // undergo fresh registry/Outer observation by the diagnostic caller.
    template<unsigned N>bool ObserveSlots(std::array<std::uintptr_t,N>& values,unsigned& count)noexcept{
        static_assert(N>=OwnerLimit*32);values={};count=0;
        if(!Healthy())return false;
        for(unsigned i=0;i<used_;++i){auto& e=entries_[i];
            if(!TryAcquireSRWLockShared(&e.owner.gate)){values={};count=0;return false;}
            bool current=false;
            __try{
                current=!e.owner.unknownSubmit.load()&&!e.owner.native.destroyRequests.load()&&
                    e.owner.native.state.load()==cfggcretain::State::Registered&&
                    e.owner.native.callbacks.load()==e.owner.native.completedCallbacks.load();
                if(current)for(unsigned j=0;j<32;++j)values[i*32+j]=e.owner.native.slots[j];
            }__finally{ReleaseSRWLockShared(&e.owner.gate);}
            if(!current){values={};count=0;return false;}
        }
        if(!Healthy()){values={};return false;}count=used_*32;return true;
    }
    bool Healthy()const noexcept{
        if(poisoned_)return false;
        for(unsigned i=0;i<used_;++i){const auto& e=entries_[i];
            if(!e.ledger||e.ledger->Poisoned()||e.owner.unknownSubmit.load()||e.owner.native.destroyRequests.load()||
               e.owner.native.state.load()!=cfggcretain::State::Registered||
               e.owner.native.callbacks.load()!=e.owner.native.completedCallbacks.load())return false;
        }
        return true;
    }
    template<class Access>bool ReserveOutputs(Access& access,std::uintptr_t module,unsigned maximum,Reservation& out)noexcept{
        out={};if(!maximum||maximum>32||!Healthy()||!access.ProvenSlotWindow()||(module_&&module_!=module))return false;
        for(unsigned group=0;group<used_;++group){auto& e=entries_[group];typename Ledger::Reservation candidate;
            const bool reserved=e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Reserve(maximum,candidate);});
            if(reserved){out={this,group,candidate};return true;}
            if(!Healthy()||!access.ProvenSlotWindow())return false;
        }
        if(!Grow(access,module))return false;
        auto& e=entries_[used_-1];typename Ledger::Reservation candidate;
        if(!e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Reserve(maximum,candidate);}))return false;
        out={this,used_-1,candidate};return true;
    }
    template<class Access,unsigned N>bool CommitFreshReturns(Access& access,Reservation& token,
        const std::array<std::uintptr_t,N>& acquired,std::array<Handle,N>& handles,unsigned actualCount=N)noexcept{
        handles={};if(token.bank!=this||token.group>=used_||!Healthy())return false;
        auto& e=entries_[token.group];
        const bool committed=e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Commit(token.slots,acquired,handles,actualCount);});
        if(committed)token={};
        return committed;
    }
    template<class Access>bool CancelUnpublished(Access& access,Reservation& token)noexcept{
        if(token.bank!=this||token.group>=used_||!Healthy())return false;auto& e=entries_[token.group];
        const bool canceled=e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Cancel(token.slots);});
        if(canceled)token={};return canceled;
    }
    template<class Access>bool ResolveRequired(Access& access,Handle handle,std::uintptr_t& object)noexcept{
        object=0;if(!Healthy())return false;
        for(unsigned group=0;group<used_;++group){auto& e=entries_[group];
            if(handle.owner==&*e.ledger)return e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Resolve(handle,object);});
        }
        return false;
    }
    template<class Access>bool Release(Access& access,Handle handle)noexcept{
        if(!Healthy())return false;
        for(unsigned group=0;group<used_;++group){auto& e=entries_[group];
            if(handle.owner==&*e.ledger)return e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->Release(handle);});
        }
        return false;
    }
    template<class Access>bool ReleaseAcquired(Access& access,Handle handle)noexcept{
        if(!Healthy())return false;
        for(unsigned group=0;group<used_;++group){auto& e=entries_[group];
            if(handle.owner==&*e.ledger)return e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{return e.ledger->ReleaseAcquired(handle);});
        }
        return false;
    }
    template<class Access,unsigned N>bool ReleaseAcquiredMany(Access& access,const std::array<Handle,N>& handles,unsigned count)noexcept{
        if(count>N||!Healthy()||!access.ProvenSlotWindow())return false;
        unsigned released=0;
        for(unsigned group=0;group<used_;++group){auto& e=entries_[group];bool needed=false;
            for(unsigned i=0;i<count;++i)needed|=handles[i].owner==&*e.ledger;
            if(!needed)continue;
            if(!e.owner.TryPublication(access,[&](cfggcretain::Owner&)noexcept{
                for(unsigned i=0;i<count;++i)if(handles[i].owner==&*e.ledger){
                    if(!e.ledger->ReleaseAcquired(handles[i]))return false;++released;
                }
                return true;
            }))return false;
        }
        return released==count;
    }
};
}
