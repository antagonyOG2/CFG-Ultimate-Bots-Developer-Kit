    bool BindOwnedSweater(std::uintptr_t ability){
        if(!TypedFeature(ability,cfgfeature::disk::Ability,false)||
            !cfgobserver::Same(r_,ability+cfgfeature::disk::AbilityOwner,bundleSweaterOwner.object)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,ability,bundleSweaterAbility))return false;
        // FJB MaintainPermanentHumanSweater: latch the earned native power at
        // pickup, not after it has already consumed/retired its instance.
        // Class capture has no allocation and cannot suppress native use.
        std::uintptr_t earnedType=0;
        if(!r_.Read(ability+cfgprobe::ObjectClass,earnedType)||bundleSweaterClass.slot.object!=earnedType||
           !bundleSweaterClassAttempted||!SweaterClassCurrent())
            bundleSweaterClassPending=!CaptureEarnedSweaterClass();
        bundleSweaterSource={};bundleSweaterSourceIndex=~0u;
        std::uintptr_t outer=0,wearer=0;
        if(r_.Read(ability+0x20,outer)&&outer&&TypedFeature(outer,cfgfeature::disk::Sweater,false)&&
            r_.Read(outer+cfgloadout::restricteddisk::OwningCharacter,wearer)&&(!wearer||wearer==bundleSweaterOwner.object)&&
            cfgobserver::Same(r_,outer+cfgfeature::disk::SweaterAbility,ability)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,outer,bundleSweaterSource)){
            // One cold lookup for the exact already-owned source, not discovery
            // of arbitrary abilities. At most256 cells, never repeated per tick.
            struct List{std::uintptr_t data=0;std::int32_t count=0,capacity=0;};List list{};
            if(r_.Read(spawnAssets.gameState+cfgfeature::disk::WorldItems,list)&&list.count>=0&&list.count<=256&&
                list.capacity>=list.count&&list.capacity<=65536&&(!list.count||cfgprobe::ObjectPointer(list.data))){
                unsigned matches=0,index=~0u;
                for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){std::uintptr_t item=0;
                    if(!r_.Read(list.data+8*i,item))return false;if(item==outer){++matches;index=i;}}
                if(matches==1&&cfgsweater::StrongSource(r_,context.objectArray,plan.world,spawnAssets.gameState,bundleSweaterOwner.object,
                    bundleSweaterSource,bundleSweaterAbility,index))bundleSweaterSourceIndex=index;
            }
        }
        return true;
    }
