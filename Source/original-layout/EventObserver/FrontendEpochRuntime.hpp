// Included after picker/human-Jason hosts. New actual native OfflinePlay top
// only; installed metadata/dispatch and all old row/text backing remain resident.
struct FrontendEpochStorage {
 std::unique_ptr<cfgmenu::rows::NativeDispatch> engine;
 std::unique_ptr<cfgmenu::reflection::RetainedMenuText> text;
 std::unique_ptr<FrontendLaunch> launch;
 std::array<std::unique_ptr<FrontendLaunch>,64> cached;
 std::unique_ptr<FrontendHumanJasonMenu> human;
 std::array<std::unique_ptr<FrontendHumanJasonMenu>,cfgmenu::FrontendEpochCapacity> humanCached;
 std::unique_ptr<FrontendEntryLabels> labels;
 std::unique_ptr<cfgmenu::entrysettings::Resources> entryResources;
 std::unique_ptr<cfgmenu::pickersource::Resources> pickerSources;
 std::unique_ptr<cfgmenu::pickerprepare::Prepared> picker;
 std::unique_ptr<cfgmenu::entryroute::Binding> entryRoute;
 std::unique_ptr<PickerCoordinator> coordinator;
};
// Published native callbacks/text buffers stay resident. Move only owning
// pointers on growth, so their targets do not move. No arbitrary match-count
// ceiling; allocation failure still takes the existing fail-closed path.
std::vector<std::unique_ptr<FrontendEpochStorage>> frontendOldEpochs;
void ReserveFrontendEpochArchive(){
 // Allocate BEFORE moving any published backing out of its live owner. A
 // failing vector growth must never destroy text/callback state still in use.
 if(frontendOldEpochs.size()<frontendOldEpochs.capacity())return;
 const auto required=frontendOldEpochs.size()+1;
 const auto grown=frontendOldEpochs.capacity()<=frontendOldEpochs.max_size()/2?
     frontendOldEpochs.capacity()*2:required;
 frontendOldEpochs.reserve((std::max)(required,grown));
}
std::uint64_t frontendEpochNext=0;
unsigned frontendEpochAttempts=0;
bool FrontendFreshClassEpoch(Reader& r,const cfgmenu::active::Context& actual,bool unclaimedTravel=false){
 using namespace cfgmenu;
 // Native travel unloads/recreates Blueprint classes. This is a complete new
 // metadata lease, NOT equality relaxation on old class/function pointers.
 // Old published callbacks and every backing buffer stay resident; foreign
 // callbacks forward to the pinned original through the active guard.
 if(actual.array.count<2||frontendReport.epochs==UINT32_MAX)return false;
 cfgnames::Observation<Reader> names(r,actual.module);cfgnames::Text label;
 if(!names.Object(actual.type,label)||label.View()!=u"OfflinePlayMenuWidget_C")return false;
 if(!AdoptFrontendUsing(r,actual,true)){frontendReport.epochFailure=9;return true;}
 ReserveFrontendEpochArchive();auto storage=std::make_unique<FrontendEpochStorage>();
 storage->engine=std::move(frontendEngine);storage->text=std::move(frontendText);storage->launch=std::move(frontendLaunch);
 storage->cached=std::move(frontendCachedLaunches);storage->human=std::move(frontendHumanJason);storage->humanCached=std::move(frontendHumanJasonCached);
 storage->labels=std::move(frontendEntryLabels);storage->entryResources=std::move(frontendEntryResources);
 storage->pickerSources=std::move(frontendPickerSources);storage->picker=std::move(frontendCounselorPicker);
 storage->entryRoute=std::move(frontendEntryRoute);storage->coordinator=std::move(frontendPickerCoordinator);
 frontendOldEpochs.push_back(std::move(storage));
 frontendPickerPrepared=false;frontendEntryPrepared=false;
 frontendSharedStartBinding.reset();
 // Same cold authored graph/native ABI/descriptor/positive owner checks as
 // initial install. No loading/injection/auth patch, no arbitrary class lookup.
 const auto& prior=*frontendOldEpochs.back();
 const auto expected=prior.coordinator?prior.coordinator->ExpectedExecutors().counselor:pickerprepare::Executors{};
 frontendInheritedPicker=nullptr;
 if(!PrepareFrontendResources(r,prior.picker.get(),prior.coordinator?&expected:nullptr)){
  frontendReport.epochFailure=10;frontendReport.epochReads=r.reads;FrontendRefused(1044);return true;}
 if(prior.picker&&prior.coordinator&&frontendCounselorPicker->type==prior.picker->type)
  frontendInheritedPicker=prior.coordinator.get();
 frontendSource=actual;
 if(unclaimedTravel){
  if(nativeOfflineStartup.Status().stage!=offlinestartup::Stage::Empty){frontendReport.epochFailure=13;FrontendRefused(1045);return true;}
 }else if(!nativeOfflineStartup.ResetEpoch()){frontendReport.epochFailure=13;FrontendRefused(1046);return true;}
 frontendCounselorClaim=false;frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;frontendConstructorState={};
 frontendLastPickerArrival={};frontendEntryReturnTicket={};frontendAttempted=false;frontendArrival={};frontendArrivalWindow={};frontendPrebirth={};
 frontendGameplayReady=false;frontendNativeArrivalStarted=false;frontendHumanJasonNext=0;frontendHumanJasonRetiringWidget=0;
 frontendHumanJasonTop={};frontendHumanJasonTopNative=false;frontendPrepareAt=frontendBridgeStarted=frontendBridgeSampleAt=0;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
 ObservePreviousWorldAtMenu(actual);
#endif
 BundleResetEpoch();
 const auto epochs=frontendReport.epochs+1;frontendReport={};frontendReport.epochs=epochs;frontendReport.epochStage=2;
 SyncWorldRootReport();
 frontendReport.epochReads=r.reads;frontendEpochAttempts=0;frontendEntryPrepared=false;
 // The ordinary state8 HUD pipeline owns entry capture, coordinator prepare,
 // paced eight-cell publication and labels on later fresh native callbacks.
 // Do not combine a full cold resource capture with a second32768-read phase.
 SyncPickerReport();SyncNativeOfflineReport();FrontendState(8);return true;
}
struct FrontendEpochHost {
 Reader& r;const cfgmenu::pickerprepare::Prepared& picker;const cfgmenu::entryroute::Binding& route;
 bool EpochCurrent(const cfgmenu::active::Context& actual,const cfgmenu::pickercoordinator::Expected& expected)noexcept{
  try{
   const bool valid=cfgmenu::active::Current(r,actual,observerOwnerThread)&&
    cfgmenu::controllergate::Menu(r,actual.module,actual.controller)&&
    cfgmenu::pickerprepare::Current(r,actual,picker,observerOwnerThread,expected.counselor)&&
    cfgmenu::entryroute::Current(r,route,observerOwnerThread,expected.entry);
   frontendReport.epochReads=r.reads;return valid;
  }catch(...){return false;}
 }
};
// Back -> MainMenu -> OfflinePlay is not gameplay travel. The native startup
// has already been reset to Empty, so ResetEpoch/BundleResetEpoch must NOT be
// used here. Reprove all resident callbacks and replace only instance leases.
bool FrontendInstanceReentry(Reader& r,const cfgmenu::active::Context& actual){
 using namespace cfgmenu;
 ++frontendReport.instanceReentryChecks;frontendReport.instanceReentryFailure=1;
 // Back/reopen may also replace just the Blueprint class without world travel.
 // Exact surviving infrastructure plus the native Empty/EntryReady ledger
 // admits a full fresh class capture, never a same-address ownership shortcut.
 if(actual.type!=frontendSource.type&&reentry::Infrastructure(frontendSource,actual)&&
    epochservice::UnclaimedTravel(frontendReport.state,frontendCounselorClaim,frontendAcceptedCounselorCommitted,
       nativeOfflineStartup.Status().stage==offlinestartup::Stage::Empty,
       frontendPickerCoordinator&&frontendPickerCoordinator->RouteState()==counselorroute::State::EntryReady,
       frontendPickerCoordinator&&frontendPickerCoordinator->PublicationState()==callbackslots::State::Installed,
       BundleEpochRetirable()))return FrontendFreshClassEpoch(r,actual,true);
 if(frontendReport.state!=8||frontendCounselorClaim||frontendAcceptedCounselorCommitted||
    nativeOfflineStartup.Status().stage!=offlinestartup::Stage::Empty||
    !frontendPickerCoordinator||!frontendEntryRoute||!frontendCounselorPicker||!frontendEntryResources||!frontendPickerSources||
    frontendPickerCoordinator->RouteState()!=counselorroute::State::EntryReady||
    frontendPickerCoordinator->PublicationState()!=callbackslots::State::Installed||
    frontendReport.epochs==UINT32_MAX||
    actual==frontendSource||actual.type!=frontendSource.type||actual.world!=frontendSource.world||
    actual.controller!=frontendSource.controller||actual.hud!=frontendSource.hud||
    actual.engine!=frontendSource.engine||actual.viewport!=frontendSource.viewport||actual.player!=frontendSource.player||
    actual.gameInstance!=frontendSource.gameInstance||actual.thread!=observerOwnerThread)return false;
 auto resources=std::make_unique<entrysettings::Resources>(*frontendEntryResources);resources->source=actual;
 auto picker=std::make_unique<pickerprepare::Prepared>(*frontendCounselorPicker);
 pickersource::Resources source;frontendReport.instanceReentryFailure=2;
 if(!epochsource::Rebind(r,actual,picker->source,source))return false;
 picker->source=source;auto saved=*frontendEntryRoute;saved.entry.source=resources.get();entryroute::Binding rebound;
 const auto expected=frontendPickerCoordinator->ExpectedExecutors();frontendReport.instanceReentryFailure=3;
 if(!entryroute::Rebind(r,saved,actual,expected.entry,rebound)||
    !pickerprepare::Current(r,actual,*picker,observerOwnerThread,expected.counselor)||!AdoptFrontendUsing(r,actual))return false;
 FrontendEpochHost host{r,*picker,rebound};frontendReport.instanceReentryFailure=4;
 // Allocate resident storage before committing the coordinator's new instance.
 ReserveFrontendEpochArchive();auto storage=std::make_unique<FrontendEpochStorage>();
 if(!frontendPickerCoordinator->Rearm(host,actual))return false;
 storage->engine=std::move(frontendEngine);storage->text=std::move(frontendText);storage->launch=std::move(frontendLaunch);
 storage->cached=std::move(frontendCachedLaunches);storage->human=std::move(frontendHumanJason);
 storage->humanCached=std::move(frontendHumanJasonCached);storage->labels=std::move(frontendEntryLabels);
 frontendOldEpochs.push_back(std::move(storage));++frontendReport.epochs;
 *frontendEntryResources=*resources;*frontendCounselorPicker=*picker;*frontendPickerSources=picker->source;
 rebound.entry.source=frontendEntryResources.get();*frontendEntryRoute=rebound;frontendEntryBinding=rebound.entry;frontendSource=actual;
 frontendLastPickerArrival={};frontendEntryReturnTicket={};frontendConstructorState={};
 frontendHumanJasonNext=0;frontendHumanJasonRetiringWidget=0;frontendHumanJasonTop={};frontendHumanJasonTopNative=false;
 frontendEngine=std::make_unique<rows::NativeDispatch>(actual.module,actual.thread,true);
 frontendText=std::make_unique<reflection::RetainedMenuText>();
 frontendEntryLabels=std::make_unique<FrontendEntryLabels>(frontendReader,*frontendEngine,*frontendText,frontendEntryBinding,frontendBinding,true);
 frontendReport.entrySetupStarted=GetTickCount64();frontendReport.entrySetupElapsedMs=0;frontendReport.entrySetupCallbacks=0;
 frontendEntryPrepared=true;++frontendReport.instanceReentries;frontendReport.instanceReentryFailure=0;
 SyncPickerReport();return true;
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
// Post-match objective restore binding (see FrontendObjectiveRetirementAfter).
cfgmenu::hud::Binding frontendRetireBinding{};std::uint64_t frontendRetireStepAt=0;
std::uintptr_t frontendRetireEvent=0;
void FrontendObjectiveRetirementBind(const cfgmenu::hud::Binding& fresh)noexcept{
 frontendRetireBinding=fresh;frontendRetireEvent=fresh.event;
}
#endif
bool FrontendEpochAfter(void* object,void* function,bool nested,bool completed){
 using namespace cfgmenu;
 if(!epochservice::Candidate(GetCurrentThreadId(),observerOwnerThread,frontendReport.state,nested,completed))return false;
 const bool unclaimed=frontendReport.state==8;
 if(unclaimed&&!epochservice::UnclaimedTravel(frontendReport.state,frontendCounselorClaim,frontendAcceptedCounselorCommitted,
    nativeOfflineStartup.Status().stage==offlinestartup::Stage::Empty,
    frontendPickerCoordinator&&frontendPickerCoordinator->RouteState()==counselorroute::State::EntryReady,
    frontendPickerCoordinator&&frontendPickerCoordinator->PublicationState()==callbackslots::State::Installed,
    BundleEpochRetirable()))return false;
 // Donor OfflineSetup has an explicit500ms frontend-return fallback even when
 // the old menu event was missed. The old HUD/event is NOT the lifetime owner
 // of a rebuilt frontend. Use an already-completed native Actor callback on
 // the owner thread only as a service opportunity; actual viewport/native Menu
 // controller/positive menu identities below remain the mutation authority.
 // No worker-thread native calls and no gameplay-world reflection polling.
 (void)object;(void)function;
 const auto now=GetTickCount64();if(!epochservice::Due(now,frontendEpochNext))return !unclaimed;
 // No full active-menu/reflection walk while gameplay world still owns viewport.
 Reader hint;hint.cap=12;std::uintptr_t world=0;
 if(!hint.Read(frontendSource.viewport+cfgprobe::ViewportWorld,world)||!world)return !unclaimed;
 const auto expectedWorld=unclaimed?frontendSource.world:plan.world;
 const auto expectedIdentity=unclaimed?frontendSource.identities[0]:plan.objects[0];
 if(world==expectedWorld){cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
  if(!cfgprobe::lifetime::ReadArrayHeader(hint,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
     !cfgprobe::lifetime::ReadSlot(hint,registry,world,slot)||slot==expectedIdentity)return !unclaimed;}
 // Cheap current native Menu classification before the full menu walk. Stock
 // Challenge gameplay must not trigger frontend reflection/epoch work.
 if(unclaimed){Reader typeHint;typeHint.cap=256;std::uintptr_t mode=0;
  if(!typeHint.Read(world+cfgprobe::WorldGameMode,mode)||!cfgprobe::ObjectPointer(mode))return false;
  std::uintptr_t native=0,ctor=0;
  if(!typeHint.Read(frontendSource.module+execdisk::FrontendClass,native)||
     !typeHint.Read(frontendSource.module+execdisk::FrontendConstructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor)||
     !cfgspawn::ObjectIs(typeHint,mode,native))return false;
 }
 Reader r;r.cap=32768;active::Context actual;
 frontendReport.epochStage=1;
 if(!active::Observe(r,frontendSource.module,actual)){
  frontendReport.epochFailure=7;frontendReport.epochReads=r.reads;return true;}
 frontendReport.epochActualType=actual.type;frontendReport.epochExpectedType=frontendSource.type;
 frontendReport.epochActualWidget=actual.widget;frontendReport.epochActualDepth=static_cast<unsigned>(actual.array.count);
 frontendReport.epochExpectedDepth=static_cast<unsigned>(frontendSource.array.count);
 frontendReport.epochActualTypeName={};
 {cfgnames::Observation<Reader> names(r,actual.module);cfgnames::Text label;
  if(names.Object(actual.type,label))for(unsigned i=0;i<label.View().size()&&i+1<frontendReport.epochActualTypeName.size();++i)
   frontendReport.epochActualTypeName[i]=label.View()[i]<=127?static_cast<char>(label.View()[i]):'?';}
 // MainMenu (depth 1) is already a valid fresh menu lifetime owner. Retire
 // departed-map roots there; waiting for OfflinePlay leaked the old map.
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
 if(actual.engine==frontendSource.engine&&actual.viewport==frontendSource.viewport&&actual.player==frontendSource.player&&
    actual.gameInstance==frontendSource.gameInstance&&actual.thread==observerOwnerThread&&controllergate::Menu(r,actual.module,actual.controller))
  FrontendWorldRetirementBind(r,actual);
#endif
 if(actual.array.count<2){
  frontendReport.epochFailure=8;frontendReport.epochReads=r.reads;return true;}
 frontendReport.epochStage=1;frontendReport.epochReads=r.reads;
 if(actual.engine!=frontendSource.engine||actual.viewport!=frontendSource.viewport||actual.player!=frontendSource.player||
    actual.gameInstance!=frontendSource.gameInstance||actual.thread!=observerOwnerThread||!controllergate::Menu(r,actual.module,actual.controller)){
  frontendReport.epochFailure=1;return true;}
 if((!unclaimed&&!nativeOfflineStartup.CanResetEpoch())||!BundleEpochRetirable()){
  // Diagnostic only (bounded): which half blocks re-adopting the Offline menu.
  static unsigned blockedTraces=0;
  if(blockedTraces<16){++blockedTraces;
   cfgclosedtrace::Mark("frontend-epoch-blocked",actual.widget,
    (static_cast<std::uint64_t>(!unclaimed&&!nativeOfflineStartup.CanResetEpoch())<<32)|
    (static_cast<std::uint64_t>(nativeOfflineStartup.Status().stage)<<16)|BundleEpochRetireMask());}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
  // Event-seeded objective restore: bind the CURRENT menu HUD once, here, where
  // the menu was just observed; restoration itself runs on that HUD's completed
  // ReceiveDrawHUD inside the proved GC window (FrontendObjectiveRetirementAfter).
  if((BundleEpochRetireMask()&16u)&&!frontendRetireEvent&&controllergate::Menu(r,actual.module,actual.controller)){
   hud::Binding fresh;if(hud::Observe(r,actual,true,fresh))FrontendObjectiveRetirementBind(fresh);}
#endif
  frontendReport.epochFailure=2;return true;}
 if(actual.type!=frontendSource.type){
  cfgnames::Observation<Reader> names(r,actual.module);cfgnames::Text label;
  if(!names.Object(actual.type,label)||label.View()!=u"OfflinePlayMenuWidget_C"){
   frontendReport.epochFailure=8;frontendReport.epochReads=r.reads;return true;}
 }
 if(unclaimed){
  cfgnames::Observation<Reader> names(r,actual.module);cfgnames::Text label;
  if(!names.Object(actual.type,label)||label.View()!=u"OfflinePlayMenuWidget_C")return false;
  if(!epochservice::AdmitAttempt(true,frontendEpochAttempts))return true;
  if(actual.type!=frontendSource.type){(void)FrontendFreshClassEpoch(r,actual,true);return true;}
  // If classes survived travel, use the existing eight-cell ledger/rearm
  // transaction below rather than attempting to publish those same cells again.
 }
 if(!unclaimed&&!epochservice::AdmitAttempt(true,frontendEpochAttempts))return true;
 if(actual.type!=frontendSource.type){
  if(!FrontendFreshClassEpoch(r,actual)){frontendReport.epochFailure=8;frontendReport.epochReads=r.reads;}
  return true;
 }
 if(!frontendPickerCoordinator||!frontendEntryRoute||!frontendCounselorPicker||!frontendEntryResources||!frontendPickerSources||
    frontendReport.epochs==UINT32_MAX){
  frontendReport.epochFailure=3;return true;}
 ReserveFrontendEpochArchive();auto storage=std::make_unique<FrontendEpochStorage>();
 auto resources=std::make_unique<entrysettings::Resources>(*frontendEntryResources);resources->source=actual;
 auto picker=std::make_unique<pickerprepare::Prepared>(*frontendCounselorPicker);
 // Re-observe the new stack's native retained parent record. Its class and
 // all captured graphs/serials must remain identical; widget may be rebuilt.
 pickersource::Resources pickerSource;
 if(!epochsource::Rebind(r,actual,picker->source,pickerSource)){
  frontendReport.epochFailure=4;return true;}
 picker->source=pickerSource;
 auto saved=*frontendEntryRoute;saved.entry.source=resources.get();entryroute::Binding rebound;
 const auto expected=frontendPickerCoordinator->ExpectedExecutors();
 if(!entryroute::Rebind(r,saved,actual,expected.entry,rebound)||
    !pickerprepare::Current(r,actual,*picker,observerOwnerThread,expected.counselor)||!AdoptFrontendUsing(r,actual)){
  frontendReport.epochFailure=5;frontendReport.epochReads=r.reads;return true;}
 FrontendEpochHost host{r,*picker,rebound};
 if(!frontendPickerCoordinator->Rearm(host,actual)||(!unclaimed&&!nativeOfflineStartup.ResetEpoch())){
  frontendReport.epochFailure=6;FrontendRefused(1047);return true;}
 // All old instance resources stay alive; do not free text buffers still
 // referenced by native widgets. No Submitted launch is ever resumed.
 storage->engine=std::move(frontendEngine);storage->text=std::move(frontendText);storage->launch=std::move(frontendLaunch);
 storage->cached=std::move(frontendCachedLaunches);storage->human=std::move(frontendHumanJason);
 storage->humanCached=std::move(frontendHumanJasonCached);storage->labels=std::move(frontendEntryLabels);
 frontendOldEpochs.push_back(std::move(storage));
 *frontendEntryResources=*resources;*frontendCounselorPicker=*picker;*frontendPickerSources=picker->source;
 rebound.entry.source=frontendEntryResources.get();*frontendEntryRoute=rebound;frontendEntryBinding=rebound.entry;frontendSource=actual;
 frontendCounselorClaim=false;frontendAcceptedCounselor={};frontendAcceptedCounselorCommitted=false;frontendConstructorState={};
 frontendLastPickerArrival={};frontendEntryReturnTicket={};frontendAttempted=false;frontendArrival={};frontendArrivalWindow={};frontendPrebirth={};
 frontendGameplayReady=false;frontendNativeArrivalStarted=false;frontendHumanJasonNext=0;frontendHumanJasonRetiringWidget=0;frontendHumanJasonTop={};frontendHumanJasonTopNative=false;
 frontendPrepareAt=frontendBridgeStarted=frontendBridgeSampleAt=0;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
 ObservePreviousWorldAtMenu(actual);
#endif
 BundleResetEpoch();
 const auto epochs=frontendReport.epochs+1;frontendReport={};frontendReport.epochs=epochs;frontendReport.epochStage=2;frontendReport.epochReads=r.reads;frontendEpochAttempts=0;
 SyncWorldRootReport();
 frontendEntryPrepared=true;
 frontendEngine=std::make_unique<rows::NativeDispatch>(actual.module,actual.thread,true);
 frontendText=std::make_unique<reflection::RetainedMenuText>();
 frontendEntryLabels=std::make_unique<FrontendEntryLabels>(frontendReader,*frontendEngine,*frontendText,frontendEntryBinding,frontendBinding,false);
 frontendReport.entrySetupStarted=GetTickCount64();frontendReport.entrySetupElapsedMs=0;frontendReport.entrySetupCallbacks=0;
 SyncPickerReport();SyncNativeOfflineReport();FrontendState(8);return true;
}
void FrontendEpochWidgetOpportunity(void* object,void* function,bool nested,bool completed)noexcept{
 FrontendChallengeReturnOpportunity(nested,completed);
 if(frontendReport.state==8){
  try{if(FrontendEpochAfter(object,function,nested,completed))return;
      FrontendEntryWidgetOpportunity(object,function,nested,completed);}
  catch(...){FrontendRefused(1048);}return;
 }
 if(frontendReport.state!=7)return; // no gameplay or first-menu widget work
 try{(void)FrontendEpochAfter(object,function,nested,completed);}
 catch(...){FrontendRefused(1049);}
}
