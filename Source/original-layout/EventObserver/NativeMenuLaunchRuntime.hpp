#pragma once
#include "FrontendSetupRuntime.hpp"
#include "NativeSettingsStartSlots.hpp"
#include "NativeSandboxStartHost.hpp"
#include "NativeGraphEntryBoundary.hpp"
#include "NativeOfflineEntryRuntime.hpp"
#include "HostedSettingsSpecs.hpp"
#include "NativeCounselorModePolicy.hpp"
#include "../Core/GameplayLimits.hpp"
#include "../RuntimeProbe/NativeLaunchSelection.hpp"
#include "../RuntimeProbe/AcceptedCounselorObservation.hpp"
#include "../Core/MenuPreparationSchedule.hpp"
#include <optional>
#include <memory>
#include <utility>
namespace cfgmenu::launch {
enum class State {Empty,Opening,Ready,Submitted,Refused,Indeterminate};
// Concrete frontend composition. Transport owns this object for the entire
// process and supplies resident thunks; no worker, injector or polling lives
// here. Only verified implemented options are accepted, never a decorative
// toggle whose gameplay counterpart has not been ported.
template<class Reader,class Engine> class NativeMenuLaunch {
    Reader& reader_;Engine& engine_;frontend::SetupRuntime<Reader,Engine> view_;
    Session session_;GameplayHandoff handoff_{session_};
    std::optional<SetupStart> start_;std::optional<startroute::Router> router_;
    startbinding::Binding binding_{};startslots::Slots slots_;sandboxstart::Route route_;
    launchselection::Observation selections_{};startroute::OriginalFrame frame_{};
    std::uintptr_t wrapper_=0,graph_=0;std::uint64_t catalogueGeneration_=0;
    bool exact_=false,invoking_=false,skillEnabled_=false;State state_=State::Empty;
    active::Context context_{};preparation::Schedule preparation_;
    std::array<rows::Entry,rows::Capacity> preparationSpecs_{};unsigned preparationCount_=0;
    HMODULE adapter_=nullptr;std::optional<hud::Binding> transport_;
    Origin origin_=Origin::NativeSandbox;
    unsigned preparationFailure_=0;
    launchselection::Diagnostic selectionDiagnostic_{};
    // Metadata copied from the exact authored map graph, not a raw owner lease.
    // Native Start's writer revalidates its current owner/schema before use.
    cfgreflection::Property authoredMode_{};
    std::optional<cfgprimary::SoftClassBytes> acceptedCounselor_;
    std::optional<startbinding::Binding> inheritedBinding_;
    using NativePreparation=bool(*)(const MenuCatalogueEvidence&,GameplayHandoff&,const active::Context&);
    NativePreparation nativePreparation_=nullptr;
public:
    NativeMenuLaunch(Reader& reader,Engine& engine,reflection::RetainedMenuText& text):reader_(reader),engine_(engine),view_(reader,engine,text){}
    NativeMenuLaunch(const NativeMenuLaunch&)=delete;NativeMenuLaunch& operator=(const NativeMenuLaunch&)=delete;
    bool UseNativeOffline(NativePreparation prepare){if(state_!=State::Empty||!prepare)return false;nativePreparation_=prepare;return true;}
    bool Open(const active::Context& context,const Capabilities& capabilities,unsigned maximumCounselors,
        const rows::Spec* specs,unsigned count,std::uint64_t catalogueGeneration,HMODULE adapter,
        std::uintptr_t wrapperThunk,std::uintptr_t graphThunk,bool exactImageVerified,const hud::Binding* transport=nullptr,
        Origin origin=Origin::NativeSandbox,const cfgreflection::Property* authoredMode=nullptr,const cfgprimary::SoftClassBytes* acceptedCounselor=nullptr,
        const startbinding::Binding* inheritedBinding=nullptr){
        if(state_!=State::Empty||!exactImageVerified||engine_.Thread()!=context.thread||!capabilities.automaticJason||
            maximumCounselors<1||maximumCounselors>cfglimits::MaximumCounselors||!catalogueGeneration||!specs||!count||count>rows::Capacity)return false;
        for(unsigned i=0;i<count;++i){if(!specs[i].Valid())return false;
            for(unsigned j=0;j<i;++j)if(specs[i].id==specs[j].id)return false;}
        if((transport&&!(transport->context==context))||
           (origin!=Origin::NativeSandbox&&origin!=Origin::AcceptedCounselorPicker))return false;
        if(authoredMode&&(origin!=Origin::AcceptedCounselorPicker||
            !cfgprobe::ObjectPointer(authoredMode->field.object)||!cfgprobe::ObjectPointer(authoredMode->field.owner)||
            authoredMode->field.label.View()!=u"PendingOfflineMode"||authoredMode->field.kind.View()!=u"ClassProperty"||
            authoredMode->dimension!=1||authoredMode->elementBytes!=8||authoredMode->offset!=selectiondisk::PendingOfflineMode||
            authoredMode->flags!=0x0/*CFG3.5 address withheld*/40000204ULL))return false;
        if(acceptedCounselor&&(origin!=Origin::AcceptedCounselorPicker||!softpath::Plain(*acceptedCounselor)))return false;
        state_=State::Opening;exact_=true;catalogueGeneration_=catalogueGeneration;
        skillEnabled_=capabilities.verified[static_cast<unsigned>(RowId::Skill)];
        wrapper_=wrapperThunk;graph_=graphThunk;
        if(!session_.Open(context.widget,Mode::Counselor,capabilities,maximumCounselors)){state_=State::Refused;return false;}
        context_=context;adapter_=adapter;preparationCount_=count;origin_=origin;
        if(authoredMode)authoredMode_=*authoredMode;
        if(acceptedCounselor)acceptedCounselor_=*acceptedCounselor;
        if(inheritedBinding)inheritedBinding_=*inheritedBinding;
        for(unsigned i=0;i<count;++i)preparationSpecs_[i].Copy(specs[i]);
        if(transport)transport_=*transport;
        selections_.evidence.maximumBots=maximumCounselors-1;
        return preparation_.Begin(4);
    }
    bool PreparationCurrent(){
        preparationFailure_=100;
        if(state_!=State::Opening||!active::Current(reader_,context_,engine_.Thread()))return false;
        preparationFailure_=101;if(transport_&&!hud::Current(reader_,*transport_,engine_.Thread()))return false;
        preparationFailure_=102;
        if(slots_.Current()==startslots::State::Installed&&
           !startbinding::Current(reader_,binding_,engine_.Thread(),wrapper_,graph_))return false;
        preparationFailure_=0;return true;
    }
    bool PrepareStage(unsigned stage){
        preparationFailure_=stage+1;
        const auto& context=context_;const auto generation=session_.Generation();
        if(stage==0){
            // Claim guarded Start before yielding to later cold stages. An
            // early owned Start consumes waiting, never launches vanilla travel.
            if(inheritedBinding_){
                if(!reentry::Infrastructure(inheritedBinding_->context,context)||inheritedBinding_->context.type!=context.type)return false;
                binding_=*inheritedBinding_;binding_.context=context;
                if(!startbinding::Current(reader_,binding_,engine_.Thread(),wrapper_,graph_))return false;
            }else{
                auto scratch=std::make_unique<startbinding::Workspace>();
                if(!startbinding::Observe(reader_,context,true,*scratch,binding_))return false;
            }
            start_.emplace(session_,handoff_,view_.Rows());router_.emplace(binding_,*start_);
            if(!start_->ClaimPreparing(context.widget,generation))return false;
            startslots::NativeAccess<Reader> access(reader_,binding_,adapter_,true);
            if(!(inheritedBinding_?slots_.Adopt(access,context.module+startdisk::ScriptExecutor,wrapper_,graph_):
                 slots_.Install(access,context.module+startdisk::ScriptExecutor,wrapper_,graph_))){
                if(slots_.Current()==startslots::State::Indeterminate)state_=State::Indeterminate;
                return false;
            }
            return true;
        }
        if(stage==1){if(!launchselection::Observe(reader_,context,engine_.Thread(),generation,catalogueGeneration_,
            selections_.evidence.maximumBots,true,selections_,nullptr,&selectionDiagnostic_))return false;
            return !acceptedCounselor_||acceptedcounselor::Apply(selections_.evidence,*acceptedCounselor_);}
        if(stage==3){
            std::array<rows::Spec,rows::Capacity> specs{};
            for(unsigned i=0;i<preparationCount_;++i)specs[i]=preparationSpecs_[i].spec;
            return view_.Open(context,generation,specs.data(),preparationCount_,true,transport_?&*transport_:nullptr);
        }
        if(stage!=2)return false;
        const auto& e=selections_.evidence;
        // Existing Sandbox stays unchanged. Dedicated accepted counselor flow
        // reuses the original map/Jason/settings sequence, switching its typed
        // pending mode only inside the already guarded original Start call.
        // Never grant this origin to the normal Offline Bots Jason entry.
        catalogues::Modes modes;
        if(!catalogues::ObserveModes(reader_,context,true,modes)||modes.selectedMap!=e.map)return false;
        if(!ModesForOrigin(origin_,modes,selections_.map.mode))return false;
        if(!(e.preserveNativeCounselor?session_.PickNativeCustomization(context.widget,generation,e.map,e.jason):
            session_.Pick(context.widget,generation,e.map,e.jason,e.counselor)))return false;
        return true;
    }
    bool After(std::uintptr_t object,std::uintptr_t function,std::uint32_t thread,bool nested,bool completed,std::uint64_t now){
        if(state_!=State::Opening)return false;
        if(preparation_.Current()!=preparation::State::Ready){
            if(nested||!completed||object!=context_.hud||thread!=context_.thread||
                (transport_&&function!=transport_->event))return false;
            const bool work=preparation_.Step(*this,now);
            if(work)preparationFailure_=0;
            if(preparation_.Current()==preparation::State::Refused&&state_!=State::Indeterminate)state_=State::Refused;
            return work; // Never combine a cold stage with row warmup/creation.
        }
        const bool work=view_.After(object,function,thread,nested,completed,now);
        if(view_.State()==rows::ViewState::Ready)state_=State::Ready;
        if(view_.State()==rows::ViewState::Failed||view_.State()==rows::ViewState::Detached)state_=State::Refused;
        return work;
    }
    bool Claimed()const noexcept{return state_!=State::Empty&&start_.has_value();}
    bool Current(std::uint64_t generation){
        if(!exact_||generation!=session_.Generation()||state_==State::Refused||state_==State::Indeterminate)return false;
        if(preparation_.Current()!=preparation::State::Ready)return active::Current(reader_,context_,engine_.Thread());
        return view_.Current(generation);
    }
    bool Index(std::uintptr_t widget,unsigned& value){return view_.Index(widget,value);}
    bool Weather(unsigned,std::uint64_t&){return false;} // Native CFG row retains original weather; no duplicate stock enum.
    bool CurrentBinding(const startbinding::Binding& binding,startroute::SlotKind){
        return binding.context==binding_.context&&slots_.Current()==startslots::State::Installed&&
            startbinding::Current(reader_,binding_,engine_.Thread(),wrapper_,graph_);
    }
    bool NativeChoices(Session& candidate,std::uint64_t generation){
#if CFG_H20_MENU && !CFG_H21_LOBBY_DISCOVERY
        if(!StoreOnlineChoices())return false;
#endif
        launchselection::Observation fresh;
        if(!launchselection::Observe(reader_,binding_.context,engine_.Thread(),generation,catalogueGeneration_,
            selections_.evidence.maximumBots,exact_,fresh))return false;
        if(acceptedCounselor_&&!acceptedcounselor::Apply(fresh.evidence,*acceptedCounselor_))return false;
        const auto& a=selections_.evidence;const auto& b=fresh.evidence;
        // A stale menu session may not silently adopt an unrelated profile/map.
        // Changing a native picker is handled by a new owned menu generation.
        const bool sameJasonPath=a.selectedClass.Valid()?(softpath::Empty(a.selectedJason)&&softpath::Empty(b.selectedJason)):
            softpath::Same(a.selectedJason,b.selectedJason);
        if(selections_.map.mode!=fresh.map.mode||a.map!=b.map||a.jason!=b.jason||a.counselor!=b.counselor||!sameJasonPath||
            !(a.selectedClass==b.selectedClass)||a.preserveNativeCounselor!=b.preserveNativeCounselor)return false;
        unsigned skill=0;
        if(skillEnabled_&&(!view_.NativeSkill(skill)||!candidate.SetChoice(generation,RowId::Skill,skill)))return false;
        selections_=fresh;return true;
    }
    bool Catalogue(MenuCatalogueEvidence& output){
        output=selections_.evidence;LARGE_INTEGER counter{};QueryPerformanceCounter(&counter);
        output.loadoutSeed=static_cast<std::uint32_t>(counter.QuadPart^GetTickCount64());return true;
    }
    bool TravelSandbox(const MenuCatalogueEvidence& evidence){
        if(!invoking_||evidence.frontend!=binding_.context.widget||evidence.generation!=session_.Generation())return false;
        if(nativePreparation_&&!nativePreparation_(evidence,handoff_,binding_.context)){state_=State::Refused;return false;}
        const auto result=sandboxstart::SubmitNativeStart(reader_,binding_,handoff_,session_.Generation(),wrapper_,graph_,frame_,route_,exact_,nullptr,
            origin_==Origin::AcceptedCounselorPicker,authoredMode_.field.object?&authoredMode_:nullptr,
            nativePreparation_?sandboxstart::Target::NativeOfflineBots:sandboxstart::Target::Sandbox);
        state_=result==sandboxstart::Result::Forwarded?State::Submitted:
            result==sandboxstart::Result::Indeterminate?State::Indeterminate:State::Refused;
        return result==sandboxstart::Result::Forwarded;
    }
    template<class Forward> startroute::Result Handle(startroute::SlotKind kind,std::uint32_t thread,
        std::uintptr_t receiver,std::uintptr_t frame,std::uintptr_t result,Forward&& original){
        if(!router_){original();return startroute::Result::Forwarded;}
        // Keep borrowed frame callback-local even when native Start broadcasts
        // notifications. A nested owned Start is consumed by Router, not allowed
        // to overwrite the outer frame or queue a second travel.
        if(invoking_)return router_->Dispatch(reader_,*this,kind,thread,receiver,frame,std::forward<Forward>(original));
        struct Scope {NativeMenuLaunch& self;~Scope(){self.frame_={};self.invoking_=false;}} scope{*this};
        invoking_=true;frame_={receiver,frame,result,kind};
        return router_->Dispatch(reader_,*this,kind,thread,receiver,frame,std::forward<Forward>(original));
    }
    State CurrentState()const noexcept{return state_;}
#if CFG_H20_MENU && !CFG_H21_LOBBY_DISCOVERY
    bool StoreOnlineChoices(){return hostedsettings::Save(view_.Rows(),view_,session_.Generation(),cfghostedoptions::Current());}
#endif
    bool RetireRowsForMode(const hud::Binding& current,std::uint64_t now){
        if(invoking_||!reentry::SameWidget(context_,current.context))return false;
#if CFG_H20_MENU && !CFG_H21_LOBBY_DISCOVERY
        if(view_.State()==rows::ViewState::Ready&&!StoreOnlineChoices())return false;
#endif
        if(view_.State()==rows::ViewState::Closed)return true;
        if(view_.State()!=rows::ViewState::Retiring&&(!view_.Rebind(current)||!view_.Close()))return false;
        view_.After(current.context.hud,current.event,current.context.thread,false,true,now);
        return view_.State()==rows::ViewState::Closed;
    }
    const active::Context& Context()const noexcept{return context_;}
    bool Resume(const hud::Binding& current,const cfgprimary::SoftClassBytes& counselor){
        if(invoking_||(state_!=State::Ready&&state_!=State::Opening)||!softpath::Plain(counselor)||!reentry::SameWidget(context_,current.context))return false;
        auto rebound=binding_;rebound.context=current.context;
        if(binding_.start.field.object&&!startbinding::Current(reader_,rebound,engine_.Thread(),wrapper_,graph_))return false;
        launchselection::Observation fresh;
        if(!launchselection::Observe(reader_,current.context,engine_.Thread(),session_.Generation(),catalogueGeneration_,
            selections_.evidence.maximumBots,true,fresh)||!acceptedcounselor::Apply(fresh.evidence,counselor))return false;
        catalogues::Modes modes;
        if(!catalogues::ObserveModes(reader_,current.context,true,modes)||modes.selectedMap!=fresh.evidence.map||
           !ModesForOrigin(origin_,modes,fresh.map.mode)||!view_.Rebind(current))return false;
        const auto& e=fresh.evidence;
        if(session_.Current()==Stage::Settings&&
           !session_.Repick(current.context.widget,session_.Generation(),e.map,e.jason,e.counselor,e.preserveNativeCounselor))return false;
        context_=current.context;binding_=rebound;transport_=current;selections_=fresh;acceptedCounselor_=counselor;return true;
    }
    unsigned PreparationStage()const noexcept{return preparation_.Stage()+1;}
    unsigned PreparationFailure()const noexcept{return preparationFailure_;}
    const launchselection::Diagnostic& SelectionDiagnostic()const noexcept{return selectionDiagnostic_;}
    unsigned WarmStage()const noexcept{return view_.WarmStage();}
    unsigned RowViewState()const noexcept{return static_cast<unsigned>(view_.State());}
    unsigned RowPrepareStage()const noexcept{return view_.PrepareStage();}
    const startbinding::Binding& StartBinding()const noexcept{return binding_;}
    const frontend::HudReport& HudReport()const noexcept{return view_.Report();}
    GameplayHandoff& PendingGameplay()noexcept{return handoff_;}
    // No blind Reset: the frontend retains one owner per positive widget
    // identity and resumes its existing rows, rather than appending duplicates.
};
}
