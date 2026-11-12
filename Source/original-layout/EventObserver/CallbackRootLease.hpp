#pragma once
#include "ResidentRootBank.hpp"
namespace cfgretain {
// Lifetime of ONE already validated completed engine callback. The bank remains
// resident; claims do not. Callers must prove current object identities before
// acquisition. This supplies no tick/lookup/dispatch authority of its own.
template<class Bank,unsigned Limit=1024>class CallbackRootLease {
    Bank& bank_;
    std::array<typename Bank::Handle,Limit> handles_{};
    std::array<std::uintptr_t,Limit> values_{};
    typename Bank::Reservation outputs_{};
    unsigned count_=0,maximum_=0;bool entered_=false,prepared_=false,closed_=false;
    std::uintptr_t module_;
    // H2.92: native entries into this lease, and for each claim the entry count at which a locked window last resolved it.
    // A claim resolved after the most recent native entry cannot have been touched by the collector since (the collector
    // only runs on the game thread, and this callback has not returned to the engine or entered native code in between).
    unsigned nativeEntries_=0;std::array<unsigned,Limit> proven_{};
public:
    // Every non-null value is a claim of THIS lease, resolved by a locked window after the last native entry. Read-only.
    template<unsigned N>bool ProvenSinceLastNative(const std::array<std::uintptr_t,N>& values)const noexcept{
        if(!Current()||entered_)return false;bool any=false;
        for(const auto value:values)if(value){any=true;bool proven=false;
            for(unsigned i=0;i<count_&&!proven;++i)proven=values_[i]==value&&proven_[i]==nativeEntries_+1;
            if(!proven)return false;}
        return any;
    }
    class DeferredRetirement {
        friend class CallbackRootLease;
        Bank* bank_=nullptr;std::uintptr_t module_=0;std::uint64_t recoveryEpoch_=0;
        std::array<typename Bank::Handle,Limit> handles_{};unsigned count_=0;
    public:
        DeferredRetirement()=default;
        DeferredRetirement(const DeferredRetirement&)=delete;
        DeferredRetirement& operator=(const DeferredRetirement&)=delete;
        unsigned Claims()const noexcept{return count_;}
        // The menu rebuilt EVERY owner/slot and invalidated all old handles.
        // Drop only bookkeeping after independently verified recovery; no slot
        // writes or replay of a partially completed release occurs here.
        bool DiscardAfterMenuRecovery(Bank& bank,std::uintptr_t module)noexcept{
            if(!count_)return true;
            if(bank_!=&bank||module_!=module||!bank.Healthy()||bank.RecoveryEpoch()<=recoveryEpoch_)return false;
            count_=0;module_=0;bank_=nullptr;handles_={};return true;
        }
        // Opaque retirement only: no values/lookups/retention/native entry.
        // The caller must supply a NEW fully proved registry window.
        template<class Access>bool Retire(Access& access,Bank& expectedBank,std::uintptr_t module)noexcept{
            if(!count_||count_>Limit||bank_!=&expectedBank||!module_||module_!=module||
               !bank_->Healthy()||!access.ProvenSlotWindow())return false;
            if(!bank_->ReleaseAcquiredMany(access,handles_,count_)){
                bank_->MarkUncertain({static_cast<unsigned>(PoisonSite::DeferredRelease),count_,0,CFG_ROOT_POISON_PC});return false; // partial release MUST never replay
            }
            count_=0;module_=0;bank_=nullptr;handles_={};return true;
        }
    };
    CallbackRootLease(Bank& bank,std::uintptr_t module)noexcept:bank_(bank),module_(module){}
    CallbackRootLease(const CallbackRootLease&)=delete;
    unsigned Claims()const noexcept{return count_;}
    bool NativeEntered()const noexcept{return entered_;} // diagnostic observation only
    bool NativePending()const noexcept{return prepared_||entered_;}
    bool ZeroOutputCallEntered()const noexcept{return prepared_&&entered_&&!maximum_&&!outputs_.slots.id;}
    bool EmptyIdle()const noexcept{return !count_&&!prepared_&&!entered_;}
    bool TakeIdleClaims(DeferredRetirement& destination)noexcept{
        if(!Current()||!count_||prepared_||entered_||outputs_.slots.id||
           destination.count_||destination.bank_||destination.module_)return false;
        destination.bank_=&bank_;destination.module_=module_;destination.handles_=handles_;destination.count_=count_;
        destination.recoveryEpoch_=bank_.RecoveryEpoch();
        // Move bookkeeping only. Registered collector slots stay untouched.
        count_=0;handles_={};values_={};closed_=true;return true;
    }
    // Local abandonment only: no retained handles, output reservation or native
    // entry exists. Does not touch engine/collector storage or revive a bank.
    bool CloseEmpty()noexcept{
        if(!Current()||!EmptyIdle()||outputs_.slots.id)return false;
        closed_=true;return true;
    }
    bool Current()const noexcept{return !closed_&&module_&&bank_.Healthy();}
    void Uncertain(PoisonSite site=PoisonSite::Explicit,std::uintptr_t pc=0)noexcept{
        bank_.MarkUncertain({static_cast<unsigned>(site),count_,(prepared_?1u:0u)|(entered_?2u:0u),pc?pc:CFG_ROOT_POISON_PC});
    }
    template<class Access>bool RequiredRange(Access& access,unsigned start,unsigned count)noexcept{
        if(!Current()||start>count_||count>count_-start||count>32)return false;
        for(unsigned i=start;i<start+count;++i){std::uintptr_t value=0;
            if(!bank_.ResolveRequired(access,handles_[i],value)||value!=values_[i])return false;}
        return true;
    }
    template<class Access,unsigned N>bool Retain(Access& access,const std::array<std::uintptr_t,N>& values)noexcept{
        static_assert(N>0&&N<=32);
        if(!Current()||entered_)return false;
        std::array<std::uintptr_t,N> fresh{};unsigned used=0;
        for(const auto value:values)if(value){
            bool covered=false;
            for(unsigned i=0;i<count_;++i)if(values_[i]==value){std::uintptr_t current=0;
                if(!bank_.ResolveRequired(access,handles_[i],current)||current!=value)return false;
                proven_[i]=nativeEntries_+1;covered=true;break;
            }
            for(unsigned i=0;i<used;++i)covered|=fresh[i]==value;
            if(!covered)fresh[used++]=value;
        }
        if(!used)return true;
        if(used>Limit-count_)return false;
        typename Bank::Reservation reserve;std::array<typename Bank::Handle,N> acquired{};
        if(!bank_.ReserveOutputs(access,module_,used,reserve))return false;
        if(!bank_.CommitFreshReturns(access,reserve,fresh,acquired,used)){
            if(!bank_.CancelUnpublished(access,reserve))Uncertain(PoisonSite::RetainCommit);return false;
        }
        for(unsigned i=0;i<used;++i){handles_[count_]=acquired[i];proven_[count_]=nativeEntries_+1;values_[count_++]=fresh[i];}
        return true;
    }
    template<class Access>bool PrepareCall(Access& access,unsigned maximumOutputs)noexcept{
        if(!Current()||prepared_||entered_||maximumOutputs>32||maximumOutputs>Limit-count_)return false;
        if(maximumOutputs&&!bank_.ReserveOutputs(access,module_,maximumOutputs,outputs_))return false;
        maximum_=maximumOutputs;prepared_=true;return true;
    }
    template<class Access>bool EnterNative(Access& access)noexcept{
        if(!Current()||!prepared_||entered_||!access.NativeEntryUnlocked())return false;
        ++nativeEntries_;entered_=true;return true; // every earlier proof is stale from here on
    }
    template<class Access>bool CancelCall(Access& access)noexcept{
        if(!Current()||entered_)return false;
        if(outputs_.slots.id&&!bank_.CancelUnpublished(access,outputs_)){Uncertain(PoisonSite::CancelOutput);return false;}
        prepared_=false;maximum_=0;return true;
    }
    template<class Access,unsigned N>bool CompleteCall(Access& access,
        const std::array<std::uintptr_t,N>& values,unsigned actualCount=N)noexcept{
        static_assert(N<=32);
        if(!Current()||!entered_||actualCount>N||actualCount>maximum_){Uncertain(PoisonSite::CompleteState);return false;}
        std::array<typename Bank::Handle,N> acquired{};
        if(outputs_.slots.id&&!bank_.CommitFreshReturns(access,outputs_,values,acquired,actualCount)){
            Uncertain(PoisonSite::CompleteOutput);return false;
        }
        for(unsigned i=0;i<actualCount;++i)if(acquired[i].generation){
            bool duplicate=false;for(unsigned j=0;j<i;++j)duplicate|=acquired[j]==acquired[i];
            if(!duplicate){handles_[count_]=acquired[i];proven_[count_]=nativeEntries_+1;values_[count_++]=values[i];}
        }
        maximum_=0;prepared_=entered_=false;return true;
    }
    // A returned call with NO output reservation publishes no collector slots.
    // The originating callback must reprove its own witness before this local
    // bookkeeping transition. Claims stay rooted until ordinary locked Close.
    // Never use for an allocating call, an exception, or a different lease.
    bool CompleteReturnedWithoutOutputs()noexcept{
        if(!Current()||!prepared_||!entered_||maximum_||outputs_.slots.id)return false;
        prepared_=entered_=false;return true;
    }
    template<class Access>bool Close(Access& access)noexcept{
        if(!Current()||entered_){Uncertain(PoisonSite::CloseState);return false;}
        if(!CancelCall(access))return false;
        // Claims are released only within a renewed proved publication window.
        // Collector-cleared claims can be retired but never resurrected.
        if(!bank_.ReleaseAcquiredMany(access,handles_,count_)){Uncertain(PoisonSite::CloseRelease);return false;}
        closed_=true;count_=0;return true;
    }
};
}
