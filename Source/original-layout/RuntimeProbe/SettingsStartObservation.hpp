#pragma once
#include "ActiveMenuObservation.hpp"
#include "MenuScriptObservation.hpp"
#include "SettingsStart.generated.hpp"
namespace cfgmenu::startbinding {
namespace shape=cfgcompat::cfg35::settingsStart;
constexpr unsigned ReadLimit=12000;
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
struct FastCall {
    std::uintptr_t graph=0;std::int32_t entry=0;
    bool operator==(const FastCall& b)const noexcept{return graph==b.graph&&entry==b.entry;}
};
template<class Reader> bool ReadFastCall(Reader& r,std::uintptr_t function,FastCall& out){
    out={};FastCall a,b;
    if(!cfgprobe::ObjectPointer(function)||!r.Read(function+startdisk::FastGraph,a.graph)||
        !r.Read(function+startdisk::FastEntry,a.entry)||!r.Read(function+startdisk::FastGraph,b.graph)||
        !r.Read(function+startdisk::FastEntry,b.entry)||!(a==b)||
        (a.graph&&!cfgprobe::ObjectPointer(a.graph)))return false;
    out=a;return true;
}
struct Slot {
    cfgreflection::Field field{};cfgprobe::lifetime::SlotObservation identity{};
    capture::ScriptHeader script{};std::uintptr_t cleanup=0;FastCall fast{};
    std::uint32_t flags=0;std::uint16_t parameters=0;std::int32_t storage=0;
};
struct Binding {
    active::Context context{};Slot start{},construct{},graph{};
    cfgreflection::Property entry{};
    // Cold capture only. No graph copy/name search on each HUD draw.
    std::array<unsigned char,22> startBytes{},constructBytes{};
    std::array<unsigned char,shape::GraphBytes> graphBytes{};
};
inline bool Wrapper(const capture::DiagnosticFunctionEvidence& f,std::uintptr_t module,
    std::uintptr_t owner,std::u16string_view name,std::uint32_t flags,std::int32_t entry){
    std::int32_t encoded=0;std::memcpy(&encoded,f.bytes.data()+14,4);
    return f.function.owner==owner&&f.function.kind.View()==u"Function"&&f.function.label.View()==name&&
        cfgprobe::ObjectPointer(f.function.object)&&f.flags==flags&&!f.parameterBytes&&!f.storageBytes&&!f.fieldsCount&&
        !f.destructorLink&&f.nativeExecutor==module+startdisk::ScriptExecutor&&f.script.count==22&&
        f.script.capacity>=22&&cfgprobe::lifetime::Extent(f.script.data,22)&&
        f.bytes[0]==0x90&&f.bytes[13]==0x5A&&encoded==entry&&f.bytes[18]==0x97&&
        f.bytes[19]==0xA5&&f.bytes[20]==0x39&&f.bytes[21]==0xE5;
}
inline Slot Record(const capture::DiagnosticFunctionEvidence& f,const FastCall& fast){
    return {f.function,f.slot,f.script,f.destructorLink,fast,f.flags,f.parameterBytes,f.storageBytes};
}
// Actual in-process metadata is still required: class-owned function names,
// bounded scripts, unique EntryPoint parameter and corroborating fast aliases.
// The wrapper's opaque serialized FName payload is compared, NOT decoded using
// the unrelated eight-byte in-memory FName layout. This is not hook permission.
inline bool Validate(const active::Context& context,const capture::DiagnosticFunctionEvidence& start,
    const capture::DiagnosticFunctionEvidence& construct,const capture::DiagnosticFunctionEvidence& graph,
    const FastCall& startFast,const FastCall& constructFast,Binding& out,const FastCall& graphFast={}){
    out={};
    if(!context.thread||!cfgprobe::ObjectPointer(context.module)||!cfgprobe::ObjectPointer(context.widget)||
        !cfgprobe::ObjectPointer(context.type)||!context.identities[2].HasAssignedSerial()||
        !Wrapper(start,context.module,context.type,u"OnClicked_Start",0x0C000000,startdisk::StartEntry)||
        !Wrapper(construct,context.module,context.type,u"Construct",0x08020808,startdisk::ConstructEntry)||
        start.function.object==construct.function.object||graph.function.object==start.function.object||
        graph.function.object==construct.function.object||!cfgprobe::ObjectPointer(graph.function.object)||
        graph.function.owner!=context.type||graph.function.kind.View()!=u"Function"||
        graph.function.label.View()!=u"ExecuteUbergraph_OfflineBotsSettingsMenuWidget"||graph.flags||
        graph.parameterBytes!=4||graph.storageBytes!=shape::StorageBytes||graph.script.count!=shape::GraphBytes||graph.script.capacity<shape::GraphBytes||
        !cfgprobe::lifetime::Extent(graph.script.data,shape::GraphBytes)||graph.nativeExecutor!=context.module+startdisk::ScriptExecutor||
        !cfgprobe::ObjectPointer(graph.destructorLink)||graph.fieldsCount>graph.fields.size())return false;
    for(unsigned i=1;i<=12;++i)if(start.bytes[i]!=construct.bytes[i])return false;
    for(const auto entry:{startdisk::StartEntry,startdisk::ConstructEntry}){
        const std::uint32_t destination=entry==startdisk::StartEntry?shape::StartDestination:shape::ConstructDestination;
        std::uint32_t actual=0;std::memcpy(&actual,graph.bytes.data()+entry+1,4);
        if(graph.bytes[entry]!=0x0A||actual!=destination)return false;
    }
    const auto alias=[&](const FastCall& f,std::int32_t entry){
        // When A0 is zero ProcessEvent ignores A8; do not manufacture an alias.
        return !f.graph||(f.graph==graph.function.object&&f.entry==entry);
    };
    if(!alias(startFast,startdisk::StartEntry)||!alias(constructFast,startdisk::ConstructEntry)||graphFast.graph)return false;
    const cfgreflection::Property* parameter=nullptr;
    for(unsigned i=0;i<graph.fieldsCount;++i){const auto& p=graph.fields[i];
        if(p.field.owner!=graph.function.object||p.dimension<1||p.elementBytes<1||p.offset<0||p.offset>graph.storageBytes||
            p.dimension>(graph.storageBytes-p.offset)/p.elementBytes)return false;
        if(!(p.flags&0x80))continue;
        if(parameter||p.field.label.View()!=u"EntryPoint"||p.field.kind.View()!=u"IntProperty"||
            !cfgprobe::ObjectPointer(p.field.object)||p.offset||p.dimension!=1||p.elementBytes!=4||
            p.flags!=0x0/*CFG3.5 address withheld*/40000294ULL)return false;
        parameter=&p;
    }
    if(!parameter)return false;
    Binding candidate;candidate.context=context;candidate.start=Record(start,startFast);
    candidate.construct=Record(construct,constructFast);candidate.graph=Record(graph,graphFast);candidate.entry=*parameter;
    std::copy_n(start.bytes.begin(),22,candidate.startBytes.begin());
    std::copy_n(construct.bytes.begin(),22,candidate.constructBytes.begin());
    std::copy_n(graph.bytes.begin(),shape::GraphBytes,candidate.graphBytes.begin());out=candidate;return true;
}
struct Workspace {capture::DiagnosticFunctionEvidence start,construct,graph;};
// Caller owns reusable cold scratch storage; no allocation or global registry
// discovery in this reader. Resolve only three functions on the ACTIVE class.
template<class Reader> bool Observe(Reader& reader,const active::Context& context,bool exactImageVerified,
    Workspace& scratch,Binding& out,unsigned* consumed=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    FastCall startFast,constructFast,graphFast;Binding candidate;
    cfgnames::Observation<Bounded<Reader>> names(r,context.module);cfgnames::Text label;
    if(!exactImageVerified||!active::Current(r,context,context.thread)||!names.Object(context.type,label)||
        label.View()!=u"OfflineBotsSettingsMenuWidget_C"||
        !capture::Function(r,context.module,context.type,u"OnClicked_Start",scratch.start)||
        !capture::Function(r,context.module,context.type,u"Construct",scratch.construct)||
        !capture::Function(r,context.module,context.type,u"ExecuteUbergraph_OfflineBotsSettingsMenuWidget",scratch.graph)||
        !ReadFastCall(r,scratch.start.function.object,startFast)||!ReadFastCall(r,scratch.construct.function.object,constructFast)||
        !ReadFastCall(r,scratch.graph.function.object,graphFast)||
        !Validate(context,scratch.start,scratch.construct,scratch.graph,startFast,constructFast,candidate,graphFast)||
        !active::Current(r,context,context.thread))return finish(false);
    out=candidate;return finish(true);
}
template<class Reader,std::size_t N> bool CurrentSlot(Reader& r,const Binding& b,const Slot& slot,
    const std::array<unsigned char,N>& bytes,std::uintptr_t expectedExecutor){
    cfgreflection::Observation<Reader> reflection(r,b.context.module);cfgreflection::Field field;
    capture::ScriptHeader script;FastCall fast;cfgprobe::lifetime::SlotObservation identity;
    std::uintptr_t executor=0,cleanup=0;std::uint32_t flags=0;std::uint16_t parameters=0;std::int32_t storage=0;
    std::array<unsigned char,N> actual{};
    return reflection.ReadField(slot.field.object,b.context.type,field)&&field.type==slot.field.type&&
        field.name==slot.field.name&&field.next==slot.field.next&&field.kind.View()==u"Function"&&
        field.label.View()==slot.field.label.View()&&
        cfgprobe::lifetime::ReadSlot(r,b.context.registry,slot.field.object,identity)&&identity==slot.identity&&
        r.Read(slot.field.object+cfgreflection::disk::FunctionFlags,flags)&&flags==slot.flags&&
        r.Read(slot.field.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)&&parameters==slot.parameters&&
        r.Read(slot.field.object+cfgreflection::disk::StorageBytes,storage)&&storage==slot.storage&&
        r.Read(slot.field.object+startdisk::Executor,executor)&&executor==expectedExecutor&&
        r.Read(slot.field.object+0x68,cleanup)&&cleanup==slot.cleanup&&
        r.Read(slot.field.object+scriptdisk::Script,script)&&script==slot.script&&
        ReadFastCall(r,slot.field.object,fast)&&fast==slot.fast&&
        r.ReadBytes(script.data,actual.data(),actual.size())&&actual==bytes;
}
// Only at an owned Start callback, not on HUD ticks. Expected executors must be
// the adapter's exclusively owned slots (or the original during diagnostics).
// Serial0 metadata is revalidated, never treated as a lifetime pin. The caller
// supplies a bounded reader and stays in one non-yielding owner-thread scope.
template<class Reader> bool Current(Reader& reader,const Binding& b,std::uint32_t thread,
    std::uintptr_t wrapperExecutor,std::uintptr_t graphExecutor,unsigned* consumed=nullptr){
    Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    if(!active::Current(r,b.context,thread)||!cfgprobe::ObjectPointer(wrapperExecutor)||!cfgprobe::ObjectPointer(graphExecutor)||
        !CurrentSlot(r,b,b.start,b.startBytes,wrapperExecutor)||
        !CurrentSlot(r,b,b.construct,b.constructBytes,b.context.module+startdisk::ScriptExecutor)||
        !CurrentSlot(r,b,b.graph,b.graphBytes,graphExecutor))return finish(false);
    cfgreflection::Observation<Bounded<Reader>> reflection(r,b.context.module);cfgreflection::Field field;cfgreflection::Property property;
    const auto& p=b.entry;
    return finish(reflection.ReadField(p.field.object,b.graph.field.object,field)&&field.type==p.field.type&&
        field.name==p.field.name&&field.next==p.field.next&&reflection.ReadProperty(field,property)&&
        property.dimension==p.dimension&&property.elementBytes==p.elementBytes&&property.offset==p.offset&&
        property.flags==p.flags&&active::Current(r,b.context,thread));
}
}
