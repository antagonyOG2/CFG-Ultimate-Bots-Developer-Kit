// Included after BundleAccess, GcRegistryAccess and frontend native ownership.
// Compiled only by CFG35_URINATION_NATIVE_PORT until full integration verifies.
cfgretain::ResidentRootBank<1> urinationAssetBank;
cfgurination::RetainedLoad<decltype(urinationAssetBank)> urinationRetainedLoad(urinationAssetBank);
class UrinationNativeHost {
    cfgobserver::Plan source_{};
    cfgprobe::lifetime::SlotObservation state_{};
    cfgurination::LoadIdentity owner_{};
    cfgurination::ActionOwner actionOwner_{};
    bool configured_=false,requesting_=false;
    bool pathAttempted_=false,loadAttempted_=false;
    cfgurination::NativePath preparedPath_{};
    cfgurination::WavePublication publication_{};
    std::array<std::uintptr_t,3> discovered_{};
    cfgprobe::lifetime::SlotObservation selectedHint_{};std::uintptr_t selectedWorld_=0;
    cfgurination::PlaybackPolicy playback_{};
    std::uint64_t playbackAt_=0;bool cleanupFinished_=false,playbackPending_=false,restorePending_=false;
    struct SelectionFrame {std::uintptr_t actor=0,function=0;bool pending=false;unsigned nativeStarts=0;} selection_{};
    GcRegistryAccess* window_=nullptr;
    struct CompletionFrame {UrinationNativeHost* host=nullptr;std::uintptr_t action=0,delegate=0,loaded=0,pc=0;};
    CompletionFrame* frame_=nullptr;
    unsigned reads_=0,faults_=0,completed_=0,refused_=0;
    using Hooks=cfgurination::NativeHooks<UrinationNativeHost>;
    bool CurrentIn(Reader& reader,const cfgurination::LoadIdentity& expected){
        if(!configured_||GetCurrentThreadId()!=owner_.thread||!bundleStarted||!bundleSchedule.active||
            expected.world!=owner_.world||expected.controller!=owner_.controller||expected.worldSerial!=owner_.worldSerial||
            expected.controllerSerial!=owner_.controllerSerial||expected.thread!=owner_.thread||
            expected.uuid!=owner_.uuid||expected.generation!=owner_.generation||
            plan.module!=source_.module||plan.world!=source_.world||plan.mode!=source_.mode||plan.controller!=source_.controller||
            !originalActorDispatcher||source_.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
        cfgobserver::Plan current{};cfgprobe::lifetime::SlotObservation state{};
        return cfgprimary::LookupVetoClearForObservation(reader,source_.module)&&
            cfgprimary::RetainedOfflineWorld(reader,source_,current)&&
            cfgjason::SameSlot(reader,current.objectArray,source_.objects[2])&&
            cfgspawn::ObjectSlot(reader,current.objectArray,state_.object,state)&&state==state_&&
            cfgobserver::Same(reader,source_.world+cfgprobe::WorldGameState,state_.object)&&
            cfgdispatch::Proof(reader,source_.module)&&Hooks::Proof(reader,source_.module)&&
            cfgprimary::LookupVetoClearForObservation(reader,source_.module);
    }
    bool FrameCurrent(Reader& reader,const CompletionFrame& frame){
        if(frame_!=&frame||frame.host!=this||!frame.action||!CurrentIn(reader,owner_))return false;
        cfgurination::ActionObservation observation(reader,*this,actionOwner_);
        const auto stage=urinationRetainedLoad.Request().Stage();
        if(stage==cfgurination::LoadStage::Waiting)return observation.Completion(urinationRetainedLoad.Request(),frame.action,frame.delegate,frame.pc);
        return stage==cfgurination::LoadStage::Completing&&observation.Publishing(urinationRetainedLoad.Request(),frame.action,frame.delegate,frame.pc);
    }
    GcRegistryAccess::NativeContextProof CompletionProof(CompletionFrame& frame){
        return {&frame,
            [](Reader& reader,void* context){auto& saved=*static_cast<CompletionFrame*>(context);return saved.host&&saved.host->FrameCurrent(reader,saved);},
            [](Reader& reader,void* context,std::uintptr_t function,std::uintptr_t pc){auto& saved=*static_cast<CompletionFrame*>(context);
                return saved.host&&function==saved.host->source_.module+cfgurination::disk::LoadedDelegate&&pc==saved.pc&&
                    saved.host->FrameCurrent(reader,saved);}};
    }
public:
    void Selected(void* object,void* function,void* parameters,bool nested){
        const bool preparing=!configured_&&!urinationRetainedLoad.Attempted();
        const bool playing=publication_.PlaybackEligible()&&!publication_.Suspended();
        if(nested||(!preparing&&!playing)||!bundleStarted||!bundleSchedule.active||
            GetCurrentThreadId()!=plan.threadId)return;
        Reader reader;reader.cap=cfgbundle::ReadCap;std::uintptr_t cached=0;
        if(!reader.Read(plan.module+cfgemotemirror::disk::SERVER_PlayEmote.cache,cached)||!cached||cached!=reinterpret_cast<std::uintptr_t>(function))return;
        cfgemoteobservation::Access<Reader> host(reader,plan);
        cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,BundleAccess{}.MirrorInputs());
        cfgemotemirror::Slot source{};cfgemotemirror::Function binding{};cfgemotemirror::Catalogue catalogue{};std::uintptr_t selected=0;
        cfgnames::Observation<Reader> names(reader,plan.module);cfgnames::Text text;
        if(access.Source(source)&&source.object==reinterpret_cast<std::uintptr_t>(object)&&access.Bind(0,binding,false)&&binding.object==cached&&
            access.Parameter(reinterpret_cast<std::uintptr_t>(parameters),selected)&&access.CatalogueFor(source,selected,catalogue)&&
            names.Object(selected,text)&&text.View()==u"Emote_Wave_C"){
            if(preparing){selectedHint_=catalogue.type;selectedWorld_=plan.world;} // observation only; not retained/native permission
            if(playing)selection_={source.object,cached,true,bundleEmotes.report.starts};
        }
        reads_+=reader.reads;
    }
    void SelectionReturned(void* object,void* function,bool nested,bool completed)noexcept{
        if(nested||GetCurrentThreadId()!=plan.threadId||!selection_.pending||
            selection_.actor!=reinterpret_cast<std::uintptr_t>(object)||selection_.function!=reinterpret_cast<std::uintptr_t>(function))return;
        const auto before=selection_.nativeStarts;selection_={};
        if(playback_.NewNativePlayback(completed,before,bundleEmotes.report.starts))playbackPending_=true;
        if(!completed)restorePending_=true; // deferred; observation has no write/native authority
    }
    bool Admitted(){BundleAccess access;return bundleStarted&&bundleSchedule.active&&!bundleSchedule.pressed&&
        !bundleKill.policy.InputExclusive(GetTickCount64())&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady&&
        access.NativeCallAllowed()&&access.Context();}
    bool Discover(){
        if(configured_||urinationRetainedLoad.Attempted()||!Admitted())return false;
        BundleAccess access;std::uintptr_t pawn=0;
        if(!access.Context()||!access.Known(0,pawn)||!bundleRootLease)return false;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            if(!registry.ProvenSlotWindow())return false;Reader reader;reader.cap=4096;cfgobserver::Plan current{};
            std::array<std::uintptr_t,3> value{};
            if(!cfgprimary::RetainedOfflineWorld(reader,plan,current))return false;
            bool found=false;cfgprobe::lifetime::SlotObservation fresh{};
            if(selectedWorld_==plan.world&&selectedHint_.object&&
                cfgspawn::ObjectSlot(reader,current.objectArray,selectedHint_.object,fresh)&&fresh==selectedHint_){
                std::uintptr_t defaults=0;cfgurination::AssetProof proof{};
                value[0]=fresh.object;
                found=reader.Read(value[0]+cfgspawn::ClassDefaultObject,defaults)&&
                    reader.Read(defaults+cfgurination::disk::MaleMontage,value[1])&&reader.Read(defaults+cfgurination::disk::FemaleMontage,value[2])&&
                    cfgurination::ObserveAssets(reader,current.objectArray,plan.module,value,0,proof);
                if(!found&&value[1]){value[2]=0;found=cfgurination::ObserveAssets(reader,current.objectArray,plan.module,value,0,proof);}
            }
            if(!found)found=cfgurination::DiscoverWave(reader,current.objectArray,plan.module,pawn,bundleCounselorClass,
                cfgemotemirror::disk::DefaultEmotes,value);
            reads_+=reader.reads;if(found)discovered_=value;return found;
        });
    }
    // No native call or hook is installed here; only the existing genuine
    // retained gameplay tick may bind a new request's immutable owner.
    bool Configure(){
        if(configured_||urinationRetainedLoad.Attempted()||!urinationAssetBank.Healthy())return false;
        BundleAccess access;
        if(!access.NativeCallAllowed()||!access.Context()||!plan.objects[0].HasAssignedSerial()||
            !plan.objects[2].HasAssignedSerial())return false;
        Reader reader;reader.cap=512;cfgprobe::lifetime::SlotObservation state{};
        if(!cfgspawn::ObjectSlot(reader,access.context.objectArray,spawnAssets.gameState,state)||
            !(state==spawnAssets.primary.stateSlot))return false;
        source_=plan;state_=state;owner_={plan.world,plan.controller,plan.objects[0].serial,plan.objects[2].serial,
            plan.threadId,0x18E030,1};
        actionOwner_={plan.module,plan.threadId,{plan.objects[2].index,plan.objects[2].serial}};
        configured_=owner_.Valid();return configured_;
    }
    bool PinnedBuild(std::uintptr_t module){
        BundleAccess access;
        return configured_&&module==source_.module&&module==plan.module&&GetCurrentThreadId()==owner_.thread&&
            access.NativeCallAllowed()&&access.Context();
    }
    bool Current(const cfgurination::LoadIdentity& expected){Reader reader;reader.cap=1536;
        const bool result=CurrentIn(reader,expected);reads_+=reader.reads;return result;}
    bool Inputs(const cfgurination::LoadIdentity& expected,const std::array<std::uintptr_t,5>& values){
        if(!Current(expected)||!window_||!window_->ProvenSlotWindow())return false;
        Reader reader;reader.cap=1536;cfgobserver::Plan current{};cfgurination::AssetProof proof{};
        const bool result=cfgprimary::RetainedOfflineWorld(reader,source_,current)&&
            cfgurination::ObserveAssets(reader,current.objectArray,source_.module,{values[2],values[3],values[4]},0,proof);
        reads_+=reader.reads;return result;
    }
    bool Loaded(const cfgurination::LoadIdentity& expected,std::uintptr_t value){
        // Completion uses its exact active native frame. Later resolution is
        // admitted only from a NEW real retained tick, never cached callback
        // permission. Both use current strong asset handles in the same window.
        if(!Current(expected)||!window_||!window_->ProvenSlotWindow())return false;
        if(frame_){if(frame_->loaded!=value)return false;}
        else{BundleAccess access;if(!access.NativeCallAllowed())return false;}
        return urinationRetainedLoad.WithAssetRoots(*window_,[&](const std::array<std::uintptr_t,3>& assets)noexcept{
            Reader reader;reader.cap=1536;cfgobserver::Plan current{};cfgurination::AssetProof proof{};
            const bool result=cfgprimary::RetainedOfflineWorld(reader,source_,current)&&
                cfgurination::ObserveAssets(reader,current.objectArray,source_.module,assets,value,proof,publication_.Fields());
            reads_+=reader.reads;return result;
        });
    }
    bool CurrentResidentAssets(){
        BundleAccess access;
        if(!configured_||LoadState()!=cfgurination::LoadStage::Ready||!urinationAssetBank.Healthy()||
            source_.module!=plan.module||plan.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||
            !access.NativeCallAllowed()||!access.Context())return false;
        Reader reader;reader.cap=512;
        return cfgdispatch::Proof(reader,plan.module)&&Hooks::Proof(reader,plan.module);
    }
    bool ResidentLoaded(std::uintptr_t value){
        if(!window_||frame_||!window_->ProvenSlotWindow()||!CurrentResidentAssets())return false;
        return urinationRetainedLoad.WithAssetRoots(*window_,[&](const std::array<std::uintptr_t,3>& assets)noexcept{
            Reader reader;reader.cap=1536;cfgobserver::Plan current{};cfgurination::AssetProof proof{};
            const bool result=cfgprimary::RetainedOfflineWorld(reader,plan,current)&&
                cfgurination::ObserveAssets(reader,current.objectArray,plan.module,assets,value,proof,publication_.Fields());
            reads_+=reader.reads;return result;
        });
    }
    struct HookApi {
        template<std::size_t N>bool Code(std::uintptr_t at,const std::array<unsigned char,N>& expected){
            Reader reader;reader.cap=1;std::array<unsigned char,N> actual{};return reader.Read(at,actual)&&actual==expected;}
        bool Create(std::uintptr_t target,std::uintptr_t detour,void*& original){return MH_CreateHook(
            reinterpret_cast<void*>(target),reinterpret_cast<void*>(detour),&original)==MH_OK;}
        template<std::size_t N>bool PrepareOwned(std::uintptr_t module,std::uintptr_t target,std::uintptr_t original,std::uintptr_t detour,
            const std::array<unsigned char,N>& expected,cfgurination::OwnedHook& out){Reader reader;reader.cap=8;
            return cfgurination::PrepareOwnedHook(reader,module,target,original,detour,expected,out);}
        bool Enable(std::uintptr_t target){return MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK;}
    };
    bool Install(){if(!configured_||!PinnedBuild(source_.module))return false;HookApi api;
        return Hooks::Install(*this,api,source_.module,owner_.thread);}
    const cfgurination::LoadIdentity& Owner()const{return owner_;}
    bool MakePath(std::uintptr_t defaults,std::uintptr_t function){
        if(pathAttempted_||!configured_||!Current(owner_))return false;
        BundleAccess access;if(!access.NativeCallAllowed())return false;
        cfgurination::MakePathPacket packet;
        constexpr int count=static_cast<int>(cfgurination::ExpectedAsset.size()+1);
        packet.input={reinterpret_cast<std::uintptr_t>(cfgurination::disk::AssetPath),count,count};
        pathAttempted_=true;
        if(!access.InvokeUrinationLoader(*this,defaults,function,&packet,false))return false;
        Reader reader;reader.cap=256;cfgnames::Observation<Reader> names(reader,source_.module);cfgnames::Text text;
        if(!packet.result.sub.Empty()||packet.result.asset.number||!names.Decode(packet.result.asset,text)||
            text.View()!=cfgurination::ExpectedAsset||!Current(owner_))return false;
        preparedPath_=packet.result;return true; // FName value only; no raw UObject
    }
    bool RequestLoad(std::uintptr_t defaults,std::uintptr_t function){
        if(loadAttempted_||!configured_||!preparedPath_.asset.index||!Current(owner_)||
            urinationRetainedLoad.Request().Stage()!=cfgurination::LoadStage::Prepared)return false;
        BundleAccess access;if(!access.NativeCallAllowed())return false;
        cfgurination::LoadPacket packet;packet.world=owner_.world;
        std::memcpy(packet.asset.data()+16,&preparedPath_,sizeof(preparedPath_));
        packet.latent={-1,owner_.uuid,{},owner_.controller};loadAttempted_=true;
        return access.InvokeUrinationLoader(*this,defaults,function,&packet,true);
    }
    bool Requesting()const noexcept{return requesting_&&GetCurrentThreadId()==owner_.thread&&
        urinationRetainedLoad.Request().Stage()==cfgurination::LoadStage::Requesting;}
    // Initial typed LoadAsset invocation will own this flag only across its
    // one genuine retained native call; no unrelated constructor can capture.
    bool BeginRequest(){BundleAccess access;
        if(!Current(owner_)||!access.NativeCallAllowed()||!bundleRootLease||!bundleRootLease->NativeEntered()||
            !urinationRetainedLoad.Enter(true))return false;requesting_=true;return true;}
    void RequestReturned(bool normal){requesting_=false;(void)urinationRetainedLoad.Returned(normal,normal&&Current(owner_));}
    bool Prepare(const std::array<std::uintptr_t,5>& inputs,std::uint64_t now){
        if(!Current(owner_)||!bundleRootLease)return false;bool prepared=false;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            window_=&registry;
            __try{prepared=urinationRetainedLoad.Prepare(registry,*this,source_.module,owner_,inputs,now);}
            __finally{window_=nullptr;}
            return prepared;
        });
    }
    bool PrepareDiscovered(std::uint64_t now){
        const auto inputs=discovered_;discovered_={};
        // Discovery was only a hint; Prepare re-proves all named identities
        // while retaining them in its NEW current registry window.
        return Prepare({owner_.world,owner_.controller,inputs[0],inputs[1],inputs[2]},now);
    }
    bool LoaderFunction(bool load){
        BundleAccess access;std::uintptr_t function=0,owner=0;
        return configured_&&access.NativeCallAllowed()&&access.Context()&&
            access.FeatureFunction(load?cfgurination::disk::LoadAsset:cfgurination::disk::MakeSoftObjectPath,function,owner);
    }
    bool LoaderResources(bool load,std::uintptr_t& function,std::uintptr_t& defaults){
        function=defaults=0;BundleAccess access;std::uintptr_t owner=0;
        if(!configured_||!access.NativeCallAllowed()||!access.Context()||
            !access.FeatureFunction(load?cfgurination::disk::LoadAsset:cfgurination::disk::MakeSoftObjectPath,function,owner))return false;
        Reader reader;reader.cap=4096;std::array<std::uintptr_t,4> roots{};
        return reader.Read(owner+cfgspawn::ClassDefaultObject,defaults)&&
            cfgurination::LoaderSchema(reader,source_.module,defaults,function,load,roots);
    }
    bool MakePath(){std::uintptr_t function=0,defaults=0;return LoaderResources(false,function,defaults)&&MakePath(defaults,function);}
    bool RequestLoad(){std::uintptr_t function=0,defaults=0;return LoaderResources(true,function,defaults)&&RequestLoad(defaults,function);}
    cfgurination::LoadStage LoadState()const noexcept{return urinationRetainedLoad.Request().Stage();}
    void Expire(std::uint64_t now){urinationRetainedLoad.Expire(now,Current(owner_));}
    void Constructed(std::uintptr_t action,std::uintptr_t returned,std::uintptr_t path,std::uintptr_t latent,std::uintptr_t caller){
        if(!Requesting())return;Reader reader;reader.cap=1024;
        (void)urinationRetainedLoad.Constructor(reader,*this,actionOwner_,action,returned,path,latent,caller);reads_+=reader.reads;
    }
    void Completing(std::uintptr_t delegate,std::uintptr_t loaded,std::uintptr_t caller){
        if(GetCurrentThreadId()!=owner_.thread||frame_||!configured_||!Current(owner_))return;
        const auto action=urinationRetainedLoad.Request().Action();
        if(!action||delegate!=action+cfgurination::disk::ActionDelegate||caller!=source_.module+cfgurination::disk::DelegateReturn)return;
        CompletionFrame frame{this,action,delegate,loaded,caller};frame_=&frame;
        GcRegistryAccess registry(CompletionProof(frame),16384);bool committed=false;
        __try{
            if(registry.Begin(source_.module+cfgurination::disk::LoadedDelegate,caller)){
                window_=&registry;Reader reader;reader.cap=1536;
                committed=urinationRetainedLoad.Complete(reader,*this,registry,actionOwner_,action,delegate,caller,loaded,GetTickCount64());
                reads_+=reader.reads;
            }
        }__finally{window_=nullptr;registry.End();frame_=nullptr;}
        if(committed)++completed_;else{++refused_;urinationRetainedLoad.Expire(GetTickCount64(),false);}
    }
    void Destroying(std::uintptr_t action)noexcept{if(GetCurrentThreadId()==owner_.thread)urinationRetainedLoad.Destroyed(action);}
    void ObservationFault()noexcept{++faults_;urinationAssetBank.MarkUncertain();urinationRetainedLoad.Expire(GetTickCount64(),false);}
    void SelectionFailed()noexcept{if(publication_.PlaybackEligible()&&!publication_.Suspended())restorePending_=true;}
    bool ResolveLoaded(std::uintptr_t& output){
        output=0;BundleAccess access;
        if(!access.NativeCallAllowed()||!CurrentResidentAssets()||!bundleRootLease||window_||frame_)return false;
        std::uintptr_t value=0;
        const bool resolved=WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            window_=&registry;bool okay=false;
            __try{okay=urinationRetainedLoad.ResolveResident(registry,*this,value);}
            __finally{window_=nullptr;}
            return okay;
        });
        if(resolved)output=value;return resolved;
    }
    bool RetireRequestInputs(){
        BundleAccess access;
        if(!access.NativeCallAllowed()||!CurrentResidentAssets()||!bundleRootLease||window_||frame_)return false;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            return urinationRetainedLoad.ReleaseTransient(registry);
        });
    }
    bool Current(){BundleAccess access;return window_&&!frame_&&window_->ProvenSlotWindow()&&
        access.NativeCallAllowed()&&CurrentResidentAssets();}
    bool Compare(std::uintptr_t address,std::uint64_t expected,std::uint64_t value){
        if(!Current()||!cfgprobe::ObjectPointer(address))return false;
        MEMORY_BASIC_INFORMATION memory{};
        if(VirtualQuery(reinterpret_cast<void*>(address),&memory,sizeof(memory))!=sizeof(memory)||memory.State!=MEM_COMMIT||
            (memory.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto protection=memory.Protect&0xFF;const auto base=reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
        if((protection!=PAGE_READWRITE&&protection!=PAGE_WRITECOPY)||address<base||address-base>memory.RegionSize||
            8>memory.RegionSize-(address-base))return false;
        return static_cast<std::uint64_t>(InterlockedCompareExchange64(reinterpret_cast<volatile LONG64*>(address),
            static_cast<LONG64>(value),static_cast<LONG64>(expected)))==expected;
    }
    bool PublishLoaded(){
        BundleAccess access;
        if(publication_.Attempted()||!access.NativeCallAllowed()||!CurrentResidentAssets()||!bundleRootLease||window_||frame_)return false;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            window_=&registry;bool published=false;
            __try{
                std::uintptr_t loaded=0;
                if(urinationRetainedLoad.ResolveResident(registry,*this,loaded))published=urinationRetainedLoad.WithAssetRoots(registry,
                    [&](const std::array<std::uintptr_t,3>& assets)noexcept{
                        Reader reader;reader.cap=8192;cfgobserver::Plan current{};
                        const bool result=cfgprimary::RetainedOfflineWorld(reader,plan,current)&&
                            publication_.Publish(reader,*this,current.objectArray,source_.module,assets,loaded);
                        reads_+=reader.reads;return result;
                    });
            }__finally{window_=nullptr;}
            return published;
        });
    }
    const cfgurination::WavePublication& Publication()const noexcept{return publication_;}
    bool PublicationDue()const noexcept{return !bundleEmotes.Pending();}
    bool RetireRejected(){
        BundleAccess access;
        if(!access.NativeCallAllowed()||!Current(owner_)||!bundleRootLease||window_||frame_)return false;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            return urinationRetainedLoad.RetireRejected(registry);
        });
    }
    bool RetireFailure(){
        if(cleanupFinished_||!Admitted())return false;
        if(!urinationRetainedLoad.Attempted()){cleanupFinished_=true;return false;}
        // A failed publication may still have a correctly retained montage.
        // Keep its bounded asset handles, but release the request owner roots.
        if(LoadState()==cfgurination::LoadStage::Ready){cleanupFinished_=RetireRequestInputs();return true;}
        urinationRetainedLoad.Expire(GetTickCount64(),false);
        cleanupFinished_=RetireRejected();return true;
    }
    // A NEW completed menu HUD supplies the window. No old-world Context(),
    // asset resolution, restore, original FFrame or gameplay/native invocation.
    bool RetireAtMenu(GcRegistryAccess& registry)noexcept{
        if(requesting_||window_||frame_||!registry.ProvenSlotWindow())return false;
        const auto stage=LoadState();
        if(stage==cfgurination::LoadStage::Requesting||stage==cfgurination::LoadStage::Completing)return false;
        if(!urinationRetainedLoad.Attempted())return true;
        if(stage==cfgurination::LoadStage::Ready)return urinationRetainedLoad.ReleaseTransient(registry);
        urinationRetainedLoad.Expire(GetTickCount64(),false);
        return urinationRetainedLoad.RetireRejected(registry);
    }
    bool Playback(std::uint64_t now,std::uintptr_t wheelPawn=0){
        if(wheelPawn&&!publication_.PlaybackEligible())return true; // no replacement has ever been published
        if(!publication_.PlaybackEligible()||(!wheelPawn&&((!playbackPending_&&!playback_.Active())||now<playbackAt_))||!Admitted())return false;
        playbackAt_=now+100;std::uintptr_t loaded=0;
        if(!ResolveLoaded(loaded))return !wheelPawn;
        BundleAccess access;std::uintptr_t pawn=0,current=0;bool alive=false;
        if(!access.Context()||!access.Known(0,pawn)||!access.Alive(pawn,alive)||!alive||(wheelPawn&&pawn!=wheelPawn))return !wheelPawn;
        Reader reader;reader.cap=256;cfgprobe::lifetime::SlotObservation slot{},fresh{};
        if(!cfgspawn::ObjectSlot(reader,access.context.objectArray,pawn,slot)||!slot.HasAssignedSerial()||
            !reader.Read(pawn+cfgemotemirror::disk::CurrentEmote,current))return !wheelPawn;
        if(wheelPawn&&current!=loaded)return true; // never cancel ordinary Wave/sweater/other native actions
        playback_.Observe(plan.world,pawn,slot.serial,current==loaded,now);
        playbackPending_=false;
        if(!playback_.Active())return !wheelPawn; // an ambiguous prior cancellation cannot replay
        DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);
        const bool move=pid==GetCurrentProcessId()&&((GetAsyncKeyState('W')&0x8000)||(GetAsyncKeyState('A')&0x8000)||
            (GetAsyncKeyState('S')&0x8000)||(GetAsyncKeyState('D')&0x8000)||bundleRBEmote.InputStatus().Movement(now));
        if(!wheelPawn&&!playback_.Due(now,move))return true;
        std::uintptr_t function=0,owner=0;
        if(!access.FeatureFunction(cfgemotemirror::disk::SERVER_CancelEmote,function,owner)||owner!=bundleCounselorClass||
            !cfgbundle::Code(reader,source_.module,cfgemotemirror::disk::SERVER_CancelEmoteExecutor,cfgemotemirror::disk::SERVER_CancelEmoteCode)||
            !cfgobserver::Same(reader,function+0xB0,source_.module+cfgemotemirror::disk::SERVER_CancelEmoteExecutor)||
            !cfgspawn::ObjectSlot(reader,access.context.objectArray,pawn,fresh)||!(fresh==slot)||
            !cfgobserver::Same(reader,pawn+cfgemotemirror::disk::CurrentEmote,loaded)||!Admitted())return !wheelPawn;
        // Native gateway roots the exact actor/function and preserves normal
        // server validation. Consumed before the call, including native faults.
        const bool consumed=wheelPawn?playback_.ConsumeWheel():playback_.Consume(now,move);
        const bool returned=consumed&&access.Invoke(pawn,function,nullptr,true);
        return wheelPawn?returned:true;
    }
    bool RestoreFailedSelection(){
        if(!restorePending_||!Admitted())return false;
        restorePending_=false;if(publication_.Suspended())return true;
        return WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            window_=&registry;bool restored=false;
            __try{
                std::uintptr_t loaded=0;
                if(urinationRetainedLoad.ResolveResident(registry,*this,loaded))restored=urinationRetainedLoad.WithAssetRoots(registry,
                    [&](const std::array<std::uintptr_t,3>& assets)noexcept{
                        Reader reader;reader.cap=4096;cfgobserver::Plan current{};
                        const bool result=cfgprimary::RetainedOfflineWorld(reader,plan,current)&&
                            publication_.Restore(reader,*this,current.objectArray,source_.module,assets,loaded);
                        reads_+=reader.reads;return result;
                    });
            }__finally{window_=nullptr;}
            cfgclosedtrace::Mark(restored?"urination-native-wave-restored":"urination-native-wave-restore-refused",plan.controller);
            return true; // consumes one bounded optional slice even on refusal
        });
    }
    bool NativeWindowOpen()const noexcept{return window_||frame_;}
};
UrinationNativeHost urinationNativeHost;
#if !CFG_J21_EMOTE_MIRROR
void BundleUrinationSelection(void* object,void* function,void* parameters,bool nested)noexcept{
    __try{urinationNativeHost.Selected(object,function,parameters,nested);}
    __except(EXCEPTION_EXECUTE_HANDLER){ /* observation never adopts a failed hint or grants native entry */ }
}
#else
void BundleUrinationSelection(void* object,void* function,void* parameters,bool nested)noexcept{
    (void)object;(void)function;(void)parameters;(void)nested;
    // J2 C': native CFG EmoteClass observation is owned by bundleEmotes.
    // Keep the legacy loader isolated; no Wave default/montage mutation.
}
#endif
#if !CFG_J21_EMOTE_MIRROR
void BundleUrinationSelectionAfter(void* object,void* function,bool nested,bool completed)noexcept{
    if(bundleRBDispatch&&reinterpret_cast<std::uintptr_t>(object)==bundleHumanTarget.object&&
        bundleEmotes.IsPlayFunction(reinterpret_cast<std::uintptr_t>(function)))nested=false;
    urinationNativeHost.SelectionReturned(object,function,nested,completed);
}
#else
void BundleUrinationSelectionAfter(void* object,void* function,bool nested,bool completed)noexcept{
    (void)object;(void)function;(void)nested;(void)completed;
}
#endif
cfgurination::RuntimePhases urinationRuntimePhases;
#if !CFG_J21_EMOTE_MIRROR
void BundleRBConfirmationFailed()noexcept{urinationNativeHost.SelectionFailed();}
#else
void BundleRBConfirmationFailed()noexcept{}
#endif // no Wave publication to roll back
#if !CFG_J21_EMOTE_MIRROR
bool BundleRBCancelCurrent(std::uintptr_t pawn)noexcept{return urinationNativeHost.Playback(GetTickCount64(),pawn);}
#else
bool BundleRBCancelCurrent(std::uintptr_t pawn)noexcept{
    (void)pawn;return true; // native RB confirmation/CancelEmote owns input; no synthetic montage reset
}
#endif
bool BundleUrinationService(std::uint64_t now)noexcept{
    bool worked=false;
    __try{
        const auto previous=urinationRuntimePhases.Stage();
        if(previous==cfgurination::RuntimeStage::Active)worked=urinationNativeHost.RestoreFailedSelection()||urinationNativeHost.Playback(now);
        else if(previous==cfgurination::RuntimeStage::Refused)worked=urinationNativeHost.RetireFailure();
        else worked=urinationRuntimePhases.Step(urinationNativeHost,now);
        if(urinationRuntimePhases.Stage()!=previous)cfgclosedtrace::Mark("urination-native-phase",plan.controller,
            static_cast<unsigned>(urinationRuntimePhases.Stage()));
    }
    __except(EXCEPTION_EXECUTE_HANDLER){urinationNativeHost.ObservationFault();}
    return worked;
}
