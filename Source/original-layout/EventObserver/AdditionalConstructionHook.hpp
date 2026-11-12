// Included after BundleAccess. Observational construction trace plus the
// exact CFG-only duplicate-primary iterator adaptation during F1 allocation.
#include <intrin.h>
bool BundleAdditionalBeginAllowed(void* actor){
    const auto& scope=bundleAdditionalConstructionScope;
    if(!scope.active||GetCurrentThreadId()!=scope.thread||scope.beginActor||
       reinterpret_cast<std::uintptr_t>(actor)!=scope.actor)return false;
    Reader r;r.cap=160;
    return cfgadditionalconstruction::BeginAllowed(r,scope,reinterpret_cast<std::uintptr_t>(actor),GetCurrentThreadId());
}
void __fastcall BundleAdditionalKillerBegin(void* actor){
    const bool allowed=BundleAdditionalBeginAllowed(actor);
    auto& scope=bundleAdditionalConstructionScope;
    const auto previous=scope.beginActor;
    // A foreign/nested BeginPlay must NOT inherit the parent's permission.
    scope.beginActor=allowed?reinterpret_cast<std::uintptr_t>(actor):0;
    if(allowed)cfgclosedtrace::Mark("f1-native-begin-enter",scope.actor,scope.primary);
    else if(scope.active&&GetCurrentThreadId()==scope.thread&&reinterpret_cast<std::uintptr_t>(actor)==scope.actor)
        cfgclosedtrace::Mark("f1-native-begin-refused",scope.actor,scope.primary);
    __try{bundleAdditionalBeginOriginal(actor);}
    __finally{scope.beginActor=previous;}
}
void __fastcall BundleAdditionalIterator(void* iterator){
    const auto caller=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
    auto& scope=bundleAdditionalConstructionScope;
    // Foreign calls remain a single native call without even reading state.
    if(!scope.active||!scope.beginActor||GetCurrentThreadId()!=scope.thread||
       (caller!=scope.module+cfgadditionalconstruction::disk::FirstIteratorReturn&&
        caller!=scope.module+cfgadditionalconstruction::disk::NextIteratorReturn)){
        bundleAdditionalIteratorOriginal(iterator);return;
    }
    cfgadditionalconstruction::Advance(bundleAdditionalIteratorOriginal,[&](){Reader r;r.cap=160;
        return cfgadditionalconstruction::SkipPrimary(r,scope,reinterpret_cast<std::uintptr_t>(iterator),caller,GetCurrentThreadId());
    },[&](){++scope.primarySkipped;cfgclosedtrace::Mark("f1-native-begin-primary-skipped",scope.actor,scope.primary);},iterator);
}
std::uintptr_t BundleAdditionalConstructionEvent(void* actor,void* function){
    auto& scope=bundleAdditionalConstructionScope;const auto value=reinterpret_cast<std::uintptr_t>(actor);
    if(!scope.active||GetCurrentThreadId()!=scope.thread||!scope.actor||value!=scope.actor||scope.events>=16)return 0;
    Reader r;r.cap=64;cfgadditionalconstruction::Sample sample{};cfgnames::Words name{};
    cfgnames::Text text;cfgnames::Observation<Reader> names(r,scope.module);
    const auto event=reinterpret_cast<std::uintptr_t>(function);
    if(!cfgadditionalconstruction::Observe(r,scope,value,scope.thread,sample)||!r.Read(event+0x18,name))return 0;
    ++scope.events;
    cfgclosedtrace::Mark("f1-construction-event-before",event,sample.flags);
    if(names.Decode(name,text)){
        char label[112]="f1-event-";unsigned at=9;
        for(unsigned i=0;i<text.count&&at<110;++i){const auto ch=text.data[i];label[at++]=ch>=32&&ch<127?static_cast<char>(ch):'?';}
        label[at]=0;cfgclosedtrace::Mark(label,value,event);
    }
    return event;
}
void BundleAdditionalConstructionEventAfter(std::uintptr_t event,bool completed){
    if(!event)return;
    Reader r;r.cap=64;cfgadditionalconstruction::Sample sample{};const auto& scope=bundleAdditionalConstructionScope;
    if(cfgadditionalconstruction::Observe(r,scope,scope.actor,GetCurrentThreadId(),sample))
        cfgclosedtrace::Mark(completed?"f1-construction-event-after":"f1-construction-event-exception",event,sample.flags);
}
void BundleAdditionalConstructionSample(void* actor,bool after){
    auto& scope=bundleAdditionalConstructionScope;const auto value=reinterpret_cast<std::uintptr_t>(actor);
    if(!scope.active||GetCurrentThreadId()!=scope.thread||scope.observations>=2||(scope.actor&&scope.actor!=value))return;
    Reader r;r.cap=64;cfgadditionalconstruction::Sample sample{};
    if(!cfgadditionalconstruction::Observe(r,scope,value,GetCurrentThreadId(),sample))return;
    scope.actor=value;++scope.observations;
    if(!after&&sample.slot.HasAssignedSerial()&&!sample.flags)scope.actorSlot=sample.slot;
    cfgclosedtrace::Mark(after?"f1-construction-after":"f1-construction-before",value,sample.flags);
    cfgclosedtrace::Mark(after?"f1-construction-pawn-after":"f1-construction-pawn-before",sample.pawn,sample.slot.serial);
    cfgclosedtrace::Mark(after?"f1-construction-state-after":"f1-construction-state-before",sample.state,sample.controller);
}
void __fastcall BundleAdditionalPostSpawn(void* actor,const void* transform,void* owner,void* instigator,
    unsigned char remote,unsigned char noFail,unsigned char defer){
    cfgadditionalconstruction::Forward(bundleAdditionalPostSpawnOriginal,[&](bool after){BundleAdditionalConstructionSample(actor,after);},
        actor,transform,owner,instigator,remote,noFail,defer);
}
bool PrepareAdditionalConstructionTrace(std::uintptr_t module){
    using namespace cfgadditionalconstruction::disk;
    if(bundleAdditionalPostSpawnTarget)return bundleAdditionalPostSpawnEnabled&&bundleAdditionalPostSpawnTarget==module+PostSpawn&&
        bundleAdditionalBeginEnabled&&bundleAdditionalIteratorEnabled&&bundleAdditionalBeginTarget==module+KillerBegin&&
        bundleAdditionalIteratorTarget==module+IteratorAdvance;
    Reader r;r.cap=12;if(!cfgbundle::Code(r,module,PostSpawn,PostSpawnCode)||
        !cfgbundle::Code(r,module,KillerBegin,KillerBeginCode)||!cfgbundle::Code(r,module,IteratorAdvance,IteratorCode)||
        !cfgbundle::Code(r,module,FirstIteratorReturn-13,FirstIteratorCode)||
        !cfgbundle::Code(r,module,NextIteratorReturn-9,NextIteratorCode))return false;
    void* begin=nullptr;void* advance=nullptr;
    if(MH_CreateHook(reinterpret_cast<void*>(module+KillerBegin),reinterpret_cast<void*>(&BundleAdditionalKillerBegin),&begin)!=MH_OK)return false;
    bundleAdditionalBeginOriginal=reinterpret_cast<AdditionalSingleOriginal>(begin);bundleAdditionalBeginTarget=module+KillerBegin;
    if(MH_CreateHook(reinterpret_cast<void*>(module+IteratorAdvance),reinterpret_cast<void*>(&BundleAdditionalIterator),&advance)!=MH_OK)return false;
    bundleAdditionalIteratorOriginal=reinterpret_cast<AdditionalSingleOriginal>(advance);bundleAdditionalIteratorTarget=module+IteratorAdvance;
    MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(bundleAdditionalIteratorTarget))!=MH_OK)return false;
    bundleAdditionalIteratorEnabled=true;
    if(MH_EnableHook(reinterpret_cast<void*>(bundleAdditionalBeginTarget))!=MH_OK)return false;
    bundleAdditionalBeginEnabled=true;
    void* original=nullptr;const auto target=module+PostSpawn;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleAdditionalPostSpawn),&original)!=MH_OK)return false;
    bundleAdditionalPostSpawnOriginal=reinterpret_cast<AdditionalPostSpawnOriginal>(original);bundleAdditionalPostSpawnTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleAdditionalPostSpawnEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
