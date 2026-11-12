#define CFGMENU_START_BINDING_FIXTURE_ONLY
#include "SettingsStartBindingTests.cpp"
#include "../RuntimeProbe/BorrowedSettingsStartObservation.hpp"
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned branch=0;branch<2;++branch)for(unsigned fault=0;fault<16;++fault){
        auto fixture=std::make_unique<StartBindingFixture>();auto& f=*fixture;OptionsReader reader{f.fixture.r};auto& r=f.fixture.r;
        active::Context context;check(active::Observe(reader,MenuFixture::Base,context),"active owner");
        auto scratch=std::make_unique<startbinding::Workspace>();startbinding::Binding binding;
        check(startbinding::Observe(reader,context,true,*scratch,binding),"Start binding");
        // These are real UObject dependencies, not array/vtable/native pointers.
        for(const auto object:{context.mode,context.stack,context.gameInstance,context.player,context.type,MenuFixture::FunctionClass,MenuFixture::IntProperty,StartBindingFixture::Entry})f.Register(object);
        // Register can extend the modeled registry; refresh only its count.
        check(cfgprobe::lifetime::ReadArrayHeader(reader,MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva,binding.context.registry),"fresh registry header");
        startroute::OriginalFrame frame{context.widget,0x2100000,0,branch?startroute::SlotKind::Graph:startroute::SlotKind::Wrapper};
        const auto& slot=branch?binding.graph:binding.start;constexpr std::uintptr_t locals=0x2110000;
        r.Set(frame.frame+startdisk::FrameNode,slot.field.object);r.Set(frame.frame+startdisk::FrameObject,context.widget);
        r.Set(frame.frame+startdisk::FrameCode,slot.script.data);r.Set(frame.frame+startdisk::FrameCurrentNative,slot.field.object);
        r.Set(frame.frame+startdisk::FrameLocals,locals);r.Set(locals,startdisk::StartEntry);
        r.Set(MenuFixture::Base+constructdisk::Invoke,constructdisk::InvokeCode);
        auto evidence=std::make_unique<startboundary::Evidence>();
        unsigned stage=99;
        const bool observed=startboundary::Observe(reader,binding,frame,MenuFixture::Base+constructdisk::Return,42,
            MenuFixture::Base+startdisk::ScriptExecutor,MenuFixture::Base+startdisk::ScriptExecutor,*evidence,&stage);
        if(!observed&&!fault)std::printf("BorrowedStart branch%u stage%u reads%u\n",branch,stage,r.reads);
        check(observed,"native wrapper/graph origin plus exact dependency slots");
        check(evidence->count>=9,"source and field owners recorded");
        r.Set(frame.frame+startdisk::FrameCode,slot.script.data+1);
        if(fault==1)evidence->returnPc++;
        if(fault==2)r.Set(frame.frame+startdisk::FrameNode,binding.construct.field.object);
        if(fault==3)r.Set(frame.frame+startdisk::FrameCurrentNative,binding.construct.field.object);
        if(fault==4)r.Set(frame.frame+startdisk::FrameObject,context.widget+8);
        if(fault==5)r.Set(slot.field.object+startdisk::Executor,MenuFixture::Base+startdisk::ScriptExecutor+8);
        if(fault==6)r.Set(context.widget+cfgprobe::ObjectClass,MenuFixture::FunctionClass);
        if(fault==7)r.Set(MenuFixture::Base+constructdisk::Invoke,std::uint8_t{0});
        if(fault==9)r.Set(context.stack+execdisk::StackArray,active::StackArray{});
        if(fault==10)r.Set(context.viewport+cfgprobe::ViewportWorld,std::uintptr_t{0x2900000});
        if(fault==11){const auto top=context.array.data+static_cast<std::uintptr_t>(context.array.count-1)*execdisk::RecordBytes+execdisk::RecordWidget;r.Set(top,std::uintptr_t{0x2900000});}
        if(fault==12)r.Set(context.player+cfgprobe::PlayerController,std::uintptr_t{0x2900000});
        if(fault==13)r.Set(context.players,std::uintptr_t{0x2900000});
        if(fault==14)r.Set(context.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{0});
        if(fault==15)r.Set(context.module+cfgprobe::EngineGlobalCandidate,std::uintptr_t{0x2900000});
        check(startboundary::Current(reader,*evidence,fault==8?43u:42u)==(fault==0),"strict pre-entry proof unchanged");
        const bool expectedReturned=fault==0||(fault>=9&&fault<=12);
        check(startboundary::ReturnedInvocation(reader,*evidence,fault==8?43u:42u)==expectedReturned,"post-return permits only menu/world/controller retirement, rejects origin/frame/infra drift");
    }
    std::printf("Borrowed original settings Start lifetime source: %u failures; owned memory only.\n",failures);return failures?1:0;
}
