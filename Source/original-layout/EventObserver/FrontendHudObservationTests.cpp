#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "../RuntimeProbe/FrontendHudObservation.hpp"
#include "FrontendHudGate.hpp"
struct HudFixture : NativeFixture {
    static constexpr std::uintptr_t HudType=0x1600000,NativeHud=0x1610000,Vtable=0x1620000,Event=0x1630000;
    HudFixture(){using namespace cfgmenu;auto& f=fixture;auto& r=f.r;
        r.Set(HUD+cfgprobe::ObjectClass,HudType);Root(NativeHud);r.Set(HudType+cfgprobe::ClassSuper,NativeHud);
        r.Set(HUD,Vtable);r.Set(Vtable+transportdisk::DispatcherSlot,MenuFixture::Base+transportdisk::ActorDispatcher);
        r.Set(Vtable+transportdisk::DrawSlot,MenuFixture::Base+transportdisk::Draw);
        r.Set(MenuFixture::Base+transportdisk::HudClass,NativeHud);r.Set(MenuFixture::Base+transportdisk::EventCache,Event);
        r.Set(MenuFixture::Base+transportdisk::Draw,transportdisk::DrawCode);
        r.Set(MenuFixture::Base+transportdisk::Wrapper,transportdisk::WrapperCode);
        r.Set(HudType+cfgreflection::disk::Children,std::uintptr_t{0});r.Set(NativeHud+cfgreflection::disk::Children,Event);
        f.Field(Event,NativeHud,MenuFixture::FunctionClass,"ReceiveDrawHUD",0);Register(Event);
        cfgnames::Words name{};r.Read(Event+cfgnames::disk::ObjectName,name);r.Set(MenuFixture::Base+transportdisk::EventName,name);
        r.Set(Event+cfgreflection::disk::Children,Event+0x100);
        r.Set(Event+cfgreflection::disk::FunctionFlags,transportdisk::EventFlags);
        r.Set(Event+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});
        r.Set(Event+cfgreflection::disk::StorageBytes,8);r.Set(Event+0xB0,MenuFixture::Base+transportdisk::BlueprintExecutor);
        r.Set(Event+0x68,std::uintptr_t{0});r.Set(Event+scriptdisk::Script,capture::ScriptHeader{});
        for(unsigned i=0;i<2;++i){const auto at=Event+0x100+0x100*i;
            f.Field(at,Event,MenuFixture::IntProperty,i?"SizeY":"SizeX",i?0:at+0x100);f.Property(at,4,static_cast<int>(4*i));
            r.Set(at+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000280ULL});
        }
    }
};
struct ExactHudHost {
    OptionsReader& reader;const cfgmenu::hud::Binding& binding;unsigned services=0;bool done=false;
    bool FrontendMatches(const cfgmenu::frontend::HudDescriptor& d){return d.event==binding.event&&d.context==binding.context&&
        cfgmenu::hud::Current(reader,binding,42);}
    bool Service(std::uint64_t){++services;done=true;return true;}
    bool SetupFinished()const{return done;}bool SetupRefused()const{return false;}
};
#ifndef CFGMENU_HUD_FIXTURE_ONLY
#ifndef CFG_HUD_FIXTURE_ONLY
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;
    for(unsigned fault=0;fault<15;++fault){HudFixture f;OptionsReader reader{f.fixture.r};active::Context context;
        check(active::Observe(reader,MenuFixture::Base,context),"current owned frontend");
        const auto event=HudFixture::Event;auto& r=f.fixture.r;
        if(fault==1)r.Set(event+cfgreflection::disk::FunctionFlags,std::uint32_t{0});
        if(fault==2)r.Set(event+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{16});
        if(fault==3)r.Set(event+0xB0,MenuFixture::Base+0x7AA0F0);
        if(fault==4)r.Set(event+0x68,event+0x100);
        if(fault==5)r.Set(event+0x100+cfgreflection::disk::PropertyOffset,4);
        if(fault==6)r.Set(event+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0010000000000080ULL});
        if(fault==7)r.Set(MenuFixture::Base+transportdisk::EventCache,event+0x100);
        if(fault==8)r.Set(HudFixture::Vtable+transportdisk::DrawSlot,MenuFixture::Base+transportdisk::Draw+16);
        if(fault==9)r.Set(MenuFixture::Base+transportdisk::Wrapper,std::uint8_t{0});
        if(fault==10)r.Set(event+scriptdisk::Script,capture::ScriptHeader{0x1700000,1,1});
        if(fault==11)r.Set(NativeFixture::Registry+24*4+16,0); // HUD is the fifth registered object.
        if(fault==12)r.Set(MenuFixture::Base+transportdisk::EventCache,std::uintptr_t{0}); // lazy cache may be empty, never called.
        if(fault==14)r.Set(event+0xB0,MenuFixture::Base+0x7AA0E0); // verified 3.4 executor is NOT valid in 3.5.
        hud::Binding binding;unsigned consumed=0;
        const auto okay=hud::Observe(reader,context,fault!=13,binding,&consumed);
        check(okay==(fault==0||fault==12),"exact event/code/ABI/owner rejection");check(consumed<=hud::ReadLimit,"hard setup read budget");
        if(!okay){check(!binding.event,"refusal publishes no stale binding");continue;}
        check(hud::Current(reader,binding,42)&&!hud::Current(reader,binding,41),"exact descriptor revalidates on owner thread only");
        check(hud::CompletedNativeCallback(reader,binding,NativeFixture::HUD,event,
            MenuFixture::Base+transportdisk::ReturnPc,42,false,true),"exact completed native HUD callback provenance");
        for(unsigned origin=0;origin<7;++origin){
            const auto reads=r.reads;
            check(!hud::CompletedNativeCallback(reader,binding,
                origin==0?NativeFixture::Controller:NativeFixture::HUD,origin==1?event+0x100:event,
                MenuFixture::Base+transportdisk::ReturnPc+(origin==2?1:origin==6?16:0),
                origin==3?41:42,origin==4,origin!=5)&&r.reads==reads,
                "wrong/nested/unreturned/native-origin HUD callback refuses before target reads");
        }
        std::printf("HUD preparation reads=%u\n",consumed);
        frontend::HudGate gate;check(gate.Bind({context,binding.event},true),"real provider binds staged pacing gate");
        ExactHudHost host{reader,binding};const auto before=r.reads;
        for(unsigned i=0;i<1000;++i)gate.After(host,NativeFixture::Controller,binding.event,42,false,true,i*25);
        check(r.reads==before&&!host.services,"unrelated dispatch performs zero reads");
        for(unsigned i=0;i<4;++i)check(gate.After(host,NativeFixture::HUD,binding.event,42,false,true,100+i*25),"three recurrence samples then one phase");
        check(host.services==1&&gate.Report().state==frontend::HudState::Finished,"ready disables HUD service");
        const auto idle=r.reads;for(unsigned i=0;i<1000;++i)gate.After(host,NativeFixture::HUD,binding.event,42,false,true,1000+i*25);
        check(idle==r.reads,"ready performs zero metadata reads");
        r.Set(event+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0});
        check(!hud::Current(reader,binding,42),"metadata serial zero never permits stale ABI");
        check(!hud::CompletedNativeCallback(reader,binding,NativeFixture::HUD,event,
            MenuFixture::Base+transportdisk::ReturnPc,42,false,true),"matching HUD origin cannot admit changed event schema");
    }
    std::printf("CFG concrete frontend HUD provider/gate tests: %s (%u failures). Owned fixtures only.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif

#endif
