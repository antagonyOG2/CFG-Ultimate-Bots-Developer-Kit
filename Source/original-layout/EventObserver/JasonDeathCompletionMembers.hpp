// Included inside BundleAccess. This path intentionally does not require the
// dying pawn to remain possessed; native death may detach its AI controller.
    bool DeathScope(bool& dying){
        dying=false;std::uint8_t flags=0,again=0;
        if(GetCurrentThreadId()!=plan.threadId||!originalActorDispatcher||!cfgprimary::LookupSafe(r_,plan.module)||
            !cfgprimary::RetainedOfflineWorld(r_,plan,context)||
            !cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn)||
            !cfgspawn::ObjectIs(r_,bundleJason.pawn.object,spawnAssets.killerBase)||
            !cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)||
            !cfgjason::SameSlot(r_,context.objectArray,spawnAssets.primary.stateSlot)||
            !cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKiller,bundleJason.pawn.object)||
            !r_.Read(bundleJason.pawn.object+cfgescape::disk::DyingFlags,flags)||
            !r_.Read(bundleJason.pawn.object+cfgescape::disk::DyingFlags,again)||flags!=again)return false;
        dying=(flags&cfgescape::disk::DyingMask)!=0;return true;
    }
    //0 normal gameplay;1 native death owns this callback;2 native ending sent;
    //3 retained identity lost. Never equate an interaction/montage with death.
    unsigned DeathCompletion(std::uint64_t now){
        if(!bundleKill.policy.interactionAttempted&&!bundleDeathEnd.confirmedAt)return 0;
        if(now<bundleDeathEnd.next)return bundleDeathEnd.confirmedAt?1u:0u;
        bundleDeathEnd.next=now+250;bool dying=false;
        if(!DeathScope(dying))return bundleDeathEnd.confirmedAt?3u:0u;
        bundleDeathEnd.Confirm(bundleKill.policy,dying,now);
        if(!bundleDeathEnd.confirmedAt)return 0;
        if(bundleDeathEnd.dispatched)return 2;
        if(!dying)return 1; // confirmed death never resumes AI on ambiguous native state
        if(TravelCleanup(now))return 1; // restore only owned helper brains before native results/cinematic work
        if(bundleDefeatedOutroCursor<2){
            auto& property=bundleDefeatedOutro[bundleDefeatedOutroCursor];
            const auto object=bundleDefeatedOutroCursor?spawnAssets.gameState:plan.mode;
            if(!property.Warm(r_,plan.module,object)){
                if(now<bundleDeathEnd.due)return 1;
                property.stage=2; // optional metadata must not delay native results indefinitely
            }
            // Optional reflected skip rule only. Exact schema + native bool ABI
            // proof is required before touching its containing byte.
            std::uintptr_t address=0;std::uint8_t mask=0,value=0;
            if(property.Value(r_,plan.module,object,address,mask,value)&&DeathScope(dying)&&dying){
                const auto desired=static_cast<std::uint8_t>(value|mask);
                if(value==desired||static_cast<std::uint8_t>(_InterlockedCompareExchange8(
                    reinterpret_cast<volatile char*>(address),static_cast<char>(desired),static_cast<char>(value)))==value)
                    ++bundleDefeatedOutroFlags;
            }
            ++bundleDefeatedOutroCursor;return 1;
        }
        if(!bundleDeathEnd.Ready(now))return 1;
        std::uintptr_t function=0,owner=0;cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(r_,plan.module);
        if(!FeatureFunction(cfgescape::disk::EndMatch,function,owner)||!cfgspawn::ObjectIs(r_,plan.mode,owner)||
            !cfgobserver::Same(r_,function+0xB0,plan.module+cfgescape::disk::EndMatchExecutor)||
            !cfgbundle::Code(r_,plan.module,cfgescape::disk::EndMatchExecutor,cfgescape::disk::EndMatchCode)||
            !r_.Read(plan.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text))return 1;
        if(text.View()!=u"InProgress")return 2; // native lifecycle already owns results
        if(!Abandon()||!DeathScope(dying)||!dying||!cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
            !bundleDeathEnd.Mark(now))return 1;
        bundleCompletion.dispatched=true;++bundleReport.endMatchAttempts;
        if(Invoke(plan.mode,function,nullptr)){++bundleReport.endMatchReturns;bundlePostMatchWatch.Begin(now);}
        return 2; // never issue EndMatch twice, including an ambiguous native return
    }
