// Completed CURRENT menu HUD only. Retire all mod-owned departed-map roots;
// preserve C++ callback backing and unrelated assets through process exit.
struct WorldRootRetirement {
 cfgretain::WorldRetirementStage stage=cfgretain::WorldRetirementStage::None;
 cfgworlddiag::Previous previous{};cfgmenu::hud::Binding binding{};
 std::array<cfgretain::NamedRootOwner,256> names{};unsigned count=0;
 std::uint64_t next=0,afterLoadAt=0;unsigned blocked=0,refusals=0,beforeChecks=0,afterChecks=0,lastObservation=0;
 const char* failureName="";std::uintptr_t failureOwner=0;
 bool notice=false,preloadDone=false,awaitArrival=false;
} worldRootRetirement;
void SyncWorldRootReport()noexcept{
 const auto& state=worldRootRetirement;auto& out=frontendReport;
 out.worldRootRetirementStage=static_cast<unsigned>(state.stage);out.worldRootRefusals=state.refusals;
 out.worldRootBeforeLoadChecks=state.beforeChecks;out.worldRootAfterLoadChecks=state.afterChecks;
 out.worldRootLastObservation=state.lastObservation;out.worldRootFailureOwner=state.failureOwner;
 out.failedStartWorldRecords=j24WorldCounters.records;out.failedStartWorldRecordFailures=j24WorldCounters.failures;
 out.failedStartWorldRetirements=j24WorldCounters.retirements;out.failedStartWorldGeneration=j24WorldCounters.generation;
 out.failedStartRootsAtRecord=j24WorldCounters.rootsAtRecord;
 out.worldRootFailureName={};strncpy_s(out.worldRootFailureName.data(),out.worldRootFailureName.size(),state.failureName,_TRUNCATE);
 const auto& cause=bundleRootDiagnostics;out.rootFirstPoisonSite=cause.firstPoison.site;
 out.rootFirstPoisonClaims=cause.firstPoison.claims;out.rootFirstPoisonFlags=cause.firstPoison.flags;out.rootFirstPoisonPc=cause.firstPoison.pc;
 out.rootFirstPoisonWindowStage=cause.poisonWindowStage;out.rootFirstPoisonWindowGate=cause.poisonWindowGate;
 out.rootFirstPoisonContention=cause.poisonContention?1u:0u;out.rootFirstPoisonUntouched=cause.poisonUntouched?1u:0u;
}
void WorldRootRefuse()noexcept{
 auto& state=worldRootRetirement;state.stage=cfgretain::WorldRetirementStage::Refused;state.refusals=1;
 if(!state.notice){state.notice=true;char tag[128]{};sprintf_s(tag,"world-root-retirement-refused:%s",state.failureName);
  cfgclosedtrace::Mark(tag,state.failureOwner,0);
  constexpr char message[]="Second match blocked: a mod root could not be released safely. Restart the game.\r\n";
  const auto console=GetStdHandle(STD_OUTPUT_HANDLE);DWORD written=0;
  if(console&&console!=INVALID_HANDLE_VALUE)WriteFile(console,message,static_cast<DWORD>(sizeof(message)-1),&written,nullptr);
 }
 SyncWorldRootReport();
}
void WorldRootDiagnostic(bool afterLoad)noexcept{
 auto& state=worldRootRetirement;Reader reader;reader.cap=256;cfgprobe::lifetime::ArrayHeader objects{};
 unsigned observation=3;
 if(cfgprimary::LookupVetoClearForObservation(reader,plan.module)&&
    cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,objects))
  observation=cfgworlddiag::ObserveAtLoad(reader,objects,state.previous,[](unsigned){});
 state.lastObservation=observation;if(afterLoad)++state.afterChecks;else ++state.beforeChecks;
 cfgclosedtrace::Mark(afterLoad?"previous-world-check-after-load":"previous-world-check-before-load",state.previous.world.object,observation);
 cfgclosedtrace::Mark(observation==1?"previous-world-alive":observation==2?"previous-world-not-current":"previous-world-unproven",state.previous.world.object,afterLoad?2u:0u);
 SyncWorldRootReport();
}
void WorldRootRemember(std::uintptr_t address,const char* name,const char* type,unsigned group)noexcept{
 if(!address)return;auto& state=worldRootRetirement;
 for(unsigned i=0;i<state.count;++i)if(state.names[i].address==address)return;
 if(state.count==state.names.size()){state.failureName="owner-name-registry-overflow";WorldRootRefuse();return;}
 state.names[state.count++]={address,name,type,group};
}
void WorldRootNames()noexcept{
 for(unsigned i=0;i<primaryRoots.Count();++i)WorldRootRemember(primaryRoots.Address(i),"primaryRoots","PermanentRootPool",i);
 const auto bank=[](auto& roots,const char* name){for(unsigned i=0;i<roots.Owners();++i)
  WorldRootRemember(roots.Address(i),name,"SynchronizedRootOwner",i);};
 bank(startupRootBank,"startupRootBank");bank(frontendRootBank,"frontendRootBank");bank(bundleRootBank,"bundleRootBank");
#if CFG_H21_LOBBY_ROWS
 bank(hostedLobbyRootBank,"hostedLobbyRootBank");
#endif
#if CFG_J214_INTRO_END_TO_END || CFG_J36_CHALLENGE_WALLS
 bank(frontendChallengeRootBank,"frontendChallengeRootBank");
#endif
#ifdef CFG35_URINATION_NATIVE_PORT
 bank(urinationAssetBank,"urinationAssetBank");
#endif
 WorldRootRemember(reinterpret_cast<std::uintptr_t>(&gcRegistryOwner),"gcRegistryOwner","Owner",0);
}
bool WorldRootInventory(Reader& reader,const cfgprobe::lifetime::ArrayHeader& objects,const char* phase,unsigned& held)noexcept{
 held=0;WorldRootNames();auto& state=worldRootRetirement;
 if(state.stage==cfgretain::WorldRetirementStage::Refused)return false;
 if(cfggcdiagnostic::registeredOwners.overflow.load()){state.failureName="owner-registration-overflow";state.failureOwner=0;return false;}
 std::array<std::uintptr_t,256> owners{};unsigned count=0;cfggcdiagnostic::registeredOwners.Snapshot(owners,count);
 // Every registration by this DLL must have a known owner and retirement path.
 for(unsigned i=0;i<count;++i){bool known=false;for(unsigned j=0;j<state.count;++j)known|=state.names[j].address==owners[i];
  if(!known){state.failureName="UNCLASSIFIED";state.failureOwner=owners[i];cfgclosedtrace::Mark("world-root-UNCLASSIFIED",owners[i],0);return false;}}
 const auto classify=[&](std::uintptr_t object){return cfgworlddiag::ClassifyMapReference(reader,objects,object,state.previous);};
 const auto emit=[&](const cfgretain::NamedRootOwner& owner,std::uintptr_t object,cfgretain::MapReference){
  char tag[160]{};sprintf_s(tag,"world-root-%s:%s[%u]:%s",phase,owner.name,owner.group,owner.type);
  cfgclosedtrace::Mark(tag,owner.address,object);
  if(object){std::uintptr_t type=0;cfgnames::Text label;cfgnames::Observation<Reader> names(reader,plan.module);
   if(reader.Read(object+cfgprobe::ObjectClass,type)&&names.Object(type,label)){
    char text[49]{};unsigned n=0;for(auto c:label.View()){if(n==48)break;text[n++]=c>=32&&c<127?static_cast<char>(c):'?';}
    sprintf_s(tag,"world-root-object:%s",text);cfgclosedtrace::Mark(tag,object,owner.address);}}
 };
 for(unsigned i=0;i<state.count;++i){unsigned references=0;
  state.failureName=state.names[i].name;state.failureOwner=state.names[i].address;
  if(!cfgretain::InspectNamedRootOwner(reader,state.names[i],classify,emit,references))return false;held+=references;}
 return true;
}
bool FrontendPreviousWorldAllowsLoad()noexcept{
 const auto& previous=bundlePreviousWorld;
 if(!previous.world.object){if(!previous.recordAttempted)return true;
  worldRootRetirement.failureName="match-record";worldRootRetirement.failureOwner=0;WorldRootRefuse();return false;}
 const auto& state=worldRootRetirement;
 const bool ready=state.previous.generation==previous.generation&&state.previous.world.object==previous.world.object&&
  state.stage==cfgretain::WorldRetirementStage::Ready;
 if(!ready&&worldRootRetirement.blocked++<8)cfgclosedtrace::Mark("previous-world-load-deferred",previous.world.object,static_cast<unsigned>(state.stage));
 if(ready&&!worldRootRetirement.preloadDone){worldRootRetirement.preloadDone=true;worldRootRetirement.awaitArrival=true;WorldRootDiagnostic(false);}
 if(!ready&&state.stage==cfgretain::WorldRetirementStage::Refused)WorldRootRefuse();
 return ready;
}
void FrontendWorldRetirementBind(Reader& reader,const cfgmenu::active::Context& menu)noexcept{
 const auto& previous=bundlePreviousWorld;auto& state=worldRootRetirement;
 if(!previous.world.object||previous.world.object==menu.world||menu.thread!=observerOwnerThread)return;
 if(state.previous.generation!=previous.generation||state.previous.world.object!=previous.world.object||state.previous.world.index!=previous.world.index){
  state.previous=previous;state.stage=cfgretain::WorldRetirementStage::Armed;state.next=state.afterLoadAt=0;
  state.blocked=state.refusals=state.beforeChecks=state.afterChecks=state.lastObservation=0;
  state.notice=state.preloadDone=state.awaitArrival=false;state.failureOwner=0;state.failureName="";
  SyncWorldRootReport();
  cfgclosedtrace::Mark("world-root-retirement-armed",previous.world.object,menu.world);
 }
 if(state.stage==cfgretain::WorldRetirementStage::Ready||state.stage==cfgretain::WorldRetirementStage::Refused)return;
 cfgmenu::hud::Binding fresh;if(cfgmenu::hud::Observe(reader,menu,true,fresh)){
  state.binding=fresh;
  if((BundleEpochRetireMask()&16u)&&!frontendRetireEvent)FrontendObjectiveRetirementBind(fresh);
 }
}
// Called ONLY at the tail of the completed-current-menu retirement transaction.
// Keep the reopen proof testable without admitting any engine/native call.
template<class Access>bool CompleteWorldRootRetirement(Access& registry,const cfggcprobe::Report& removal,bool inventoryEmpty)noexcept{
 if(removal.result!=cfggcprobe::Result::Passed||removal.stage!=12||!inventoryEmpty||
    primaryRoots.Count()||primaryRoots.Poisoned()||!registry.ProvenSlotWindow()||
    !BundleAccess::ResetRetentionAfterMenuRecovery(registry))return false;
 bundleRootDiagnostics.strandedClaims=0;
 primaryBootstrapRootsOpen=true; // fresh match may acquire its finite bootstrap roots
 return true;
}
bool RetireWorldRootsLocked(GcRegistryAccess& registry,Reader& reader)noexcept{
 auto& state=worldRootRetirement;cfgprobe::lifetime::ArrayHeader objects{};unsigned held=0;
 state.failureName="object-registry";state.failureOwner=0;
 if(!registry.ProvenSlotWindow()||!cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,objects)||
    !WorldRootInventory(reader,objects,"before",held))return false;
 // Complete pre-release names/types/slots have been traced above. Never use
 // cached expected pointers as roots or mutate a foreign native referencer.
 state.failureName="bundleDeferredRetirement";state.failureOwner=0;
 const bool recoverGameplay=!bundleRootBank.Healthy();
 if(!recoverGameplay&&bundleDeferredRetirement.Claims()&&!bundleDeferredRetirement.Retire(registry,bundleRootBank,plan.module))return false;
#ifdef CFG35_URINATION_NATIVE_PORT
 state.failureName="urinationAssetBank";state.failureOwner=urinationAssetBank.Address(0);
 if(!urinationNativeHost.RetireAtMenu(registry))return false;
#endif
 const auto classify=[&](std::uintptr_t object){return cfgworlddiag::ClassifyMapReference(reader,objects,object,state.previous);};
 unsigned released=0,total=0;
 const auto bank=[&](auto& roots,const char* name){state.failureName=name;state.failureOwner=0;
  return roots.RetireMapReferences(registry,classify,released,&state.failureOwner);};
 if(!bank(startupRootBank,"startupRootBank"))return false;total+=released;
 if(!bank(frontendRootBank,"frontendRootBank"))return false;total+=released;
#if CFG_H21_LOBBY_ROWS
 if(hostedLobbyRootBank.Healthy()){if(!bank(hostedLobbyRootBank,"hostedLobbyRootBank"))return false;}
 else{state.failureName="hostedLobbyRootBank-recovery";state.failureOwner=0;
  if(!hostedLobbyRootBank.RecoverMapReferencesAtMenu(registry,classify,released,&state.failureOwner))return false;}
 total+=released;
#endif
#if CFG_J214_INTRO_END_TO_END || CFG_J36_CHALLENGE_WALLS
 if(frontendChallengeRootBank.Healthy()){
  if(!bank(frontendChallengeRootBank,"frontendChallengeRootBank"))return false;
 }else{
  state.failureName="frontendChallengeRootBank-recovery";state.failureOwner=0;
  if(!frontendChallengeRootBank.RecoverMapReferencesAtMenu(registry,classify,released,&state.failureOwner))return false;
 }
 total+=released;
#endif
 if(recoverGameplay){state.failureName="bundleRootBank-recovery";state.failureOwner=0;
  if(!bundleRootBank.RecoverMapReferencesAtMenu(registry,classify,released,&state.failureOwner))return false;
  total+=released;state.failureName="bundleDeferredRetirement-recovered";
  if(!bundleDeferredRetirement.DiscardAfterMenuRecovery(bundleRootBank,plan.module))return false;
  cfgclosedtrace::Mark("world-root-gameplay-bank-recovered",bundleRootBank.RecoveryEpoch(),released);
 }else{if(!bank(bundleRootBank,"bundleRootBank"))return false;total+=released;}
#ifdef CFG35_URINATION_NATIVE_PORT
 if(!bank(urinationAssetBank,"urinationAssetBank"))return false;total+=released;
#endif
 state.failureName="primaryRoots";state.failureOwner=primaryRoots.Address(0);
 cfggcprobe::Report removal;if(!primaryRoots.ReleaseAllLocked(registry,removal)){state.failureOwner=removal.owner;return false;}
 cfgclosedtrace::Mark("world-root-retired",removal.removed,total);
 unsigned remaining=0;const bool inventoryEmpty=WorldRootInventory(reader,objects,"after",remaining)&&remaining==0;
 return CompleteWorldRootRetirement(registry,removal,inventoryEmpty);
}
void FrontendWorldRetirementAfter(void* object,void* function,bool nested,bool completed,std::uintptr_t pc)noexcept{
 auto& state=worldRootRetirement;
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread)return;
 SyncWorldRootReport();
 // Read-only diagnostic, independently veto checked; never gates gameplay or
 // retains a World. Start schedules it; arrival arms the TWO-second delay.
 if(state.stage==cfgretain::WorldRetirementStage::Ready&&state.awaitArrival){Reader hint;hint.cap=32;std::uintptr_t world=0;
  if(cfgprimary::LookupVetoClearForObservation(hint,plan.module)&&hint.Read(state.binding.context.viewport+cfgprobe::ViewportWorld,world)&&
     world&&world!=state.binding.context.world){state.awaitArrival=false;state.afterLoadAt=GetTickCount64()+2000;}}
 if(state.stage==cfgretain::WorldRetirementStage::Ready&&state.afterLoadAt&&GetTickCount64()>=state.afterLoadAt){
  state.afterLoadAt=0;WorldRootDiagnostic(true);}
 if(state.stage!=cfgretain::WorldRetirementStage::Armed)return;
 if(nested||!completed||GetCurrentThreadId()!=observerOwnerThread||reinterpret_cast<std::uintptr_t>(object)!=state.binding.context.hud||
    reinterpret_cast<std::uintptr_t>(function)!=state.binding.event||state.binding.context.world==state.previous.world.object)return;
 const auto now=GetTickCount64();if(now<state.next)return;state.next=now+100;
 if(bundleRootLease||cfgmenu::rows::NativeRetentionScope::Occupied()||cfgmenu::startupretain::Scope::Occupied())return;
 // Restore borrowed scripts and finish every native startup transaction before
 // releasing metadata/bootstrap roots. Pending objective restore uses the same
 // fresh HUD independently, including depth-1 MainMenu.
 if(!BundleEpochRetirable()||(nativeOfflineStartup.Status().stage!=cfgmenu::offlinestartup::Stage::Empty&&
    !nativeOfflineStartup.CanResetEpoch()))return;
 FrontendMenuRootProvider provider(state.binding,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),pc,nested,completed);
 if(!provider.Admitted()){state.binding={};return;}
 if(state.stage==cfgretain::WorldRetirementStage::Armed){Reader read;read.cap=131072;bool entered=false;
  const bool retired=provider.RegistryRetirementWindow([&](GcRegistryAccess& registry)noexcept{entered=true;return RetireWorldRootsLocked(registry,read);});
  if(!entered)return; // contended synchronization window; no mutation attempted
  if(!retired){WorldRootRefuse();return;}
  // Native LoadMap is allowed to perform its OWN collection after all mod
  // roots/memberships are verified gone. World survival is diagnostic only.
  state.stage=cfgretain::WorldRetirementStage::Ready;bundlePreviousWorld.checked=true;
  if(CFG_J24_FAILED_START_RETIRE)j24WorldCounters.Retired(state.previous.generation);
  state.failureName="";state.failureOwner=0;SyncWorldRootReport();
 }
}
