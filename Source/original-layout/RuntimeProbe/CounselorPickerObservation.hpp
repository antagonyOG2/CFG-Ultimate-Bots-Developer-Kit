#pragma once
#include "MenuGraphSnapshot.hpp"
#include "SettingsStartObservation.hpp"
#include "OfflineEntrySettingsObservation.hpp"
#include "CounselorPickerCast.generated.hpp"
#include "FrontendCatalogues.generated.hpp"
#include <algorithm>
namespace cfgmenu::counselorpicker {
constexpr unsigned GraphBytes=11155,ReadLimit=16384,CastOpcodeOffset=0x1045,CastOperandOffset=0x1046;
constexpr int ConstructEntry=0x15D6,AcceptEntry=0x1CB1,TransitionEntry=0x270F;
struct Binding {
    active::Context context{};startbinding::Slot construct{},accept{},transition{},graph{};
    std::array<unsigned char,GraphBytes> bytes{};
    cfgreflection::Property lobbyLocal{},baseLocal{},lobbyFlag{},controllerState{};
    std::uintptr_t lobbyClass=0,graphBaseClass=0,nativeState=0,state=0;
    cfgprobe::lifetime::SlotObservation stateIdentity{};
};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
template<class Function> std::uintptr_t Pointer(const Function& f,unsigned at){
    std::uintptr_t value=0;if(at+8<=static_cast<unsigned>(f.script.count))std::memcpy(&value,f.bytes.data()+at,8);return value;
}
inline std::uintptr_t Pointer(const Binding& b,unsigned at){std::uintptr_t value=0;if(at+8<=b.bytes.size())std::memcpy(&value,b.bytes.data()+at,8);return value;}
template<class Function> bool WrapperName(const Function& f,const cfgnames::Words& graph,unsigned offset=1){
    std::int32_t one=0,two=0,number=-1;std::memcpy(&one,f.bytes.data()+offset,4);std::memcpy(&two,f.bytes.data()+offset+4,4);std::memcpy(&number,f.bytes.data()+offset+8,4);
    return one==graph.index&&two==graph.index&&number==0&&graph.number==0;
}
template<class Graph> bool GraphShape(const Graph& f,std::uintptr_t type,std::uintptr_t module){
    return f.function.owner==type&&f.function.label.View()==u"ExecuteUbergraph_Counselor_Menu"&&f.function.kind.View()==u"Function"&&
        f.flags==0x0/*CFG3.5 address withheld*/&&f.parameterBytes==4&&f.storageBytes==2762&&f.script.count==GraphBytes&&
        f.nativeExecutor==module+startdisk::ScriptExecutor&&cfgprobe::ObjectPointer(f.destructorLink)&&
        f.bytes[CastOpcodeOffset]==counselorpickerdisk::DynamicCast&&f.bytes[0x1E30]==counselorpickerdisk::DynamicCast&&
        f.bytes[0x229D]==counselorpickerdisk::DynamicCast&&f.bytes[0x23C3]==counselorpickerdisk::DynamicCast&&
        Pointer(f,0x1E31)==Pointer(f,0x229E)&&Pointer(f,0x1E31)==Pointer(f,0x23C4)&&
        f.bytes[0x103C]==0x76&&f.bytes[0x1E27]==0x76&&f.bytes[0x104E]==0xC6&&f.bytes[0x104F]==0x76&&
        Pointer(f,0x105C)==Pointer(f,0x1065)&&f.bytes[0x1064]==0xDB&&f.bytes[0x1095]==0xDB&&
        f.bytes[ConstructEntry]==0xC6&&f.bytes[AcceptEntry]==0xC6&&f.bytes[TransitionEntry]==0xC6;
}
template<class Reader> bool Property(Reader& r,std::uintptr_t module,std::uintptr_t pointer,std::uintptr_t owner,
    std::u16string_view name,std::u16string_view kind,int bytes,int offset,std::uint64_t flags,cfgreflection::Property& out){
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field field;
    return reflection.ReadField(pointer,owner,field)&&field.label.View()==name&&field.kind.View()==kind&&
        capture::RawProperty(r,field,out)&&out.dimension==1&&out.elementBytes==bytes&&out.offset==offset&&out.flags==flags;
}
template<class Reader> bool DeclaredClass(Reader& r,std::uintptr_t module,const cfgreflection::Property& field,std::uintptr_t expected){
    std::uintptr_t vtable=0,type=0,again=0;
    return field.field.kind.View()==u"ObjectProperty"&&r.Read(field.field.object,vtable)&&vtable==module+counselorpickerdisk::ObjectPropertyVtable&&
        r.Read(field.field.object+counselorpickerdisk::DeclaredObjectClass,type)&&type==expected&&cfgprobe::ObjectPointer(type)&&
        r.Read(field.field.object+counselorpickerdisk::DeclaredObjectClass,again)&&again==type;
}
template<class Reader> bool Current(Reader& r,const Binding& b,std::uint32_t thread,bool widened=false){
    std::uintptr_t opcode=0;
    if(!active::Current(r,b.context,thread)||!r.Read(b.context.module+counselorpickerdisk::DynamicCastSlot,opcode)||
       opcode!=b.context.module+counselorpickerdisk::DynamicCastExecutor)return false;
    capture::ScriptHeader header;auto bytes=std::make_unique<std::array<unsigned char,GraphBytes>>();
    if(!capture::Script(r,b.graph.field.object,header,*bytes)||!(header==b.graph.script))return false;
    auto expected=b.bytes;if(widened)std::memcpy(expected.data()+CastOperandOffset,&b.nativeState,8);
    if(*bytes!=expected)return false;
    std::uintptr_t state=0,type=0,ctor=0;cfgprobe::lifetime::SlotObservation identity;
    if(!r.Read(b.context.controller+0x388,state)||state!=b.state||
       !cfgprobe::lifetime::ReadSlot(r,b.context.registry,state,identity)||!(identity==b.stateIdentity)||
       !r.Read(b.context.module+cataloguedisk::SCPlayerStateClass,type)||type!=b.nativeState||
       !r.Read(b.context.module+cataloguedisk::SCPlayerStateConstructor,ctor)||!cfgprobe::MatchingClassCache(type,ctor))return false;
    cfgreflection::Property lobby,base,flag;std::uintptr_t slot=0;
    if(!Property(r,b.context.module,b.lobbyLocal.field.object,b.graph.field.object,u"K2Node_DynamicCast_AsSCPlayer_State_Lobby",u"ObjectProperty",8,1368,0x0008001040000200ULL,lobby)||
       !Property(r,b.context.module,b.baseLocal.field.object,b.graph.field.object,u"K2Node_DynamicCast_AsSCPlayer_State",u"ObjectProperty",8,1400,0x0008001040000200ULL,base)||
       !Property(r,b.context.module,b.lobbyFlag.field.object,b.context.type,u"IsFromLobby",u"BoolProperty",1,1480,0x0000001040010205ULL,flag)||
       !DeclaredClass(r,b.context.module,lobby,b.lobbyClass)||!DeclaredClass(r,b.context.module,base,b.graphBaseClass))return false;
    for(const auto* record:{&b.construct,&b.accept,&b.transition,&b.graph}){
        cfgprobe::lifetime::SlotObservation fresh;startbinding::FastCall fast;
        if(!cfgprobe::lifetime::ReadSlot(r,b.context.registry,record->field.object,fresh)||!(fresh==record->identity)||
           !r.Read(record->field.object+0xB0,slot)||slot!=b.context.module+startdisk::ScriptExecutor||
           !startbinding::ReadFastCall(r,record->field.object,fast)||!(fast==record->fast))return false;
    }
    return true;
}
// Acquires only a currently owned native Counselor_Menu instance. It does NOT
// discover/load a first-entry class, approve an arbitrary cached class pointer,
// resolve the selected96-byte row, or prove frontend selection -> fresh birth.
template<class Reader> bool Observe(Reader& reader,const active::Context& context,std::uint32_t thread,bool exactImageVerified,
    Binding& out,unsigned* reads=nullptr,unsigned* stage=nullptr){
    out={};Bounded<Reader> r(reader);auto value=std::make_unique<Binding>();auto& b=*value;b.context=context;
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};if(stage)*stage=1;
    cfgnames::Observation<Bounded<Reader>> names(r,context.module);cfgnames::Text label;std::uintptr_t menu=0,ctor=0;
    if(!exactImageVerified||!active::Current(r,context,thread)||!names.Object(context.type,label)||label.View()!=u"Counselor_Menu_C"||
       !r.Read(context.module+pushdisk::MenuClass,menu)||!r.Read(context.module+pushdisk::MenuConstructor,ctor)||
       !cfgprobe::MatchingClassCache(menu,ctor)||!maps::Subclass(r,context.type,menu))return finish(false);
    cfgreflection::Observation<Bounded<Reader>> reflection(r,context.module);cfgreflection::Field field;
    auto graph=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
    if(stage)*stage=2;
    if(!reflection.Find(context.type,u"ExecuteUbergraph_Counselor_Menu",field)||!capture::Graph(r,context.module,field,*graph,diagnostic)||
       !GraphShape(*graph,context.type,context.module))return finish(false);
    startbinding::FastCall fast;if(!startbinding::ReadFastCall(r,field.object,fast)||fast.graph||fast.entry)return finish(false);
    b.graph={graph->function,graph->slot,graph->script,graph->destructorLink,fast,graph->flags,graph->parameterBytes,graph->storageBytes};
    std::copy_n(graph->bytes.begin(),GraphBytes,b.bytes.begin());
    if(stage)*stage=3;
    const auto observeWrapper=[&](std::u16string_view name,unsigned flags,int entry,startbinding::Slot& record){
        auto f=std::make_unique<capture::DiagnosticFunctionEvidence>();startbinding::FastCall sampled;
        if(!capture::Function(r,context.module,context.type,name,*f)||!startbinding::Wrapper(*f,context.module,context.type,name,flags,entry)||
           !WrapperName(*f,graph->function.name)||!startbinding::ReadFastCall(r,f->function.object,sampled)||sampled.graph||sampled.entry)return false;
        record=startbinding::Record(*f,sampled);return true;};
    if(!observeWrapper(u"Construct",0x08020808,ConstructEntry,b.construct)||!observeWrapper(u"OnClick_Accept",0x0C000000,AcceptEntry,b.accept))return finish(false);
    auto transition=std::make_unique<capture::DiagnosticFunctionEvidence>();
    if(!capture::Function(r,context.module,context.type,u"OnPlayTransitionOut",*transition)||transition->flags!=0x08080800||
       transition->parameterBytes!=8||transition->storageBytes!=8||transition->script.count!=40||transition->fieldsCount!=1||transition->destructorLink||
       transition->nativeExecutor!=context.module+startdisk::ScriptExecutor||!WrapperName(*transition,graph->function.name,19)||
       transition->bytes[18]!=0x90||transition->bytes[31]!=0x5A||transition->bytes[36]!=0x97||transition->bytes[37]!=0xA5||transition->bytes[38]!=0x39||transition->bytes[39]!=0xE5)return finish(false);
    int transitionEntry=0;std::memcpy(&transitionEntry,transition->bytes.data()+32,4);const auto& parameter=transition->fields[0];
    if(transitionEntry!=TransitionEntry||parameter.field.label.View()!=u"TransitioningTo"||parameter.field.kind.View()!=u"ClassProperty"||
       parameter.offset||parameter.elementBytes!=8||parameter.dimension!=1||parameter.flags!=0x0/*CFG3.5 address withheld*/40000294ULL||
       !startbinding::ReadFastCall(r,transition->function.object,fast)||fast.graph||fast.entry)return finish(false);
    b.transition=startbinding::Record(*transition,fast);
    if(stage)*stage=4;
    b.lobbyClass=Pointer(*graph,CastOperandOffset);b.graphBaseClass=Pointer(*graph,0x1E31);
    if(!Property(r,context.module,Pointer(*graph,0x103D),field.object,u"K2Node_DynamicCast_AsSCPlayer_State_Lobby",u"ObjectProperty",8,1368,0x0008001040000200ULL,b.lobbyLocal)||
       !Property(r,context.module,Pointer(*graph,0x1E28),field.object,u"K2Node_DynamicCast_AsSCPlayer_State",u"ObjectProperty",8,1400,0x0008001040000200ULL,b.baseLocal)||
       !Property(r,context.module,Pointer(*graph,0x1096),context.type,u"IsFromLobby",u"BoolProperty",1,1480,0x0000001040010205ULL,b.lobbyFlag)||
       !DeclaredClass(r,context.module,b.lobbyLocal,b.lobbyClass)||!DeclaredClass(r,context.module,b.baseLocal,b.graphBaseClass)||
       !r.Read(context.module+cataloguedisk::SCPlayerStateClass,b.nativeState)||!r.Read(context.module+cataloguedisk::SCPlayerStateConstructor,ctor)||
       !cfgprobe::MatchingClassCache(b.nativeState,ctor)||!maps::Subclass(r,b.lobbyClass,b.nativeState)||!maps::Subclass(r,b.graphBaseClass,b.nativeState))return finish(false);
    // The expression obtains the controller's named native PlayerState, not an
    // arbitrary lobby-only member. No descendant-specific dereference is added.
    cfgreflection::Field stateField;cfgreflection::Property property;std::uintptr_t controllerType=0;
    if(!r.Read(context.controller+cfgprobe::ObjectClass,controllerType)||!reflection.Find(controllerType,u"PlayerState",stateField)||
       stateField.object!=Pointer(*graph,0x105C)||!capture::RawProperty(r,stateField,property)||property.field.kind.View()!=u"ObjectProperty"||
       property.dimension!=1||property.elementBytes!=8||property.offset!=0x388||!r.Read(context.controller+0x388,b.state)||
       !cfgspawn::ObjectIs(r,b.state,b.nativeState)||!cfgprobe::lifetime::ReadSlot(r,context.registry,b.state,b.stateIdentity)||!b.stateIdentity.HasAssignedSerial())return finish(false);
    b.controllerState=property;
    std::array<unsigned char,counselorpickerdisk::DynamicCastCode.size()> code{};
    if(stage)*stage=5;
    if(!r.ReadBytes(context.module+counselorpickerdisk::DynamicCastExecutor,code.data(),code.size())||code!=counselorpickerdisk::DynamicCastCode||
       !Current(r,b,thread,false))return finish(false);
    out=b;return finish(true);
}
enum class Callback {Construct,Accept,Transition,Other};
template<class Reader> bool Frame(Reader& r,const Binding& b,std::uintptr_t receiver,std::uintptr_t frame,std::uint32_t thread,Callback& kind){
    kind=Callback::Other;if(receiver!=b.context.widget||thread!=b.context.thread||!cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    std::uintptr_t node=0,object=0,code=0,current=0,locals=0;
    if(!r.Read(frame+startdisk::FrameNode,node)||!r.Read(frame+startdisk::FrameObject,object)||object!=receiver||
       !r.Read(frame+startdisk::FrameCode,code)||!r.Read(frame+startdisk::FrameCurrentNative,current)||current!=node)return false;
    if(node==b.construct.field.object){if(code!=b.construct.script.data)return false;kind=Callback::Construct;}
    else if(node==b.accept.field.object){if(code!=b.accept.script.data)return false;kind=Callback::Accept;}
    else if(node==b.transition.field.object){if(code!=b.transition.script.data)return false;kind=Callback::Transition;}
    else if(node==b.graph.field.object){int entry=-1;if(code!=b.graph.script.data||!r.Read(frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||!r.Read(locals,entry))return false;
        kind=entry==ConstructEntry?Callback::Construct:entry==AcceptEntry?Callback::Accept:entry==TransitionEntry?Callback::Transition:Callback::Other;}
    return kind!=Callback::Other;
}
}
