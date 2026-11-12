// Included inside BundleAccess. Only registered living helper counselors may
// reach this donor-native GiveStartingItem path; local counselor is excluded.
    bool ConvergenceAxePhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Axes);
#endif

        if(cfgj21::ConvergenceEnabled&&DonorHeldDecisionInstalled())return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Axes,1)
#else
 false
#endif
;
        if(cfgj21::LeanEnabled)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Axes,2)
#else
 false
#endif
; // lean uses the guarded Observe/Grant/Verify gateway
        struct Host {
            BundleAccess& a;unsigned index=0;
            void Select(unsigned i){index=i;}
            bool Pause(unsigned i)const{return BundleEmoteHeld(i);}
            std::uintptr_t LoadedAxeHint(){
                cfgnames::Observation<Reader> names(a.r_,plan.module);cfgnames::Text text;
                const auto exact=[&](std::uintptr_t type){return cfgprobe::ObjectPointer(type)&&names.Object(type,text)&&
                    text.View()==cfgloadout::ClassName(cfgloadout::Slot::Weapon,3);};
                const auto cached=bundleExpandedLoadout.report.type;
                if(exact(cached))return cached;
                std::uintptr_t item=0,type=0;
                if(cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[3])&&
                   a.r_.Read(plan.pawn+cfgloadout::restricteddisk::EquippedItem,item)&&cfgprobe::ObjectPointer(item)&&
                   cfgobserver::Same(a.r_,item+cfgloadout::restricteddisk::OwningCharacter,plan.pawn)&&
                   a.r_.Read(item+cfgprobe::ObjectClass,type)&&exact(type))return type;
                return 0;
            }
            bool Owner(cfgofflinesettings::Request& out){
                std::uintptr_t pawn=0;if(BundleEmoteHeld(index)||!a.Context()||!a.Known(index+1,pawn))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Axes,3)
#else
 false
#endif
;
                out={bundleConfiguration.submission,plan.world,pawn,plan.objects[0].serial,bundlePawnSlots[index].serial};
                return out.Valid();
            }
            bool Current(const cfgofflinesettings::Request& request){
                std::uintptr_t pawn=0;
                return index<bundleReport.bots&&!BundleEmoteHeld(index)&&request.Valid()&&request.submission==bundleConfiguration.submission&&
                    request.world==plan.world&&request.worldSerial==plan.objects[0].serial&&
                    request.pawn==bundleReport.roster[index].pawn&&request.pawn!=plan.pawn&&
                    request.pawnSerial==bundlePawnSlots[index].serial&&a.Context()&&a.Known(index+1,pawn)&&pawn==request.pawn;
            }
            bool InvokeGrant(std::uintptr_t entry,std::uintptr_t pawn,std::uintptr_t type)noexcept{
                return a.ConvergenceAxeGrant(index,entry,pawn,type);
            }
        }host{*this};
        return bundleConvergenceAxes.Step(r_,host,plan.module,plan.world,bundleReport.bots,bundleConvergence.active,now,!bundleHelpers.Active());
    }
    bool ConvergenceAxeGrant(unsigned index,std::uintptr_t entry,std::uintptr_t pawn,std::uintptr_t type)noexcept{
        if(index>=bundleReport.bots||pawn!=bundleReport.roster[index].pawn||pawn==plan.pawn||
           entry!=plan.module+cfgmenu::loadoutdisk::GiveStartingItem||!NativeCallAllowed())return false;
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(entry)(reinterpret_cast<void*>(pawn),reinterpret_cast<void*>(type));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
