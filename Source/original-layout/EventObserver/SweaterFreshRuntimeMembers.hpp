    bool FreshSweaterRearm(std::uint64_t now){
        bundleReport.sweaterFreshFailure=1;
        bundleReport.sweaterFreshAdmission=0;bundleReport.sweaterFreshAdmissionReads=0;bundleReport.sweaterFreshAdmissionCalls=0;
        const bool initial=bundleSweaterPolicy.stage==cfgsweater::Stage::Discovering&&!bundleSweaterPolicy.lastUse;
        const bool repair=bundleSweaterPolicy.repairingOwned;
        if((initial?bundleSweaterFreshGrantAttempted:
           (bundleSweaterPolicy.stage!=cfgsweater::Stage::Waiting||(!repair&&!bundleSweaterPolicy.lastUse)||
            now<bundleSweaterPolicy.rearmAt||(!repair&&bundleSweaterFreshUse==bundleSweaterPolicy.lastUse)))||
           bundleSweaterPolicy.publicationStarted)return false;
        auto readsBefore=Reads();
        if(!SweaterContext()){bundleReport.sweaterTurnContextReads+=Reads()-readsBefore;return false;}
        if(!SweaterAlive()){bundleReport.sweaterTurnContextReads+=Reads()-readsBefore;return false;}
        bundleReport.sweaterTurnContextReads+=Reads()-readsBefore;
        if(initial){bool mask=false;readsBefore=Reads();const bool observed=HasPamelaTrigger(mask);
            bundleReport.sweaterTurnPamelaReads+=Reads()-readsBefore;if(!observed||!mask)return false;}
        if(bundleSweaterPolicy.hostedOwnerHandoffUsed){
            bool ownMask=false;readsBefore=Reads();const bool observed=HasPamelaTrigger(ownMask);
            bundleReport.sweaterTurnPamelaReads+=Reads()-readsBefore;if(!observed||!ownMask)return false;
        }
        if(initial&&bundleSweaterClass.anchor==5&&bundleSweaterClass.earnedOwner.object!=bundleSweaterOwner.object){
            bool living=false;readsBefore=Reads();const bool observed=Alive(bundleSweaterClass.earnedOwner.object,living);
            bundleReport.sweaterTurnHostedReads+=Reads()-readsBefore;if(!observed||living)return false;
        }
        std::uintptr_t previous=0;
        readsBefore=Reads();
        if(!r_.Read(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,previous)||
           !SweaterRearmPrevious(previous)){bundleReport.sweaterTurnPreviousReads+=Reads()-readsBefore;return false;} // live native replacement wins
        bundleReport.sweaterTurnPreviousReads+=Reads()-readsBefore;
        bundleReport.sweaterFreshFailure=2;
        const auto admission=[&](unsigned reason){bundleReport.sweaterFreshAdmission=reason;
            bundleReport.sweaterFreshAdmissionReads=r_.reads;bundleReport.sweaterFreshAdmissionCalls=calls_;return false;};
        readsBefore=Reads();const bool classCurrent=SweaterClassCurrent();
        bundleReport.sweaterTurnClassReads+=Reads()-readsBefore;
        if(!classCurrent){
            // A historical successful class capture is not current authority.
            // In particular anchor5 expires when the previous wearer's native
            // cleanup clears the reciprocal edge. Let the still-bounded mask
            // discovery acquire an independent CURRENT world-source lease,
            // instead of queueing the same refused constructor indefinitely.
            if(bundleSweaterClass.slot.object){
                cfgsweater::ExpireClassForDiscovery(bundleSweaterClass,bundleSweaterClassAttempted,bundleSweaterCursor);
                bundleSweaterSettingsCursor=0;
            }
            bundleReport.sweaterFreshClass=0;bundleReport.sweaterClassAnchor=0;
            bundleReport.sweaterClassSerial=0;bundleReport.sweaterClassCdoSerial=0;
            // Anchors4-7 were tried first. A current WorldSettings declaration
            // survives native garment/instance retirement; construction still
            // waits for its OWN fresh callback and full Current admission.
            readsBefore=Reads();const bool discovered=DiscoverSweaterSettings();
            bundleReport.sweaterTurnDiscoveryReads+=Reads()-readsBefore;
            if(discovered||bundleSweaterClass.failure==32)bundleSweaterFreshRequested=true;
            return admission(1);
        }
        if(bundleSweaterClass.anchor==6){
            std::uintptr_t wearer=0,owner=0;bool living=false;
            readsBefore=Reads();
            if(!r_.Read(bundleSweaterClass.worldSource.source.object+cfgloadout::restricteddisk::OwningCharacter,wearer)||
               !r_.Read(bundleSweaterClass.worldSource.ability.object+cfgfeature::disk::AbilityOwner,owner)||
               (wearer&&wearer!=bundleSweaterOwner.object&&(!Alive(wearer,living)||living))||
               (owner&&owner!=bundleSweaterOwner.object&&(!Alive(owner,living)||living))){
                bundleReport.sweaterTurnHostedReads+=Reads()-readsBefore;return admission(1);}
            bundleReport.sweaterTurnHostedReads+=Reads()-readsBefore;
        }
        readsBefore=Reads();
        if(!cfgbundle::Code(r_,plan.module,cfgsweater::freshdisk::SpawnObject,cfgsweater::freshdisk::SpawnObjectCode)){
            bundleReport.sweaterTurnAdmissionReads+=Reads()-readsBefore;return admission(2);}
        if(r_.reads+600>cfgbundle::ReadCap){bundleReport.sweaterTurnAdmissionReads+=Reads()-readsBefore;return admission(3);}
        bundleReport.sweaterTurnAdmissionReads+=Reads()-readsBefore;
        if(!NativeCallAllowed())return admission(4);
        admission(0);
        // One constructor per witnessed use, consumed BEFORE native entry.
        // No allocation replay after ambiguous/partial return or publication.
        if(initial)bundleSweaterFreshGrantAttempted=true;else bundleSweaterFreshUse=bundleSweaterPolicy.lastUse;
        if(repair)bundleSweaterPolicy.repairingOwned=false; // consume before ambiguous allocation
        ++calls_;++bundleReport.sweaterFreshAttempts;
        std::uintptr_t fresh=0;
        if(!ConstructFreshSweater(fresh)){bundleSweaterPolicy.Refuse();
            cfgbundle::RecordSweaterFirstRefusal(bundleReport,201,bundleSweaterPolicy.lastFailure,
                static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,GetTickCount64(),bundleReport.sweaterFreshFailure);return false;}
        bundleReport.sweaterFreshFailure=3;
        cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t owner=0;std::uint8_t used=255;
        // Fresh object array scope is refreshed after native allocation. Strong
        // pawn ownership is published in THIS callback, never keep an unowned
        // serial-zero constructor result across a yield/GC.
        if(!fresh||!FreshSweaterWorldAfterAllocation()||
           !cfgjason::SameSlot(r_,context.objectArray,bundleSweaterOwner)||
           !cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,bundleSweaterOwner.object)||
           !cfgobserver::Same(r_,bundleSweaterOwner.object+cfgescape::disk::Controller,plan.controller)||
           !SweaterAlive()||!SweaterClassCurrent()||
           !cfgspawn::ObjectSlot(r_,context.objectArray,fresh,slot)||
           !cfgobserver::Same(r_,fresh+cfgprobe::ObjectClass,bundleSweaterClass.slot.object)||
           !cfgobserver::Same(r_,fresh+0x20,bundleSweaterOwner.object)||!r_.Read(fresh+cfgfeature::disk::AbilityOwner,owner)||owner||
           !r_.Read(fresh+cfgsweater::disk::Used,used)||used||
           !cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,previous)||
           !SweaterRearmPrevious(previous)){
                bundleSweaterPolicy.Refuse();cfgbundle::RecordSweaterFirstRefusal(bundleReport,202,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,GetTickCount64(),bundleReport.sweaterFreshFailure);return false;}
        bundleReport.sweaterFreshFailure=4;
        if(!SweaterStore(fresh+cfgfeature::disk::AbilityOwner,std::uintptr_t{},bundleSweaterOwner.object)||
           !SweaterStore(bundleSweaterOwner.object+cfgfeature::disk::AbilityField,previous,fresh)||
           !cfgobserver::Same(r_,fresh+cfgfeature::disk::AbilityOwner,bundleSweaterOwner.object)||
           !cfgobserver::Same(r_,bundleSweaterOwner.object+cfgfeature::disk::AbilityField,fresh)){
                bundleSweaterPolicy.Refuse();cfgbundle::RecordSweaterFirstRefusal(bundleReport,203,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,GetTickCount64(),bundleReport.sweaterFreshFailure);return false;}
        bundleSweaterPolicy.publicationStarted=false;bundleSweaterAbility=slot;bundleSweaterSource={};bundleSweaterSourceIndex=~0u;bundleSweaterRetiredAbility={};
        bundleSweaterFreshPending=true;++bundleReport.sweaterFreshCreated;bundleReport.sweaterFreshFailure=0;
        // Next fresh phase performs existing garment/used/HUD contract. Keep
        // lastUse,30s kill window and rearmAt; mask pickup cannot reset them.
        return true;
    }
    // A queued rearm belongs to one exact local pawn. Resolve a newly possessed
    // incarnation before consuming it; Tommy's lifecycle must start from his
    // own current native mask/ability observations, never the prior wearer's.
    bool FreshSweaterContinuation(std::uint64_t now){
        const auto prior=bundleSweaterOwner;
        bundleReport.sweaterTurnOwnerReads=bundleReport.sweaterTurnContextReads=bundleReport.sweaterTurnPamelaReads=0;
        bundleReport.sweaterTurnClassReads=bundleReport.sweaterTurnDiscoveryReads=bundleReport.sweaterTurnHostedReads=0;
        bundleReport.sweaterTurnPreviousReads=bundleReport.sweaterTurnAdmissionReads=0;
        const auto before=Reads();const bool refreshed=RefreshSweaterOwner(now);
        bundleReport.sweaterTurnOwnerReads=Reads()-before;
        if(!refreshed)return false;
        if(!(prior==bundleSweaterOwner))return true; // owner refresh retired the old one-shot obligation
        bundleSweaterFreshRequested=false;
        return FreshSweaterRearm(now);
    }
