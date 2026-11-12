#pragma once
#include "NativeMenuRowAccess.hpp"
#include <optional>
namespace cfgmenu::rows {
enum class ViewState {Closed,Warming,Building,Ready,RollingBack,Retiring,Failed,Detached};
// Integrated owner-thread view controller. A class-scoped frontend callback
// provider must supply the current view; this does NOT install a broad hook.
// Warm one native binding OR run one row phase per callback, never both.
template<class Reader,class Engine> class NativeSetup {
    Reader& reader_;Engine& engine_;reflection::RetainedMenuText& text_;
    std::optional<NativeAccess<Reader,Engine>> access_;
    Runtime rows_;
    ViewState state_=ViewState::Closed;unsigned warmed_=0,prepareStage_=0;bool borrowOnly_=false;
    std::uint64_t generation_=0,last_=0;bool paced_=false,servicing_=false;
    bool Warm(){using namespace execdisk;
        switch(warmed_){
        case 0:return access_->WarmBinding(Create,CreateParameters,CreateCode,CreateFactoryCode);
        case 1:return access_->WarmBinding(AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode,AddChildToVerticalBoxFactoryCode);
        case 2:return access_->WarmBinding(Conv_StringToText,Conv_StringToTextParameters,Conv_StringToTextCode,Conv_StringToTextFactoryCode);
        case 3:return access_->WarmBinding(SetText,SetTextParameters,SetTextCode,SetTextFactoryCode);
        case 4:return access_->WarmBinding(RemoveChild,RemoveChildParameters,RemoveChildCode,RemoveChildFactoryCode);
        default:return false;
        }
    }
    void Sync(){
        switch(rows_.CurrentState()){
        case State::Building:state_=ViewState::Building;break;
        case State::Ready:state_=ViewState::Ready;break;
        case State::RollingBack:state_=ViewState::RollingBack;break;
        case State::Retiring:state_=ViewState::Retiring;break;
        case State::Empty:access_.reset();state_=ViewState::Closed;generation_=last_=0;paced_=false;break;
        case State::Failed:state_=ViewState::Failed;break;
        case State::Detached:state_=ViewState::Detached;break;
        default:state_=ViewState::Failed;break;
        }
    }
public:
    NativeSetup(Reader& reader,Engine& engine,reflection::RetainedMenuText& text):reader_(reader),engine_(engine),text_(text){}
    NativeSetup(const NativeSetup&)=delete;NativeSetup& operator=(const NativeSetup&)=delete;
    bool Open(const active::Context& context,std::uint64_t generation,const Spec* specs,unsigned count){
        return OpenForClass(context,generation,specs,count,false);
    }
    bool OpenForClass(const active::Context& context,std::uint64_t generation,const Spec* specs,unsigned count,bool challenges){
        if(servicing_||state_!=ViewState::Closed||!generation||!specs||!count||count>Capacity)return false;
        // Validate strings before reflection preparation; no partial live view
        // is accepted merely because some of its rows can be constructed.
        for(unsigned i=0;i<count;++i){if(!specs[i].Valid())return false;
            for(unsigned j=0;j<i;++j)if(specs[i].id==specs[j].id)return false;}
        access_.emplace(reader_,engine_,text_);
        if(!access_->Prepare(context,generation,challenges)||!rows_.Begin(generation,specs,count)){
            prepareStage_=access_->prepareStage;
            access_.reset();state_=ViewState::Failed;return false;
        }
        borrowOnly_=true;for(unsigned i=0;i<count;++i)borrowOnly_&=specs[i].reuseNativeCount;
        generation_=generation;warmed_=0;last_=0;paced_=false;state_=borrowOnly_?ViewState::Building:ViewState::Warming;return true;
    }
    bool Service(std::uint64_t now){
        // Most callbacks after Ready perform NO reads/native calls/allocation.
        // Start explicitly validates current ownership and all choices anew.
        if(servicing_||(state_!=ViewState::Warming&&state_!=ViewState::Building&&state_!=ViewState::RollingBack&&state_!=ViewState::Retiring)||
            (paced_&&(now<last_||now-last_<PhaseGapMs)))return false;
        struct Guard {bool& flag;explicit Guard(bool& value):flag(value){flag=true;}~Guard(){flag=false;}} guard(servicing_);
        paced_=true;last_=now;
        if(!engine_.BeginPhase()){rows_.Detached();state_=ViewState::Failed;return false;}
        if(state_==ViewState::Warming){
            if(!access_->Current(generation_)){rows_.Detached();state_=ViewState::Detached;return false;}
            if(!Warm()){rows_.Detached();state_=ViewState::Failed;return false;}
            if(++warmed_==5)state_=ViewState::Building;return true;
        }
        const auto result=rows_.Step(*access_,now);Sync();return result;
    }
    bool Rebind(const active::Context& current){return !servicing_&&
        (state_==ViewState::Ready||state_==ViewState::Warming||state_==ViewState::Building)&&access_&&access_->Rebind(current);}
    bool Current(std::uint64_t generation){return access_&&generation==generation_&&access_->Current(generation);}
    bool Index(std::uintptr_t widget,unsigned& out){out=0;return access_&&access_->Index(widget,out);}
    bool NativeSkill(unsigned& out){out=0;return access_&&access_->NativeSkill(out);}
    bool HideDifficulty(){return access_&&engine_.BeginPhase()&&access_->HideDifficulty();}
    Runtime& Rows()noexcept{return rows_;}
    ViewState State()const noexcept{return state_;}
    unsigned WarmStage()const noexcept{return borrowOnly_?0:warmed_+1;} //0 native-control reuse;1Create..5RemoveChild;6complete
    unsigned PrepareStage()const noexcept{return prepareStage_;}
    // Asynchronous close: caller consumes Back/travel until State()==Closed.
    // It must preserve a committed GameplayHandoff separately during cleanup.
    // Forgetting attached handles here would duplicate rows on cached replay.
    bool Close(){
        if(servicing_)return false;if(state_==ViewState::Closed||state_==ViewState::Retiring)return true;
        if(!access_||!access_->Current(generation_)){rows_.Detached();state_=ViewState::Detached;return false;}
        if(!rows_.Retire())return false;state_=ViewState::Retiring;return true;
    }
};
}
