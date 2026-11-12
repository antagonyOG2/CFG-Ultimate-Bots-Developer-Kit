#ifndef CFG_CALLBACK_OBSERVATION_MEMBER_FIXTURE_ONLY
#include "NativeCallbackObservationMembers.hpp"
#endif
    bool SummaryScope(){
        // Registration/installation still requires the real completed-tick
        // capability. Native row/filter getters only observe the current UI.
        return GetCurrentThreadId()==plan.threadId&&originalActorDispatcher&&
            cfgprimary::LookupSafe(r_,plan.module)&&SummaryObservationScope();
    }
    bool SummaryEndingPhase(unsigned* failure=nullptr){
        if(failure)*failure=2;
        cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(r_,plan.module);
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text))return false;
        if(failure)*failure=4;
        if(!cfgsummaryending::Phase(text.View()))return false;
        if(failure)*failure=1;
        if(!SummaryScope()||!cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase))return false;
        if(failure)*failure=0;return true;
    }
    bool SummaryEndingRefusal(bool ending,unsigned stage,std::uintptr_t state=0){
        if(!ending||bundleSummary.endingTraces++>=12)return true;
        cfgclosedtrace::Mark("ending-summary-refused",state,(static_cast<std::uint64_t>(stage)<<32)|Reads());
        std::uintptr_t owner=0;std::uint32_t flags=0;cfgsummary::List players{};
        if(state&&r_.Read(state+0x88,owner)&&r_.Read(state+0x394,flags))
            cfgclosedtrace::Mark("ending-summary-state-owner-flags",owner,flags);
        if(r_.Read(spawnAssets.gameState+cfgsummary::disk::Players,players)&&players.Sane())
            cfgclosedtrace::Mark("ending-summary-native-participants",spawnAssets.gameState,players.count);
        return true; // bounded observations only, never fallback authority
    }
    bool SummaryPhase(std::uint64_t now,bool ending=false){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Summary);
#endif

        if(now<bundleSummary.next)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Summary,1)
#else
 false
#endif
;bundleSummary.next=now+100;
        if(!SummaryScope())return SummaryEndingRefusal(ending,1);
        if(!bundleSummaryFilterEnabled){
            const bool ready=PrepareBundleSummaryFilter(plan.module);
            cfgclosedtrace::Mark(ready?"summary-offline-bot-filter-ready":"summary-offline-bot-filter-refused",plan.world,Reads());
            if(!ready)bundleSummary.next=now+5000;return true;
        }
        if(!bundleSummaryRowReadyEnabled){
            const bool ready=PrepareBundleSummaryRowReady(plan.module);
            cfgclosedtrace::Mark(ready?"summary-row-presentation-ready":"summary-row-presentation-refused",plan.world,Reads());
            if(!ready)bundleSummary.next=now+5000;return true;
        }
        if(!bundleSummaryPresentationEnabled){
            const bool ready=PrepareBundleSummaryPresentation(plan.module);
            cfgclosedtrace::Mark(ready?"summary-native-presentation-ready":"summary-native-presentation-refused",plan.world,Reads());
            if(!ready)bundleSummary.next=now+5000;return true;
        }
        const bool statusTurn=!ending&&(++bundleSummarySurfaceTurn%4)==0;
        auto& s=statusTurn?bundleStatusRows:bundleSummary;
        auto& roster=statusTurn?bundleStatusBots:bundleSummaryBots;
        // Real native participants survive mortal counselor/controller teardown
        // and include Jason. One bot state per callback; no synthetic rows.
        std::uintptr_t state=0;cfgprobe::lifetime::SlotObservation stateSlot{};
        cfgsummary::List players{};
        if(!r_.Read(spawnAssets.gameState+cfgsummary::disk::Players,players)||!players.Sane())return SummaryEndingRefusal(ending,2);
        if(!players.count)return SummaryEndingRefusal(ending,3);
        if(s.cursor>=static_cast<unsigned>(players.count))s.cursor=0;
        unsigned source=s.cursor++;
        if(!r_.Read(players.data+8ULL*source,state)||!cfgspawn::ObjectSlot(r_,context.objectArray,state,stateSlot))return SummaryEndingRefusal(ending,4,state);
        struct Host {BundleAccess& a;
            bool Scope(){return a.SummaryScope();}
            bool NativeClass(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& type){return a.NativeClass(reflection,constructor,type);}
            bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){return cfgspawn::ObjectSlot(a.r_,a.context.objectArray,object,slot);}
            bool StateType(std::uintptr_t object){return a.SummaryStateType(object);}
        } host{*this};
        bool endingStack=false;
        if(ending){
            endingStack=bundleStatusSurface.Step(r_,host,plan.module,plan.controller);
            // Diagnostic only (bounded): why the end-screen list was not bound.
            static unsigned endingSurfaceTraces=0;
            if(!endingStack&&endingSurfaceTraces<16){++endingSurfaceTraces;
                cfgclosedtrace::Mark("ending-status-surface-pending",bundleStatusSurface.binding.root.object,
                    (static_cast<std::uint64_t>(bundleStatusSurface.lastFailure)<<32)|Reads());}
            if(!endingStack&&Reads()>512)return true; // discovery owns its bounded turn
        }
        if(statusTurn&&!bundleStatusSurface.Step(r_,host,plan.module,plan.controller))return true;
        if(!cfgsummarybots::Participant(r_,host,spawnAssets.gameState,state,stateSlot,&source))return SummaryEndingRefusal(ending,5,state);
        unsigned entryIndex=32;
        for(unsigned i=0;i<bundleSummaryBots.entries.size();++i){const auto& candidate=bundleSummaryBots.entries[i];
            if(candidate.state.object==state){entryIndex=i;break;}
            if(!candidate.state.object&&entryIndex==32)entryIndex=i;}
        if(entryIndex==32)return true;
        auto& entry=bundleSummaryBots.entries[entryIndex];
        if(entry.state.object&&!cfgbundle::OwnedClassSlotMatches(entry.state,stateSlot))return true;
        // Latch an assigned serial if the real strong state has acquired one.
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,state,stateSlot))return true;
        entry.state=stateSlot;
        cfgsummary::Proof proof{};
        const auto* status=(statusTurn||endingStack)?&bundleStatusSurface.binding:nullptr;
        if(!cfgsummary::Observe(r_,host,plan.module,plan.controller,spawnAssets.gameState,state,source,proof,status)){
            ++s.refused;return SummaryEndingRefusal(ending,6,state);}
        if(!statusTurn)bundleSummaryStackBound=endingStack;
        roster.entries=bundleSummaryBots.entries;
        roster.display=proof.display.object;roster.displaySlot=proof.display;
        roster.hud=proof.hud;roster.root=proof.root;
        if(s.widget!=proof.display.object||s.widgetSerial!=proof.display.serial){
            s.widget=proof.display.object;s.widgetSerial=proof.display.serial;s.attempted={};s.confirmedBefore={};s.count=0;}
        if(proof.present&&proof.rowPresent){s.Confirm(state);return true;}
        // Native Construct/refresh can rebuild a previously confirmed list.
        // Re-add only after BOTH current native collections prove absence.
        // A never-confirmed ambiguous call remains non-retryable.
        if(!s.MayRegister(state,proof.present,proof.rowPresent))return true;
        if(calls_>=cfgbundle::CallCap||!cfgsummarybots::Participant(r_,host,spawnAssets.gameState,state,stateSlot)||
           !NativeEnvironmentAllowed())return true;
        s.Attempt(state);++s.attempts;++calls_;
        const bool returned=SummaryRegister(proof.display.object,state);
        cfgsummary::List registered{},rows{};bool present=false,rowPresent=false,complete=true;
        if(returned&&r_.Read(proof.display.object+cfgsummary::disk::Registered,registered)&&registered.Sane()&&
           r_.Read(proof.display.object+cfgsummary::disk::Rows,rows)&&rows.Sane()){
            for(int i=0;i<registered.count;++i){std::uintptr_t value=0;if(!r_.Read(registered.data+8ULL*i,value)){complete=false;break;}if(value==state)present=true;}
            for(int i=0;i<rows.count;++i){std::uintptr_t row=0,value=0;cfgprobe::lifetime::SlotObservation slot{};
                if(!r_.Read(rows.data+8ULL*i,row)||!cfgspawn::ObjectSlot(r_,context.objectArray,row,slot)||
                   !slot.HasAssignedSerial()||!cfgobserver::Same(r_,row+cfgprobe::ObjectClass,proof.rowClass)||
                   !cfgobserver::Same(r_,row+0x20,proof.display.object)||
                   !r_.Read(row+cfgsummary::disk::RowPlayerState,value)){complete=false;break;}if(value==state)rowPresent=true;}
            complete=complete&&cfgsummary::SameList(r_,proof.display.object+cfgsummary::disk::Registered,registered)&&
                cfgsummary::SameList(r_,proof.display.object+cfgsummary::disk::Rows,rows)&&
                cfgsummarybots::Owned(host,proof.display)&&host.Scope()&&
                (!status||cfgstatussurface::Current(r_,host,plan.module,plan.controller,*status));
        }
        if(complete&&present&&rowPresent){++s.confirmed;s.Confirm(state);}else ++s.refused;
        cfgclosedtrace::Mark(complete&&present&&rowPresent?(statusTurn?"status-native-bot-row-confirmed":"summary-native-bot-row-confirmed"):
            "summary-native-registration-unconfirmed",state,Reads());return true;
    }
    bool SummaryRegister(std::uintptr_t widget,std::uintptr_t state)noexcept{
#ifdef CFG_SUMMARY_MEMBER_FIXTURE_ONLY
        return FixtureSummaryRegister(widget,state);
#else
        if(!NativeEnvironmentAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,5>{plan.world,plan.mode,spawnAssets.gameState,widget,state})||
           !BeginBundleNative())return false;
#endif
        __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(plan.module+cfgsummary::disk::Register)(
            reinterpret_cast<void*>(widget),reinterpret_cast<void*>(state));}
        __except(EXCEPTION_EXECUTE_HANDLER){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;
        }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
#endif
    }
    bool SummaryBotEligible(std::uintptr_t query,std::uintptr_t state){
        struct Host {BundleAccess& a;
            bool Scope(){return a.SummaryObservationScope();}
            bool NativeClass(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& type){return a.NativeClass(reflection,constructor,type);}
            bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){return cfgspawn::ObjectSlot(a.r_,a.context.objectArray,object,slot);}
            bool StateType(std::uintptr_t object){return a.SummaryStateType(object);}
        }host{*this};
        std::uintptr_t display=0;if(!r_.Read(query,display))return false;
        if(display==bundleSummaryBots.display&&bundleSummaryStackBound&&bundleStatusSurface.ready)
            return cfgsummarybots::Eligible(r_,host,bundleSummaryBots,query,plan.controller,spawnAssets.gameState,state,&bundleStatusSurface.binding,plan.module);
        if(display==bundleStatusBots.display&&bundleStatusSurface.ready)
            return cfgsummarybots::Eligible(r_,host,bundleStatusBots,query,plan.controller,spawnAssets.gameState,state,&bundleStatusSurface.binding,plan.module);
        return cfgsummarybots::Admit(r_,host,bundleSummaryBots,plan.module,query,plan.controller,spawnAssets.gameState,state);
    }
    bool SummaryStateType(std::uintptr_t object){return cfgspawn::ObjectIs(r_,object,spawnAssets.resources.classes[cfgspawn::PlayerState]);}
    bool SummaryRowReady(std::uintptr_t row){
        struct Host {BundleAccess& a;
            bool Scope(){return a.SummaryObservationScope();}
            bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){return cfgspawn::ObjectSlot(a.r_,a.context.objectArray,object,slot);}
            bool StateType(std::uintptr_t object){return a.SummaryStateType(object);}
        }host{*this};
        std::uintptr_t display=0;if(!r_.Read(row+0x20,display))return false;
        if(display==bundleSummaryBots.display&&bundleSummaryStackBound&&bundleStatusSurface.ready)
            return cfgsummarybots::RowReady(r_,host,bundleSummaryBots,row,plan.module,plan.controller,spawnAssets.gameState,&bundleStatusSurface.binding);
        if(display==bundleStatusBots.display&&bundleStatusSurface.ready)
            return cfgsummarybots::RowReady(r_,host,bundleStatusBots,row,plan.module,plan.controller,spawnAssets.gameState,&bundleStatusSurface.binding);
        return cfgsummarybots::RowReady(r_,host,bundleSummaryBots,row,plan.module,plan.controller,spawnAssets.gameState);
    }
    bool SummaryPresentedState(std::uintptr_t row,std::uintptr_t state){
        return cfgobserver::Same(r_,row+cfgsummary::disk::RowPlayerState,state)&&SummaryRowReady(row);
    }
