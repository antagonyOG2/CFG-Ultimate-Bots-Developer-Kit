#pragma once
#include "MenuGraphSnapshot.hpp"
#include "SettingsStartObservation.hpp"
#include "OfflineEntrySettingsObservation.hpp"
#include <algorithm>
namespace cfgmenu::jasonpicker {
constexpr unsigned GraphBytes=23684,PreviousBytes=512,ReadLimit=16384;
constexpr int AcceptEntry=0x233A,TransitionEntry=0x393B,OfflineAcceptEntry=0x3EF2,ConstructEntry=0x405A;
// Captured serialized operands, NOT process addresses or permissions to call.
constexpr std::array<unsigned,3> DestinationConstants={0x2788,0x404F,0x414D};
constexpr std::array<unsigned,3> TransitionOperands={0x2780,0x4047,0x4145};
struct Binding {
    active::Context context{};startbinding::Slot graph{},previous{},accept{},offlineAccept{},construct{},transition{};
    std::array<unsigned char,GraphBytes> bytes{};std::array<unsigned char,PreviousBytes> previousBytes{};
    std::array<std::array<unsigned char,40>,4> wrapperBytes{}; // Construct/Accept/OfflineAccept/transition
    cfgreflection::Property index{},roster{};
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
template<class Function> bool WrapperName(const Function& f,const cfgnames::Words& graph,unsigned at=1){
    std::int32_t one=0,two=0,number=-1;std::memcpy(&one,f.bytes.data()+at,4);std::memcpy(&two,f.bytes.data()+at+4,4);std::memcpy(&number,f.bytes.data()+at+8,4);
    return one==graph.index&&two==graph.index&&number==0&&graph.number==0;
}
template<class Graph> bool GraphShape(const Graph& g,std::uintptr_t type,std::uintptr_t module){
    if(g.function.owner!=type||g.function.label.View()!=u"ExecuteUbergraph_Jason_Select_Widget"||g.function.kind.View()!=u"Function"||
       g.flags!=0x0/*CFG3.5 address withheld*/||g.parameterBytes!=4||g.storageBytes!=3728||g.script.count!=GraphBytes||
       g.nativeExecutor!=module+startdisk::ScriptExecutor||!cfgprobe::ObjectPointer(g.destructorLink)||
       g.bytes[AcceptEntry]!=0xC6||g.bytes[TransitionEntry]!=0xC6||g.bytes[OfflineAcceptEntry]!=0x0A||
       g.bytes[ConstructEntry]!=0x0A)return false;
    std::int32_t target=0;std::memcpy(&target,g.bytes.data()+OfflineAcceptEntry+1,4);
    if(target!=0x3BD0)return false;
    std::memcpy(&target,g.bytes.data()+ConstructEntry+1,4);if(target!=0x3BFE)return false;
    const auto native=Pointer(g,TransitionOperands[0]);
    if(!cfgprobe::ObjectPointer(native)||g.bytes[0x399F]!=0xF5||Pointer(g,0x39A0)!=native)return false;
    for(unsigned i=0;i<DestinationConstants.size();++i){const auto at=DestinationConstants[i];
        if(g.bytes[TransitionOperands[i]-1]!=0xF5||Pointer(g,TransitionOperands[i])!=native||g.bytes[at]!=0x89||
           !cfgprobe::ObjectPointer(Pointer(g,at+1))||g.bytes[at+9]!=0x97||g.bytes[at+10]!=0x87)return false;}
    return true;
}
inline bool PreviousShape(const capture::DiagnosticFunctionEvidence& f,std::uintptr_t type,std::uintptr_t module){
    if(f.function.owner!=type||f.function.label.View()!=u"GetPreviousMenu"||f.function.kind.View()!=u"Function"||
       f.flags!=0x0C420000||f.parameterBytes!=8||f.storageBytes!=88||f.script.count!=PreviousBytes||f.fieldsCount!=11||
       f.destructorLink||f.nativeExecutor!=module+startdisk::ScriptExecutor)return false;
    constexpr std::array<std::u16string_view,11> names={u"PreviousMenu",u"CallFunc_GetOwningPlayer_ReturnValue",u"CallFunc_GetHUD_ReturnValue",
        u"K2Node_DynamicCast_AsSCHUD",u"K2Node_DynamicCast_bSuccess",u"K2Node_DynamicCast_AsILLHUD_Lobby",u"K2Node_DynamicCast_bSuccess2",
        u"CallFunc_GetMenuStackComp_ReturnValue",u"CallFunc_GetMenuStackComp_ReturnValue2",u"CallFunc_GetPreviousMenuClass_ReturnValue",u"CallFunc_GetPreviousMenuClass_ReturnValue2"};
    constexpr std::array<int,11> offsets={0,8,16,24,32,40,48,56,64,72,80};
    for(unsigned i=0;i<names.size();++i){const auto& p=f.fields[i];const bool boolean=i==4||i==6,klass=i==0||i>=9;
        if(p.field.owner!=f.function.object||p.field.label.View()!=names[i]||p.field.kind.View()!=(boolean?u"BoolProperty":klass?u"ClassProperty":u"ObjectProperty")||
           p.dimension!=1||p.offset!=offsets[i]||p.elementBytes!=(boolean?1:8)||
           p.flags!=(i==0?0x0008001040000380ULL:boolean?0x0000001040000200ULL:0x0008001040000200ULL))return false;}
    // Both native HUD variants use the actual MenuStack getter. No destination
    // class literal exists here; PreviousMenu is a native stack result.
    for(const auto pair:{std::array<unsigned,2>{0x0A,0x103},{0x34,0x12D},{0xAD,0x1A6},{0xD7,0x1D0}}){
        if(f.bytes[pair[0]]!=0xF5||f.bytes[pair[1]]!=0xF5||!cfgprobe::ObjectPointer(Pointer(f,pair[0]+1))||Pointer(f,pair[0]+1)!=Pointer(f,pair[1]+1))return false;}
    return true;
}
template<class Reader> bool PreviousNativeNames(Reader& r,std::uintptr_t module,const std::array<unsigned char,PreviousBytes>& bytes){
    constexpr std::array<unsigned,4> offsets={0x0B,0x35,0xAE,0xD8};
    constexpr std::array<std::u16string_view,4> names={u"GetOwningPlayer",u"GetHUD",u"GetMenuStackComp",u"GetPreviousMenuClass"};
    cfgreflection::Observation<Reader> reflection(r,module);
    for(unsigned i=0;i<offsets.size();++i){std::uintptr_t pointer=0,owner=0;std::memcpy(&pointer,bytes.data()+offsets[i],8);cfgreflection::Field field;
        if(!cfgprobe::ObjectPointer(pointer)||!r.Read(pointer+0x20,owner)||!cfgprobe::ObjectPointer(owner)||!reflection.ReadField(pointer,owner,field)||
           field.kind.View()!=u"Function"||field.label.View()!=names[i])return false;}
    return true;
}
template<class Reader> bool StableScript(Reader& r,const startbinding::Slot& slot,const void* bytes,unsigned count){
    capture::ScriptHeader header;auto fresh=std::make_unique<std::array<unsigned char,65536>>();
    return capture::Script(r,slot.field.object,header,*fresh)&&header==slot.script&&header.count==static_cast<int>(count)&&!std::memcmp(fresh->data(),bytes,count);
}
template<class Reader> bool Current(Reader& r,const Binding& b,std::uint32_t thread){
    if(!active::Current(r,b.context,thread)||!StableScript(r,b.graph,b.bytes.data(),GraphBytes)||
       !StableScript(r,b.previous,b.previousBytes.data(),PreviousBytes)||!PreviousNativeNames(r,b.context.module,b.previousBytes))return false;
    const std::array<const startbinding::Slot*,4> wrappers={&b.construct,&b.accept,&b.offlineAccept,&b.transition};
    for(unsigned i=0;i<wrappers.size();++i)if(!StableScript(r,*wrappers[i],b.wrapperBytes[i].data(),i==3?40:22))return false;
    cfgreflection::Observation<Reader> reflection(r,b.context.module);
    for(const auto* slot:{&b.graph,&b.previous,&b.accept,&b.offlineAccept,&b.construct,&b.transition}){
        cfgprobe::lifetime::SlotObservation identity;startbinding::FastCall fast;std::uintptr_t executor=0,cleanup=0;
        cfgreflection::Field field;std::uint32_t flags=0;std::uint16_t parameters=0;int storage=0;
        if(!reflection.ReadField(slot->field.object,b.context.type,field)||field.type!=slot->field.type||!(field.name==slot->field.name)||
           !r.Read(slot->field.object+cfgreflection::disk::FunctionFlags,flags)||flags!=slot->flags||
           !r.Read(slot->field.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=slot->parameters||
           !r.Read(slot->field.object+cfgreflection::disk::StorageBytes,storage)||storage!=slot->storage||
           !r.Read(slot->field.object+0x68,cleanup)||cleanup!=slot->cleanup||
           !cfgprobe::lifetime::ReadSlot(r,b.context.registry,slot->field.object,identity)||!(identity==slot->identity)||
           !r.Read(slot->field.object+0xB0,executor)||executor!=b.context.module+startdisk::ScriptExecutor||
           !startbinding::ReadFastCall(r,slot->field.object,fast)||!(fast==slot->fast))return false;}
    return true;
}
// Binding is current-owner evidence only. It cannot intercept a first Construct
// before native creation/activation, publish callbacks, load classes, infer the
// selected soft-class row, or approve any native call/profile write.
template<class Reader> bool Observe(Reader& reader,const active::Context& c,std::uint32_t thread,bool imageVerified,
    Binding& out,unsigned* reads=nullptr){
    out={};Bounded<Reader> r(reader);auto result=std::make_unique<Binding>();auto& b=*result;b.context=c;
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};cfgnames::Observation<Bounded<Reader>> names(r,c.module);cfgnames::Text label;
    if(!imageVerified||thread!=c.thread||!active::Current(r,c,thread)||!names.Object(c.type,label)||label.View()!=u"Jason_Select_Widget_C")return finish(false);
    cfgreflection::Observation<Bounded<Reader>> reflection(r,c.module);cfgreflection::Field field;capture::FunctionDiagnostic diagnostic;
    // One bounded class-owned walk, not eight repeated full-class Find calls.
    // No child widget graphs or unrelated graph-local frame layouts are read.
    constexpr std::array<std::u16string_view,8> wanted={u"ExecuteUbergraph_Jason_Select_Widget",u"Construct",u"OnClick_Accept",u"OfflineBots_AcceptJason",
        u"OnPlayTransitionOut",u"GetPreviousMenu",u"CurrentIndex",u"JasonClassList"};
    std::array<cfgreflection::Field,8> fields{};std::array<std::uintptr_t,256> seen{};unsigned count=0;std::uintptr_t head=0,again=0;
    if(!r.Read(c.type+cfgreflection::disk::Children,head)||!cfgprobe::ObjectPointer(head))return finish(false);
    auto next=head;while(next){if(count==seen.size()||std::find(seen.begin(),seen.begin()+count,next)!=seen.begin()+count)return finish(false);
        seen[count++]=next;if(!reflection.ReadField(next,c.type,field))return finish(false);
        for(unsigned i=0;i<wanted.size();++i)if(field.label.View()==wanted[i]){if(fields[i].object)return finish(false);fields[i]=field;}
        next=field.next;}
    if(!r.Read(c.type+cfgreflection::disk::Children,again)||again!=head)return finish(false);
    for(const auto& candidate:fields)if(!candidate.object)return finish(false);
    auto graph=std::make_unique<capture::GraphSnapshot>();startbinding::FastCall fast;
    field=fields[0];if(!capture::Graph(r,c.module,field,*graph,diagnostic)||
       !GraphShape(*graph,c.type,c.module)||!startbinding::ReadFastCall(r,field.object,fast)||fast.graph||fast.entry)return finish(false);
    b.graph={graph->function,graph->slot,graph->script,graph->destructorLink,fast,graph->flags,graph->parameterBytes,graph->storageBytes};
    std::copy_n(graph->bytes.begin(),GraphBytes,b.bytes.begin());
    const auto wrapper=[&](std::u16string_view name,std::uint32_t flags,int entry,startbinding::Slot& slot,unsigned index){
        auto function=std::make_unique<capture::DiagnosticFunctionEvidence>();startbinding::FastCall alias;
        if(!capture::KnownFunction(r,c.module,fields[index+1],name,*function)||!startbinding::Wrapper(*function,c.module,c.type,name,flags,entry)||
           !WrapperName(*function,graph->function.name)||!startbinding::ReadFastCall(r,function->function.object,alias)||alias.graph||alias.entry)return false;
        slot=startbinding::Record(*function,alias);std::copy_n(function->bytes.begin(),22,b.wrapperBytes[index].begin());return true;};
    if(!wrapper(u"Construct",0x08020808,ConstructEntry,b.construct,0)||!wrapper(u"OnClick_Accept",0x0C000000,AcceptEntry,b.accept,1)||
       !wrapper(u"OfflineBots_AcceptJason",0x0C000000,OfflineAcceptEntry,b.offlineAccept,2))return finish(false);
    auto function=std::make_unique<capture::DiagnosticFunctionEvidence>();
    if(!capture::KnownFunction(r,c.module,fields[4],u"OnPlayTransitionOut",*function)||function->flags!=0x08080800||function->parameterBytes!=8||
       function->storageBytes!=8||function->script.count!=40||function->fieldsCount!=1||function->destructorLink||
       function->nativeExecutor!=c.module+startdisk::ScriptExecutor||!WrapperName(*function,graph->function.name,19)||
       function->bytes[0]!=6||function->bytes[9]!=0x76||function->bytes[18]!=0x90||function->bytes[31]!=0x5A||
       function->bytes[36]!=0x97||function->bytes[37]!=0xA5||function->bytes[38]!=0x39||function->bytes[39]!=0xE5)return finish(false);
    int transition=-1;std::memcpy(&transition,function->bytes.data()+32,4);const auto& p=function->fields[0];
    if(transition!=TransitionEntry||p.field.owner!=function->function.object||p.field.label.View()!=u"TransitioningTo"||p.field.kind.View()!=u"ClassProperty"||
       p.offset||p.elementBytes!=8||p.dimension!=1||p.flags!=0x0/*CFG3.5 address withheld*/40000294ULL||
       !startbinding::ReadFastCall(r,function->function.object,fast)||fast.graph||fast.entry)return finish(false);
    b.transition=startbinding::Record(*function,fast);std::copy_n(function->bytes.begin(),40,b.wrapperBytes[3].begin());
    if(!capture::KnownFunction(r,c.module,fields[5],u"GetPreviousMenu",*function)||!PreviousShape(*function,c.type,c.module)||
       !startbinding::ReadFastCall(r,function->function.object,fast)||fast.graph||fast.entry)return finish(false);
    b.previous=startbinding::Record(*function,fast);std::copy_n(function->bytes.begin(),PreviousBytes,b.previousBytes.begin());
    if(!capture::RawProperty(r,fields[6],b.index)||!capture::RawProperty(r,fields[7],b.roster))return finish(false);
    if(b.index.field.kind.View()!=u"IntProperty"||b.index.offset!=1672||b.index.elementBytes!=4||b.index.dimension!=1||b.index.flags!=0x0/*CFG3.5 address withheld*/40010205ULL||
       b.roster.field.kind.View()!=u"ArrayProperty"||b.roster.offset!=1656||b.roster.elementBytes!=16||b.roster.dimension!=1||b.roster.flags!=0x0000000000010205ULL||
       !Current(r,b,thread))return finish(false);
    out=b;return finish(true);
}
enum class Callback {Construct,Accept,OfflineAccept,Transition,Previous,Other};
template<class Reader> bool Frame(Reader& r,const Binding& b,std::uintptr_t receiver,std::uintptr_t frame,std::uint32_t thread,Callback& kind){
    kind=Callback::Other;if(receiver!=b.context.widget||thread!=b.context.thread||!cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8))return false;
    std::uintptr_t node=0,object=0,code=0,current=0,locals=0;
    if(!r.Read(frame+startdisk::FrameNode,node)||!r.Read(frame+startdisk::FrameObject,object)||object!=receiver||
       !r.Read(frame+startdisk::FrameCode,code)||!r.Read(frame+startdisk::FrameCurrentNative,current)||current!=node)return false;
    if(node==b.graph.field.object){int entry=-1;if(code!=b.graph.script.data||!r.Read(frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||!r.Read(locals,entry))return false;
        kind=entry==ConstructEntry?Callback::Construct:entry==AcceptEntry?Callback::Accept:entry==OfflineAcceptEntry?Callback::OfflineAccept:entry==TransitionEntry?Callback::Transition:Callback::Other;}
    else{const std::array<const startbinding::Slot*,5> slots={&b.construct,&b.accept,&b.offlineAccept,&b.transition,&b.previous};
        constexpr std::array<Callback,5> kinds={Callback::Construct,Callback::Accept,Callback::OfflineAccept,Callback::Transition,Callback::Previous};
        for(unsigned i=0;i<slots.size();++i)if(node==slots[i]->field.object&&code==slots[i]->script.data){kind=kinds[i];break;}}
    return kind!=Callback::Other;
}
enum class Destination {Other,Settings,OfflineMap,Counselor};
inline Destination Classify(std::u16string_view name){
    return name==u"OfflineBotsSettingsMenuWidget_C"?Destination::Settings:name==u"PickOfflineBotsMapMenuWidget_C"?Destination::OfflineMap:
        name==u"Counselor_Menu_C"?Destination::Counselor:Destination::Other;
}
struct Reference {std::uintptr_t type=0;cfgprobe::lifetime::SlotObservation identity{};cfgnames::Text name{};Destination destination=Destination::Other;};
template<class Reader> bool NamedNativeReferences(Reader& r,const active::Context& c,std::uintptr_t referenceNative,const std::array<std::uintptr_t,3>& types,std::array<Reference,3>& out){
    out={};std::uintptr_t native=0,owner=0,ctor=0,executor=0;cfgreflection::Field field;cfgreflection::Observation<Reader> reflection(r,c.module);
    if(!r.Read(c.module+transitiondisk::FunctionCache,native)||native!=referenceNative||
       !r.Read(c.module+transitiondisk::Owner,owner)||!r.Read(c.module+transitiondisk::Constructor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor)||
       !maps::Subclass(r,c.type,owner)||!reflection.ReadField(native,owner,field)||field.kind.View()!=u"Function"||field.label.View()!=u"PlayTransitionOut"||
       !r.Read(native+0xB0,executor)||executor!=c.module+transitiondisk::Executor)return false;
    std::array<unsigned char,transitiondisk::Code.size()> code{};std::uint32_t flags=0;std::uint16_t parameters=0;int storage=0;
    if(!r.ReadBytes(executor,code.data(),code.size())||code!=transitiondisk::Code||!r.Read(native+cfgreflection::disk::FunctionFlags,flags)||flags!=0x04020409||
       !r.Read(native+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=8||!r.Read(native+cfgreflection::disk::StorageBytes,storage)||storage!=8)return false;
    std::array<Reference,3> result;cfgnames::Observation<Reader> names(r,c.module);
    for(unsigned i=0;i<result.size();++i){auto& value=result[i];value.type=types[i];
        if(!names.Object(value.type,value.name)||!maps::Subclass(r,value.type,owner)||!cfgprobe::lifetime::ReadSlot(r,c.registry,value.type,value.identity))return false;
        value.destination=Classify(value.name.View());}
    for(const auto& value:result){cfgprobe::lifetime::SlotObservation identity;cfgnames::Text name;
        if(!names.Object(value.type,name)||name.View()!=value.name.View()||!cfgprobe::lifetime::ReadSlot(r,c.registry,value.type,identity)||!(identity==value.identity))return false;}
    out=result;return true;
}
// Late graph literals were captured but their pointee names were omitted by the
// old 64-reference diagnostic cap. Resolve ONLY these exact current native-owned
// references; no registry/name scan, CDO manufacture or lazy loader. Zero-serial
// metadata is valid evidence only under this current positive-serial widget.
template<class Reader> bool LateReferences(Reader& reader,const Binding& b,std::uint32_t thread,bool imageVerified,std::array<Reference,3>& out,unsigned* reads=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};
    if(!imageVerified||!Current(r,b,thread))return finish(false);
    std::array<std::uintptr_t,3> types{};for(unsigned i=0;i<types.size();++i)types[i]=Pointer(b,DestinationConstants[i]+1);
    std::array<Reference,3> result;
    if(!NamedNativeReferences(r,b.context,Pointer(b,TransitionOperands[0]),types,result)||!Current(r,b,thread))return finish(false);
    out=result;return finish(true);
}
// Diagnostic adapter for the graph-only capture helper: reuse its saved owned
// snapshot and validate just these late native references. No repeated primary
// class walk, selection/preview local metadata or wrapper invocation is needed.
template<class Reader> bool LateGraphReferences(Reader& reader,const active::Context& c,const capture::GraphSnapshot& saved,std::uint32_t thread,
    bool imageVerified,std::array<Reference,3>& out,unsigned* reads=nullptr){
    out={};Bounded<Reader> r(reader);const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};
    cfgnames::Observation<Bounded<Reader>> names(r,c.module);cfgnames::Text label;
    if(!imageVerified||thread!=c.thread||!active::Current(r,c,thread)||!names.Object(c.type,label)||label.View()!=u"Jason_Select_Widget_C"||
       !GraphShape(saved,c.type,c.module))return finish(false);
    auto first=std::make_unique<capture::GraphSnapshot>(),second=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
    const auto same=[&](const capture::GraphSnapshot& sample){return GraphShape(sample,c.type,c.module)&&sample.function.type==saved.function.type&&
        sample.function.name==saved.function.name&&sample.script==saved.script&&sample.slot==saved.slot&&sample.bytes==saved.bytes&&
        sample.destructorLink==saved.destructorLink;};
    if(!capture::Graph(r,c.module,saved.function,*first,diagnostic)||!same(*first))return finish(false);
    std::array<std::uintptr_t,3> types{};for(unsigned i=0;i<types.size();++i)types[i]=Pointer(*first,DestinationConstants[i]+1);
    std::array<Reference,3> result;
    if(!NamedNativeReferences(r,c,Pointer(*first,TransitionOperands[0]),types,result)||!active::Current(r,c,thread)||
       !capture::Graph(r,c.module,first->function,*second,diagnostic)||!same(*second))return finish(false);
    out=result;return finish(true);
}
}
