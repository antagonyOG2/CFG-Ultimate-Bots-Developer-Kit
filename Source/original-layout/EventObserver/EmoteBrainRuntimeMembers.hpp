// Inside BundleAccess. Ordinary helper work yields while the native emote owns
// the bot. Never claim an externally disabled brain or interrupt a paired kill.
    bool mirrorBrainRetired_=false;
    int MirrorIndex(const cfgemotemirror::Recipient& target)const{
        for(unsigned i=0;i<bundleReport.bots;++i)
            if(target.pawn==bundlePawnSlots[i]&&target.controller==bundleControllerSlots[i])return static_cast<int>(i);
        return -1;
    }
    bool MirrorBrainCurrent(unsigned index,bool acquire){
        mirrorBrainRetired_=false;
        if(index>=bundleReport.bots||!Context())return false;
        auto& hold=bundleEmoteBrains[index];std::uintptr_t pawn=0;bool living=false;
        cfgprobe::lifetime::SlotObservation brain{};
        if(!Known(index+1,pawn)||!Alive(pawn,living))return false;
        if(!living){mirrorBrainRetired_=true;return false;}
        if(!cfgconvergencetravel::Brain(r_,context.objectArray,bundleControllerSlots[index],
            spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],brain))return false;
        if(acquire){hold.pawn=bundlePawnSlots[index];hold.controller=bundleControllerSlots[index];hold.brain=brain;return true;}
        if(!(hold.pawn==bundlePawnSlots[index])||!(hold.controller==bundleControllerSlots[index])||
            hold.brain.object!=brain.object||hold.brain.index!=brain.index){mirrorBrainRetired_=true;return false;}
        if(!hold.brain.serial&&brain.serial>0)hold.brain=brain; // native weak assignment, SAME proved positive owner's strong child
        mirrorBrainRetired_=!(hold.brain==brain);return !mirrorBrainRetired_;
    }
    bool MirrorBrainTick(unsigned index,bool set,bool& enabled){
        namespace d=cfgconvergencetravel::disk;
        if(!MirrorBrainCurrent(index,false))return false;
        std::uint8_t value=enabled?1:0;const auto brain=bundleEmoteBrains[index].brain.object;
        const bool okay=set?CarCall(d::SetComponentTickEnabled,d::SetComponentTickEnabledExecutor,d::SetComponentTickEnabledCode,brain,&value):
            CarCall(d::IsComponentTickEnabled,d::IsComponentTickEnabledExecutor,d::IsComponentTickEnabledCode,brain,&value);
        if(!okay||value>1||!MirrorBrainCurrent(index,false))return false;
        enabled=value!=0;return true;
    }
    unsigned MirrorInterrupt(const cfgemotemirror::Recipient& target){
        const auto found=MirrorIndex(target);if(found<0||bundleKill.policy.Exclusive(GetTickCount64())||bundleCompletionReady)return 2;
        const auto index=static_cast<unsigned>(found);auto& hold=bundleEmoteBrains[index];
        const auto now=GetTickCount64();
        if(!hold.phase){
            // Do not steal the convergence lane's owned disabled brain.
            if(bundleConvergenceTravel[index].policy.owned){TravelResume(index,now,false);return 0;}
            if(!MirrorBrainCurrent(index,true))return 2;
            bool enabled=false;if(!MirrorBrainTick(index,false,enabled)||!enabled){hold={};return 2;}
            hold.phase=1;hold.until=now+3000;return 0;
        }
        if(!MirrorBrainCurrent(index,false))return 2;
        if(hold.phase==4){
            std::uintptr_t montage=0;if(!r_.Read(target.pawn.object+cfgemotemirror::disk::CurrentEmote,montage))return 2;
            if(montage&&montage!=hold.montage&&!bundleEmotes.OwnsMontage(target,montage)){hold.phase=5;return 2;} // preserve an independent emote
            hold.phase=1;hold.until=now+3000;
            if(montage&&!CarCall(cfgemotemirror::disk::SERVER_CancelEmote,cfgemotemirror::disk::SERVER_CancelEmoteExecutor,
               cfgemotemirror::disk::SERVER_CancelEmoteCode,target.pawn.object,nullptr)){hold.phase=5;return 2;}
            return 0; // stop ONLY prior owned emote before starting the new selection
        }
        if(now>=hold.until)return 2;
        if(hold.phase==1){
            bool enabled=false;hold.phase=2;hold.changed=true; // cleanup owed before ambiguous native setter
            if(!MirrorBrainTick(index,true,enabled)){hold.phase=5;return 2;}return 0;
        }
        if(hold.phase==2){
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,
               target.controller.object,nullptr)){hold.phase=5;return 2;}
            hold.phase=3;++bundleEmoteStops;return 0;
        }
        return hold.phase==3?1u:2u;
    }
    void MirrorPlayed(const cfgemotemirror::Recipient& target,std::uintptr_t montage){
        const auto found=MirrorIndex(target);if(found<0)return;auto& hold=bundleEmoteBrains[static_cast<unsigned>(found)];
        if(hold.phase!=3)return;
        hold.montage=montage;hold.phase=montage?4u:5u;hold.until=GetTickCount64()+25000;
    }
    bool MirrorResume(unsigned index){
        auto& hold=bundleEmoteBrains[index];if(!hold.phase)return true;
        if(!hold.changed){hold={};return true;}
        // One native operation per fresh callback. Query+setter plus repeated
        // 3.5 owner proofs must not share a single1536-read gameplay budget.
        bool enabled=hold.phase==6;
        if(!MirrorBrainTick(index,hold.phase==6,enabled)){
            if(mirrorBrainRetired_)hold={};
            return false; // unproved/GC/budget failure owes cleanup on a fresh callback
        }
        if(!enabled){hold.phase=6;return false;} // owned restore setter is due next callback
        hold={};++bundleEmoteResumes;return true;
    }
    bool MirrorMaintain(std::uint64_t now){
        if(now<bundleEmoteMaintainAt)return false;
        for(unsigned n=0;n<bundleReport.bots;++n){
            const unsigned index=bundleEmoteMaintainCursor++%bundleReport.bots;auto& hold=bundleEmoteBrains[index];
            if(!hold.phase)continue;
            // Pending interruption owns three short callbacks. No native work
            // is performed here until playback, cancellation or finite expiry.
            if(hold.phase<4&&now<hold.until&&bundleEmotes.InterruptPending({hold.pawn,hold.controller}))continue;
            bundleEmoteMaintainAt=now+100;
            if(hold.phase==4){
                if(!MirrorBrainCurrent(index,false)){
                    if(mirrorBrainRetired_)hold={};
                    return true; // retain obligation on transient proof failure; no native call
                }
                std::uintptr_t montage=0;if(!r_.Read(hold.pawn.object+cfgemotemirror::disk::CurrentEmote,montage))return true;
                if(montage!=hold.montage&&bundleEmotes.OwnsMontage({hold.pawn,hold.controller},montage))hold.montage=montage;
                if(montage==hold.montage){
                    if(now<hold.until)return true;
                    // Donor's 25s bound: only cancel the EXACT emote we started.
                    std::uintptr_t function=0,owner=0;
                    if(!FeatureFunction(cfgemotemirror::disk::SERVER_CancelEmote,function,owner)||
                       !cfgspawn::ObjectIs(r_,hold.pawn.object,owner)||
                       !cfgobserver::Same(r_,function+0xB0,plan.module+cfgemotemirror::disk::SERVER_CancelEmoteExecutor)||
                       !cfgbundle::Code(r_,plan.module,cfgemotemirror::disk::SERVER_CancelEmoteExecutor,cfgemotemirror::disk::SERVER_CancelEmoteCode))return true;
                    hold.phase=5;Invoke(hold.pawn.object,function,nullptr,true);return true;
                }
                hold.phase=5; // completion/new independent emote revokes playback ownership
            }
            MirrorResume(index);return true;
        }
        return false; // no held bots: no native idle polling
    }
