#pragma once
#include "EntrySourceDiagnostic.hpp"
#include "SettingsStartObservation.hpp"
namespace cfgmenu::startdiagnostic {
// Read-only source evidence, NEVER a Start binding/callback capability. The
// caller obtains the class from the validated current authored entry chain.
// No guessed settings instance, engine allocation/call, cache write or scan.
struct Snapshot {
    // Evidence-only ceiling accommodates changed cooked graphs; never consumed
    // by the strict runtime Start validator or dispatcher.
    std::array<capture::FunctionEvidenceStorage<256,65536>,3> functions{};
    std::array<startbinding::FastCall,3> fast{};
    std::array<unsigned,3> roleStage{};
    std::array<capture::FunctionDiagnostic,3> failure{};
    unsigned completed=0,observed=0,fastObserved=0,stage=0;
};
inline constexpr std::array<std::u16string_view,3> Names{
    u"OnClicked_Start",u"Construct",u"ExecuteUbergraph_OfflineBotsSettingsMenuWidget"};
template<class Reader>bool ObserveClass(Reader& r,const active::Context& actual,std::uintptr_t settings,Snapshot& out){
    // The evidence arrays live on the heap. Do not aggregate-assign Snapshot{}
    // here: MSVC materializes a ~650KiB stack temporary before copying it.
    // Clear validity first; KnownFunction resets each destination it observes.
    out.completed=out.observed=out.fastObserved=0;out.roleStage={};out.fast={};out.failure={};out.stage=1;
    cfgnames::Observation<Reader> names(r,actual.module);cfgnames::Text label;
    cfgreflection::Observation<Reader> reflection(r,actual.module);
    if(!active::Current(r,actual,actual.thread)||!names.Object(settings,label)||label.View()!=u"OfflineBotsSettingsMenuWidget_C")return false;
    out.stage=0;
    for(unsigned i=0;i<Names.size();++i){cfgreflection::Field field;auto& stage=out.roleStage[i];stage=2+3*i;
        if(!reflection.Find(settings,Names[i],field,false)||field.owner!=settings)continue;
        stage=3+3*i;
        if(!capture::KnownFunction(r,actual.module,field,Names[i],out.functions[i],&out.failure[i]))continue;
        out.observed|=1u<<i;stage=4+3*i;
        if(!startbinding::ReadFastCall(r,field.object,out.fast[i]))continue;
        out.fastObserved|=1u<<i;++out.completed;stage=0;
    }
    for(const auto stage:out.roleStage)if(stage&&!out.stage)out.stage=stage;
    // A failed role must not prevent recording the other two in this capture.
    if(out.completed!=Names.size())return false;
    out.stage=11;if(!active::Current(r,actual,actual.thread))return false;
    out.stage=0;return true;
}
inline void Print(const Snapshot& value,bool captured){
    std::printf("SettingsSourceSnapshot=%s Completed=%u/3 Stage=%u MetadataMask=%u FastMask=%u\n",
        captured?"CAPTURED":"REFUSED",value.completed,value.stage,value.observed,value.fastObserved);
    for(unsigned i=0;i<Names.size();++i){
        std::printf("SettingsSourceRole=%u Name=",i);entrydiagnostic::Label(Names[i]);
        std::printf(" Stage=%u FunctionStage=%s Fields=%u ScriptBytes=%d FastRead=%s FastGraph=0x%llX FastEntry=%d\n",
            value.roleStage[i],value.failure[i].stage,value.failure[i].fields,value.failure[i].scriptBytes,
            value.fastObserved&(1u<<i)?"yes":"no",static_cast<unsigned long long>(value.fast[i].graph),value.fast[i].entry);
        if(value.observed&(1u<<i))entrydiagnostic::Function(value.functions[i]);
    }
    std::puts("SettingsSourceEvidenceOnly=yes\nNoSettingsInstanceFabricated=yes\nStartBindingApproved=no\nSnapshotIsNotAtomic=yes");
}
}
