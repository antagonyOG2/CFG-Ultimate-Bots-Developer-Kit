#pragma once
#include "NativeHumanJasonOptionsRuntime.hpp"
#include "FrontendSetupRuntime.hpp"
#include "NativeSettingsStartSlots.hpp"
#include "NativeGraphEntryBoundary.hpp"
#include "NativeHumanJasonRouteObservation.hpp"
namespace cfgmenu::humanjason {
inline std::array<rows::Spec,6> Specs(){
 std::array<rows::Spec,6> result{};
 result[0].id=RowId::Skill;result[0].title=u"Counselor Bot Skill";result[0].count=3;result[0].selected=1;result[0].reuseNativeSkill=true;
 result[0].labels[0]=u"Low";result[0].labels[1]=u"Medium";result[0].labels[2]=u"High";
 result[1].id=RowId::Difficulty;result[1].title=u"Jason Difficulty";result[1].count=3;result[1].selected=1;
 result[1].labels[0]=u"Easy";result[1].labels[1]=u"Normal";result[1].labels[2]=u"Hard";
 result[2].id=RowId::Duration;result[2].title=u"Match Length";result[2].count=4;result[2].selected=1;
 result[2].labels[0]=u"30 minutes";result[2].labels[1]=u"60 minutes";result[2].labels[2]=u"90 minutes";result[2].labels[3]=u"Unlimited";
 result[3].id=RowId::Knives;result[3].title=u"Unlimited Throwing Knives";
 result[4].id=RowId::Recharge;result[4].title=u"Faster Ability Recharge";
 result[5].id=RowId::Movement;result[5].title=u"Faster Jason Movement (2x)";
 for(unsigned i=3;i<result.size();++i){result[i].labels[0]=u"Off";result[i].labels[1]=u"On";result[i].count=2;}
 for(auto& spec:result)spec.preserveNativeValue=true;return result;
}
inline bool OptionsForChoices(const std::array<unsigned,rows::Capacity>& choices,Options& result){
 if(choices[0]>2||choices[1]>2||choices[2]>3||choices[3]>1||choices[4]>1||choices[5]>1)return false;
 result={choices[3]!=0,choices[5]!=0,choices[4]!=0,choices[2]==3?UINT32_MAX:(choices[2]+1)*1800,
     choices[0]==0?1u:choices[0]==1?0u:2u,choices[1]};return true;
}
template<class Reader>bool NativeRoute(Reader& r,const active::Context& context){
 maps::Selection selection;cfgnames::Text name;cfgnames::Observation<Reader> names(r,context.module);
 return maps::Observe(r,context,context.thread,true,selection)&&names.Object(selection.mode,name)&&name.View()==u"ModeDef_OfflineBots_C";
}
// Uses the existing native rows/Start ABI, but NO counselor Session/Handoff.
template<class Reader,class Engine>class Menu {
 Reader& r_;Engine& engine_;frontend::SetupRuntime<Reader,Engine> view_;
 active::Context context_{};hud::Binding hud_{};startbinding::Binding start_{};
 startslots::Slots slots_;HMODULE resident_=nullptr;std::uintptr_t wrapper_=0,graph_=0;
 unsigned phase_=0,failure_=0;bool failed_=false,invoking_=false,submitted_=false,inherited_=false;
 void Fail(unsigned why){failed_=true;failure_=why;}
public:
 Runtime gameplay;
 Menu(Reader& r,Engine& engine,reflection::RetainedMenuText& text):r_(r),engine_(engine),view_(r,engine,text){}
 const active::Context& Context()const{return context_;}
 const startbinding::Binding& StartBinding()const{return start_;}
 bool Open(const active::Context& context,const hud::Binding& hud,HMODULE resident,std::uintptr_t wrapper,std::uintptr_t graph,const startbinding::Binding* inherited){
  if(context_.widget||!NativeRoute(r_,context)||!(hud.context==context)||!hud::Current(r_,hud,engine_.Thread()))return false;
  context_=context;hud_=hud;resident_=resident;wrapper_=wrapper;graph_=graph;
  if(inherited){start_=*inherited;start_.context=context;inherited_=true;}return true;
 }
 bool Owns(std::uintptr_t receiver)const{return context_.widget==receiver;}
 bool Ready()const{return !failed_&&!submitted_&&phase_==2&&view_.State()==rows::ViewState::Ready;}
 bool Failed()const{return failed_;}
 unsigned Stage()const{return failed_?3u:submitted_?4u:Ready()?2u:phase_?1u:0u;}
 unsigned Failure()const{return failure_;}
 bool Published()const{return slots_.Current()==startslots::State::Installed;}
 bool RetireRowsForMode(const hud::Binding& current,std::uint64_t now){
  if(invoking_||!reentry::SameWidget(context_,current.context))return false;
  if(view_.State()==rows::ViewState::Closed)return true;
  if(view_.State()!=rows::ViewState::Retiring&&(!view_.Rebind(current)||!view_.Close()))return false;
  view_.After(current.context.hud,current.event,current.context.thread,false,true,now);
  return view_.State()==rows::ViewState::Closed;
 }
 bool Resume(const active::Context& context,const hud::Binding& hud){
  if(!reentry::SameWidget(context_,context)||failed_||!(hud.context==context)||
   !hud::Current(r_,hud,engine_.Thread())||!NativeRoute(r_,context)||!view_.Rebind(hud))return false;
  if(submitted_){
   if(!gameplay.ResetForVerifiedMenu(context))return false;
   submitted_=false;
  }
  context_=context;hud_=hud;start_.context=context;return true;
 }
 void After(std::uintptr_t object,std::uintptr_t function,std::uint32_t thread,bool nested,bool completed,std::uint64_t now){
  if(failed_||submitted_||nested||!completed||thread!=context_.thread||object!=context_.hud||function!=hud_.event)return;
  if(!phase_){
   if(!inherited_){auto work=std::make_unique<startbinding::Workspace>();if(!startbinding::Observe(r_,context_,true,*work,start_)){Fail(1);return;}}
   startslots::NativeAccess<Reader> access(r_,start_,resident_,true);
   if(!(inherited_?slots_.Adopt(access,context_.module+startdisk::ScriptExecutor,wrapper_,graph_):
      slots_.Install(access,context_.module+startdisk::ScriptExecutor,wrapper_,graph_))){Fail(2);return;}
   phase_=1;return;
  }
  if(phase_==1){const auto specs=Specs();if(!view_.Open(context_,1,specs.data(),static_cast<unsigned>(specs.size()),true,&hud_)){Fail(3);return;}phase_=2;return;}
  view_.After(object,function,thread,nested,completed,now);
  if(view_.State()==rows::ViewState::Failed||view_.State()==rows::ViewState::Detached)Fail(4);
 }
 template<class Forward>void Start(startroute::SlotKind kind,std::uint64_t now,Forward forward){
  if(invoking_){forward();return;}
  if(kind!=startroute::StartBoundary(start_)){forward();return;}
  if(!Ready()||submitted_){if(!failed_)failure_=5;return;}
  if(!startbinding::Current(r_,start_,engine_.Thread(),wrapper_,graph_)||!NativeRoute(r_,context_)){failure_=6;return;}
  std::array<unsigned,rows::Capacity> choices{};
  Options options;
  if(!view_.Rows().Choices(view_,1,choices)||!OptionsForChoices(choices,options)||
     !gameplay.Arm(context_,options,now)){failure_=7;return;}
  submitted_=true;invoking_=true;failure_=0;
  struct Clear{bool& v;~Clear(){v=false;}}clear{invoking_};forward();
 }
};
}
