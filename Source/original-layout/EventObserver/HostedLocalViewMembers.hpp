// H2.48 hosted host local-view natives. Included INSIDE BundleAccess right after HostedJasonMembers.hpp.
// Transcribes donor rc1 OfflineSetup.cpp:9773-10085 through cfghostview::Refresh. Every native is a UFunction exec thunk
// reached through UObject::ProcessEvent exactly as the donor does (SafeProcessEventCall); each step re-proves, read-only:
//   owner class cache pair, factory code, UFunction cache slot + frame size, function.Outer, function.Exec == pinned thunk,
//   the complete thunk bytes (cfgbundle::Code), receiver type, then runs the same retained-call gateway the hosted births use
//   (HostedPrepare roots + BeginBundleNative, raw SEH dispatch, FinishBundleNative). A failed/ambiguous dispatch poisons the
//   retention lease (BundleRetentionFailed) and is never replayed. View calls are a fixed <=9-call sequence and are charged
//   to hostedViewCalls_, not calls_, so the takeover rollback keeps its own 4-call budget (same precedent as HasFullyTraveled).
#ifdef CFG_H1_JASON
#if CFG_H247_HOST_TAKEOVER
    struct HostedViewFn {std::uintptr_t function=0,owner=0;};
    unsigned hostedViewCalls_=0;
    unsigned hostedViewFailure_=0,hostedViewResolveStep_=0;
    unsigned HostedViewFailureCode()const noexcept{return hostedViewFailure_*10u+hostedViewResolveStep_;}
    // H2.52c diagnostics: refresh (LocalView stage) failure = 1,000,000 + 100000*hudSub + 1000*viewCalls + 100*why + resolveStep.
    // why: 1 unpinned,2 restart,3 reset input,4 view target,5 acknowledged,6 lens,7 HUD,8 game input.
    // hudSub (why 7): 1 facts,2 dispatch,3 null result,4 retain/world,5 result class/owner,6 after facts/top.
    unsigned hostedViewHudSub_=0,hostedViewRefreshWhy_=0;
    unsigned HostedViewRefreshCode()const noexcept{
        return 1000000u+100000u*hostedViewHudSub_+1000u*(hostedViewCalls_>9u?9u:hostedViewCalls_)+100u*hostedViewRefreshWhy_+hostedViewResolveStep_;}
    template<std::size_t N> bool HostedViewResolve(const cfgvehicle::disk::Binding& b,std::uintptr_t executor,
        const std::array<unsigned char,N>& code,HostedViewFn& out){
        out={};std::uintptr_t reflection=0,constructor=0,function=0,owner=0;std::uint16_t bytes=0;
        cfgprobe::lifetime::SlotObservation ownerSlot{},functionSlot{};
        if(!r_.Read(plan.module+b.ownerReflection,reflection)||!r_.Read(plan.module+b.ownerConstructor,constructor)||
           !(owner=cfgprobe::MatchingClassCache(reflection,constructor))||!cfgbundle::Code(r_,plan.module,b.factory,b.code)||
           !r_.Read(plan.module+b.cache,function)||!cfgprobe::ObjectPointer(function)||        // cache must already be populated: read-only, no factory call
           !cfgspawn::ObjectSlot(r_,context.objectArray,owner,ownerSlot)||!cfgspawn::ObjectSlot(r_,context.objectArray,function,functionSlot)||
           !r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||!cfgbundle::NativeFrame(bytes,b.nativeBytes,b.storageBytes)||
           !cfgobserver::Same(r_,function+0x20,owner)||!cfgobserver::Same(r_,function+0xB0,plan.module+executor)||
           !cfgbundle::Code(r_,plan.module,executor,code))return false;
        out={function,owner};return true;
    }
    struct HostedViewFns {HostedViewFn restart,clear,reset,ignored,view,change,gameInput;std::uintptr_t library=0;};
    // Read-only facts about the local controller + HUD. counselor==0 before the restart (Ready); the proven counselor after.
    struct HostedViewHud {std::uintptr_t hud=0,roleClass=0,top=0,topClass=0;};
    bool HostedViewHudFacts(std::uintptr_t controller,const HostedViewFn& change,HostedViewHud& out){
        out={};std::uintptr_t hudType=0,widgetType=0,owner=0,stack=0;cfgmenu::active::StackArray records{};
        namespace v=cfghostview::disk;
        // H2.51 diagnostics: hostedViewResolveStep_ = the first failing HUD fact (same checks, same order):
        // 11 HUD/widget classes, 12 controller HUD, 13 HUD type, 14 HUD owner, 15 counselor HUD class, 16 widget class, 17 stack, 18 top widget.
        hostedViewResolveStep_=11;
        if(!cfghosted::native::Class(r_,plan.module,v::SCInGameHUDReflection,v::SCInGameHUDConstructor,hudType)||
           !cfghosted::native::Class(r_,plan.module,v::SCHUDWidgetReflection,v::SCHUDWidgetConstructor,widgetType))return false;
        hostedViewResolveStep_=12;if(!r_.Read(controller+cfgmenu::execdisk::ControllerHUD,out.hud)||!cfgprobe::ObjectPointer(out.hud))return false;
        hostedViewResolveStep_=13;if(!cfgspawn::ObjectIs(r_,out.hud,hudType)||!cfgspawn::ObjectIs(r_,out.hud,change.owner))return false;
        hostedViewResolveStep_=14;if(!r_.Read(out.hud+0x368,owner)||owner!=controller)return false; // AHUD::PlayerOwner
        hostedViewResolveStep_=15;if(!r_.Read(out.hud+v::CounselorHUDClass,out.roleClass)||!cfgprobe::ObjectPointer(out.roleClass))return false;
        hostedViewResolveStep_=16;if(!cfgspawn::ClassContains(r_,out.roleClass,widgetType))return false;
        hostedViewResolveStep_=17;if(!r_.Read(out.hud+cfgmenu::execdisk::HUDStack,stack))return false;
#if CFG_H252_EMPTY_MENU_STACK
        // H2.52: donor reads the current menu via GetCurrentMenu, which is null when the HUD has no menu
        // stack or an empty one (live H2.51: Failure=47 while the host played Jason). Same here: top=0 means
        // "no current menu", so RootHud always rebuilds. A present stack whose array is malformed still refuses (19).
        if(!stack){hostedViewResolveStep_=0;return true;}
        hostedViewResolveStep_=19;if(!cfgprobe::ObjectPointer(stack)||!r_.Read(stack+cfgmenu::execdisk::StackArray,records))return false;
        if(records.count==0&&records.capacity>=0&&records.capacity<=64){hostedViewResolveStep_=0;return true;}
        if(!records.Sane())return false;
#else
        if(!cfgprobe::ObjectPointer(stack)||!r_.Read(stack+cfgmenu::execdisk::StackArray,records)||!records.Sane())return false;
#endif
        hostedViewResolveStep_=18;
        if(!r_.Read(records.data+static_cast<std::uintptr_t>(records.count-1)*cfgmenu::execdisk::RecordBytes+cfgmenu::execdisk::RecordWidget,out.top)||
           !cfgprobe::ObjectPointer(out.top)||!r_.Read(out.top+cfgprobe::ObjectClass,out.topClass))return false;
        hostedViewResolveStep_=0;return true;
    }
    bool HostedViewReceiver(std::uintptr_t controller,std::uintptr_t counselor,const HostedViewFn& fn){
        const auto& s=hostedTx.snap;
        if(!controller||controller!=s.localController||!cfgspawn::ObjectIs(r_,controller,fn.owner))return false;
        return !counselor||cfgobserver::Same(r_,controller+cfgprobe::ControllerPawn,counselor); // donor ownership check controller+0x370==pawn
    }
    bool HostedViewResolveAll(HostedViewFns& f){
        namespace v=cfghostview::disk;f={};
        hostedViewResolveStep_=9;if(!HostedWorld())return false;
        std::uintptr_t library=0;
        // H2.50c diagnostics: hostedViewResolveStep_ names the first function that failed (same order, same checks).
        hostedViewResolveStep_=1;if(!HostedViewResolve(v::ClientRestart,v::ClientRestartExecutor,v::ClientRestartExecutorCode,f.restart))return false;
        hostedViewResolveStep_=2;if(!HostedViewResolve(v::ClientClearCameraLensEffects,v::ClientClearCameraLensEffectsExecutor,v::ClientClearCameraLensEffectsExecutorCode,f.clear))return false;
        hostedViewResolveStep_=3;if(!HostedViewResolve(v::ResetIgnoreInputFlags,v::ResetIgnoreInputFlagsExecutor,v::ResetIgnoreInputFlagsExecutorCode,f.reset))return false;
        hostedViewResolveStep_=4;if(!HostedViewResolve(v::IsMoveInputIgnored,v::IsMoveInputIgnoredExecutor,v::IsMoveInputIgnoredExecutorCode,f.ignored))return false;
        hostedViewResolveStep_=5;if(!HostedViewResolve(cfgspectator::disk::SetViewTargetWithBlend,cfgspectator::disk::SetViewTargetWithBlendExecutor,
               cfgspectator::disk::SetViewTargetWithBlendExecutorCode,f.view))return false;
        hostedViewResolveStep_=6;if(!HostedViewResolve(v::ChangeRootMenu,v::ChangeRootMenuExecutor,v::ChangeRootMenuExecutorCode,f.change))return false;
        hostedViewResolveStep_=7;if(!HostedViewResolve(v::SetInputMode_GameOnly,v::SetInputMode_GameOnlyExecutor,v::SetInputMode_GameOnlyExecutorCode,f.gameInput))return false;
        hostedViewResolveStep_=8;if(!cfgbundle::Code(r_,plan.module,v::SetInputMode_GameOnlyImpl,v::SetInputMode_GameOnlyImplCode)||
           !r_.Read(f.gameInput.owner+cfgspawn::ClassDefaultObject,library)||!cfgprobe::ObjectPointer(library)||
           !cfgspawn::ObjectIs(r_,library,f.gameInput.owner))return false;
        hostedViewResolveStep_=0;f.library=library;return true;
    }
    // Read-only: no native call, no write, no factory call. false on any pin/schema/typing mismatch.
    bool HostedLocalViewReady(){
        HostedViewFns f;HostedViewHud hud;const auto controller=hostedTx.snap.localController;
        hostedViewFailure_=1;if(!HostedViewResolveAll(f))return false;
        hostedViewFailure_=2;if(!HostedViewReceiver(controller,0,f.restart)||!HostedViewReceiver(controller,0,f.reset))return false;
        hostedViewFailure_=3;if(!HostedViewHudFacts(controller,f.change,hud))return false;
        hostedViewFailure_=0;return true;
    }
    bool HostedViewDispatch(const HostedViewFn& fn,std::uintptr_t receiver,void* packet,unsigned outputs,
        std::uintptr_t a=0,std::uintptr_t b=0,std::uintptr_t c=0){
        const auto& s=hostedTx.snap;
        if(hostedViewCalls_>=cfgbundle::CallCap)return false;
        if(!HostedPrepare(std::array<std::uintptr_t,14>{plan.world,plan.mode,hostedTx.gameState,s.jason,s.human,s.humanState,
            hostedTx.birth.controller,hostedTx.birth.state,receiver,fn.function,fn.owner,a,b,c},outputs))return false;
        ++hostedViewCalls_;
        if(!HostedLeafProcessEvent(plan.module+cfgspawn::GenericDispatcherRva,receiver,fn.function,packet)){BundleRetentionFailed();return false;}
        return outputs?true:FinishBundleNative();
    }
    // UserWidget::GetOwningPlayer virtual (pinned slot + body), same proof ObjectiveWidgetOwner uses; result must be the local controller.
    bool HostedViewWidgetOwner(std::uintptr_t widget,std::uintptr_t controller){
        std::uintptr_t base=0,table=0,getter=0,owner=0;
        if(!cfgobjectivepanel::NativeClass(r_,plan.module,cfgmenu::widgetowner::ClassCache,cfgmenu::widgetowner::ClassConstructor,u"UserWidget",base)||
           !cfgspawn::ObjectIs(r_,widget,base)||!r_.Read(widget,table)||!cfgprobe::ObjectPointer(table)||
           !r_.Read(table+cfgmenu::widgetowner::VirtualSlot,getter)||getter!=plan.module+cfgmenu::widgetowner::VirtualGetter||
           !cfgbundle::Code(r_,plan.module,cfgmenu::widgetowner::VirtualGetter,cfgmenu::widgetowner::VirtualCode)||
           hostedViewCalls_>=cfgbundle::CallCap||!HostedPrepare(HostedRoots(widget,controller),0))return false;
        ++hostedViewCalls_;
        if(!HostedLeafOwnerGetter(plan.module+cfgmenu::widgetowner::VirtualGetter,widget,owner)){BundleRetentionFailed();return false;}
        return FinishBundleNative()&&owner==controller;
    }
    unsigned hostedViewHudTop_=0,hostedViewHudRebuilt_=0;   // H2.53 diagnostics: 0 no menu,1 counselor root,2 other root; 1 = ChangeRootMenu sent
    cfghostview::Result hostedViewLast_{};
    bool HostedLocalViewRefresh(const cfghostedtx::Snapshot& s,std::uintptr_t counselor,bool gameInput=true){
        struct Steps {
            BundleAccess& a;const cfghostedtx::Snapshot& snap;std::uintptr_t counselor;HostedViewFns f{};
            bool Pinned(){
                if(!a.HostedViewResolveAll(f))return false;
                HostedViewHud hud;
                return a.HostedViewReceiver(snap.localController,counselor,f.restart)&&a.HostedViewReceiver(snap.localController,counselor,f.reset)&&
                    a.HostedViewHudFacts(snap.localController,f.change,hud);
            }
            bool Fresh(const HostedViewFn& fn){return a.HostedWorld()&&a.HostedViewReceiver(snap.localController,counselor,fn);}
            // H2.52d: ChangeRootMenu (owner = HUD class) and SetInputMode_GameOnly (owner = widget library) are not dispatched on the
            // controller, so Fresh(fn) could never pass for them (live H2.52c: refresh code 1105700, HUD sub 1). Prove the local controller
            // and counselor ownership against the controller-owned restart function instead; the HUD and library receivers keep their own
            // type proofs (HostedViewHudFacts: hud is change.owner; GameInput: library is gameInput.owner).
            bool FreshController(){return a.HostedWorld()&&a.HostedViewReceiver(snap.localController,counselor,f.restart);}
            bool MoveIgnored(bool& ignored){
                struct {std::uint8_t value;unsigned char pad[7];} packet{};
                if(!Fresh(f.ignored)||!a.HostedViewDispatch(f.ignored,snap.localController,&packet,0,counselor))return false;
                ignored=packet.value!=0;return true;
            }
            bool ClientRestart(){struct {std::uintptr_t pawn;} packet{counselor};
                return Fresh(f.restart)&&a.HostedViewDispatch(f.restart,snap.localController,&packet,0,counselor);}
            bool ResetInput(){alignas(8) unsigned char packet[8]{};
                return Fresh(f.reset)&&a.HostedViewDispatch(f.reset,snap.localController,packet,0,counselor);}
            bool ViewTarget(){
                // Donor sends ClientSetViewTarget then SetViewTargetWithBlend and accepts either (clientApplied||localApplied).
                // ClientSetViewTarget's 16-byte FViewTargetTransitionParams input is outside the admitted retention frames, so
                // SetViewTargetWithBlend (admitted, same zero-blend frame, same call RestoreCounselor makes) is the donor-accepted path.
                struct {std::uintptr_t target;float time;std::uint8_t function;unsigned char pad[3];float exponent;std::uint8_t lock;unsigned char tail[3];} packet{};
                static_assert(sizeof(packet)==24&&offsetof(decltype(packet),lock)==20,"native21/padded24 view target frame");
                packet.target=counselor;
                return Fresh(f.view)&&a.HostedViewDispatch(f.view,snap.localController,&packet,0,counselor);
            }
            bool Acknowledged(){return a.HostedWorld()&&cfgobserver::Same(a.r_,snap.localController+cfgprobe::ControllerAcknowledgedPawn,counselor);}
            bool ClearLens(){alignas(8) unsigned char packet[8]{};
                return Fresh(f.clear)&&a.HostedViewDispatch(f.clear,snap.localController,packet,0,counselor);}
            bool RootHud(){
                HostedViewHud hud;auto& sub=a.hostedViewHudSub_;
                sub=1;if(!FreshController()||!a.HostedViewHudFacts(snap.localController,f.change,hud))return false;
                a.hostedViewHudTop_=!hud.top?0u:hud.topClass==hud.roleClass?1u:2u;
                // Donor: an already-correct root is left alone (a rebuild can reclaim controller input).
                if(hud.top&&hud.topClass==hud.roleClass&&a.HostedViewWidgetOwner(hud.top,snap.localController)){sub=0;return true;}
                struct {std::uintptr_t menu;std::uint8_t force;unsigned char pad[7];std::uintptr_t result;} packet{hud.roleClass,1,{},0};
                static_assert(sizeof(packet)==24,"ChangeRootMenu frame");
                sub=2;if(!FreshController()||!a.HostedViewDispatch(f.change,hud.hud,&packet,1,counselor,hud.roleClass))return false;
                a.hostedViewHudRebuilt_=1;
                sub=3;if(!packet.result){(void)a.FinishBundleNative();return false;}
                sub=4;if(!a.RetainBundleReturnedObject(packet.result)||!a.HostedWorld())return false;
                std::uintptr_t resultClass=0;HostedViewHud after;
                sub=5;if(!a.r_.Read(packet.result+cfgprobe::ObjectClass,resultClass)||resultClass!=hud.roleClass||
                    !a.HostedViewWidgetOwner(packet.result,snap.localController))return false;
                sub=6;if(!a.HostedViewHudFacts(snap.localController,f.change,after)||after.top!=packet.result)return false;
                sub=0;return true;
            }
            bool GameInput(){
                struct {std::uintptr_t controller;} packet{snap.localController};
                return FreshController()&&cfgspawn::ObjectIs(a.r_,f.library,f.gameInput.owner)&&a.HostedViewDispatch(f.gameInput,f.library,&packet,0,counselor);
            }
        } steps{*this,s,counselor};
        hostedViewCalls_=0;hostedViewHudSub_=0;hostedViewResolveStep_=0;hostedViewHudTop_=0;hostedViewHudRebuilt_=0;
        const auto result=cfghostview::Refresh(steps,gameInput);hostedViewLast_=result;
        hostedViewRefreshWhy_=static_cast<unsigned>(result.why);
        hostedViewFailure_=result.ok?0u:100u+static_cast<unsigned>(result.why);
        return result.ok;
    }
#if CFG_H253_HOST_VIEW_REPAIR
    // H2.53: transcribes donor PumpHostedCounselorView's two host repairs. The Hunt opening can republish the former Jason
    // camera/input/HUD after the takeover's first refresh, so the same pinned sequence runs again at +2 s and +20 s, with
    // SetInputMode_GameOnly on the last one only. Runs from the adopted controller's returned tick (same lease as
    // HostedRestoreClassAtEnd). Each repair is one shot; a failure is reported and never rolls back or replays. Every step
    // re-proves controller ownership of the counselor first, so a dead or replaced counselor produces no native call.
    // Published in H250HostLocalView Failure: 2000000 + 100000*repair(1,2) + 10000*movesIgnoredBefore + 1000*movesIgnoredAfter
    //   (0 unknown,1 no,2 yes) + 100*why (0 ok, see HostedViewRefreshCode; 9 abandoned) + 10*hudRebuilt + hudTopBefore (0 none,1 counselor,2 other).
    void HostedViewRepairArm(std::uintptr_t counselor){
        auto& t=hostedViewRepair;t={};const auto now=GetTickCount64();
        t.pending=true;t.world=plan.world;t.counselor=counselor;t.earlyAt=now+2000;t.finalAt=now+20000;t.deadline=now+30000;
    }
    void HostedViewRepairService(std::uint64_t now){
        auto& t=hostedViewRepair;const auto& s=hostedTx.snap;
        if(now>=t.deadline||t.world!=plan.world||!s.localController){t.pending=false;t.code=2000000u+100000u*t.done+900u;return;}
        const bool last=t.done==1;++t.done;if(last)t.pending=false;
        (void)HostedLocalViewRefresh(s,t.counselor,last);
        const auto& r=hostedViewLast_;
        t.code=2000000u+100000u*t.done+10000u*r.before+1000u*r.after+100u*static_cast<unsigned>(r.why)+10u*hostedViewHudRebuilt_+hostedViewHudTop_;
    }
#endif
#endif
#endif
