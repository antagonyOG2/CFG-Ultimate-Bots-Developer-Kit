#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendSetupRuntime.hpp"
#include <memory>
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    rows::Spec spec;spec.id=RowId::Duration;spec.title=u"Match Length";spec.count=2;spec.selected=1;
    spec.labels[0]=u"30 minutes";spec.labels[1]=u"60 minutes";
    {auto f=std::make_unique<HudFixture>();OptionsReader reader{f->fixture.r};ModelEngine engine{*f};
        auto text=std::make_unique<reflection::RetainedMenuText>();frontend::SetupRuntime<OptionsReader,ModelEngine> view(reader,engine,*text);
        active::Context c;hud::Binding bound;check(active::Observe(reader,MenuFixture::Base,c)&&hud::Observe(reader,c,true,bound),"existing current transport binding");
        auto wrong=bound;wrong.context.widget+=8;
        check(!view.Open(c,19,&spec,1,true,&wrong)&&!engine.calls,"other transport owner refuses before row preparation");
        check(view.Open(c,19,&spec,1,true,&bound)&&!engine.calls,"reuse exact directly revalidated transport without cold HUD rediscovery");
    }
    for(unsigned fault=0;fault<3;++fault){auto f=std::make_unique<HudFixture>();OptionsReader reader{f->fixture.r};ModelEngine engine{*f};
        auto text=std::make_unique<reflection::RetainedMenuText>();auto view=std::make_unique<frontend::SetupRuntime<OptionsReader,ModelEngine>>(reader,engine,*text);
        active::Context context;check(active::Observe(reader,MenuFixture::Base,context),"exact current native owner");
        check(!view->Open(context,19,&spec,1,false)&&!engine.calls,"unverified image refuses before view build");
        check(view->Open(context,19,&spec,1,true),"concrete HUD provider opens real native menu view");
        for(unsigned i=0;i<3;++i)check(view->After(HudFixture::HUD,HudFixture::Event,42,false,true,i*25),"three observation callbacks");
        check(!engine.calls&&view->State()==rows::ViewState::Warming,"no mutations before real event recurrence");
        if(!fault){hud::Binding resume;check(hud::Observe(reader,context,true,resume)&&view->Rebind(resume),"Back/return during warmup rebinds the same positively owned tree");
            for(unsigned i=0;i<3;++i)check(view->After(HudFixture::HUD,HudFixture::Event,42,false,true,i*25),"resumed warmup establishes fresh HUD recurrence");}
        if(fault==1)f->fixture.r.Set(HudFixture::Event+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0});
        if(fault==2)f->fixture.r.Set(HudFixture::Registry+24*4+16,999);
        if(fault){check(!view->After(HudFixture::HUD,HudFixture::Event,42,false,true,75)&&!engine.calls&&
            view->Report().state==frontend::HudState::Refused,"changed ABI/HUD stops before native construction");continue;}
        for(unsigned i=0;i<11;++i)check(view->After(HudFixture::HUD,HudFixture::Event,42,false,true,75+i*25),"one production warmup/row phase");
        check(view->State()==rows::ViewState::Ready&&engine.creates==1&&engine.attaches==1,"full native row reaches ready through exact event provider");
        const auto reads=f->fixture.r.reads,calls=engine.calls;
        for(unsigned i=0;i<1000;++i)view->After(HudFixture::HUD,HudFixture::Event,42,false,true,1000+i*25);
        check(f->fixture.r.reads==reads&&engine.calls==calls,"finished menu no ongoing scans/calls");
        std::array<unsigned,rows::Capacity> choices;check(view->Rows().Choices(*view,19,choices)&&choices[0]==1,"native Start can atomically read real addon choice");
        check(view->Close()&&view->State()==rows::ViewState::Retiring,"departure holds until paced own-row cleanup");
        for(unsigned i=0;i<5;++i)check(view->After(HudFixture::HUD,HudFixture::Event,42,false,true,40000+i*25),"fresh recurrence then own-row retirement");
        check(view->State()==rows::ViewState::Closed&&engine.removes==1,"cached native menu cleanup completes before departure");
        check(view->Open(context,20,&spec,1,true),"same cached menu reopens with new generation");
    }
    std::printf("CFG exact HUD -> native setup integration: %s (%u failures). Owned engine only; no live hook/install.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
