#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    bool DonorConvergenceRead(std::uintptr_t pawn,cfgdonorhunt::CounselorObservation& value){
        value={};Reader proof;proof.cap=768;cfgprobe::lifetime::ArrayHeader registry{};
        cfgprobe::lifetime::SlotObservation expected{};
        for(unsigned i=0;i<64;++i)if(bundleDonorRoster.pawns[i].object==pawn){expected=bundleDonorRoster.pawns[i];break;}
        if(!expected.HasAssignedSerial()||!cfgprobe::lifetime::ReadArrayHeader(proof,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
        return cfgdonorhunt::ObserveDonorCounselor(proof,registry,plan.module,plan.world,expected,bundleCounselorClass,value)&&value.Live();
    }
    bool DonorConvergencePostGrab(std::uintptr_t pawn,std::uint64_t now){
        for(const auto& item:bundleDonorPostGrab)if(item.hold.world==plan.world&&item.hold.victim==pawn&&now<item.until)return true;
        return pawn==bundleHeldRecovery.released&&now<bundleHeldRecovery.quarantineUntil;
    }
    DonorConvergenceBot* DonorConvergenceBotFor(std::uintptr_t pawn){
        if(!pawn||pawn==bundleDonorRoster.roster.LocalCounselorTarget||pawn==bundleHumanTarget.object||pawn==plan.pawn)return nullptr;
        cfgdonorhunt::CounselorObservation value;
        if(!DonorConvergenceRead(pawn,value))return nullptr;
        Reader proof;proof.cap=128;
        if(!cfgspawn::ObjectIs(proof,value.controller.object,spawnAssets.resources.classes[cfgspawn::CounselorAI]))return nullptr;
        DonorConvergenceBot* empty=nullptr;
        for(auto& bot:bundleDonorConvergence.bots){
            if(bot.pawn.object==pawn){
                if(!(bot.pawn==value.pawn)||!(bot.controller==value.controller)){
                    if(bot.brainOwned)++bundleDonorConvergence.brainLost;
                    bot={}; // never adopt a replacement brain through an old receipt
                }
                bot.pawn=value.pawn;bot.controller=value.controller;return &bot;
            }
            if(!bot.pawn.object&&!empty)empty=&bot;
            else if(bot.pawn.object&&!bot.brainOwned&&bundleDonorRoster.roster.DonorFindJasonAITargetIndex(bot.pawn.object)<0&&!empty){
                bot={};empty=&bot;
            }
        }
        if(empty){empty->pawn=value.pawn;empty->controller=value.controller;}return empty;
    }
    bool DonorConvergenceWarm(){
        using namespace cfgconvergence;
        if(bundleConvergencePrepared<ConvergenceFunctions.size()){
            auto& resource=bundleConvergenceResources[bundleConvergencePrepared];
            if(!FeatureFunction(ConvergenceFunctions[bundleConvergencePrepared],resource.function,resource.owner)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,resource.function,resource.functionSlot)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,resource.owner,resource.ownerSlot))return false;
            ++bundleConvergencePrepared;return false;
        }
        if(bundleReport.convergenceKeys<KeyCount){
            const auto key=bundleReport.convergenceKeys;const auto text=KeyText[key];unsigned length=0;
            while(text[length])++length;
            NamePacket packet;packet.input={reinterpret_cast<std::uintptr_t>(text),static_cast<int>(length+1),static_cast<int>(length+1)};
            const auto owner=bundleConvergenceResources[0].owner;std::uintptr_t cdo=0;
            if(!r_.Read(owner+cfgspawn::ClassDefaultObject,cdo)||!cfgobserver::Same(r_,cdo+cfgprobe::ObjectClass,owner)||
                !ConvergenceInvoke(0,cdo,&packet))return false;
            cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text decoded{};
            if(packet.result.number!=0||!names.Decode({packet.result.index,packet.result.number},decoded)||
                decoded.View()!=std::u16string_view(text,length))return false;
            bundleConvergenceNames[key]=packet.result;++bundleReport.convergenceKeys;return false;
        }
        if(BlackboardClassesWarm())return false;return !bundleBlackboardClassesRefused;
    }
    bool DonorConvergenceBlackboard(DonorConvergenceBot& bot,std::uintptr_t& blackboard){
        blackboard=0;std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation fresh{};
        if(!cfgjason::SameSlot(r_,context.objectArray,bot.pawn)||!cfgjason::SameSlot(r_,context.objectArray,bot.controller)||
            !cfgobserver::Same(r_,bot.pawn.object+cfgescape::disk::Controller,bot.controller.object)||
            !cfgobserver::Same(r_,bot.controller.object+cfgescape::disk::Pawn,bot.pawn.object)||
            !NativeClass(cfgconvergence::disk::BlackboardComponentReflection,cfgconvergence::disk::BlackboardComponentConstructor,type)||
            !r_.Read(bot.controller.object+cfgconvergence::disk::ControllerBlackboard,blackboard)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,blackboard,fresh)||!cfgspawn::ObjectIs(r_,blackboard,type)||
            !cfgobserver::Same(r_,blackboard+0x20,bot.controller.object)||!cfgobserver::Same(r_,blackboard+0xE0,bot.controller.object))return false;
        if(bot.blackboard.object&&!(bot.blackboard==fresh)){bot.schema.Reset();bot.pending=0;}
        bot.blackboard=fresh;return true;
    }
    std::uintptr_t DonorConvergenceWeapon(std::uintptr_t pawn){
        std::uintptr_t result=0;
        auto* bot=DonorConvergenceBotFor(pawn);
        if(!bot||!cfgspawn::ObjectIs(r_,pawn,bundleConvergenceResources[5].owner)||!ConvergenceInvoke(5,pawn,&result,true))return 0;
        for(unsigned i=0;i<bundleReport.bots&&i<bundleReport.convergenceWeaponResult.size();++i)
            if(bundleReport.roster[i].pawn==pawn){
                if(!result)bundleReport.convergenceWeaponResult[i]=4;
                else {std::uintptr_t type=0;if(r_.Read(result+cfgprobe::ObjectClass,type))
                    bundleReport.convergenceWeaponResult[i]=type==bundleDonorConvergence.donor.CounselorConvergenceMeleeClass?1u:
                        type==bundleDonorConvergence.donor.CounselorConvergenceMacheteClass?2u:3u;}
                break;
            }
        if(!result||!HeldWeapon(pawn,result))return 0;
        return result;
    }
    bool DonorConvergenceBrain(std::uintptr_t pawn,bool enabled){
        auto* bot=DonorConvergenceBotFor(pawn);if(!bot)return false;
        cfgprobe::lifetime::SlotObservation brain{};
        if(!cfgconvergencetravel::Brain(r_,context.objectArray,bot->controller,
            spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],brain))return false;
        if(bot->brainOwned&&!(bot->brain==brain)){bot->brainOwned=false;++bundleDonorConvergence.brainLost;return false;}
        if(enabled&&!bot->brainOwned)return true; // never claim an externally disabled brain
        namespace d=cfgconvergencetravel::disk;std::uint8_t on=0;
        if(!CarCall(d::IsComponentTickEnabled,d::IsComponentTickEnabledExecutor,d::IsComponentTickEnabledCode,brain.object,&on)||on>1)return false;
        if(!enabled&&!bot->brainOwned&&!on)return false;
        if(on==static_cast<unsigned>(enabled)){if(enabled)bot->brainOwned=false;return true;}
        const bool wasOwned=bot->brainOwned;bot->brain=brain;bot->brainOwned=true;on=enabled?1:0;bundleInvocationStage_=0;
        if(!CarCall(d::SetComponentTickEnabled,d::SetComponentTickEnabledExecutor,d::SetComponentTickEnabledCode,brain.object,&on)){
            if(bundleInvocationStage_<5)bot->brainOwned=wasOwned;return false;
        }
        cfgprobe::lifetime::SlotObservation after{};
        if(!cfgconvergencetravel::Brain(r_,context.objectArray,bot->controller,
            spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],after)||!(after==brain))return false;
        std::uint8_t observed=2;
        if(!CarCall(d::IsComponentTickEnabled,d::IsComponentTickEnabledExecutor,d::IsComponentTickEnabledCode,brain.object,&observed)||
            observed!=(enabled?1u:0u))return false;
        if(enabled){bot->brainOwned=false;++bundleDonorConvergence.brainResumes;}return true;
    }
    bool DonorConvergenceGrantNative(std::uintptr_t entry,std::uintptr_t pawn,std::uintptr_t type,bool& entered)noexcept{
        entered=false;
        if(entry!=plan.module+cfgmenu::loadoutdisk::GiveStartingItem||!DonorConvergenceBotFor(pawn)||
            DonorConvergencePostGrab(pawn,GetTickCount64())||!NativeCallAllowed()||
            !cfgbundle::Code(r_,plan.module,cfgmenu::loadoutdisk::GiveStartingItem,cfgmenu::loadoutdisk::GiveStartingItemCode)||
            !RetainBundleActors(pawn)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{type})||
            !NativeCallAllowed()||!BeginBundleNative())return false;
        entered=true;++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(entry)(reinterpret_cast<void*>(pawn),reinterpret_cast<void*>(type));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;BundleRetentionFailed();return false;}
        return FinishBundleNative();
    }
    // H2.59: the Observe stage 1 request needs a non-zero submission id, which only the offline menu sets; a hosted match has none (live: 1239 refusals, 0 axes).
    std::uint64_t DonorGrantSubmission()const noexcept{return CFG_H259_HOSTED_GRANT_SUBMISSION&&HostedRoutineActive()?std::uint64_t{1}:bundleConfiguration.submission;}
    bool DonorConvergenceGrant(std::uintptr_t pawn,std::uintptr_t type){
        bundleDonorConvergence.grantStage=0;auto* bot=DonorConvergenceBotFor(pawn);
        if(!bot){bundleDonorConvergence.grantStage=1;return false;}
        if(bot->grantAmbiguous||bundleDonorConvergence.grantThisTurn||DonorConvergencePostGrab(pawn,GetTickCount64())){bundleDonorConvergence.grantStage=2;return false;}
        if(HostedRoutineActive()&&type!=bundleDonorConvergence.donor.CounselorConvergenceMeleeClass&&!(CFG_H256_HOSTED_AXE&&type&&(type==bundleJ21Lean.axe||type==bundleJ21Lean.machete)))return false;
        Reader proof;proof.cap=4096;cfgnames::Observation<Reader> names(proof,plan.module);cfgnames::Text name;
        if(!names.Object(type,name)){bundleDonorConvergence.grantStage=3;return false;}
        const unsigned choice=name.View()==u"CounselorTwoHandedAxe_C"?3u:name.View()==u"CounselorMachete_C"?2u:0u;
        if(!choice){bundleDonorConvergence.grantStage=3;return false;}
        const auto equipped=DonorConvergenceWeapon(pawn);std::uintptr_t currentType=0;
        if(equipped&&proof.Read(equipped+cfgprobe::ObjectClass,currentType)&&currentType==type)return true;
        cfgofflinesettings::Request owner{DonorGrantSubmission(),plan.world,pawn,plan.objects[0].serial,bot->pawn.serial};
        struct Host {BundleAccess& a;DonorConvergenceBot& bot;bool entered=false;
            bool Current(const cfgofflinesettings::Request& request){
                return request.Valid()&&request.submission==a.DonorGrantSubmission()&&request.world==plan.world&&
                    request.worldSerial==plan.objects[0].serial&&request.pawn==bot.pawn.object&&request.pawnSerial==bot.pawn.serial&&
                    a.DonorConvergenceBotFor(request.pawn)==&bot&&!a.DonorConvergencePostGrab(request.pawn,GetTickCount64())&&a.NativeEnvironmentAllowed();
            }
            bool InvokeGrant(std::uintptr_t entry,std::uintptr_t receiver,std::uintptr_t weapon){return a.DonorConvergenceGrantNative(entry,receiver,weapon,entered);}
        } host{*this,*bot};
        cfgloadout::expanded::Request request{owner,cfgloadout::Slot::Weapon,choice};
        cfgloadout::expanded::NativeAccess<Reader,Host> access(proof,host,request,plan.module,type);
        access.hostedOnline=HostedRoutineActive();
        cfgloadout::expanded::Evidence before{};
        if(!access.Observe(before,true)){bundleDonorConvergence.grantStage=4;
            static unsigned markedStage=~0u,marks=0;if(access.lastStage!=markedStage||marks<4){markedStage=access.lastStage;++marks;cfgclosedtrace::Mark("h258-grant-observe-refused",pawn,access.lastStage);}return false;}
        bundleDonorConvergence.grantThisTurn=true;
        if(access.Grant(before)&&access.Verify(before)){++bundleDonorConvergence.grants;return true;}
        bundleDonorConvergence.grantStage=host.entered?6:5;
        if(host.entered){bot->grantAmbiguous=true;++bundleDonorConvergence.ambiguousGrants;
            cfgclosedtrace::Mark("j2-convergence-grant-uncertain",pawn,access.lastStage);}
        else ++bundleDonorConvergence.deferrals;
        return false;
    }
    bool DonorConvergencePublish(){
        using namespace cfgconvergence;
        auto& state=bundleDonorConvergence;
        for(unsigned n=0;n<state.bots.size();++n){const auto index=state.publishCursor++%static_cast<unsigned>(state.bots.size());auto& bot=state.bots[index];
            if(!bot.pending||!DonorConvergenceBotFor(bot.pawn.object)||DonorConvergencePostGrab(bot.pawn.object,GetTickCount64()))continue;
            std::uintptr_t blackboard=0;if(!DonorConvergenceBlackboard(bot,blackboard))return false;
            const auto status=bot.schema.Step(r_,context.objectArray,plan.module,blackboard,bot.blackboard,bundleConvergenceNames);
            if(status!=BlackboardSchema::State::Ready){
                if(r_.reads>=r_.cap){bot.schema.Reset();++state.deferrals;}
                else if(status==BlackboardSchema::State::Unavailable)bot.pending=0;
                return false;
            }
            unsigned key=0;while(key<KeyCount&&!(bot.pending&(1u<<key)))++key;if(key==KeyCount)return false;
            if(!bot.schema.Supports(key)){bot.pending&=~(1u<<key);return true;}
            bool result=false;
            if(key<=cfgconvergence::Jason){ObjectPacket packet{bundleConvergenceNames[key],bot.objects[key]};
                const auto expected=packet.value;
                if(!bot.schema.AcceptsObject(r_,key,expected))return false;
                bundleInvocationStage_=0;
                if(!ConvergenceInvoke(3,blackboard,&packet)){
                    if(bundleInvocationStage_>=5){bot.uncertainKeys|=1u<<key;bot.pending&=~(1u<<key);++state.uncertainPublications;}return false;
                }
                packet.value=0;result=ConvergenceInvoke(1,blackboard,&packet)&&packet.value==expected;
            }else{BoolPacket packet{bundleConvergenceNames[key]};packet.value=bot.booleans[key]?1:0;const auto expected=packet.value;
                bundleInvocationStage_=0;
                if(!ConvergenceInvoke(4,blackboard,&packet)){
                    if(bundleInvocationStage_>=5){bot.uncertainKeys|=1u<<key;bot.pending&=~(1u<<key);++state.uncertainPublications;}return false;
                }
                packet.value=2;result=ConvergenceInvoke(2,blackboard,&packet)&&packet.value==expected;
            }
            if(result){bot.pending&=~(1u<<key);++state.publications;}return result;
        }
        return false;
    }
    bool DonorConvergenceCleanup(){
        for(auto& bot:bundleDonorConvergence.bots)if(bot.brainOwned){
            if(!DonorConvergenceBotFor(bot.pawn.object)){bot.brainOwned=false;++bundleDonorConvergence.brainLost;return true;}
            DonorConvergenceBrain(bot.pawn.object,true);return true;
        }return false;
    }
    bool DonorKillTeamConvergence(const cfgjasontick::Owner& owner,std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Convergence);
#endif

        if(!cfgj21::ConvergenceLaneEnabled(cfgj21::ConvergenceEnabled,HostedRoutineActive()))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,1)
#else
 false
#endif
;
        const bool bodyDue=now>=bundleDonorConvergence.donor.NextCounselorConvergenceAt;
        bool publicationDue=false;
        if(now>=bundleDonorConvergence.publishAt)for(const auto& bot:bundleDonorConvergence.bots)publicationDue=publicationDue||bot.pending!=0;
        const bool warming=bundleConvergencePrepared<ConvergenceFunctions.size()||bundleReport.convergenceKeys<cfgconvergence::KeyCount||bundleBlackboardClassIndex<4;
        if((warming&&now<bundleDonorConvergence.warmAt)||(!warming&&!bodyDue&&!publicationDue))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,2)
#else
 false
#endif
;
        if(!DonorNativeOwner(owner))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,3)
#else
 false
#endif
;
        auto& native=bundleDonorConvergence;
        if(!(native.world==plan.objects[0])){
            const auto lost=native.brainLost+static_cast<std::uint64_t>(std::count_if(native.bots.begin(),native.bots.end(),[](const auto& b){return b.brainOwned;}));
            native={};native.world=plan.objects[0];native.brainLost=lost; // old-world calls are forbidden, never adopt its brains
        }
        ++native.turns;native.grantThisTurn=false;
        cfgdonorhunt::MatchPhase phase;
        if(!DonorNativeMatchPhase(phase)||phase!=cfgdonorhunt::MatchPhase::InProgress||!bundleSchedule.active||
            bundleCompletionReady||bundleDeathEnd.confirmedAt||bundleKill.policy.interactionAttempted){DonorConvergenceCleanup();return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,4)
#else
 false
#endif
;}
        const bool vehicleIntercept=bundleCarPursuit.Active(now)||bundleVehicleActions.car||bundleCarActionPending.active;
        const bool tranceKneelRequested=bundleKill.policy.stanceAttempted&&bundleKill.policy.Exclusive(now);
        const auto finalContextUntil=bundleKill.policy.Exclusive(now)?bundleKill.policy.until:0;
        if(vehicleIntercept||tranceKneelRequested||now<finalContextUntil)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,5)
#else
 false
#endif
;
        if(warming){native.warmAt=now+250;if(!DonorConvergenceWarm())return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,6)
#else
 false
#endif
;}
        if(!bodyDue)return publicationDue;
        struct Host {
            BundleAccess& a;const cfgjasontick::Owner& owner;std::uint64_t now;
            Reader classes;cfgprobe::lifetime::ArrayHeader registry{};
            Host(BundleAccess& access,const cfgjasontick::Owner& tick,std::uint64_t time):a(access),owner(tick),now(time){classes.cap=32768;}
            bool HostedOnlineBridge(){return HostedRoutineActive();}
            bool IsHostedAuthorityWorld(){
#ifdef CFG_H1_JASON
                return HostedRoutineActive()&&a.HostedContext(false);
#else
                return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,7)
#else
 false
#endif
;
#endif
            }
            bool RegisteredUnmaskedJasonInWorld(){return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,8)
#else
 false
#endif
;}
            bool PermanentHumanSweaterLatched(){return cfgj21::EarnedSweater(bundleSweaterPolicy.nativeOwnershipEarned,bundleSweaterPolicy.ownershipObserved,bundleSweaterPolicy.ownershipHas);}
            bool KillTeamFinalInteractionCommitted(){return bundleKill.policy.interactionAttempted||bundleDeathEnd.confirmedAt;}
            int TargetCount(){return bundleDonorRoster.roster.count;}
            std::uintptr_t Target(int i){return i>=0&&i<TargetCount()?bundleDonorRoster.roster.actors[static_cast<unsigned>(i)]:0;}
            std::uintptr_t LocalCounselorTarget(){return bundleDonorRoster.roster.LocalCounselorTarget;}
            bool IsValidatedLiveCounselorPawn(std::uintptr_t p){cfgdonorhunt::CounselorObservation value;return a.DonorConvergenceRead(p,value);}
            bool HasPamelaSweater(std::uintptr_t p){Reader r;r.cap=64;std::uint8_t has=0;return r.Read(p+cfgfeature::disk::HasSweater,has)&&has==1;}
            bool IsCounselorInPostGrabTransition(std::uintptr_t p,std::uint64_t at){return a.DonorConvergencePostGrab(p,at);}
            bool IsAIControlledCounselor(std::uintptr_t p){return a.DonorConvergenceBotFor(p)!=nullptr;}
            bool ObjectDerivesFromNameContaining(std::uintptr_t p,const char* token){
                Reader r;r.cap=512;std::uintptr_t type=0;
                if(!r.Read(p+cfgprobe::ObjectClass,type))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,9)
#else
 false
#endif
;
                for(unsigned depth=0;type&&depth<16;++depth){cfgnames::Text name;if(!J21ClassName(r,type,name))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,10)
#else
 false
#endif
;
                    char value[160]{};for(unsigned i=0;i<name.count;++i)value[i]=name.data[i]<128?static_cast<char>(name.data[i]):'?';
                    if(std::strstr(value,token))return true;if(!r.Read(type+cfgprobe::ClassSuper,type))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,11)
#else
 false
#endif
;}
                return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,12)
#else
 false
#endif
;
            }
            int CurrentObjectCount(){return cfgprobe::lifetime::ReadArrayHeader(classes,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)?registry.count:0;}
            bool ReadClassCandidate(int i,std::uintptr_t& object,std::string& name){
                object=0;name.clear();std::uintptr_t meta=0,item=0,actualMeta=0;
                if(i<0||i>=registry.count||!classes.Read(registry.data+static_cast<std::uintptr_t>(i)*cfgprobe::lifetime::ItemBytes,object)||
                    !cfgprobe::ObjectPointer(object)||!classes.Read(plan.module+cfgloadout::restricted::itemdisk::SCItemClass,item)||
                    !classes.Read(item+cfgprobe::ObjectClass,meta)||!classes.Read(object+cfgprobe::ObjectClass,actualMeta)||
                    !cfgmenu::maps::Subclass(classes,actualMeta,meta))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,13)
#else
 false
#endif
;
                cfgnames::Text text;if(!J21ClassName(classes,object,text))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,14)
#else
 false
#endif
;
                if(text.View()!=u"CounselorTwoHandedAxe_C"&&text.View()!=u"CounselorMachete_C")return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,15)
#else
 false
#endif
;
                for(auto c:text.View())name.push_back(static_cast<char>(c));return true;
            }
            bool CurrentUClass(std::uintptr_t type){
                Reader r;r.cap=256;std::uintptr_t item=0;cfgprobe::lifetime::ArrayHeader current{};cfgprobe::lifetime::SlotObservation slot{};
                return cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,current)&&
                    cfgspawn::ObjectSlot(r,current,type,slot)&&r.Read(plan.module+cfgloadout::restricted::itemdisk::SCItemClass,item)&&
                    cfgobserver::Same(r,plan.module+cfgloadout::restricted::itemdisk::SCItemConstructor,item)&&cfgmenu::maps::Subclass(r,type,item);
            }
            bool GiveStartingItem(std::uintptr_t pawn,std::uintptr_t type){return a.DonorConvergenceGrant(pawn,type);}
            bool GiveConvergenceMeleeWeapon(std::uintptr_t pawn){return bundleDonorConvergence.donor.GiveConvergenceMeleeWeapon(*this,pawn,now);}
            void SuppressConvergenceCounselorFear(std::uintptr_t pawn){
                if(!a.DonorConvergenceBotFor(pawn))return;cfgconvergencefear::Evidence proof;
                if(cfgconvergencefear::Capture(a.r_,a.context.objectArray,plan.module,pawn,proof)&&proof.amount!=0)
                    if(a.CarSpeedWrite(proof.manager.object+cfghook::disk::FearValue,proof.amount,0.f))++bundleConvergenceFearWrites;
            }
            int Key(const char* text){for(unsigned i=0;i<cfgconvergence::KeyCount;++i){const auto wide=cfgconvergence::KeyText[i];unsigned n=0;
                while(text[n]&&wide[n]&&text[n]==wide[n])++n;if(!text[n]&&!wide[n]&&bundleConvergenceNames[i].index>=0)return static_cast<int>(i);}return -1;}
            std::uintptr_t GetPawnControllerSafe(std::uintptr_t p){auto* b=a.DonorConvergenceBotFor(p);return b?b->controller.object:0;}
            std::uintptr_t GetCounselorBlackboardCached(std::uintptr_t controller){
                for(auto& b:bundleDonorConvergence.bots)if(b.controller.object==controller){std::uintptr_t blackboard=0;
                    if(a.DonorConvergenceBlackboard(b,blackboard))return blackboard;}return 0;
            }
            DonorConvergenceBot* ForBlackboard(std::uintptr_t blackboard){for(auto& b:bundleDonorConvergence.bots)if(b.blackboard.object==blackboard)return &b;return nullptr;}
            std::uintptr_t GetCounselorCurrentWeapon(std::uintptr_t p){return a.DonorConvergenceWeapon(p);}
            std::uintptr_t GetBlackboardObject(std::uintptr_t blackboard,int key){
                auto* b=ForBlackboard(blackboard);if(!b||key<0||!b->schema.Supports(static_cast<unsigned>(key))||!b->schema.Current(a.r_,a.context.objectArray))return 0;
                cfgconvergence::ObjectPacket packet{bundleConvergenceNames[key],0};
                return a.ConvergenceInvoke(1,blackboard,&packet)&&(!packet.value||a.HeldWeapon(b->pawn.object,packet.value))?packet.value:0;
            }
            void SetBlackboardObject(std::uintptr_t blackboard,int key,std::uintptr_t value){auto* b=ForBlackboard(blackboard);
                if(b&&key>=0&&key<=1&&!(b->uncertainKeys&(1u<<key))){b->objects[key]=value;b->pending|=1u<<key;}}
            void SetBlackboardBool(std::uintptr_t blackboard,int key,bool value){auto* b=ForBlackboard(blackboard);
                if(b&&key>=2&&key<static_cast<int>(cfgconvergence::KeyCount)&&!(b->uncertainKeys&(1u<<key))){b->booleans[key]=value;b->pending|=1u<<key;}}
            void ArmKillTeamCounselorCombat(std::uintptr_t pawn){
                const auto controller=GetPawnControllerSafe(pawn),blackboard=GetCounselorBlackboardCached(controller),weapon=GetCounselorCurrentWeapon(pawn);
                if(!blackboard||!weapon)return;
                SetBlackboardObject(blackboard,Key("SCWeapon"),weapon);SetBlackboardObject(blackboard,Key("JasonCharacter"),GetJasonDeathTarget());
                for(unsigned i=cfgconvergence::Fight;i<cfgconvergence::KeyCount;++i)
                    SetBlackboardBool(blackboard,static_cast<int>(i),cfgconvergence::Desired(static_cast<cfgconvergence::Key>(i),cfgconvergence::Action::Publish));
            }
            std::uintptr_t GetJasonDeathTarget(){return bundleJason.pawn.object;}
            bool GetJasonAIActorLocation(std::uintptr_t p,cfgdonorconvergence::Position& out){
                Reader r;r.cap=128;cfgprobe::lifetime::SlotObservation slot{};cfgprobe::lifetime::ArrayHeader current{};
                if(!cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,current)||!cfgspawn::ObjectSlot(r,current,p,slot))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,16)
#else
 false
#endif
;
                cfgspawn::Vec3 point{};if(!a.DonorNativePosition(slot,point))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,17)
#else
 false
#endif
;out={point.x,point.y,point.z};return true;
            }
            bool SetCounselorBrainTickEnabled(std::uintptr_t p,bool enabled){return a.DonorConvergenceBrain(p,enabled);}
            bool IssueAIMoveToLocationOnGameThread(std::uintptr_t controller,const cfgdonorconvergence::Position& point,float acceptance,const char*){
                for(auto& b:bundleDonorConvergence.bots)if(b.controller.object==controller&&b.brainOwned&&a.DonorConvergenceBotFor(b.pawn.object)==&b){
                    cfgprobe::lifetime::SlotObservation brain{};
                    if(!cfgconvergencetravel::Brain(a.r_,a.context.objectArray,b.controller,spawnAssets.resources.classes[cfgspawn::CounselorAI],
                        spawnAssets.resources.classes[cfgspawn::Brain],brain)||!(brain==b.brain))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,18)
#else
 false
#endif
;
                    std::uint8_t ticking=2;namespace d=cfgconvergencetravel::disk;
                    if(!a.CarCall(d::IsComponentTickEnabled,d::IsComponentTickEnabledExecutor,d::IsComponentTickEnabledCode,brain.object,&ticking)||ticking)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,19)
#else
 false
#endif
;
                    cfgtrap::MoveLocationParameters packet{};packet.destination={point.X,point.Y,point.Z};packet.acceptance=acceptance;packet.result=255;
                    return a.CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,controller,&packet)&&packet.result>0&&packet.result<=2;
                }return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,20)
#else
 false
#endif
;
            }
            std::string JasonAISafeName(std::uintptr_t){return "current-counselor";}
            void Log(const std::string&){} // bounded metrics replace donor per-bot file spam
        } host{*this,owner,now};
        native.donor.DriveCounselorSweaterConvergence(host,now);
        if(now<native.publishAt)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,21)
#else
 false
#endif
;
        for(const auto& bot:native.bots)if(bot.pending)return true;
        return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Convergence,22)
#else
 false
#endif
;
    }
#endif
