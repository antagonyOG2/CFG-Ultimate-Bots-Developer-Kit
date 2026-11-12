#pragma once
#include "ResidentRootBank.hpp"
namespace cfgretain {
// The existing native gateways own the real dispatch and its ABI. This small
// adapter owns only retention preparation/commit. The caller must end its engine
// publication lock BEFORE EnteredNative(), never hold it across native dispatch,
// and reestablish the proved window before CommitReturned()/ReleaseCompleted().
// Not selected by production gateways until the source/root bridge is complete.
template<class Bank>class RetainedNativeTransaction {
public:
    enum class State {Empty,Prepared,Entered,Returned,Complete,Refused,Indeterminate};
private:
    Bank& bank_;typename Bank::Reservation outputReserve_{};
    std::array<typename Bank::Handle,32> inputs_{},outputs_{};
    unsigned inputCount_=0,outputMaximum_=0,outputCount_=0;State state_=State::Empty;
    template<class Access>bool ReleaseInputs(Access& access)noexcept{
        bool okay=true;for(unsigned i=0;i<inputCount_;++i)if(inputs_[i].generation){
            for(unsigned j=i+1;j<inputCount_;++j)if(inputs_[j]==inputs_[i])inputs_[j]={};
            okay=bank_.Release(access,inputs_[i])&&okay;inputs_[i]={};}
        inputCount_=0;return okay;
    }
public:
    explicit RetainedNativeTransaction(Bank& bank)noexcept:bank_(bank){}
    RetainedNativeTransaction(const RetainedNativeTransaction&)=delete;
    RetainedNativeTransaction& operator=(const RetainedNativeTransaction&)=delete;
    State CurrentState()const noexcept{return state_;}
    template<class Access,unsigned N>bool Prepare(Access& access,std::uintptr_t module,
        const std::array<std::uintptr_t,N>& verifiedInputs,unsigned maximumOutputs,unsigned actualInputCount=N)noexcept{
        static_assert(N>0&&N<=32);
        if(state_!=State::Empty||maximumOutputs>32||!actualInputCount||actualInputCount>N)return false;
        state_=State::Refused;
        // Input values must already have the caller's exact current identity/
        // schema proof. This function cannot convert stale addresses into proof.
        typename Bank::Reservation reserve;std::array<typename Bank::Handle,N> acquired{};
        if(!bank_.ReserveOutputs(access,module,actualInputCount,reserve))return false;
        if(!bank_.CommitFreshReturns(access,reserve,verifiedInputs,acquired,actualInputCount)){
            (void)bank_.CancelUnpublished(access,reserve);return false;
        }
        for(unsigned i=0;i<actualInputCount;++i)inputs_[i]=acquired[i];inputCount_=actualInputCount;
        if(maximumOutputs&&!bank_.ReserveOutputs(access,module,maximumOutputs,outputReserve_)){
            if(!ReleaseInputs(access))state_=State::Indeterminate;return false;
        }
        outputMaximum_=maximumOutputs;state_=State::Prepared;return true;
    }
    template<class Access>bool EnteredNative(Access& access)noexcept{
        if(state_!=State::Prepared||!bank_.Healthy()||!access.NativeEntryUnlocked())return false;
        state_=State::Entered;return true;
    }
    template<class Access,unsigned N>bool CommitReturned(Access& access,
        const std::array<std::uintptr_t,N>& verifiedOutputs,unsigned actualCount=N)noexcept{
        static_assert(N<=32);
        if(state_!=State::Entered)return false;
        state_=State::Indeterminate;
        if(actualCount>N||actualCount>outputMaximum_){bank_.MarkUncertain();return false;}
        std::array<typename Bank::Handle,N> acquired{};
        if(outputReserve_.slots.id){
            if(!bank_.CommitFreshReturns(access,outputReserve_,verifiedOutputs,acquired,actualCount)){bank_.MarkUncertain();return false;}
        }
        for(unsigned i=0;i<actualCount;++i)outputs_[i]=acquired[i];outputCount_=actualCount;
        // An input can become pending-kill during the allocating call. Retain
        // outputs first, then refuse use of any invalidated required handle.
        for(unsigned i=0;i<inputCount_;++i)if(inputs_[i].generation){std::uintptr_t value=0;
            if(!bank_.ResolveRequired(access,inputs_[i],value)){bank_.MarkUncertain();return false;}}
        state_=State::Returned;return true;
    }
    template<class Access>bool CancelBeforeNative(Access& access)noexcept{
        if(state_!=State::Prepared)return false;state_=State::Refused;
        const bool canceled=!outputMaximum_||bank_.CancelUnpublished(access,outputReserve_);
        const bool released=ReleaseInputs(access);
        if(!canceled||!released)state_=State::Indeterminate;return canceled&&released;
    }
    template<class Access>bool ReleaseCompleted(Access& access)noexcept{
        if(state_!=State::Returned)return false;state_=State::Indeterminate;
        for(unsigned i=0;i<outputCount_;++i)if(outputs_[i].generation){
            for(unsigned j=i+1;j<outputCount_;++j)if(outputs_[i]==outputs_[j])outputs_[j]={};
        }
        bool released=ReleaseInputs(access);
        for(unsigned i=0;i<outputCount_;++i)if(outputs_[i].generation){
            released=bank_.Release(access,outputs_[i])&&released;outputs_[i]={};}
        // For zero actual outputs, cancel the worst-case reservation too.
        if(outputReserve_.slots.id)released=bank_.CancelUnpublished(access,outputReserve_)&&released;
        if(released)state_=State::Complete;else bank_.MarkUncertain();return released;
    }
    template<class Access>bool TakeReturnedOutputs(Access& access,
        std::array<typename Bank::Handle,32>& retained,unsigned& count)noexcept{
        retained={};count=0;if(state_!=State::Returned)return false;state_=State::Indeterminate;
        if(!ReleaseInputs(access)) {bank_.MarkUncertain();return false;}
        if(outputReserve_.slots.id&&!bank_.CancelUnpublished(access,outputReserve_)){bank_.MarkUncertain();return false;}
        // Transfer one claim per distinct output handle. The existing owning
        // feature must keep these until native ownership/lifecycle is proved;
        // another allocating call must not observe an unretained new widget.
        for(unsigned i=0;i<outputCount_;++i)if(outputs_[i].generation){
            bool duplicate=false;for(unsigned j=0;j<count;++j)duplicate|=retained[j]==outputs_[i];
            if(!duplicate)retained[count++]=outputs_[i];
        }
        outputs_={};outputCount_=0;state_=State::Complete;return true;
    }
    void NativeFailed()noexcept{if(state_==State::Entered){state_=State::Indeterminate;bank_.MarkUncertain();}}
};
}
