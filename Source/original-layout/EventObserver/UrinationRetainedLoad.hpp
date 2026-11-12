#pragma once
#include "UrinationActionObservation.hpp"
#include "ResidentRootBank.hpp"
namespace cfgurination {
// Compatibility handoff for the donor's ONE asynchronous request. Reuses the
// existing engine-visible bank; no UObject RootSet writes, raw asset cache,
// native-call permission, hook installation, scheduling or emote policy here.
// Every bank operation requires the caller's genuinely proved registry window.
template<class Bank>class RetainedLoad {
    Bank& bank_;LoadRequest request_{};
    typename Bank::Reservation output_{};
    std::array<typename Bank::Handle,5> inputs_{};
    typename Bank::Handle loaded_{};
    bool attempted_=false;unsigned retireCursor_=0,transientCursor_=0;
public:
    explicit RetainedLoad(Bank& bank):bank_(bank){}
    const LoadRequest& Request()const noexcept{return request_;}
    bool Attempted()const noexcept{return attempted_;}
    unsigned RetiredInputs()const noexcept{return retireCursor_;}
    bool HasLoadedHandle()const noexcept{return loaded_.generation!=0;}
    // Input order is world/controller/Wave class/original male/original female
    // (female may be zero). Wave CDO is retained by its verified rooted class.
    template<class Access,class Host>bool Prepare(Access& access,Host& host,std::uintptr_t module,
        const LoadIdentity& owner,const std::array<std::uintptr_t,5>& values,std::uint64_t now)noexcept{
        if(attempted_||!owner.Valid()||values[0]!=owner.world||values[1]!=owner.controller||
            !values[2]||!values[3]||!host.Current(owner)||!access.ProvenSlotWindow()||!host.Inputs(owner,values))return false;
        attempted_=true;typename Bank::Reservation reserve{};
        if(!bank_.ReserveOutputs(access,module,5,reserve)){request_.Reject();return false;}
        if(!bank_.CommitFreshReturns(access,reserve,values,inputs_)){
            // Keep the opaque reservation if publication was uncertain. Never
            // turn an uncertain operation into a second native request.
            output_=reserve;request_.Reject();return false;
        }
        if(!bank_.ReserveOutputs(access,module,1,output_)||!host.Current(owner)||
            !request_.Prepare(owner,now,true,true)){request_.Reject();return false;}
        return true;
    }
    bool Enter(bool nativeBoundaryAdmitted)noexcept{return attempted_&&request_.Enter(nativeBoundaryAdmitted);}
    template<class Reader,class Host>bool Constructor(Reader& reader,Host& host,ActionOwner owner,
        std::uintptr_t action,std::uintptr_t returned,std::uintptr_t path,std::uintptr_t latent,std::uintptr_t caller){
        ActionObservation observation(reader,host,owner);
        return observation.Constructor(request_,action,returned,path,latent,caller);
    }
    bool Returned(bool normalReturn,bool current)noexcept{return request_.RequestReturned(normalReturn,current);}
    // Asset metadata is used only through current strong handles inside the
    // caller's proved window. No old world/controller pointer is resolved.
    template<class Access,class Visitor>bool WithAssetRoots(Access& access,Visitor&& visitor)noexcept{
        const auto stage=request_.Stage();
        if((stage!=LoadStage::Prepared&&stage!=LoadStage::Requesting&&stage!=LoadStage::Waiting&&
            stage!=LoadStage::Completing&&stage!=LoadStage::Ready)||!access.ProvenSlotWindow())return false;
        std::array<std::uintptr_t,3> assets{};
        for(unsigned i=0;i<assets.size();++i){const auto handle=inputs_[i+2];
            if(handle.generation){if(!bank_.ResolveRequired(access,handle,assets[i])||!assets[i])return false;}
            else if(i<2)return false;
        }
        return access.ProvenSlotWindow()&&visitor(assets);
    }
    template<class Reader,class Host,class Access>bool Complete(Reader& reader,Host& host,Access& access,
        ActionOwner owner,std::uintptr_t action,std::uintptr_t delegate,std::uintptr_t caller,
        std::uintptr_t loaded,std::uint64_t now)noexcept{
        ActionObservation observation(reader,host,owner);
        if(!observation.Completion(request_,action,delegate,caller)){
            // Only the exact owned frame may retire this request. Foreign
            // action/thread/caller events do not consume our pending action.
            if(request_.Stage()==LoadStage::Waiting&&action==request_.Action()&&owner.thread==request_.Owner().thread&&
                delegate==action+disk::ActionDelegate&&caller==owner.module+disk::DelegateReturn)request_.Reject();
            return false;
        }
        if(!request_.BeginCompletion(action,owner.thread,request_.Owner().generation,now,true,true,true,false))return false;
        // Consume before any root/helper operation can reenter. Do not save the
        // raw callback output even if its type or publication later refuses.
        std::array<typename Bank::Handle,1> result{};
        const bool published=loaded&&host.Current(request_.Owner())&&host.Loaded(request_.Owner(),loaded)&&
            request_.Stage()==LoadStage::Completing&&request_.Action()==action&&access.ProvenSlotWindow()&&
            request_.Stage()==LoadStage::Completing&&request_.Action()==action&&
            bank_.CommitFreshReturns(access,output_,std::array<std::uintptr_t,1>{loaded},result)&&result[0].generation;
        if(published){loaded_=result[0];
            // A publication with changed callback ownership is uncertain, not
            // a reusable asset or permission to replay the native load.
            if(request_.Stage()!=LoadStage::Completing||request_.Action()!=action){
                bank_.MarkUncertain();request_.Reject();return false;
            }}
        return request_.Complete(published);
    }
    // Used only at a later real retained tick. Resolving a strong handle is not
    // authority to patch Wave or dispatch an emote; those guards stay separate.
    template<class Access,class Host>bool Resolve(Access& access,Host& host,std::uintptr_t& loaded)noexcept{
        loaded=0;std::uintptr_t value=0;
        if(request_.Stage()!=LoadStage::Ready||!host.Current(request_.Owner())||!access.ProvenSlotWindow()||
            !bank_.ResolveRequired(access,loaded_,value)||!value||!host.Loaded(request_.Owner(),value))return false;
        loaded=value;return true;
    }
    // Completed resident ASSETS may be reused in a later match. This path does
    // not resolve the old world/controller and cannot authorize another load.
    // Host independently proves the NEW real tick/owner/GC window and validates
    // the current asset metadata through these still-owned strong handles.
    template<class Access,class Host>bool ResolveResident(Access& access,Host& host,std::uintptr_t& loaded)noexcept{
        loaded=0;std::uintptr_t value=0;
        if(request_.Stage()!=LoadStage::Ready||!access.ProvenSlotWindow()||!host.CurrentResidentAssets()||
            !bank_.ResolveRequired(access,loaded_,value)||!value||!host.ResidentLoaded(value)||
            request_.Stage()!=LoadStage::Ready||!host.CurrentResidentAssets()||!access.ProvenSlotWindow()||!bank_.Healthy())return false;
        loaded=value;return true;
    }
    // Free only request-world/controller roots after successful handoff. Asset
    // roots remain resident, preserving the donor's bounded asset lifetime.
    template<class Access>bool ReleaseTransient(Access& access)noexcept{
        if(request_.Stage()!=LoadStage::Ready||!access.ProvenSlotWindow())return false;
        while(transientCursor_<2){auto& handle=inputs_[transientCursor_];
            if(handle.generation&&!bank_.ReleaseAcquired(access,handle))return false;
            handle={};++transientCursor_;
        }
        return true;
    }
    void Expire(std::uint64_t now,bool current)noexcept{request_.Expire(now,current);}
    void Destroyed(std::uintptr_t action)noexcept{request_.ObserveDestroyed(action);}
    // Failed/cancelled requests may be cleaned only in a NEW proved window.
    // Partial progress is kept; never release already retired handles again.
    template<class Access>bool RetireRejected(Access& access)noexcept{
        if(request_.Stage()!=LoadStage::Rejected||!access.ProvenSlotWindow())return false;
        if(output_.bank&&!bank_.CancelUnpublished(access,output_))return false;
        while(retireCursor_<inputs_.size()){auto& handle=inputs_[retireCursor_];
            if(handle.generation&&!bank_.ReleaseAcquired(access,handle))return false;
            handle={};++retireCursor_;
        }
        return true;
    }
};
}
