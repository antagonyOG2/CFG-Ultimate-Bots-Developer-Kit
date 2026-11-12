#pragma once
#include "UrinationLoadRequest.hpp"
namespace cfgurination {
enum class RuntimeStage:unsigned {Discovery,Configure,Install,Prepare,MakePathFunction,MakePath,LoadFunction,Load,Waiting,Publish,Retire,Active,Refused};
// Compatibility orchestration of the original one-shot native request. One
// operation per optional gameplay slice; refusal never restarts a native call.
class RuntimePhases {
    RuntimeStage stage_=RuntimeStage::Discovery;std::uint64_t next_=0;
public:
    RuntimeStage Stage()const noexcept{return stage_;}
    template<class Host>bool Step(Host& host,std::uint64_t now){
        if(stage_==RuntimeStage::Active||stage_==RuntimeStage::Refused||now<next_||!host.Admitted())return false;
        next_=now+100;bool okay=false;
        switch(stage_){
        case RuntimeStage::Discovery:
            if(!host.Discover()){next_=now+1000;return true;}stage_=RuntimeStage::Configure;return true;
        case RuntimeStage::Configure:okay=host.Configure();break;
        case RuntimeStage::Install:okay=host.Install();break;
        case RuntimeStage::Prepare:okay=host.PrepareDiscovered(now);break;
        case RuntimeStage::MakePathFunction:okay=host.LoaderFunction(false);break;
        case RuntimeStage::MakePath:okay=host.MakePath();break;
        case RuntimeStage::LoadFunction:okay=host.LoaderFunction(true);break;
        case RuntimeStage::Load:okay=host.RequestLoad();break;
        case RuntimeStage::Waiting:
            host.Expire(now);
            if(host.LoadState()==LoadStage::Waiting)return true;
            stage_=host.LoadState()==LoadStage::Ready?RuntimeStage::Publish:RuntimeStage::Refused;return true;
        case RuntimeStage::Publish:
            if(!host.PublicationDue())return true;okay=host.PublishLoaded();break;
        case RuntimeStage::Retire:okay=host.RetireRequestInputs();break;
        default:stage_=RuntimeStage::Refused;return false;
        }
        stage_=okay?static_cast<RuntimeStage>(static_cast<unsigned>(stage_)+1):RuntimeStage::Refused;
        return true;
    }
};
}
