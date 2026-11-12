#pragma once
#include "OfflineEntryObservation.hpp"
#include "NativeCounselorTransition.hpp"
namespace cfgmenu::entryroute {
constexpr unsigned ReadLimit=16384;
enum class Role:unsigned {Click,Graph,Transition,Count};
constexpr unsigned Roles=static_cast<unsigned>(Role::Count);
struct Function {startbinding::Slot slot{};std::array<unsigned char,40> bytes{};};
struct Binding {
    entry::Binding entry{};std::array<Function,Roles> functions{};
    cfgreflection::Property parameter{},persistent{};
    std::uintptr_t push=0,valid=0;
};
struct Executors {std::array<std::uintptr_t,Roles> values{};};
inline Executors Original(const Binding& b){Executors out;out.values.fill(b.entry.context.module+startdisk::ScriptExecutor);return out;}
inline std::uintptr_t Pointer(const std::array<unsigned char,40>& bytes,unsigned at){std::uintptr_t result=0;if(at+8<=bytes.size())std::memcpy(&result,bytes.data()+at,8);return result;}
inline bool Shape(const Binding& b){
    if(!b.entry.source)return false;
    const auto& t=b.functions[2];const auto& g=b.entry.source->entry;
    if(t.slot.field.owner!=b.entry.context.type||t.slot.field.label.View()!=u"OnPlayTransitionOut"||t.slot.flags!=0x08080800||
       t.slot.parameters!=8||t.slot.storage!=8||t.slot.cleanup||t.slot.script.count!=40||t.slot.fast.graph||t.slot.fast.entry||
       !counselortransition::Property(b.parameter,u"TransitioningTo",u"ClassProperty",8,0,0x000C001040000294ULL)||
       !counselortransition::Property(b.persistent,u"K2Node_Event_TransitioningTo",u"ClassProperty",8,40,0x0008001040000200ULL)||
       b.parameter.field.owner!=t.slot.field.object||b.persistent.field.owner!=g.function.object||
       t.bytes[0]!=0x06||Pointer(t.bytes,1)!=b.persistent.field.object||t.bytes[9]!=0x76||Pointer(t.bytes,10)!=b.parameter.field.object||
       t.bytes[18]!=0x90||t.bytes[31]!=0x5A||t.bytes[36]!=0x97||t.bytes[37]!=0xA5||t.bytes[38]!=0x39||t.bytes[39]!=0xE5)return false;
    int entry=0;std::memcpy(&entry,t.bytes.data()+32,4);if(entry!=0x2E4)return false;
    const auto& click=b.entry.sandboxClick;for(unsigned i=0;i<12;++i)if(t.bytes[19+i]!=click.bytes[1+i])return false;
    if(g.script.count!=1944||g.storageBytes!=113||g.bytes[0x78]!=0xF5||entrysettings::Pointer(g,0x79)!=b.valid||
       g.bytes[0x81]!=0x76||entrysettings::Pointer(g,0x82)!=b.persistent.field.object||g.bytes[0x8B]!=0x6F||
       g.bytes[0xA3]!=0xF5||entrysettings::Pointer(g,0xA4)!=b.push||g.bytes[0xAC]!=0x76||
       entrysettings::Pointer(g,0xAD)!=b.persistent.field.object||g.bytes[0xB5]!=0x4C||g.bytes[0xB6]!=0x97||
       g.bytes[0xC2]!=0xF5||g.bytes[0x3AF]!=0xF5)return false;
    // The actual sandbox button supplies the original class as a literal to
    // named native PlayTransitionOut. No graph bytes are altered by this reader.
    return g.bytes[0xDB]==0xF5&&entrysettings::Pointer(g,0xDC)==b.entry.source->push&&g.bytes[0xE4]==0x89&&
        entrysettings::Pointer(g,0xE5)==b.entry.sandboxMap&&g.bytes[0xED]==0x97;
}
template<class Reader> bool SlotCurrent(Reader& r,const Binding& b,unsigned role,std::uintptr_t expected){
    const auto& f=b.functions[role];const auto& s=f.slot;cfgreflection::Observation<Reader> reflection(r,b.entry.context.module);
    cfgreflection::Field field;cfgprobe::lifetime::SlotObservation identity;startbinding::FastCall fast;
    std::uintptr_t executor=0,cleanup=0;std::uint32_t flags=0;std::uint16_t parameters=0;int storage=0;
    if(!expected||!reflection.ReadField(s.field.object,b.entry.context.type,field)||field.type!=s.field.type||!(field.name==s.field.name)||
       !cfgprobe::lifetime::ReadSlot(r,b.entry.context.registry,s.field.object,identity)||!(identity==s.identity)||
       !r.Read(s.field.object+0xB0,executor)||executor!=expected||!startbinding::ReadFastCall(r,s.field.object,fast)||!(fast==s.fast)||
       !r.Read(s.field.object+cfgreflection::disk::FunctionFlags,flags)||flags!=s.flags||
       !r.Read(s.field.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=s.parameters||
       !r.Read(s.field.object+cfgreflection::disk::StorageBytes,storage)||storage!=s.storage||!r.Read(s.field.object+0x68,cleanup)||cleanup!=s.cleanup)return false;
    capture::ScriptHeader header;auto bytes=std::make_unique<std::array<unsigned char,65536>>();
    if(!capture::Script(r,s.field.object,header,*bytes)||!(header==s.script))return false;
    const auto* saved=role==static_cast<unsigned>(Role::Graph)?b.entry.source->entry.bytes.data():f.bytes.data();
    return !std::memcmp(bytes->data(),saved,static_cast<unsigned>(header.count));
}
template<class Reader> bool Current(Reader& r,const Binding& b,std::uint32_t thread,const Executors& expected){
    if(!b.entry.source||!entry::Current(r,b.entry,thread)||!Shape(b)||!counselortransition::Code(r,b.entry.context.module)||
       !counselortransition::Native(r,b.entry.context.module,true,b.push)||!counselortransition::Native(r,b.entry.context.module,false,b.valid))return false;
    for(unsigned i=0;i<Roles;++i)if(!SlotCurrent(r,b,i,expected.values[i]))return false;
    cfgreflection::Property parameter,persistent;
    return counselorpicker::Property(r,b.entry.context.module,b.parameter.field.object,b.functions[2].slot.field.object,u"TransitioningTo",u"ClassProperty",8,0,b.parameter.flags,parameter)&&
        counselorpicker::Property(r,b.entry.context.module,b.persistent.field.object,b.entry.source->entry.function.object,u"K2Node_Event_TransitioningTo",u"ClassProperty",8,40,b.persistent.flags,persistent)&&
        parameter.field.type==b.parameter.field.type&&parameter.field.name==b.parameter.field.name&&persistent.field.type==b.persistent.field.type&&persistent.field.name==b.persistent.field.name;
}
template<class Reader> bool Rebind(Reader& r,const Binding& saved,const active::Context& actual,const Executors& expected,Binding& out){
    out={};Binding fresh=saved;
    if(!entry::Rebind(r,saved.entry,actual,fresh.entry)||!Current(r,fresh,actual.thread,expected))return false;
    out=fresh;return true;
}
template<class Reader> bool Prepare(Reader& reader,const entry::Binding& entryBinding,Binding& out,unsigned* reads=nullptr){
    out={};counselortransition::Bounded<Reader> r(reader);auto value=std::make_unique<Binding>();auto& b=*value;b.entry=entryBinding;
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};
    if(!entryBinding.source||!entry::Current(r,entryBinding,entryBinding.context.thread))return finish(false);
    const auto record=[&](const auto& f,unsigned i){startbinding::FastCall fast;
        if(!startbinding::ReadFastCall(r,f.function.object,fast)||fast.graph||fast.entry)return finish(false);
        auto& target=b.functions[i];target.slot={f.function,f.slot,f.script,f.destructorLink,fast,f.flags,f.parameterBytes,f.storageBytes};
        if(i==0)std::copy_n(f.bytes.begin(),22,target.bytes.begin());return true;};
    if(!record(entryBinding.sandboxClick,0)||!record(entryBinding.source->entry,1))return finish(false);
    auto transition=std::make_unique<capture::DiagnosticFunctionEvidence>();startbinding::FastCall fast;
    if(!capture::Function(r,b.entry.context.module,b.entry.context.type,u"OnPlayTransitionOut",*transition)||transition->fieldsCount!=1||
       transition->script.count!=40||!startbinding::ReadFastCall(r,transition->function.object,fast))return finish(false);
    b.functions[2].slot=startbinding::Record(*transition,fast);std::copy_n(transition->bytes.begin(),40,b.functions[2].bytes.begin());b.parameter=transition->fields[0];
    if(!counselorpicker::Property(r,b.entry.context.module,Pointer(b.functions[2].bytes,1),b.entry.source->entry.function.object,
            u"K2Node_Event_TransitioningTo",u"ClassProperty",8,40,0x0008001040000200ULL,b.persistent)||
       !r.Read(b.entry.context.module+counselortransitiondisk::PushFunction,b.push)||!r.Read(b.entry.context.module+counselortransitiondisk::ValidFunction,b.valid)||
       !Current(r,b,b.entry.context.thread,Original(b)))return finish(false);
    out=b;return finish(true);
}
struct Invocation {Role role=Role::Click;std::uintptr_t locals=0,value=0;};
template<class Reader> bool Borrowed(Reader& r,const Binding& b,const Executors& expected,std::uintptr_t receiver,std::uintptr_t frame,
    std::uint32_t thread,Invocation& out){
    out={};if(receiver!=b.entry.context.widget||thread!=b.entry.context.thread||!cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    std::uintptr_t node=0,object=0,code=0,native=0;if(!r.Read(frame+startdisk::FrameNode,node)||!r.Read(frame+startdisk::FrameObject,object)||object!=receiver||
       !r.Read(frame+startdisk::FrameCode,code)||!r.Read(frame+startdisk::FrameCurrentNative,native)||native!=node)return false;
    Invocation value;unsigned role=Roles;for(unsigned i=0;i<Roles;++i)if(node==b.functions[i].slot.field.object)role=i;
    if(role==Roles||code!=b.functions[role].slot.script.data)return false;value.role=static_cast<Role>(role);
    if(value.role!=Role::Click){if(!r.Read(frame+startdisk::FrameLocals,value.locals))return false;
        if(value.role==Role::Graph){int entry=-1;if(!cfgprobe::lifetime::Extent(value.locals,4)||!r.Read(value.locals,entry)||entry!=0xDB)return false;}
        else if(!cfgprobe::lifetime::Extent(value.locals,8)||!r.Read(value.locals,value.value))return false;}
    if(!Current(r,b,thread,expected))return false;out=value;return true;
}
}
