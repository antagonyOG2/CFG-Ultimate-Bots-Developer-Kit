// Included inside the isolated observer's private namespace. Transport lifetime
// is process-scoped; native menu/session ownership is a separate transaction.
bool frontendGameplayReady=false;
bool frontendNativeArrivalStarted=false;
Reader frontendReader;
cfgmenu::hud::Binding frontendBinding;
cfgmenu::active::Context frontendSource{};
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
bool FrontendPreviousWorldAllowsLoad()noexcept;
void SyncWorldRootReport()noexcept;
void FrontendWorldRetirementBind(Reader&,const cfgmenu::active::Context&)noexcept;
void FrontendWorldRetirementAfter(void*,void*,bool,bool,std::uintptr_t)noexcept;
#else
bool FrontendPreviousWorldAllowsLoad()noexcept{return true;}
void SyncWorldRootReport()noexcept{}
#endif
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
#include "FrontendRetentionRuntime.hpp"
#include "NativeStartupRetentionRuntime.hpp"
#include "FrontendStartRetentionRuntime.hpp"
#endif
std::unique_ptr<cfgmenu::rows::NativeDispatch> frontendEngine;
std::unique_ptr<cfgmenu::reflection::RetainedMenuText> frontendText;
using FrontendLaunch=cfgmenu::launch::NativeMenuLaunch<Reader,cfgmenu::rows::NativeDispatch>;
std::unique_ptr<FrontendLaunch> frontendLaunch;
// Native Push can replace widgets; cached instances keep their owned rows.
// Never erase a still-attached row owner or append again to the same identity.
std::array<std::unique_ptr<FrontendLaunch>,64> frontendCachedLaunches;
std::optional<cfgmenu::startbinding::Binding> frontendSharedStartBinding;
bool frontendCounselorClaim=false;
cfgmenu::active::Context frontendLastPickerArrival{};
bool frontendAttempted=false;
std::array<std::u16string,cfglimits::MaximumCounselors> frontendCountLabels;
using FrontendReport=cfgmenu::FrontendReport;
FrontendReport frontendReport;
void FrontendState(unsigned)noexcept;
void FrontendRefused(unsigned)noexcept;
void FrontendHumanJasonIntroSettings(void*)noexcept;
bool FrontendRestartCounselor(void*)noexcept;
bool RetireChallengeAttempt(Reader&,const cfgmenu::active::Context&)noexcept;
unsigned frontendRestartAttempts=0,frontendRestartReturns=0,frontendRestartFailure=0;
void SyncNativeOfflineReport()noexcept{
    frontendReport.counselorRestartAttempts=frontendRestartAttempts;frontendReport.counselorRestartReturns=frontendRestartReturns;
    frontendReport.counselorRestartFailure=frontendRestartFailure;
    const auto& s=nativeOfflineStartup.Status();
    frontendReport.nativeStartupDetail=s.detail;
    frontendReport.nativeHookState=static_cast<unsigned>(nativeOfflineHooks.Status());
    frontendReport.nativeHookError=nativeOfflineHooks.LastError();
    frontendReport.nativeStartupStage=static_cast<unsigned>(s.stage);frontendReport.nativeStartupFailure=s.failure;
    frontendReport.nativeStartupBots=s.bots;frontendReport.nativeStartupReads=nativeOfflineReader.reads;
    frontendReport.nativeStartupWorld=s.world;frontendReport.nativeStartupHuman=s.human;
    frontendReport.nativeIntroCalled=s.introCalled?1u:0u;frontendReport.nativeMatchCalled=s.matchCalled?1u:0u;
    frontendReport.nativeCleanupAttempted=s.cleanupAttempted?1u:0u;frontendReport.nativeCleanupRestored=s.cleanupRestored?1u:0u;
    const auto& presentation=nativeOfflineStartup.Presentation();
    frontendReport.introPresentationReady=presentation.Ready()?1u:0u;
    frontendReport.introPresentationStage=presentation.stage;frontendReport.introPresentationSkin=presentation.observedSkin;
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
bool ObserveNativeStartupEntry(cfgmenu::startupretain::Role role,std::uintptr_t receiver,std::uintptr_t pc,
    cfgmenu::promotion::Context* observed=nullptr)noexcept{
    if(observed)*observed={};
    if(GetCurrentThreadId()!=observerOwnerThread)return false;
    bool ownerObserved=false;
    auto& startupReport=frontendRetentionReport;auto& entry=startupReport.startup[role==cfgmenu::startupretain::Role::Intro?0:1];
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&startupReport.generation));
    ++entry.candidates;entry.receiver=receiver;entry.returnPc=pc;
    namespace p=cfgcompat::cfg35::startupEntry;
    entry.expectedPc=plan.module+(role==cfgmenu::startupretain::Role::Intro?p::IntroReturnPc:p::MatchReturnPc);
    Reader reader;reader.cap=16384;unsigned stage=0;
    if(cfgmenu::startupentry::Origin(reader,plan.module,receiver,role,pc,GetCurrentThreadId(),observerOwnerThread,nativeOfflineHooks.Enabled(),stage)){
        ++entry.originPassed;
        const auto& c=frontendSource;
        const cfgmenu::freshgame::Source source{c.module,c.engine,c.viewport,c.player,c.gameInstance,c.world,c.thread,true};
        cfgmenu::promotion::Context owner;
        if(cfgmenu::offlineowner::Observe(reader,source,receiver,GetCurrentThreadId(),true,owner,nullptr,&stage,
                cfgmenu::offlineowner::Purpose::DiagnosticReadOnly)){
            ++entry.ownerPassed;entry.world=owner.world;
            ownerObserved=true;if(observed)*observed=owner;
        }else stage+=100;
    }
    entry.stage=stage;entry.reads=reader.reads;entry.error=reader.error;
    ++entry.forwards;MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&startupReport.generation));
    return ownerObserved;
}
class NativeStartupCallbackScope {
    NativeStartupRootSession* session_=nullptr;
    std::unique_ptr<cfgmenu::startupretain::Scope> scope_;
    std::unique_ptr<cfgprimary::ScopedLookupEligibility<Reader>> lookup_;
    using OwnerReader=cfgmenu::offlineowner::Budget<Reader>;
    std::unique_ptr<cfgprimary::ScopedLookupEligibility<OwnerReader>> ownerLookup_;
public:
    NativeStartupCallbackScope(cfgmenu::startupretain::Role role,void* receiver,std::uintptr_t pc){
        cfgmenu::promotion::Context owner;
        if(!ObserveNativeStartupEntry(role,reinterpret_cast<std::uintptr_t>(receiver),pc,&owner))return;
        const bool intro=role==cfgmenu::startupretain::Role::Intro;
        const bool armed=nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Armed;
        const bool replacing=nativeStartupRootSession&&intro&&armed;
        if(!cfgstartupreuse::Prepare(nativeStartupRootSession,intro,armed,cfgmenu::startupretain::Scope::Occupied(),
            [&](){return std::make_unique<NativeStartupRootSession>(owner);}))return;
        if(replacing)cfgclosedtrace::Mark("startup-root-session-renewed",owner.world,owner.identities[0].serial);
        if(!nativeStartupRootSession->Begin(role,owner,pc))return;
        session_=nativeStartupRootSession.get();
        scope_=std::make_unique<cfgmenu::startupretain::Scope>(owner.module,role,session_->Hooks(),true);
        if(scope_->Entered())lookup_=std::make_unique<cfgprimary::ScopedLookupEligibility<Reader>>(owner.module,session_,
            [](Reader& reader,void* context){return static_cast<NativeStartupRootSession*>(context)->LookupCurrent(reader);},true);
        // NativeOfflineOwner Observe/VerifyHuman use a bounded reader view.
        // Eligibility is reader-typed; admitting only Reader makes that view
        // reject the SAME valid callback at its first runtime lookup (PID16812).
        // Its validator reads through the bounded view and renews the same
        // session roots/context; no unwrapping or diagnostic-only fallback.
        if(lookup_&&lookup_->Entered())ownerLookup_=std::make_unique<cfgprimary::ScopedLookupEligibility<OwnerReader>>(owner.module,session_,
            [](OwnerReader& reader,void* context){return static_cast<NativeStartupRootSession*>(context)->LookupCurrent(reader);},true);
    }
    ~NativeStartupCallbackScope(){ownerLookup_.reset();lookup_.reset();scope_.reset();if(session_)session_->End();}
    bool Entered()const noexcept{return scope_&&scope_->Entered()&&lookup_&&lookup_->Entered()&&ownerLookup_&&ownerLookup_->Entered();}
    bool Finish()noexcept{return Entered()&&session_->Finish();}
};
#ifdef CFG35_NATIVE_STARTUP_ROOT_TEST
bool ForwardNativeStartupStock(cfgmenu::offlinehooks::Handler original,void* mode)noexcept{
    __try{original(mode);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void ObserveRetainedNativeStartup(cfgmenu::startupretain::Role role,void* mode,std::uintptr_t pc)noexcept{
    using namespace cfgmenu;
    promotion::Context owner;
    const auto original=role==startupretain::Role::Intro?nativeOfflineOriginalIntro:nativeOfflineOriginalMatch;
    if(!ObserveNativeStartupEntry(role,reinterpret_cast<std::uintptr_t>(mode),pc,&owner)){
        original(mode);return;
    }
    if(!nativeStartupRootSession){
        if(role!=startupretain::Role::Intro){original(mode);return;}
        nativeStartupRootSession=std::make_unique<NativeStartupRootSession>(owner);
    }
    if(!nativeStartupRootSession->Begin(role,owner,pc)){original(mode);return;}
    struct CallbackExit {NativeStartupRootSession& session;~CallbackExit(){session.End();}} exit{*nativeStartupRootSession};
    startupretain::Scope scope(owner.module,role,nativeStartupRootSession->Hooks(),true);
    cfgprimary::ScopedLookupEligibility<Reader> lookup(owner.module,nativeStartupRootSession.get(),
        [](Reader& reader,void* context){return static_cast<NativeStartupRootSession*>(context)->LookupCurrent(reader);},scope.Entered());
    if(!scope.Entered()||!lookup.Entered()){original(mode);return;}
    bool entered=false;
    const bool returned=startupretain::Invoke(owner.module,
        role==startupretain::Role::Intro?startupretain::Call::ParentIntro:startupretain::Call::ParentMatch,
        promotion::RetainedInputs(owner),[&](){entered=true;return ForwardNativeStartupStock(original,mode);});
    // Never replay after an entered native call, even if completion/owner proof
    // fails. This diagnostic performs no replacement spawn/restart/AI work.
    if(!entered){original(mode);return;}
    if(returned&&role==startupretain::Role::Match)(void)nativeStartupRootSession->Finish();
}
#endif
#endif
void __fastcall NativeOfflineIntro(void* mode){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    const auto nativeEntryPc=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
#endif
#ifdef CFG35_NATIVE_STARTUP_ENTRY_TEST
    const auto pc=nativeEntryPc;
#ifdef CFG35_NATIVE_STARTUP_ROOT_TEST
    ObserveRetainedNativeStartup(cfgmenu::startupretain::Role::Intro,mode,pc);return;
#else
    ObserveNativeStartupEntry(cfgmenu::startupretain::Role::Intro,reinterpret_cast<std::uintptr_t>(mode),pc);
    nativeOfflineOriginalIntro(mode);return;
#endif
#else
    if(GetCurrentThreadId()==observerOwnerThread){
        FrontendRestartCounselor(mode);
        nativeOfflineReader.reads=0;nativeOfflineReader.error=0;nativeOfflineReader.cap=131072;
    }
    if(GetCurrentThreadId()!=observerOwnerThread||!nativeOfflineStartup.Claimed(reinterpret_cast<std::uintptr_t>(mode),GetCurrentThreadId())){
        FrontendHumanJasonIntroSettings(mode);nativeOfflineOriginalIntro(mode);return;
    }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    NativeStartupCallbackScope nativeScope(cfgmenu::startupretain::Role::Intro,mode,nativeEntryPc);
    if(!nativeScope.Entered()){nativeOfflineStartup.Abort(601);SyncNativeOfflineReport();FrontendRefused(1001);return;}
#endif
    nativeOfflineReader.reads=0;nativeOfflineReader.error=0;nativeOfflineReader.cap=131072;
    try{nativeOfflineStartup.Intro(reinterpret_cast<std::uintptr_t>(mode));}
    catch(...){nativeOfflineStartup.Abort(99);}
    SyncNativeOfflineReport();
    if(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Refused)FrontendRefused(1002);
#endif
}
void __fastcall NativeOfflineMatch(void* mode){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    const auto nativeEntryPc=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
#endif
#ifdef CFG35_NATIVE_STARTUP_ENTRY_TEST
    const auto pc=nativeEntryPc;
#ifdef CFG35_NATIVE_STARTUP_ROOT_TEST
    ObserveRetainedNativeStartup(cfgmenu::startupretain::Role::Match,mode,pc);return;
#else
    ObserveNativeStartupEntry(cfgmenu::startupretain::Role::Match,reinterpret_cast<std::uintptr_t>(mode),pc);
    nativeOfflineOriginalMatch(mode);return;
#endif
#else
    if(GetCurrentThreadId()==observerOwnerThread){nativeOfflineReader.reads=0;nativeOfflineReader.error=0;nativeOfflineReader.cap=65536;}
    if(GetCurrentThreadId()!=observerOwnerThread||!nativeOfflineStartup.Claimed(reinterpret_cast<std::uintptr_t>(mode),GetCurrentThreadId())){
        nativeOfflineOriginalMatch(mode);return;
    }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    NativeStartupCallbackScope nativeScope(cfgmenu::startupretain::Role::Match,mode,nativeEntryPc);
    if(!nativeScope.Entered()){nativeOfflineStartup.Abort(602);SyncNativeOfflineReport();FrontendRefused(1003);return;}
#endif
    nativeOfflineReader.reads=0;nativeOfflineReader.error=0;nativeOfflineReader.cap=65536;
    try{nativeOfflineStartup.Match(reinterpret_cast<std::uintptr_t>(mode));}
    catch(...){nativeOfflineStartup.Abort(99);}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Ready&&!nativeScope.Finish())nativeOfflineStartup.Abort(603);
#endif
    SyncNativeOfflineReport();
    if(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Refused)FrontendRefused(1004);
#endif
}
#include "TommyEndRaceRuntime.inl"
bool PrepareNativeOfflineHooks(std::uintptr_t module,HMODULE resident)noexcept{
    const cfgmenu::offlinehooks::Request request{module,reinterpret_cast<std::uintptr_t>(resident),
        &NativeOfflineIntro,&NativeOfflineMatch,true,true,true};
    if(!nativeOfflineHooks.Prepare(request)){SyncNativeOfflineReport();return false;}
    nativeOfflineOriginalIntro=nativeOfflineHooks.OriginalIntro();nativeOfflineOriginalMatch=nativeOfflineHooks.OriginalMatch();
#if defined(CFG35_NATIVE_STARTUP_ENTRY_TEST) || defined(CFG35_NATIVE_COUNSELOR_STARTUP_TEST)
    // Observe only these original native entries; do not install unrelated
    // feature hooks. The counselor startup candidate admits the EXISTING
    // coordinator, but not the later gameplay backend or optional hooks.
#ifdef CFG35_KNIFE_RELEASE_ADAPTER
    if(!PrepareBundleKnifeReleaseAdapter(module))return false;
#elif defined(CFG35_KNIFE_BOUNDARY_OBSERVE)
    if(!PrepareBundleKnifeBoundaryObservation(module))return false;
#endif
#ifdef CFG35_PRIMARY_GAMEPLAY_TEST
    // Full parity candidate includes native human-Jason difficulty as well as
    // the counselor route. Isolated startup probes still install no hit hook.
    if(!PrepareBundleDifficultyHook(module))return false;
    (void)PrepareJ215PracticeHooks(module); // independent optional presentation filter; OFF forwards unchanged
#if CFG_H276_SOUND_EMITTER
    (void)PrepareBundleSoundHook(module); // donor continuous blip suppression; a failed install never blocks startup (original forwards)
#endif
    // The full candidate also needs the EXISTING UserWidget event bridge.
    // Actor ProcessEvent cannot observe spectator status labels; without this
    // hook the current Jason view keeps the last counselor's display text.
    // Isolated native-startup probes retain their original minimal hook set.
    if(!PrepareBundleWidgetDispatcher(module))return false;
#endif
    MemoryBarrier();return nativeOfflineHooks.Enable();
#else
    if(!PrepareBundleSoundHook(module))return false;
    if(!PrepareBundleTrapHook(module))return false;
    if(!PrepareBundleDifficultyHook(module))return false;
    if(!PrepareBundleSweaterCleanup(module))return false;
    if(!PrepareBundleKnifeNotify(module))return false;
    if(!PrepareBundleSpectatorCycle(module))return false;
    if(!PrepareBundleWidgetDispatcher(module))return false;
    if(!PrepareNativeTommyEndGuard(module))return false;
    MemoryBarrier();const bool enabled=nativeOfflineHooks.Enable();SyncNativeOfflineReport();return enabled;
#endif
}
bool PrepareNativeOfflineStart(const cfgmenu::MenuCatalogueEvidence& evidence,cfgmenu::GameplayHandoff& handoff,const cfgmenu::active::Context& c){
    cfgmenu::MenuCatalogueEvidence armed;cfgmenu::Selection selection;std::uint64_t submission=0;
    if(!nativeOfflineHooks.Enabled()||GetCurrentThreadId()!=observerOwnerThread||!handoff.InspectArmed(armed,selection,submission)||
       armed.frontend!=evidence.frontend||armed.generation!=evidence.generation||armed.frontendWorld!=c.world||
       !armed.counselorPathSelection||!cfgmenu::softpath::Plain(armed.selectedCounselor)||!selection.counselorCount)return false;
    if(!RetireChallengeAttempt(frontendReader,c))return false;
    cfgmenu::promotion::Packet counselor{},killer{};counselor.bytes=armed.selectedCounselor;killer.bytes=armed.selectedJason;
    // Never retain weak-object cache tags from a preview/catalogue selection.
    std::fill(counselor.bytes.begin(),counselor.bytes.begin()+16,static_cast<unsigned char>(0));
    std::fill(killer.bytes.begin(),killer.bytes.begin()+16,static_cast<unsigned char>(0));
    const cfgmenu::freshgame::Source source{c.module,c.engine,c.viewport,c.player,c.gameInstance,c.world,c.thread,true};
    const auto skill=selection.skillEnabled?(selection.skill==0?1u:selection.skill==1?0u:2u):3u;
    const bool okay=nativeOfflineStartup.Arm(source,counselor,killer,selection.counselorCount-1,skill,armed.selectedClass);
    SyncNativeOfflineReport();return okay;
}
cfgmenu::arrivalwindow::Window frontendArrivalWindow;
cfgmenu::observedarrival::Observation frontendArrival;
cfgmenu::prebirth::AcceptedCounselorPrebirth frontendPrebirth;
std::uint64_t frontendPrepareAt=0;
bool FrontendRestartCounselor(void* mode)noexcept{
    if(!frontendGameplayReady||!frontendLaunch||!nativeOfflineStartup.CanResetEpoch())return false;
    try{
        nativeOfflineReader.reads=0;nativeOfflineReader.error=0;nativeOfflineReader.cap=32768;
        cfgmenu::promotion::Context fresh;
        if(!nativeOfflineStartup.RestartCandidate(reinterpret_cast<std::uintptr_t>(mode),observerOwnerThread,fresh))return false;
        ++frontendRestartAttempts;frontendRestartFailure=1;
        if(!BundleEpochRetirable())return false;
        const auto prior=nativeOfflineStartup.Owner();
        frontendRestartFailure=2;
        if(!frontendLaunch->PendingGameplay().RearmForRestart(prior.world,prior.identities[0].serial,fresh.world,fresh.identities[0].serial))return false;
        frontendRestartFailure=3;
        // Reset only after the old metadata borrow was restored and the new
        // owner is independently proved. No release calls on departed actors.
        if(!nativeOfflineStartup.Restart(reinterpret_cast<std::uintptr_t>(mode),observerOwnerThread)){
            frontendLaunch->PendingGameplay().Cancel();return false;
        }
        BundleResetEpoch();frontendGameplayReady=false;frontendArrival={};frontendArrivalWindow={};frontendPrebirth={};
        frontendNativeArrivalStarted=false;frontendPrepareAt=0;
        frontendReport.arrivalFailure=frontendReport.handoffFailure=frontendReport.spawnQueued=0;
        ++frontendRestartReturns;frontendRestartFailure=0;FrontendState(4);return true;
    }catch(...){frontendRestartFailure=99;return false;}
}
std::uintptr_t frontendSettingsClass=0;
cfgprobe::lifetime::SlotObservation frontendSettingsIdentity{};
std::unique_ptr<cfgmenu::mapcontinuation::Binding> frontendContinuation;
cfgmenu::continuation::Transaction frontendContinuationInstall;
std::uint64_t frontendBridgeStarted=0,frontendBridgeSampleAt=0;
std::unique_ptr<cfgmenu::entrysettings::Resources> frontendEntryResources;
std::unique_ptr<cfgmenu::pickersource::Resources> frontendPickerSources;
std::unique_ptr<cfgmenu::pickerprepare::Prepared> frontendCounselorPicker;
std::uintptr_t frontendPickerListWidget=0;std::uint64_t frontendPickerListAt=0;
void FrontendPickerListAfter(void* object,bool nested,bool completed){
    if(nested||!completed||!object||!frontendPickerSources||
        GetCurrentThreadId()!=observerOwnerThread)return;
#ifdef CFG_GAMEPLAY_BUNDLE
    if(bundleStarted)return;
#endif
    const auto widget=reinterpret_cast<std::uintptr_t>(object),now=GetTickCount64();
    if(widget==frontendPickerListWidget&&now<frontendPickerListAt)return;
    Reader r;r.cap=8192;std::uintptr_t type=0;
    if(!r.Read(widget+cfgprobe::ObjectClass,type)||type!=frontendPickerSources->jason)return;
    frontendPickerListWidget=widget;frontendPickerListAt=now+1000;
    cfgmenu::active::Context c;cfgmenu::pickerlist::Snapshot value;unsigned stage=1;
    if(cfgmenu::active::Observe(r,frontendSource.module,c)&&c.widget==widget&&
        cfgmenu::pickerlist::Observe(r,c,value,stage)){
        frontendReport.pickerRosterModule=c.module;frontendReport.pickerRosterWidget=widget;
        frontendReport.pickerRosterPackets=value.classes.entries;
        frontendReport.pickerRosterCount=static_cast<unsigned>(value.classes.header.count);
    }else frontendReport.pickerRosterCount=0;
    frontendReport.pickerRosterStage=stage;
}
cfgmenu::entry::Binding frontendEntryBinding;
using FrontendEntryLabels=cfgmenu::entry::View<Reader,cfgmenu::rows::NativeDispatch>;
std::unique_ptr<FrontendEntryLabels> frontendEntryLabels;
bool frontendEntryPrepared=false;
const cfgmenu::pickercoordinator::Coordinator* frontendInheritedPicker=nullptr;
bool AdmitPickerStart(cfgmenu::startroute::SlotKind,void*,void*,void*)noexcept;
void __fastcall FrontendStart(void*,void*,void*);
void __fastcall FrontendGraph(void*,void*,void*);
bool FrontendHumanJasonStart(cfgmenu::startroute::SlotKind,void*,void*,void*);
bool FrontendHumanJasonOwns(std::uintptr_t)noexcept;
void FrontendState(unsigned value)noexcept{
    MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&frontendReport.state),static_cast<LONG>(value));
}
void FrontendRefused(unsigned site)noexcept{
    frontendReport.refusalSite=site;
    if(!frontendReport.firstRefusalSite)frontendReport.firstRefusalSite=site;
    FrontendState(5);
}
void FrontendPreparationRefused(unsigned reads,DWORD error) noexcept {
    frontendReport.reads=reads;frontendReport.error=error;FrontendRefused(1005);
}
bool PrepareFrontendResources(Reader& reader,const cfgmenu::pickerprepare::Prepared* priorPicker=nullptr,
    const cfgmenu::pickerprepare::Executors* priorExpected=nullptr){
    using namespace cfgmenu;std::uintptr_t native=0,ctor=0;
    frontendReport.entryFailure=1001;
    if(!plan.frontend.widget||!cfgmenu::screengate::Entry(reader,plan.frontend)||!cfgobserver::ContextUnchanged(reader,plan)||
        !reader.Read(plan.module+selectiondisk::SCGame_MenuClass,native)||
        !reader.Read(plan.module+selectiondisk::SCGame_MenuConstructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor)||
        !cfgspawn::ObjectIs(reader,plan.mode,native))return false;
    frontendReport.entryFailure=1002;
    if(!hud::Observe(reader,plan.frontend,true,frontendBinding))return false;
    frontendReport.entryFailure=0;
    frontendEntryResources=std::make_unique<entrysettings::Resources>();unsigned reads=0,stage=0;
    if(!entrysettings::Observe(reader,plan.frontend,plan.threadId,true,*frontendEntryResources,&reads,&stage)){
        frontendReport.entryFailure=stage;frontendReport.entryReads=reads;return false;}
    // One cold, retained authored source chain. It does not load a picker,
    // publish its callbacks or require a customization visit before entry.
    frontendPickerSources=std::make_unique<pickersource::Resources>();
    unsigned pickerReads=0;
    if(!pickersource::Observe(reader,plan.frontend,plan.threadId,true,*frontendPickerSources,&pickerReads,&stage)){
        frontendReport.entryFailure=300+stage;frontendReport.entryReads=reads+pickerReads;return false;}
    frontendReport.entryReads=reads+pickerReads;
    frontendCounselorPicker=std::make_unique<pickerprepare::Prepared>();
    unsigned counselorReads=0;
    if(priorPicker&&priorExpected&&priorPicker->type==frontendPickerSources->counselor){
        // OfflinePlay can be recreated while Counselor_Menu survives with our
        // five resident callbacks. Revalidate the COMPLETE saved metadata
        // against the NEW actual authored source; never accept foreign hooks.
        *frontendCounselorPicker=*priorPicker;frontendCounselorPicker->source=*frontendPickerSources;
        if(!pickerprepare::Current(reader,plan.frontend,*frontendCounselorPicker,plan.threadId,*priorExpected,false,&stage)){
            frontendReport.entryFailure=500+stage;frontendReport.entryReads=reader.reads;return false;}
    }else if(!pickerprepare::Prepare(reader,plan.frontend,*frontendPickerSources,pickerprepare::Kind::Counselor,
            plan.threadId,true,*frontendCounselorPicker,&counselorReads,&stage)){
        frontendReport.entryFailure=400+stage;frontendReport.entryReads+=counselorReads;return false;}
    frontendReport.entryReads+=counselorReads;
    // Normal Jason selection remains native. Its independent adapter is tested,
    // but preparing unused Jason graph metadata here adds cold work needlessly.
    frontendSettingsIdentity=frontendEntryResources->settingsIdentity;
    frontendSource=plan.frontend;frontendSettingsClass=frontendEntryResources->settings;
    frontendReport.entryFailure=0;
    return true; // Read-only preparation; readiness published only after all gates.
}
void FrontendOriginal(void* receiver,void* frame,void* result){
    reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(frontendBinding.context.module+cfgmenu::startdisk::ScriptExecutor)(receiver,frame,result);
}
void FrontendHandle(cfgmenu::startroute::SlotKind kind,void* receiver,void* frame,void* result,std::uintptr_t callbackPc){
    if(!frontendLaunch||GetCurrentThreadId()!=observerOwnerThread){FrontendOriginal(receiver,frame,result);return;}
    // Shared native settings class is also used by ordinary Jason matches.
    // A different widget instance never inherits the dedicated counselor claim.
    if(reinterpret_cast<std::uintptr_t>(receiver)!=frontendLaunch->StartBinding().context.widget){
        if(!frontendCounselorClaim)FrontendOriginal(receiver,frame,result);
        return; // A fresh counselor settings instance must be adopted first.
    }
    if(frontendCounselorClaim&&frontendReport.state!=3&&frontendReport.state!=4)return;
    if(!FrontendPreviousWorldAllowsLoad())return; // never retain/replay this borrowed FFrame
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    const cfgmenu::startroute::OriginalFrame invocation{reinterpret_cast<std::uintptr_t>(receiver),
        reinterpret_cast<std::uintptr_t>(frame),reinterpret_cast<std::uintptr_t>(result),kind};
    FrontendStartRootProvider roots(frontendLaunch->StartBinding(),invocation,callbackPc,
        reinterpret_cast<std::uintptr_t>(&FrontendStart),reinterpret_cast<std::uintptr_t>(&FrontendGraph));
    cfgmenu::startupretain::Scope startupScope(plan.module,cfgmenu::startupretain::Role::Start,roots.Hooks(),roots.Admitted());
    cfgprimary::ScopedLookupEligibility<Reader> lookup(plan.module,&roots,
        [](Reader& reader,void* p){return static_cast<FrontendStartRootProvider*>(p)->LookupCurrent(reader);},startupScope.Entered());
    if(!startupScope.Entered()||!lookup.Entered()){frontendReport.routeFailure=12;FrontendRefused(1006);return;}
#else
    (void)callbackPc;
#endif
    frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=32768;
    if(!AdmitPickerStart(kind,receiver,frame,result)){frontendReport.routeFailure=9;FrontendRefused(1007);return;}
    const auto outcome=frontendLaunch->Handle(kind,GetCurrentThreadId(),reinterpret_cast<std::uintptr_t>(receiver),
        reinterpret_cast<std::uintptr_t>(frame),reinterpret_cast<std::uintptr_t>(result),
        [&](){FrontendOriginal(receiver,frame,result);});
    frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;
    if(frontendLaunch->CurrentState()==cfgmenu::launch::State::Submitted){
        if(frontendReport.state!=4&&frontendReport.state<6&&
           (nativeOfflineHooks.Enabled()||frontendArrivalWindow.Start(GetTickCount64())))FrontendState(4);
    }
    else if(outcome==cfgmenu::startroute::Result::Refused)FrontendRefused(1008);
}
void FrontendEntry(cfgmenu::startroute::SlotKind kind,void* receiver,void* frame,void* result,std::uintptr_t callbackPc){
    if(kind==cfgmenu::startroute::SlotKind::Graph){
        Reader r;r.cap=6;std::uintptr_t locals=0;std::int32_t entry=-1;
        if(!r.Read(reinterpret_cast<std::uintptr_t>(frame)+cfgmenu::startdisk::FrameLocals,locals)||!r.Read(locals,entry))return;
        // Construct, Back and all other graph branches remain native. They do
        // not consume the Start admission or retire the counselor transaction.
        if(entry!=cfgmenu::startdisk::StartEntry){FrontendOriginal(receiver,frame,result);return;}
    }
#ifdef CFG35_MENU_RETENTION_TEST
    // Staged candidate validates the existing UI only. Even an accidental
    // Start click must not enter unadapted native birth/restart code.
#ifdef CFG35_NATIVE_STARTUP_ENTRY_TEST
    // ONLY the ordinary Jason route continues STOCK startup. The dedicated
    // counselor/mod route stays disabled and cannot use the unadapted path.
    if(!frontendCounselorClaim)FrontendOriginal(receiver,frame,result);
    return;
#elif !defined(CFG35_NATIVE_COUNSELOR_STARTUP_TEST)
    return;
#else
    // J34: human-Jason owns this receiver only when its own native menu is
    // current (or its retiring widget is still suppressing reentry). Never
    // route an unrelated callback into a counselor start or double-forward it.
    if(!frontendCounselorClaim){
        const auto address=reinterpret_cast<std::uintptr_t>(receiver);
        const bool owned=FrontendHumanJasonOwns(address);
        cfgmenu::jasonstart::Dispatch(false,owned,
            [&](){return FrontendHumanJasonStart(kind,receiver,frame,result);},
            [&](){FrontendOriginal(receiver,frame,result);});
        return;
    }
#endif
#endif
    if(!frontendCounselorClaim){if(!FrontendHumanJasonStart(kind,receiver,frame,result))FrontendOriginal(receiver,frame,result);return;}
    // Apply ownership/warmup hold BEFORE choosing the wrapper/fast-graph
    // boundary: a fresh widget must not forward through either alias.
    if(!frontendLaunch)return;
    const bool ownedReady=reinterpret_cast<std::uintptr_t>(receiver)==frontendLaunch->Context().widget&&
        (frontendReport.state==3||frontendReport.state==4);
    cfgmenu::startroute::ClaimedNativeEntryBoundary(frontendLaunch->StartBinding(),kind,ownedReady,[&](){FrontendHandle(kind,receiver,frame,result,callbackPc);return cfgmenu::startroute::Result::Consumed;},
        [&](){FrontendOriginal(receiver,frame,result);});
}
void __fastcall FrontendStart(void* receiver,void* frame,void* result){FrontendEntry(cfgmenu::startroute::SlotKind::Wrapper,receiver,frame,result,reinterpret_cast<std::uintptr_t>(_ReturnAddress()));}
void __fastcall FrontendGraph(void* receiver,void* frame,void* result){FrontendEntry(cfgmenu::startroute::SlotKind::Graph,receiver,frame,result,reinterpret_cast<std::uintptr_t>(_ReturnAddress()));}
bool AdoptFrontendUsing(Reader& sourceReader,const cfgmenu::active::Context& c,bool freshEpoch=false){
    if(c.module!=frontendSource.module||c.engine!=frontendSource.engine||c.viewport!=frontendSource.viewport||
       c.player!=frontendSource.player||c.gameInstance!=frontendSource.gameInstance||c.thread!=observerOwnerThread)return false;
    cfgobserver::Plan next=plan;next.frontend=c;next.engine=c.engine;next.viewport=c.viewport;next.world=c.world;next.mode=c.mode;
    next.controller=c.controller;next.pawn=c.hud;next.player=c.player;next.gameInstance=c.gameInstance;next.localPlayerData=c.players;
    const std::array<std::uintptr_t,4> objects{{c.world,c.mode,c.controller,c.hud}};
    if(!cfgprobe::lifetime::ObserveBatch(sourceReader,c.module+cfgprobe::lifetime::ObjectArrayRva,objects,next.objectArray,next.objects)||
       !cfgprobe::dispatch::Observe(sourceReader,c.module,c.hud,next.dispatch)||!cfgobserver::SanePlan(next)||
       !cfgobserver::ContextUnchanged(sourceReader,next)||!cfgmenu::controllergate::Menu(sourceReader,c.module,c.controller))return false;
    cfgmenu::hud::Binding fresh;if(!cfgmenu::hud::Observe(sourceReader,c,true,fresh)||(!freshEpoch&&fresh.event!=frontendBinding.event))return false;
#ifdef CFG_H1_JASON
    if(!RetireHostedBeforeOffline(c.world))return false;
#endif
    plan=next;frontendBinding=fresh;return true;
}
bool AdoptFrontend(const cfgmenu::active::Context& c){return AdoptFrontendUsing(frontendReader,c);}
#include "FrontendPickerRuntime.hpp"
#include "NativeHumanJasonFrontend.hpp"
void FrontendEntryWidgetOpportunity(void*,void*,bool,bool)noexcept;
#include "FrontendEpochRuntime.hpp"
void FrontendBridgeAfter(void* object,void* function,bool wasNested,bool completed,bool entryOpportunity=false,bool hudOpportunity=false){
    using namespace cfgmenu;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(!rows::NativeRetentionScope::Current(plan.module))return;
#endif
    if(wasNested||!completed||GetCurrentThreadId()!=observerOwnerThread||
       (!entryOpportunity&&!hudOpportunity&&reinterpret_cast<std::uintptr_t>(function)!=frontendBinding.event)||
       ((frontendReport.state<1||frontendReport.state>3)&&(frontendReport.state<8||frontendReport.state>10)))return;
    // Publication/preparation still use real owner opportunities. Once the
    // route is installed, however, only the bound recurring HUD event can
    // arm labels. Auxiliary no-ops must not steal its shared25ms slot.
    if(frontendReport.state==8&&frontendEntryLabels&&frontendPickerCoordinator&&
       frontendPickerCoordinator->PublicationState()==callbackslots::State::Installed&&
       !frontend::MayClaimSetupPacing(frontendEntryLabels->Report().state,entryOpportunity||hudOpportunity))return;
    const auto now=GetTickCount64();if(!frontendBridgeStarted)frontendBridgeStarted=now;
    // User-paced selection has no wall-clock deadline. Publication and row
    // construction retain their separate bounded work budgets.
    if(frontendReport.state>=3&&now<frontendBridgeSampleAt)return;frontendBridgeSampleAt=now+25;
    frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=32768;
    if(frontendReport.state==9){
        mapcontinuation::NativeHost<Reader> host(frontendReader,*frontendContinuation);
        frontendContinuationInstall.Step(host,now);
        frontendReport.continuationCells=frontendContinuationInstall.Published();
        const auto state=frontendContinuationInstall.CurrentState();
        if(state==continuation::State::Ready)FrontendState(10);
        else if(state!=continuation::State::Installing&&state!=continuation::State::Restoring)FrontendRefused(1009);
        frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;return;
    }
    active::Context current;
    if(!active::Observe(frontendReader,frontendSource.module,current)||
        reinterpret_cast<std::uintptr_t>(object)!=(entryOpportunity?current.widget:current.hud))return;
    if(current.engine!=frontendSource.engine||current.viewport!=frontendSource.viewport||current.player!=frontendSource.player||
       current.gameInstance!=frontendSource.gameInstance||current.thread!=observerOwnerThread){FrontendRefused(1010);return;}
    if(frontendReport.state>=1&&frontendReport.state<=3){
        if(current==plan.frontend)return;
        FrontendState(10);
    }
    if(frontendReport.state==10&&current.type==frontendSource.type&&frontendPickerCoordinator){
        if(!frontendPickerCoordinator->ReturnFromSelection(current)){
            frontendReport.routeFailure=12;FrontendRefused(1011);return;
        }
        frontendCounselorClaim=false;
        frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;
        frontendReport.acceptedChoiceCommitted=0;FrontendState(8);
    }
    if(frontendReport.state==8){
        cfgnames::Observation<Reader> names(frontendReader,current.module);cfgnames::Text name;
        if(!names.Object(current.type,name))return;
        if(name.View()==u"OfflinePlayMenuWidget_C"){
            // A cosmetics/customization pop can bypass the old counselor's Back
            // executor. Its retained route must not block a real Offline Play
            // return indefinitely. The policy proves the positive same-world
            // entry top; PrepareFrontendEntryReturn then revalidates all owned
            // callbacks/source graphs and fresh children before advertising it.
            if(frontendPickerCoordinator&&frontendPickerCoordinator->ReturnState()==entryreturn::State::Ready&&
               frontendPickerCoordinator->RouteState()!=counselorroute::State::EntryReady&&
               !(frontendPickerCoordinator->RouteState()==counselorroute::State::AwaitCounselor&&current.widget==frontendSource.widget)){
                if(nativeOfflineStartup.Status().stage!=offlinestartup::Stage::Empty||frontendAcceptedCounselorCommitted||
                   !frontendPickerCoordinator->ReturnFromSelection(current)){
                    frontendReport.routeFailure=12;FrontendRefused(1012);return;
                }
                frontendCounselorClaim=false;frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;
                frontendReport.acceptedChoiceCommitted=0;
            }
            if(frontendPickerCoordinator&&frontendPickerCoordinator->ReturnState()==entryreturn::State::AwaitEntry){
                if(!PrepareFrontendEntryReturn(current)){frontendPickerCoordinator->Retire();frontendReport.routeFailure=10;FrontendRefused(1013);}
                SyncPickerReport();return;
            }
            if(!(current==frontendSource)){
                // An actual new same-world native instance needs a new lease,
                // not the gameplay-only epoch reset (startup is already Empty).
                (void)FrontendInstanceReentry(frontendReader,current);return;
            }
            if(!frontendEntryResources){FrontendRefused(1014);return;}
            if(!frontendEntryPrepared){
                frontendEntryPrepared=true;unsigned stage=0;
                const bool observed=entry::Observe(frontendReader,*frontendEntryResources,frontendEntryBinding,&stage);
                frontendReport.entryReads=frontendReader.reads;
                frontendReport.entryMaximumReads=(std::max)(frontendReport.entryMaximumReads,frontendReader.reads);
                frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;
                if(!observed){frontendReport.entryFailure=100+stage;FrontendRefused(1015);return;}
                frontendEngine=std::make_unique<rows::NativeDispatch>(current.module,current.thread,true);
                frontendText=std::make_unique<reflection::RetainedMenuText>();
                if(!PrepareFrontendPicker()){frontendReport.routeFailure=1;FrontendRefused(1016);return;}
                frontendEntryLabels=std::make_unique<FrontendEntryLabels>(frontendReader,*frontendEngine,*frontendText,frontendEntryBinding,frontendBinding,true);
                frontendReport.entrySetupStarted=now;frontendReport.entrySetupElapsedMs=0;frontendReport.entrySetupCallbacks=0;
                frontendReport.entryReads=frontendReader.reads;
                frontendReport.entryMaximumReads=(std::max)(frontendReport.entryMaximumReads,frontendReader.reads);
                frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;
                return; // A separate native callback owns the first label stage.
            }
            if(!frontendPickerCoordinator){FrontendRefused(1017);return;}
            if(frontendPickerCoordinator->PublicationState()!=callbackslots::State::Installed){
                FrontendPickerHost host;frontendPickerCoordinator->Tick(host,now);SyncPickerReport();
                const auto published=frontendPickerCoordinator->PublicationState();
                if(published==callbackslots::State::Refused||published==callbackslots::State::Indeterminate){frontendReport.routeFailure=5;FrontendRefused(1018);}
                return; // Labels do not advertise a half-published route.
            }
            if(!frontendEntryLabels||frontendEntryLabels->Ready())return;
            ++frontendReport.entrySetupCallbacks;
            if(frontendReport.entrySetupStarted&&now>=frontendReport.entrySetupStarted)
                frontendReport.entrySetupElapsedMs=now-frontendReport.entrySetupStarted;
            if(hudOpportunity)frontendEntryLabels->HudOpportunity(reinterpret_cast<std::uintptr_t>(object),false,true,now);
            else if(entryOpportunity)frontendEntryLabels->Opportunity(reinterpret_cast<std::uintptr_t>(object),false,true,now);
            else frontendEntryLabels->After(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),false,true,now);
            frontendReport.entryStage=frontendEntryLabels->Stage();
            frontendReport.entryLabelFailure=frontendEntryLabels->Failure();
            frontendReport.entryCurrentFailure=frontendEntryLabels->CurrentFailure();
            frontendReport.entryOrderState=frontendEntryLabels->OrderState();frontendReport.entryOrderFailure=frontendEntryLabels->OrderFailure();
            frontendReport.entryOrderCaptureStage=frontendEntryLabels->OrderCaptureStage();
            frontendReport.menuWarmDetail=frontendEntryLabels->OrderWarmDiagnostic();
            frontendReport.hudSamples=frontendEntryLabels->Report().samples;frontendReport.hudPhases=frontendEntryLabels->Report().phases;
            frontendReport.entryReads=frontendReader.reads;
            frontendReport.entryMaximumReads=(std::max)(frontendReport.entryMaximumReads,frontendReader.reads);
            if(frontendEntryLabels->Refused()){frontendReport.entryFailure=200+frontendReport.entryStage;FrontendRefused(1019);}
            else if(frontendEntryLabels->Ready()&&frontendPickerCoordinator->ReturnState()==entryreturn::State::Relabeling){
                FrontendEntryReturnHost host;
                if(!frontendPickerCoordinator->CompleteReturn(host,frontendEntryReturnTicket)){
                    frontendPickerCoordinator->Retire();frontendReport.routeFailure=10;FrontendRefused(1020);}
                SyncPickerReport();
            }
            return;
        }
        if(!frontendPickerCoordinator||!frontendEntryLabels||!frontendEntryLabels->Ready())return;
        FrontendPickerHost host;
        if(current.type==frontendPickerSources->counselor){
            if(frontendPickerCoordinator->RouteState()==counselorroute::State::AwaitCounselor)
                frontendPickerCoordinator->ObserveCounselorHud(host,current);
            frontendReport.counselorHudArrival=static_cast<unsigned>(frontendPickerCoordinator->CounselorHudState());
            SyncPickerReport();return;
        }
        if(current.type!=frontendEntryResources->map)return;
        if(!cfgmenu::maparrival::ShouldArrive(frontendCounselorClaim))return;
        if(!frontendPickerCoordinator->Arrive(host,current)){frontendReport.routeFailure=7;FrontendRefused(1021);return;}
        frontendLastPickerArrival=current;
        SyncPickerReport();FrontendState(10); // Native map already points at its native settings class.
    }else if(current.type==frontendEntryResources->map){
        if(!frontendCounselorClaim)return;
        if(current==frontendLastPickerArrival)return;
        FrontendPickerHost host;
        if(!frontendPickerCoordinator||!frontendPickerCoordinator->Arrive(host,current)){
            frontendReport.routeFailure=7;FrontendRefused(1022);return;
        }
        frontendLastPickerArrival=current;SyncPickerReport();
    }else if(current.type==frontendSettingsClass){
        if(!frontendCounselorClaim)return;
        std::uintptr_t pending=0;cfgprobe::lifetime::SlotObservation identity;
        if(!frontendPickerCoordinator){frontendReport.arrivalFailure=23;FrontendRefused(1023);return;}
        FrontendPickerHost host;
        if(!frontendPickerCoordinator->Arrive(host,current)||
           !mapcontinuation::LoadedSettings(frontendReader,current,frontendSettingsClass,identity,frontendEntryResources.get())||!(identity==frontendSettingsIdentity)||
           !frontendReader.Read(current.mode+selectiondisk::PendingOfflineSettingsWidget,pending)||pending!=frontendSettingsClass||
           !AdoptFrontend(current)){frontendReport.arrivalFailure=23;FrontendRefused(1024);return;}
        SyncPickerReport();
        if(frontendLaunch){
            if(frontendLaunch->StartBinding().start.field.object)frontendSharedStartBinding=frontendLaunch->StartBinding();
            unsigned empty=static_cast<unsigned>(frontendCachedLaunches.size());
            for(unsigned i=0;i<frontendCachedLaunches.size();++i)if(!frontendCachedLaunches[i]){empty=i;break;}
            if(empty==frontendCachedLaunches.size()){frontendReport.arrivalFailure=24;FrontendRefused(1025);return;}
            frontendCachedLaunches[empty]=std::move(frontendLaunch);
        }
        for(auto& cached:frontendCachedLaunches){
            if(!cached||!reentry::SameWidget(cached->Context(),current))continue;
            frontendLaunch=std::move(cached);
            if(!frontendAcceptedCounselorCommitted||!frontendLaunch->Resume(frontendBinding,frontendAcceptedCounselor.path)){
                frontendReport.arrivalFailure=25;FrontendRefused(1026);return;
            }
            frontendAttempted=true;FrontendState(frontendLaunch->CurrentState()==launch::State::Ready?3:2);return;
        }
        frontendAttempted=false;
        FrontendState(1); // Real native continuation arrived; add our owned setup rows next.
    }
    frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;
}
void FrontendAfter(void* object,void* function,bool wasNested,bool completed){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(!cfgmenu::rows::NativeRetentionScope::Current(plan.module))return;
#endif
    using namespace cfgmenu;
#if CFG_H20_MENU && !CFG_H21_LOBBY_DISCOVERY
    if(frontendCounselorClaim&&frontendLaunch&&!wasNested&&completed&&GetCurrentThreadId()==plan.threadId&&
       reinterpret_cast<std::uintptr_t>(object)==frontendBinding.context.hud&&reinterpret_cast<std::uintptr_t>(function)==frontendBinding.event&&
       frontendLaunch->CurrentState()==launch::State::Ready){
        static std::uint64_t nextOnlineChoice=0;const auto now=GetTickCount64();
        if(now>=nextOnlineChoice){nextOnlineChoice=now+100;frontendReader.reads=0;frontendReader.cap=32768;
            (void)frontendLaunch->StoreOnlineChoices();}
    }
#endif
    // Unrelated callbacks, gameplay and an already ready view do zero work.
    if(!frontendCounselorClaim||wasNested||GetCurrentThreadId()!=plan.threadId||reinterpret_cast<std::uintptr_t>(object)!=frontendBinding.context.hud||
        reinterpret_cast<std::uintptr_t>(function)!=frontendBinding.event||frontendReport.state>=3)return;
    if(!completed){FrontendRefused(1027);return;}
    frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=32768;
    ++frontendReport.ownerCallbacks;
    if(!frontendAttempted){
        frontendAttempted=true; // No repeated cold reflection/construction after refusal.
        if(!hud::Current(frontendReader,frontendBinding,GetCurrentThreadId())){FrontendRefused(1028);return;}
        HMODULE resident=nullptr;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&FrontendStart),&resident)){FrontendRefused(1029);return;}
        if(!frontendEngine||!frontendText||!frontendEntryResources){FrontendRefused(1030);return;}
        if(!frontendAcceptedCounselorCommitted||!softpath::Plain(frontendAcceptedCounselor.path)){frontendReport.routeFailure=11;FrontendRefused(1031);return;}
        frontendLaunch=std::make_unique<FrontendLaunch>(frontendReader,*frontendEngine,*frontendText);
        if(!frontendLaunch->UseNativeOffline(&PrepareNativeOfflineStart)){FrontendRefused(1032);return;}
        std::array<rows::Spec,15> menu{};unsigned menuCount=6;menu[0]=settingsoptions::CounselorSkill();auto& count=menu[1];
        count.id=RowId::Count;count.title=u"Counselors";count.count=cfglimits::MaximumCounselors;count.selected=6;
        count.reuseNativeCount=true;count.configureNativeCount=true;
        for(unsigned i=0;i<count.count;++i){frontendCountLabels[i]=i==9?u"10":std::u16string(1,static_cast<char16_t>(u'1'+i));count.labels[i]=frontendCountLabels[i];}
        Capabilities caps;caps.automaticJason=true;caps.verified[static_cast<unsigned>(RowId::Count)]=true;
        settingsoptions::EnableDifficulty(caps);menu[2]=settingsoptions::JasonDifficulty();
        settingsoptions::EnableDuration(caps);settingsoptions::EnableSkill(caps);menu[3]=settingsoptions::MatchLength();
        settingsoptions::EnableExpandedLoadout(caps);menu[4]=settingsoptions::StartingWeapon();menu[5]=settingsoptions::ExpandedStartingInventory();
        const auto addPractice=[&](bool enabled,RowId id,std::u16string_view title){if(enabled){
            caps.verified[static_cast<unsigned>(id)]=true;menu[menuCount++]=settingsoptions::Practice(id,title);
        }};
        addPractice(CFG_J215_PAMELA_MUTE,RowId::PamelaMute,u"Mute Pamela (keep sweater voice)");
        addPractice(CFG_J215_GRAB_MUTE,RowId::GrabMute,u"Mute Distant Bot Grab Sounds");
        addPractice(CFG_J215_MINIMAL_HUD,RowId::MinimalHud,u"Minimal HUD (keep objectives)");
        addPractice(!CFG_J216_FINISHING&&CFG_J215_HARD_MOVEMENT,RowId::HardMovement,u"Hard AI Jason Movement (2x)");
        addPractice(!CFG_J216_FINISHING&&CFG_J215_BOT_SWEATER,RowId::BotSweater,u"Bot Sweater Starts Convergence");
#if CFG_H20_MENU && !CFG_H21_LOBBY_DISCOVERY
        for(const auto& spec:hostedsettings::Specs(cfghostedoptions::Current().values)){
            caps.verified[static_cast<unsigned>(spec.id)]=true;menu[menuCount++]=spec;
        }
#endif
        if(!frontendLaunch->Open(plan.frontend,caps,cfglimits::MaximumCounselors,menu.data(),menuCount,1,resident,
            reinterpret_cast<std::uintptr_t>(&FrontendStart),reinterpret_cast<std::uintptr_t>(&FrontendGraph),true,&frontendBinding,
            launch::Origin::AcceptedCounselorPicker,&frontendEntryResources->pendingMode,&frontendAcceptedCounselor.path,
            frontendSharedStartBinding?&*frontendSharedStartBinding:nullptr)){
            frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;FrontendRefused(1033);return;}
        FrontendState(2);
    }
    frontendReport.setupStage=frontendLaunch->PreparationStage();
    frontendLaunch->After(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),
        plan.threadId,false,true,GetTickCount64());
    frontendReport.reads=frontendReader.reads;frontendReport.error=frontendReader.error;
    frontendReport.maximumSetupReads=(std::max)(frontendReport.maximumSetupReads,frontendReader.reads);
    frontendReport.launchPreparationFailure=frontendLaunch->PreparationFailure();
    const auto& selectionDiagnostic=frontendLaunch->SelectionDiagnostic();
    frontendReport.launchSelectionPhase=selectionDiagnostic.phase;
    frontendReport.launchSelectionMapFailure=selectionDiagnostic.mapFailure;
    frontendReport.jasonPathStage=selectionDiagnostic.jason.stage;frontendReport.jasonPathLength=selectionDiagnostic.jason.length;
    frontendReport.selectedJasonRoute=selectionDiagnostic.jasonRoute;frontendReport.selectedJasonClassSerial=selectionDiagnostic.selectedClassSerial;
    frontendReport.jasonPathMatches=selectionDiagnostic.jason.matches;frontendReport.jasonPathRosterCount=selectionDiagnostic.jason.rosterCount;
    frontendReport.selectedJasonPath=selectionDiagnostic.jason.selected;frontendReport.jasonRosterPaths=selectionDiagnostic.jason.roster;
    frontendReport.jasonRosterPathLengths=selectionDiagnostic.jason.rosterLengths;
    frontendReport.launchSelectionProfileStage={};
    if(selectionDiagnostic.profile.stage)for(unsigned i=0;selectionDiagnostic.profile.stage[i]&&i+1<frontendReport.launchSelectionProfileStage.size();++i)
        frontendReport.launchSelectionProfileStage[i]=selectionDiagnostic.profile.stage[i];
    frontendReport.warmStage=frontendLaunch->WarmStage();frontendReport.rowViewState=frontendLaunch->RowViewState();
    frontendReport.rowPrepareStage=frontendLaunch->RowPrepareStage();
    frontendReport.hudSamples=frontendLaunch->HudReport().samples;frontendReport.hudPhases=frontendLaunch->HudReport().phases;
    if(frontendLaunch->CurrentState()==launch::State::Ready)FrontendState(3);
    else if(frontendLaunch->CurrentState()==launch::State::Refused||frontendLaunch->CurrentState()==launch::State::Indeterminate)FrontendRefused(1034);
}
void FrontendArrivalAfter(void* object,bool wasNested,bool completed){
#if defined(CFG35_NATIVE_COUNSELOR_STARTUP_TEST) && !defined(CFG35_PRIMARY_GAMEPLAY_TEST)
    // First restore native counselor startup/possession and borrowed metadata.
    // Full gameplay resource preparation/primary AI is a later integration.
    (void)object;(void)wasNested;(void)completed;
#else
    using namespace cfgmenu;
    if(wasNested||!completed||GetCurrentThreadId()!=observerOwnerThread||!frontendLaunch||
       (frontendReport.state!=4&&frontendReport.state!=6))return;
    const auto now=GetTickCount64();
    if(frontendReport.state==6){
        if(now>=frontendPrepareAt+5000){frontendReport.arrivalFailure=9;FrontendRefused(1035);return;}
        if(now<frontendPrepareAt||reinterpret_cast<std::uintptr_t>(object)!=frontendArrival.plan.pawn)return;
        // Adopt only the genuinely native, acknowledged human. No unpossess,
        // SetActiveCharacterClass, RestartPlayer, input/HUD repair or mode swap.
        plan=frontendArrival.plan;
        Reader r;r.cap=512;
        const auto currentArrival=[&](){
#ifdef CFG_RETAINED_PRIMARY_JASON
            cfgobserver::Plan refreshed{};
            if(!cfgprimary::RefreshKnownObjectStorage(r,plan,refreshed))return false;
            plan=refreshed;return true;
#else
            return cfgobserver::ContextUnchanged(r,plan);
#endif
        };
        if(!currentArrival()||!PrepareSpawnResources()){
            frontendReport.resourceStep=spawnRuntime.report.preparationStep;
            frontendReport.arrivalFailure=10;FrontendRefused(1036);return;
        }
        frontendReport.resourceStep=spawnRuntime.report.preparationStep;
        const auto& configuration=frontendArrival.configuration;
        if(!cfgbundle::MatchesConfiguration(configuration,spawnRuntime.report.identity,spawnAssets.primary,cfgbundle::MaximumBots)||
           !currentArrival()||
           !(configuration.selectedClass.Valid()?
                cfgselectedjason::Current(r,plan.module,plan.objectArray,spawnAssets.killerBase,configuration.selectedClass):
                cfgprimary::SameEntry(r,plan.mode,spawnAssets.primary,configuration.jasonCatalogueIndex))||
           InterlockedCompareExchange(&bundleConfigurationState,1,0)!=0){
            frontendReport.arrivalFailure=11;FrontendRefused(1037);return;
        }
        bundleTrapFamily=cfgtrap::startup::MapFamilyForArrival(frontendArrival.actualPackage.View(),frontendArrival.plan.world,plan.world);
        bundleTrapMapPackage=frontendArrival.actualPackage;
        bundleTrapMapWorld=frontendArrival.plan.world;
        bundleConfiguration=configuration;MemoryBarrier();InterlockedExchange(&bundleConfigurationState,2);
        cfgspawn::SpawnRequest request;request.identity=configuration.identity;
        if(!spawnRuntime.Submit(request)){frontendReport.arrivalFailure=12;FrontendRefused(1038);return;}
        frontendReport.spawnQueued=1;frontendGameplayReady=true;FrontendState(7);return;
    }
    if(frontendReport.state!=4)return;
    if(nativeOfflineStartup.Status().stage!=cfgmenu::offlinestartup::Stage::Ready)return;
    if(!frontendNativeArrivalStarted){
        frontendNativeArrivalStarted=true;
        if(!frontendArrivalWindow.Start(now)){frontendReport.arrivalFailure=9;FrontendRefused(1039);return;}
    }
    const auto& c=frontendBinding.context;
    if(frontendArrivalWindow.Due(now)){
    frontendReport.arrivalChecks=frontendArrivalWindow.checks;
    Reader r;r.cap=32;std::uintptr_t world=0,controller=0,pawn=0,mode=0,state=0,tree=0;
    // Small fixed-chain readiness sample only while ONE launch is pending.
    // Full typed/serial/class/path validation below remains authoritative.
    const bool ready=cfgobserver::Same(r,c.module+cfgprobe::EngineGlobalCandidate,c.engine)&&
        cfgobserver::Same(r,c.engine+cfgprobe::EngineGameViewport,c.viewport)&&
        r.Read(c.viewport+cfgprobe::ViewportWorld,world)&&cfgprobe::ObjectPointer(world)&&world!=c.world&&
        cfgobserver::Same(r,world+cfgprobe::WorldNetDriver,std::uintptr_t{})&&
        cfgobserver::Same(r,world+cfgprobe::WorldGameInstance,c.gameInstance)&&
        r.Read(world+cfgprobe::WorldGameMode,mode)&&cfgprobe::ObjectPointer(mode)&&
        r.Read(world+cfgprobe::WorldGameState,state)&&cfgprobe::ObjectPointer(state)&&
        r.Read(mode+cfgprimary::CounselorTree,tree)&&cfgprobe::ObjectPointer(tree)&&
        r.Read(c.player+cfgprobe::PlayerController,controller)&&cfgprobe::ObjectPointer(controller)&&
        r.Read(controller+cfgprobe::ControllerPawn,pawn)&&cfgprobe::ObjectPointer(pawn)&&
        cfgobserver::Same(r,controller+cfgprobe::ControllerAcknowledgedPawn,pawn)&&
        cfgobserver::Same(r,pawn+0x3A0,controller)&&cfgprimary::LookupVetoClearForObservation(r,c.module);
    frontendReport.reads=r.reads;frontendReport.error=r.error;
    frontendArrivalWindow.Sample(now,world,pawn,ready);
    }else if(frontendArrivalWindow.Finished()){frontendReport.arrivalFailure=9;FrontendRefused(1040);return;}
    if(!frontendArrivalWindow.ReadyFor(reinterpret_cast<std::uintptr_t>(object)))return;
    frontendArrivalWindow.Finish(); // Never repeat cold reflection after refusal.
    freshgame::Source source{c.module,c.engine,c.viewport,c.player,c.gameInstance,c.world,c.thread,true};
    frontendReader.reads=0;frontendReader.error=0;frontendReader.cap=observedarrival::ReadLimit;
    observedarrival::Failure failure;freshgame::Trace trace;arrival::Trace handoffTrace;
    const auto accepted=observedarrival::Consume(frontendReader,frontendLaunch->PendingGameplay(),source,observerOwnerThread,true,
        frontendArrival,&frontendReport.arrivalReads,&failure,&trace,&handoffTrace);
        frontendReport.handoffExpectedCounselor=handoffTrace.expected;frontendReport.handoffActualCounselor=handoffTrace.actual;
        frontendReport.handoffBirthStep=static_cast<unsigned>(handoffTrace.birth.step);frontendReport.handoffBirthReads=handoffTrace.birth.reads;
        frontendReport.handoffRelationship=handoffTrace.relationship;
        frontendReport.freshStep=static_cast<unsigned>(trace.step);frontendReport.birthStep=static_cast<unsigned>(trace.human.step);
        frontendReport.defaultIndex=trace.defaultIndex;frontendReport.birthFlags=trace.human.flags;
        frontendReport.birthReads=trace.human.reads;frontendReport.nativeActiveVerified=trace.human.nativeActiveVerified?1u:0u;
        frontendReport.activeNonempty=trace.human.activeNonempty?1u:0u;frontendReport.birthPawn=trace.human.pawn;
        frontendReport.birthState=trace.human.state;frontendReport.birthType=trace.human.type;frontendReport.birthSpawned=trace.human.spawned;
    if(!accepted){
        if(failure==observedarrival::Failure::Handoff)frontendReport.handoffFailure=static_cast<unsigned>(frontendLaunch->PendingGameplay().LastFailure());
        frontendReport.error=frontendReader.error;frontendReport.arrivalFailure=static_cast<unsigned>(failure);FrontendRefused(1041);return;
    }
    frontendReport.world=frontendArrival.plan.world;frontendReport.pawn=frontendArrival.plan.pawn;
    frontendPrepareAt=now+100;FrontendState(6); // Separate resource preparation from arrival reads.
#endif
}
void FrontendPrebirthSafe(void* object,void* function,void* parameters)noexcept{
    // PendingGameplay must already contain a guarded Start submission. No work
    // outside this one pending counselor launch, including ordinary Jason mode.
    if(!frontendLaunch||GetCurrentThreadId()!=observerOwnerThread||
       nativeOfflineHooks.Enabled()||
       frontendLaunch->PendingGameplay().State()!=cfgmenu::HandoffState::Armed||
       frontendPrebirth.CurrentState()!=cfgmenu::prebirth::State::Empty)return;
    try{
        const auto& c=frontendBinding.context;
        cfgmenu::freshgame::Source source{c.module,c.engine,c.viewport,c.player,c.gameInstance,c.world,c.thread};
        Reader reader;reader.cap=32768;
        cfgmenu::promotion::WindowsCalls calls(c.module,c.thread,true);
        cfgmenu::starting::Event event{c.module,reinterpret_cast<std::uintptr_t>(object),
            reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),GetCurrentThreadId(),true};
        frontendPrebirth.Before(reader,calls,event,source,frontendLaunch->PendingGameplay());
        const auto& d=frontendPrebirth.Report();
        frontendReport.prebirthState=static_cast<unsigned>(frontendPrebirth.CurrentState());
        frontendReport.prebirthStage=d.stage;frontendReport.prebirthOwnerStage=d.ownerStage;frontendReport.prebirthEvents=d.events;
        frontendReport.prebirthAttempts=d.attempts;frontendReport.prebirthReturns=d.returns;
        if(d.events){frontendReport.prebirthReads=reader.reads;frontendReport.prebirthError=reader.error;}
        frontendReport.prebirthWorld=d.world;frontendReport.prebirthPlayerState=d.playerState;
    }catch(...){frontendReport.prebirthState=static_cast<unsigned>(cfgmenu::prebirth::State::Indeterminate);frontendReport.prebirthStage=7;}
}
void FrontendEntryWidgetOpportunity(void* object,void* function,bool nested,bool completed)noexcept{
    if(frontendReport.state!=8||nested||!completed||!function||GetCurrentThreadId()!=observerOwnerThread||
       !object||GetTickCount64()<frontendBridgeSampleAt)return;
    if(reinterpret_cast<std::uintptr_t>(object)==frontendSource.widget){
        if(frontendEntryLabels&&frontendEntryLabels->Ready())return;
    }else{
        // Cheap typed candidate only. The full actual active stack, positive
        // identities and eight published callback proofs still decide re-entry.
        Reader hint;hint.cap=2;
        if(!cfgobserver::Same(hint,reinterpret_cast<std::uintptr_t>(object)+0x10,frontendSource.type))return;
    }
    try{FrontendBridgeAfter(object,function,false,true,true);}catch(...){FrontendRefused(1042);}
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
// Post-match objective HUD script restore (live PID2472: 16 owned patches stayed
// PendingRestore, BundleEpochRetirable() false, Offline Play menu never re-adopted).
// Runs ONLY on the CURRENT menu's freshly observed HUD completed ReceiveDrawHUD,
// inside FrontendMenuRootProvider's proved GC registry window, one patch per
// callback. Placed before the epoch check, which returns early while blocked.
// Event-seeded: FrontendEpochAfter binds the rebuilt menu HUD ONCE when it is
// blocked by this restore (state lives in FrontendEpochRuntime.hpp); every
// other callback costs two pointer compares.
void FrontendObjectiveRetirementAfter(void* object,void* function,bool nested,bool completed,std::uintptr_t returnPc)noexcept{
    const auto receiver=reinterpret_cast<std::uintptr_t>(object),event=reinterpret_cast<std::uintptr_t>(function);
    if(!frontendRetireEvent||receiver!=frontendRetireBinding.context.hud||event!=frontendRetireEvent)return;
    if(nested||!completed||!bundleObjectiveRetiring||!bundleObjectives.PendingRestore()||
       GetCurrentThreadId()!=observerOwnerThread||GetCurrentThreadId()!=bundleObjectiveThread){
        if(!bundleObjectives.PendingRestore()){frontendRetireBinding={};frontendRetireEvent=0;}return;}
    const auto now=GetTickCount64();if(now<frontendRetireStepAt)return;frontendRetireStepAt=now+100;
    FrontendMenuRootProvider provider(frontendRetireBinding,receiver,event,returnPc,nested,completed);
    if(!provider.Admitted()){frontendRetireBinding={};frontendRetireEvent=0;return;} // stale: epoch re-binds
    // Same odd/even publication and read/time accounting as BundleObjectiveLifecycle;
    // the menu proof reads are charged to this one phase reader.
    const bool publication=(bundleReport.generation&1u)==0;
    if(publication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    LARGE_INTEGER started{},finished{},frequency{};QueryPerformanceCounter(&started);QueryPerformanceFrequency(&frequency);
    Reader reader;reader.cap=cfgbundle::ReadCap;
    (void)provider.MetadataWindow<cfgobjective::Budget<Reader>>(reader,[&](Reader& phase)noexcept{
        struct Host{Reader& reader;
            bool Current(){return cfgprimary::LookupSafe(reader,bundleObjectiveModule);}
            bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){return cfgobjective::CompareWrite(at,expected,value);}
        }host{phase};
        bundleObjectives.RetireOne(phase,bundleObjectiveModule,host);return true;
    });
    BundleObjectiveReport();bundleReport.reads+=reader.reads;
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);bundleReport.error=reader.error;
    QueryPerformanceCounter(&finished);
    if(frequency.QuadPart>0)bundleReport.maxMicroseconds=(std::max)(bundleReport.maxMicroseconds,
        static_cast<std::uint64_t>((finished.QuadPart-started.QuadPart)*1000000/frequency.QuadPart));
    if(publication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#endif
void FrontendAfterSafe(void* object,void* function,bool wasNested,bool completed,std::uintptr_t returnPc=0)noexcept{
#ifndef CFG_RETAINED_GAMEPLAY_BUNDLE
    (void)returnPc;
#endif
    try{FrontendPickerListAfter(object,wasNested,completed);FrontendEscapeAfter(wasNested,completed);
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && CFG_J36_CHALLENGE_WALLS
        FrontendWallsAfter(object,function,wasNested,completed,returnPc);
#endif
        FrontendBoundaryAfter(wasNested,completed);
        if(!CFG_J22_CHALLENGES_RETENTION)FrontendChallengeReturnOpportunity(wasNested,completed);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        FrontendObjectiveRetirementAfter(object,function,wasNested,completed,returnPc);
        FrontendWorldRetirementAfter(object,function,wasNested,completed,returnPc);
#endif
        if(FrontendEpochAfter(object,function,wasNested,completed))return;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        FrontendMenuRootProvider rootProvider(frontendBinding,reinterpret_cast<std::uintptr_t>(object),
            reinterpret_cast<std::uintptr_t>(function),returnPc,wasNested,completed);
        cfgmenu::rows::NativeRetentionScope nativeScope(plan.module,rootProvider.Hooks(),rootProvider.Admitted());
        cfgmenu::startupretain::Scope challengePageScope(plan.module,cfgmenu::startupretain::Role::Frontend,
            rootProvider.ChallengePageHooks(),CFG_J22_CHALLENGES_RETENTION&&rootProvider.Admitted());
#endif
        if(CFG_J22_CHALLENGES_RETENTION)FrontendChallengeReturnOpportunity(wasNested,completed);
        FrontendChallengeQueuedAfter();
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(rootProvider.CleanupOnly())return;
#endif
        if(FrontendHumanJasonAfter(object,function,wasNested,completed,returnPc))return;
        const bool additionalHud=frontendReport.state==8&&object&&function&&
            (!frontendEntryLabels||!frontendEntryLabels->Ready())&&
            reinterpret_cast<std::uintptr_t>(object)==frontendSource.hud&&
            reinterpret_cast<std::uintptr_t>(function)!=frontendBinding.event;
        FrontendBridgeAfter(object,function,wasNested,completed,false,additionalHud);FrontendArrivalAfter(object,wasNested,completed);FrontendAfter(object,function,wasNested,completed);}
    catch(...){FrontendRefused(1043);} // Own C++ allocation failure never escapes into game dispatch.
}
