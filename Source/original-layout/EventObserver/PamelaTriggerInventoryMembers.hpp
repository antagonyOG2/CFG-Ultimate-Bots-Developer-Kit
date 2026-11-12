    bool HasPamelaTrigger(bool& acquired){
        acquired=false;unsigned char has=0;std::uintptr_t ability=0,pawn=0;
        if(!Known(0,pawn)||!r_.Read(pawn+cfgfeature::disk::HasSweater,has)||has>1||
            !r_.Read(pawn+cfgfeature::disk::AbilityField,ability))return false;
        if(has&&ability&&TypedFeature(ability,cfgfeature::disk::Ability,false)&&
            cfgobserver::Same(r_,ability+cfgfeature::disk::AbilityOwner,pawn)&&
            cfgobserver::Same(r_,pawn+cfgfeature::disk::AbilityField,ability)){acquired=true;return true;}
        // Two pawn-owned inventory lists only; never scan GObjects or world items.
        struct List{std::uintptr_t data=0;std::int32_t count=0,capacity=0;
            bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
        if(HostedRoutineActive()&&cfgh241mask::memo.world==plan.world&&cfgh241mask::memo.pawn==pawn){
            auto& m=cfgh241mask::memo;std::uintptr_t inventory=0,item=0,itemClass=0;
            List list{};cfgprobe::lifetime::SlotObservation inventorySlot{},itemSlot{};
            const bool current=r_.Read(pawn+m.inventoryOffset,inventory)&&inventory==m.inventory&&
                TypedFeature(inventory,cfgfeature::disk::Inventory,false)&&
                cfgobserver::Same(r_,inventory+0x20,pawn)&&cfgobserver::Same(r_,inventory+0xE0,pawn)&&
                r_.Read(inventory+cfgfeature::disk::InventoryItems,list)&&list.data==m.listData&&
                list.count==m.itemCount&&list.capacity==m.itemCapacity&&m.itemIndex<static_cast<unsigned>(list.count)&&
                r_.Read(list.data+8*m.itemIndex,item)&&item==m.item&&
                TypedFeature(item,cfgfeature::disk::Item,false)&&
                cfgobserver::Same(r_,item+cfgfeature::disk::OuterInventory,inventory)&&
                r_.Read(item+cfgprobe::ObjectClass,itemClass)&&itemClass==m.itemClass&&
                cfgspawn::ObjectSlot(r_,context.objectArray,inventory,inventorySlot)&&inventorySlot==m.inventorySlot&&
                cfgspawn::ObjectSlot(r_,context.objectArray,item,itemSlot)&&itemSlot==m.itemSlot&&
                r_.Read(inventory+cfgfeature::disk::InventoryItems,list)&&list.data==m.listData&&
                list.count==m.itemCount&&list.capacity==m.itemCapacity&&
                cfgobserver::Same(r_,pawn+m.inventoryOffset,inventory)&&
                cfgobserver::Same(r_,list.data+8*m.itemIndex,item);
            if(current){acquired=true;return true;}
            m.Clear();
        }
        std::uintptr_t maskClass=0;bool nativeMaskClass=false;
#ifdef CFG_H1_JASON
        nativeMaskClass=NativeClass(cfgfeature::disk::Classes[cfgfeature::disk::Mask].reflection,
            cfgfeature::disk::Classes[cfgfeature::disk::Mask].constructor,maskClass);
        // Keep the offline path byte-for-byte fail-closed when its exact native
        // mask cache is cold. Hosted has an independent typed-item + owned
        // inventory + donor-class-name path below; names never grant call authority.
        if(!nativeMaskClass&&!HostedRoutineActive())return false;
#else
        nativeMaskClass=NativeClass(cfgfeature::disk::Classes[cfgfeature::disk::Mask].reflection,
            cfgfeature::disk::Classes[cfgfeature::disk::Mask].constructor,maskClass);
        if(!nativeMaskClass)return false;
#endif
        for(const auto offset:{cfgfeature::disk::SpecialInventory,cfgfeature::disk::SmallInventory}){
            std::uintptr_t inventory=0;if(!r_.Read(pawn+offset,inventory))return false;if(!inventory)continue;
            // Serial0 is an allowed read-only observation of a strongly held
            // pawn component, not a weak identity or a serial-allocation request.
            if(!TypedFeature(inventory,cfgfeature::disk::Inventory,false)||
                !cfgobserver::Same(r_,inventory+0x20,pawn)||!cfgobserver::Same(r_,inventory+0xE0,pawn))return false;
            List list{},fresh{};if(!r_.Read(inventory+cfgfeature::disk::InventoryItems,list)||list.count<0||
                list.capacity<list.count||list.capacity>32||(list.count&&!cfgprobe::ObjectPointer(list.data)))return false;
            for(int i=0;i<list.count;++i){
                std::uintptr_t item=0;if(!r_.Read(list.data+8*static_cast<unsigned>(i),item))return false;
                if(!item)continue;
                // Limit the expensive positive-lifetime check to candidates of
                // the native mask class; unrelated carried items need no calls.
                const bool nativeMask=nativeMaskClass&&cfgspawn::ObjectIs(r_,item,maskClass);
                const bool hostedMask=!nativeMask&&HostedMaskPickup(item);
                if(!nativeMask&&!hostedMask)continue;
                if(!(nativeMask?TypedFeature(item,cfgfeature::disk::Mask,false):hostedMask)||!cfgobserver::Same(r_,item+cfgfeature::disk::OuterInventory,inventory)||
                    !cfgobserver::Same(r_,list.data+8*static_cast<unsigned>(i),item)||
                    !r_.Read(inventory+cfgfeature::disk::InventoryItems,fresh)||!(fresh==list)||
                    !cfgobserver::Same(r_,pawn+offset,inventory))return false;
                acquired=true;
                if(HostedRoutineActive()){
                    cfgprobe::lifetime::SlotObservation inventorySlot{},itemSlot{};std::uintptr_t itemClass=0;
                    if(cfgspawn::ObjectSlot(r_,context.objectArray,inventory,inventorySlot)&&
                       cfgspawn::ObjectSlot(r_,context.objectArray,item,itemSlot)&&r_.Read(item+cfgprobe::ObjectClass,itemClass))
                        cfgh241mask::memo={plan.world,pawn,inventory,item,itemClass,
                            list.data,static_cast<std::uint32_t>(offset),static_cast<std::uint32_t>(i),list.count,list.capacity,inventorySlot,itemSlot};
                }
                return true;
            }
        }
        return true;
    }

