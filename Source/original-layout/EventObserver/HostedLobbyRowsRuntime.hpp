#include "HostedLobbyLayout.hpp"
#include "HostedLobbyRetainedRuntime.hpp"
#include "HostedLobbyInputEvidence.hpp"
struct HostedLobbyRowsState {
    Reader reader;std::unique_ptr<cfgmenu::rows::HostedLobbyNativeDispatch> engine;
    std::unique_ptr<cfgmenu::rows::NativeAccess<Reader,cfgmenu::rows::HostedLobbyNativeDispatch>> access;
    cfgmenu::reflection::RetainedMenuText texts;cfghostedlobby::RowLayout layout{};HostedLobbyRowReceiptStore receipts{};
    std::uintptr_t diagnosticWorld=0;HostedLobbyRowFunctionDiagnostic rowFunctionDiagnostic{};bool rowFunctionTracePending=false,rowFunctionTraceEmitted=false;
    bool chatDone=false;std::uint64_t chatAt=0;unsigned warm=0;bool prepared=false,rowNativeEntered=false;cfghostedlobby::RowRetry retry;std::uint64_t next=0;
    static bool Current(Reader& r,void* opaque){auto& s=*static_cast<HostedLobbyRowsState*>(opaque);
        return cfghostedlobby::LayoutCurrent(r,hostedLobbyDiscovery,s.layout);}
} hostedLobbyRows;
void ServiceHostedLobbyRows(std::uintptr_t actor,std::uintptr_t function,std::uintptr_t pc){
    const auto& c=hostedLobbyDiscovery.context;const auto now=GetTickCount64();
    if(!actor||!function||pc!=plan.module+cfgtick::disk::ReturnPc||
       (actor!=c.controller&&actor!=c.mode)||now<hostedLobbyRows.next||!hostedLobbyReport.histories)return;
    auto& snapshot=hostedLobbyReport.history[hostedLobbyReport.current];const auto& row=snapshot.team;
    if(!snapshot.listenHost||row.node>=snapshot.cursor||row.parent>=snapshot.cursor)return;
    // Tree-cap truncation in unrelated branches is harmless only when THIS
    // original native anchor's entire current strong path is reproved below.
    const auto& item=snapshot.nodes[row.node];const auto& panel=snapshot.nodes[row.parent];
    cfghostedlobby::RowLayout layout{snapshot.world,snapshot.menu,item.type,panel.object,item.object,row.childIndex+1,static_cast<std::uint64_t>(snapshot.serial)+1};
    HostedLobbyPublishScope publication;auto& state=hostedLobbyRows;
    const bool layoutChanged=state.layout.world!=layout.world||state.layout.menu!=layout.menu||state.layout.generation!=layout.generation;
    state.next=now+25;
    if(cfgmenu::rows::NativeRetentionScope::Occupied()||cfgmenu::rows::HostedLobbyRetentionScope::Occupied())return;
    // The previous callback's provider and retention scope have now destructed.
    // Flush its first bounded RowFunction refusal only at this safe later entry.
    if(state.rowFunctionTracePending&&!state.rowFunctionTraceEmitted){
        const bool emitted=FlushHostedLobbyRowFunctionDiagnostic(state.rowFunctionDiagnostic,
            [](const char* tag,std::uint64_t clause,std::uint64_t readsAndCap){cfgclosedtrace::Mark(tag,clause,readsAndCap);});
        if(emitted){state.rowFunctionTracePending=false;state.rowFunctionTraceEmitted=true;}
    }
    if(state.diagnosticWorld!=layout.world){state.diagnosticWorld=layout.world;state.rowFunctionDiagnostic={};state.rowFunctionTracePending=false;state.rowFunctionTraceEmitted=false;}
    // Failed rows have no work after native entry. A clean pre-entry retry
    // opens a provider only when due and consumes its attempt after admission.
    const bool failed=hostedLobbyPlacement.State()==static_cast<unsigned>(cfgmenu::rows::State::Failed);
    if(!layoutChanged&&failed&&!state.retry.Due(true,state.rowNativeEntered,
            state.access?state.access->OwnedCount():0,hostedLobbyRootBank.Poisoned(),state.engine&&state.engine->Faulted(),now))return;
    ++hostedLobbyReport.rowCallbacks;hostedLobbyReport.rowStage=1;
    // A prior provider has already closed between callbacks. Discard its
    // identity/topology hints before revalidating against a different layout.
    if(layoutChanged)state.receipts={};
    auto provider=std::make_unique<HostedLobbyRootProvider>(hostedLobbyDiscovery,layout,actor,function,pc,&hostedLobbyRows.receipts);
    provider->SetRowFunctionFailureSink(&state,[](void* opaque,unsigned clause,unsigned reads,unsigned cap){
        auto& target=*static_cast<HostedLobbyRowsState*>(opaque);
        if(!target.rowFunctionTracePending&&!target.rowFunctionTraceEmitted){
            target.rowFunctionDiagnostic={clause,reads,cap};target.rowFunctionTracePending=true;
        }
    });
    cfgmenu::rows::HostedLobbyRetentionScope scope(c.module,provider->Hooks(),provider->Admitted());
    if(!scope.Entered()){snapshot.placementRefusal=unsigned(cfghostedlobby::Failure::NoNativeLease);return;}
    ++hostedLobbyReport.rowAdmitted;hostedLobbyReport.rowStage=2;
    struct InputEvidence {
        HostedLobbyRootProvider& provider;HostedLobbyRowsState& state;
        ~InputEvidence(){cfghostedlobbyinputs::RecordInputs(hostedLobbyReport,provider,state.access.get(),
            cfgmenu::rows::HostedLobbyRetentionScope::InputFailure());}
    } evidence{*provider,state};
    if(layoutChanged){
        state.access.reset();state.engine.reset();state.layout=layout;state.warm=0;state.prepared=false;state.rowNativeEntered=false;state.retry={};
        hostedLobbyPlacement.Detach();state.chatDone=false;state.chatAt=0;
        state.engine=std::make_unique<cfgmenu::rows::HostedLobbyNativeDispatch>(c.module,c.thread,true);
        state.access=std::make_unique<cfgmenu::rows::NativeAccess<Reader,cfgmenu::rows::HostedLobbyNativeDispatch>>(state.reader,*state.engine,state.texts);
    }
    if(state.retry.Poll(hostedLobbyPlacement.State()==static_cast<unsigned>(cfgmenu::rows::State::Failed),state.rowNativeEntered,
            state.access?state.access->OwnedCount():0,hostedLobbyRootBank.Poisoned(),state.engine&&state.engine->Faulted(),now)){
        state.access.reset();state.engine=std::make_unique<cfgmenu::rows::HostedLobbyNativeDispatch>(c.module,c.thread,true);
        state.access=std::make_unique<cfgmenu::rows::NativeAccess<Reader,cfgmenu::rows::HostedLobbyNativeDispatch>>(state.reader,*state.engine,state.texts);
        state.prepared=false;state.warm=0;hostedLobbyPlacement.Detach();
    }
    state.reader.reads=0;state.reader.cap=131072;
    if(!state.engine||!state.access||!state.engine->BeginPhase())return;
    if(!state.prepared){cfgmenu::active::Context context;context.module=c.module;context.thread=c.thread;
        context.world=c.world;context.mode=c.mode;context.controller=c.controller;context.hud=c.hud;context.widget=c.menu;context.type=c.type;
        state.prepared=state.access->PrepareHosted(context,layout.generation,layout.anchor,layout.panel,&state,&HostedLobbyRowsState::Current);
        hostedLobbyReport.rowStage=state.prepared?3u:20u;return;
    }
    // Warm at most one exact, pinned native factory per opportunity. Never load
    // assets or borrow an offline startup capability to construct lobby rows.
    using namespace cfgmenu::execdisk;bool ready=false;
    switch(state.warm){
    case 0:ready=state.access->WarmBinding(Create,CreateParameters,CreateCode,CreateFactoryCode);break;
    case 1:ready=state.access->WarmBinding(AddChildToVerticalBox,AddChildToVerticalBoxParameters,AddChildToVerticalBoxCode,AddChildToVerticalBoxFactoryCode);break;
    case 2:ready=state.access->WarmBinding(Conv_StringToText,Conv_StringToTextParameters,Conv_StringToTextCode,Conv_StringToTextFactoryCode);break;
    case 3:ready=state.access->WarmBinding(SetText,SetTextParameters,SetTextCode,SetTextFactoryCode);break;
    case 4:ready=state.access->WarmBinding(RemoveChild,RemoveChildParameters,RemoveChildCode,RemoveChildFactoryCode);break;
    default:ready=true;break;
    }
    if(state.warm<5){if(ready)++state.warm;hostedLobbyReport.rowWarm=state.warm;hostedLobbyReport.rowStage=30+state.warm;return;}
    struct Host {
        HostedLobbyRowsState& state;
        bool ProvedHostLobbyLease(const cfghostedlobby::RowLayout& l){return l.menu==state.layout.menu&&l.generation==state.layout.generation&&
            cfgmenu::rows::HostedLobbyRetentionScope::Current(plan.module)&&HostedLobbyRowsState::Current(state.reader,&state);}
        bool AppendAfter(const cfghostedlobby::RowLayout& l,unsigned index,std::uintptr_t& row,bool& attached){return ProvedHostLobbyLease(l)&&state.access->AppendHostedAfter(index,row,attached);}
        bool CopyOptions(std::uintptr_t row,const cfgmenu::rows::Spec& spec){return state.access->CopyOptions(row,spec);}
        bool Text(std::u16string_view text,cfgmenu::reflection::RetainedMenuText::Request& out){return state.access->Text(text,out);}
        bool SetText(std::uintptr_t row,std::u16string_view field,const cfgmenu::reflection::RetainedMenuText::Request& text){return state.access->SetText(row,field,text);}
        bool Index(std::uintptr_t row,unsigned& out){return state.access->Index(row,out);}
        bool RemoveOwned(std::uintptr_t row){return state.access->RemoveOwned(row);}
    }host{state};
    (void)hostedLobbyPlacement.Service(host,layout,now,cfghostedoptions::Current());
    state.rowNativeEntered|=state.engine->Calls()!=0;
    if(!hostedLobbyReport.rowFirstFailure&&(hostedLobbyPlacement.State()==static_cast<unsigned>(cfgmenu::rows::State::RollingBack)||hostedLobbyPlacement.State()==static_cast<unsigned>(cfgmenu::rows::State::Failed))){
        hostedLobbyReport.rowFirstFailure=40u+static_cast<unsigned>(state.engine->LastFailure());hostedLobbyReport.rowFirstAdapter=state.access->operationStage;
        hostedLobbyReport.rowFirstReads=state.reader.reads;
    }
#if CFG_H269_CHAT_MOVE
    // One attempt per layout, after the rows are in. 120 px is three option rows; the slot reads as design-resolution pixels.
    if(snapshot.rowsPlaced&&!state.chatDone&&hostedLobbyDiscovery.chatSlot){
        if(!state.chatAt)state.chatAt=now+500;
        else if(now>=state.chatAt){state.chatDone=true;
            const auto slot=hostedLobbyDiscovery.chatSlot;std::array<float,2> at{};std::uintptr_t fn=0;unsigned why=1;
            if(state.reader.Read(slot+0x38,at)&&std::isfinite(at[0])&&std::isfinite(at[1])&&at[1]>=0.f&&at[1]<=800.f){why=2;
                if(state.access->HostedCanvasFunction(slot,fn)){why=3;provider->AllowChatSlot(slot,hostedLobbyDiscovery.chatWidget,hostedLobbyDiscovery.chatPanel,fn);
                    if(state.access->MoveHostedCanvasSlot(slot,hostedLobbyDiscovery.chatWidget,fn,at[0],at[1]+120.f))why=0;}}
            cfgclosedtrace::Mark(why?"h269-chat-move-refused":"h269-chat-moved",slot,(static_cast<std::uint64_t>(why)<<32)|static_cast<std::uint32_t>(at[1]));
        }
    }
#endif
    hostedLobbyReport.rowRetries=state.retry.attempts;
    snapshot.rowsPlaced=hostedLobbyPlacement.Placed();snapshot.placementRefusal=snapshot.rowsPlaced?0u:unsigned(cfghostedlobby::Failure::NoCompatibleRow);
    hostedLobbyReport.rowStage=snapshot.rowsPlaced?100u:40u+static_cast<unsigned>(state.engine->LastFailure());
    hostedLobbyReport.rowRootPoison=hostedLobbyRootBank.Poisoned()?1u:0u;
    hostedLobbyReport.rowAdapterStage=state.access->operationStage;hostedLobbyReport.rowState=hostedLobbyPlacement.State();
    hostedLobbyReport.rowOwned=state.access->OwnedCount();hostedLobbyReport.rowCalls=state.engine->Calls();
    hostedLobbyReport.rowStaticOwnerName=state.access->staticOwnerName;hostedLobbyReport.rowStaticClassName=state.access->staticClassName;
    hostedLobbyReport.rowStaticOwner=state.access->staticOwner;hostedLobbyReport.rowStaticDefault=state.access->staticDefault;
}
