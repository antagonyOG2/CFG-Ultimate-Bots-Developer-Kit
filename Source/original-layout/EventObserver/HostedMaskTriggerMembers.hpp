    bool HostedMaskPickup(std::uintptr_t item){
#ifdef CFG_H1_JASON
        if(!HostedRoutineActive()||!TypedFeature(item,cfgfeature::disk::Item,false))return false;
        std::uintptr_t type=0,fresh=0;cfgnames::Text className,itemName;cfgnames::Observation<Reader> names(r_,plan.module);
        if(!r_.Read(item+cfgprobe::ObjectClass,type))return false;
        // The donor identifies a held mask by either the item instance name or
        // its class name. Some inventory pickups have a generic class but keep
        // the identifying MaskPickup name on the instance itself.
        const bool classMatch=names.Object(type,className)&&cfghostedmask::PickupClass(className.View());
        const bool itemMatch=names.Object(item,itemName)&&cfghostedmask::PickupClass(itemName.View());
        return (classMatch||itemMatch)&&r_.Read(item+cfgprobe::ObjectClass,fresh)&&fresh==type;
#else
        (void)item;return false;
#endif
    }
