#pragma once
#include "NativePickerSourceObservation.hpp"
#include "CounselorPickerObservation.hpp"
#include "JasonPickerObservation.hpp"
#include "NativeWidgetOwnershipObservation.hpp"
#include "PickerCallbackDiagnostic.hpp"
namespace cfgmenu::pickerprepare {
constexpr unsigned ReadLimit=16384;
enum class Kind {Counselor,Jason};
enum class Role:unsigned {Graph,Construct,Accept,Transition,Back,OfflineAccept,Previous,Count};
constexpr unsigned Roles=static_cast<unsigned>(Role::Count);
struct Handler {
    startbinding::Slot slot{};std::array<unsigned char,512> bytes{};
    std::array<cfgreflection::Property,16> fields{};unsigned fieldCount=0;int entry=-1;
    bool present=false;
};
struct Prepared {
    Kind kind=Kind::Counselor;pickersource::Resources source{};std::uintptr_t type=0;
    capture::GraphSnapshot graph{};std::array<Handler,Roles> handlers{};
    cfgreflection::Property index{},roster{},lobbyFlag{},lobbyLocal{},baseLocal{};
    std::uintptr_t lobbyClass=0,baseClass=0,nativeState=0;
    widgetowner::Getter owningPlayer{};
};
struct Executors {std::array<std::uintptr_t,Roles> values{};};
inline Executors Original(const Prepared& prepared){Executors out;for(unsigned i=0;i<Roles;++i)if(prepared.handlers[i].present)out.values[i]=prepared.source.source.module+startdisk::ScriptExecutor;return out;}
inline const Handler& Get(const Prepared& p,Role role){return p.handlers[static_cast<unsigned>(role)];}
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
template<class Reader> bool SlotCurrent(Reader& r,const active::Context& owner,const Prepared& prepared,const Handler& handler,std::uintptr_t executor,bool ownedCounselorWidening=false){
    if(!handler.present)return !executor;
    const auto& slot=handler.slot;cfgreflection::Observation<Reader> reflection(r,owner.module);cfgreflection::Field field;
    cfgprobe::lifetime::SlotObservation identity;startbinding::FastCall fast;std::uint32_t flags=0;std::uint16_t parameters=0;
    int storage=0;std::uintptr_t actual=0,cleanup=0;
    if(!executor||!reflection.ReadField(slot.field.object,prepared.type,field)||field.type!=slot.field.type||!(field.name==slot.field.name)||
       !cfgprobe::lifetime::ReadSlot(r,owner.registry,slot.field.object,identity)||!(identity==slot.identity)||
       !r.Read(slot.field.object+0xB0,actual)||actual!=executor||!startbinding::ReadFastCall(r,slot.field.object,fast)||!(fast==slot.fast)||
       !r.Read(slot.field.object+cfgreflection::disk::FunctionFlags,flags)||flags!=slot.flags||
       !r.Read(slot.field.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=slot.parameters||
       !r.Read(slot.field.object+cfgreflection::disk::StorageBytes,storage)||storage!=slot.storage||!r.Read(slot.field.object+0x68,cleanup)||cleanup!=slot.cleanup)return false;
    auto bytes=std::make_unique<std::array<unsigned char,65536>>();capture::ScriptHeader header;
    if(!capture::Script(r,slot.field.object,header,*bytes)||!(header==slot.script))return false;
    const bool graph=slot.field.object==prepared.graph.function.object;
    const auto* saved=graph?prepared.graph.bytes.data():handler.bytes.data();
    if(graph&&ownedCounselorWidening){if(prepared.kind!=Kind::Counselor)return false;
        constexpr unsigned at=counselorpicker::CastOperandOffset;std::uintptr_t operand=0;std::memcpy(&operand,bytes->data()+at,8);
        if(operand!=prepared.nativeState||std::memcmp(bytes->data(),saved,at)||std::memcmp(bytes->data()+at+8,saved+at+8,static_cast<unsigned>(header.count)-at-8))return false;
    }else if(std::memcmp(bytes->data(),saved,static_cast<unsigned>(header.count)))return false;
    for(unsigned i=0;i<handler.fieldCount;++i){const auto& savedField=handler.fields[i];cfgreflection::Property property;
        if(!reflection.ReadField(savedField.field.object,slot.field.object,field)||!(field.name==savedField.field.name)||field.type!=savedField.field.type||
           !capture::RawProperty(r,field,property)||property.offset!=savedField.offset||property.dimension!=savedField.dimension||
           property.elementBytes!=savedField.elementBytes||property.flags!=savedField.flags)return false;}
    return true;
}
// Exact published executor expectations are an explicit coordinator seam. This
// reader never publishes a hook or silently tolerates arbitrary foreign slots.
template<class Reader> bool Current(Reader& r,const active::Context& owner,const Prepared& prepared,std::uint32_t thread,const Executors& expected,bool ownedCounselorWidening=false,unsigned* stage=nullptr,unsigned* sourceStage=nullptr){
    if(stage)*stage=1;
    if(!prepared.type||!pickersource::Retained(r,owner,prepared.source,thread,sourceStage)||prepared.type!=(prepared.kind==Kind::Counselor?prepared.source.counselor:prepared.source.jason))return false;
    if(stage)*stage=2;
    if(!widgetowner::Current(r,owner,prepared.type,prepared.owningPlayer))return false;
    if(stage)*stage=3;
    if(ownedCounselorWidening&&prepared.kind!=Kind::Counselor)return false;
    for(unsigned i=0;i<Roles;++i){if(stage)*stage=10+i;if(!SlotCurrent(r,owner,prepared,prepared.handlers[i],expected.values[i],ownedCounselorWidening))return false;}
    if(stage)*stage=20;
    cfgreflection::Observation<Reader> reflection(r,owner.module);
    for(const auto* saved:{&prepared.index,&prepared.roster}){cfgreflection::Field field;cfgreflection::Property property;
        if(!reflection.ReadField(saved->field.object,prepared.type,field)||!(field.name==saved->field.name)||field.type!=saved->field.type||
           !capture::RawProperty(r,field,property)||property.offset!=saved->offset||property.dimension!=saved->dimension||property.elementBytes!=saved->elementBytes||property.flags!=saved->flags)return false;}
    if(prepared.kind==Kind::Counselor){cfgreflection::Property lobby,base,flag;std::uintptr_t native=0,ctor=0;
        if(stage)*stage=21;
        std::uintptr_t dynamic=0;std::array<unsigned char,counselorpickerdisk::DynamicCastCode.size()> code{};
        if(!r.Read(owner.module+counselorpickerdisk::DynamicCastSlot,dynamic)||dynamic!=owner.module+counselorpickerdisk::DynamicCastExecutor||
           !r.ReadBytes(owner.module+counselorpickerdisk::DynamicCastExecutor,code.data(),code.size())||code!=counselorpickerdisk::DynamicCastCode)return false;
        if(stage)*stage=22;
        if(!counselorpicker::Property(r,owner.module,prepared.lobbyLocal.field.object,prepared.graph.function.object,u"K2Node_DynamicCast_AsSCPlayer_State_Lobby",u"ObjectProperty",8,1368,0x0008001040000200ULL,lobby)||
           !counselorpicker::Property(r,owner.module,prepared.baseLocal.field.object,prepared.graph.function.object,u"K2Node_DynamicCast_AsSCPlayer_State",u"ObjectProperty",8,1400,0x0008001040000200ULL,base)||
           !counselorpicker::Property(r,owner.module,prepared.lobbyFlag.field.object,prepared.type,u"IsFromLobby",u"BoolProperty",1,1480,0x0000001040010205ULL,flag)||
           !counselorpicker::DeclaredClass(r,owner.module,lobby,prepared.lobbyClass)||!counselorpicker::DeclaredClass(r,owner.module,base,prepared.baseClass)||
           !r.Read(owner.module+cataloguedisk::SCPlayerStateClass,native)||native!=prepared.nativeState||!r.Read(owner.module+cataloguedisk::SCPlayerStateConstructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor))return false;
    }
    if(stage)*stage=0;return true;
}
inline bool Transition(const capture::DiagnosticFunctionEvidence& f,const capture::GraphSnapshot& graph,std::uintptr_t module,int entry){
    if(f.function.owner!=graph.function.owner||f.function.label.View()!=u"OnPlayTransitionOut"||f.function.kind.View()!=u"Function"||
       f.flags!=0x08080800||f.parameterBytes!=8||f.storageBytes!=8||f.script.count!=40||f.fieldsCount!=1||f.destructorLink||
       f.nativeExecutor!=module+startdisk::ScriptExecutor||!jasonpicker::WrapperName(f,graph.function.name,19)||f.bytes[0]!=6||f.bytes[9]!=0x76||
       f.bytes[18]!=0x90||f.bytes[31]!=0x5A||f.bytes[36]!=0x97||f.bytes[37]!=0xA5||f.bytes[38]!=0x39||f.bytes[39]!=0xE5)return false;
    int encoded=-1;std::memcpy(&encoded,f.bytes.data()+32,4);const auto& p=f.fields[0];
    return encoded==entry&&p.field.owner==f.function.object&&p.field.label.View()==u"TransitioningTo"&&p.field.kind.View()==u"ClassProperty"&&
        !p.offset&&p.elementBytes==8&&p.dimension==1&&p.flags==0x0/*CFG3.5 address withheld*/40000294ULL;
}
// Metadata-only preparation on the ORIGINAL actual OfflinePlay owner. The
// selected picker class comes solely from its freshly retained authored chain;
// no fake active context, picker instance, class default or lazy native call.
template<class Reader> bool Prepare(Reader& reader,const active::Context& owner,const pickersource::Resources& source,Kind kind,std::uint32_t thread,bool imageVerified,
    Prepared& out,unsigned* reads=nullptr,unsigned* stage=nullptr){
    out={};Bounded<Reader> r(reader);auto value=std::make_unique<Prepared>();auto& p=*value;p.kind=kind;p.source=source;p.type=kind==Kind::Counselor?source.counselor:source.jason;
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};if(stage)*stage=1;
    if(!imageVerified||(kind!=Kind::Counselor&&kind!=Kind::Jason)||!(owner==source.source)||!pickersource::Retained(r,owner,source,thread))return finish(false);
    const bool counselor=kind==Kind::Counselor;constexpr std::array<std::u16string_view,Roles> names={u"",u"Construct",u"OnClick_Accept",u"OnPlayTransitionOut",u"OnClick_Back",u"OfflineBots_AcceptJason",u"GetPreviousMenu"};
    std::array<cfgreflection::Field,Roles> fields{};cfgreflection::Field index,roster;cfgreflection::Observation<Bounded<Reader>> reflection(r,owner.module);
    std::array<std::uintptr_t,256> visited{};unsigned count=0;std::uintptr_t head=0,fresh=0;
    if(stage)*stage=2;
    if(!r.Read(p.type+cfgreflection::disk::Children,head)||!cfgprobe::ObjectPointer(head))return finish(false);
    for(auto at=head;at;){if(count==visited.size()||std::find(visited.begin(),visited.begin()+count,at)!=visited.begin()+count)return finish(false);visited[count++]=at;
        cfgreflection::Field field;if(!reflection.ReadField(at,p.type,field))return finish(false);at=field.next;
        const auto graphName=counselor?u"ExecuteUbergraph_Counselor_Menu":u"ExecuteUbergraph_Jason_Select_Widget";
        if(field.label.View()==graphName){if(fields[0].object)return finish(false);fields[0]=field;}
        for(unsigned i=1;i<Roles;++i)if(field.label.View()==names[i]){if(fields[i].object)return finish(false);fields[i]=field;}
        if(field.label.View()==u"CurrentIndex"){if(index.object)return finish(false);index=field;}
        if(field.label.View()==(counselor?u"CharacterClassList":u"JasonClassList")){if(roster.object)return finish(false);roster=field;}}
    if(!r.Read(p.type+cfgreflection::disk::Children,fresh)||fresh!=head||!fields[0].object||!index.object||!roster.object)return finish(false);
    capture::FunctionDiagnostic diagnostic;startbinding::FastCall fast;
    if(stage)*stage=3;
    if(!capture::Graph(r,owner.module,fields[0],p.graph,diagnostic)||
       !(counselor?counselorpicker::GraphShape(p.graph,p.type,owner.module):jasonpicker::GraphShape(p.graph,p.type,owner.module))||
       !startbinding::ReadFastCall(r,fields[0].object,fast)||fast.graph||fast.entry)return finish(false);
    auto& graphHandler=p.handlers[0];graphHandler.present=true;graphHandler.slot={p.graph.function,p.graph.slot,p.graph.script,p.graph.destructorLink,fast,p.graph.flags,p.graph.parameterBytes,p.graph.storageBytes};
    graphHandler.fields[0]=p.graph.fields[0];graphHandler.fieldCount=1;
    for(unsigned i=1;i<Roles;++i){if(counselor&&i>=5)continue;if(!fields[i].object)return finish(false);
        auto function=std::make_unique<capture::DiagnosticFunctionEvidence>();auto& f=*function;
        if(!capture::KnownFunction(r,owner.module,fields[i],names[i],f)||f.fieldsCount>16||f.script.count>512||!startbinding::ReadFastCall(r,f.function.object,fast)||fast.graph||fast.entry)return finish(false);
        auto& handler=p.handlers[i];int entry=-1;
        if(i==static_cast<unsigned>(Role::Transition)){entry=counselor?counselorpicker::TransitionEntry:jasonpicker::TransitionEntry;if(!Transition(f,p.graph,owner.module,entry))return finish(false);}
        else if(i==static_cast<unsigned>(Role::Previous)){if(!jasonpicker::PreviousShape(f,p.type,owner.module))return finish(false);}
        else{if(f.script.count!=22)return finish(false);std::memcpy(&entry,f.bytes.data()+14,4);
            const int wanted=i==1?(counselor?counselorpicker::ConstructEntry:jasonpicker::ConstructEntry):i==2?(counselor?counselorpicker::AcceptEntry:jasonpicker::AcceptEntry):i==5?jasonpicker::OfflineAcceptEntry:entry;
            if(entry<0||entry>=p.graph.script.count||entry!=wanted||!startbinding::Wrapper(f,owner.module,p.type,names[i],i==1?0x08020808:0x0C000000,entry)||!jasonpicker::WrapperName(f,p.graph.function.name))return finish(false);}
        handler.present=true;handler.entry=entry;handler.slot=startbinding::Record(f,fast);handler.fieldCount=f.fieldsCount;
        std::copy_n(f.bytes.begin(),static_cast<unsigned>(f.script.count),handler.bytes.begin());std::copy_n(f.fields.begin(),f.fieldsCount,handler.fields.begin());}
    if(!capture::RawProperty(r,index,p.index)||!capture::RawProperty(r,roster,p.roster)||p.index.field.kind.View()!=u"IntProperty"||p.index.offset!=(counselor?1456:1672)||p.index.dimension!=1||p.index.elementBytes!=4||p.index.flags!=0x0/*CFG3.5 address withheld*/40010205ULL||
       p.roster.field.kind.View()!=u"ArrayProperty"||p.roster.offset!=(counselor?1440:1656)||p.roster.dimension!=1||p.roster.elementBytes!=16||p.roster.flags!=0x0000000000010205ULL)return finish(false);
    if(counselor){p.lobbyClass=counselorpicker::Pointer(p.graph,counselorpicker::CastOperandOffset);p.baseClass=counselorpicker::Pointer(p.graph,0x1E31);
        if(!counselorpicker::Property(r,owner.module,counselorpicker::Pointer(p.graph,0x103D),p.graph.function.object,u"K2Node_DynamicCast_AsSCPlayer_State_Lobby",u"ObjectProperty",8,1368,0x0008001040000200ULL,p.lobbyLocal)||
           !counselorpicker::Property(r,owner.module,counselorpicker::Pointer(p.graph,0x1E28),p.graph.function.object,u"K2Node_DynamicCast_AsSCPlayer_State",u"ObjectProperty",8,1400,0x0008001040000200ULL,p.baseLocal)||
           !reflection.Find(p.type,u"IsFromLobby",index,false)||index.object!=counselorpicker::Pointer(p.graph,0x1096)||!capture::RawProperty(r,index,p.lobbyFlag)||
           !counselorpicker::DeclaredClass(r,owner.module,p.lobbyLocal,p.lobbyClass)||!counselorpicker::DeclaredClass(r,owner.module,p.baseLocal,p.baseClass))return finish(false);
        std::uintptr_t ctor=0;if(!r.Read(owner.module+cataloguedisk::SCPlayerStateClass,p.nativeState)||!r.Read(owner.module+cataloguedisk::SCPlayerStateConstructor,ctor)||!cfgprobe::MatchingClassCache(p.nativeState,ctor)||
           !maps::Subclass(r,p.lobbyClass,p.nativeState)||!maps::Subclass(r,p.baseClass,p.nativeState))return finish(false);}
    if(stage)*stage=4;
    if(!widgetowner::Observe(r,owner,p.type,p.owningPlayer))return finish(false);
    if(!Current(r,owner,p,thread,Original(p)))return finish(false);
    out=p;return finish(true);
}
// Cheap exact callback prefilter. Kept separate from metadata/ownership proof;
// diagnostics reuse these reads without broadening callback admission.
template<class Reader> bool CallbackFrame(Reader& r,const Prepared& p,unsigned cell,std::uintptr_t widget,std::uintptr_t frame,
    Role& role,pickerdiagnostic::Trace* trace=nullptr){
    using Stage=pickerdiagnostic::Frame;role=Role::Count;
    const auto stage=[&](Stage s){if(trace)trace->frameStage=static_cast<unsigned>(s);};
    const auto observed=[&](std::uint32_t bit,std::uintptr_t value,std::uintptr_t pickerdiagnostic::Trace::*member){if(trace){trace->observed|=bit;trace->*member=value;}};
    stage(Stage::Extent);if(cell>=5||!cfgprobe::ObjectPointer(widget)||!cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    const auto& handler=p.handlers[cell];std::uintptr_t node=0,object=0,code=0,native=0,type=0,locals=0;
    stage(Stage::Type);if(!r.Read(widget+cfgprobe::ObjectClass,type))return false;observed(pickerdiagnostic::Type,type,&pickerdiagnostic::Trace::type);if(type!=p.type)return false;
    stage(Stage::Node);if(!r.Read(frame+startdisk::FrameNode,node))return false;observed(pickerdiagnostic::Node,node,&pickerdiagnostic::Trace::node);if(node!=handler.slot.field.object)return false;
    stage(Stage::Object);if(!r.Read(frame+startdisk::FrameObject,object))return false;observed(pickerdiagnostic::Object,object,&pickerdiagnostic::Trace::object);if(object!=widget)return false;
    stage(Stage::Code);if(!r.Read(frame+startdisk::FrameCode,code))return false;observed(pickerdiagnostic::Code,code,&pickerdiagnostic::Trace::code);if(code!=handler.slot.script.data)return false;
    stage(Stage::Native);if(!r.Read(frame+startdisk::FrameCurrentNative,native))return false;observed(pickerdiagnostic::Native,native,&pickerdiagnostic::Trace::native);if(native!=node)return false;
    if(cell==0){stage(Stage::Locals);int entry=-1;if(!r.Read(frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||!r.Read(locals,entry))return false;
        stage(Stage::Entry);if(entry==Get(p,Role::Construct).entry)role=Role::Construct;
        else if(entry==Get(p,Role::Accept).entry)role=Role::Accept;
        else if(entry==Get(p,Role::Back).entry)role=Role::Back;
    }else role=static_cast<Role>(cell);
    stage(Stage::None);return true;
}
struct Receiver {std::uintptr_t widget=0,type=0;cfgprobe::lifetime::SlotObservation identity{},owningController{};};
template<class Reader> bool ReceiverCurrent(Reader& r,const active::Context& current,const Prepared& prepared,const Executors& expected,const Receiver& receiver,
    std::uint32_t thread,bool ownedCounselorWidening=false,unsigned* metadataStage=nullptr,unsigned* sourceStage=nullptr){
    if(!receiver.identity.HasAssignedSerial()||receiver.type!=prepared.type||receiver.identity.object!=receiver.widget||
       !Current(r,current,prepared,thread,expected,ownedCounselorWidening,metadataStage,sourceStage))return false;
    std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation identity;
    return r.Read(receiver.widget+cfgprobe::ObjectClass,type)&&type==receiver.type&&cfgprobe::lifetime::ReadSlot(r,current.registry,receiver.widget,identity)&&identity==receiver.identity;
}
struct ConstructorState {std::uintptr_t state=0;cfgprobe::lifetime::SlotObservation identity{};cfgreflection::Property playerState{};bool needsWidening=false;};
// Fresh ACTUAL native controller state from the exact named descriptor embedded
// in the counselor Construct graph, never a lobby-class alias or fake state.
// Read-only: this does not request/save a class or replace the controller/state.
template<class Reader> bool CounselorState(Reader& r,const active::Context& current,const Prepared& prepared,const Executors& expected,const Receiver& receiver,
    std::uint32_t thread,ConstructorState& out){
    out={};if(prepared.kind!=Kind::Counselor||!receiver.owningController.HasAssignedSerial()||!(receiver.owningController==current.identities[1])||
       !ReceiverCurrent(r,current,prepared,expected,receiver,thread))return false;
    const auto descriptor=counselorpicker::Pointer(prepared.graph,0x105C);std::uintptr_t owner=0,controllerType=0,stateClass=0;
    cfgreflection::Observation<Reader> reflection(r,current.module);cfgreflection::Field field;ConstructorState result;
    if(!r.Read(descriptor+0x20,owner)||!r.Read(current.controller+cfgprobe::ObjectClass,controllerType)||!maps::Subclass(r,controllerType,owner)||
       !reflection.ReadField(descriptor,owner,field)||field.label.View()!=u"PlayerState"||field.kind.View()!=u"ObjectProperty"||
       !capture::RawProperty(r,field,result.playerState)||result.playerState.offset!=0x388||result.playerState.dimension!=1||result.playerState.elementBytes!=8||
       !r.Read(current.controller+0x388,result.state)||!cfgspawn::ObjectIs(r,result.state,prepared.nativeState)||
       !cfgprobe::lifetime::ReadSlot(r,current.registry,result.state,result.identity)||!result.identity.HasAssignedSerial()||
       !r.Read(result.state+cfgprobe::ObjectClass,stateClass))return false;
    result.needsWidening=!maps::Subclass(r,stateClass,prepared.lobbyClass);std::uintptr_t again=0;cfgprobe::lifetime::SlotObservation identity;
    if(!r.Read(current.controller+0x388,again)||again!=result.state||!cfgprobe::lifetime::ReadSlot(r,current.registry,result.state,identity)||!(identity==result.identity))return false;
    out=result;return true;
}
// Real borrowed native constructor only. It may run BEFORE active top changes.
// No fabricated frame/active context/receiver identity or native serial allocator.
// This establishes callback/source identity, not OwningPlayer/profile/birth proof.
template<class Reader> bool FirstConstruct(Reader& r,const active::Context& current,const Prepared& prepared,const Executors& expected,
    std::uintptr_t receiver,std::uintptr_t frame,std::uint32_t thread,Receiver& out,pickerdiagnostic::Trace* trace=nullptr){
    using Stage=pickerdiagnostic::Construct;
    const auto stage=[&](Stage s){if(trace)trace->constructStage=static_cast<unsigned>(s);};
    const auto observed=[&](std::uint32_t bit,std::uintptr_t value,std::uintptr_t pickerdiagnostic::Trace::*member){if(trace){trace->observed|=bit;trace->*member=value;}};
    out={};stage(Stage::Metadata);
    if(!Current(r,current,prepared,thread,expected,false,trace?&trace->metadataStage:nullptr,trace?&trace->sourceStage:nullptr))return false;
    stage(Stage::Extent);if(!cfgprobe::ObjectPointer(receiver)||receiver==prepared.source.source.widget||
       !cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    Receiver value;value.widget=receiver;std::uintptr_t node=0,object=0,code=0,native=0,locals=0;
    stage(Stage::Type);if(!r.Read(receiver+cfgprobe::ObjectClass,value.type))return false;
    observed(pickerdiagnostic::Type,value.type,&pickerdiagnostic::Trace::type);if(value.type!=prepared.type)return false;
    stage(Stage::Slot);if(!cfgprobe::lifetime::ReadSlot(r,current.registry,receiver,value.identity))return false;
    if(trace){trace->receiverSerial=value.identity.serial;trace->observed|=pickerdiagnostic::Serial;}
    stage(Stage::Serial);if(!value.identity.HasAssignedSerial())return false;
    stage(Stage::Node);if(!r.Read(frame+startdisk::FrameNode,node))return false;observed(pickerdiagnostic::Node,node,&pickerdiagnostic::Trace::node);
    stage(Stage::Object);if(!r.Read(frame+startdisk::FrameObject,object))return false;observed(pickerdiagnostic::Object,object,&pickerdiagnostic::Trace::object);if(object!=receiver)return false;
    stage(Stage::Code);if(!r.Read(frame+startdisk::FrameCode,code))return false;observed(pickerdiagnostic::Code,code,&pickerdiagnostic::Trace::code);
    stage(Stage::Native);if(!r.Read(frame+startdisk::FrameCurrentNative,native))return false;observed(pickerdiagnostic::Native,native,&pickerdiagnostic::Trace::native);if(native!=node)return false;
    stage(Stage::Role);
    const auto& construct=Get(prepared,Role::Construct);
    if(node==construct.slot.field.object){if(code!=construct.slot.script.data)return false;}
    else if(node==prepared.graph.function.object){stage(Stage::Entry);int entry=-1;if(code!=prepared.graph.script.data||!r.Read(frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||!r.Read(locals,entry)||entry!=construct.entry)return false;}
    else return false;
    stage(Stage::Recheck);
    cfgprobe::lifetime::SlotObservation again;std::uintptr_t type=0;
    if(!r.Read(receiver+cfgprobe::ObjectClass,type)||type!=value.type||!cfgprobe::lifetime::ReadSlot(r,current.registry,receiver,again)||!(again==value.identity))return false;
    out=value;stage(Stage::None);return true;
}
// Admission used by the coordinator BEFORE forwarding a first Construct. A
// Cold metadata was sampled through live native hard references, not a weak
// lease. UE can assign its first weak serial during widget initialization.
// Only adopt that one zero->positive observation at first Construct, on the
// same object/index. Fully revalidate the retained native graph/owner chain and
// actual positive receiver BEFORE committing. Once positive, equality remains
// strict. Never call the serial allocator, manufacture an ID or change an actor.
template<class Reader> bool FirstInitializedConstruct(Reader& r,const active::Context& current,Prepared& prepared,const Executors& expected,
    std::uintptr_t widget,std::uintptr_t frame,std::uint32_t thread,Receiver& out,pickerdiagnostic::Trace* trace=nullptr){
    out={};auto candidate=std::make_unique<Prepared>(prepared);unsigned stage=0,assignments=0;
    const auto refresh=[&](cfgprobe::lifetime::SlotObservation& saved){
        ++stage;if(trace)trace->metadataRefreshStage=stage;
        cfgprobe::lifetime::SlotObservation fresh;
        if(!cfgprobe::lifetime::ReadSlot(r,current.registry,saved.object,fresh)||fresh.object!=saved.object||fresh.index!=saved.index||
           (saved.serial!=0&&fresh.serial!=saved.serial))return false;
        if(saved.serial==0&&fresh.serial>0){saved=fresh;++assignments;}return true;
    };
    if(prepared.kind!=Kind::Counselor||thread!=current.thread||current.module!=prepared.source.source.module||!prepared.source.classes[0].HasAssignedSerial())return false;
    for(auto& slot:candidate->source.classes)if(!refresh(slot))return false;
    if(!refresh(candidate->source.mainGraph.slot)||!refresh(candidate->source.customizeGraph.slot)||!refresh(candidate->graph.slot))return false;
    for(auto& handler:candidate->handlers)if(handler.present&&!refresh(handler.slot.identity))return false;
    if(!refresh(candidate->owningPlayer.function.slot))return false;
    if(!FirstConstruct(r,current,*candidate,expected,widget,frame,thread,out,trace))return false;
    // Both independent metadata snapshots and the live receiver are now valid.
    // This is an internal snapshot update, not any write to CFG memory.
    prepared=*candidate;
    if(trace){trace->metadataRefreshStage=0;trace->nativeSerialAssignments=assignments;}return true;
}
// Same cold->native-assigned metadata transition at a proved menu arrival.
// Caller stages a copy and commits only after ALL independent route sources
// are valid. No actor/world/controller lease ever uses this helper.
template<class Reader> bool InitializedMetadata(Reader& r,const active::Context& current,Prepared& staged,const Executors& expected,
    std::uint32_t thread,unsigned* stage=nullptr,unsigned* metadataStage=nullptr,unsigned* sourceStage=nullptr){
    unsigned step=0;
    const auto refresh=[&](cfgprobe::lifetime::SlotObservation& saved){
        if(stage)*stage=++step;cfgprobe::lifetime::SlotObservation fresh;
        if(!cfgprobe::lifetime::ReadSlot(r,current.registry,saved.object,fresh)||fresh.object!=saved.object||fresh.index!=saved.index||
            (saved.serial&&saved.serial!=fresh.serial))return false;saved=fresh;return true;};
    if(staged.kind!=Kind::Counselor||thread!=current.thread||!staged.source.classes[0].HasAssignedSerial())return false;
    for(auto& slot:staged.source.classes)if(!refresh(slot))return false;
    if(!refresh(staged.source.mainGraph.slot)||!refresh(staged.source.customizeGraph.slot)||!refresh(staged.graph.slot))return false;
    for(auto& handler:staged.handlers)if(handler.present&&!refresh(handler.slot.identity))return false;
    if(!refresh(staged.owningPlayer.function.slot)||!Current(r,current,staged,thread,expected,false,metadataStage,sourceStage))return false;
    if(stage)*stage=0;return true;
}
// Admission used by the coordinator BEFORE forwarding a first Construct. A
// structural borrowed-frame match alone is not an OwningPlayer lease. The only
// native operation is one getter on the actual positively identified instance.
// Host dispatch must be the existing exact-build/thread guarded native adapter.
template<class Reader,class Dispatch> bool FirstOwnedConstruct(Reader& reader,Dispatch& dispatch,const active::Context& current,const Prepared& prepared,
    const Executors& expected,std::uintptr_t widget,std::uintptr_t frame,std::uint32_t thread,Receiver& out,unsigned* reads=nullptr){
    static_assert(noexcept(dispatch.Invoke(widget,prepared.owningPlayer.function.function.object,static_cast<void*>(nullptr))),"Owner getter dispatch must protect native failures and be noexcept");
    out={};Receiver value;Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};
    if(dispatch.Thread()!=thread||!FirstConstruct(r,current,prepared,expected,widget,frame,thread,value)||
       !widgetowner::Receiver(r,current,widget,prepared.type,prepared.owningPlayer))return finish(false);
    widgetowner::Packet packet;
    if(!dispatch.Invoke(widget,prepared.owningPlayer.function.function.object,&packet)||
       !widgetowner::Result(r,current,packet,value.owningController)||
       !ReceiverCurrent(r,current,prepared,expected,value,thread)||
       !widgetowner::Receiver(r,current,widget,prepared.type,prepared.owningPlayer))return finish(false);
    out=value;return finish(true);
}
template<class Reader> bool OwnedReceiverCurrent(Reader& r,const active::Context& current,const Prepared& prepared,const Executors& expected,
    const Receiver& receiver,std::uint32_t thread,bool widened=false,unsigned* metadataStage=nullptr,unsigned* sourceStage=nullptr){
    if(!receiver.owningController.HasAssignedSerial()||!(receiver.owningController==current.identities[1]))return false;
    return ReceiverCurrent(r,current,prepared,expected,receiver,thread,widened,metadataStage,sourceStage)&&widgetowner::Receiver(r,current,receiver.widget,prepared.type,prepared.owningPlayer);
}
struct ConstructorDiagnostic {unsigned stage=0,metadataStage=0,sourceStage=0;};
template<class Reader> bool ConstructorStateCurrent(Reader& r,const active::Context& current,const Prepared& prepared,const Executors& expected,
    const Receiver& receiver,const ConstructorState& state,std::uint32_t thread,bool widened=false,ConstructorDiagnostic* trace=nullptr){
    ConstructorDiagnostic diagnostic;diagnostic.stage=1;
    const auto finish=[&](bool okay){if(trace)*trace=diagnostic;return okay;};
    if(prepared.kind!=Kind::Counselor||!state.identity.HasAssignedSerial()||state.identity.object!=state.state)return finish(false);
    diagnostic.stage=2;
    if(!OwnedReceiverCurrent(r,current,prepared,expected,receiver,thread,widened,&diagnostic.metadataStage,&diagnostic.sourceStage))return finish(false);
    diagnostic.stage=3;
    if(counselorpicker::Pointer(prepared.graph,0x105C)!=state.playerState.field.object)return finish(false);
    cfgreflection::Observation<Reader> reflection(r,current.module);cfgreflection::Field field;cfgreflection::Property property;
    std::uintptr_t actual=0,controllerType=0;cfgprobe::lifetime::SlotObservation identity;
    if(!reflection.ReadField(state.playerState.field.object,state.playerState.field.owner,field)||!(field.name==state.playerState.field.name)||
       field.type!=state.playerState.field.type||field.label.View()!=u"PlayerState"||field.kind.View()!=u"ObjectProperty"||
       !capture::RawProperty(r,field,property)||property.dimension!=1||property.elementBytes!=8||property.offset!=0x388||property.flags!=state.playerState.flags||
       !r.Read(current.controller+cfgprobe::ObjectClass,controllerType)||!maps::Subclass(r,controllerType,field.owner)||
       !r.Read(current.controller+0x388,actual)||actual!=state.state||!cfgspawn::ObjectIs(r,actual,prepared.nativeState)||
       !cfgprobe::lifetime::ReadSlot(r,current.registry,actual,identity)||!(identity==state.identity))return finish(false);
    // A retained identity alone cannot authorize adaptation if its sampled
    // class/lobby classification changed during this borrowed native callback.
    diagnostic.stage=4;
    if(state.needsWidening!=!cfgspawn::ObjectIs(r,actual,prepared.lobbyClass))return finish(false);
    diagnostic.stage=0;return finish(true);
}
}
