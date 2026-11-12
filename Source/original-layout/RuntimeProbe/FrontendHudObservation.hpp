#pragma once
#include "ActiveMenuObservation.hpp"
#include "MenuScriptObservation.hpp"
#include "FrontendTransport.generated.hpp"
namespace cfgmenu::hud {
constexpr unsigned ReadLimit=12000;
struct Binding {
    active::Context context{};
    std::uintptr_t hudType=0,vtable=0,nativeClass=0,event=0;
    capture::FunctionEvidence function{};
};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& value){
        if(reads==ReadLimit)return false;++reads;return reader_.Read(at,value);
    }
    bool ReadBytes(std::uintptr_t at,void* value,std::size_t bytes){
        if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,value,bytes);
    }
};
// Exact native ReceiveDrawHUD ABI from the current physical capture. This is
// NOT a callable UI binding: the event is observed after its ordinary dispatch.
inline bool Frame(const capture::FunctionEvidence& f,std::uintptr_t module,std::uintptr_t owner){
    if(f.function.owner!=owner||f.function.kind.View()!=u"Function"||f.function.label.View()!=u"ReceiveDrawHUD"||
        f.flags!=transportdisk::EventFlags||f.parameterBytes!=8||f.storageBytes!=8||
        f.nativeExecutor!=module+transportdisk::BlueprintExecutor||f.destructorLink||f.script.data||f.script.count||f.script.capacity||f.fieldsCount!=2)return false;
    std::array<bool,2> seen{};
    for(unsigned i=0;i<2;++i){const auto& p=f.fields[i];const auto label=p.field.label.View();
        const unsigned index=label==u"SizeX"?0:label==u"SizeY"?1:2;
        if(index==2||seen[index]||p.field.owner!=f.function.object||p.field.kind.View()!=u"IntProperty"||
            !cfgprobe::ObjectPointer(p.field.object)||p.dimension!=1||p.elementBytes!=4||p.offset!=static_cast<int>(index*4)||
            p.flags!=0x0/*CFG3.5 address withheld*/40000280ULL)return false;seen[index]=true;
    }
    return seen[0]&&seen[1];
}
template<class Reader> bool Path(Reader& r,const Binding& b){
    std::uintptr_t type=0,vtable=0,dispatch=0,draw=0,nativeClass=0,cache=0;
    cfgnames::Words eventName{};
    std::array<unsigned char,transportdisk::DrawCode.size()> drawBytes{};
    std::array<unsigned char,transportdisk::WrapperCode.size()> wrapperBytes{};
    return r.Read(b.context.hud+cfgprobe::ObjectClass,type)&&type==b.hudType&&
        r.Read(b.context.hud,vtable)&&vtable==b.vtable&&
        r.Read(vtable+transportdisk::DispatcherSlot,dispatch)&&dispatch==b.context.module+transportdisk::ActorDispatcher&&
        r.Read(vtable+transportdisk::DrawSlot,draw)&&draw==b.context.module+transportdisk::Draw&&
        r.Read(b.context.module+transportdisk::HudClass,nativeClass)&&nativeClass==b.nativeClass&&
        r.Read(b.context.module+transportdisk::EventCache,cache)&&(!cache||cache==b.event)&&
        r.Read(b.context.module+transportdisk::EventName,eventName)&&eventName==b.function.function.name&&
        r.ReadBytes(b.context.module+transportdisk::Draw,drawBytes.data(),drawBytes.size())&&drawBytes==transportdisk::DrawCode&&
        r.ReadBytes(b.context.module+transportdisk::Wrapper,wrapperBytes.data(),wrapperBytes.size())&&wrapperBytes==transportdisk::WrapperCode;
}
template<class Reader> bool Observe(Reader& reader,const active::Context& context,bool exactImageVerified,
    Binding& out,unsigned* consumed=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    Binding candidate;candidate.context=context;
    if(!exactImageVerified||!active::Current(r,context,context.thread)||
        !r.Read(context.hud+cfgprobe::ObjectClass,candidate.hudType)||!cfgprobe::ObjectPointer(candidate.hudType)||
        !r.Read(context.hud,candidate.vtable)||!cfgprobe::ObjectPointer(candidate.vtable)||
        !r.Read(context.module+transportdisk::HudClass,candidate.nativeClass)||!cfgprobe::ObjectPointer(candidate.nativeClass)||
        !capture::Function(r,context.module,candidate.hudType,u"ReceiveDrawHUD",candidate.function)||
        !Frame(candidate.function,context.module,candidate.nativeClass))return finish(false);
    candidate.event=candidate.function.function.object;
    if(!Path(r,candidate)||!active::Current(r,context,context.thread))return finish(false);
    out=candidate;return finish(true);
}
// Bounded direct revalidation of already-resolved metadata, not repeated field
// hierarchy/name discovery. Call only for the exact HUD/event/thread in HudGate.
// Metadata serial zero is not a GC pin: every stored owner/type/name/schema and
// slot is rechecked; mutable HUD lifetime uses its positive current serial.
template<class Reader> bool Current(Reader& reader,const Binding& b,std::uint32_t callerThread,unsigned* consumed=nullptr){
    Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;return okay;};
    if(!active::Current(r,b.context,callerThread)||!Path(r,b))return finish(false);
    cfgreflection::Observation<Bounded<Reader>> reflection(r,b.context.module);capture::FunctionEvidence fresh;
    cfgreflection::Field field;
    if(!reflection.ReadField(b.event,b.nativeClass,field)||field.type!=b.function.function.type||
        !(field.name==b.function.function.name)||field.next!=b.function.function.next||
        !capture::KnownFunction(r,b.context.module,field,u"ReceiveDrawHUD",fresh)||
        !Frame(fresh,b.context.module,b.nativeClass)||!(fresh.slot==b.function.slot))return finish(false);
    for(unsigned i=0;i<2;++i){const auto& a=fresh.fields[i];const auto& old=b.function.fields[i];
        if(a.field.object!=old.field.object||a.field.type!=old.field.type||a.field.next!=old.field.next||
            !(a.field.name==old.field.name))return finish(false);
    }
    return finish(active::Current(r,b.context,callerThread));
}
// Callback provenance only, not GC/lifetime or native-call permission. Unlike
// auxiliary widget opportunities, this exact ordinary engine wrapper is pinned
// on disk. A matching event name or the correct thread alone is insufficient.
template<class Reader> bool CompletedNativeCallback(Reader& reader,const Binding& b,
    std::uintptr_t receiver,std::uintptr_t function,std::uintptr_t returnPc,
    std::uint32_t callerThread,bool nested,bool returned){
    if(nested||!returned||callerThread!=b.context.thread||!b.context.module||
       !b.context.hud||!b.event||receiver!=b.context.hud||function!=b.event||
       returnPc!=b.context.module+transportdisk::ReturnPc)return false;
    return Current(reader,b,callerThread);
}
}
