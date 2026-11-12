    // Function metadata may legitimately have an unassigned serial. It is
    // never accepted by that alone: re-resolve the exact named member from
    // the current positively identified widget class before every dispatch.
    struct NativeLabelEvent {cfgprobe::lifetime::SlotObservation widget{},function{};std::uintptr_t widgetClass=0;};
    bool FindNativeLabelFunction(std::uintptr_t type,cfgreflection::Field& output){
        // Donor FindFunction resolves by FName, not by decoding the names of
        // every unrelated HUD member. The native cache supplies NAME ONLY;
        // the actual receiver's derived-first member chain owns dispatch.
        struct Header {std::uintptr_t type;cfgnames::Words name;std::uintptr_t owner,next;};
        struct Member {std::uintptr_t object;Header header;};
        static_assert(sizeof(Header)==32);
        output={};cfgnames::Words wanted{},again{};cfgnames::Text label;
        cfgnames::Observation<Reader> names(r_,plan.module);
        if(!bundleSpectatorStatusFunction||!r_.Read(bundleSpectatorStatusFunction+cfgnames::disk::ObjectName,wanted)||
           !names.Decode(wanted,label)||label.View()!=u"OnSpectatorUpdateStatus"||
           !r_.Read(bundleSpectatorStatusFunction+cfgnames::disk::ObjectName,again)||!(again==wanted))return false;
        std::array<Member,256> members{};unsigned count=0;
        std::array<std::uintptr_t,32> parents{},heads{},supers{};unsigned depth=0;
        cfgreflection::Observation<Reader> reflection(r_,plan.module);
        for(auto owner=type;owner;){
            if(!cfgprobe::ObjectPointer(owner)||depth==parents.size())return false;
            for(unsigned i=0;i<depth;++i)if(parents[i]==owner)return false;
            parents[depth]=owner;
            std::uintptr_t child=0,match=0;unsigned matches=0;
            if(!r_.Read(owner+cfgreflection::disk::Children,child))return false;
            heads[depth++]=child;
            while(child){
                if(!cfgprobe::ObjectPointer(child)||count==members.size())return false;
                for(unsigned i=0;i<count;++i)if(members[i].object==child)return false;
                Header h{};if(!r_.Read(child+cfgprobe::ObjectClass,h)||h.owner!=owner||
                    !cfgprobe::ObjectPointer(h.type)||(h.next&&!cfgprobe::ObjectPointer(h.next)))return false;
                members[count++]={child,h};
                if(h.name==wanted){match=child;++matches;}
                child=h.next;
            }
            if(matches){
                if(matches!=1||!reflection.ReadField(match,owner,output)||output.kind.View()!=u"Function"||
                   output.label.View()!=u"OnSpectatorUpdateStatus")return false;
                for(unsigned i=0;i<count;++i){Header h{};const auto& m=members[i];
                    if(!r_.Read(m.object+cfgprobe::ObjectClass,h)||h.type!=m.header.type||!(h.name==m.header.name)||
                       h.owner!=m.header.owner||h.next!=m.header.next)return false;}
                for(unsigned i=0;i<depth;++i){
                    if(!cfgobserver::Same(r_,parents[i]+cfgreflection::disk::Children,heads[i])||
                       (i+1<depth&&!cfgobserver::Same(r_,parents[i]+cfgprobe::ClassSuper,supers[i])))return false;}
                return r_.Read(bundleSpectatorStatusFunction+cfgnames::disk::ObjectName,again)&&again==wanted;
            }
            if(!cfgobserver::Same(r_,owner+cfgreflection::disk::Children,heads[depth-1])||
               !r_.Read(owner+cfgprobe::ClassSuper,supers[depth-1]))return false;
            owner=supers[depth-1];
        }
        return false;
    }
    bool NativeLabelWidget(std::uintptr_t& output){
        using namespace cfgsweater::huddisk;
        output=0;std::uintptr_t hud=0,widget=0,hudType=0,widgetType=0;
        cfgprobe::lifetime::SlotObservation hs{},ws{};
        // HUD can be replaced on death/return while the camera stays valid.
        // Reacquire ONLY through the current local controller's typed HUD.
        // This is a label hint, never camera or arbitrary-widget authority.
        if(!SummaryObservationScope()||!SpectatorLocalOwner()||
           !NativeClass(SCInGameHUDReflection,SCInGameHUDConstructor,hudType)||
           !NativeClass(SCHUDWidgetReflection,SCHUDWidgetConstructor,widgetType)||
           !r_.Read(plan.controller+ControllerHUD,hud)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,hud,hs)||!hs.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r_,hud,hudType)||!r_.Read(hud+HUDWidget,widget)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,widget,ws)||!ws.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r_,widget,widgetType)||!cfgjason::SameSlot(r_,context.objectArray,hs)||
           !cfgjason::SameSlot(r_,context.objectArray,ws)||
           !cfgobserver::Same(r_,plan.controller+ControllerHUD,hud)||
           !cfgobserver::Same(r_,hud+HUDWidget,widget)||!cfgprimary::LookupVetoClearForObservation(r_,plan.module))return false;
        output=widget;return true;
    }
    // Cheap cache admission, NOT dispatch authority. The full member-chain
    // proof below is required after frame/view validation, just before Invoke.
    bool NativeLabelEventIdentityCurrent(const NativeLabelEvent& event){
        return event.widget.HasAssignedSerial()&&event.function.object&&
            cfgjason::SameSlot(r_,context.objectArray,event.widget)&&
            cfgobserver::Same(r_,event.widget.object+cfgprobe::ObjectClass,event.widgetClass);
    }
    bool NativeLabelEventCurrent(const NativeLabelEvent& event){
        cfgprobe::lifetime::ArrayHeader registry{};cfgreflection::Field field;
        cfgprobe::lifetime::SlotObservation function{};
        return event.widget.HasAssignedSerial()&&cfgprimary::LookupVetoClearForObservation(r_,plan.module)&&
            cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgjason::SameSlot(r_,registry,event.widget)&&
            cfgobserver::Same(r_,event.widget.object+cfgprobe::ObjectClass,event.widgetClass)&&
            FindNativeLabelFunction(event.widgetClass,field)&&field.kind.View()==u"Function"&&
            field.object==event.function.object&&cfgspawn::ObjectSlot(r_,registry,field.object,function)&&function==event.function&&
            cfgjason::SameSlot(r_,registry,event.widget)&&
            cfgobserver::Same(r_,event.widget.object+cfgprobe::ObjectClass,event.widgetClass)&&cfgprimary::LookupVetoClearForObservation(r_,plan.module);
    }
    bool ResolveNativeLabelEvent(std::uintptr_t widget,NativeLabelEvent& output){
        output={};NativeLabelEvent event;cfgreflection::Field field;
        if(!SummaryObservationScope()||!cfgspawn::ObjectSlot(r_,context.objectArray,widget,event.widget)||!event.widget.HasAssignedSerial()||
           !r_.Read(widget+cfgprobe::ObjectClass,event.widgetClass)||
           !FindNativeLabelFunction(event.widgetClass,field)||field.kind.View()!=u"Function"||
           !cfgspawn::ObjectSlot(r_,context.objectArray,field.object,event.function)||
           !cfgjason::SameSlot(r_,context.objectArray,event.widget)||
           !cfgobserver::Same(r_,widget+cfgprobe::ObjectClass,event.widgetClass)||!cfgprimary::LookupVetoClearForObservation(r_,plan.module))return false;
        output=event;return true; // full HUD/frame/current-view proof still required
    }
    // Text-only native-selection proof, independent of Runtime::Following().
    // A native-stock camera can keep watching Jason after that local scheduling
    // flag drops. Never move/reset/reselect the accepted camera to fix its name.
    bool NativeJasonView(){
        // Native Tick's typed Viewing+3F8 is pinned by
        // Verify-CfgNativeSpectatorFollow.ps1. The camera manager must actually
        // view that stock spectator, not a camp camera retaining an old field.
        namespace d=cfgspectatorview::disk;
        std::uintptr_t camera=0,type=0,view=0,spectator=0,pawnType=0;cfgprobe::lifetime::SlotObservation cam{},pawn{};
        if(!r_.Read(plan.controller+d::ControllerCamera,camera)||
           !NativeClass(cfgcamera::disk::CameraClassCache,cfgcamera::disk::CameraConstructorCache,type)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,camera,cam)||!cam.HasAssignedSerial()||!cfgspawn::ObjectIs(r_,camera,type)||
           !cfgobserver::Same(r_,camera+d::PCOwner,plan.controller)||!r_.Read(camera+d::Active,view)||
           !cfgobserver::Same(r_,camera+d::Pending,std::uintptr_t{})||
           !cfgbundle::Code(r_,plan.module,d::Getter,d::GetterCode))return false;
        if(view!=bundleJason.pawn.object){
            if(!r_.Read(plan.controller+cfgprobe::ControllerSpectatorPawn,spectator)||!spectator||view!=spectator||
               !NativeClass(cfgspectator::disk::PawnClassCache,cfgspectator::disk::PawnConstructorCache,pawnType)||
               !cfgspawn::ObjectSlot(r_,context.objectArray,spectator,pawn)||!pawn.HasAssignedSerial()||
               !cfgspawn::ObjectIs(r_,spectator,pawnType)||!cfgobserver::Same(r_,spectator+cfgescape::disk::Controller,plan.controller)||
               !cfgobserver::Same(r_,spectator+0x3F8,bundleJason.pawn.object)||
               !cfgjason::SameSlot(r_,context.objectArray,pawn))return false;
        }
        return cfgjason::SameSlot(r_,context.objectArray,cam)&&cfgobserver::Same(r_,camera+d::PCOwner,plan.controller)&&
            cfgobserver::Same(r_,camera+d::Active,view)&&cfgobserver::Same(r_,camera+d::Pending,std::uintptr_t{})&&
            cfgobserver::Same(r_,plan.controller+d::ControllerCamera,camera);
    }
    unsigned nativeLabelProofStage_=0;
    bool NativeJasonLabelProof(cfgescape::State* terminal=nullptr){
        std::uintptr_t state=0,type=0;std::uint8_t dead=0,escaped=0;
        cfgprobe::lifetime::SlotObservation slot{},again{};
        cfgnames::Words phase{};cfgnames::Text label;cfgnames::Observation<Reader> names(r_,plan.module);
        const auto stage=[&](unsigned value,bool okay){nativeLabelProofStage_=value;return okay;};
        const bool proven=stage(1,SummaryObservationScope()&&SpectatorLocalOwner()&&cfgjason::SameSlot(r_,context.objectArray,plan.objects[2]))&&
            stage(2,NativeJasonView())&&
            stage(3,Jason()&&r_.Read(plan.controller+cfgescape::disk::PlayerState,state)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,state,slot)&&slot.HasAssignedSerial()&&
            NativeClass(cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,type)&&
            cfgspawn::ObjectIs(r_,state,type)&&cfgobserver::Same(r_,state+0x88,plan.controller))&&
            stage(4,r_.Read(state+cfgescape::disk::bDead,dead)&&dead<=1&&
            r_.Read(state+cfgescape::disk::bEscaped,escaped)&&escaped<=1&&(dead||escaped))&&
            stage(5,r_.Read(plan.mode+cfgescape::disk::MatchState,phase)&&names.Decode(phase,label)&&label.View()==u"InProgress")&&
            stage(6,cfgspawn::ObjectSlot(r_,context.objectArray,state,again)&&again==slot&&
            cfgobserver::Same(r_,plan.controller+cfgescape::disk::PlayerState,state)&&
            cfgobserver::Same(r_,state+0x88,plan.controller)&&
            cfgescape::Same(r_,state+cfgescape::disk::bDead,dead)&&cfgescape::Same(r_,state+cfgescape::disk::bEscaped,escaped)&&
            cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase))&&
            stage(7,NativeJasonView())&&stage(8,cfgprimary::LookupVetoClearForObservation(r_,plan.module));
        if(proven&&terminal)*terminal=dead?cfgescape::State::Dead:cfgescape::State::Escaped;
        return proven;
    }
    bool NativeNeutralLabel(std::uintptr_t widget,std::uintptr_t function,std::uintptr_t parameters,cfgspectatorstatus::Frame& replacement){
        cfgspectatorstatus::Frame before{},after{};cfgspectator::Snapshot snapshot{};
        if(!r_.Read(parameters,before)||!SpectatorStatus(widget,function,parameters,nullptr)||
           !SpectatorSafety(snapshot,false)||snapshot.nativeSelection.object==bundleJason.pawn.object||
           NativeJasonView()||
           !cfgobserver::Same(r_,plan.controller+cfgspectator::disk::SpectatingPlayer,snapshot.nativeSelection.object)||
           !r_.Read(parameters,after)||std::memcmp(&before,&after,sizeof(before)))return false;
        cfgspectatorstatus::NeutralFrame(after,replacement);bundleReport.spectatorStatusJasonView=0;return true;
    }
    bool NativeJasonLabel(std::uintptr_t widget,std::uintptr_t function,std::uintptr_t parameters,cfgspectatorstatus::Frame& replacement){
        nativeLabelProofStage_=0;
        cfgspectatorstatus::Frame before{},after{};
        if(!r_.Read(parameters,before)||!SpectatorStatus(widget,function,parameters,nullptr)||!NativeJasonLabelProof()||
           !r_.Read(parameters,after)||std::memcmp(&before,&after,sizeof(before))){
            return false;}
        cfgspectatorstatus::JasonFrame(bundleReport.spectatorStatusText,after,replacement);
        bundleReport.spectatorStatusJasonView=1;++bundleReport.spectatorLabelOverrides;
        return true;
    }
