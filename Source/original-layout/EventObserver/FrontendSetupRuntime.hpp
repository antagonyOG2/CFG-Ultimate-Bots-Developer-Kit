#pragma once
#include "NativeSetupRuntime.hpp"
#include "FrontendHudGate.hpp"
#include "../RuntimeProbe/FrontendHudObservation.hpp"
namespace cfgmenu::frontend {
// Concrete exact-event provider wired to the production native row adapter.
// The actor trampoline calls After only AFTER forwarding original normally,
// with its nesting guard still active. This class installs no hook and never
// chooses a map/profile or invokes stock OfflineBots travel on refusal.
template<class Reader,class Engine> class SetupRuntime {
    Reader& reader_;Engine& engine_;
    hud::Binding binding_{};HudGate gate_;
    rows::NativeSetup<Reader,Engine> view_;
    bool FrontendMatches(const HudDescriptor& descriptor){
        return descriptor.context==binding_.context&&descriptor.event==binding_.event&&
            hud::Current(reader_,binding_,engine_.Thread());
    }
    bool Service(std::uint64_t now){return view_.Service(now);}
    bool SetupFinished()const{return view_.State()==rows::ViewState::Ready||view_.State()==rows::ViewState::Closed;}
    bool SetupRefused()const{return view_.State()==rows::ViewState::Failed||view_.State()==rows::ViewState::Detached;}
    friend class HudGate;
public:
    SetupRuntime(Reader& reader,Engine& engine,reflection::RetainedMenuText& texts):reader_(reader),engine_(engine),view_(reader,engine,texts){}
    SetupRuntime(const SetupRuntime&)=delete;SetupRuntime& operator=(const SetupRuntime&)=delete;
    bool Open(const active::Context& context,std::uint64_t generation,const rows::Spec* specs,unsigned count,bool exactImageVerified,const hud::Binding* transport=nullptr){
        if(view_.State()!=rows::ViewState::Closed||engine_.Thread()!=context.thread)return false;
        hud::Binding candidate;
        if(transport){
            // The frontend transport already observed this exact HUD binding.
            // Revalidate it directly, never repeat its cold hierarchy discovery.
            if(!exactImageVerified||!(transport->context==context)||!hud::Current(reader_,*transport,engine_.Thread()))return false;
            candidate=*transport;
        }else if(!hud::Observe(reader_,context,exactImageVerified,candidate))return false;
        gate_.Retire();
        if(!gate_.Bind({context,candidate.event},true))return false;
        binding_=candidate;
        if(!view_.Open(context,generation,specs,count)){gate_.Retire();binding_={};return false;}
        return true;
    }
    bool After(std::uintptr_t object,std::uintptr_t function,std::uint32_t thread,bool nested,bool completed,std::uint64_t now){
        return gate_.After(*this,object,function,thread,nested,completed,now);
    }
    bool Close(){
        // Keep committed gameplay handoff independent: neither menu cleanup nor
        // HudGate retirement resets/cancels a pending fresh-world submission.
        if(view_.State()==rows::ViewState::Closed){gate_.Retire();binding_={};return true;}
        if(gate_.Report().state==HudState::Refused||!hud::Current(reader_,binding_,engine_.Thread())||!view_.Close())return false;
        if(gate_.Report().state==HudState::Finished){gate_.Retire();return gate_.Bind({binding_.context,binding_.event},true);}
        return true;
    }
    bool Rebind(const hud::Binding& current){
        if(gate_.Report().state==HudState::Refused||!hud::Current(reader_,current,engine_.Thread()))return false;
        if(view_.State()==rows::ViewState::Closed){binding_=current;return true;}
        if(!view_.Rebind(current.context))return false;
        binding_=current;
        if(view_.State()==rows::ViewState::Ready)return true;
        gate_.Retire();return gate_.Bind({current.context,current.event},true);
    }
    bool Current(std::uint64_t generation){return gate_.Report().state!=HudState::Refused&&view_.Current(generation);}
    bool Index(std::uintptr_t widget,unsigned& value){return gate_.Report().state!=HudState::Refused&&view_.Index(widget,value);}
    bool NativeSkill(unsigned& value){return gate_.Report().state!=HudState::Refused&&view_.NativeSkill(value);}
    rows::Runtime& Rows()noexcept{return view_.Rows();}
    rows::ViewState State()const noexcept{return view_.State();}
    unsigned WarmStage()const noexcept{return view_.WarmStage();}
    unsigned PrepareStage()const noexcept{return view_.PrepareStage();}
    const HudReport& Report()const noexcept{return gate_.Report();}
};
}
