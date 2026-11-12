// Inside existing BundleAccess. Typed compatibility gateway only; no AI logic.
    template<class Host>bool InvokeUrinationLoader(Host& host,std::uintptr_t defaults,std::uintptr_t function,
        void* packet,bool load){
        if(!NativeCallAllowed()||host.Owner().world!=plan.world||host.Owner().controller!=plan.controller||
            !host.Current(host.Owner()))return false;
        Reader metadata;metadata.cap=4096;std::array<std::uintptr_t,4> roots{};
        if(!cfgurination::LoaderSchema(metadata,plan.module,defaults,function,load,roots)||
            !cfgurination::LoaderPacket(metadata,plan.module,host.Owner(),packet,load))return false;
        Reader retention;retention.cap=512;
        if(!RetainPrimaryObjects(retention,std::array<std::uintptr_t,6>{roots[0],roots[1],roots[2],roots[3],plan.world,plan.controller})||
            !NativeCallAllowed()||!host.Current(host.Owner())||!BeginBundleNative())return false;
        // Load's constructor observer is armed only after the real retained
        // native entry is recorded and every input above has been rooted.
        if(load&&!host.BeginRequest())return FinishBundleNative()&&false;
        ++calls_;bool returned=false;
        __try{reinterpret_cast<Dispatch>(plan.module+cfgspawn::GenericDispatcherRva)(
            reinterpret_cast<void*>(defaults),reinterpret_cast<void*>(function),packet);returned=true;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;BundleRetentionFailed();}
        if(load)host.RequestReturned(returned);
        if(!returned)return false;
        return FinishBundleNative(); // no native UObject output in either frame
    }
