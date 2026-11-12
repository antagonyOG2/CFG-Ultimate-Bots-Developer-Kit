// Inside BundleAccess. Only the two existing, named Kismet soft-class frames.
    cfgsoftframe::Kind bundleSoftFrame_=cfgsoftframe::Kind::None;
    cfgsoftframe::Diagnostic bundleSoftProof_{};
    bool RetainTypedSoftClassFrame(std::uintptr_t actor,std::uintptr_t function,void* packet,bool& matched){
        matched=false;bundleSoftFrame_=cfgsoftframe::Kind::None;
        Reader metadata;metadata.cap=4096;cfgsoftframe::Result result;
        const bool observed=cfgsoftframe::Observe(metadata,plan.module,actor,function,packet,spawnAssets.kismetDefault,
            spawnAssets.conversionFunction,plan.world,plan.controller,result,&bundleSoftProof_);
        bundleSoftProof_.reads=metadata.reads;if(!observed)return false;
        matched=result.kind!=cfgsoftframe::Kind::None;if(!matched)return true;
        bundleSoftProof_.stage=50;if(!RetainPrimaryObjects(r_,result.roots))return false;
        bundleSoftProof_.stage=0;
        bundleNextOutputMaximum_=result.outputs;bundleSoftFrame_=result.kind;return true;
    }
    bool CompleteTypedSoftClassFrame(void* packet,bool& matched){
        const auto kind=bundleSoftFrame_;bundleSoftFrame_=cfgsoftframe::Kind::None;
        matched=kind!=cfgsoftframe::Kind::None;if(!matched)return true;
        if(kind==cfgsoftframe::Kind::Preload)return FinishBundleNative();
        if(!packet)return false;
        const auto result=static_cast<const cfgprimary::ConversionParameters*>(packet)->result;
        return CompleteBundleCallbackCall(r_,std::array<std::uintptr_t,1>{result},1);
    }
