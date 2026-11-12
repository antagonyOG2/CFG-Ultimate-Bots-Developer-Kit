#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeChallengeRowsFixture.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
int main(){unsigned failures=0;const auto check=[&](bool ok,const char* label){if(!ok){++failures;std::printf("FAIL %s\n",label);}};
 auto f=std::make_unique<NativeFixture>();f->fixture.Name(MenuFixture::Type,"SPChallengesSettingsMenuWidget_C");
 OptionsReader reader{f->fixture.r};ChallengeRowsEngine engine{*f};auto text=std::make_unique<cfgmenu::reflection::RetainedMenuText>();
 f->fixture.r.Set(MenuFixture::Donor+0x188,NativeFixture::Title);
 f->fixture.r.Set(NativeFixture::Title+cfgprobe::ObjectClass,NativeFixture::TextType);
 cfgmenu::active::Context context;check(cfgmenu::active::Observe(reader,MenuFixture::Base,context),"current Challenge page ownership");
 cfgmenu::rows::NativeAccess<OptionsReader,ChallengeRowsEngine> ordinary(reader,engine,*text);
 check(!ordinary.Prepare(context,1)&&!engine.calls,"ordinary OfflineBots provider does not inherit Challenge ownership");
 cfgmenu::rows::NativeSetup<OptionsReader,ChallengeRowsEngine> view(reader,engine,*text);
 const auto specs=cfgmenu::challengemenu::Specs();
 check(view.OpenForClass(context,1,specs.data(),static_cast<unsigned>(specs.size()),true),"actual shipping Challenge UI provider opens eight donor rows");
 for(unsigned i=0;i<48;++i)view.Service(25*i);
 check(view.State()==cfgmenu::rows::ViewState::Ready&&engine.creates==7&&engine.texts==8,"native first focus row and seven distinct deep-copied clones reach Ready");
 check(view.Rows().At(7).spec.id==cfgmenu::RowId::Boundary&&view.Rows().At(7).spec.title==u"Disable Jason Boundary","shipping boundary row identity/title connected");
 const auto& replay=view.Rows().At(5).spec;
 check(replay.id==cfgmenu::RowId::SkipReplay&&view.Rows().At(6).spec.id==cfgmenu::RowId::PreventEscapes,"donor replay row precedes escape prevention");
 if(CFG_J22_CHALLENGES_RETENTION&&!CFG_J22_CHALLENGE_REPLAY)
  check(replay.title==u"Skip Intro on Replay (Unavailable)"&&replay.labels[0]==u"Unavailable"&&replay.labels[1]==u"Unavailable","unverified replay is explicitly unavailable at both native indices");
 else check(replay.title==u"Skip Intro on Replay"&&replay.labels[0]==u"Off"&&replay.labels[1]==u"On","verified legacy replay retains donor labels");
 check(view.Rows().At(0).widget==MenuFixture::Donor&&view.Rows().At(0).attached,"first row retains actual authored native focus owner, not a hidden replacement");
 std::array<unsigned,cfgmenu::rows::Capacity> choices{};
 check(view.Rows().Choices(view,1,choices)&&choices[0]==0&&choices[4]==0&&choices[5]==0&&choices[6]==0&&choices[7]==0,"all eight options start Off");
 f->fixture.r.Set(view.Rows().At(7).widget+0x210,int{1});
 check(view.Rows().Choices(view,1,choices)&&choices[7]==1&&choices[6]==0&&choices[5]==0,"boundary choice independent of escape prevention and replay skip");
 f->fixture.r.Set(view.Rows().At(5).widget+0x210,int{1});
 check(view.Rows().Choices(view,1,choices)&&choices[5]==1&&choices[6]==0,"replay skip choice independent of escape prevention");
 f->fixture.r.Set(view.Rows().At(5).widget+0x210,int{0});
 for(unsigned index=0;index<8;++index){
  f->fixture.r.Set(view.Rows().At(index).widget+0x210,int{1});
  check(view.Rows().Choices(view,1,choices)&&choices[index]==1,"every actual row selection readback including native first row");
 }
 f->fixture.r.Set(MenuFixture::Donor+0x210,int{0});
 check(view.Rows().Choices(view,1,choices)&&choices[0]==0&&choices[4]==1&&choices[5]==1&&choices[6]==1&&choices[7]==1,"first native row remains independent after changing every subsequent row");
 const auto calls=engine.calls,reads=f->fixture.r.reads;for(unsigned i=0;i<1000;++i)view.Service(1000+i*25);
 check(engine.calls==calls&&f->fixture.r.reads==reads,"ready page performs no idle rebuilds or native calls");
 f->fixture.r.Set(engine.widgets[2]+0x210,int{2});check(!view.Rows().Choices(view,1,choices)&&choices[0]==0,"invalid native index rejects whole commit");
 std::printf("Challenge eight-row production native UI/ownership/deep-copy/choice path: %u failures. Synthetic engine only.\n",failures);return failures?1:0;
}
