// Inside BundleAccess. Presentation uses the existing retained native gateway;
// it does not replace the Jason driver, allocate actors, or touch game stats.
    template<class T,class Current>cfgpractice::Write PracticeSet(std::uintptr_t object,const cfgvehicle::disk::Binding& binding,
        std::uintptr_t executor,const T& packet,bool visibility,Current&& current){
        std::uintptr_t function=0,owner=0;std::uint8_t count=0;std::uint32_t flags=0;
        if(!NativeCallAllowed()||!FeatureFunction(binding,function,owner)||!cfgspawn::ObjectIs(r_,object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+executor)||!r_.Read(function+0x8C,count)||count!=1||
           !r_.Read(function+0x88,flags)||!(flags&0x400u))return cfgpractice::Write::NotEntered;
        const bool code=visibility?cfgbundle::Code(r_,plan.module,executor,cfgpractice::disk::SetVisibilityExecutorCode):
            cfgbundle::Code(r_,plan.module,executor,cfgpractice::disk::SetVolumeMultiplierExecutorCode);
        cfgreflection::Observation<Reader> reflection(r_,plan.module);cfgreflection::Field field,argument;
        cfgreflection::Property property;std::uintptr_t child=0;
        if(!code||!reflection.ReadField(function,owner,field)||field.label.View()!=(visibility?u"SetVisibility":u"SetVolumeMultiplier")||
           !r_.Read(function+cfgreflection::disk::Children,child)||!reflection.ReadField(child,function,argument)||argument.next||
           argument.label.View()!=(visibility?u"InVisibility":u"NewVolumeMultiplier")||
           argument.kind.View()!=(visibility?u"EnumProperty":u"FloatProperty")||!cfgmenu::capture::RawProperty(r_,argument,property)||
           property.offset||property.dimension!=1||property.elementBytes!=sizeof(T)||!(property.flags&0x80u)||(property.flags&0x500u))return cfgpractice::Write::NotEntered;
        if(!current())return cfgpractice::Write::NotEntered;
        T value=packet;const auto before=calls_;const bool returned=Invoke(object,function,&value,false);
        return returned?cfgpractice::Write::Returned:calls_==before?cfgpractice::Write::NotEntered:cfgpractice::Write::Uncertain;
    }
    bool PracticePoint(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t actor,cfgspawn::Vec3& out){
        std::uintptr_t root=0;cfgprobe::lifetime::SlotObservation slot;
        return r.Read(actor+cfgposition::disk::ActorRootComponent,root)&&cfgpracticeobs::Clear(r,array,root,slot)&&
            cfgpracticeobs::Typed(r,array,plan.module,root,cfgposition::disk::SceneClassCache)&&
            cfgobserver::Same(r,root+0xE0,actor)&&r.Read(root+cfgposition::disk::SceneWorldTranslation,out)&&cfgspawn::Finite(out)&&
            cfgobserver::Same(r,actor+cfgposition::disk::ActorRootComponent,root);
    }
    bool PracticeAudioCurrent(const PracticeAudio& audio,unsigned index){
        Reader r;r.cap=8192;cfgsoundblip::Scope scope;cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation component;
        const bool okay=J215PracticeScope(r,index!=0,scope)&&scope.killer==audio.killer&&
            cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
            cfgobserver::Same(r,scope.killer.object+cfgpractice::disk::GrabbedAudioComponent,audio.component.object)&&
            cfgpracticeobs::Clear(r,array,audio.component.object,component)&&component.object==audio.component.object&&component.index==audio.component.index&&
            (audio.component.serial==0||component.serial==audio.component.serial)&&
            cfgpracticeobs::Typed(r,array,plan.module,component.object,cfgpractice::disk::AudioComponentReflection);
        bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);return okay;
    }
    bool PracticeDistantGrab(unsigned index,bool observe=false){
        Reader r;r.cap=2048;cfgsoundblip::Scope scope;cfgprobe::lifetime::ArrayHeader array;
        cfgprobe::lifetime::SlotObservation victim;std::uintptr_t target=0;cfgspawn::Vec3 human{},killer{};
        if(!J215PracticeScope(r,index!=0,scope)||!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)||
           !r.Read(scope.killer.object+cfgpractice::disk::GrabbedCounselor,target))return false;
        if(!target||target==scope.humanPawn){if(observe)bundlePractice.grabs[index].Observe(0,0,0,bundleReport.j215Practice);return false;}
        if(!cfgpracticeobs::Clear(r,array,target,victim)||!cfgspawn::ObjectIs(r,target,bundleCounselorClass)||
           !PracticePoint(r,array,scope.humanPawn,human)||!PracticePoint(r,array,scope.killer.object,killer))return false;
        const auto x=human.x-killer.x,y=human.y-killer.y,z=human.z-killer.z;const auto squared=x*x+y*y+z*z;
        if(!std::isfinite(squared)||!cfgobserver::Same(r,scope.killer.object+cfgpractice::disk::GrabbedCounselor,target))return false;
        const bool distant=squared>1500.f*1500.f;
        if(observe)bundlePractice.grabs[index].Observe(target,victim.serial,distant?2u:1u,bundleReport.j215Practice);
        return distant;
    }
    bool PracticeAudioStep(){
#if CFG_J215_GRAB_MUTE
        const unsigned index=bundlePractice.audioCursor++%2;auto& audio=bundlePractice.audio[index];
        // Count validated episodes/range transitions even if no audio component
        // exists. Factory/pre-write reproofs do not double-count the event.
        const bool distant=cfgpractice::Enabled(bundleConfiguration.counselorPractice,cfgpractice::GrabMute)&&PracticeDistantGrab(index,true);
        Reader r;r.cap=8192;cfgsoundblip::Scope scope;cfgprobe::lifetime::ArrayHeader array;
        cfgprobe::lifetime::SlotObservation component;std::uintptr_t pointer=0;
        if(!J215PracticeScope(r,index!=0,scope)||!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)||
           !r.Read(scope.killer.object+cfgpractice::disk::GrabbedAudioComponent,pointer))return false;
        if(pointer!=audio.component.object||!(audio.killer==scope.killer)){
            if(audio.value.owned)++bundleReport.j215Practice.grabDeparted;audio={};audio.killer=scope.killer;
            if(!pointer)return false;
            if(!cfgpracticeobs::Clear(r,array,pointer,component)||!cfgpracticeobs::Typed(r,array,plan.module,pointer,cfgpractice::disk::AudioComponentReflection))return false;
            audio.component=component;
        }
        if(!audio.component.object)return false;
        const bool wanted=distant;
        struct Host {BundleAccess& a;PracticeAudio& audio;unsigned index;
            bool Current(){return a.PracticeAudioCurrent(audio,index);}
            bool Read(float& value){return a.r_.Read(audio.component.object+cfgpractice::disk::VolumeMultiplier,value)&&std::isfinite(value)&&value>=0&&value<=16;}
            cfgpractice::Write Set(float value){return a.PracticeSet(audio.component.object,cfgpractice::disk::SetVolumeMultiplier,cfgpractice::disk::SetVolumeMultiplierExecutor,value,false,[&]{return Current()&&(value!=0.f||a.PracticeDistantGrab(index));});}
        }host{*this,audio,index};
        const auto calls=calls_;bool changed=false;
        if(wanted){changed=audio.value.Apply(host,0.f);if(changed)++bundleReport.j215Practice.grabWrites;}
        else if(audio.value.owned){changed=audio.value.Restore(host);if(changed){++bundleReport.j215Practice.grabRestores;audio.value={};}}
        if(audio.value.uncertain)++bundleReport.j215Practice.grabRefused;
        bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);return changed||calls_!=calls;
#else
        return false;
#endif
    }
    bool PracticeWidgetCurrent(const PracticeWidget& widget){
        Reader r;r.cap=8192;cfgsweater::HudEvidence hud;cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation fresh;
        if(!J215PracticeHud(r,hud,array)||!cfgpracticeobs::Clear(r,array,widget.widget.object,fresh)||fresh.index!=widget.widget.index||
           (widget.widget.serial>0&&fresh.serial!=widget.widget.serial)||
           !cfgpracticeobs::Typed(r,array,plan.module,widget.widget.object,cfgpractice::disk::WidgetReflection))return false;
        const bool okay=CFG_J219_FINISHING?
            cfgpracticeobs::CurrentMinimalTarget(r,plan.module,array,hud,bundlePractice.graph,widget.widget.object,widget.node,widget.minimap)!=cfgpracticeobs::MinimalTarget::None:
            (!CFG_J218_FINISHING||(!widget.minimap&&cfgpracticeobs::ExactHealthWidget(r,plan.module,array,widget.widget.object)))&&
            (widget.minimap?cfgobserver::Same(r,hud.widget.object+cfgpractice::disk::Minimap,widget.widget.object):
            cfgpracticeobs::NodeCurrent(r,plan.module,array,hud,bundlePractice.graph,widget.node));
        bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);return okay;
    }
    void PracticeAddWidget(const cfgprobe::lifetime::SlotObservation& widget,unsigned node,bool minimap){
        for(unsigned i=0;i<bundlePractice.targets;++i)if(bundlePractice.widgets[i].widget.object==widget.object)return;
        if(bundlePractice.targets==bundlePractice.widgets.size())return;
        auto& target=bundlePractice.widgets[bundlePractice.targets++];target.widget=widget;target.node=node;target.minimap=minimap;
    }
    bool PracticeScanWidget(){
        // Diagnostic collection never adds mutation targets.
#if CFG_J215_MINIMAL_HUD
        Reader r;r.cap=8192;cfgsweater::HudEvidence hud;cfgprobe::lifetime::ArrayHeader array;
        if(!J215PracticeHud(r,hud,array))return false;auto& graph=bundlePractice.graph;
        if(graph.hudWidget.object&&graph.hudWidget.object!=hud.widget.object){
            for(const auto& target:bundlePractice.widgets)if(target.value.owned)++bundleReport.j215Practice.hudDeparted;
            graph={};bundlePractice.widgets={};bundlePractice.targets=0;
            bundlePractice.healthTargetAdded=false;
#if CFG_J217_FINISHING
            bundleReport.j217Hud={};
#endif
        }
        std::uintptr_t minimap=0;cfgprobe::lifetime::SlotObservation child;
        if((!CFG_J218_FINISHING||CFG_J219_FINISHING)&&r.Read(hud.widget.object+cfgpractice::disk::Minimap,minimap)&&minimap&&cfgpracticeobs::Clear(r,array,minimap,child)&&
           cfgpracticeobs::Typed(r,array,plan.module,minimap,cfgpractice::disk::WidgetReflection))PracticeAddWidget(child,UINT32_MAX,true);
        const auto add=[&](cfgpracticeobs::WidgetNode node){
            if(graph.count==graph.nodes.size())return;
            for(unsigned i=0;i<graph.count;++i)if(graph.nodes[i].widget.object==node.widget.object)return;
            graph.nodes[graph.count++]=node;
        };
        if(!graph.count){cfgpracticeobs::WidgetNode root;std::uintptr_t object=0;
            if(!r.Read(hud.widget.object+cfgpractice::disk::WidgetTree,root.tree)||!cfgpracticeobs::Typed(r,array,plan.module,root.tree,cfgpractice::disk::WidgetTreeReflection)||
               !r.Read(root.tree+cfgpractice::disk::RootWidget,object)||!cfgpracticeobs::Clear(r,array,object,root.widget))return false;
            graph.hudWidget=hud.widget;graph.world=plan.world;add(root);
        }
        if(graph.cursor<graph.count){const auto index=graph.cursor++;const auto parent=graph.nodes[index].widget.object;
            if(!cfgpracticeobs::NodeCurrent(r,plan.module,array,hud,graph,index)){
                ++bundleReport.j215Practice.hudRefused;PracticeHudRefusal(parent,index,1);return false;}
#if CFG_J217_FINISHING
            if(!bundleReport.j217Hud.complete)bundleReport.j217Hud.Visible(cfgpracticeobs::Diagnostic(r,plan.module,array,parent,index,0));
#endif
            if(CFG_J218_FINISHING){
                if(CFG_J219_FINISHING){
                    if(!bundlePractice.healthTargetAdded&&bundlePractice.targets<bundlePractice.widgets.size()&&cfgpracticeobs::ExactHealthWidget(r,plan.module,array,parent)){
                        PracticeAddWidget(graph.nodes[index].widget,index,false);bundlePractice.healthTargetAdded=true;
                    }else if(cfgpracticeobs::Typed(r,array,plan.module,parent,cfgpractice::disk::SCStaminaWidgetReflection))PracticeAddWidget(graph.nodes[index].widget,index,false);
                }else if(!bundlePractice.targets&&cfgpracticeobs::ExactHealthWidget(r,plan.module,array,parent))PracticeAddWidget(graph.nodes[index].widget,index,false);
            }else if(cfgpracticeobs::Typed(r,array,plan.module,parent,cfgpractice::disk::SCStaminaWidgetReflection))PracticeAddWidget(graph.nodes[index].widget,index,false);
            if(cfgpracticeobs::Typed(r,array,plan.module,parent,cfgpractice::disk::PanelWidgetReflection)){
                cfgpracticeobs::WidgetNode::List list;
                if(r.Read(parent+cfgpractice::disk::Slots,list)&&list.Sane())for(unsigned i=0;i<static_cast<unsigned>(list.count)&&graph.count<graph.nodes.size();++i){
                    cfgpracticeobs::WidgetNode node;node.parent=index;node.index=i;node.kind=2;node.list=list;std::uintptr_t object=0;
                    if(!r.Read(list.data+8ULL*i,node.slot)||!cfgpracticeobs::Typed(r,array,plan.module,node.slot,cfgpractice::disk::PanelSlotReflection)||
                       !r.Read(node.slot+cfgpractice::disk::Content,object)||!cfgpracticeobs::Clear(r,array,object,node.widget))continue;add(node);
                }
            }else if(cfgpracticeobs::Typed(r,array,plan.module,parent,cfgpractice::disk::UserWidgetReflection)){
                cfgpracticeobs::WidgetNode node;node.parent=index;node.kind=1;std::uintptr_t object=0;
                if(r.Read(parent+cfgpractice::disk::WidgetTree,node.tree)&&cfgpracticeobs::Typed(r,array,plan.module,node.tree,cfgpractice::disk::WidgetTreeReflection)&&
                   r.Read(node.tree+cfgpractice::disk::RootWidget,object)&&cfgpracticeobs::Clear(r,array,object,node.widget))add(node);
            }
        }
        bundleReport.j215Practice.hudNodes=graph.count;bundleReport.j215Practice.hudStage=graph.cursor>=graph.count?2u:1u;
#if CFG_J217_FINISHING
        bundleReport.j217Hud.treeCapacityHit=graph.count==graph.nodes.size()?1u:0u;
        if(graph.cursor>=graph.count)bundleReport.j217Hud.complete=1;
#endif
        bundleReport.j215Practice.peakReads=(std::max)(bundleReport.j215Practice.peakReads,r.reads);return false;
#else
        return false;
#endif
    }
    void PracticeHudRefusal(std::uintptr_t object,unsigned node,unsigned reason){
#if CFG_J217_FINISHING
        Reader r;r.cap=8192;cfgsweater::HudEvidence hud;cfgprobe::lifetime::ArrayHeader array;
        if(!J215PracticeHud(r,hud,array))return;
        bundleReport.j217Hud.Refused(cfgpracticeobs::Diagnostic(r,plan.module,array,object,node,reason));
#else
        (void)object;(void)node;(void)reason;
#endif
    }
    bool PracticeWidgetStep(bool restore=false){
#if CFG_J215_MINIMAL_HUD
        if(!bundlePractice.targets)return false;
        auto& target=bundlePractice.widgets[bundlePractice.widgetCursor++%bundlePractice.targets];
        struct Host {BundleAccess& a;PracticeWidget& target;unsigned refusal=0;
            bool Current(){return a.PracticeWidgetCurrent(target);}
            bool Read(std::uint8_t& value){return a.r_.Read(target.widget.object+cfgpractice::disk::Visibility,value)&&value<=4;}
            cfgpractice::Write Set(std::uint8_t value){const auto result=a.PracticeSet(target.widget.object,cfgpractice::disk::SetVisibility,cfgpractice::disk::SetVisibilityExecutor,value,true,[&]{return Current();});
                refusal=result==cfgpractice::Write::NotEntered?4u:result==cfgpractice::Write::Uncertain?5u:0u;return result;}
        }host{*this,target};
        const auto calls=calls_;bool changed=false;
        std::uint8_t observed=0;
        if(!host.Current()){PracticeHudRefusal(target.widget.object,target.node,2);return false;}
        if(!host.Read(observed)){PracticeHudRefusal(target.widget.object,target.node,3);return false;}
        if(restore){changed=target.value.Restore(host);if(changed)++bundleReport.j215Practice.hudRestores;}
        else{
            std::uint8_t value=0;
            if(target.value.owned&&host.Current()&&host.Read(value)&&value!=target.value.desired)target.value={};
            changed=target.value.Apply(host,std::uint8_t{1});if(changed)++bundleReport.j215Practice.hudWrites;
        }
        if(target.value.uncertain)++bundleReport.j215Practice.hudRefused;
        if(host.refusal||target.value.uncertain)PracticeHudRefusal(target.widget.object,target.node,host.refusal?host.refusal:6u);
        return changed||calls_!=calls;
#else
        (void)restore;return false;
#endif
    }
    bool PracticeInitialHealth(){
#if CFG_J215_MINIMAL_HUD
        if(CFG_J218_FINISHING)return false; // exact health widget only; do not invoke a broad HUD event
        Reader observation;observation.cap=8192;cfgsweater::HudEvidence hud;cfgprobe::lifetime::ArrayHeader array;
        if(!J215PracticeHud(observation,hud,array))return false;
        if(bundlePractice.healthOwner!=hud.widget.object){bundlePractice.healthOwner=hud.widget.object;bundlePractice.healthAttempted=false;}
        if(bundlePractice.healthAttempted)return false;
        std::uintptr_t function=0,owner=0;std::uint8_t count=0;std::uint32_t flags=0;
        if(!FeatureFunction(cfgpractice::disk::HealthEvent,function,owner)||!cfgspawn::ObjectIs(r_,hud.widget.object,owner)||
           !r_.Read(function+0x88,flags)||!(flags&0x08000000u)||!(flags&0x800u)||(flags&0x40u)||
           !r_.Read(function+0x8C,count)||count!=1)return false;
        cfgreflection::Observation<Reader> reflection(r_,plan.module);cfgreflection::Field field,argument;
        cfgreflection::Property property;std::uintptr_t child=0;
        if(!reflection.ReadField(function,owner,field)||field.label.View()!=u"OnShowHealthBar"||
           !r_.Read(function+cfgreflection::disk::Children,child)||!reflection.ReadField(child,function,argument)||
           argument.label.View()!=u"bShow"||argument.kind.View()!=u"BoolProperty"||argument.next||
           !cfgmenu::capture::RawProperty(r_,argument,property)||property.offset||property.dimension!=1||property.elementBytes!=1||
           !(property.flags&0x80u)||(property.flags&0x500u))return false;
        cfgsweater::HudEvidence fresh;if(!J215PracticeHud(observation,fresh,array)||fresh.widget.object!=hud.widget.object)return false;
        std::uint8_t hidden=0;const auto calls=calls_;bundlePractice.healthAttempted=true;
        if(Invoke(hud.widget.object,function,&hidden,false)){++bundleReport.j215Practice.healthHidden;return true;}
        if(calls_==calls)bundlePractice.healthAttempted=false;else ++bundleReport.j215Practice.healthRefused;
        return calls_!=calls;
#else
        return false;
#endif
    }
    bool PracticePhase(std::uint64_t now){
        const auto options=bundleConfiguration.counselorPractice;bundleReport.j215Practice.options=options;
        bundleReport.j215Practice.voiceHookReady=bundlePracticeVoiceHook.enabled?1u:0u;
        bundleReport.j215Practice.voiceHookFailure=bundlePracticeVoiceInstallFailure;
        if(bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady)return false;
        if(!(options&(cfgpractice::GrabMute|cfgpractice::MinimalHud))||now<bundlePractice.next)return false;
        if(!(bundlePractice.world==plan.objects[0])){bundlePractice={};bundlePractice.world=plan.objects[0];}
        bundlePractice.next=now+100;
        if(options&cfgpractice::GrabMute)if(PracticeAudioStep())return true;
        if(options&cfgpractice::MinimalHud){if(PracticeInitialHealth())return true;PracticeScanWidget();return PracticeWidgetStep();}
        return false;
    }
    void PracticeCleanup(){
        // Exact current-world observations are required by each host. On
        // departure we discard only weak bookkeeping; never touch old actors.
        (void)J215RestoreHard(bundleDonorPrimary);(void)J215RestoreHard(bundleDonorAdditional);
        if(!(bundlePractice.world==plan.objects[0]))return;
        for(unsigned i=0;i<bundlePractice.audio.size();++i){auto& audio=bundlePractice.audio[i];
            if(!audio.value.owned)continue;
            struct Host {BundleAccess& a;PracticeAudio& audio;unsigned index;
                bool Current(){return a.PracticeAudioCurrent(audio,index);}
                bool Read(float& value){return a.r_.Read(audio.component.object+cfgpractice::disk::VolumeMultiplier,value)&&std::isfinite(value);}
                cfgpractice::Write Set(float value){return a.PracticeSet(audio.component.object,cfgpractice::disk::SetVolumeMultiplier,cfgpractice::disk::SetVolumeMultiplierExecutor,value,false,[&]{return Current();});}
            }host{*this,audio,i};
            if(audio.value.Restore(host))++bundleReport.j215Practice.grabRestores;
        }
        for(unsigned i=0;i<bundlePractice.targets;++i)(void)PracticeWidgetStep(true);
    }
