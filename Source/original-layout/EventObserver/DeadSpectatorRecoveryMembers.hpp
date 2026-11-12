// Include inside BundleAccess. No custom pose, camera Tick, spring-arm writes,
// possession changes or repeated RPC. Reuse the accepted native restore path.
    bool RecoverDeadSpectator(std::uint64_t now){
        if(!bundleDeadSpectatorRecovery.Due(now))return false;
        cfgspectator::Snapshot snapshot{};cfgescape::State state{};
        cfgspectator::Identity selected{};
        if(!SpectatorSafety(snapshot)||snapshot.paused||!SpectatorStockView(snapshot)||
           !snapshot.nativeSelection.object||snapshot.nativeSelection.object==bundleJason.pawn.object||
           !SpectatorIdentity(snapshot.nativeSelection.object,selected)||selected!=snapshot.nativeSelection||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,selected.object,bundleCounselorClass,state)||
           !cfgescape::Terminal(state)){
            bundleDeadSpectatorRecovery.Lost();return true;
        }
        if(!bundleDeadSpectatorRecovery.Dead(selected.object,selected.serial,now))return true;
        cfgspawn::PlayerArray list{};
        if(!r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,list)||
           list.data!=bundleSpectatorList.data||list.count!=bundleSpectatorList.count||list.capacity!=bundleSpectatorList.capacity||
           list.count<1||list.count>static_cast<int>(cfgspectator::MaximumCounselors))return true;
        // Only the last observer's known typed roster; no fresh world census.
        for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){
            const auto& target=bundleSpectatorRoster[i];
            if(!target.pawn.object||target.pawn==selected)continue;
            if(!cfgobserver::Same(r_,list.data+8*i,target.pawn.object)||!SpectatorCachedLiving(i))continue;
            cfgescape::State again{};cfgspectator::Identity current{};
            if(!cfgescape::Counselor(r_,context.objectArray,plan.module,selected.object,bundleCounselorClass,again)||again!=state||
               !SpectatorIdentity(selected.object,current)||current!=selected||!SpectatorSafetyLinks()||
               !cfgobserver::Same(r_,plan.controller+cfgspectator::disk::SpectatingPlayer,selected.object))return true;
            bundleDeadSpectatorRecovery.Consume(); // never replay an ambiguous handoff
            RecoverDeadSpectatorHandoff(snapshot,target);return true;
        }
        return true;
    }
    bool RecoverDeadSpectatorHandoff(const cfgspectator::Snapshot& snapshot,const cfgspectator::Target& target){
        const bool returned=RestoreCounselor(snapshot,target);
        DeadSpectatorRecoveryTrace(snapshot.nativeSelection.object,target.pawn.object,returned);return returned;
    }
