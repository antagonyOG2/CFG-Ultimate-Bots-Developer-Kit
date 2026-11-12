#pragma once
#include "MenuGraphSnapshot.hpp"
#include "MenuStackParentObservation.hpp"
#include "NativePickerSource.generated.hpp"
#include "MenuTransition.generated.hpp"
#include "SettingsStart.generated.hpp"
#include "NativeMapSelectionObservation.hpp"
#include "NativeMenuTransitionObservation.hpp"
namespace cfgmenu::pickersource {
constexpr unsigned ReadLimit=16384,MainBytes=8161,CustomizeBytes=1526;
constexpr unsigned MainCustomize=0x5EC,CustomizeCounselor=0x240,CustomizeJason=0x28F;
struct Resources {
    active::Context source{};parentcapture::Record root{};
    std::uintptr_t customize=0,counselor=0,jason=0,transition=0;
    std::array<cfgprobe::lifetime::SlotObservation,4> classes{}; // root/customize/counselor/Jason
    capture::GraphSnapshot mainGraph{},customizeGraph{};
};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
inline std::uintptr_t Pointer(const capture::GraphSnapshot& graph,unsigned at){std::uintptr_t value=0;
    if(at+8<=static_cast<unsigned>(graph.script.count))std::memcpy(&value,graph.bytes.data()+at,8);return value;}
inline bool Edge(const capture::GraphSnapshot& graph,unsigned at,std::uintptr_t native,std::uintptr_t& type){
    type=0;if(at<9||at+10>=static_cast<unsigned>(graph.script.count)||graph.bytes[at-9]!=pickersourcedisk::FinalFunction||
        Pointer(graph,at-8)!=native||graph.bytes[at]!=pickersourcedisk::ObjectConst||graph.bytes[at+9]!=0x97||graph.bytes[at+10]!=0x87)return false;
    type=Pointer(graph,at+1);return cfgprobe::ObjectPointer(type)&&cfgprobe::ObjectPointer(native);
}
inline bool MainShape(const capture::GraphSnapshot& graph,std::uintptr_t type,std::uintptr_t module){
    return graph.function.owner==type&&graph.function.kind.View()==u"Function"&&graph.function.label.View()==u"ExecuteUbergraph_MainMenuWidget"&&
        graph.flags==0x0/*CFG3.5 address withheld*/&&graph.parameterBytes==4&&graph.storageBytes==1072&&graph.script.count==MainBytes&&
        graph.nativeExecutor==module+startdisk::ScriptExecutor&&cfgprobe::ObjectPointer(graph.destructorLink);
}
inline bool CustomizeShape(const capture::GraphSnapshot& graph,std::uintptr_t type,std::uintptr_t module){
    return graph.function.owner==type&&graph.function.kind.View()==u"Function"&&graph.function.label.View()==u"ExecuteUbergraph_CustomizeMenuWidget"&&
        !graph.flags&&graph.parameterBytes==4&&graph.storageBytes==137&&graph.script.count==CustomizeBytes&&
        graph.nativeExecutor==module+startdisk::ScriptExecutor&&!graph.destructorLink;
}
template<class Reader> bool Opcodes(Reader& r,std::uintptr_t module){
    std::uintptr_t object=0,final=0;std::array<unsigned char,pickersourcedisk::ObjectCode.size()> objectCode{};
    std::array<unsigned char,pickersourcedisk::FinalCode.size()> finalCode{};
    return r.Read(module+pickersourcedisk::ObjectSlot,object)&&object==module+pickersourcedisk::ObjectExecutor&&
        r.Read(module+pickersourcedisk::FinalSlot,final)&&final==module+pickersourcedisk::FinalExecutor&&
        r.ReadBytes(object,objectCode.data(),objectCode.size())&&objectCode==pickersourcedisk::ObjectCode&&
        r.ReadBytes(final,finalCode.data(),finalCode.size())&&finalCode==pickersourcedisk::FinalCode;
}
template<class Reader> bool NativeOwners(Reader& r,const active::Context& c,std::uintptr_t& menu,std::uintptr_t& native,std::uintptr_t expected=0){
    menu=native=0;std::uintptr_t ctor=0,stackOwner=0,stackCtor=0,stackType=0;
    if(!parentcapture::NativeCode(r,c.module)||!Opcodes(r,c.module)||
       !r.Read(c.module+parentdisk::Owner,stackOwner)||!r.Read(c.module+parentdisk::Constructor,stackCtor)||!cfgprobe::MatchingClassCache(stackOwner,stackCtor)||
       !r.Read(c.stack+cfgprobe::ObjectClass,stackType)||!maps::Subclass(r,stackType,stackOwner)||
       !r.Read(c.module+transitiondisk::Owner,menu)||!r.Read(c.module+transitiondisk::Constructor,ctor)||!cfgprobe::MatchingClassCache(menu,ctor)||
       !maps::Subclass(r,c.type,menu))return false;
    return nativetransition::Observe(r,c.module,menu,native,expected);
}
template<class Reader> bool Class(Reader& r,const active::Context& c,std::uintptr_t type,std::uintptr_t menu,std::u16string_view name,cfgprobe::lifetime::SlotObservation& identity){
    cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text label,kind;std::uintptr_t meta=0;
    return cfgprobe::ObjectPointer(type)&&names.Object(type,label)&&label.View()==name&&r.Read(type+cfgprobe::ObjectClass,meta)&&
        names.Object(meta,kind)&&kind.View()==u"WidgetBlueprintGeneratedClass"&&maps::Subclass(r,type,menu)&&cfgprobe::lifetime::ReadSlot(r,c.registry,type,identity);
}
template<class Reader> bool GraphCurrent(Reader& r,std::uintptr_t module,const capture::GraphSnapshot& saved){
    auto fresh=std::make_unique<capture::GraphSnapshot>();capture::FunctionDiagnostic diagnostic;
    return capture::Graph(r,module,saved.function,*fresh,diagnostic)&&fresh->function.type==saved.function.type&&fresh->function.name==saved.function.name&&
        fresh->script==saved.script&&fresh->slot==saved.slot&&fresh->flags==saved.flags&&fresh->storageBytes==saved.storageBytes&&
        fresh->destructorLink==saved.destructorLink&&fresh->nativeExecutor==saved.nativeExecutor&&fresh->bytes==saved.bytes;
}
// A source chain lease is a fresh native stack CLASS record plus its unchanged
// serialized class references, not the optional parent Widget. Zero metadata
// serials are never promoted/assigned. Losing ANY owner/reference refuses.
template<class Reader> bool Retained(Reader& r,const active::Context& current,const Resources& saved,std::uint32_t thread,unsigned* stage=nullptr){
    const auto& source=saved.source;std::uintptr_t menu=0,native=0;
    if(stage)*stage=1;
    if(current.module!=source.module||current.thread!=source.thread||thread!=current.thread||current.world!=source.world||current.mode!=source.mode||
       current.controller!=source.controller||current.hud!=source.hud||current.stack!=source.stack||current.gameInstance!=source.gameInstance)return false;
    if(stage)*stage=2;if(!active::Current(r,current,thread))return false;
    if(stage)*stage=3;if(!parentcapture::Current(r,current,saved.root))return false;
    if(stage)*stage=4;if(!NativeOwners(r,current,menu,native,saved.transition)||native!=saved.transition)return false;
    if(stage)*stage=5;if(
       !MainShape(saved.mainGraph,saved.root.menuClass,current.module)||!CustomizeShape(saved.customizeGraph,saved.customize,current.module))return false;
    // World/controller/HUD identity persists across the native push. The active
    // widget may change, but recycled infrastructure is never a source lease.
    if(stage)*stage=6;for(const auto i:{0U,1U,3U})if(!source.identities[i].HasAssignedSerial()||!(current.identities[i]==source.identities[i]))return false;
    std::uintptr_t customize=0,counselor=0,jason=0;
    if(stage)*stage=7;if(!Edge(saved.mainGraph,MainCustomize,native,customize)||customize!=saved.customize||
       !Edge(saved.customizeGraph,CustomizeCounselor,native,counselor)||counselor!=saved.counselor||
       !Edge(saved.customizeGraph,CustomizeJason,native,jason)||jason!=saved.jason)return false;
    constexpr std::array<std::u16string_view,4> names={u"MainMenuWidget_C",u"CustomizeMenuWidget_C",u"Counselor_Menu_C",u"Jason_Select_Widget_C"};
    const std::array<std::uintptr_t,4> types={saved.root.menuClass,saved.customize,saved.counselor,saved.jason};
    for(unsigned i=0;i<types.size();++i){cfgprobe::lifetime::SlotObservation identity;
        if(stage)*stage=10+i;
        if(!Class(r,current,types[i],menu,names[i],identity)||!(identity==saved.classes[i])||(i==0&&!identity.HasAssignedSerial()))return false;}
    if(stage)*stage=20;if(!GraphCurrent(r,current.module,saved.mainGraph))return false;
    if(stage)*stage=21;if(!GraphCurrent(r,current.module,saved.customizeGraph))return false;
    if(stage)*stage=22;if(!parentcapture::Current(r,current,saved.root))return false;
    if(stage)*stage=23;if(!active::Current(r,current,thread))return false;
    if(stage)*stage=0;
    return true;
}
// First OfflinePlay: native retained MainMenu -> authored Customize graph ->
// authored Counselor/Jason classes. No customization visit, CDO, class getter,
// soft path, global object/name discovery, native call, or callback publication.
template<class Reader> bool Observe(Reader& reader,const active::Context& c,std::uint32_t thread,bool exactImageVerified,Resources& out,unsigned* reads=nullptr,unsigned* stage=nullptr){
    out={};Bounded<Reader> r(reader);auto value=std::make_unique<Resources>();auto& result=*value;result.source=c;
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};cfgnames::Observation<Bounded<Reader>> names(r,c.module);cfgnames::Text label;
    if(stage)*stage=1;
    if(!exactImageVerified||thread!=c.thread||c.array.count!=2||!active::Current(r,c,thread)||!names.Object(c.type,label)||label.View()!=u"OfflinePlayMenuWidget_C")return finish(false);
    std::uintptr_t menu=0;
    if(stage)*stage=2;
    if(!NativeOwners(r,c,menu,result.transition)||!parentcapture::Observe(r,c,0,result.root)||
       !Class(r,c,result.root.menuClass,menu,u"MainMenuWidget_C",result.classes[0])||!result.classes[0].HasAssignedSerial())return finish(false);
    cfgreflection::Observation<Bounded<Reader>> reflection(r,c.module);cfgreflection::Field field;capture::FunctionDiagnostic diagnostic;
    if(stage)*stage=3;
    if(!reflection.Find(result.root.menuClass,u"ExecuteUbergraph_MainMenuWidget",field,false)||field.owner!=result.root.menuClass||
       !capture::Graph(r,c.module,field,result.mainGraph,diagnostic)||!MainShape(result.mainGraph,result.root.menuClass,c.module)||
       !Edge(result.mainGraph,MainCustomize,result.transition,result.customize)||
       !Class(r,c,result.customize,menu,u"CustomizeMenuWidget_C",result.classes[1]))return finish(false);
    if(stage)*stage=4;
    if(!reflection.Find(result.customize,u"ExecuteUbergraph_CustomizeMenuWidget",field,false)||field.owner!=result.customize||
       !capture::Graph(r,c.module,field,result.customizeGraph,diagnostic)||!CustomizeShape(result.customizeGraph,result.customize,c.module)||
       !Edge(result.customizeGraph,CustomizeCounselor,result.transition,result.counselor)||!Class(r,c,result.counselor,menu,u"Counselor_Menu_C",result.classes[2])||
       !Edge(result.customizeGraph,CustomizeJason,result.transition,result.jason)||!Class(r,c,result.jason,menu,u"Jason_Select_Widget_C",result.classes[3]))return finish(false);
    if(stage)*stage=5;
    if(result.counselor==result.jason||result.customize==result.root.menuClass||!Retained(r,c,result,thread))return finish(false);
    out=result;return finish(true);
}
}
