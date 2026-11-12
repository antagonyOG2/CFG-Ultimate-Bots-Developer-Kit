#pragma once
#include "PickerPrecreationObservation.hpp"
#include "NativeCounselorTransition.generated.hpp"
#include <type_traits>
namespace cfgmenu::counselortransition {
constexpr unsigned ReadLimit=16384,OriginalCallLimit=1,LeaseCallLimit=1,ParameterExchangeLimit=2;
using Original=void(*)(void*,void*,void*) noexcept;
struct Consumer {cfgreflection::Property persistent{};std::uintptr_t push=0,valid=0;};
struct Packet {std::uintptr_t address=0,value=0;};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads>=ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads>=ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
inline bool Property(const cfgreflection::Property& p,std::u16string_view name,std::u16string_view kind,int bytes,int offset,std::uint64_t flags){
    return p.field.label.View()==name&&p.field.kind.View()==kind&&p.elementBytes==bytes&&p.offset==offset&&p.dimension==1&&p.flags==flags;
}
inline bool Shape(const pickerprepare::Prepared& p,const Consumer& c){
    const auto& t=pickerprepare::Get(p,pickerprepare::Role::Transition);const auto& g=p.graph;
    if(p.kind!=pickerprepare::Kind::Counselor||!t.present||t.entry!=0x270F||t.slot.parameters!=8||t.slot.storage!=8||
       t.slot.flags!=0x08080800||t.slot.script.count!=40||t.slot.cleanup||t.fieldCount!=1||t.slot.field.owner!=p.type||
       t.slot.field.label.View()!=u"OnPlayTransitionOut"||!counselorpicker::GraphShape(g,p.type,p.source.source.module)||
       !Property(t.fields[0],u"TransitioningTo",u"ClassProperty",8,0,0x000C001040000294ULL)||
       t.fields[0].field.owner!=t.slot.field.object||c.persistent.field.owner!=g.function.object||
       !Property(c.persistent,u"K2Node_Event_TransitioningTo",u"ClassProperty",8,1248,0x0008001040000200ULL)||
       !cfgprobe::ObjectPointer(c.push)||!cfgprobe::ObjectPointer(c.valid))return false;
    // All forty original wrapper bytes are accounted for, including persistent
    // destination, parameter source, graph FName and entry. No guessed local write.
    std::array<unsigned char,40> wrapper{};wrapper[0]=6;wrapper[9]=0x76;wrapper[18]=0x90;wrapper[31]=0x5A;
    wrapper[36]=0x97;wrapper[37]=0xA5;wrapper[38]=0x39;wrapper[39]=0xE5;
    std::memcpy(wrapper.data()+1,&c.persistent.field.object,8);std::memcpy(wrapper.data()+10,&t.fields[0].field.object,8);
    // CFG script FName serializes comparison/display indices plus instance number.
    const std::array<std::int32_t,3> name={g.function.name.index,g.function.name.index,g.function.name.number};
    std::memcpy(wrapper.data()+19,name.data(),12);const int entry=0x270F;std::memcpy(wrapper.data()+32,&entry,4);
    if(std::memcmp(wrapper.data(),t.bytes.data(),wrapper.size()))return false;
    const auto ptr=[&](unsigned at){return counselorpicker::Pointer(g,at);};
    const auto u32=[&](unsigned at){std::uint32_t value=0;std::memcpy(&value,g.bytes.data()+at,4);return value;};
    // Exact source-to-consumer expressions and branch target. Push's second
    // argument is the independently pinned execTrue (skip a second transition).
    if(g.bytes[0xF]!=0xCD||g.bytes[0x10]!=0x76||g.bytes[0x2F]!=0xF5||ptr(0x30)!=c.valid||
       g.bytes[0x38]!=0x76||ptr(0x39)!=c.persistent.field.object||g.bytes[0x41]!=0x97||
       g.bytes[0x42]!=0x6F||u32(0x43)!=0x19B||g.bytes[0x47]!=0x76||ptr(0x48)!=ptr(0x11)||
       g.bytes[0x50]!=0x0D||g.bytes[0x51]!=0x76||g.bytes[0x5A]!=0xF5||ptr(0x5B)!=c.push||
       g.bytes[0x63]!=0x76||ptr(0x64)!=c.persistent.field.object||g.bytes[0x6C]!=0x4C||g.bytes[0x6D]!=0x97||
       g.bytes[0x19B]!=0xCD||g.bytes[0x1A5]!=0xF5)return false;
    // Latent continuation retains this graph's own FName and returns to0F.
    // The late callee name was not captured: do not invent a named Delay proof.
    if(g.bytes[0x279F]!=0xF5||g.bytes[0x27B2]!=0x74||u32(0x27BB)!=32||
       g.bytes[0x27BF]!=0x79||u32(0x27C0)!=0xF||g.bytes[0x27C9]!=0xEE||
       std::memcmp(g.bytes.data()+0x27CA,name.data(),12)||g.bytes[0x27D6]!=0xFC||
       g.bytes[0x27D7]!=0xD9||g.bytes[0x27D8]!=0x97||g.bytes[0x27D9]!=0x87)return false;
    // The real Accept exits pass null. Non-null cosmetic arguments are excluded.
    for(const auto at:{0x234EU,0x2A31U})if(g.bytes[at]!=0xF5||ptr(at+1)!=p.source.transition||g.bytes[at+9]!=0x7B||g.bytes[at+10]!=0x97)return false;
    return true;
}
template<class Reader> bool Code(Reader& r,std::uintptr_t module){
    using namespace counselortransitiondisk;std::array<unsigned char,2048> bytes{};
    for(const auto& op:Opcodes){std::uintptr_t value=0;if(!r.Read(module+op.slot,value)||value!=module+op.executor)return false;}
    for(const auto& code:Codes)if(code.size>bytes.size()||!r.ReadBytes(module+code.rva,bytes.data(),code.size)||std::memcmp(bytes.data(),code.bytes,code.size))return false;
    std::array<unsigned char,startdisk::ScriptExecutorCode.size()> original{};
    return r.ReadBytes(module+startdisk::ScriptExecutor,original.data(),original.size())&&original==startdisk::ScriptExecutorCode&&pickersource::Opcodes(r,module);
}
using NativeFunction=capture::FunctionEvidenceStorage<3,1>;
template<std::size_t Fields,std::size_t Bytes>inline bool NativeSchema(const capture::FunctionEvidenceStorage<Fields,Bytes>& f,bool push){
    if(f.script.count||f.destructorLink||f.parameterBytes!=(push?24:9)||f.storageBytes!=(push?24:16)||f.fieldsCount!=(push?3U:2U))return false;
    unsigned mask=0;
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];if(p.field.owner!=f.function.object)return false;unsigned bit=0;
        if(Property(p,push?u"MenuClass":u"Class",u"ClassProperty",8,0,push?0x001C001040000280ULL:0x0018001040000280ULL))bit=1;
        if(push&&Property(p,u"bSkipTransitionOut",u"BoolProperty",1,8,0x0018041040000282ULL))bit=2;
        if(Property(p,u"ReturnValue",push?u"ObjectProperty":u"BoolProperty",push?8:1,push?16:8,push?0x0018001040080788ULL:0x0018001040000780ULL))bit=4;
        if(!bit||(mask&bit))return false;mask|=bit;
    }return mask==(push?7U:5U);
}
template<class Reader> bool Native(Reader& r,std::uintptr_t module,bool push,std::uintptr_t expected){
    using namespace counselortransitiondisk;std::uintptr_t owner=0,ctor=0,function=0;
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field field;NativeFunction snapshot;
    return r.Read(module+(push?PushOwner:ValidOwner),owner)&&r.Read(module+(push?PushConstructor:ValidConstructor),ctor)&&
        cfgprobe::MatchingClassCache(owner,ctor)&&r.Read(module+(push?PushFunction:ValidFunction),function)&&function==expected&&
        reflection.ReadField(function,owner,field)&&field.kind.View()==u"Function"&&
        capture::KnownFunction(r,module,field,push?u"PushMenu":u"IsValidClass",snapshot)&&
        snapshot.nativeExecutor==module+(push?PushExecutor:ValidExecutor)&&snapshot.flags==(push?PushFlags:ValidFlags)&&NativeSchema(snapshot,push);
}
template<class Reader> bool ConsumerCurrent(Reader& r,const pickerprepare::Prepared& p,const Consumer& c){
    cfgreflection::Property fresh;
    return Shape(p,c)&&Code(r,p.source.source.module)&&Native(r,p.source.source.module,true,c.push)&&Native(r,p.source.source.module,false,c.valid)&&
        counselorpicker::Property(r,p.source.source.module,c.persistent.field.object,p.graph.function.object,u"K2Node_Event_TransitioningTo",u"ClassProperty",8,1248,0x0008001040000200ULL,fresh)&&
        fresh.field.type==c.persistent.field.type&&fresh.field.name==c.persistent.field.name;
}
// One bounded metadata preparation; no registry/name discovery, construction or
// native calls. Revalidation later compares the COMPLETE saved graph/wrapper.
template<class Reader> bool Prepare(Reader& reader,const active::Context& current,const pickerprepare::Prepared& p,std::uint32_t thread,Consumer& out,unsigned* reads=nullptr,const pickerprepare::Executors* expected=nullptr){
    out={};Bounded<Reader> r(reader);Consumer value;const auto finish=[&](bool ok){if(reads)*reads=r.reads;return ok;};
    if(p.kind!=pickerprepare::Kind::Counselor||!pickerprepare::Current(r,current,p,thread,expected?*expected:pickerprepare::Original(p)))return finish(false);
    const auto& t=pickerprepare::Get(p,pickerprepare::Role::Transition);std::uintptr_t persistent=0;std::memcpy(&persistent,t.bytes.data()+1,8);
    if(!counselorpicker::Property(r,current.module,persistent,p.graph.function.object,u"K2Node_Event_TransitioningTo",u"ClassProperty",8,1248,0x0008001040000200ULL,value.persistent)||
       !r.Read(current.module+counselortransitiondisk::PushFunction,value.push)||!r.Read(current.module+counselortransitiondisk::ValidFunction,value.valid)||
       !ConsumerCurrent(r,p,value))return finish(false);out=value;return finish(true);
}
template<class Reader> bool Borrowed(Reader& r,const active::Context& current,const pickerprepare::Prepared& p,const pickerprepare::Receiver& receiver,
    std::uintptr_t widget,std::uintptr_t frame,std::uint32_t thread,Packet& out){
    out={};const auto& t=pickerprepare::Get(p,pickerprepare::Role::Transition);
    if(!widget||widget!=receiver.widget||receiver.type!=p.type||current.widget!=widget||current.type!=p.type||thread!=current.thread||
       !receiver.identity.HasAssignedSerial()||!(current.identities[2]==receiver.identity)||!current.identities[1].HasAssignedSerial()||
       !cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    std::uintptr_t node=0,object=0,code=0,native=0,type=0;Packet value;cfgprobe::lifetime::SlotObservation identity;
    if(!r.Read(widget+cfgprobe::ObjectClass,type)||type!=p.type||!cfgprobe::lifetime::ReadSlot(r,current.registry,widget,identity)||!(identity==receiver.identity)||
       !r.Read(frame+startdisk::FrameNode,node)||node!=t.slot.field.object||!r.Read(frame+startdisk::FrameObject,object)||object!=widget||
       !r.Read(frame+startdisk::FrameCode,code)||code!=t.slot.script.data||!r.Read(frame+startdisk::FrameCurrentNative,native)||native!=node||
       !r.Read(frame+startdisk::FrameLocals,value.address)||!cfgprobe::lifetime::Extent(value.address,8)||!r.Read(value.address,value.value))return false;
    out=value;return true;
}
// Host must derive this lease from its existing real Accept callback/owned route,
// revalidate actual OwningPlayer (not infer it from the active controller), and
// validate the retained authored map-class reference and its graph/owner. A name,
// pointer, metadata serial0, or an old accepted flag alone is never authority.
struct Lease {
    std::uint64_t generation=0,acceptedGeneration=0;
    std::uintptr_t widget=0,controller=0,sourceClass=0,targetClass=0,targetReference=0;
    std::uint32_t thread=0;
    cfgprobe::lifetime::SlotObservation widgetIdentity{},controllerIdentity{};
};
struct Request {std::uintptr_t widget=0,frame=0,parameter=0;std::uint32_t thread=0;};
inline bool Matches(const Lease& lease,const active::Context& current,const pickerprepare::Prepared& p,const pickerprepare::Receiver& receiver){
    return lease.generation&&lease.generation==lease.acceptedGeneration&&lease.widget==receiver.widget&&lease.widget==current.widget&&
        lease.controller==current.controller&&lease.sourceClass==p.type&&lease.sourceClass==p.source.counselor&&
        lease.thread==current.thread&&lease.widgetIdentity.HasAssignedSerial()&&lease.controllerIdentity.HasAssignedSerial()&&
        lease.widgetIdentity==receiver.identity&&lease.controllerIdentity==current.identities[1]&&
        cfgprobe::ObjectPointer(lease.targetClass)&&lease.targetClass!=p.type&&cfgprobe::lifetime::Extent(lease.targetReference,8);
}
enum class Result {OriginalOnly,Redirected,RestoreConflict,MissingOriginal};
// Per installed counselor adapter, owner-thread only. Generations must increase
// across sessions; never reset this while that installed adapter remains callable.
struct State {bool busy=false;std::uint64_t consumedGeneration=0;};
// Auditable operation core. Exchange must be a conditional compare/exchange on
// the one borrowed input slot, not an arbitrary Write/rollback. Original's copy
// persists in the engine graph frame; only our temporary caller-input is restored.
template<class Exchange,class Forward> Result ForwardBorrowed(State& state,const Packet& packet,const Lease& lease,bool authorized,Exchange&& exchange,Forward&& original,std::uintptr_t expectedInput=0) noexcept {
    static_assert(std::is_nothrow_invocable_r_v<bool,Exchange,std::uintptr_t,std::uintptr_t,std::uintptr_t>);
    static_assert(std::is_nothrow_invocable_v<Forward>);
    if(!authorized||state.busy||packet.value!=expectedInput||!packet.address||!lease.generation||lease.generation!=lease.acceptedGeneration||
       lease.generation<=state.consumedGeneration||!lease.targetClass){original();return Result::OriginalOnly;}
    state.busy=true;
    if(!exchange(packet.address,expectedInput,lease.targetClass)){state.busy=false;original();return Result::OriginalOnly;}
    state.consumedGeneration=lease.generation;original();
    const bool restored=exchange(packet.address,lease.targetClass,expectedInput);state.busy=false;
    return restored?Result::Redirected:Result::RestoreConflict;
}
// Host: Read/ReadBytes; ValidateAcceptedRoute(Request, Lease&) noexcept (one call,
// bounded/no discovery/no native dispatch); ExchangeParameter(address,old,new)
// noexcept (exact scoped slot only). No installation, global detour or polling.
template<class Host> Result Dispatch(Host& host,State& state,const active::Context& current,const pickerprepare::Prepared& p,
    const pickerprepare::Executors& expected,const Consumer& consumer,const pickerprepare::Receiver& receiver,
    void* context,void* frame,void* result,std::uint32_t thread,Original original,unsigned* reads=nullptr) noexcept {
    if(reads)*reads=0;if(!original)return Result::MissingOriginal;
    const auto forward=[&]() noexcept {original(context,frame,result);};
    // Even a rejected guard forwards exactly once, without writing any input.
    if(reinterpret_cast<std::uintptr_t>(original)!=current.module+startdisk::ScriptExecutor||state.busy){forward();return Result::OriginalOnly;}
    Bounded<Host> r(host);Packet packet;Lease lease;bool authorized=false;
    try {
        authorized=pickerprepare::Current(r,current,p,thread,expected)&&ConsumerCurrent(r,p,consumer)&&
            Borrowed(r,current,p,receiver,reinterpret_cast<std::uintptr_t>(context),reinterpret_cast<std::uintptr_t>(frame),thread,packet)&&!packet.value;
        if(authorized){const Request request{reinterpret_cast<std::uintptr_t>(context),reinterpret_cast<std::uintptr_t>(frame),packet.address,thread};
            static_assert(noexcept(host.ValidateAcceptedRoute(request,lease)));
            authorized=host.ValidateAcceptedRoute(request,lease)&&Matches(lease,current,p,receiver);
            std::uintptr_t target=0;if(authorized)authorized=r.Read(lease.targetReference,target)&&target==lease.targetClass;
        }
    }catch(...){authorized=false;}
    if(reads)*reads=r.reads;
    return ForwardBorrowed(state,packet,lease,authorized,[&](std::uintptr_t at,std::uintptr_t oldValue,std::uintptr_t newValue) noexcept {
        static_assert(noexcept(host.ExchangeParameter(at,oldValue,newValue)));return host.ExchangeParameter(at,oldValue,newValue);
    },forward);
}
}
