#pragma once
#include "MenuScriptObservation.hpp"
#include "ActiveMenuObservation.hpp"
#include "NativeMapSelectionObservation.hpp"
#include "CounselorPickerCast.generated.hpp"
namespace cfgmenu::widgetowner {
// Named native UserWidget.GetOwningPlayer, not a private PlayerContext layout
// approximation. Corroborated by Inspect-CfgPickerOwnership.ps1 on the pinned PE.
constexpr std::uintptr_t ClassCache=0x0/*CFG3.5 address withheld*/,ClassConstructor=0x0/*CFG3.5 address withheld*/,FunctionCache=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t Executor=0x0/*CFG3.5 address withheld*/,VirtualGetter=0x0/*CFG3.5 address withheld*/,VirtualSlot=0x250;
constexpr std::array<unsigned char,48> ExecutorCode={0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B,0x42,0x20,0x45,0x33,0xC9,0x48,0x85,0xC0,0x49,0x8B,0xD8,0x41,0x0F,0x95,0xC1,0x4C,0x03,0xC8,0x4C,0x89,0x4A,0x20,0x48,0x8B,0x01,0xFF,0x90,0x50,0x02,0x00,0x00,0x48,0x89,0x03,0x48,0x83,0xC4,0x20,0x5B,0xC3};
constexpr std::array<unsigned char,46> VirtualCode={0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8D,0x99,0xE8,0x01,0x00,0x00,0x48,0x8B,0xCB,0xE8,0x5B,0x23,0x4E,0x00,0x84,0xC0,0x74,0x0D,0x48,0x8B,0xCB,0x48,0x83,0xC4,0x20,0x5B,0xE9,0x7A,0xC0,0x4D,0x00,0x33,0xC0,0x48,0x83,0xC4,0x20,0x5B,0xC3};
struct Getter {std::uintptr_t owner=0,controllerClass=0;capture::FunctionEvidenceStorage<1,1> function{};};
template<class Reader> bool Observe(Reader& r,const active::Context& context,std::uintptr_t pickerClass,Getter& out){
    out={};std::uintptr_t owner=0,constructor=0,function=0;cfgreflection::Field field;Getter value;
    cfgreflection::Observation<Reader> reflection(r,context.module);cfgnames::Observation<Reader> names(r,context.module);cfgnames::Text label;
    if(!r.Read(context.module+ClassCache,owner)||!r.Read(context.module+ClassConstructor,constructor)||!cfgprobe::MatchingClassCache(owner,constructor)||
       !names.Object(owner,label)||label.View()!=u"UserWidget"||!maps::Subclass(r,pickerClass,owner)||
       !r.Read(context.module+FunctionCache,function)||!reflection.ReadField(function,owner,field)||
       !capture::KnownFunction(r,context.module,field,u"GetOwningPlayer",value.function))return false;
    const auto& f=value.function;
    if(f.flags!=0x54020409||f.parameterBytes!=8||f.storageBytes!=8||f.script.count||f.destructorLink||f.fieldsCount!=1||
       f.nativeExecutor!=context.module+Executor)return false;
    const auto& p=f.fields[0];
    std::uintptr_t propertyTable=0;
    if(p.field.label.View()!=u"ReturnValue"||p.field.kind.View()!=u"ObjectProperty"||p.offset||p.dimension!=1||p.elementBytes!=8||p.flags!=0x0/*CFG3.5 address withheld*/40000780ULL||
       !r.Read(p.field.object,propertyTable)||propertyTable!=context.module+counselorpickerdisk::ObjectPropertyVtable||
       !r.Read(p.field.object+0x70,value.controllerClass)||!names.Object(value.controllerClass,label)||label.View()!=u"PlayerController")return false;
    std::uintptr_t controllerType=0;std::array<unsigned char,ExecutorCode.size()> exec{};std::array<unsigned char,VirtualCode.size()> code{};
    if(!r.Read(context.controller+cfgprobe::ObjectClass,controllerType)||!maps::Subclass(r,controllerType,value.controllerClass)||
       !r.ReadBytes(context.module+Executor,exec.data(),exec.size())||exec!=ExecutorCode||
       !r.ReadBytes(context.module+VirtualGetter,code.data(),code.size())||code!=VirtualCode)return false;
    value.owner=owner;out=value;return true;
}
template<class Reader> bool Current(Reader& r,const active::Context& context,std::uintptr_t pickerClass,const Getter& saved){
    auto fresh=std::make_unique<Getter>();if(!Observe(r,context,pickerClass,*fresh))return false;
    const auto& a=fresh->function;const auto& b=saved.function;
    return fresh->owner==saved.owner&&fresh->controllerClass==saved.controllerClass&&a.function.object==b.function.object&&a.function.name==b.function.name&&
        a.slot==b.slot&&a.fields[0].field.object==b.fields[0].field.object&&a.fields[0].field.name==b.fields[0].field.name;
}
template<class Reader> bool Receiver(Reader& r,const active::Context& context,std::uintptr_t widget,std::uintptr_t pickerClass,const Getter& saved){
    std::uintptr_t table=0,getter=0;return Current(r,context,pickerClass,saved)&&r.Read(widget,table)&&cfgprobe::ObjectPointer(table)&&
        r.Read(table+VirtualSlot,getter)&&getter==context.module+VirtualGetter;
}
// One pointer-sized native output packet. The getter resolves its own weak
// LocalPlayer/World mapping; no raw context edits or synthetic FFrame are used.
struct Packet {std::uintptr_t controller=0;};static_assert(sizeof(Packet)==8);
template<class Reader> bool Result(Reader& r,const active::Context& current,const Packet& packet,cfgprobe::lifetime::SlotObservation& out){
    out={};cfgprobe::lifetime::SlotObservation actual;
    if(packet.controller!=current.controller||!cfgprobe::lifetime::ReadSlot(r,current.registry,packet.controller,actual)||!actual.HasAssignedSerial()||!(actual==current.identities[1]))return false;
    out=actual;return true;
}
}
