// Inside BundleAccess. Native pickup queries only observe their engine-owned inputs.
    bool SweaterPickupSource(bool observation=false){
        auto& s=bundleSweaterPickup;
        cfgsweater::PickupList list{},again{};unsigned currentIndex=~0u;
        std::uintptr_t item=s.source.object,level=0,levelType=0,defaults=0,type=0;
        cfgprobe::lifetime::SlotObservation source,freshLevel;
        return s.world==plan.world&&s.module==plan.module&&(observation?SummaryObservationScope():Context())&&
            TypedFeature(spawnAssets.gameState,cfgfeature::disk::GameState,false)&&
            cfgsweater::LocatePickup(r_,spawnAssets.gameState+cfgfeature::disk::WorldItems,item,list,currentIndex)&&
            TypedFeature(item,cfgfeature::disk::Sweater,false)&&cfgspawn::ObjectSlot(r_,context.objectArray,item,source)&&source==s.source&&
            r_.Read(item+cfgprobe::ObjectClass,type)&&r_.Read(type+cfgspawn::ClassDefaultObject,defaults)&&cfgprobe::ObjectPointer(defaults)&&defaults!=item&&
            r_.Read(item+0x20,level)&&level==s.level.object&&
            NativeClass(cfgkill::disk::LevelReflection,cfgkill::disk::LevelConstructor,levelType)&&cfgspawn::ObjectIs(r_,level,levelType)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,level,freshLevel)&&freshLevel==s.level&&
            cfgobserver::Same(r_,level+cfgkill::disk::OwningWorld,plan.world)&&
            r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,again)&&again==list&&
            cfgobserver::Same(r_,list.data+8ULL*currentIndex,item)&&cfgobserver::Same(r_,item+0x20,level);
    }
    bool SweaterPickupAllowed(std::uintptr_t item,std::uintptr_t counselor){
        auto& s=bundleSweaterPickup;
        s.lastFailure=6;
        if(!cfgsweater::pickupdisk::RuntimeVerified)return false;
        if(counselor!=bundleSweaterOwner.object||!cfgsweater::PickupScope(s,plan.module,plan.world,item)||!BundleGameplayStarted()||!bundleSchedule.active)return false;
        s.lastFailure=7;if(!SweaterPickupSource(true))return false;
        s.lastFailure=8;if(!KnownLiving(counselor))return false;
        s.lastFailure=9;if(!cfgobserver::Same(r_,item,s.clone+8))return false;
        s.lastFailure=10;if(!cfgbundle::Code(r_,plan.module,cfgsweater::pickupdisk::Original,cfgsweater::pickupdisk::OriginalCode)||
            !cfgbundle::Code(r_,plan.module,cfgsweater::pickupdisk::Base,cfgsweater::pickupdisk::BaseCode)||
            !SummaryObservationScope()||!cfgprimary::LookupVetoClearForObservation(r_,plan.module))return false;
        s.lastFailure=0;
        return true; // caller still invokes the full native base predicate and records its attempt
    }
