// Exact existing native FString setter, adapted to current callback retention.
    bool BotNameNative(std::uintptr_t state,const cfgbotname::String& packet,std::uintptr_t pawn,std::uintptr_t controller){
        if(!state||!pawn||!controller||!packet.data||packet.count<2||packet.count>31||packet.capacity!=packet.count||
           packet.data[static_cast<unsigned>(packet.count-1)]!=0||!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(state)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{state,pawn,controller})||
           !NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;
        __try{reinterpret_cast<void(__fastcall*)(void*,const cfgbotname::String*)>(plan.module+cfgbotname::disk::Setter)(reinterpret_cast<void*>(state),&packet);}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;
        }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
