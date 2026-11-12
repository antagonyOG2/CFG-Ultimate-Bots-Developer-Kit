#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "../RuntimeProbe/PickerPrecreationObservation.hpp"
#include "PickerPrecreationFixture.hpp"
struct OwnedGetterDispatch {
    std::uint32_t thread=42;unsigned calls=0;std::uintptr_t result=NativeFixture::Controller,widget=0,function=0;bool okay=true;
    std::uint32_t Thread()const noexcept{return thread;}
    bool Invoke(std::uintptr_t actualWidget,std::uintptr_t actualFunction,void* packet)noexcept{
        ++calls;widget=actualWidget;function=actualFunction;static_cast<cfgmenu::widgetowner::Packet*>(packet)->controller=result;return okay;
    }
};
int main(int argc,char** argv){using namespace cfgmenu;using namespace pickerprepare;unsigned checks=0,failures=0;
    const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    const char* roots=argc>1?argv[1]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-16530-15103-Roots.txt";
    const char* counselor=argc>2?argv[2]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Native-Picker-Chain-3243-25117-Counselor.txt";
    const char* jason=argc>3?argv[3]:"G:\\Codex\\CampForestGreenOfflineBots-Port\\Diagnostics\\CFG-Menu-Roots-And-Jason-11776-28527-Jason.txt";
    auto fixture=std::make_unique<PickerPreparedFixture>(roots,counselor,jason);check(fixture->initialized,"owned retained native source fixture");if(!fixture->initialized)return 1;
    OptionsReader reader{fixture->fixture.r};auto& r=fixture->fixture.r;unsigned maximumReads=0;
    for(const auto kind:{Kind::Counselor,Kind::Jason}){auto prepared=std::make_unique<Prepared>();unsigned reads=0,stage=0;
        const bool okay=Prepare(reader,fixture->owner,*fixture->source,kind,42,true,*prepared,&reads,&stage);
        if(!okay)std::printf("Preparation failure kind%u stage%u reads%u\n",static_cast<unsigned>(kind),stage,reads);
        check(okay,"class metadata prepared before any picker instance exists");
        check(prepared->type==(kind==Kind::Counselor?PickerPreparedFixture::Counselor:PickerPreparedFixture::Jason)&&reads<ReadLimit,"source class exact and preparation bounded");
        maximumReads=(std::max)(maximumReads,reads);if(!prepared->type)continue;
        auto expected=Original(*prepared);check(Current(reader,fixture->owner,*prepared,42,expected),"prepared original slot metadata current");
        const auto fn=Get(*prepared,Role::Construct).slot.field.object;const auto hook=std::uintptr_t{0x4200000};r.Set(fn+0xB0,hook);
        check(!Current(reader,fixture->owner,*prepared,42,expected),"published slot is never silently accepted as original");expected.values[static_cast<unsigned>(Role::Construct)]=hook;
        check(Current(reader,fixture->owner,*prepared,42,expected),"explicit exact coordinator publication expectation only");
        const auto widget=kind==Kind::Counselor?std::uintptr_t{0x4400000}:std::uintptr_t{0x4500000};const auto frame=widget+0x1000,locals=widget+0x2000;
        fixture->Register(widget,kind==Kind::Counselor?8100:8101);r.Set(widget+cfgprobe::ObjectClass,prepared->type);
        r.Set(widget,PickerPreparedFixture::WidgetVtable);
        active::Context owner;check(active::Observe(reader,MenuFixture::Base,owner)&&owner.widget==MenuFixture::Widget,"real active top is still OfflinePlay, not a fake picker context");
        r.Set(frame+startdisk::FrameNode,fn);r.Set(frame+startdisk::FrameObject,widget);r.Set(frame+startdisk::FrameCode,Get(*prepared,Role::Construct).slot.script.data);r.Set(frame+startdisk::FrameCurrentNative,fn);
        Receiver receiver;check(FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver)&&receiver.widget==widget,"borrowed first constructor before active push validates actual receiver");
        if(kind==Kind::Counselor){
            // Reproduce the production Guard's exact prefilter/active/Construct
            // ordering on owned bytes to locate the live 412-read refusal.
            for(unsigned i=0;i<4;++i){const auto& saved=fixture->source->classes[i];
                const auto at=NativeFixture::Registry+24*saved.index+16;r.Set(at,saved.serial?saved.serial+1:9200);
                Bounded<OptionsReader> counted(reader);Role role;active::Context current;Receiver attempted;pickerdiagnostic::Trace trace;
                const bool admitted=CallbackFrame(counted,*prepared,1,widget,frame,role)&&
                    active::Observe(counted,MenuFixture::Base,current)&&FirstConstruct(counted,current,*prepared,expected,widget,frame,42,attempted,&trace);
                std::printf("Constructor metadata serial-change class%u admitted=%u reads=%u metadata=%u\n",i,admitted?1u:0u,counted.reads,trace.metadataStage);
                check(!admitted&&trace.metadataStage==1,"strict cold snapshot reproduces first constructor source refusal");
                auto rebound=std::make_unique<Prepared>(*prepared);trace={};Receiver initialized;
                const bool accepted=FirstInitializedConstruct(reader,owner,*rebound,expected,widget,frame,42,initialized,&trace);
                check(accepted==(i!=0),"native zero-to-positive class serial can initialize; assigned root serial cannot change");
                if(i!=0){check(rebound->source.classes[i].serial==9200&&prepared->source.classes[i].serial==0&&trace.nativeSerialAssignments==1,
                    "native metadata assignment latched only in successful internal snapshot");
                    check(Current(reader,owner,*rebound,42,expected),"latched native metadata is strict and current");
                    r.Set(at,9201);Receiver rejected;
                    check(!Current(reader,owner,*rebound,42,expected)&&!FirstInitializedConstruct(reader,owner,*rebound,expected,widget,frame,42,rejected)&&!rejected.widget,
                        "positive-to-different serial never refreshed as initialization");}
                r.Set(at,saved.serial);
            }
            r.Set(NativeFixture::Records+execdisk::RecordWidget,widget);
            Bounded<OptionsReader> counted(reader);Role role;active::Context current;Receiver attempted;pickerdiagnostic::Trace trace;
            const bool admitted=CallbackFrame(counted,*prepared,1,widget,frame,role)&&active::Observe(counted,MenuFixture::Base,current)&&
                FirstConstruct(counted,current,*prepared,expected,widget,frame,42,attempted,&trace);
            std::printf("Constructor nullable-root-widget-change admitted=%u reads=%u metadata=%u\n",admitted?1u:0u,counted.reads,trace.metadataStage);
            r.Set(NativeFixture::Records+execdisk::RecordWidget,std::uintptr_t{0});
            // Native creation can also initialize class-owned graph/function
            // weak serials; each remains held by its exact reflected owner.
            std::array<cfgprobe::lifetime::SlotObservation,10> slots{};unsigned count=0;
            slots[count++]=prepared->source.mainGraph.slot;slots[count++]=prepared->source.customizeGraph.slot;slots[count++]=prepared->graph.slot;
            for(const auto& handler:prepared->handlers)if(handler.present)slots[count++]=handler.slot.identity;
            slots[count++]=prepared->owningPlayer.function.slot;
            for(unsigned n=0;n<count;++n)r.Set(NativeFixture::Registry+24*slots[n].index+16,9300+slots[n].index);
            auto initialized=std::make_unique<Prepared>(*prepared);pickerdiagnostic::Trace initializedTrace;Receiver initializedReceiver;
            check(FirstInitializedConstruct(reader,owner,*initialized,expected,widget,frame,42,initializedReceiver,&initializedTrace)&&
                initializedTrace.nativeSerialAssignments>=7&&Current(reader,owner,*initialized,42,expected),
                "native class-owned function and graph serial initialization admitted and latched together");
            OwnedGetterDispatch warmDispatch;Receiver warmOwned;
            check(FirstOwnedConstruct(reader,warmDispatch,owner,*initialized,expected,widget,frame,42,warmOwned)&&
                warmDispatch.calls==1&&OwnedReceiverCurrent(reader,owner,*initialized,expected,warmOwned,42),
                "initialized native metadata continues through actual owner-getter admission, not merely structural acceptance");
            for(unsigned n=0;n<count;++n)r.Set(NativeFixture::Registry+24*slots[n].index+16,slots[n].serial);
            const auto classAt=NativeFixture::Registry+24*prepared->source.classes[2].index+16;r.Set(classAt,9400);
            r.Set(fn+0xB0,hook+8);auto rejected=std::make_unique<Prepared>(*prepared);Receiver noReceiver;
            check(!FirstInitializedConstruct(reader,owner,*rejected,expected,widget,frame,42,noReceiver)&&
                !noReceiver.widget&&rejected->source.classes[2].serial==0,"changed callback executor blocks refresh and leaves original snapshot untouched");
            r.Set(fn+0xB0,hook);r.Set(classAt,0);
            for(unsigned fault=0;fault<3;++fault){r.Set(classAt,9400);
                if(fault==0)r.Set(NativeFixture::Records,PickerPreparedFixture::Customize);
                if(fault==1)r.Set(NativeFixture::Registry+24*(fixture->slotCount-1)+16,0);
                auto refusal=std::make_unique<Prepared>(*prepared);Receiver absent;
                check(!FirstInitializedConstruct(reader,owner,*refusal,expected,widget,frame,fault==2?43u:42u,absent)&&
                    !absent.widget&&refusal->source.classes[2].serial==0,
                    "lost root, unassigned receiver or foreign thread cannot commit warmed metadata");
                r.Set(NativeFixture::Records,PickerPreparedFixture::RootType);
                r.Set(NativeFixture::Registry+24*(fixture->slotCount-1)+16,8100);r.Set(classAt,0);
            }
        }
        {pickerdiagnostic::Trace trace;Receiver plain,diagnosed;Bounded<OptionsReader> a(reader),b(reader);
            check(FirstConstruct(a,owner,*prepared,expected,widget,frame,42,plain)&&
                FirstConstruct(b,owner,*prepared,expected,widget,frame,42,diagnosed,&trace)&&a.reads==b.reads&&plain.identity==diagnosed.identity&&
                trace.constructStage==0&&trace.metadataStage==0&&trace.observed==63&&trace.receiverSerial>0,
                "successful constructor diagnostics add zero reads and no authority");
            Role role;trace={};Bounded<OptionsReader> cheap(reader);
            check(CallbackFrame(cheap,*prepared,1,widget,frame,role,&trace)&&role==Role::Construct&&cheap.reads==5&&trace.observed==31&&trace.frameStage==0,
                "production callback prefilter records exact existing five frame reads");
            for(unsigned fault=0;fault<6;++fault){
                const auto slot=NativeFixture::Registry+24*(fixture->slotCount-1)+16;
                if(fault==0)r.Set(frame+startdisk::FrameCurrentNative,std::uintptr_t{0});
                if(fault==1)r.Set(frame+startdisk::FrameCode,Get(*prepared,Role::Construct).slot.script.data+1);
                if(fault==2)r.Set(slot,0);
                if(fault==3)r.Set(NativeFixture::Records,PickerPreparedFixture::Customize);
                if(fault==4)r.Set(fn+0xB0,hook+8);
                if(fault==5)r.Set(widget+cfgprobe::ObjectClass,PickerPreparedFixture::Customize);
                trace={};Bounded<OptionsReader> left(reader),right(reader);Receiver one,two;
                const bool without=FirstConstruct(left,owner,*prepared,expected,widget,frame,42,one);
                const bool with=FirstConstruct(right,owner,*prepared,expected,widget,frame,42,two,&trace);
                check(!without&&!with&&left.reads==right.reads&&!one.widget&&!two.widget,"failed diagnostic preserves exact guard/read/refusal behavior");
                if(fault==0)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Native)&&
                    (trace.observed&pickerdiagnostic::Native)&&!trace.native&&trace.node==fn,"zero CurrentNative remains refused and explicitly observed");
                if(fault==1)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Role)&&trace.code==Get(*prepared,Role::Construct).slot.script.data+1,"advanced code is diagnosed without relaxing cursor");
                if(fault==2)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Serial)&&
                    (trace.observed&pickerdiagnostic::Serial)&&!trace.receiverSerial,"unassigned serial distinct from unobserved serial");
                if(fault==3)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Metadata)&&trace.metadataStage==1,"retained source metadata failure is located");
                if(fault==4)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Metadata)&&trace.metadataStage==11,"exact Construct executor failure is located");
                if(fault==5)check(trace.constructStage==static_cast<unsigned>(pickerdiagnostic::Construct::Type)&&trace.type==PickerPreparedFixture::Customize,"receiver type mismatch remains refused");
                if(fault==0||fault==1||fault==5){trace={};Role rejected;
                    check(!CallbackFrame(reader,*prepared,1,widget,frame,rejected,&trace)&&rejected==Role::Count&&
                        trace.frameStage==static_cast<unsigned>(fault==0?pickerdiagnostic::Frame::Native:fault==1?pickerdiagnostic::Frame::Code:pickerdiagnostic::Frame::Type),
                        "production cheap frame refusal reports exact failing existing read");}
                r.Set(frame+startdisk::FrameCurrentNative,fn);r.Set(frame+startdisk::FrameCode,Get(*prepared,Role::Construct).slot.script.data);
                r.Set(slot,kind==Kind::Counselor?8100:8101);r.Set(NativeFixture::Records,PickerPreparedFixture::RootType);
                r.Set(fn+0xB0,hook);r.Set(widget+cfgprobe::ObjectClass,prepared->type);
            }
        }
        Receiver admitted=receiver;check(ReceiverCurrent(reader,owner,*prepared,expected,admitted,42),"admitted first-constructor receiver has fresh exact class/slot/source lease");
        OwnedGetterDispatch dispatch;Receiver owned;unsigned callbackReads=0;
        check(FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,owned,&callbackReads)&&dispatch.calls==1&&dispatch.widget==widget&&dispatch.function==PickerPreparedFixture::OwnerGetter&&callbackReads<ReadLimit,
            "one exact named native owner getter on actual pre-push constructor receiver");
        std::printf("%s first owned Construct reads=%u native owner calls=%u\n",kind==Kind::Counselor?"Counselor":"Jason",callbackReads,dispatch.calls);
        check(OwnedReceiverCurrent(reader,owner,*prepared,expected,owned,42)&&!OwnedReceiverCurrent(reader,owner,*prepared,expected,admitted,42),"structural receiver alone cannot impersonate an owning-player admission");
        if(kind==Kind::Counselor){ConstructorState state;
            check(CounselorState(reader,owner,*prepared,expected,owned,42,state)&&state.needsWidening&&state.state==0x2420000,"fresh actual base native state from graph-embedded PlayerState descriptor");
            check(!CounselorState(reader,owner,*prepared,expected,admitted,42,state),"structural receiver cannot acquire native state adaptation authority");
            r.Set(std::uintptr_t{0x2420000}+cfgprobe::ObjectClass,PickerPreparedFixture::LobbyState);
            check(CounselorState(reader,owner,*prepared,expected,owned,42,state)&&!state.needsWidening,"existing lobby subclass does not request cast widening");r.Set(std::uintptr_t{0x2420000}+cfgprobe::ObjectClass,PickerPreparedFixture::State);
            r.Set(std::uintptr_t{0x2410000}+cfgreflection::disk::PropertyOffset,0x389);check(!CounselorState(reader,owner,*prepared,expected,owned,42,state),"changed graph PlayerState property refuses");r.Set(std::uintptr_t{0x2410000}+cfgreflection::disk::PropertyOffset,0x388);
        }
        dispatch.result=MenuFixture::Widget;check(!FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,receiver)&&!receiver.widget,"foreign GetOwningPlayer result refuses");dispatch.result=NativeFixture::Controller;
        dispatch.result=0;check(!FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,receiver),"null GetOwningPlayer result refuses");dispatch.result=NativeFixture::Controller;
        dispatch.okay=false;check(!FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,receiver),"protected getter native fault refuses admission");dispatch.okay=true;
        const auto count=dispatch.calls;r.Set(PickerPreparedFixture::WidgetVtable+widgetowner::VirtualSlot,MenuFixture::Base+widgetowner::VirtualGetter+1);
        check(!FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,receiver)&&dispatch.calls==count,"foreign virtual owner getter refuses before dispatch");r.Set(PickerPreparedFixture::WidgetVtable+widgetowner::VirtualSlot,MenuFixture::Base+widgetowner::VirtualGetter);
        for(unsigned fault=0;fault<4;++fault){const auto function=PickerPreparedFixture::OwnerGetter;const auto parameter=function+0x100;
            if(fault==0)r.Set(function+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{16});
            if(fault==1)r.Set(parameter+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0010000000000580ULL});
            if(fault==2)r.Set(MenuFixture::Base+widgetowner::Executor,std::uint8_t{0xCC});
            if(fault==3)r.Set(MenuFixture::Base+widgetowner::FunctionCache,function+0x100);
            check(!FirstOwnedConstruct(reader,dispatch,owner,*prepared,expected,widget,frame,42,receiver)&&dispatch.calls==count,"changed named native getter ABI/code/cache refuses before dispatch");
            r.Set(function+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});r.Set(parameter+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000780ULL});
            r.Set(MenuFixture::Base+widgetowner::Executor,widgetowner::ExecutorCode);r.Set(MenuFixture::Base+widgetowner::FunctionCache,function);
        }
        check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,43,receiver)&&!receiver.widget,"foreign constructor thread refuses");
        r.Set(frame+startdisk::FrameCode,Get(*prepared,Role::Construct).slot.script.data+1);check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"advanced constructor cursor is not an entry callback");r.Set(frame+startdisk::FrameCode,Get(*prepared,Role::Construct).slot.script.data);
        r.Set(frame+startdisk::FrameObject,MenuFixture::Widget);check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"foreign borrowed frame object refuses");r.Set(frame+startdisk::FrameObject,widget);
        r.Set(frame+startdisk::FrameCurrentNative,Get(*prepared,Role::Accept).slot.field.object);check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"current native node mismatch refuses");r.Set(frame+startdisk::FrameCurrentNative,fn);
        r.Set(widget+cfgprobe::ObjectClass,PickerPreparedFixture::Customize);check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"unrelated created class refuses");r.Set(widget+cfgprobe::ObjectClass,prepared->type);
        const auto widgetIndex=fixture->slotCount-1;r.Set(NativeFixture::Registry+24*widgetIndex+16,0);check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"unassigned receiver serial is never manufactured");r.Set(NativeFixture::Registry+24*widgetIndex+16,kind==Kind::Counselor?8100:8101);
        r.Set(NativeFixture::Registry+24*widgetIndex+16,9100);check(!ReceiverCurrent(reader,owner,*prepared,expected,admitted,42),"reused widget pointer never inherits admitted receiver lease");r.Set(NativeFixture::Registry+24*widgetIndex+16,kind==Kind::Counselor?8100:8101);
        const auto graph=prepared->graph.function.object;r.Set(frame+startdisk::FrameNode,graph);r.Set(frame+startdisk::FrameCurrentNative,graph);r.Set(frame+startdisk::FrameCode,prepared->graph.script.data);r.Set(frame+startdisk::FrameLocals,locals);r.Set(locals,Get(*prepared,Role::Construct).entry);
        check(FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"actual native wrapper-to-graph constructor recursion");r.Set(locals,Get(*prepared,Role::Accept).entry);
        check(!FirstConstruct(reader,owner,*prepared,expected,widget,frame,42,receiver),"non-constructor graph entry cannot create first-constructor lease");
        if(kind==Kind::Counselor){const auto at=prepared->graph.script.data+counselorpicker::CastOperandOffset;const auto original=prepared->lobbyClass;
            r.Set(at,prepared->nativeState);check(!Current(reader,owner,*prepared,42,expected)&&Current(reader,owner,*prepared,42,expected,true),"only explicit exact owned counselor cast operand is tolerated temporarily");
            r.Set(at+8,std::uint8_t{0});check(!Current(reader,owner,*prepared,42,expected,true),"cast lease never permits adjacent bytecode changes");
            r.Set(prepared->graph.script.data,prepared->graph.bytes);r.Set(at,original);
        }else check(!Current(reader,owner,*prepared,42,expected,true),"Jason never inherits counselor widening authority");
        const auto script=Get(*prepared,Role::Accept).slot.script.data;unsigned char original=0;r.Read(script,original);r.Set(script,std::uint8_t{0x0A});check(!Current(reader,owner,*prepared,42,expected),"changed prepared accept script refuses");r.Set(script,original);
        r.Set(NativeFixture::Records,PickerPreparedFixture::Customize);check(!Current(reader,owner,*prepared,42,expected),"lost native root source revokes published preparation");r.Set(NativeFixture::Records,PickerPreparedFixture::RootType);
        r.Set(fn+0xB0,MenuFixture::Base+startdisk::ScriptExecutor);
        if(kind==Kind::Counselor){
            const auto native=Original(*prepared);auto hooks=native;
            for(unsigned i=0;i<5;++i){hooks.values[i]=hook+0x100*i;r.Set(prepared->handlers[i].slot.field.object+0xB0,hooks.values[i]);}
            auto cold=std::make_unique<Prepared>();
            check(!Prepare(reader,fixture->owner,*fixture->source,kind,42,true,*cold,&reads,&stage)&&stage==3,
                "physical55 failure403 reproduced: cold capture refuses surviving own graph executor");
            auto retained=std::make_unique<Prepared>(*prepared);retained->source=*fixture->source;
            check(Current(reader,fixture->owner,*retained,42,hooks),"complete retained metadata validates exact five owned executors on fresh source");
            r.Set(retained->handlers[2].slot.field.object+0xB0,hook+1);
            check(!Current(reader,fixture->owner,*retained,42,hooks),"foreign mixed-epoch callback still refuses");
            for(unsigned i=0;i<5;++i)r.Set(prepared->handlers[i].slot.field.object+0xB0,native.values[i]);
        }
        auto refused=std::make_unique<Prepared>();check(!Prepare(reader,fixture->owner,*fixture->source,kind,42,false,*refused)&&!refused->type,"unsupported image produces no precreation binding");
        std::printf("%s class preparation reads=%u stage=%u\n",kind==Kind::Counselor?"Counselor":"Jason",reads,stage);
    }
    std::printf("CFG source-guarded precreation/borrowed first Construct: %s (%u checks, %u failures; maximum reads%u/%u). Owned fixtures; no hooks/native calls/writes/gameplay approval.\n",failures?"FAIL":"PASS",checks,failures,maximumReads,ReadLimit);
    return failures?1:0;
}
