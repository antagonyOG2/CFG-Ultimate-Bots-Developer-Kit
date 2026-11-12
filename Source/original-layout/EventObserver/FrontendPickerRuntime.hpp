// Included in the observer's private namespace after the existing frontend
// transport/resources. No independent timer, dispatcher, loader or game scan.
using PickerCoordinator=cfgmenu::pickercoordinator::Coordinator;
std::unique_ptr<PickerCoordinator> frontendPickerCoordinator;
std::unique_ptr<cfgmenu::entryroute::Binding> frontendEntryRoute;
cfgmenu::counselortransition::Consumer frontendCounselorConsumer;
cfgmenu::pickerprepare::ConstructorState frontendConstructorState;
bool frontendPickerPrepared=false;
// Set once before any executor publication; never rebased with active widgets.
std::uintptr_t frontendPickerOriginal=0;
cfgmenu::entryreturn::Ticket frontendEntryReturnTicket;
cfgmenu::acceptedcounselor::Choice frontendAcceptedCounselor;
bool frontendAcceptedCounselorCommitted=false;
void __fastcall FrontendPickerGraph(void*,void*,void*);
void __fastcall FrontendPickerConstruct(void*,void*,void*);
void __fastcall FrontendPickerAccept(void*,void*,void*);
void __fastcall FrontendPickerTransition(void*,void*,void*);
void __fastcall FrontendPickerBack(void*,void*,void*);
void __fastcall FrontendEntryClick(void*,void*,void*);
void __fastcall FrontendEntryGraph(void*,void*,void*);
void __fastcall FrontendEntryTransition(void*,void*,void*);
bool PickerNativeForward(std::uintptr_t original,const cfgmenu::pickercoordinator::Call& call)noexcept{
    __try{
        reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(original)(reinterpret_cast<void*>(call.widget),
            reinterpret_cast<void*>(call.frame),reinterpret_cast<void*>(call.result));return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
bool PickerCheapFrame(Reader& r,const cfgmenu::pickerprepare::Prepared& p,const cfgmenu::pickercoordinator::Call& call,
    cfgmenu::pickercoordinator::Kind& kind,cfgmenu::pickerdiagnostic::Trace* trace=nullptr){
    using namespace cfgmenu;using K=pickercoordinator::Kind;kind=K::Other;
    pickerprepare::Role role;if(!pickerprepare::CallbackFrame(r,p,call.cell,call.widget,call.frame,role,trace))return false;
    if(role==pickerprepare::Role::Construct)kind=K::Construct;
    else if(role==pickerprepare::Role::Accept)kind=K::Accept;
    else if(role==pickerprepare::Role::Transition)kind=K::Transition;
    else if(role==pickerprepare::Role::Back)kind=K::Back;
    return true;
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
FrontendMenuRootProvider* frontendConstructRoots=nullptr; // synchronous owner-thread scope only
#endif
struct FrontendPickerHost {
    std::uintptr_t callbackPc=0;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    std::unique_ptr<FrontendMenuRootProvider> constructRoots;
    std::unique_ptr<cfgmenu::rows::NativeRetentionScope> constructScope;
    ~FrontendPickerHost(){
        if(constructRoots){constructScope.reset();frontendConstructRoots=nullptr;constructRoots.reset();}
    }
    bool EnsureConstructScope(const cfgmenu::pickercoordinator::Call& call,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::active::Context& actual){
        if(frontendConstructRoots)return true; // nested owned wrapper->graph, no independent lease/getter
        if(!frontendCounselorPicker||constructRoots)return false;
        constructRoots=std::make_unique<FrontendMenuRootProvider>(actual,*frontendCounselorPicker,expected.counselor,
            call.widget,call.frame,callbackPc);
        if(!constructRoots->Admitted())return false;
        constructScope=std::make_unique<cfgmenu::rows::NativeRetentionScope>(actual.module,constructRoots->Hooks(),true);
        if(!constructScope->Entered())return false;frontendConstructRoots=constructRoots.get();return true;
    }
#endif
    bool EntryReady()const noexcept{return frontendEntryLabels&&frontendEntryLabels->Ready()&&frontendReport.state==8;}
    cfgmenu::callbackslots::Plan permittedPlan{};bool publicationAuthorized=false;
    std::uintptr_t permittedParameter=0,parameterPrevious=0,parameterTarget=0;
    bool Process()const noexcept{return GetCurrentThreadId()==observerOwnerThread&&frontendSource.module==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));}
    bool Resident(const cfgmenu::callbackslots::Plan& p)noexcept{
        if(!Process())return false;HMODULE module=nullptr;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&FrontendPickerGraph),&module))return false;
        for(unsigned i=0;i<p.count;++i){MEMORY_BASIC_INFORMATION memory{};
            if(VirtualQuery(reinterpret_cast<const void*>(p.cells[i].replacement),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
               memory.Type!=MEM_IMAGE||reinterpret_cast<HMODULE>(memory.AllocationBase)!=module||(memory.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
               !(memory.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)))return false;}
        return true;
    }
    bool Plan(const cfgmenu::callbackslots::Plan& p)const noexcept{
        if(!frontendCounselorPicker||!frontendEntryRoute||p.count!=cfgmenu::pickercoordinator::Cells)return false;
        for(unsigned i=0;i<p.count;++i){const auto expected=i<5?frontendCounselorPicker->handlers[i].slot.field.object:
            frontendEntryRoute->functions[i-5].slot.field.object;
            if(p.cells[i].address!=expected+cfgmenu::startdisk::Executor||p.cells[i].original!=frontendSource.module+cfgmenu::startdisk::ScriptExecutor)return false;}
        return true;
    }
    bool Current(const cfgmenu::callbackslots::Plan& p,unsigned installed)noexcept{
        publicationAuthorized=false;
        try {if(!Process()||!Plan(p)||installed>p.count)return false;Reader r;r.cap=32768;cfgmenu::active::Context actual;
            cfgmenu::pickercoordinator::Configuration c;c.plan=p;c.original.counselor=cfgmenu::pickerprepare::Original(*frontendCounselorPicker);
            c.original.entry=cfgmenu::entryroute::Original(*frontendEntryRoute);const auto expected=cfgmenu::pickercoordinator::Executors(c,installed);
            const bool okay=cfgmenu::active::Observe(r,frontendSource.module,actual)&&actual==frontendSource&&
                cfgmenu::pickerprepare::Current(r,actual,*frontendCounselorPicker,observerOwnerThread,expected.counselor)&&
                cfgmenu::entryroute::Current(r,*frontendEntryRoute,observerOwnerThread,expected.entry);
            frontendReport.routeReads=r.reads;frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);
            if(okay){permittedPlan=p;publicationAuthorized=true;}return okay;
        }catch(...){return false;}
    }
    cfgmenu::sandboxstart::WriteResult Exchange(std::uintptr_t at,std::uintptr_t oldValue,std::uintptr_t newValue)noexcept{
        if(!Process()||!frontendPickerCoordinator||!publicationAuthorized)return cfgmenu::sandboxstart::WriteResult::Refused;
        bool permitted=false;for(unsigned i=0;i<permittedPlan.count;++i){const auto& cell=permittedPlan.cells[i];
            if(at==cell.address&&((oldValue==cell.original&&newValue==cell.replacement)||(oldValue==cell.replacement&&newValue==cell.original)))permitted=true;}
        if(!permitted)return cfgmenu::sandboxstart::WriteResult::Refused;
        return cfgmenu::ExchangeOwnedPointer(at,oldValue,newValue);
    }
    bool Forward(std::uintptr_t original,const cfgmenu::pickercoordinator::Call& call)noexcept{
        if(original!=frontendSource.module+cfgmenu::startdisk::ScriptExecutor)return false;
        const bool okay=PickerNativeForward(original,call);if(!okay){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(frontendConstructRoots)frontendConstructRoots->Uncertain();
#endif
            frontendReport.routeFailure=8;FrontendRefused(1050);}return okay;
    }
    bool ObserveRoute(const cfgmenu::active::Context& actual,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::pickerprepare::Receiver& receiver,bool arrival)noexcept{
        Reader r;r.cap=16384;cfgmenu::pickerarrival::Diagnostic diagnostic;
        const auto finish=[&](bool okay){
            frontendReport.pickerArrivalStage=diagnostic.stage;frontendReport.pickerArrivalEntryStage=diagnostic.entryStage;
            frontendReport.pickerArrivalResetStage=diagnostic.reset.stage;frontendReport.pickerArrivalReads=r.reads;
            frontendReport.pickerArrivalMetadataStage=diagnostic.metadataStage;frontendReport.pickerArrivalSourceStage=diagnostic.sourceStage;
            frontendReport.pickerArrivalRefreshStage=diagnostic.refreshStage;
            frontendReport.pickerArrivalError=r.error;frontendReport.pickerArrivalBudgetHit=r.reads>=r.cap?1u:0u;
            frontendReport.routeReads=r.reads;frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);
            return okay;};
        try {diagnostic.stage=1;if(!Process()||!frontendPickerCoordinator||!frontendPickerSources||!frontendCounselorPicker||!frontendEntryResources||!frontendEntryRoute)return finish(false);
            if(arrival&&actual.type!=frontendCounselorPicker->type){
                const auto state=frontendPickerCoordinator->RouteState();
                if(!(((state==cfgmenu::counselorroute::State::AwaitMap||state==cfgmenu::counselorroute::State::MapReady||
                       state==cfgmenu::counselorroute::State::SettingsReady)&&actual.type==frontendEntryResources->map)||
                     ((state==cfgmenu::counselorroute::State::MapReady||state==cfgmenu::counselorroute::State::SettingsReady)&&
                       actual.type==frontendEntryResources->settings)))return finish(false);
                const bool okay=cfgmenu::pickerarrival::Initialize(r,actual,*frontendCounselorPicker,expected.counselor,receiver,
                    *frontendEntryResources,observerOwnerThread,&diagnostic);
                frontendReport.routeArrivalStage=diagnostic.stage;frontendReport.routeArrivalReads=r.reads;
                frontendReport.mapResetStage=diagnostic.reset.stage;frontendReport.mapResetPropertyFailure=diagnostic.reset.property.failure;
                frontendReport.mapResetFlags=diagnostic.reset.property.flags;frontendReport.mapResetField=diagnostic.reset.property.field;
                frontendReport.mapResetOffset=diagnostic.reset.property.offset;frontendReport.mapResetSavedNext=diagnostic.reset.savedNext;
                frontendReport.mapResetCurrentNext=diagnostic.reset.currentNext;
                if(okay){*frontendPickerSources=frontendCounselorPicker->source;frontendSettingsIdentity=frontendEntryResources->settingsIdentity;}
                return finish(okay);
            }
            return finish(cfgmenu::pickerarrival::Current(r,actual,*frontendCounselorPicker,expected.counselor,receiver,
                *frontendEntryResources,observerOwnerThread,&diagnostic));
        }catch(...){diagnostic.stage=5;return finish(false);}
    }
    bool ArrivalCurrent(const cfgmenu::active::Context& actual,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::pickerprepare::Receiver& receiver)noexcept{return ObserveRoute(actual,expected,receiver,true);}
    bool RouteCurrent(const cfgmenu::active::Context& actual,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::pickerprepare::Receiver& receiver)noexcept{return ObserveRoute(actual,expected,receiver,false);}
    bool Guard(const cfgmenu::pickercoordinator::Call& call,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::pickerprepare::Receiver& receiver,bool constructing,cfgmenu::pickercoordinator::Invocation& out)noexcept{
        using namespace cfgmenu;using K=pickercoordinator::Kind;out={};permittedParameter=0;
        using Stage=pickerdiagnostic::Guard;pickerdiagnostic::Trace trace;Reader r;r.cap=16384;
        const auto stage=[&](Stage s){trace.guardStage=static_cast<unsigned>(s);};
        const auto finish=[&](bool okay){
            trace.reads=r.reads;trace.cap=r.cap;trace.error=r.error;trace.budgetAtLimit=r.reads>=r.cap?1u:0u;
            frontendReport.routeReads=r.reads;frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);
            if(call.cell<pickercoordinator::Cells){if(okay)++frontendReport.pickerGuardPasses[call.cell];
                else{++frontendReport.pickerGuardFailures[call.cell];frontendReport.pickerGuard[call.cell]=trace;}}return okay;};
        stage(Stage::Prerequisites);
        try {if(!Process()||!frontendCounselorPicker||!frontendEntryRoute||call.cell>=pickercoordinator::Cells)return finish(false);
            K kind=K::Other;
            if(call.cell<5){stage(Stage::CheapFrame);if(!PickerCheapFrame(r,*frontendCounselorPicker,call,kind,&trace))return finish(false);
                stage(Stage::OtherRole);if(kind==K::Other)return finish(false);}
            else{stage(Stage::EntryReceiver);if(call.widget!=frontendSource.widget)return finish(false);}
            stage(Stage::ActiveContext);active::Context actual;if(!active::Observe(r,frontendSource.module,actual))return finish(false);
            if(call.cell>=5){entryroute::Invocation invocation;
                stage(Stage::EntryBorrowed);if(!entryroute::Borrowed(r,*frontendEntryRoute,expected.entry,call.widget,call.frame,observerOwnerThread,invocation))return finish(false);
                stage(Stage::EntrySource);if(!pickersource::Retained(r,actual,*frontendPickerSources,observerOwnerThread))return finish(false);
                kind=invocation.role==entryroute::Role::Click?K::EntryClick:invocation.role==entryroute::Role::Graph?K::EntryGraph:K::EntryTransition;
                out.parameter={invocation.locals,invocation.value};
            }else if(kind==K::Construct){
                if(constructing){stage(Stage::OwnedConstruct);if(receiver.widget!=call.widget||!pickerprepare::OwnedReceiverCurrent(r,actual,*frontendCounselorPicker,expected.counselor,
                        receiver,observerOwnerThread,frontendConstructorState.needsWidening))return finish(false);}
                else{stage(Stage::FirstConstruct);pickerprepare::Receiver structural;
                    const bool first=!receiver.widget;
                    const bool valid=first?pickerprepare::FirstInitializedConstruct(r,actual,*frontendCounselorPicker,expected.counselor,
                        call.widget,call.frame,observerOwnerThread,structural,&trace):pickerprepare::FirstConstruct(r,actual,*frontendCounselorPicker,expected.counselor,
                        call.widget,call.frame,observerOwnerThread,structural,&trace);
                    if(!valid)return finish(false);
                    if(first){*frontendPickerSources=frontendCounselorPicker->source;frontendReport.nativeMetadataSerialAssignments+=trace.nativeSerialAssignments;}
                }
            }else{
                stage(Stage::OwnedReceiver);
                if(actual.widget!=call.widget||receiver.widget!=call.widget||!pickerprepare::OwnedReceiverCurrent(r,actual,*frontendCounselorPicker,
                        expected.counselor,receiver,observerOwnerThread))return finish(false);
                stage(Stage::SettingsSource);entrysettings::ResetDiagnostic settingsDiagnostic;
                const bool settingsRetained=entrysettings::Retained(r,actual,*frontendEntryResources,&trace.sourceStage,&settingsDiagnostic);
                frontendReport.mapResetStage=settingsDiagnostic.stage;
                frontendReport.mapResetPropertyFailure=settingsDiagnostic.property.failure;
                frontendReport.mapResetField=settingsDiagnostic.property.field;
                frontendReport.mapResetFlags=settingsDiagnostic.property.flags;
                frontendReport.mapResetOffset=settingsDiagnostic.property.offset;
                frontendReport.mapResetSavedNext=settingsDiagnostic.savedNext;
                frontendReport.mapResetCurrentNext=settingsDiagnostic.currentNext;
                if(!settingsRetained)return finish(false);
                if(kind==K::Transition){stage(Stage::TransitionConsumer);if(!counselortransition::ConsumerCurrent(r,*frontendCounselorPicker,frontendCounselorConsumer))return finish(false);
                    stage(Stage::TransitionFrame);if(!counselortransition::Borrowed(r,actual,*frontendCounselorPicker,receiver,call.widget,call.frame,observerOwnerThread,out.parameter))return finish(false);}
                if(kind==K::Accept&&call.cell==2){Reader choiceReader;choiceReader.cap=acceptedcounselor::ReadLimit;
                    frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;unsigned choiceStage=0;
                    const bool captured=acceptedcounselor::Observe(choiceReader,actual,*frontendCounselorPicker,frontendAcceptedCounselor,&choiceStage);
                    frontendReport.acceptedChoiceStage=choiceStage;frontendReport.acceptedChoiceReads=choiceReader.reads;
                    if(!captured)return finish(false);
                    frontendReport.acceptedChoiceIndex=static_cast<unsigned>(frontendAcceptedCounselor.index);
                    frontendReport.acceptedChoicePath=frontendAcceptedCounselor.name.data;
                }
            }
            if(kind==K::EntryTransition||kind==K::Transition){permittedParameter=out.parameter.address;
                parameterPrevious=kind==K::EntryTransition?frontendEntryBinding.sandboxMap:0;
                parameterTarget=kind==K::EntryTransition?frontendPickerSources->counselor:frontendEntryResources->map;}
            out.kind=kind;out.actual=actual;stage(Stage::None);return finish(true);
        }catch(...){stage(Stage::Exception);return finish(false);}
    }
    bool AdmitConstruct(const cfgmenu::pickercoordinator::Call& call,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::active::Context& actual,cfgmenu::pickerprepare::Receiver& receiver)noexcept{
        try {if(!Process()||!frontendEngine||!frontendCounselorPicker)return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!EnsureConstructScope(call,expected,actual))return false;
#endif
            if(!frontendEngine->BeginPhase())return false;Reader r;r.cap=16384;
            const auto calls=frontendEngine->Calls();const bool okay=cfgmenu::pickerprepare::FirstOwnedConstruct(r,*frontendEngine,actual,*frontendCounselorPicker,
                expected.counselor,call.widget,call.frame,observerOwnerThread,receiver);
            frontendReport.ownerGetterCalls+=frontendEngine->Calls()-calls;frontendReport.routeReads=r.reads;
            frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);
            if(okay)++frontendReport.constructorAdmissions;else frontendReport.routeFailure=2;return okay;
        }catch(...){return false;}
    }
    template<class Forward> bool Construct(const cfgmenu::pickercoordinator::Call& call,const cfgmenu::pickercoordinator::Expected& expected,
        const cfgmenu::pickerprepare::Receiver& receiver,cfgmenu::counselorpicker::ScopedConstruct& scope,Forward&& forward)noexcept{
        try {if(scope.State()==cfgmenu::counselorpicker::ConstructState::Forwarding)return forward();Reader r;r.cap=16384;cfgmenu::active::Context actual;
            if(!Process()||!cfgmenu::active::Observe(r,frontendSource.module,actual))return forward();
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!EnsureConstructScope(call,expected,actual)){frontendPickerCoordinator->Retire();return forward();}
#endif
            if(
               !cfgmenu::pickerprepare::CounselorState(r,actual,*frontendCounselorPicker,expected.counselor,receiver,observerOwnerThread,frontendConstructorState)){
                frontendReport.routeFailure=3;frontendPickerCoordinator->Retire();return forward();}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!frontendConstructRoots->RetainConstructorState(frontendConstructorState)||!frontendConstructRoots->Required()){
                frontendReport.routeFailure=3;frontendPickerCoordinator->Retire();return forward();}
#endif
            cfgmenu::counselorpicker::NativePreparedCastHost<Reader> host(r,*frontendCounselorPicker,expected.counselor,receiver,frontendConstructorState,call.frame);
            const auto retainedForward=[&]()noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                if(!frontendConstructRoots->BeginOriginal())return false;
                const bool returned=forward();return frontendConstructRoots->EndOriginal(returned)&&returned;
#else
                return forward();
#endif
            };
            const bool okay=scope.Run(host,retainedForward);frontendReport.routeReads=r.reads;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(scope.State()==cfgmenu::counselorpicker::ConstructState::Indeterminate)frontendConstructRoots->Uncertain();
#endif
            const auto& diagnostic=host.Trace();frontendReport.constructState=static_cast<unsigned>(scope.State());
            frontendReport.constructStage=diagnostic.stage;frontendReport.constructReads=r.reads;
            frontendReport.constructStateStage=diagnostic.state.stage;frontendReport.constructMetadataStage=diagnostic.state.metadataStage;
            frontendReport.constructSourceStage=diagnostic.state.sourceStage;
            frontendReport.constructCurrentChecks=diagnostic.currentChecks;frontendReport.constructOwnedChecks=diagnostic.ownedChecks;
            frontendReport.constructWidenResult=diagnostic.widenResult;frontendReport.constructRestoreResult=diagnostic.restoreResult;
            frontendReport.constructError=diagnostic.error;frontendReport.constructBudgetHit=r.reads>=r.cap?1u:0u;
            frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);return okay;
        }catch(...){frontendReport.routeFailure=3;if(frontendPickerCoordinator)frontendPickerCoordinator->Retire();return forward();}
    }
    bool ExchangeParameter(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t replacement)noexcept{
        if(!Process()||!permittedParameter||at!=permittedParameter||
           !((expected==parameterPrevious&&replacement==parameterTarget)||(expected==parameterTarget&&replacement==parameterPrevious)))return false;
        return cfgmenu::ExchangeOwnedNullablePointer(at,expected,replacement)==cfgmenu::sandboxstart::WriteResult::Changed;
    }
};
struct FrontendEntryReturnHost {
    bool ReturnCurrent(const cfgmenu::active::Context& previous,const cfgmenu::active::Context& actual)noexcept{
        try {
            FrontendPickerHost owner;if(!owner.Process()||!frontendPickerCoordinator||!frontendEntryRoute||
                !frontendCounselorPicker||!frontendEntryResources||previous.type!=actual.type)return false;
            Reader r;r.cap=32768;const auto expected=frontendPickerCoordinator->ExpectedExecutors();
            cfgmenu::entryroute::Binding rebound;cfgmenu::hud::Binding transport;
            const bool okay=cfgmenu::active::Current(r,actual,observerOwnerThread)&&
                cfgmenu::pickerprepare::Current(r,actual,*frontendCounselorPicker,observerOwnerThread,expected.counselor)&&
                cfgmenu::entryroute::Rebind(r,*frontendEntryRoute,actual,expected.entry,rebound)&&
                cfgmenu::hud::Observe(r,actual,true,transport)&&transport.event==frontendBinding.event;
            frontendReport.routeReads=r.reads;frontendReport.routeMaximumReads=(std::max)(frontendReport.routeMaximumReads,r.reads);
            return okay;
        }catch(...){return false;}
    }
    bool RelabeledCurrent(const cfgmenu::active::Context& actual)noexcept{
        try {if(!frontendEntryLabels||!frontendEntryLabels->Ready()||!(frontendEntryBinding.context==actual))return false;
            Reader r;r.cap=16384;return cfgmenu::entry::Current(r,frontendEntryBinding,observerOwnerThread);
        }catch(...){return false;}
    }
};
bool PrepareFrontendEntryReturn(const cfgmenu::active::Context& actual){
    using namespace cfgmenu;FrontendEntryReturnHost host;
    if(!frontendPickerCoordinator->ObserveReturn(host,actual,frontendEntryReturnTicket))return false;
    Reader r;r.cap=32768;entryroute::Binding rebound;
    if(!entryroute::Rebind(r,*frontendEntryRoute,actual,frontendPickerCoordinator->ExpectedExecutors().entry,rebound)||
       !AdoptFrontend(actual))return false;
    // Only the current native instance/children change. Metadata and the eight
    // resident executor cells remain exactly the already-owned publication.
    frontendSource=actual;frontendEntryResources->source=actual;frontendPickerSources->source=actual;
    frontendCounselorPicker->source.source=actual;frontendEntryBinding=rebound.entry;*frontendEntryRoute=rebound;
    frontendEntryLabels=std::make_unique<FrontendEntryLabels>(frontendReader,*frontendEngine,*frontendText,frontendEntryBinding,frontendBinding,true);
    frontendReport.entrySetupStarted=GetTickCount64();frontendReport.entrySetupElapsedMs=0;frontendReport.entrySetupCallbacks=0;
    frontendReport.entryStage=0;return !frontendEntryLabels->Refused();
}
void SyncPickerReport()noexcept{
    if(!frontendPickerCoordinator)return;frontendReport.callbackState=static_cast<unsigned>(frontendPickerCoordinator->PublicationState());
    frontendReport.callbackCells=frontendPickerCoordinator->Installed();frontendReport.routeState=static_cast<unsigned>(frontendPickerCoordinator->RouteState());
}
void FrontendPickerCall(unsigned cell,void* receiver,void* frame,void* result,std::uintptr_t callbackPc=0)noexcept{
    const cfgmenu::pickercoordinator::Call call{cell,reinterpret_cast<std::uintptr_t>(receiver),reinterpret_cast<std::uintptr_t>(frame),reinterpret_cast<std::uintptr_t>(result)};
    if(cfgmenu::pickercoordinator::ForwardForeign(GetCurrentThreadId(),observerOwnerThread,frontendPickerOriginal,call,PickerNativeForward))return;
    if(cell<cfgmenu::pickercoordinator::Cells)++frontendReport.pickerCallbacks[cell];
    // The retained menu stage now includes original Accept/map/settings. Row
    // creation+attachment stays atomic under the HUD lease; native match Start
    // remains independently disabled in FrontendEntry.
    FrontendPickerHost host;host.callbackPc=callbackPc;
    if(!frontendPickerCoordinator){host.Forward(frontendSource.module+cfgmenu::startdisk::ScriptExecutor,call);return;}
    const auto before=frontendPickerCoordinator->RouteState();const auto redirects=frontendReport.acceptRedirects;
    frontendPickerCoordinator->Invoke(host,call);const auto after=frontendPickerCoordinator->RouteState();
    if(before==cfgmenu::counselorroute::State::EntryRequested&&after==cfgmenu::counselorroute::State::AwaitCounselor)
        frontendCounselorClaim=true;
    if(after==cfgmenu::counselorroute::State::AwaitCounselor||after==cfgmenu::counselorroute::State::CounselorReady||
       (cell==4&&after==cfgmenu::counselorroute::State::EntryReady))FrontendState(8);
    if(cell==4&&after==cfgmenu::counselorroute::State::EntryReady)frontendCounselorClaim=false;
    const auto commitBefore=(frontendReport.acceptRedirects>redirects&&
        (before==cfgmenu::counselorroute::State::MapReady||before==cfgmenu::counselorroute::State::SettingsReady))?
        cfgmenu::counselorroute::State::CounselorReady:before;
    if(cell==2){frontendAcceptedCounselorCommitted=cfgmenu::acceptedcommit::Commit(cfgmenu::softpath::Plain(frontendAcceptedCounselor.path),commitBefore,after);
        frontendReport.acceptedChoiceCommitted=frontendAcceptedCounselorCommitted?1u:0u;}
    if(cell==4||(cell>=5&&after==cfgmenu::counselorroute::State::AwaitCounselor&&before!=after)){
        frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;frontendReport.acceptedChoiceCommitted=0;}
    if(cell==7&&before==cfgmenu::counselorroute::State::EntryRequested&&after==cfgmenu::counselorroute::State::AwaitCounselor)++frontendReport.entryRedirects;
    if(cell==3&&before==cfgmenu::counselorroute::State::AcceptRequested&&after==cfgmenu::counselorroute::State::AwaitMap)++frontendReport.acceptRedirects;
    SyncPickerReport();if(after==cfgmenu::counselorroute::State::Retired){frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;frontendReport.acceptedChoiceCommitted=0;if(!frontendReport.routeFailure)frontendReport.routeFailure=4;FrontendRefused(1051);}
}
void __fastcall FrontendPickerGraph(void* a,void* b,void* c){FrontendPickerCall(0,a,b,c,reinterpret_cast<std::uintptr_t>(_ReturnAddress()));}
void __fastcall FrontendPickerConstruct(void* a,void* b,void* c){FrontendPickerCall(1,a,b,c,reinterpret_cast<std::uintptr_t>(_ReturnAddress()));}
void __fastcall FrontendPickerAccept(void* a,void* b,void* c){FrontendPickerCall(2,a,b,c);}
void __fastcall FrontendPickerTransition(void* a,void* b,void* c){FrontendPickerCall(3,a,b,c);}
void __fastcall FrontendPickerBack(void* a,void* b,void* c){FrontendPickerCall(4,a,b,c);}
void __fastcall FrontendEntryClick(void* a,void* b,void* c){FrontendPickerCall(5,a,b,c);}
void __fastcall FrontendEntryGraph(void* a,void* b,void* c){FrontendPickerCall(6,a,b,c);}
void __fastcall FrontendEntryTransition(void* a,void* b,void* c){FrontendPickerCall(7,a,b,c);}
bool PrepareFrontendPicker(){
    using namespace cfgmenu;if(frontendPickerPrepared)return frontendPickerCoordinator!=nullptr;
    frontendPickerPrepared=true;frontendReport.pickerPreparationStage=1;
    frontendEntryRoute=std::make_unique<entryroute::Binding>();
    if(!entryroute::Prepare(frontendReader,frontendEntryBinding,*frontendEntryRoute))return false;
    frontendReport.pickerPreparationStage=2;
    const auto expected=frontendInheritedPicker?frontendInheritedPicker->ExpectedExecutors().counselor:pickerprepare::Original(*frontendCounselorPicker);
    if(!counselortransition::Prepare(frontendReader,frontendSource,*frontendCounselorPicker,observerOwnerThread,frontendCounselorConsumer,nullptr,&expected))return false;
    pickercoordinator::Configuration config;config.source=frontendSource;config.counselor=frontendPickerSources->counselor;
    config.map=frontendEntryResources->map;config.settings=frontendEntryResources->settings;config.sandbox=frontendEntryBinding.sandboxMap;
    config.original.counselor=pickerprepare::Original(*frontendCounselorPicker);config.original.entry=entryroute::Original(*frontendEntryRoute);
    const std::array<std::uintptr_t,8> thunks={reinterpret_cast<std::uintptr_t>(&FrontendPickerGraph),reinterpret_cast<std::uintptr_t>(&FrontendPickerConstruct),
        reinterpret_cast<std::uintptr_t>(&FrontendPickerAccept),reinterpret_cast<std::uintptr_t>(&FrontendPickerTransition),reinterpret_cast<std::uintptr_t>(&FrontendPickerBack),
        reinterpret_cast<std::uintptr_t>(&FrontendEntryClick),reinterpret_cast<std::uintptr_t>(&FrontendEntryGraph),reinterpret_cast<std::uintptr_t>(&FrontendEntryTransition)};
    config.plan.count=8;for(unsigned i=0;i<8;++i){const auto function=i<5?frontendCounselorPicker->handlers[i].slot.field.object:frontendEntryRoute->functions[i-5].slot.field.object;
        config.plan.cells[i]={function+startdisk::Executor,frontendSource.module+startdisk::ScriptExecutor,thunks[i]};}
    frontendPickerOriginal=frontendSource.module+startdisk::ScriptExecutor;
    frontendPickerCoordinator=std::make_unique<PickerCoordinator>();FrontendPickerHost host;
    const bool prepared=frontendInheritedPicker?frontendPickerCoordinator->PrepareInherited(host,config,*frontendInheritedPicker):
        frontendPickerCoordinator->Prepare(host,config);
    if(!prepared){frontendPickerCoordinator.reset();return false;}
    frontendInheritedPicker=nullptr;
    frontendReport.pickerPreparationStage=3;SyncPickerReport();return true;
}
bool AdmitPickerStart(cfgmenu::startroute::SlotKind kind,void* receiver,void* frame,void* result)noexcept{
    try {if(!frontendPickerCoordinator||!frontendLaunch)return false;
        cfgmenu::startroute::OriginalFrame call{reinterpret_cast<std::uintptr_t>(receiver),reinterpret_cast<std::uintptr_t>(frame),
            reinterpret_cast<std::uintptr_t>(result),kind};
        const auto admission=cfgmenu::pickerstart::Admit(frontendLaunch->CurrentState(),frontendPickerCoordinator->RouteState(),
            cfgmenu::startroute::CurrentFrame(frontendReader,frontendLaunch->StartBinding(),call,observerOwnerThread));
        // Existing Router consumes valid early Start as waiting. It cannot
        // travel or consume the Submit lease while setup is still Opening.
        if(admission==cfgmenu::pickerstart::Admission::Waiting||admission==cfgmenu::pickerstart::Admission::AlreadySubmitted)return true;
        if(admission!=cfgmenu::pickerstart::Admission::Submit)return false;
        cfgmenu::active::Context actual;if(!cfgmenu::active::Observe(frontendReader,frontendSource.module,actual))return false;
        FrontendPickerHost host;const bool accepted=frontendPickerCoordinator->Submit(host,actual);SyncPickerReport();return accepted;
    }catch(...){return false;}
}
