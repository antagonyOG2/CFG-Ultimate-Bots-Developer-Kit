// Native pickup retires its instance in CFG3.5 (PID17748). Keep ONLY the earned
// exact class/CDO for this match, using the existing named/rooted owner pool.
// No new holder, bootstrap reopening, serial allocation or world scan.
    bool SweaterClassMetadataCurrent(){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return bundleSweaterClass.anchor==8&&
            primaryRoots.CurrentReference(r_,bundleSweaterClass.slot.object)&&
            primaryRoots.CurrentReference(r_,bundleSweaterClass.defaultSlot.object);
#else
        return false;
#endif
    }
    bool SweaterClassCurrent(){
        const bool hosted=HostedRoutineActive()&&bundleSweaterClass.anchor>=7&&bundleSweaterClass.anchor<=9&&
            cfgh241sweater::Matches(cfgh241sweater::memo,plan.world,plan.module,bundleSweaterClass.slot.object,
                bundleSweaterClass.defaults,bundleSweaterClass.slot,bundleSweaterClass.defaultSlot);
        if(hosted){
            cfgprobe::lifetime::SlotObservation classSlot{},cdoSlot{};std::uintptr_t native=0,ctor=0,cdo=0;
            const auto& binding=cfgfeature::disk::Classes[cfgfeature::disk::Ability];
            if(cfgprimary::LookupSafe(r_,plan.module)&&
               r_.Read(plan.module+binding.reflection,native)&&r_.Read(plan.module+binding.constructor,ctor)&&
               native==bundleSweaterClass.native&&ctor==native&&
               cfgspawn::ObjectSlot(r_,context.objectArray,bundleSweaterClass.slot.object,classSlot)&&
               classSlot==cfgh241sweater::memo.classSlot&&
               r_.Read(bundleSweaterClass.slot.object+cfgspawn::ClassDefaultObject,cdo)&&cdo==bundleSweaterClass.defaults&&
               cfgspawn::ObjectSlot(r_,context.objectArray,cdo,cdoSlot)&&cdoSlot==cfgh241sweater::memo.cdoSlot&&
               cfgobserver::Same(r_,cdo+cfgprobe::ObjectClass,bundleSweaterClass.slot.object)&&
               cfgspawn::ClassContains(r_,bundleSweaterClass.slot.object,native)&&
               (bundleSweaterClass.anchor!=8||(primaryRoots.CurrentReference(r_,classSlot.object)&&
                 primaryRoots.CurrentReference(r_,cdoSlot.object)))&&cfgprimary::LookupSafe(r_,plan.module))return true;
            cfgh241sweater::memo.Clear();
        }
        const bool current=bundleSweaterClass.Current(r_,context.objectArray,plan.module,SweaterClassMetadataCurrent());
        if(current&&HostedRoutineActive()&&bundleSweaterClass.anchor>=7&&bundleSweaterClass.anchor<=9&&
           (bundleSweaterClass.anchor==8||bundleSweaterClass.worldSource.world==plan.world)){
            cfgh241sweater::memo={plan.world,plan.module,bundleSweaterClass.slot.object,bundleSweaterClass.defaults,
                bundleSweaterClass.slot,bundleSweaterClass.defaultSlot};
        }
        return current;
    }
    bool RetainSweaterClassMetadata(){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const auto values=std::array<std::uintptr_t,2>{bundleSweaterClass.slot.object,bundleSweaterClass.defaultSlot.object};
        const bool kept=WithBundleRootWindow([&](GcRegistryAccess& registry)noexcept{
            cfggcprobe::Report registration;
            return bundleSweaterClass.Current(r_,context.objectArray,plan.module)&&
                primaryRoots.AddLocked(registry,values,registration);
        });
        return kept&&primaryRoots.CurrentReference(r_,values[0])&&primaryRoots.CurrentReference(r_,values[1]);
#else
        return false;
#endif
    }
