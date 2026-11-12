// Same startup coordinator in production and the owned hosted->offline replay.
// Access::Initialize remains responsible for every engine/input/root proof.
template<class Access>bool BundleInitializeCore(Access& a,std::uint64_t now){
    return spawnRuntime.report.state==static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted)&&
        BundleInitializeCheck(cfgbundle::InitializationStep::Configuration,InterlockedCompareExchange(&bundleConfigurationState,2,2)==2)&&
        a.Initialize()&&
        BundleInitializeCheck(cfgbundle::InitializationStep::Schedule,bundleSchedule.Start(now,bundleConfiguration.counselorBots,bundleConfiguration.matchSeconds))&&
        BundleInitializeCheck(cfgbundle::InitializationStep::Duration,bundleDuration.Begin(cfgofflinesettings::FromConfiguration(bundleConfiguration),now));
}
void BundlePublishInitialized()noexcept{
    bundleReport.initializationStage=static_cast<unsigned>(cfgbundle::InitializationStep::Ready);
    bundleReport.state=static_cast<unsigned>(cfgbundle::State::Active);
}
