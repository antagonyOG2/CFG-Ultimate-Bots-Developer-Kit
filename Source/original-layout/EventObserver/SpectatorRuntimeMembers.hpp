// Included inside BundleAccess. Separate local camera lane; never changes
// possession, input bindings, HUD ownership, or any watched pawn's modifiers.
    bool spectatorSafetyProved_=false;
    struct SpectatorStatusEvent {
        cfgprobe::lifetime::SlotObservation widget{},function{};
        std::uintptr_t widgetClass=0;
    };
    bool SpectatorStatusEventCurrent(const SpectatorStatusEvent& event){
        cfgprobe::lifetime::ArrayHeader registry{};
        return event.widget.HasAssignedSerial()&&event.function.HasAssignedSerial()&&
            cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgjason::SameSlot(r_,registry,event.widget)&&cfgjason::SameSlot(r_,registry,event.function)&&
            cfgobserver::Same(r_,event.widget.object+cfgprobe::ObjectClass,event.widgetClass);
    }
    bool ResolveSpectatorStatusEvent(std::uintptr_t widget,SpectatorStatusEvent& output){
        output={};SpectatorStatusEvent event;cfgreflection::Field field;
        cfgreflection::Observation<Reader> reflection(r_,plan.module);
        // Native CFG wrapper4E6C20 resolves the event on its RECEIVER, like
        // donor FindFunction. The native factory cache is only a name hint;
        // dispatching it directly can skip the visible Blueprint override.
        if(!Context()||!cfgspawn::ObjectSlot(r_,context.objectArray,widget,event.widget)||!event.widget.HasAssignedSerial()||
            !r_.Read(widget+cfgprobe::ObjectClass,event.widgetClass)||
            !reflection.Find(event.widgetClass,u"OnSpectatorUpdateStatus",field,true)||field.kind.View()!=u"Function"||
            !cfgspawn::ObjectSlot(r_,context.objectArray,field.object,event.function)||!SpectatorStatusEventCurrent(event))return false;
        output=event;return true; // hint only; full HUD/frame/selection proof remains mandatory before Invoke
    }
    bool SpectatorStatus(std::uintptr_t widget,std::uintptr_t function,std::uintptr_t parameters,cfgspectatorstatus::Frame* replacement=nullptr){
        r_.cap=cfgbundle::ReadCap-2; // reserve the event-name hint outside this fresh scope
        using namespace cfgsweater::huddisk;
        std::uintptr_t eventOwner=0,hud=0,actual=0,hudClass=0,widgetClass=0,child=0;
        cfgprobe::lifetime::SlotObservation hs{},ws{};cfgreflection::Field event;
        cfgreflection::Observation<Reader> reflection(r_,plan.module);
        std::uint16_t bytes=0;std::uint8_t count=0;
        if(!parameters||!SummaryObservationScope()||!SpectatorLocalOwner()||
           !NativeClass(SCInGameHUDReflection,SCInGameHUDConstructor,hudClass)||!NativeClass(SCHUDWidgetReflection,SCHUDWidgetConstructor,widgetClass)||
           !r_.Read(plan.controller+ControllerHUD,hud)||!cfgspawn::ObjectSlot(r_,context.objectArray,hud,hs)||!hs.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r_,hud,hudClass)||!r_.Read(hud+HUDWidget,actual)||actual!=widget||
           !cfgspawn::ObjectSlot(r_,context.objectArray,widget,ws)||!ws.HasAssignedSerial()||!cfgspawn::ObjectIs(r_,widget,widgetClass)||
           !r_.Read(function+0x20,eventOwner)||!cfgmenu::maps::Subclass(r_,eventOwner,widgetClass)||!cfgspawn::ObjectIs(r_,widget,eventOwner)||
           !reflection.ReadField(function,eventOwner,event)||event.kind.View()!=u"Function"||event.label.View()!=u"OnSpectatorUpdateStatus"||
           !r_.Read(function+0x8C,count)||count!=2||!r_.Read(function+0x8E,bytes)||bytes!=32||
           !r_.Read(function+cfgreflection::disk::Children,child))return false;
        unsigned mask=0;
        for(unsigned i=0;child&&mask!=3&&i<16;++i){cfgreflection::Field f;cfgreflection::Property p;std::uint64_t flags=0;
            if(!reflection.ReadField(child,function,f)||!r_.Read(f.object+cfgreflection::disk::PropertyFlags,flags))return false;
            child=f.next;if(!(flags&0x80u))continue; // Blueprint override locals are not input parameters
            if(!reflection.ReadProperty(f,p)||f.kind.View()!=u"StrProperty"||
               p.dimension!=1||p.elementBytes!=16)return false;
            unsigned bit=0;if(f.label.View()==u"StatusText"&&p.offset==0)bit=1;
            if(f.label.View()==u"NextMode"&&p.offset==16)bit=2;
            if(!bit||(mask&bit))return false;mask|=bit;
        }
        std::array<char16_t,96> text{};std::array<char16_t,64> mode{};cfgspectatorstatus::Frame before{};
        // NumParms==2 and ParmsSize==32 above bound the input frame. The rest
        // of a Blueprint override's Children chain contains script locals,
        // not extra input parameters. Never require its tail to be empty.
        if(mask!=3||!r_.Read(parameters,before)||!cfgspectatorstatus::Copy(r_,parameters,text)||!cfgspectatorstatus::Copy(r_,parameters+16,mode)||
           !cfgjason::SameSlot(r_,context.objectArray,hs)||!cfgjason::SameSlot(r_,context.objectArray,ws)||
           !cfgobserver::Same(r_,plan.controller+ControllerHUD,hud)||!cfgobserver::Same(r_,hud+HUDWidget,widget)||
           !cfgobserver::Same(r_,function+0x20,eventOwner)||!cfgescape::Same(r_,function+0x8C,count)||
           !cfgescape::Same(r_,function+0x8E,bytes))return false;
        bundleReport.spectatorStatusText=text;bundleReport.spectatorStatusNextMode=mode;
        bundleReport.spectatorStatusWidget=widget;bundleReport.spectatorStatusJasonView=bundleSpectator.Following()?1u:0u;
        ++bundleReport.spectatorStatusEvents;
        if(!replacement)return true;
        cfgspectator::Snapshot camera{};cfgspectator::Identity selected{};bool living=false;
        cfgspectatorstatus::Frame original{};
        if(!bundleSpectator.Following()||!SpectatorSafety(camera,false)||
           !Jason()||!Alive(bundleJason.pawn.object,living)||!living||
           !SpectatorIdentity(bundleJason.pawn.object,selected)||camera.nativeSelection!=selected||
           !r_.Read(parameters,original)||std::memcmp(&before,&original,sizeof(before))!=0||!cfgjason::SameSlot(r_,context.objectArray,hs)||
           !cfgjason::SameSlot(r_,context.objectArray,ws)||
           !cfgobserver::Same(r_,plan.controller+ControllerHUD,hud)||!cfgobserver::Same(r_,hud+HUDWidget,widget)||
           !cfgobserver::Same(r_,plan.controller+cfgspectator::disk::SpectatingPlayer,selected.object)||
           !SpectatorSafetyLinks())return false;
        cfgspectatorstatus::JasonFrame(text,original,*replacement);
        ++bundleReport.spectatorLabelOverrides;return true;
    }
    cfgspectator::Snapshot spectatorSafetyProof_{};
    cfgnames::Words spectatorPhase_{};
    std::uintptr_t spectatorPossessed_=0,spectatorNative_=0,spectatorClass_=0;
    cfgprobe::lifetime::SlotObservation spectatorSlotProof_{};
    bool spectatorTerminalProved_=false;
    unsigned spectatorTerminalCalls_=0;
    cfgprobe::lifetime::SlotObservation spectatorTerminalHuman_{},spectatorTerminalPawn_{};
    cfgprobe::lifetime::ArrayHeader spectatorTerminalRegistry_{};
    cfgescape::State spectatorTerminalState_=cfgescape::State::Unknown;
    bool SpectatorTerminal(const cfgprobe::lifetime::SlotObservation& spectator,cfgescape::State& terminal){
        // BundleAccess is callback-local. TerminalSpectatorOwner is read-only;
        // reuse its complete proof ONLY before another native call in this same
        // callback. Never carry terminal authority across ReadPose, pause query,
        // selection/rotation, or camera dispatch. All other Links remain fresh.
        const auto& registry=context.objectArray;
        if(spectatorTerminalProved_&&spectatorTerminalCalls_==calls_&&
           spectatorTerminalHuman_==bundleHumanTarget&&spectatorTerminalPawn_==spectator&&
           spectatorTerminalRegistry_.data==registry.data&&spectatorTerminalRegistry_.count==registry.count){terminal=spectatorTerminalState_;return true;}
        spectatorTerminalProved_=false;
        if(!cfgspectator::TerminalSpectatorOwner(r_,registry,plan.module,plan.controller,bundleHumanTarget,bundleCounselorClass,
            spectator,terminal)||!cfgescape::Terminal(terminal))return false;
        spectatorTerminalCalls_=calls_;spectatorTerminalHuman_=bundleHumanTarget;spectatorTerminalPawn_=spectator;
        spectatorTerminalRegistry_=registry;spectatorTerminalState_=terminal;spectatorTerminalProved_=true;return true;
    }
    bool SpectatorSafetyLinks(){
        cfgspectator::Identity identity{};cfgescape::State terminal;
        const auto check=[&](unsigned leaf,bool value){bundleReport.spectatorLinkReads=r_.reads;
            if(!value)bundleReport.spectatorLinkFailure=leaf;return value;};
        // Preserve each failed proof leaf. ABI45's latest successful/paused
        // observation could hide an earlier camera refusal; the outer receipt
        // now retains it. Same-slice terminal credit expires at any native call.
        const bool okay=check(1,spectatorSafetyProved_&&GetCurrentThreadId()==plan.threadId)&&
            check(2,cfgprimary::LookupSafe(r_,plan.module))&&check(3,cfgprimary::RetainedOfflineWorld(r_,plan,context))&&
            check(4,cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))&&check(5,SpectatorLocalOwner())&&
            check(6,cfgjason::SameSlot(r_,context.objectArray,plan.objects[2]))&&
            check(7,SpectatorTerminal(spectatorSlotProof_,terminal)&&cfgescape::Terminal(terminal))&&
            check(8,cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,spectatorPossessed_))&&
            check(9,cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerSpectatorPawn,spectatorNative_))&&
            check(10,SpectatorIdentity(spectatorSafetyProof_.spectator.object,identity)&&identity==spectatorSafetyProof_.spectator)&&
            check(11,cfgobserver::Same(r_,identity.object+cfgprobe::ObjectClass,spectatorClass_)&&
                cfgobserver::Same(r_,identity.object+cfgescape::disk::Controller,plan.controller))&&
            check(12,cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,spectatorPhase_));
        if(okay)bundleReport.spectatorLinkFailure=0;return okay;
    }
    bool SpectatorIdentity(std::uintptr_t object,cfgspectator::Identity& out){
        cfgprobe::lifetime::SlotObservation slot{};out={};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,object,slot)||!slot.HasAssignedSerial())return false;
        out={slot.object,slot.serial};return true;
    }
    bool SpectatorTarget(std::uintptr_t pawn,bool killer,cfgspectator::Target& out){
        out={};std::uintptr_t state=0,controller=0;bool living=false;
        if((!killer&&(!Alive(pawn,living)||!living))||!SpectatorIdentity(pawn,out.pawn)||
            !r_.Read(pawn+cfgescape::disk::PlayerState,state)||!state||
            !r_.Read(pawn+cfgescape::disk::Controller,controller)||!controller||
            // OwnedPlayerState repeats both reciprocal links and the complete
            // state class/lifetime proof below. Do not pay for a duplicate walk.
            !cfgspectator::OwnedPlayerState(r_,context.objectArray,pawn,controller,state,spawnAssets.resources.classes[cfgspawn::PlayerState],out.state)||
            (killer&&!cfgobserver::Same(r_,spawnAssets.gameState+cfgprimary::CurrentKillerState,state))||
            !cfgspawn::ObjectIs(r_,pawn,killer?spawnAssets.killerBase:bundleCounselorClass)||
            (killer&&(!cfgspectator::KillerLiving(r_,pawn,state,living)||!living)))return false;
        out.living=true;return true;
    }
    void SpectatorStep(std::uint64_t now,bool f4){
#if CFG_H281_SPECTATOR_BUDGET
        BundleAccess view(4096,calls_);view.context=context;ChildAccounting accounting(*this,view);
        bundleSpectator.Step(view,now,f4);exception_=exception_||view.exception_;return;
#endif
        bundleSpectator.Step(*this,now,f4);
    }
    bool SpectatorWarm(){
        return false; // Native pause getter has no reflected factory/CDO prerequisite.
    }
    bool SpectatorPause(bool& paused){
        using namespace cfgspectator::pausedisk;
        paused=true;
        bundleReport.spectatorFailure=6;
        if(GetCurrentThreadId()!=plan.threadId||calls_>=cfgbundle::CallCap||
           context.world!=plan.world||context.module!=plan.module||
           !cfgprimary::LookupSafe(r_,plan.module)||
           !cfgjason::SameSlot(r_,context.objectArray,plan.objects[0])||
           !cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)||
           !cfgobserver::Same(r_,plan.module+Engine,plan.engine)||
           !cfgbundle::Code(r_,plan.module,Query,QueryCode))return false;
        ++calls_;
        __try{paused=reinterpret_cast<bool(__fastcall*)(void*)>(plan.module+Query)(reinterpret_cast<void*>(plan.world));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
        // The fully pinned query is a pure world getter: recheck GC/world slot and
        // engine/state around it, not two additional190-read cold owner walks.
        // Safety still revalidates terminal/possession/phase before mutation.
        return cfgprimary::LookupSafe(r_,plan.module)&&
            cfgjason::SameSlot(r_,context.objectArray,plan.objects[0])&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&
            cfgobserver::Same(r_,plan.module+Engine,plan.engine);
    }
    bool SpectatorSafety(cfgspectator::Snapshot& out,bool pause=true){
        // Refresh this metadata hint only in the already paced camera lane.
        // Unrelated ProcessEvent callbacks do no memory reads for diagnostics.
        std::uintptr_t status=0,hud=0,widget=0;cfgnames::Words statusName{};
        if(r_.Read(plan.module+cfgspectator::disk::StatusFunctionCache,status)&&cfgprobe::ObjectPointer(status)&&
           r_.Read(status+cfgnames::disk::ObjectName,statusName)&&
           r_.Read(plan.controller+cfgsweater::huddisk::ControllerHUD,hud)&&cfgprobe::ObjectPointer(hud)&&
           r_.Read(hud+cfgsweater::huddisk::HUDWidget,widget)&&cfgprobe::ObjectPointer(widget)){
            bundleSpectatorStatusFunction=status;bundleSpectatorStatusName=statusName;
            bundleSpectatorStatusWidgetHint=widget;bundleSpectatorStatusNameReady=true;
        }
        bundleReport.spectatorFailure=1;
        if(spectatorSafetyProved_){
            out=spectatorSafetyProof_;std::uintptr_t selected=0;
            // Pure pinned pause query first, then the complete fresh authority
            // proof. Links refreshes the object array before selected-pawn
            // identity is observed; no native call intervenes afterward.
            bundleReport.spectatorFailure=6;
            if(pause&&!SpectatorPause(out.paused))return false;
            if(!SpectatorSafetyLinks())return false;
            bundleReport.spectatorFailure=1;
            if(!r_.Read(plan.controller+cfgspectator::disk::SpectatingPlayer,selected))return false;
            out.observedState={};out.nativeSelection={};
            if(selected&&!SpectatorIdentity(selected,out.nativeSelection))out.nativeSelection={selected,0};
            bundleReport.spectatorFailure=0;return true;
        }
        out={};std::uintptr_t spectator=0,possessed=0,type=0,state=0;
        cfgescape::State human;cfgnames::Words phase;cfgnames::Text text;
        if(!Context()||!cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])||!SpectatorLocalOwner())return false;
        bundleReport.spectatorFailure=3;
        // Exact failing admission leaf, using the SAME current reads as the
        // original guard. No extra idle scans, weakened owner or serial rule.
        bundleReport.spectatorPossessedPawn=0;bundleReport.spectatorStockPawn=0;
        bundleReport.spectatorCandidatePawn=0;bundleReport.spectatorAdmissionType=0;
        bundleReport.spectatorAdmissionOwner=0;bundleReport.spectatorAdmissionSerial=0;
        bundleReport.spectatorAdmission=1;
        if(!r_.Read(plan.controller+cfgprobe::ControllerPawn,possessed))return false;
        bundleReport.spectatorPossessedPawn=possessed;bundleReport.spectatorAdmission=2;
        if(!r_.Read(plan.controller+cfgprobe::ControllerSpectatorPawn,spectator))return false;
        bundleReport.spectatorStockPawn=spectator;bundleReport.spectatorAdmission=3;
        if(!spectator&&!(spectator=possessed))return false;
        bundleReport.spectatorCandidatePawn=spectator;bundleReport.spectatorAdmission=4;
        if(possessed&&possessed!=spectator&&possessed!=bundleHumanTarget.object)return false;
        bundleReport.spectatorAdmission=5;
        if(!NativeClass(cfgspectator::disk::PawnClassCache,cfgspectator::disk::PawnConstructorCache,type))return false;
        bundleReport.spectatorAdmissionType=type;bundleReport.spectatorAdmission=6;
        if(!cfgspawn::ObjectIs(r_,spectator,type))return false;
        bundleReport.spectatorAdmission=7;std::uintptr_t owner=0;
        if(!r_.Read(spectator+cfgescape::disk::Controller,owner))return false;
        bundleReport.spectatorAdmissionOwner=owner;if(owner!=plan.controller)return false;
        bundleReport.spectatorAdmission=8;cfgprobe::lifetime::SlotObservation identity{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,spectator,identity))return false;
        bundleReport.spectatorAdmissionSerial=identity.serial;
        if(!identity.HasAssignedSerial())return false;
        out.spectator={identity.object,identity.serial};bundleReport.spectatorAdmission=9;
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,phase))return false;
        bundleReport.spectatorAdmission=0;
        cfgprobe::lifetime::SlotObservation spectatorSlot{};
        // The pinned getter touches only the already-proved world. Query before
        // acquiring terminal authority, then retain the COMPLETE owner/serial/
        // terminal/phase proof after it. This avoids proving a lagged native
        // death twice on opposite sides of the same pure pause call.
        bundleReport.spectatorFailure=6;
        if(pause&&!SpectatorPause(out.paused))return false;
        bundleReport.spectatorFailure=2;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,spectator,spectatorSlot)||
           !SpectatorTerminal(spectatorSlot,human)||!cfgescape::Terminal(human))return false;
        // A retained old SpectatorPawn must never take camera ownership back
        // from a living native respawn/Tommy, even if terminal state lags.
        if(possessed&&possessed!=spectator){cfgescape::State current;
            if(!cfgescape::Counselor(r_,context.objectArray,plan.module,possessed,bundleCounselorClass,current)||!cfgescape::Terminal(current))return false;}
        bundleReport.spectatorFailure=4;
        cfgnames::Observation<Reader> names(r_,plan.module);
        if(!names.Decode(phase,text)||text.View()!=u"InProgress"||
            !SpectatorIdentity(plan.world,out.world)||!SpectatorIdentity(plan.controller,out.controller)||
            !r_.Read(plan.controller+cfgspectator::disk::SpectatingPlayer,state))return false;
        // A stale native selected pawn is an observation, not an adopted
        // target. Runtime restores a living target if it is absent from roster.
        if(state&&!SpectatorIdentity(state,out.nativeSelection))out.nativeSelection={state,0};
        out.offline=true;out.localOwned=true;out.gameThread=true;out.inProgress=true;
        out.humanTerminal=true;out.spectatorPawn=true;
        bundleReport.spectatorFailure=6;
        const bool okay=cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,possessed)&&
            cfgprimary::LookupSafe(r_,plan.module)&&cfgprimary::RetainedOfflineWorld(r_,plan,context);
        if(okay){
            if(!r_.Read(plan.controller+cfgprobe::ControllerSpectatorPawn,spectatorNative_)||
               !r_.Read(spectator+cfgprobe::ObjectClass,spectatorClass_))return false;
            spectatorPossessed_=possessed;spectatorPhase_=phase;spectatorSlotProof_=spectatorSlot;spectatorSafetyProof_=out;spectatorSafetyProved_=true;
            if(!SpectatorSafetyLinks())return false;
            bundleReport.spectatorFailure=0;
        }return okay;
    }
    bool SpectatorLocalOwner(){
        cfgobserver::LocalPlayers list{};
        return r_.Read(plan.gameInstance+cfgprobe::GameInstanceLocalPlayers,list)&&
            list.data==plan.localPlayerData&&list.count==1&&list.capacity>=1&&list.capacity<=1024&&
            cfgobserver::Same(r_,list.data,plan.player)&&cfgobserver::Same(r_,plan.player+cfgprobe::PlayerController,plan.controller)&&
            cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPlayer,plan.player);
    }
    // Hosted-only metadata seed for the shared J2.19 label consumer. It is a
    // typed local HUD hint; it grants no camera or native-call authority.
    bool HostedSpectatorLabelHint(){
#if defined(CFG_H1_JASON) && !defined(CFG_SPECTATOR_MEMBER_FIXTURE_ONLY)
        if(!HostedContext(false))return false;
        std::uintptr_t widget=0,status=0;cfgnames::Words name{};
        if(!NativeLabelWidget(widget)||!r_.Read(plan.module+cfgspectator::disk::StatusFunctionCache,status)||
           !cfgprobe::ObjectPointer(status)||!r_.Read(status+cfgnames::disk::ObjectName,name))return false;
        cfghostedlabel::Hint hint{};
        if(!cfghostedlabel::Publish({true,true,true,cfgprimary::LookupVetoClearForObservation(r_,plan.module),widget,status},hint))return false;
        bundleSpectatorStatusFunction=hint.function;bundleSpectatorStatusWidgetHint=hint.widget;
        bundleSpectatorStatusName=name;bundleSpectatorStatusNameReady=true;return true;
#else
        return false;
#endif
    }
    bool Abandon(){
        if(!bundleSpectatorInstalled.object)return true;
        if(!Context()||!SpectatorLocalOwner()||!cfgjason::SameSlot(r_,context.objectArray,plan.objects[2]))return false;
        std::uintptr_t state=0;
        if(!r_.Read(plan.controller+cfgspectator::disk::SpectatingPlayer,state))return false;
        // If native Next/Previous already moved elsewhere, this override no
        // longer owns the field. Never erase another native selection.
        if(state==bundleSpectatorInstalled.object&&!SpectatorSelect(state,0))return false;
        bundleSpectatorInstalled={};return true;
    }
    bool SpectatorCachedLiving(unsigned index){
        auto& t=bundleSpectatorRoster[index];if(!cfgspectator::Usable(t))return false;
        cfgspectator::Identity pawn{},state{};std::uint8_t escaped=1,dead=1,stateEscaped=1;float health=0;std::uintptr_t controller=0;
        if(!SpectatorIdentity(t.pawn.object,pawn)||pawn!=t.pawn||
            !cfgobserver::Same(r_,t.pawn.object+cfgprobe::ObjectClass,bundleSpectatorPawnClasses[index])||
            !cfgobserver::Same(r_,t.state.object+cfgprobe::ObjectClass,bundleSpectatorStateClasses[index])||
            !cfgobserver::Same(r_,t.pawn.object+cfgescape::disk::PlayerState,t.state.object)||
            !r_.Read(t.pawn.object+cfgescape::disk::bHasEscaped,escaped)||escaped||
            !r_.Read(t.pawn.object+cfghunter::disk::Health,health)||!std::isfinite(health)||health<=cfghunter::disk::AliveThreshold||
            !r_.Read(t.state.object+cfgescape::disk::bDead,dead)||dead||
            !r_.Read(t.state.object+cfgescape::disk::bEscaped,stateEscaped)||stateEscaped||
            !r_.Read(t.pawn.object+cfgescape::disk::Controller,controller)||!controller||
            // OwnedPlayerState below repeats these reciprocal controller links.
            !cfgspectator::OwnedPlayerState(r_,context.objectArray,t.pawn.object,controller,t.state.object,spawnAssets.resources.classes[cfgspawn::PlayerState],state)||state!=t.state)return false;
        return true;
    }
    bool Observe(cfgspectator::Snapshot& out){
        if(!SpectatorSafety(out)||out.paused)return false;
        cfgspawn::PlayerArray list{},fresh{};
        std::array<bool,cfgspectator::MaximumCounselors> livingProved{};
        if(!r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,list)||list.count<0||list.count>16||
            list.capacity<list.count||list.capacity>32||(list.count&&!cfgprobe::ObjectPointer(list.data)))return false;
        if(list.data!=bundleSpectatorList.data||list.count!=bundleSpectatorList.count||list.capacity!=bundleSpectatorList.capacity){
            bundleSpectatorList=list;bundleSpectatorRoster={};bundleSpectatorCursor=0;bundleSpectatorRosterReady=false;
        }
        // At most two native roster entries acquire full type/lifetime proof
        // per camera slice. Warm entries only need exact identities, classes,
        // ownership and terminal flags. This includes native Tommy, not only
        // the helpers spawned by this mod; no world or global-object scan.
        for(unsigned n=0;n<2&&list.count;++n){
            const auto i=bundleSpectatorCursor;std::uintptr_t pawn=0;cfgspectator::Target target{};
            if(!r_.Read(list.data+8*i,pawn))return false;
            const bool warm=pawn&&bundleSpectatorRoster[i].pawn.object==pawn&&SpectatorCachedLiving(i);
            livingProved[i]=warm;
            if(warm)target=bundleSpectatorRoster[i];
            if(pawn&&pawn!=bundleJason.pawn.object&&(warm||SpectatorTarget(pawn,false,target))){
                bundleSpectatorRoster[i]=target;
                if(!r_.Read(pawn+cfgprobe::ObjectClass,bundleSpectatorPawnClasses[i])||
                    !r_.Read(target.state.object+cfgprobe::ObjectClass,bundleSpectatorStateClasses[i]))return false;
            }else bundleSpectatorRoster[i]={};
            if(!cfgobserver::Same(r_,list.data+8*i,pawn))return false;
            if(++bundleSpectatorCursor>=static_cast<unsigned>(list.count)){bundleSpectatorCursor=0;bundleSpectatorRosterReady=true;}
        }
        if(!r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,fresh)||
            fresh.data!=list.data||fresh.count!=list.count||fresh.capacity!=list.capacity||!bundleSpectatorRosterReady)return false;
        for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){
            if(bundleSpectatorRoster[i].pawn.object&&!cfgobserver::Same(r_,list.data+8*i,bundleSpectatorRoster[i].pawn.object))continue;
            // No native call intervenes between the warm proof and this loop.
            // Reuse only this callback's exact roster entry, never a retained
            // proof from a previous slice. Publication still repeats authority,
            // target and roster checks after ReadPose's native call.
            if(!livingProved[i]&&!SpectatorCachedLiving(i))continue;
            bool duplicate=false;for(unsigned j=0;j<out.count;++j)duplicate|=out.counselors[j].pawn==bundleSpectatorRoster[i].pawn;
            if(!duplicate)out.counselors[out.count++]=bundleSpectatorRoster[i];
        }
        bundleReport.spectatorFailure=7;
        if(!Jason()||!SpectatorTarget(bundleJason.pawn.object,true,out.jason))return false;
        // Disk property class is SCCharacter, not SCPlayerState. Translate a
        // fresh pawn identity to donor policy state; unknown/serial0 refuses.
        cfgspectator::ResolveNativeSelection(out);
        // CFG may cycle the attached native camera while SpectatingPlayer is
        // null or stale. Observe the active stock view on every bounded slice;
        // only identities in the freshly proved native roster can resolve.
        SpectatorViewObservation(out);
        bundleReport.spectatorFailure=0;
        out.cycleEvent=bundleSpectatorCycleEvents;
        bundleReport.spectatorObservedState=out.observedState.object;bundleReport.spectatorRosterCount=out.count;
        bundleReport.spectatorNativePawn=out.nativeSelection.object;
        bundleReport.spectatorRosterStates={};
        for(unsigned i=0;i<out.count;++i)bundleReport.spectatorRosterStates[i]=out.counselors[i].state.object;
        if(!SpectatorSafetyLinks()){bundleReport.spectatorFailure=6;return false;}return true;
    }
    bool SpectatorViewObservation(cfgspectator::Snapshot& out,std::uintptr_t* viewedOut=nullptr){
        using namespace cfgspectatorview::disk;
        std::uintptr_t camera=0,type=0,constructor=0,active=0,pending=0,attached=0;cfgprobe::lifetime::SlotObservation before{},after{};
        if(!r_.Read(plan.controller+ControllerCamera,camera)||!camera||
           !r_.Read(plan.module+cfgcamera::disk::CameraClassCache,type)||
           !r_.Read(plan.module+cfgcamera::disk::CameraConstructorCache,constructor)||!cfgprobe::MatchingClassCache(type,constructor)||
           !cfgspawn::ObjectIs(r_,camera,type)||!cfgspawn::ObjectSlot(r_,context.objectArray,camera,before)||
           !cfgobserver::Same(r_,camera+PCOwner,plan.controller)||
           !cfgbundle::Code(r_,plan.module,Getter,GetterCode)||
           !r_.Read(camera+Pending,pending)||!r_.Read(camera+Active,active))return false;
        const auto viewed=pending?pending:active;
        // CFG keeps the stock SCSpectatorPawn as the camera target while its
        // attached Viewing actor changes on the native Next/Previous cycle.
        // The pawn identity itself is not evidence that Jason is selected.
        if(viewed==out.spectator.object&&!r_.Read(viewed+0x3F8,attached))return false;
        // State identity comes only from the ALREADY proved native roster, not
        // a display-name match or adoption of an arbitrary camera target.
        cfgspectator::Identity state{};
        for(unsigned i=0;i<out.count;++i)if(viewed==out.counselors[i].pawn.object||viewed==out.counselors[i].state.object)
            state=out.counselors[i].state;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,camera,after)||!(before==after)||
           !cfgobserver::Same(r_,plan.controller+ControllerCamera,camera)||
           !cfgobserver::Same(r_,camera+PCOwner,plan.controller)||
           !cfgobserver::Same(r_,camera+Pending,pending)||!cfgobserver::Same(r_,camera+Active,active))return false;
        if(viewed==out.spectator.object&&!cfgobserver::Same(r_,viewed+0x3F8,attached))return false;
        // A positively proved native camp/other camera wins over our stale
        // SpectatingPlayer selection too. Never reassert follow over that view.
        if(viewed==out.spectator.object){
            if(cfgspectator::ResolveAttachedSelection(out,attached))state=out.observedState;
        }else if(viewed==out.jason.pawn.object)state=out.jason.state;
        if(viewed)out.observedState=state;
        if(viewedOut)*viewedOut=viewed;
        return true;
    }
    bool SpectatorRosterCurrent(const cfgspectator::Snapshot& expected){
        if(!expected.count||expected.count>cfgspectator::MaximumCounselors)return false;
        cfgspawn::PlayerArray list{};
        if(!r_.Read(spawnAssets.gameState+cfgescape::disk::PlayerCharacterList,list)||
           list.data!=bundleSpectatorList.data||list.count!=bundleSpectatorList.count||list.capacity!=bundleSpectatorList.capacity||
           list.count<1||list.count>static_cast<int>(cfgspectator::MaximumCounselors))return false;
        for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){
            const auto& cached=bundleSpectatorRoster[i];
            for(unsigned n=0;n<expected.count;++n){
                if(cached.pawn!=expected.counselors[n].pawn||cached.state!=expected.counselors[n].state)continue;
                if(cfgobserver::Same(r_,list.data+8*i,cached.pawn.object)&&SpectatorCachedLiving(i))return true;
            }
        }
        return false; // no camera RPC if the last native counselor retired after Observe
    }
    bool SpectatorCurrent(const cfgspectator::Snapshot& expected,const cfgspectator::Target& target,bool killer){
        cfgspectator::Snapshot fresh{};cfgspectator::Target current{};
        return SpectatorSafety(fresh)&&!fresh.paused&&fresh.world==expected.world&&fresh.controller==expected.controller&&
            fresh.spectator==expected.spectator&&fresh.nativeSelection==expected.nativeSelection&&SpectatorTarget(target.pawn.object,killer,current)&&
            current.pawn==target.pawn&&current.state==target.state&&(!killer||SpectatorRosterCurrent(expected));
    }
    bool ReadPose(const cfgspectator::Snapshot&,const cfgspectator::Target& target,cfgspectator::Vector& base,
        float& yaw,cfgspectator::Rotation& look,bool& hasLook){
        cfgspawn::Vec3 p{};std::uintptr_t function=0,owner=0;
        bundleReport.spectatorFailure=8;
        hasLook=false;yaw=0;
        if(target.pawn.object!=bundleJason.pawn.object||!Location(target.pawn.object,p))return false;
        base={p.x,p.y,p.z};
        using namespace cfgspectator::disk;
        if(!FeatureFunction(GetControlRotation,function,owner)||!cfgspawn::ObjectIs(r_,plan.controller,owner)||
            !cfgobserver::Same(r_,function+0xB0,plan.module+GetControlRotationExecutor)||
            !cfgbundle::Code(r_,plan.module,GetControlRotationExecutor,GetControlRotationExecutorCode)||
            !Invoke(plan.controller,function,&look)||!cfgspectator::Finite(look))return false;
        yaw=look.yaw;hasLook=bundleSpectator.Following();bundleReport.spectatorFailure=0;return true;
    }
    bool SpectatorSelect(std::uintptr_t expected,std::uintptr_t pawn)noexcept{
        if(GetCurrentThreadId()!=plan.threadId)return false;
        // Only the validated local spectator controller field. No PlayerState
        // construction, actor flag edits or writes to AI/human pawns.
        __try{return InterlockedCompareExchangePointer(reinterpret_cast<void* volatile*>(plan.controller+
            cfgspectator::disk::SpectatingPlayer),reinterpret_cast<void*>(pawn),reinterpret_cast<void*>(expected))==reinterpret_cast<void*>(expected);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    template<class B,std::size_t N> bool SpectatorResource(unsigned id,const B& binding,std::uintptr_t executor,
        const std::array<unsigned char,N>& code,std::uintptr_t& function,std::uintptr_t& owner){
        // Record the exact existing guard/read. Do not issue a second diagnostic
        // walk after failure: native fields may change and budgets are finite.
        bundleReport.spectatorResourceBinding=id;bundleReport.spectatorResourceFailure=2;
        bundleReport.spectatorNativeBytes=0;std::uint16_t extent=0;
        const bool ready=FeatureFunction(binding,function,owner,&extent);
        bundleReport.spectatorNativeBytes=extent;if(!ready)return false;
        bundleReport.spectatorResourceFailure=3;if(!cfgspawn::ObjectIs(r_,plan.controller,owner))return false;
        bundleReport.spectatorResourceFailure=4;if(!cfgobserver::Same(r_,function+0xB0,plan.module+executor))return false;
        bundleReport.spectatorResourceFailure=5;if(!cfgbundle::Code(r_,plan.module,executor,code))return false;
        bundleReport.spectatorResourceFailure=0;return true;
    }
    bool spectatorPublicationDeferred_=false;
    bool CameraPublicationDeferred()const{return spectatorPublicationDeferred_;}
    bool SpectatorStockView(const cfgspectator::Snapshot& s){
        auto observed=s;std::uintptr_t viewed=0;
        return SpectatorViewObservation(observed,&viewed)&&viewed==s.spectator.object;
    }
    bool Publish(const cfgspectator::Snapshot& s,const cfgspectator::Target& target,
        const cfgspectator::CameraFrame& frame,bool selecting){
        using namespace cfgspectator::disk;
        spectatorPublicationDeferred_=false;
        bundleReport.spectatorFailure=9;
        bundleReport.spectatorResourceBinding=0;bundleReport.spectatorResourceFailure=1;bundleReport.spectatorNativeBytes=0;
        std::uintptr_t view=0,viewOwner=0;
        if(!cfgspectator::Finite(frame.location)||!cfgspectator::Finite(frame.rotation))return false;
        // CFG's stock SCSpectatorPawn Tick attaches its root to SpectatingPlayer
        // and writes a killer-relative offset. The stock spring arm supplies
        // camera distance/collision. Teleporting this root to the DONOR CAMERA
        // EYE fights that native writer and applies a second chase distance.
        // Keep the native follow; no RPC pose, root teleport, arm edits or Tick hook.
        if(selecting&&!SpectatorResource(3,SetViewTargetWithBlend,SetViewTargetWithBlendExecutor,SetViewTargetWithBlendExecutorCode,view,viewOwner))return false;
        if(!SpectatorCurrent(s,target,true)){bundleReport.spectatorFailure=10;return false;}
        if(!selecting){
            // Read-only maintenance never reasserts over a native camp/cycle,
            // respawn, pause or world change. Success means ownership checked,
            // NOT that a custom pose was dispatched or a native Tick completed.
            bundleReport.spectatorFailure=35;
            if(!SpectatorStockView(s))return false;
            bundleReport.spectatorFailure=0;return true;
        }
        if(r_.reads>cfgbundle::ReadCap-110||calls_>cfgbundle::CallCap-1){
            bundleReport.spectatorFailure=10;return false;
        }
        bundleReport.spectatorFailure=11;
        if(!SpectatorSelect(s.nativeSelection.object,target.pawn.object))return false;
        bundleSpectatorInstalled=target.pawn;
        // Preserve the player's existing native spectator look. This is the
        // ONLY native follow handoff; no rotation seed or intervening RPC.
        // The complete SpectatorCurrent proof above is still same-call fresh.
        bundleReport.spectatorFailure=33;
        if(!cfgobserver::Same(r_,plan.controller+SpectatingPlayer,target.pawn.object))return false;
        cfgspectator::RestoreFrame blend;blend.target=s.spectator.object;
        bundleReport.spectatorFailure=34;
        if(!Invoke(plan.controller,view,&blend))return false;
        bundleReport.spectatorFailure=35;
        // Read back the handoff; no mutation follows this native call. The
        // next callback reacquires full terminal/roster authority before any
        // future operation. Never spend another cold terminal walk merely to
        // label a successful native view handoff as a custom pose dispatch.
        cfgspectator::Identity spectator{};cfgobserver::Plan freshWorld;
        if(!cfgprimary::LookupSafe(r_,plan.module)||!SpectatorLocalOwner()||
           !cfgprimary::RetainedOfflineWorld(r_,plan,freshWorld)||
           !SpectatorIdentity(s.spectator.object,spectator)||spectator!=s.spectator||
           !cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerPawn,spectatorPossessed_)||
           !cfgobserver::Same(r_,plan.controller+cfgprobe::ControllerSpectatorPawn,spectatorNative_)||
           !cfgobserver::Same(r_,plan.controller+SpectatingPlayer,target.pawn.object)||
           !SpectatorStockView(s))return false;
        bundleReport.spectatorFailure=0;return true;
    }
    bool RestoreCounselor(const cfgspectator::Snapshot& s,const cfgspectator::Target& target){
        using namespace cfgspectator::disk;
        bundleReport.spectatorFailure=14;
        struct Blend {std::uintptr_t target=0;float time=0;std::uint8_t function=0;unsigned char pad[3]{};float exponent=0;
            std::uint8_t lock=0;unsigned char tail[3]{};} packet;
        static_assert(sizeof(Blend)==24&&offsetof(Blend,lock)==20,"native21/padded24 view target frame");
        std::uintptr_t function=0,owner=0;
        if(!SpectatorResource(3,SetViewTargetWithBlend,SetViewTargetWithBlendExecutor,SetViewTargetWithBlendExecutorCode,function,owner)||
            !SpectatorCurrent(s,target,false)||!SpectatorSelect(s.nativeSelection.object,target.pawn.object))return false;
        bundleSpectatorInstalled={};packet.target=target.pawn.object;const bool okay=Invoke(plan.controller,function,&packet);if(okay)bundleReport.spectatorFailure=0;return okay;
    }
