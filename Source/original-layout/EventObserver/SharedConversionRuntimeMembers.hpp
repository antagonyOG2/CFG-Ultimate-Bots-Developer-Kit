// Included by the primary SpawnAccess and composed resident-selection fixture.
    bool PrepareSharedConversion() {
        std::uintptr_t function=0;
        if (!ContextMatches() || !ResourcesMatch() || !r_.Read(plan.module+cfgprimary::disk::Conversion.cache,function)) return false;
        if (!function) {
            std::array<unsigned char,47> prefix{};
            if (calls_>=cfgspawn::MaximumNativeCalls || !r_.Read(plan.module+cfgprimary::disk::Conversion.factory,prefix) ||
                prefix!=cfgprimary::disk::Conversion.prefix || !ContextMatches() || !ResourcesMatch()) return false;
#ifdef CFG_RETAINED_PRIMARY_JASON
            if(!PrimaryRootsCurrent(r_))return false;
#endif
            ++calls_;
            __try { function=reinterpret_cast<std::uintptr_t(*)()>(plan.module+cfgprimary::disk::Conversion.factory)(); }
            __except(EXCEPTION_EXECUTE_HANDLER) { exception_=true; return false; }
            if (!Refresh()) return false;
        }
        if (!cfgprobe::ObjectPointer(function) || !cfgobserver::Same(r_,plan.module+cfgprimary::disk::Conversion.cache,function) ||
            !cfgspawn::ObjectSlot(r_,current_.objectArray,function,spawnAssets.conversionSlot) ||
            !cfgobserver::Same(r_,function+0x20,spawnAssets.kismetClass)) return false;
        spawnAssets.conversionFunction=function;
#ifdef CFG_RETAINED_PRIMARY_JASON
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{function}))return false;
#endif
        return true;
    }
