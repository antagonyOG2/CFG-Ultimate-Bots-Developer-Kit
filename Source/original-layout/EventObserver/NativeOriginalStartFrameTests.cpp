#define CFGMENU_START_BINDING_FIXTURE_ONLY
#include "SettingsStartBindingTests.cpp"
#include "NativeOriginalStartFrame.hpp"
template class cfgmenu::startroute::NativeOriginalStart<OptionsReader>;
int main(){using namespace cfgmenu;using namespace startroute;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned branch=0;branch<2;++branch)for(unsigned fault=0;fault<10;++fault){
        auto f=std::make_unique<StartBindingFixture>();OptionsReader reader{f->fixture.r};active::Context context;
        check(active::Observe(reader,MenuFixture::Base,context),"current menu fixture");
        auto scratch=std::make_unique<startbinding::Workspace>();startbinding::Binding binding;
        check(startbinding::Observe(reader,context,true,*scratch,binding),"complete native Start evidence");
        OriginalFrame frame{context.widget,0x2100000,0,branch?SlotKind::Graph:SlotKind::Wrapper};
        const auto& slot=branch?binding.graph:binding.start;const std::uintptr_t locals=0x2110000;
        auto& r=f->fixture.r;
        r.Set(frame.frame+startdisk::FrameNode,slot.field.object);r.Set(frame.frame+startdisk::FrameObject,context.widget);
        r.Set(frame.frame+startdisk::FrameCode,slot.script.data);r.Set(frame.frame+startdisk::FrameCurrentNative,slot.field.object);
        r.Set(frame.frame+startdisk::FrameLocals,locals);r.Set(locals,startdisk::StartEntry);
        unsigned thread=context.thread;
        if(fault==1)thread++;
        if(fault==2)frame.receiver+=8;
        if(fault==3)frame.frame=0;
        if(fault==4)r.Set(frame.frame+startdisk::FrameNode,binding.construct.field.object);
        if(fault==5)r.Set(frame.frame+startdisk::FrameObject,context.widget+8);
        if(fault==6)r.Set(frame.frame+startdisk::FrameCode,slot.script.data+1);
        if(fault==7)r.Set(frame.frame+startdisk::FrameCurrentNative,binding.construct.field.object);
        if(fault==8)r.Set(frame.frame+startdisk::FrameLocals,std::uintptr_t{0});
        if(fault==9)r.Set(locals,startdisk::ConstructEntry);
        const bool expected=fault==0||(!branch&&fault>=8);
        check(CurrentFrame(reader,binding,frame,thread)==expected,"only exact live Start frame admitted, never Construct/stepped/foreign frame");
        // The fixture module is not the host EXE. Even correct synthetic metadata
        // must never become executable authority. This compiles the real adapter.
        NativeOriginalStart<OptionsReader> native(reader,binding);
        const auto before=f->fixture.r.reads;
        check(!native.Prepare(frame,true)&&!native.Forward()&&!native.Called()&&before==f->fixture.r.reads,
            "synthetic module refuses Windows native call without reads or execution");
    }
    std::printf("CFG original Start live-frame/Windows forwarder: %s (%u failures). No native game calls.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
