#define CFGMENU_ENTRY_FIXTURE_ONLY
#include "OfflineEntryRuntimeTests.cpp"
#undef CFGMENU_ENTRY_FIXTURE_ONLY
#include "NativeOfflineEntryReturnPolicy.hpp"
int main(){using namespace cfgmenu;using F=EntryFixture;using M=MenuFixture;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* why){++checks;if(!okay){++failures;std::printf("FAIL %s\n",why);}};
    auto f=std::make_unique<F>();OptionsReader reader{f->fixture.r};auto& memory=f->fixture.r;
    memory.Set(F::Stack+execdisk::StackArray,active::StackArray{F::Records,2,2});
    memory.Set(F::Records+execdisk::RecordBytes,M::Type);memory.Set(F::Records+execdisk::RecordBytes+execdisk::RecordWidget,M::Widget);
    active::Context original;check(active::Observe(reader,M::Base,original),"original actual native context");
    auto resources=std::make_unique<entrysettings::Resources>();entry::Binding binding;unsigned stage=0;
    check(entrysettings::Observe(reader,original,42,true,*resources)&&entry::Observe(reader,*resources,binding,&stage),"original native source and label binding");
    if(!binding.source)return 1;
    ModelEngine engine{*f};auto text=std::make_unique<reflection::RetainedMenuText>();
    auto labels=std::make_unique<entry::Labels<OptionsReader,ModelEngine>>(reader,engine,*text,binding);
    for(unsigned i=0;i<6;++i)check(labels->Service(i*25),"initial existing native label phase");
    check(labels->Ready()&&engine.texts==2,"initial labels ready");
    const auto hook=std::uintptr_t{0x3800000};memory.Set(F::SandboxClick+startdisk::Executor,hook);
    // Model the real native pop/recreation: fresh positive entry and child slots,
    // SAME retained class record. Old instance is not used as return authority.
    for(unsigned visit=0;visit<2;++visit){const std::uintptr_t widget=0x3900000+visit*0x10000,sandbox=widget+0x1000,jason=widget+0x2000;
        f->Register(widget,600+static_cast<int>(visit));f->Register(sandbox,610+static_cast<int>(visit));f->Register(jason,620+static_cast<int>(visit));
        memory.Set(widget+cfgprobe::ObjectClass,M::Type);memory.Set(widget+0x420,sandbox);memory.Set(widget+0x418,jason);
        memory.Set(sandbox+cfgprobe::ObjectClass,F::ButtonType);memory.Set(jason+cfgprobe::ObjectClass,F::ButtonType);
        memory.Set(sandbox+0x400,F::Title);memory.Set(jason+0x400,F::Value);
        memory.Set(F::Records+execdisk::RecordBytes,original.type);memory.Set(F::Records+execdisk::RecordBytes+execdisk::RecordWidget,widget);
        active::Context actual;check(active::Observe(reader,M::Base,actual)&&actual.widget==widget,"actual newly returned native instance");
        check(!entry::Current(reader,binding,42),"old native widget and child binding is revoked");
        entry::Binding rebound;check(entry::Rebind(reader,binding,actual,rebound)&&rebound.sandbox==sandbox&&rebound.jason==jason,
            "fresh children through unchanged exact authored descriptors with owned callback still installed");
        if(!rebound.source)return 1;
        check(!entry::Observe(reader,*resources,rebound,&stage),"cold original-executor discovery is not a return mechanism");
        check(entry::Rebind(reader,binding,actual,rebound),"rebind does not broaden original Observe");
        const auto propertyFlags=binding.sandboxMember.flags;memory.Set(F::SandboxField+cfgreflection::disk::PropertyFlags,propertyFlags^1);
        entry::Binding refused;check(!entry::Rebind(reader,binding,actual,refused),"changed native member descriptor refuses return");
        memory.Set(F::SandboxField+cfgreflection::disk::PropertyFlags,propertyFlags);
        std::int32_t index=0;memory.Read(sandbox+cfgprobe::lifetime::ObjectIndexOffset,index);
        memory.Set(F::Registry+24*index+16,0);check(!entry::Rebind(reader,binding,actual,refused),"zero serial child cannot inherit old label authority");
        memory.Set(F::Registry+24*index+16,610+static_cast<int>(visit));
        binding=rebound;resources->source=actual;
        labels=std::make_unique<entry::Labels<OptionsReader,ModelEngine>>(reader,engine,*text,binding);
        const auto texts=engine.texts;for(unsigned i=0;i<6;++i)check(labels->Service(1000+visit*1000+i*25),"returned native label phase");
        check(labels->Ready()&&engine.texts==texts+2&&!engine.creates&&!engine.attaches,"both fresh labels rebuilt with no widget creation");
        std::uintptr_t executor=0;memory.Read(F::SandboxClick+startdisk::Executor,executor);check(executor==hook,"return never republishes original or owned callback");
    }
    std::printf("OfflinePlay native return/rebind/relabel memory path: %s (%u checks; owned memory only).\n",failures?"FAIL":"PASS",checks);
    return failures?1:0;
}
