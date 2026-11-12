#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/BorrowedConstructObservation.hpp"
#include "PickerPrecreationFixture.hpp"
int main(){using namespace cfgmenu;unsigned failures=0,checks=0;
    const auto check=[&](bool okay,const char* name){++checks;if(!okay){++failures;std::printf("FAIL %s\n",name);}};
    auto fixture=std::make_unique<PickerPreparedFixture>(
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-16530-15103-Roots.txt",
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Native-Picker-Chain-3243-25117-Counselor.txt",
        "G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-11776-28527-Jason.txt");
    check(fixture->initialized,"existing owned source fixture");if(!fixture->initialized)return 1;
    OptionsReader reader{fixture->fixture.r};auto& r=fixture->fixture.r;auto p=std::make_unique<pickerprepare::Prepared>();
    check(pickerprepare::Prepare(reader,fixture->owner,*fixture->source,pickerprepare::Kind::Counselor,42,true,*p),"existing prepared counselor metadata");
    constexpr std::uintptr_t widget=0x4400000,frame=0x4401000,hook=0x4600000;
    fixture->Register(widget,8100);r.Set(widget+cfgprobe::ObjectClass,p->type);
    auto expected=pickerprepare::Original(*p);const auto fn=pickerprepare::Get(*p,pickerprepare::Role::Construct).slot.field.object;
    r.Set(fn+startdisk::Executor,hook);expected.values[1]=hook;
    r.Set(frame+startdisk::FrameNode,fn);r.Set(frame+startdisk::FrameObject,widget);
    r.Set(frame+startdisk::FrameCode,pickerprepare::Get(*p,pickerprepare::Role::Construct).slot.script.data);
    r.Set(frame+startdisk::FrameCurrentNative,fn);r.Set(MenuFixture::Base+constructdisk::Invoke,constructdisk::InvokeCode);
    check(constructboundary::Dependencies(fixture->owner,*p,widget,[&](std::uintptr_t value){
        if(!value)return true;cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot;
        if(!cfgprobe::lifetime::ReadArrayHeader(reader,MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        if(!cfgprobe::lifetime::ReadSlot(reader,registry,value,slot))fixture->Register(value);return true;
    }),"register only explicit owned dependency fixtures");
    active::Context actual;check(active::Observe(reader,MenuFixture::Base,actual),"actual old menu remains active during borrowed Construct");
    constructboundary::Evidence evidence;unsigned stage=0;
    check(constructboundary::Observe(reader,actual,*p,expected,widget,frame,MenuFixture::Base+constructdisk::Return,42,evidence,&stage),"exact native origin/frame/class admission");
    if(!evidence.count){std::printf("Refusal stage=%u\n",stage);return 1;}
    check(evidence.count<constructboundary::MaximumRoots&&evidence.receiver.widget==widget,"bounded unique typed dependency list");
    check(constructboundary::Current(reader,evidence,42),"same live invocation proof");
    for(unsigned fault=0;fault<5;++fault){auto changed=evidence;
        if(fault==0)++changed.returnPc;if(fault==1)changed.node+=8;if(fault==2)changed.receiver.identity.serial++;
        if(fault==3)changed.source.identities[0].serial++;
        check(!constructboundary::Current(reader,changed,fault==4?43u:42u),"wrong origin/frame/recycled owner/foreign thread refuse");}
    r.Set(frame+startdisk::FrameCode,std::uintptr_t{0x123456});
    check(constructboundary::Current(reader,evidence,42),"advanced original cursor does not invalidate cleanup");
    r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,widget);
    r.Set(NativeFixture::Records+execdisk::RecordBytes+parentdisk::RecordClass,p->type);
    check(constructboundary::Current(reader,evidence,42),"native active menu replacement preserves fixed infrastructure and invocation");
    r.Set(frame+startdisk::FrameCurrentNative,std::uintptr_t{0});
    check(!constructboundary::Current(reader,evidence,42),"returned or foreign invocation cannot release borrowed lease");
    r.Set(frame+startdisk::FrameCurrentNative,fn);r.Set(MenuFixture::Base+constructdisk::Invoke,std::uint8_t{0});
    check(!constructboundary::Current(reader,evidence,42),"changed full native body refuses");
    std::printf("Borrowed Construct native origin/current-frame/dependency proof: %u checks, %u failures; owned fixtures only\n",checks,failures);
    return failures?1:0;
}
