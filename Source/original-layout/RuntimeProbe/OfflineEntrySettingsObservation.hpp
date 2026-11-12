#pragma once
#include "MenuScriptObservation.hpp"
#include "PendingModeMemberObservation.hpp"
#include "MenuPush.generated.hpp"
#include "MenuTransition.generated.hpp"
#include "SettingsStart.generated.hpp"
#include "NativeMenuTransitionObservation.hpp"
#include <memory>
namespace cfgmenu::entrysettings {
constexpr unsigned ReadLimit=16384;
// These are serialized reference locations, independently present in captures
// 21645-31640 and 21510-26961. They are not native call or hook entry points.
constexpr unsigned EntryMapConstant=0x3F3,MapSettingsConstant=0x128;
struct ResetDiagnostic {unsigned stage=0;pendingmember::PropertyDiagnostic property{};std::uintptr_t savedNext=0,currentNext=0;};
struct Resources {
    active::Context source{};std::uintptr_t map=0,settings=0,push=0;
    cfgprobe::lifetime::SlotObservation mapIdentity{},settingsIdentity{};
    cfgreflection::Property pending{},pendingMode{},pendingMap{};
    capture::DiagnosticFunctionEvidence entry{},mapGraph{};
    ResetDiagnostic reset{};
};
template<class Reader> class Bounded {
    Reader& r_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):r_(reader){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return r_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return r_.ReadBytes(at,out,bytes);}
};
inline std::uintptr_t Pointer(const capture::DiagnosticFunctionEvidence& f,unsigned offset){
    std::uintptr_t value=0;if(offset+sizeof(value)<=static_cast<unsigned>(f.script.count))std::memcpy(&value,f.bytes.data()+offset,sizeof(value));return value;
}
inline bool EntryReference(const capture::DiagnosticFunctionEvidence& f,std::uintptr_t owner,std::uintptr_t module,
    std::uintptr_t push,std::uintptr_t& map){
    map=0;
    if(f.function.owner!=owner||f.function.label.View()!=u"ExecuteUbergraph_OfflinePlayMenuWidget"||
       f.function.kind.View()!=u"Function"||f.flags!=0x0/*CFG3.5 address withheld*/||f.parameterBytes!=4||f.storageBytes!=113||
       f.script.count!=1944||f.nativeExecutor!=module+startdisk::ScriptExecutor||f.destructorLink||
       f.bytes[0x3E8]!=0x97||f.bytes[0x3E9]!=0x87||f.bytes[0x3EA]!=0xF5||
       Pointer(f,0x3EB)!=push||f.bytes[EntryMapConstant]!=0x89||f.bytes[0x3FC]!=0x97||f.bytes[0x3FD]!=0x87)return false;
    map=Pointer(f,EntryMapConstant+1);return cfgprobe::ObjectPointer(push)&&cfgprobe::ObjectPointer(map);
}
inline bool SettingsReference(const capture::DiagnosticFunctionEvidence& f,std::uintptr_t owner,std::uintptr_t module,
    std::uintptr_t pending,std::uintptr_t& settings){
    settings=0;
    if(f.function.owner!=owner||f.function.label.View()!=u"ExecuteUbergraph_PickOfflineBotsMapMenuWidget"||
       f.function.kind.View()!=u"Function"||f.flags||f.parameterBytes!=4||f.storageBytes!=193||
       f.script.count!=1867||f.nativeExecutor!=module+startdisk::ScriptExecutor||!cfgprobe::ObjectPointer(f.destructorLink)||
       f.bytes[0x108]!=0x0D||f.bytes[0x109]!=0xC6||f.bytes[0x10A]!=0x76||f.bytes[0x113]!=9||
       f.bytes[0x114]||f.bytes[0x115]||f.bytes[0x116]||Pointer(f,0x117)!=pending||
       f.bytes[0x11F]!=0xDB||Pointer(f,0x120)!=pending||f.bytes[MapSettingsConstant]!=0x89||f.bytes[0x131]!=0x87)return false;
    settings=Pointer(f,MapSettingsConstant+1);return cfgprobe::ObjectPointer(pending)&&cfgprobe::ObjectPointer(settings);
}
template<class Reader> bool StableScript(Reader& r,const capture::DiagnosticFunctionEvidence& saved){
    capture::ScriptHeader header;auto bytes=std::make_unique<std::array<unsigned char,scriptdisk::MaxBytes>>();
    return capture::Script(r,saved.function.object,header,*bytes)&&header==saved.script&&*bytes==saved.bytes;
}
// Three consecutive native resets are independently present at these offsets
// in both archived map graphs. Addresses come only from authored operands;
// native property allocation order or address strides are never inferred.
inline bool ResetReferences(const capture::DiagnosticFunctionEvidence& f,std::uintptr_t settings,
    std::array<std::uintptr_t,3>& members,std::uintptr_t& local){
    members={};local=0;if(f.script.count!=1867)return false;
    constexpr std::array<unsigned,3> starts={0x5D2,0x5F3,0x614};
    for(unsigned i=0;i<starts.size();++i){const auto at=starts[i];
        if(f.bytes[at]!=0x0D||f.bytes[at+1]!=0xC6||f.bytes[at+2]!=0x76||
           f.bytes[at+11]!=9||f.bytes[at+12]||f.bytes[at+13]||f.bytes[at+14]||
           f.bytes[at+23]!=0xDB||f.bytes[at+32]!=0x7B)return false;
        const auto context=Pointer(f,at+3);if(!cfgprobe::ObjectPointer(context)||(i&&context!=local))return false;
        local=context;members[i]=Pointer(f,at+15);
        if(!cfgprobe::ObjectPointer(members[i])||Pointer(f,at+24)!=members[i])return false;
        for(unsigned j=0;j<i;++j)if(members[j]==members[i])return false;
    }
    return members[2]==settings;
}
template<class Reader> bool ResetMembers(Reader& r,const active::Context& c,std::uintptr_t frontend,
    const capture::DiagnosticFunctionEvidence& graph,const cfgreflection::Property& settings,
    cfgreflection::Property& mode,cfgreflection::Property& map,ResetDiagnostic* diagnostic=nullptr){
    mode={};map={};std::array<std::uintptr_t,3> members{};std::uintptr_t local=0;
    cfgreflection::Property m,p,s,context;
    ResetDiagnostic detail;const auto mark=[&](unsigned stage){detail.stage=stage;if(diagnostic)*diagnostic=detail;};
    const auto property=[&](unsigned stage,std::uintptr_t at,std::uintptr_t owner,std::u16string_view name,std::u16string_view kind,int offset,cfgreflection::Property& out){
        const bool okay=pendingmember::PropertyAt(r,c,at,owner,name,kind,8,offset,out,&detail.property);mark(stage);return okay;};
    mark(1);if(!ResetReferences(graph,settings.field.object,members,local))return false;
    if(!property(2,local,graph.function.object,u"K2Node_DynamicCast_AsSCGame_Menu2",u"ObjectProperty",184,context))return false;
    mark(3);if(context.flags!=0x0/*CFG3.5 address withheld*/40000200ULL)return false;
    if(!property(4,members[0],frontend,u"PendingOfflineMode",u"ClassProperty",selectiondisk::PendingOfflineMode,m)||
       !property(5,members[1],frontend,u"PendingOfflineMap",u"ClassProperty",selectiondisk::PendingOfflineMap,p)||
       !property(6,members[2],frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",selectiondisk::PendingOfflineSettingsWidget,s))return false;
    mark(7);if(m.flags!=0x0/*CFG3.5 address withheld*/40000204ULL||p.flags!=m.flags||s.flags!=m.flags)return false;
    mark(8);if(m.field.type!=s.field.type||p.field.type!=s.field.type)return false;
    detail.savedNext=settings.field.next;detail.currentNext=s.field.next;mark(9);if(!pendingmember::Same(s,settings))return false;
    mark(0);
    mode=m;map=p;return true;
}
// Read only the active entry's authored dependency chain. No scan, path-based
// load, factory, dispatcher or menu/character mutation. Metadata serial zero
// remains explicitly unassigned; this observation never allocates a serial.
// Re-observe from the current native owner callback before integration. The
// result does not approve a counselor click, settings Start or gameplay birth.
template<class Reader> bool Observe(Reader& reader,const active::Context& c,std::uint32_t thread,
    bool exactImageVerified,Resources& out,unsigned* reads=nullptr,unsigned* stage=nullptr,
    nativetransition::Diagnostic* transitionDiagnostic=nullptr,pendingmember::PropertyDiagnostic* propertyDiagnostic=nullptr){
    out={};Bounded<Reader> r(reader);auto result=std::make_unique<Resources>();
    if(transitionDiagnostic)*transitionDiagnostic={};
    if(propertyDiagnostic)*propertyDiagnostic={};
    const auto finish=[&](bool okay){if(reads)*reads=r.reads;return okay;};
    result->source=c;cfgnames::Observation<Bounded<Reader>> names(r,c.module);cfgnames::Text label;
    std::uintptr_t menu=0,menuCtor=0,frontend=0,frontendCtor=0,modeType=0;
    if(stage)*stage=1;
    if(!exactImageVerified||thread!=c.thread||!active::Current(r,c,thread))return finish(false);
    if(stage)*stage=101;
    if(!names.Object(c.type,label)||label.View()!=u"OfflinePlayMenuWidget_C")return finish(false);
    if(stage)*stage=102;
    if(!r.Read(c.module+pushdisk::MenuClass,menu)||!r.Read(c.module+pushdisk::MenuConstructor,menuCtor)||
       !cfgprobe::MatchingClassCache(menu,menuCtor))return finish(false);
    if(stage)*stage=103;
    if(!maps::Subclass(r,c.type,menu))return finish(false);
    if(stage)*stage=104;
    if(!r.Read(c.module+selectiondisk::SCGame_MenuClass,frontend)||
       !r.Read(c.module+selectiondisk::SCGame_MenuConstructor,frontendCtor)||!cfgprobe::MatchingClassCache(frontend,frontendCtor))return finish(false);
    if(stage)*stage=105;
    if(!r.Read(c.mode+cfgprobe::ObjectClass,modeType)||!maps::Subclass(r,modeType,frontend))return finish(false);
    if(stage)*stage=23;
    std::uintptr_t transitionOwner=0,transitionCtor=0;
    if(!r.Read(c.module+transitiondisk::Owner,transitionOwner)||!r.Read(c.module+transitiondisk::Constructor,transitionCtor)||
       !cfgprobe::MatchingClassCache(transitionOwner,transitionCtor)||!maps::Subclass(r,c.type,transitionOwner))return finish(false);
    if(stage)*stage=24;
    if(!nativetransition::Observe(r,c.module,transitionOwner,result->push,0,transitionDiagnostic))return finish(false);
    if(stage)*stage=3;
    if(!capture::Function(r,c.module,c.type,u"ExecuteUbergraph_OfflinePlayMenuWidget",result->entry)||
       !EntryReference(result->entry,c.type,c.module,result->push,result->map)||!names.Object(result->map,label)||
       label.View()!=u"PickOfflineBotsMapMenuWidget_C"||!maps::Subclass(r,result->map,menu)||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,result->map,result->mapIdentity))return finish(false);
    if(stage)*stage=4;
    if(!capture::Function(r,c.module,result->map,u"ExecuteUbergraph_PickOfflineBotsMapMenuWidget",result->mapGraph))return finish(false);
    // Both authored operands must identify the same native descriptor. Validate
    // the complete known graph shape before using that descriptor as evidence;
    // the native owner's other reflected children are not source dependencies.
    const auto pendingObject=Pointer(result->mapGraph,0x117);
    if(!SettingsReference(result->mapGraph,result->map,c.module,pendingObject,result->settings))return finish(false);
    if(stage)*stage=21;
    if(!pendingmember::PropertyAt(r,c,pendingObject,frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",8,
           selectiondisk::PendingOfflineSettingsWidget,result->pending,propertyDiagnostic))return finish(false);
    if(stage)*stage=22;
    if(result->pending.flags!=0x0/*CFG3.5 address withheld*/40000204ULL)return finish(false);
    if(stage)*stage=25;
    if(!ResetMembers(r,c,frontend,result->mapGraph,result->pending,result->pendingMode,result->pendingMap))return finish(false);
    if(stage)*stage=4;
    if(!names.Object(result->settings,label)||label.View()!=u"OfflineBotsSettingsMenuWidget_C"||!maps::Subclass(r,result->settings,menu)||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,result->settings,result->settingsIdentity))return finish(false);
    if(stage)*stage=5;
    cfgprobe::lifetime::SlotObservation mapSlot,settingsSlot;cfgreflection::Property pending,pendingMode,pendingMap;
    std::uintptr_t pushAgain=0;
    if(!StableScript(r,result->entry)||!StableScript(r,result->mapGraph)||!active::Current(r,c,thread)||
       !nativetransition::Observe(r,c.module,transitionOwner,pushAgain,result->push,transitionDiagnostic)||pushAgain!=result->push||
       !pendingmember::PropertyAt(r,c,result->pending.field.object,frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",8,
           selectiondisk::PendingOfflineSettingsWidget,pending)||!pendingmember::Same(pending,result->pending)||
       !ResetMembers(r,c,frontend,result->mapGraph,pending,pendingMode,pendingMap)||
       !pendingmember::Same(pendingMode,result->pendingMode)||!pendingmember::Same(pendingMap,result->pendingMap)||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,result->map,mapSlot)||!(mapSlot==result->mapIdentity)||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,result->settings,settingsSlot)||!(settingsSlot==result->settingsIdentity))return finish(false);
    out=*result;return finish(true);
}
// Native Push intentionally clears previous Widget pointers but retains their
// CLASS records. Metadata is leased through that exact fresh native record and
// unchanged infrastructure/graphs, not a dangling former widget or raw cache.
template<class Reader> bool Retained(Reader& r,const active::Context& current,const Resources& saved,unsigned* stage=nullptr,ResetDiagnostic* diagnostic=nullptr){
    const auto& source=saved.source;std::uintptr_t type=0,again=0;cfgprobe::lifetime::SlotObservation map,settings;
    std::uintptr_t frontend=0,frontendCtor=0;cfgreflection::Property pending,pendingMode,pendingMap;
    if(stage)*stage=1;
    if(current.module!=source.module||current.thread!=source.thread||current.world!=source.world||current.mode!=source.mode||current.controller!=source.controller||current.stack!=source.stack||current.hud!=source.hud||
       current.gameInstance!=source.gameInstance||!source.array.Sane()||!current.array.Sane()||current.array.count<source.array.count)return false;
    if(stage)*stage=2;if(!active::Current(r,current,current.thread))return false;
    if(stage)*stage=3;if(!r.Read(current.array.data+static_cast<std::uintptr_t>(source.array.count-1)*execdisk::RecordBytes,type)||type!=source.type||saved.entry.function.owner!=type)return false;
    if(stage)*stage=4;if(!StableScript(r,saved.entry)||!StableScript(r,saved.mapGraph))return false;
    if(stage)*stage=5;if(!r.Read(current.module+selectiondisk::SCGame_MenuClass,frontend)||frontend!=saved.pending.field.owner||
       !r.Read(current.module+selectiondisk::SCGame_MenuConstructor,frontendCtor)||!cfgprobe::MatchingClassCache(frontend,frontendCtor))return false;
    if(stage)*stage=6;if(!pendingmember::PropertyAt(r,current,saved.pending.field.object,frontend,u"PendingOfflineSettingsWidget",u"ClassProperty",8,
           selectiondisk::PendingOfflineSettingsWidget,pending)||!pendingmember::SameSchema(pending,saved.pending))return false;
    if(stage)*stage=7;if(!ResetMembers(r,current,frontend,saved.mapGraph,pending,pendingMode,pendingMap,diagnostic))return false;
    // Both direct authored descriptors were just independently read twice.
    // Their unrelated Next links may change between native menu events; no
    // linked-list traversal or writer relies on the old links. All actual
    // field/owner/type/name/schema values must still match exactly.
    if(!pendingmember::SameSchema(pendingMode,saved.pendingMode)){if(diagnostic){diagnostic->stage=10;diagnostic->savedNext=saved.pendingMode.field.next;diagnostic->currentNext=pendingMode.field.next;}return false;}
    if(!pendingmember::SameSchema(pendingMap,saved.pendingMap)){if(diagnostic){diagnostic->stage=11;diagnostic->savedNext=saved.pendingMap.field.next;diagnostic->currentNext=pendingMap.field.next;}return false;}
    if(stage)*stage=8;if(!cfgprobe::lifetime::ReadSlot(r,current.registry,saved.map,map)||!(map==saved.mapIdentity))return false;
    if(stage)*stage=9;if(!cfgprobe::lifetime::ReadSlot(r,current.registry,saved.settings,settings)||!(settings==saved.settingsIdentity))return false;
    if(stage)*stage=10;
    for(const auto i:{0U,1U,3U})if(!source.identities[i].HasAssignedSerial()||!(current.identities[i]==source.identities[i]))return false;
    if(stage)*stage=11;const bool okay=r.Read(current.array.data+static_cast<std::uintptr_t>(source.array.count-1)*execdisk::RecordBytes,again)&&again==type&&active::Current(r,current,current.thread);
    if(okay&&stage)*stage=0;return okay;
}
template<class Reader> bool FirstNativeArrival(Reader& r,const active::Context& current,Resources& staged,unsigned* stage=nullptr){
    if(stage)*stage=12;
    if((current.type!=staged.map&&current.type!=staged.settings)||!current.identities[2].HasAssignedSerial())return false;
    for(auto* slot:{&staged.mapIdentity,&staged.settingsIdentity}){cfgprobe::lifetime::SlotObservation fresh;
        if(!cfgprobe::lifetime::ReadSlot(r,current.registry,slot->object,fresh)||slot->index!=fresh.index||
           (slot->serial&&slot->serial!=fresh.serial))return false;*slot=fresh;}
    return Retained(r,current,staged,stage,&staged.reset);
}
}
