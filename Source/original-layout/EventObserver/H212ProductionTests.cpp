#define H212_PRODUCTION_EXTRA 1
unsigned H212ProductionExtra();
#include "H210ProductionTests.cpp"
#define CFG_MULTI_MATCH_ACCESS_ONLY
#include "MultiMatchLifecycleProductionTests.cpp"
#undef CFG_MULTI_MATCH_ACCESS_ONLY
unsigned H212ProductionExtra(){
 unsigned n=0,bad=0;auto check=[&](bool okay){++n;if(!okay){++bad;std::printf("FAIL H212 startup %u\n",n);}};
 {
  const auto oldWorld=plan.world;plan.world=0xA22200;ResetBundleManagerRetry();
  unsigned observations=0,services=0,stops=0;bool valid=false;
  const auto admitted=[&](std::uint64_t now){const auto result=BundleManagerGate(now,true,[&](){++observations;return valid;});
   if(result==BundleManagerGateResult::Ready)++services;else if(result==BundleManagerGateResult::Failed)++stops;return result;};
  check(admitted(100)==BundleManagerGateResult::Deferred&&observations==1&&!services&&!stops&&bundleManagerRetry.failures==1);
  check(admitted(349)==BundleManagerGateResult::Deferred&&observations==1&&!services&&!stops);
  valid=true;check(admitted(350)==BundleManagerGateResult::Ready&&observations==2&&services==1&&!stops);
  valid=false;observations=0;std::uint64_t now=1000;
  for(unsigned i=0;i<BundleManagerRetryLimit;++i){check(admitted(now)==BundleManagerGateResult::Deferred);now+=BundleManagerRetryDelayMs;}
  check(bundleManagerRetry.exhausted&&observations==BundleManagerRetryLimit&&admitted(now)==BundleManagerGateResult::Deferred&&
   observations==BundleManagerRetryLimit&&services==1&&!stops);
  valid=true;check(admitted(bundleManagerRetry.retryAt)==BundleManagerGateResult::Ready&&
   observations==BundleManagerRetryLimit+1&&services==2&&!stops);
  plan.world=oldWorld;ResetBundleManagerRetry();
  std::printf("H222 manager gate recovery: transient defers 250ms, valid next callback resumes, persistent invalid backs off after %u attempts then resumes on a fresh valid proof; no stop.\n",BundleManagerRetryLimit);
 }
 const auto native=gcRegistryReport.calls;const auto open=primaryBootstrapRootsOpen;
 check(!cfghostedrestore::EndPhase(u"InProgress")&&!cfghostedrestore::EndPhase(u"None")&&!cfghostedrestore::EndPhase(u""));
 check(cfghostedrestore::EndPhase(u"WaitingPostMatch")&&cfghostedrestore::EndPhase(u"LeavingMap"));
 check(BundleEpochRetirable());BundleResetEpoch();primaryBootstrapRootsOpen=false;
 check(!cfgstartuplease::CanRetain(primaryBootstrapRootsOpen,false));
 check(cfgstartuplease::CanRetain(primaryBootstrapRootsOpen,true)); // only the independently admitted CURRENT returned tick
 const bool queued=cfgstartuplease::ReturnedPending(true,false,false,true);
 check(queued&&cfgtick::CompletedWorkCandidate(true,false,true,true,true&&!queued));
 check(!cfgtick::CompletedWorkCandidate(true,true,true,true,true&&!queued));
 check(!cfgtick::CompletedWorkCandidate(true,false,false,true,true&&!queued));
 check(!cfgstartuplease::ReturnedPending(true,false,true,true)&&!cfgstartuplease::ReturnedPending(true,false,false,false));
 check(!cfgstartuplease::ReturnedPending(true,true,false,true)&&!cfgstartuplease::ReturnedPending(false,false,false,true));
 cfgspawn::Identity id{0x10000000,0x20000000,31,32};cfgspawn::SpawnRequest request;request.identity=id;
 check(spawnRuntime.Configure(id,true,0,0x20000000,0)&&spawnRuntime.Submit(request));
 // Native metadata/engine responses are owned fixture seams; the actual spawn
 // policy, configuration/schedule/duration coordinator and READY publication run.
 struct Init {unsigned calls=0;bool okay=true;bool Initialize(){++calls;return okay;}} init;
 check(!BundleInitializeCore(init,1000)&&!init.calls&&bundleReport.initializationStage!=29);
 MultiSpawnAccess spawn;spawn.resolve=true;spawnRuntime.Service(spawn);
 check(spawnRuntime.report.state==static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted)&&spawn.spawnCalls==1);
 bundleConfiguration={};bundleConfiguration.counselorBots=0;bundleConfiguration.identity=id;bundleConfiguration.submission=1;
 bundleConfiguration.jasonCatalogueIndex=0;bundleConfiguration.selectedJason[0]=1;
 check(cfgbundle::ValidConfiguration(bundleConfiguration,cfgbundle::MaximumBots));InterlockedExchange(&bundleConfigurationState,2);
 init.okay=false;check(!BundleInitializeCore(init,1000)&&bundleReport.initializationStage!=29);
 init.okay=true;const bool initialized=BundleInitializeCore(init,1000);check(initialized);if(initialized)BundlePublishInitialized();
 check(bundleReport.initializationStage==29&&bundleReport.state==static_cast<unsigned>(cfgbundle::State::Active));
 check(!primaryBootstrapRootsOpen&&gcRegistryReport.calls==native);
 check(!lifecycleFailures);
 BundleResetEpoch();primaryBootstrapRootsOpen=open;
 std::printf("H212 actual hosted->offline startup replay: %u checks, %u failures; closed bootstrap -> OWN callback lease -> actual spawn policy -> actual bundle coordinator/READY29; engine initialization is a stub, no game/native calls.\n",n,bad);
 return bad;
}

