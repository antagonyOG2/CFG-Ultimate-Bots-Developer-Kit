#pragma once
#include "OfflineEntrySettingsObservation.hpp"
namespace cfgmenu::entry {
constexpr std::u16string_view SandboxClick=u"BndEvt__HostSandboxMatchButton_K2Node_ComponentBoundEvent_34_OnClicked__DelegateSignature";
constexpr std::u16string_view JasonClick=u"BndEvt__OfflineBotsButton_K2Node_ComponentBoundEvent_49_OnClicked__DelegateSignature";
struct Binding {
    active::Context context{};std::uintptr_t sandbox=0,jason=0,buttonType=0,sandboxTitle=0,jasonTitle=0,sandboxMap=0;
    cfgprobe::lifetime::SlotObservation sandboxIdentity{},jasonIdentity{};
    cfgreflection::Property sandboxMember{},jasonMember{},titleMember{};
    capture::FunctionEvidence sandboxClick{},jasonClick{};
    const entrysettings::Resources* source=nullptr;
};
inline bool Click(const capture::FunctionEvidence& f,std::uintptr_t owner,std::uintptr_t module,std::u16string_view name,unsigned entry){
    if(f.function.owner!=owner||f.function.label.View()!=name||f.function.kind.View()!=u"Function"||f.flags!=0x08000000||
       f.parameterBytes||f.storageBytes||f.fieldsCount||f.destructorLink||f.script.count!=22||f.nativeExecutor!=module+startdisk::ScriptExecutor||
       f.bytes[0]!=0x90||f.bytes[13]!=0x5A||f.bytes[18]!=0x97||f.bytes[19]!=0xA5||f.bytes[20]!=0x39||f.bytes[21]!=0xE5)return false;
    unsigned actual=0;std::memcpy(&actual,f.bytes.data()+14,4);return actual==entry;
}
template<class Reader> bool Member(Reader& r,const active::Context& c,std::uintptr_t type,std::u16string_view name,int offset,cfgreflection::Property& out){
    return pendingmember::Property(r,c,type,name,u"ObjectProperty",8,offset,out)&&out.flags==0x0/*CFG3.5 address withheld*/C008020CULL;
}
// Revalidate the exact descriptor found during preparation. Re-searching the
// whole class children list for each retained property adds repeated menu work
// but no ownership evidence beyond this same typed owner/name/schema check.
template<class Reader> bool CapturedMember(Reader& r,const active::Context& c,std::uintptr_t type,const cfgreflection::Property& saved,cfgreflection::Property& out){
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgreflection::Field field;
    return reflection.ReadField(saved.field.object,type,field)&&field.type==saved.field.type&&field.name==saved.field.name&&
        field.label.View()==saved.field.label.View()&&field.kind.View()==u"ObjectProperty"&&capture::RawProperty(r,field,out)&&
        out.dimension==1&&out.elementBytes==8&&out.offset==saved.offset&&out.flags==0x0/*CFG3.5 address withheld*/C008020CULL&&pendingmember::Same(out,saved);
}
template<class Reader> bool Current(Reader& r,const Binding& b,std::uint32_t thread,unsigned* failure=nullptr){
    const auto stage=[&](unsigned n){if(failure)*failure=n;};
    stage(1);if(!b.source)return false;
    stage(2);if(!active::Current(r,b.context,thread))return false;
    stage(3);if(!entrysettings::StableScript(r,b.source->entry))return false;
    cfgreflection::Property sandbox,jason,title;cfgprobe::lifetime::SlotObservation a,z;std::uintptr_t x=0,y=0,type=0;
    stage(4);if(!CapturedMember(r,b.context,b.context.type,b.sandboxMember,sandbox))return false;
    stage(5);if(!CapturedMember(r,b.context,b.context.type,b.jasonMember,jason))return false;
    stage(6);if(!CapturedMember(r,b.context,b.buttonType,b.titleMember,title))return false;
    stage(7);if(!r.Read(b.context.widget+0x420,x)||x!=b.sandbox||!r.Read(b.context.widget+0x418,y)||y!=b.jason)return false;
    stage(8);if(!r.Read(x+cfgprobe::ObjectClass,type)||type!=b.buttonType||!r.Read(y+cfgprobe::ObjectClass,type)||type!=b.buttonType)return false;
    stage(9);if(!r.Read(x+0x400,x)||x!=b.sandboxTitle||!r.Read(y+0x400,y)||y!=b.jasonTitle)return false;
    stage(10);if(!cfgprobe::lifetime::ReadSlot(r,b.context.registry,b.sandbox,a)||!(a==b.sandboxIdentity)||!a.HasAssignedSerial())return false;
    stage(11);if(!cfgprobe::lifetime::ReadSlot(r,b.context.registry,b.jason,z)||!(z==b.jasonIdentity)||!z.HasAssignedSerial())return false;
    stage(0);return true;
}

// Native Back may rebuild OfflinePlay and its children. Rebind ONLY the actual
// positive native top through the previously captured exact descriptors. The
// route layer separately checks all three already-published executor slots.
template<class Reader> bool Rebind(Reader& r,const Binding& saved,const active::Context& actual,Binding& out){
    out={};if(!saved.source||actual.type!=saved.context.type||actual.module!=saved.context.module||
       actual.thread!=saved.context.thread||actual.array.count<2||!active::Current(r,actual,actual.thread)||
       !entrysettings::Retained(r,actual,*saved.source))return false;
    Binding b=saved;b.context=actual;cfgreflection::Property property;std::uintptr_t type=0;
    if(!CapturedMember(r,actual,actual.type,saved.sandboxMember,property)||
       !CapturedMember(r,actual,actual.type,saved.jasonMember,property)||
       !CapturedMember(r,actual,saved.buttonType,saved.titleMember,property)||
       !r.Read(actual.widget+saved.sandboxMember.offset,b.sandbox)||!cfgprobe::ObjectPointer(b.sandbox)||
       !r.Read(actual.widget+saved.jasonMember.offset,b.jason)||!cfgprobe::ObjectPointer(b.jason)||b.sandbox==b.jason||
       !r.Read(b.sandbox+cfgprobe::ObjectClass,type)||type!=saved.buttonType||
       !r.Read(b.jason+cfgprobe::ObjectClass,type)||type!=saved.buttonType||
       !r.Read(b.sandbox+saved.titleMember.offset,b.sandboxTitle)||!cfgprobe::ObjectPointer(b.sandboxTitle)||
       !r.Read(b.jason+saved.titleMember.offset,b.jasonTitle)||!cfgprobe::ObjectPointer(b.jasonTitle)||
       !cfgprobe::lifetime::ReadSlot(r,actual.registry,b.sandbox,b.sandboxIdentity)||!b.sandboxIdentity.HasAssignedSerial()||
       !cfgprobe::lifetime::ReadSlot(r,actual.registry,b.jason,b.jasonIdentity)||!b.jasonIdentity.HasAssignedSerial()||
       !Current(r,b,actual.thread))return false;
    out=b;return true;
}
template<class Reader> bool Observe(Reader& r,const entrysettings::Resources& source,Binding& out,unsigned* stage=nullptr){
    out={};Binding b;b.context=source.source;b.source=&source;const auto& c=b.context;cfgnames::Observation<Reader> names(r,c.module);cfgnames::Text name;
    if(stage)*stage=1;
    if(!active::Current(r,c,c.thread)||!entrysettings::StableScript(r,source.entry)||!Member(r,c,c.type,u"SandboxButton",0x420,b.sandboxMember)||
       !Member(r,c,c.type,u"OfflineBotsButton",0x418,b.jasonMember)||!r.Read(c.widget+0x420,b.sandbox)||
       !r.Read(c.widget+0x418,b.jason)||b.sandbox==b.jason||!cfgprobe::ObjectPointer(b.sandbox)||!cfgprobe::ObjectPointer(b.jason)||
       !r.Read(b.sandbox+cfgprobe::ObjectClass,b.buttonType)||!names.Object(b.buttonType,name)||name.View()!=u"WideMenuTextButton_C"||
       !Member(r,c,b.buttonType,u"TitleText",0x400,b.titleMember)||!r.Read(b.sandbox+0x400,b.sandboxTitle)||
       !r.Read(b.jason+0x400,b.jasonTitle)||!cfgprobe::ObjectPointer(b.sandboxTitle)||!cfgprobe::ObjectPointer(b.jasonTitle)||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,b.sandbox,b.sandboxIdentity)||!b.sandboxIdentity.HasAssignedSerial()||
       !cfgprobe::lifetime::ReadSlot(r,c.registry,b.jason,b.jasonIdentity)||!b.jasonIdentity.HasAssignedSerial())return false;
    if(stage)*stage=2;
    if(!capture::Function(r,c.module,c.type,SandboxClick,b.sandboxClick)||!Click(b.sandboxClick,c.type,c.module,SandboxClick,0xDB)||
       !capture::Function(r,c.module,c.type,JasonClick,b.jasonClick)||!Click(b.jasonClick,c.type,c.module,JasonClick,0x3EA))return false;
    const auto graphName=[&](const capture::FunctionEvidence& click){cfgnames::Words a{},z{};std::memcpy(&a,click.bytes.data()+1,8);
        // Virtual-call serialization duplicates the index; number follows at9.
        std::memcpy(&z.index,click.bytes.data()+5,4);std::memcpy(&z.number,click.bytes.data()+9,4);
        return a.index==z.index&&names.Decode(z,name)&&name.View()==u"ExecuteUbergraph_OfflinePlayMenuWidget";};
    if(!graphName(b.sandboxClick)||!graphName(b.jasonClick))return false;
    if(stage)*stage=3;
    const auto& graph=source.entry;
    if(graph.bytes[0xDB]!=0xF5||entrysettings::Pointer(graph,0xDC)!=source.push||graph.bytes[0xE4]!=0x89||graph.bytes[0xED]!=0x97||graph.bytes[0xEE]!=0x87)return false;
    b.sandboxMap=entrysettings::Pointer(graph,0xE5);
    std::uintptr_t native=0,ctor=0;
    if(!names.Object(b.sandboxMap,name)||name.View()!=u"PickSandboxMapMenuWidget_C"||
       !r.Read(c.module+pushdisk::MenuClass,native)||!r.Read(c.module+pushdisk::MenuConstructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor)||
       !maps::Subclass(r,b.sandboxMap,native)||!Current(r,b,c.thread))return false;
    out=b;return true;
}
}
