// Included inside BundleAccess; never serviced on menu/loading worker threads.
    bool ObjectiveStackAdmission(std::uint64_t now){
        ++bundleReport.objectiveStackChecks;
        std::uintptr_t hud=0,stack=0,top=0,hudClass=0,stackClass=0;cfgmenu::active::StackArray records;
        if(!ObjectiveScope()||!cfgprimary::LookupSafe(r_,plan.module)||
           !NativeClass(cfgsweater::huddisk::SCInGameHUDReflection,cfgsweater::huddisk::SCInGameHUDConstructor,hudClass)||
           !NativeClass(cfgmenu::parentdisk::Owner,cfgmenu::parentdisk::Constructor,stackClass)||
           !r_.Read(plan.controller+cfgmenu::execdisk::ControllerHUD,hud)||!cfgspawn::ObjectIs(r_,hud,hudClass)||
           !r_.Read(hud+cfgmenu::execdisk::HUDStack,stack)||!cfgspawn::ObjectIs(r_,stack,stackClass)||
           !r_.Read(stack+cfgmenu::execdisk::StackArray,records)||!records.Sane()||
           !r_.Read(records.data+static_cast<std::uintptr_t>(records.count-1)*cfgmenu::execdisk::RecordBytes+cfgmenu::execdisk::RecordWidget,top))return true;
        // No repeat walk of an unchanged stack. This paced direct chain is
        // separate from the finite global metadata survey and stops on adoption.
        if(records==bundleObjectiveStackLast&&top==bundleObjectiveStackTop)return true;
        bundleObjectiveStackLast=records;bundleObjectiveStackTop=top;++bundleObjectiveStackVisits;
        bundleReport.objectiveStackVisits=bundleObjectiveStackVisits;
        // The HUD root can be born after the original gameplay Plan. Use the
        // CURRENT registry header, not that plan's earlier object-count limit.
        cfgprobe::lifetime::ArrayHeader registry,fresh;
        cfgobjective::stack::Observation observed;
        if(!cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfgobjective::stack::Observe(r_,registry,plan.controller,hud,stack,bundleObjectives.RootClass(),observed)||
           !cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,fresh)||!(registry==fresh)||!ObjectiveScope()){
            // A read-budget deferral is not a stable negative observation.
            // Retry this same local stack within the existing128-visit cap.
            if(r_.reads>=r_.cap){bundleObjectiveStackLast={};bundleObjectiveStackTop=0;}
            return true;
        }
        struct Host{BundleAccess& a;bool Current(){return a.ObjectiveScope()&&cfgprimary::LookupSafe(a.r_,plan.module);}}host{*this};
        unsigned candidate=0;
        if(!bundleObjectives.EventCandidate(r_,plan.module,observed.widget.object,now,host,candidate)){
            // The existing per-instance1s/16-attempt gate may defer admission.
            // Do not permanently cache a live root merely because it is cold.
            bundleObjectiveStackLast={};bundleObjectiveStackTop=0;return true;
        }
        bundleObjectiveCandidate=candidate;bundleObjectivePanelBinding={};
        bundleObjectiveRepair=cfgobjective::repair::Runtime{};bundleObjectiveRepair.Configure(cfgobjective::repair::AllMask);
        bundleObjectiveRepairFailure=0;++bundleReport.objectiveStackAdmissions;return true;
    }
    bool ObjectiveWidgetEvent(std::uintptr_t object,std::uintptr_t function,std::uint64_t now){
        // Do not reopen an attempted/ambiguous native finder transaction, and
        // never replace an already owner-verified widget through this hint.
        if(bundleObjectiveRetiring||bundleObjectiveWidget.object||bundleObjectiveRepair.report.attempts||
           GetCurrentThreadId()!=plan.threadId)return false;
        struct Host{BundleAccess& a;bool Current(){return a.ObjectiveScope()&&cfgprimary::LookupSafe(a.r_,plan.module);}}host{*this};
        bundleObjectives.EventOwner(r_,plan.module,object,function,now,host);
        unsigned candidate=0;
        if(!bundleObjectives.EventCandidate(r_,plan.module,object,now,host,candidate))return false;
        if(candidate==bundleObjectiveCandidate&&!bundleObjectiveRepair.Finished())return false;
        bundleObjectiveCandidate=candidate;bundleObjectivePanelBinding={};
        // An ownerless preload can exhaust every20s phase before the actual
        // panel exists. Only an event-verified candidate grants a fresh window.
        // No call was attempted, so this cannot replay a native finder.
        bundleObjectiveRepair=cfgobjective::repair::Runtime{};
        bundleObjectiveRepair.Configure(cfgobjective::repair::AllMask);
        bundleObjectiveRepairFailure=0;return true;
    }
    bool ObjectiveOwnerNative(std::uintptr_t widget,std::uintptr_t& controller)noexcept{
        controller=0;if(!NativeCallAllowed())return false;++calls_;
        __try{controller=reinterpret_cast<std::uintptr_t(__fastcall*)(void*)>(plan.module+cfgmenu::widgetowner::VirtualGetter)(reinterpret_cast<void*>(widget));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    bool ObjectiveWidgetOwner(const cfgprobe::lifetime::SlotObservation& wanted){
        bundleObjectiveRepairFailure=1;
        if(!ObjectiveScope()||!cfgprimary::LookupSafe(r_,plan.module))return false;
        cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation widget,controller;
        std::uintptr_t type=0,base=0,table=0,getter=0,defaults=0,owner=0;
        bundleObjectiveRepairFailure=2;
        if(!wanted.HasAssignedSerial()||!cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfgspawn::ObjectSlot(r_,registry,wanted.object,widget)||!(widget==wanted)||
           !r_.Read(wanted.object+cfgprobe::ObjectClass,type)||type!=bundleObjectives.RootClass()||
           !r_.Read(type+cfgspawn::ClassDefaultObject,defaults)||defaults==wanted.object)return false;
        bundleObjectiveRepairFailure=3;
        if(!cfgobjectivepanel::NativeClass(r_,plan.module,cfgmenu::widgetowner::ClassCache,cfgmenu::widgetowner::ClassConstructor,u"UserWidget",base)||
           !cfgspawn::ObjectIs(r_,wanted.object,base)||!r_.Read(wanted.object,table)||!cfgprobe::ObjectPointer(table)||
           !r_.Read(table+cfgmenu::widgetowner::VirtualSlot,getter)||getter!=plan.module+cfgmenu::widgetowner::VirtualGetter||
           !cfgbundle::Code(r_,plan.module,cfgmenu::widgetowner::VirtualGetter,cfgmenu::widgetowner::VirtualCode))return false;
        bundleObjectiveRepairFailure=4;
        bundleReport.objectiveOwnerWidget=wanted.object;bundleReport.objectiveOwnerExpected=plan.controller;
        bundleReport.objectiveOwnerResult=0;bundleReport.objectiveOwnerFailure=1;
        if(!ObjectiveOwnerNative(wanted.object,owner))return false;
        bundleReport.objectiveOwnerResult=owner;bundleReport.objectiveOwnerFailure=2;
        if(owner!=plan.controller)return false;
        bundleReport.objectiveOwnerFailure=3;
        if(!cfgspawn::ObjectSlot(r_,registry,owner,controller)||!(controller==plan.objects[2]))return false;
        bundleReport.objectiveOwnerFailure=4;
        if(!cfgspawn::ObjectSlot(r_,registry,wanted.object,widget)||!(widget==wanted)||
           !cfgobserver::Same(r_,wanted.object+cfgprobe::ObjectClass,type))return false;
        bundleReport.objectiveOwnerFailure=5;if(!ObjectiveScope())return false;
        bundleReport.objectiveOwnerFailure=0;
        bundleObjectiveRepairFailure=0;return true;
    }
    cfgobjective::repair::Status ObjectiveRepairObserve(cfgobjective::repair::Snapshot& out){
        using S=cfgobjective::repair::Status;out={};
        if(!bundleObjectiveWidget.object){
            if(bundleObjectiveCandidate>=bundleObjectives.InstanceCount())return S::Pending;
            const auto candidate=bundleObjectives.Instance(bundleObjectiveCandidate);
            if(!ObjectiveWidgetOwner(candidate)){
                if(bundleObjectiveRepairFailure!=1&&r_.reads<r_.cap&&!exception_)++bundleObjectiveCandidate;
                return exception_?S::Unavailable:S::Pending;
            }
            bundleObjectiveWidget=candidate;
            return S::Pending; // Owner adoption and cold panel discovery have separate budgets.
        }
        if(!ObjectiveWidgetOwner(bundleObjectiveWidget))return r_.reads>=r_.cap||bundleObjectiveRepairFailure==1?S::Pending:S::Unavailable;
        bundleObjectiveRepairFailure=5;
        const bool panelReady=cfgobjectivepanel::ReadPanel(r_,plan.module,bundleObjectiveWidget.object,bundleObjectives.RootClass(),out.panel,&bundleObjectivePanelBinding);
        const auto& binding=bundleObjectivePanelBinding;const auto& evidence=binding.evidence;
        bundleReport.objectivePanelStage=binding.stage;bundleReport.objectivePanelChild=binding.childIndex;
        bundleReport.objectivePanelFields=binding.panelSearch.count;bundleReport.objectivePanelOffset=binding.panel.offset;
        bundleReport.objectivePanelPointer=evidence.panel;bundleReport.objectivePanelSerial=evidence.panelIdentity.serial;
        const auto& child=evidence.children[binding.childIndex];
        bundleReport.objectivePanelSlot=child.slot;bundleReport.objectiveSlotSerial=child.slotIdentity.serial;
        bundleReport.objectivePanelContent=child.widget;bundleReport.objectiveChildSerial=child.widgetIdentity.serial;
        if(!panelReady)
            return r_.reads>=r_.cap||bundleObjectivePanelBinding.pending?S::Pending:S::Unavailable;
        out.owner={plan.module,plan.objects[0],plan.objects[2],out.panel.rootIdentity,out.panel.panelIdentity};
        bundleObjectiveRepairFailure=0;return S::Ready;
    }
    bool ObjectiveFinderFrame(cfgobjective::repair::Finder& out){
        const auto function=out.helper.functionSlot.object;
        std::uint16_t returnOffset=0,frameAgain=1,returnAgain=0;std::uint8_t countAgain=1;
        if(!r_.Read(function+cfgobjectivepanel::disk::FunctionNumParms,out.numParms)||out.numParms||
           !r_.Read(function+cfgobjectivepanel::disk::FunctionParameterBytes,out.frameBytes)||out.frameBytes||
           !r_.Read(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,returnOffset)||returnOffset!=0xffff||
           !r_.Read(function+cfgobjectivepanel::disk::FunctionNumParms,countAgain)||countAgain||
           !r_.Read(function+cfgobjectivepanel::disk::FunctionParameterBytes,frameAgain)||frameAgain||
           !r_.Read(function+cfgobjectivepanel::disk::FunctionReturnValueOffset,returnAgain)||returnAgain!=returnOffset)return false;
        out.voidReturn=true;out.helperVerified=true;return true;
    }
    cfgobjective::repair::Status ObjectiveRepairPrepare(const cfgobjective::repair::Owner& owner,cfgobjective::repair::Target target,cfgobjective::repair::Finder& out){
        using S=cfgobjective::repair::Status;out={};bundleObjectiveRepairFailure=6;bundleReport.objectiveFinderGate=1;
        if(owner.module!=plan.module||!(owner.world==plan.objects[0])||!(owner.controller==plan.objects[2])||
           !(owner.widget==bundleObjectiveWidget)||!ObjectiveScope())
            return r_.reads>=r_.cap?S::Pending:S::Unavailable;
        bundleReport.objectiveFinderGate=2;
        if(!bundleObjectives.ReadyHelper(r_,plan.module,cfgobjective::repair::HelperIndex(target),out.helper,owner.widget))
            return r_.reads>=r_.cap?S::Pending:S::Unavailable;
        bundleReport.objectiveFinderGate=3;const bool frame=ObjectiveFinderFrame(out);
        bundleReport.objectiveFinderParms=out.numParms;bundleReport.objectiveFinderBytes=out.frameBytes;
        if(!frame)return r_.reads>=r_.cap?S::Pending:S::Unavailable;
        bundleReport.objectiveFinderGate=0;
        bundleObjectiveRepairFailure=0;return S::Ready;
    }
    template<class Mark>cfgobjective::repair::Status ObjectiveRepairDispatch(const cfgobjective::repair::Owner& owner,
        const cfgobjective::repair::Finder& finder,const cfgobjective::repair::Snapshot& before,Mark mark){
        using S=cfgobjective::repair::Status;cfgobjective::repair::Snapshot fresh;
        const auto observed=ObjectiveRepairObserve(fresh);if(observed!=S::Ready)return observed;
        bundleObjectiveRepairFailure=7;
        if(!(fresh.owner==owner)||!cfgobjective::repair::Same(before,fresh))return S::Unavailable;
        cfgobjective::repair::Finder checked;
        if(!bundleObjectives.ReadyHelper(r_,plan.module,finder.helper.spec,checked.helper,owner.widget)||
           checked.helper.owner!=finder.helper.owner||!(checked.helper.functionSlot==finder.helper.functionSlot)||
           !(checked.helper.script==finder.helper.script)||!ObjectiveFinderFrame(checked)||
           !cfgbundle::Code(r_,plan.module,cfgspawn::GenericDispatcherRva,cfgspawn::GenericDispatcherPrefix)||
           !cfgprimary::LookupSafe(r_,plan.module)||!ObjectiveScope())return r_.reads>=r_.cap?S::Pending:S::Unavailable;
        if(r_.reads+8>=r_.cap||calls_>=cfgbundle::CallCap)return S::Pending;
        if(!mark())return S::Unavailable;
        const bool returned=Invoke(owner.widget.object,checked.helper.functionSlot.object,nullptr,false);
        bundleObjectiveRepairFailure=returned?0:8;return returned?S::Ready:S::Unavailable;
    }
    struct ObjectiveRepairHost{
            BundleAccess& a;
            auto Observe(cfgobjective::repair::Snapshot& out){return a.ObjectiveRepairObserve(out);}
            auto Prepare(const cfgobjective::repair::Owner& owner,cfgobjective::repair::Target target,cfgobjective::repair::Finder& out){return a.ObjectiveRepairPrepare(owner,target,out);}
            template<class Mark>auto Dispatch(const cfgobjective::repair::Owner& owner,const cfgobjective::repair::Finder& finder,
                const cfgobjective::repair::Snapshot& before,Mark mark){return a.ObjectiveRepairDispatch(owner,finder,before,mark);}
    };
    bool ObjectiveRepairPhase(std::uint64_t now){
        ObjectiveRepairHost host{*this};
        return bundleObjectiveRepair.Step(host,now);
    }
