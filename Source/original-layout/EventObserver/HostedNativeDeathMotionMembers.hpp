#ifdef CFG_H1_JASON
    bool HostedNativeDeathOwnsMotion(const cfgjasontick::Owner& owner,std::uint64_t now){
        if(!owner.adoption.valid||!HostedAdoptionOwnsDriver()||
           !(owner.actor.pawn==hostedAdoptionReceipt.pawn)||
           !(owner.actor.killerController==hostedAdoptionReceipt.controller))return false;
        if(hostedNativeDeathMotion)return true;
        if(now<hostedNativeDeathMotionCheckAt)return false;
        hostedNativeDeathMotionCheckAt=now+250;
        cfgobserver::Plan fresh{};
        if(!cfgprimary::LookupSafe(r_,plan.module)||!cfgprimary::RetainedWorld(r_,plan,fresh)||
           !cfgjason::SameSlot(r_,fresh.objectArray,owner.actor.pawn)||
           !cfgspawn::ObjectIs(r_,owner.actor.pawn.object,spawnAssets.killerBase)||
           !cfgjason::SameSlot(r_,fresh.objectArray,spawnAssets.primary.stateSlot)||
           !cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)||
           !cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKiller,owner.actor.pawn.object)||
           !cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKillerState,owner.adoption.controllerState))return false;
        const auto flagsAt=owner.actor.pawn.object+cfgescape::disk::DyingFlags;
        std::uint8_t flags=0;
        if(!r_.Read(flagsAt,flags)||!(flags&cfgescape::disk::DyingMask)||
           !cfgescape::Same(r_,flagsAt,flags))return false;
        // The native dying flag owns motion even when the mod did not start the
        // kill. Stop only the current, still-linked AI controller, once.
        hostedNativeDeathMotion=true;
        cfgdonorhunt::NativeOutcome outcome{};
        const bool stopped=DonorNativeStopMovement(owner,outcome);
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();
        bundleMoveAt=0;bundleTargetLockUntil=0;
        cfgclosedtrace::Mark(stopped?"hosted-native-death-motion-stopped":"hosted-native-death-motion-suppressed",
            owner.actor.pawn.object,static_cast<unsigned>(outcome));
        return true;
    }
#endif
