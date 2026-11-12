#pragma once
#include "NativeMenuRowAccess.hpp"
#include "../RuntimeProbe/OfflineEntryObservation.hpp"
#include "FrontendHudGate.hpp"
#include "../RuntimeProbe/FrontendHudObservation.hpp"
#include "OfflineEntryOrder.hpp"
namespace cfgmenu::entry {
// Reuses the native buttons, label children, immutable text pool and existing
// exact native converter/SetText bindings. No new menu, row or focus layout.
template<class Reader,class Engine> class Labels {
    Reader& reader_;Engine& engine_;reflection::RetainedMenuText& text_;Binding binding_{};
    reflection::RetainedMenuText::Request counselor_{},jason_{};unsigned stage_=0;bool refused_=false;
    std::uint64_t last_=0;bool paced_=false;unsigned failure_=0,currentFailure_=0;
    template<std::size_t P,std::size_t C,std::size_t F> bool Warm(const execdisk::Binding& function,
        const std::array<execdisk::Parameter,P>& parameters,const std::array<unsigned char,C>& code,const std::array<unsigned char,F>& factory){
        std::uintptr_t cached=0,owner=0,ctor=0;
        if(!Current()||!reader_.Read(binding_.context.module+function.reflection,owner)||
           !reader_.Read(binding_.context.module+function.constructor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor)||
           !reader_.Read(binding_.context.module+function.cache,cached))return false;
        if(!cached){std::array<unsigned char,F> actual{};
            if(!reader_.ReadBytes(binding_.context.module+function.factory,actual.data(),actual.size())||actual!=factory||
               !Current()||!rows::FactoryWithOwner(engine_,binding_.context.module+function.factory,owner,cached))return false;}
        capture::FunctionEvidence evidence;
        return Current()&&native::CachedFunction(reader_,binding_.context.module,function,parameters,code,evidence)&&evidence.function.object==cached;
    }
    bool Convert(std::u16string_view label,reflection::RetainedMenuText::Request& request){
        if(!Current())return false;if(text_.Find(label,request))return true;
        capture::FunctionEvidence function;std::uintptr_t receiver=0;cfgprobe::Ancestry ancestry;
        if(!native::CachedFunction(reader_,binding_.context.module,execdisk::Conv_StringToText,
               execdisk::Conv_StringToTextParameters,execdisk::Conv_StringToTextCode,function)||
           !reader_.Read(function.function.owner+cfgspawn::ClassDefaultObject,receiver)||!cfgprobe::ObjectPointer(receiver)||
           !cfgprobe::CaptureAncestry(reader_,receiver,ancestry)||!ancestry.Contains(function.function.owner)||
           !cfgprobe::StableAncestry(reader_,receiver,ancestry)||!Current()||!text_.Begin(label,request))return false;
        const bool returned=engine_.Invoke(receiver,function.function.object,request.packet);
        return text_.Complete(request,returned)&&Current();
    }
    bool Label(std::uintptr_t target,const reflection::RetainedMenuText::Request& request){
        const auto* packet=text_.Borrow(request);capture::FunctionEvidence function;cfgprobe::Ancestry ancestry;
        failure_=10;if(!packet)return false;
        failure_=11;if(!Current())return false;
        failure_=12;if(!native::CachedFunction(reader_,binding_.context.module,execdisk::SetText,execdisk::SetTextParameters,execdisk::SetTextCode,function))return false;
        failure_=13;if(!cfgprobe::CaptureAncestry(reader_,target,ancestry))return false;
        failure_=14;if(!ancestry.Contains(function.function.owner))return false;
        failure_=15;if(!cfgprobe::StableAncestry(reader_,target,ancestry))return false;
        failure_=16;if(!Current())return false;
        failure_=17;if(!engine_.Invoke(target,function.function.object,const_cast<reflection::OpaqueText*>(packet)))return false;
        failure_=18;if(!Current())return false;
        failure_=0;return true;
    }
public:
    Labels(Reader& reader,Engine& engine,reflection::RetainedMenuText& text,const Binding& binding):reader_(reader),engine_(engine),text_(text),binding_(binding){}
    bool Current(){return !refused_&&entry::Current(reader_,binding_,engine_.Thread(),&currentFailure_);}
    bool Service(std::uint64_t now,unsigned maximumStages=1){
        if(refused_||stage_==6||(paced_&&(now<last_||now-last_<25)))return false;
        if(!maximumStages||maximumStages>2)return false;
        paced_=true;last_=now;failure_=1;if(!engine_.BeginPhase()){refused_=true;return false;}
        failure_=2;if(!Current()){refused_=true;return false;}failure_=3;
        // Donor performs conversion + SetText together. Keep that paired work
        // on one genuine owner callback, bounded to two native calls and the
        // original shared reader cap. Every step retains its fresh guards.
        for(unsigned step=0;step<maximumStages&&stage_<6;++step){
        if(step&&!setupslice::MayContinue(reader_))break;
        bool okay=false;using namespace execdisk;
        switch(stage_){
        case 0:okay=Warm(Conv_StringToText,Conv_StringToTextParameters,Conv_StringToTextCode,Conv_StringToTextFactoryCode);break;
        case 1:okay=Warm(SetText,SetTextParameters,SetTextCode,SetTextFactoryCode);break;
        case 2:okay=Convert(u"Offline Bots - Play as a Counselor",counselor_);break;
        case 3:okay=Label(binding_.sandboxTitle,counselor_);break;
        case 4:okay=Convert(u"Offline Bots - Play as Jason",jason_);break;
        case 5:okay=Label(binding_.jasonTitle,jason_);break;
        default:break;
        }
        if(!okay){refused_=true;return false;}failure_=0;++stage_;
        }
        return true;
    }
    bool Ready()const noexcept{return !refused_&&stage_==6;}
    bool Refused()const noexcept{return refused_;}
    unsigned Stage()const noexcept{return stage_;}
    unsigned Failure()const noexcept{return failure_;}
    unsigned CurrentFailure()const noexcept{return currentFailure_;}
};
template<class Reader,class Engine> class View {
    Reader& reader_;Engine& engine_;hud::Binding transport_{};frontend::HudGate gate_;Labels<Reader,Engine> labels_;
    bool valid_=false;
    std::unique_ptr<entryorder::Native<Reader,Engine>> order_;
    bool FrontendMatches(const frontend::HudDescriptor& descriptor){return valid_&&descriptor.context==transport_.context&&
        descriptor.event==transport_.event&&hud::Current(reader_,transport_,engine_.Thread());}
    bool Service(std::uint64_t now){return labels_.Ready()&&order_&&!order_->Finished()?order_->ServiceSlice():labels_.Service(now,2);}
    bool SetupFinished()const{return Ready();}
    bool SetupRefused()const{return !valid_||labels_.Refused()||(order_&&order_->Finished()&&!entryorder::Usable(order_->Status()));}
    friend class frontend::HudGate;
public:
    View(Reader& reader,Engine& engine,reflection::RetainedMenuText& text,const Binding& binding,const hud::Binding& transport,bool reorder=false):
        reader_(reader),engine_(engine),transport_(transport),labels_(reader,engine,text,binding){
        valid_=binding.context==transport.context&&hud::Current(reader_,transport_,engine_.Thread())&&
            gate_.Bind({transport.context,transport.event},true);
        if(reorder)order_=std::make_unique<entryorder::Native<Reader,Engine>>(reader_,engine_,binding);
    }
    bool After(std::uintptr_t object,std::uintptr_t function,bool nested,bool completed,std::uint64_t now){
        return gate_.After(*this,object,function,engine_.Thread(),nested,completed,now);
    }
    bool Ready()const{return valid_&&labels_.Ready()&&(!order_||order_->Finished())&&
        (!order_||entryorder::Usable(order_->Status()));}
    bool Opportunity(std::uintptr_t widget,bool nested,bool completed,std::uint64_t now){
        return gate_.Opportunity(*this,widget,engine_.Thread(),nested,completed,now);
    }
    bool HudOpportunity(std::uintptr_t hud,bool nested,bool completed,std::uint64_t now){
        return gate_.CompletedHudOpportunity(*this,hud,engine_.Thread(),nested,completed,now);
    }
    bool Refused()const{return !valid_||labels_.Refused()||gate_.Report().state==frontend::HudState::Refused;}
    unsigned Stage()const{return labels_.Stage();}
    unsigned Failure()const{return labels_.Failure();}
    unsigned CurrentFailure()const{return labels_.CurrentFailure();}
    unsigned OrderState()const{return order_?order_->Status():0;}
    unsigned OrderFailure()const{return order_?order_->Failure():0;}
    unsigned OrderCaptureStage()const{return order_?order_->CaptureStage():0;}
    MenuWarmDiagnostic OrderWarmDiagnostic()const{return order_?order_->WarmDiagnostic():MenuWarmDiagnostic{};}
    const frontend::HudReport& Report()const{return gate_.Report();}
};
}
