#ifndef CFG_H247_F1_HOSTED
#define CFG_H247_F1_HOSTED 1
#endif
#ifndef CFG_H248_F1_ADOPTED
#define CFG_H248_F1_ADOPTED 1
#endif
#ifdef CFG_H1_JASON
    // H2.61: hosted F1 random Jason. Donor OfflineSetup.cpp:10421-10427 samples the mode's KillerCharacterClasses at a random index.
    // Only classes ALREADY resident are usable here: the pinned Conv_SoftClassReferenceToClass (the same call offline selection makes) returns
    // null for a cold class and no synchronous load binding is pinned (LoadAssetClass is latent), so a cold entry is skipped, never loaded.
    // Reasons: 1 no own class, 2 mode/world, 3 list unstable, 4 empty list, 5 conversion binding, 6 slots, 7 no usable distinct class.
    // H2.62: donor LoadLifecycleSoftClass. Same pinned loader as the offline startup (NativeSoftClassLoader.hpp), through the hosted retained-call gateway.
    // A hitch on F1 is expected. An unexpected null closes the window; a faulted load poisons as any uncertain native does. The class is re-proved later by AdditionalTemplate.
    bool HostedLoadJasonClass(const std::array<unsigned char,40>& entry,std::uintptr_t& object){
        namespace d=cfgmenu::softloaddisk;object=0;cfgmenu::promotion::Packet source{},owned{};source.bytes=entry;std::array<wchar_t,1024> path{};
        if(!cfgmenu::population::softdetail::OwnPath(r_,source,owned,path)||
           !cfgbundle::Code(r_,plan.module,d::LoadSoftObject,d::LoadSoftObjectCode)||!cfgbundle::Code(r_,plan.module,d::LoadSoftPath,d::LoadSoftPathCode)||
           !cfgbundle::Code(r_,plan.module,d::StaticLoadObject,d::StaticLoadObjectCode)||!HostedPrepare(HostedRoots(),1))return false;
        ++calls_;
        if(!cfgmenu::population::softdetail::Load(plan.module+d::LoadSoftObject,owned,object)){BundleRetentionFailed();return false;}
        if(!object){(void)FinishBundleNative();return false;}
        return RetainBundleReturnedObject(object)&&HostedWorld();
    }
    // H2.62: live H2.61 ended reason 7 with no entry ever null: the multi-entry search skips an entry when a callback's read budget runs out.
    // One entry per callback, loaded directly as donor OfflineSetup.cpp:10423-10424 does (the loader returns a resident class too).
    bool HostedRandomJasonStep(cfgspawn::TemplateObservation& out,std::array<unsigned char,40>& soft,unsigned& chosen,unsigned& why){
        namespace c=cfgmenu::catalogues;namespace d=cfgmenu::cataloguedisk;auto& s=bundleAdditional;
        std::uintptr_t own=0,native=0,ctor=0,type=0,loaded=0;c::CharacterList first,second;
        why=1;if(!Jason()||!r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,own)||!own)return false;
        why=2;if(!HostedWorld()||!r_.Read(plan.module+d::GameModeClass,native)||!r_.Read(plan.module+d::GameModeConstructor,ctor)||
            !cfgprobe::MatchingClassCache(native,ctor)||!r_.Read(plan.mode+cfgprobe::ObjectClass,type)||!cfgmenu::maps::Subclass(r_,type,native))return false;
        why=3;if(!c::CharacterSample(r_,plan.mode+d::KillerCharacterClasses,first)||!c::CharacterSample(r_,plan.mode+d::KillerCharacterClasses,second)||!(first==second))return false;
        const unsigned count=static_cast<unsigned>(first.header.count);
        why=4;if(!count)return false;
        if(s.hostedRandomTries==1)s.hostedRandomSeed=static_cast<unsigned>(GetTickCount64()%count);
        const unsigned index=(s.hostedRandomSeed+s.hostedRandomTries-1)%count;const auto& entry=first.entries[index];cfgprimary::ConversionParameters packet{};
        cfgclosedtrace::Mark("f1-h262-entry",count,(static_cast<std::uint64_t>(index)<<32)|Reads());
        why=5;if(s.hostedRandomTries>count)return false;
        why=6;if(!c::Nonempty(entry)||!cfgprimary::BuildConversion(entry,packet))return false; // empty subpath is proven here for the birth stage
        why=9;if(!HostedLoadJasonClass(entry,loaded))return false;
        why=7;if(loaded==own)return false;
        cfgspawn::TemplateObservation candidate{};
        why=8;if(!cfgprimary::ObserveResolvedTemplate(r_,context.objectArray,loaded,spawnAssets.killerBase,spawnAssets.resources.classes[cfgspawn::BaseAI],candidate)||candidate.pawnClass==own)return false;
        out=candidate;soft=entry;chosen=index;why=0;return true;
    }
#endif
    // One resident native conversion per callback. Never changes primary/CDO,
    // loads a cold class or allocates an actor while searching.
    unsigned AdditionalSelect(){
        auto& s=bundleAdditional;const auto index=s.selection++;
        // H2.93: a hosted match never captures this catalogue; one still present is an earlier OFFLINE match's (live: offline then
        // online, F1 refused "no compatible resident class" on the stale entries). Hosted selection treats it as absent.
        const std::int32_t catalogueCount=(CFG_H293_HOSTED_STALE_CATALOGUE&&HostedRoutineActive())?0:spawnAssets.primary.catalogue.count;
#if CFG_H251_HOSTED_F1
        s.hostedOwnClass=false;
#endif
        if(index>=static_cast<unsigned>(catalogueCount)){
            if(s.policy.choices&&s.selectedIndex<static_cast<unsigned>(catalogueCount))return 1u;
            // User rule: F1 is a different Jason whenever one exists. A hosted
            // one-entry KillerCharacterClasses list may offer only the current
            // class; use it only then (donor F1 samples that list).
            if(s.sameClassFallbackIndex<static_cast<unsigned>(catalogueCount)){
                s.selected=s.sameClassFallback;s.selectedIndex=s.sameClassFallbackIndex;return 1u;}
#if CFG_H251_HOSTED_F1
            // H2.51: the hosted match never captures spawnAssets.primary.catalogue (that capture is the
            // offline OfflineBots/Sandbox startup only), so count==0 and the H2.47/H2.48 fallbacks below
            // (both gated on count>0) were unreachable: live "SelectRefusal=1 SelectIndex=1". The hosted
            // adoption already filled killerBase, resources.classes[] and the adopted pawn template; use
            // the adopted Jason's own resident class, re-validated like any catalogue candidate. Never
            // loads a class; the birth stages re-prove it through AdditionalTemplate (hostedOwnClass).
#ifdef CFG_H1_JASON
#if CFG_H261_HOSTED_RANDOM_JASON
            if(HostedRoutineActive()&&catalogueCount==0&&s.hostedRandomTries<5){
                ++s.hostedRandomTries; // one entry per callback, fresh read budget each time
                cfgspawn::TemplateObservation random{};std::array<unsigned char,40> soft{};unsigned chosen=0,why=0;
                if(HostedRandomJasonStep(random,soft,chosen,why)){
                    s.selected=random;s.selectedIndex=0;s.hostedOwnClass=false;s.hostedRandom=true;s.hostedRandomSoft=soft;
                    cfgclosedtrace::Mark("f1-h261-random-chosen",random.pawnClass,chosen);return 1u;}
                cfgclosedtrace::Mark("f1-h262-step-refused",s.hostedRandomTries,(static_cast<std::uint64_t>(why)<<32)|Reads());
                if(s.hostedRandomTries<5&&why>=5)return 0u; // next entry on the next callback
                cfgclosedtrace::Mark("f1-h261-random-fallback",0,why);
            }
#endif
#endif
            if(catalogueCount==0&&HostedRoutineActive()){
                std::uintptr_t own=0;cfgspawn::TemplateObservation candidate{};
                if(Jason()&&r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,own)&&own&&
                   cfgprimary::ObserveResolvedTemplate(r_,context.objectArray,own,spawnAssets.killerBase,
                       spawnAssets.resources.classes[cfgspawn::BaseAI],candidate)){
                    s.selected=candidate;s.selectedIndex=0;s.hostedOwnClass=true;return 1u;}
            }
#endif
#if CFG_H247_F1_HOSTED
            // H2.47: hosted catalogue may hold only classes not resident on the host (conversion
            // returns null). Nothing else usable: use the primary AI Jason's own resident template;
            // index 0 only anchors the entry-unchanged check.
            if(catalogueCount>0&&JasonTemplate().pawnClass){
                s.selected=JasonTemplate();s.selectedIndex=0;return 1u;}
#endif
#if CFG_H248_F1_ADOPTED
            // H2.48: hosted primary template is never filled (offline startup only). Use the
            // adopted Jason's own pawn class, validated like any catalogue candidate.
            if(catalogueCount>0&&HostedRoutineActive()&&!JasonTemplate().pawnClass){
                std::uintptr_t own=0;cfgspawn::TemplateObservation candidate{};
                if(Jason()&&r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,own)&&own&&
                   cfgprimary::ObserveResolvedTemplate(r_,context.objectArray,own,spawnAssets.killerBase,
                       spawnAssets.resources.classes[cfgspawn::BaseAI],candidate)){
                    s.selected=candidate;s.selectedIndex=0;return 1u;}
            }
#endif
            s.selectionRefusal=1;return 2u;
        }
        cfgprimary::ConversionParameters packet{};cfgprobe::lifetime::SlotObservation slot{};
        if(!Context()){s.selectionRefusal=2;return 2;}
        if(!Jason()){s.selectionRefusal=3;return 2;}
        if(!AdditionalInProgress()){s.selectionRefusal=4;return 2;}
        if(!cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,index)){s.selectionRefusal=5;return 2;}
        if(!cfgprimary::BuildConversion(spawnAssets.primary.entries[index],packet)){++s.selectionConversionFailures;return 0;}
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.conversionFunction,slot)){s.selectionRefusal=6;return 2;}
        if(!cfgbundle::OwnedClassSlotMatches(slot,spawnAssets.conversionSlot)){s.selectionRefusal=7;return 2;}
        if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::Conversion.cache,spawnAssets.conversionFunction)){s.selectionRefusal=8;return 2;}
        if(!cfgobserver::Same(r_,spawnAssets.conversionFunction+0x20,spawnAssets.kismetClass)){s.selectionRefusal=9;return 2;}
        if(!cfgobserver::Same(r_,spawnAssets.kismetClass+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault)){s.selectionRefusal=10;return 2;}
        if(!cfgobserver::Same(r_,spawnAssets.kismetDefault+cfgprobe::ObjectClass,spawnAssets.kismetClass)){s.selectionRefusal=11;return 2;}
        if(!cfgbundle::Code(r_,plan.module,cfgprimary::disk::Conversion.factory,cfgprimary::disk::Conversion.prefix)){s.selectionRefusal=12;return 2;}
        if(!Invoke(spawnAssets.kismetDefault,spawnAssets.conversionFunction,&packet,false)){s.selectionRefusal=13;return 2;}
        if(!Context()){s.selectionRefusal=14;return 2;}
        if(!cfgprimary::SameEntry(r_,plan.mode,spawnAssets.primary,index)){s.selectionRefusal=15;return 2;}
        if(!packet.result)return 0;
        cfgspawn::TemplateObservation candidate{};
        if(!cfgprimary::ObserveResolvedTemplate(r_,context.objectArray,packet.result,spawnAssets.killerBase,
            spawnAssets.resources.classes[cfgspawn::BaseAI],candidate)){++s.selectionTemplateFailures;return 0;}
        if(packet.result==JasonTemplate().pawnClass){
            if(s.sameClassFallbackIndex==~0u){s.sameClassFallback=candidate;s.sameClassFallbackIndex=index;}
            return 0;
        }
        if(s.policy.Choose(s.policy.edgeAt)){s.selected=candidate;s.selectedIndex=index;}
        return 0;
    }
