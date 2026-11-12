#pragma once
#include "MenuScriptObservation.hpp"
#include "MenuExecution.generated.hpp"
#include "../Core/MenuWarmDiagnostic.hpp"
namespace cfgmenu::native {
inline std::u16string_view Kind(unsigned id)noexcept{
    switch(id){case 12:return u"BoolProperty";case 17:return u"ClassProperty";case 18:return u"ObjectProperty";
    case 21:return u"StrProperty";case 28:return u"TextProperty";default:return {};}
}
// Native construction adds property-kind flags to generated descriptors. Use
// the actual capture's complete flags, not the superficially similar UHT words.
inline std::uint64_t RuntimeFlags(const execdisk::Binding& b,const execdisk::Parameter& p)noexcept{
    if(p.kind==18){
        if(p.name==u"ReturnValue")return 0x0018001040080788ULL;
        if(b.name==u"AddChildToVerticalBox"||b.name==u"RemoveChild")return 0x0018001040080280ULL;
        return 0x0018001040000280ULL;
    }
    if(p.kind==17)return 0x001C001040000280ULL;
    if(p.kind==21)return 0x0018000000000280ULL;
    if(p.kind==28)return p.name==u"ReturnValue"?0x0010000000000580ULL:0x0010000000000080ULL;
    if(p.kind==12){
        // Native BoolProperty adds its own property-kind/output flags. The
        // live RemoveChild capture30843 records0780, not UHT descriptor0580.
        if(b.name==u"RemoveChild"&&p.name==u"ReturnValue")return 0x0018001040000780ULL;
        return p.name==u"ReturnValue"?0x0010000000000580ULL:0x0018001040000280ULL;
    }
    return 0;
}
inline constexpr unsigned ElementBytes(unsigned kind)noexcept{
    return kind==12?1:kind==21?16:kind==28?24:(kind==17||kind==18)?8:0;
}
// UFunction::ParmsSize ends at the last parameter, whereas PropertiesSize
// includes the native struct's tail alignment. RemoveChild is9/16 and
// ShowLoadingAndTravel49/56 in the actual CFG capture. Do not treat the
// padded UHT descriptor size as the live ParmsSize or relax storage checks.
template<std::size_t N> constexpr unsigned ParameterExtent(const execdisk::Binding& b,
    const std::array<execdisk::Parameter,N>& parameters)noexcept{
    unsigned extent=0;
    for(std::size_t i=0;i<N;++i){const auto& p=parameters[i];const auto size=ElementBytes(p.kind);
        if(!size||!p.dimension||p.offset>b.frame||p.dimension>(b.frame-p.offset)/size)return 0;
        const auto end=p.offset+size*p.dimension;
        for(std::size_t j=0;j<i;++j){const auto& prior=parameters[j];
            const auto priorEnd=prior.offset+ElementBytes(prior.kind)*prior.dimension;
            if(p.offset<priorEnd&&prior.offset<end)return 0;
        }
        if(end>extent)extent=end;
    }
    return extent;
}
template<std::size_t N> bool FrameMatches(const capture::FunctionEvidence& f,std::uintptr_t module,std::uintptr_t owner,
    const execdisk::Binding& b,const std::array<execdisk::Parameter,N>& parameters,MenuWarmDiagnostic* diagnostic=nullptr)noexcept{
    if(diagnostic){diagnostic->frame=1;diagnostic->flags=f.flags;diagnostic->parameterBytes=f.parameterBytes;
        diagnostic->storageBytes=f.storageBytes;diagnostic->fields=f.fieldsCount;}
    const auto extent=ParameterExtent(b,parameters);
    if(!extent||!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(owner)||f.function.owner!=owner||
        f.function.label.View()!=b.name||f.function.kind.View()!=u"Function"||f.flags!=b.flags||
        f.parameterBytes!=extent||f.storageBytes!=static_cast<int>(b.frame)||f.nativeExecutor!=module+b.executor||
        f.destructorLink||f.script.count||f.script.data||f.script.capacity||f.fieldsCount!=N)return false;
    std::array<bool,N> seen{};
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];bool matched=false;
        if(diagnostic){diagnostic->frame=2;diagnostic->parameter=i;diagnostic->offset=p.offset;diagnostic->dimension=p.dimension;
            diagnostic->elementBytes=p.elementBytes;diagnostic->propertyFlags=p.flags;}
        for(unsigned j=0;j<N;++j){const auto& expected=parameters[j];
            // FName is case-insensitive; CFG's first registered spelling of
            // this one parameter is inString, while UHT emits InString.
            const bool spelling=p.field.label.View()==expected.name||
                (b.name==u"Conv_StringToText"&&expected.name==u"InString"&&p.field.label.View()==u"inString");
            if(!spelling)continue;
            if(diagnostic)diagnostic->frame=3;
            const auto size=static_cast<int>(ElementBytes(expected.kind));
            if(seen[j]||p.field.owner!=f.function.object||!cfgprobe::ObjectPointer(p.field.object)||
                p.field.kind.View()!=Kind(expected.kind)||p.offset!=static_cast<int>(expected.offset)||
                p.dimension!=static_cast<int>(expected.dimension)||p.elementBytes!=size||p.flags!=RuntimeFlags(b,expected))return false;
            seen[j]=matched=true;break;
        }
        if(!matched)return false;
    }
    if(diagnostic)diagnostic->frame=4;
    for(bool value:seen)if(!value)return false;if(diagnostic)diagnostic->frame=0;return true;
}
template<class Reader,std::size_t N,std::size_t C> bool CachedFunction(Reader& r,std::uintptr_t module,
    const execdisk::Binding& binding,const std::array<execdisk::Parameter,N>& parameters,const std::array<unsigned char,C>& code,
    capture::FunctionEvidence& out,MenuWarmDiagnostic* diagnostic=nullptr){
    if(diagnostic)diagnostic->evidence=1;
    out={};std::uintptr_t owner=0,constructor=0,function=0;
    std::array<unsigned char,C> actual{};
    if(!r.Read(module+binding.reflection,owner)||!r.Read(module+binding.constructor,constructor)||
        !cfgprobe::MatchingClassCache(owner,constructor)||!r.Read(module+binding.cache,function)||!cfgprobe::ObjectPointer(function)||
        !r.ReadBytes(module+binding.executor,actual.data(),actual.size())||actual!=code)return false;
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field field;capture::FunctionEvidence candidate;
    if(diagnostic)diagnostic->evidence=2;
    if(!reflection.ReadField(function,owner,field))return false;
    if(diagnostic)diagnostic->evidence=3;
    if(!capture::KnownFunction(r,module,field,binding.name,candidate))return false;
    if(diagnostic)diagnostic->evidence=4;
    if(!FrameMatches(candidate,module,owner,binding,parameters,diagnostic))return false;
    if(diagnostic)diagnostic->evidence=5;
    std::uintptr_t fresh=0;
    if(!r.Read(module+binding.reflection,fresh)||fresh!=owner||!r.Read(module+binding.constructor,fresh)||fresh!=owner||
        !r.Read(module+binding.cache,fresh)||fresh!=function)return false;
    out=candidate;if(diagnostic)diagnostic->evidence=0;return true;
}
}
