#define CFGMENU_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFGMENU_HUD_FIXTURE_ONLY
#include "NativePreparedCounselorConstruct.hpp"
#include "PickerPrecreationFixture.hpp"
#include <functional>
struct PreparedOwnerDispatch {
    unsigned calls=0;std::uint32_t Thread()const noexcept{return 42;}
    bool Invoke(std::uintptr_t,std::uintptr_t,void* packet)noexcept{++calls;static_cast<cfgmenu::widgetowner::Packet*>(packet)->controller=NativeFixture::Controller;return true;}
};
template<class Reader> struct PreparedFixtureHost {
    PickerPreparedFixture& fixture;Reader& reader;cfgmenu::pickerprepare::Prepared& prepared;
    const cfgmenu::pickerprepare::Executors& executors;const cfgmenu::pickerprepare::Receiver& receiver;
    const cfgmenu::pickerprepare::ConstructorState& state;std::uintptr_t frame=0;unsigned writes=0,restores=0;
    bool widened=false,refuse=false,uncertain=false,ownedProof=false,currentProof=false;
    unsigned currentChecks=0,ownedChecks=0;std::function<void()> afterRestore;
    cfgmenu::pickerprepare::ConstructorDiagnostic diagnostic{};
    bool Current(bool expected){++currentChecks;diagnostic={};cfgmenu::active::Context actual;currentProof=expected==widened&&cfgmenu::active::Observe(reader,fixture.owner.module,actual)&&cfgmenu::pickerprepare::ConstructorStateCurrent(reader,actual,prepared,executors,receiver,state,42,expected,&diagnostic);return currentProof;}
    bool OwnedConstruct(){++ownedChecks;cfgmenu::active::Context actual;ownedProof=!widened&&cfgmenu::active::Observe(reader,fixture.owner.module,actual)&&cfgmenu::counselorpicker::PreparedConstructFrame(reader,actual,prepared,executors,receiver,frame,42);return ownedProof;}
    bool NeedsWidening()const noexcept{return state.needsWidening;}
    cfgmenu::counselorpicker::CastResult Widen(){using namespace cfgmenu::counselorpicker;
        if(refuse||!currentProof||!ownedProof)return CastResult::Refused;currentProof=false;if(uncertain)return CastResult::Indeterminate;
        std::uintptr_t actual=0;if(!reader.Read(prepared.graph.script.data+CastOperandOffset,actual)||actual!=prepared.lobbyClass)return CastResult::Refused;
        fixture.fixture.r.Set(prepared.graph.script.data+CastOperandOffset,prepared.nativeState);widened=true;++writes;return CastResult::Changed;}
    cfgmenu::counselorpicker::CastResult Restore(){using namespace cfgmenu::counselorpicker;
        if(!currentProof)return CastResult::Refused;currentProof=false;std::uintptr_t actual=0;
        if(!reader.Read(prepared.graph.script.data+CastOperandOffset,actual)||actual!=prepared.nativeState)return CastResult::Refused;
        fixture.fixture.r.Set(prepared.graph.script.data+CastOperandOffset,prepared.lobbyClass);
        if(!reader.Read(prepared.graph.script.data+CastOperandOffset,actual)||actual!=prepared.lobbyClass)return CastResult::Indeterminate;
        widened=false;++restores;if(afterRestore)afterRestore();return CastResult::Changed;}
};
int main(int argc,char** argv){using namespace cfgmenu;using namespace pickerprepare;using namespace counselorpicker;
    unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    if(argc!=4)return 2;auto fixture=std::make_unique<PickerPreparedFixture>(argv[1],argv[2],argv[3]);check(fixture->initialized,"retained source fixture");if(!fixture->initialized)return 1;
    OptionsReader reader{fixture->fixture.r};auto& r=fixture->fixture.r;auto p=std::make_unique<Prepared>();
    check(Prepare(reader,fixture->owner,*fixture->source,Kind::Counselor,42,true,*p),"precreation metadata source");if(!p->type)return 1;auto executors=Original(*p);
    constexpr std::uintptr_t widget=0x4400000,frame=0x4401000;fixture->Register(widget,8200);r.Set(widget+cfgprobe::ObjectClass,p->type);r.Set(widget,PickerPreparedFixture::WidgetVtable);
    check(active::Observe(reader,MenuFixture::Base,fixture->owner)&&fixture->owner.widget==MenuFixture::Widget,"fresh actual old top and grown registry before first native push");
    const auto& wrapper=Get(*p,Role::Construct);r.Set(frame+startdisk::FrameNode,wrapper.slot.field.object);r.Set(frame+startdisk::FrameCurrentNative,wrapper.slot.field.object);
    r.Set(frame+startdisk::FrameObject,widget);r.Set(frame+startdisk::FrameCode,wrapper.slot.script.data);
    PreparedOwnerDispatch dispatch;Receiver receiver;check(FirstOwnedConstruct(reader,dispatch,fixture->owner,*p,executors,widget,frame,42,receiver)&&dispatch.calls==1,"one owner getter admission before active push");
    ConstructorState state;check(CounselorState(reader,fixture->owner,*p,executors,receiver,42,state)&&state.needsWidening,"captured actual native base state");
    check(ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),"positive state and typed PlayerState property lease");
    unsigned maximumReads=0;for(unsigned scenario=0;scenario<10;++scenario){auto actualState=state;ScopedConstruct scope;pickerprepare::Bounded<OptionsReader> budget(reader);
        if(scenario==1){r.Set(state.state+cfgprobe::ObjectClass,PickerPreparedFixture::LobbyState);actualState.needsWidening=false;}
        PreparedFixtureHost<pickerprepare::Bounded<OptionsReader>> host{*fixture,budget,*p,executors,receiver,actualState,frame};host.refuse=scenario==6;host.uncertain=scenario==7;
        unsigned calls=0;bool inside=false;const bool returned=scope.Run(host,[&]()noexcept{
            ++calls;std::uintptr_t operand=0;r.Read(p->graph.script.data+CastOperandOffset,operand);
            inside=operand==((scenario==1||scenario==6||scenario==7)?p->lobbyClass:p->nativeState);
            // Original wrapper-to-graph recursion: same outer writer lease,
            // original borrowed forwarding, no second getter or widening.
            if(scenario==2)scope.Run(host,[&]()noexcept{++calls;return true;});
            if(scenario==3)r.Set(p->graph.script.data+CastOperandOffset,PickerPreparedFixture::Customize);
            if(scenario==4)r.Set(NativeFixture::Records,PickerPreparedFixture::Customize);
            if(scenario==5)r.Set(NativeFixture::Controller+0x388,MenuFixture::Widget);
            if(scenario==9)r.Set(widget+1480,static_cast<unsigned char>(1)); // Native IsFromLobby initialization is ordinary instance state.
            r.Set(frame+startdisk::FrameCode,wrapper.slot.script.data+1);return scenario!=8;
        });
        check(inside,"only owned exact native base-state cast is visible while original Construct forwards");
        check(returned==(scenario!=8),"original native forward return/fault is preserved");
        if(scenario==1||scenario==6||scenario==7)check(!host.writes&&!host.restores,"lobby/refused/uncertain exchange never writes fixture graph");
        else if(scenario==3||scenario==4||scenario==5)check(host.writes==1&&!host.restores&&scope.State()==ConstructState::Indeterminate,"foreign operand/lost root/changed state is not rollback authority");
        else check(host.writes==1&&host.restores==1&&scope.State()==(scenario==8?ConstructState::Refused:ConstructState::Returned),"owned cast restores even after native forward failure and advanced borrowed frame");
        check(calls==(scenario==2?2U:1U)&&dispatch.calls==1,"recursive forwarding does not add owner calls or writes");
        if(scenario==9)check(scope.State()==ConstructState::Returned&&host.currentChecks==3,"native IsFromLobby change does not invalidate immutable ownership/schema proof");
        check(budget.reads<pickerprepare::ReadLimit,"complete source/state/borrowed callback transaction stays within16K reads");maximumReads=(std::max)(maximumReads,budget.reads);
        r.Set(p->graph.script.data+CastOperandOffset,p->lobbyClass);r.Set(NativeFixture::Records,PickerPreparedFixture::RootType);
        r.Set(NativeFixture::Controller+0x388,state.state);r.Set(state.state+cfgprobe::ObjectClass,PickerPreparedFixture::State);r.Set(frame+startdisk::FrameCode,wrapper.slot.script.data);
    }
    for(unsigned scenario=0;scenario<3;++scenario){ScopedConstruct scope;pickerprepare::Bounded<OptionsReader> budget(reader);
        PreparedFixtureHost<pickerprepare::Bounded<OptionsReader>> host{*fixture,budget,*p,executors,receiver,state,frame};
        host.afterRestore=[&](){
            if(scenario==1)r.Set(NativeFixture::Records,PickerPreparedFixture::Customize);
            else r.Set(widget,PickerPreparedFixture::Customize);
        };
        unsigned calls=0;const bool returned=scope.Run(host,[&]()noexcept{++calls;r.Set(frame+startdisk::FrameCode,wrapper.slot.script.data+1);return scenario!=2;});
        check(returned==(scenario!=2)&&calls==1,"final validation failure preserves original native result");
        check(host.writes==1&&host.restores==1&&host.currentChecks==3&&host.ownedChecks==1,
            "physical three-check signature reproduces only after confirmed restoration");
        check(scope.State()==(scenario==2?ConstructState::Refused:ConstructState::RestoredAwaitValidation),
            "restored successful native call awaits independent validation; original failure stays refused");
        check(host.diagnostic.stage==2&&(scenario==1?(host.diagnostic.metadataStage==1&&host.diagnostic.sourceStage==3):
            (!host.diagnostic.metadataStage&&!host.diagnostic.sourceStage)),"final proof diagnostic distinguishes retained source from receiver getter failure");
        std::uintptr_t operand=0;r.Read(p->graph.script.data+CastOperandOffset,operand);
        check(operand==p->lobbyClass&&!host.widened,"awaiting validation never leaves widened class operand");
        check(!ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),
            "restored-awaiting state grants no authority over currently invalid owner/source");
        const auto before=budget.reads;
        check(scope.Run(host,[&]()noexcept{++calls;return true;})&&budget.reads==before&&host.writes==1&&host.restores==1&&
            host.currentChecks==3&&host.ownedChecks==1&&dispatch.calls==1,"later Construct only forwards; never repeats widening/restore/owner getter");
        maximumReads=(std::max)(maximumReads,budget.reads);check(budget.reads<pickerprepare::ReadLimit,"restored final validation stays within transaction budget");
        r.Set(widget,PickerPreparedFixture::WidgetVtable);r.Set(NativeFixture::Records,PickerPreparedFixture::RootType);
        r.Set(frame+startdisk::FrameCode,wrapper.slot.script.data);
        check(ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),
            "subsequent real source/receiver check can independently recover authority");
    }
    r.Set(state.playerState.field.object+cfgreflection::disk::PropertyFlags,state.playerState.flags^1);
    check(!ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),"changed typed controller state property refuses");r.Set(state.playerState.field.object+cfgreflection::disk::PropertyFlags,state.playerState.flags);
    const auto stateSlot=NativeFixture::Registry+24*state.identity.index+16;r.Set(stateSlot,state.identity.serial+1);
    check(!ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),"reused state address refuses captured identity");r.Set(stateSlot,state.identity.serial);
    r.Set(MenuFixture::Base+counselorpickerdisk::DynamicCastSlot,MenuFixture::Base+counselorpickerdisk::DynamicCastExecutor+1);
    check(!ConstructorStateCurrent(reader,fixture->owner,*p,executors,receiver,state,42),"changed native cast opcode executor refuses");r.Set(MenuFixture::Base+counselorpickerdisk::DynamicCastSlot,MenuFixture::Base+counselorpickerdisk::DynamicCastExecutor);
    NativePreparedCastHost<OptionsReader> native(reader,*p,executors,receiver,state,frame);
    check(native.Widen()==CastResult::Refused&&native.Restore()==CastResult::Refused,"native adapter refuses synthetic fixture module/thread; no VirtualProtect/native write occurs");
    std::printf("CFG prepared pre-push counselor Construct lease: %s (%u checks, %u failures; maximum transaction reads%u/16384). Owned fixture exchanges only; no game access/native publication/calls.\n",failures?"FAIL":"PASS",checks,failures,maximumReads);return failures?1:0;
}
