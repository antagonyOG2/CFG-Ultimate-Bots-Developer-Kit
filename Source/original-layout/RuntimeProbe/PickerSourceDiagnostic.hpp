#pragma once
#include "EntrySourceDiagnostic.hpp"
#include "FrontendHudObservation.hpp"
#include "PickerPrecreationObservation.hpp"
namespace cfgmenu::pickerdiagnostic {
// Failure snapshots only. No global scan, no loading, no calls, no fallback
// binding, no promotion of scanned bytecode operands to runtime authority.
template<class Reader>bool NamedClass(Reader& r,const active::Context& c,std::uintptr_t object,std::u16string_view expected){
    cfgprobe::lifetime::SlotObservation slot;std::uintptr_t type=0;cfgnames::Text label,kind;
    cfgnames::Observation<Reader> names(r,c.module);
    return cfgprobe::ObjectPointer(object)&&cfgprobe::lifetime::ReadSlot(r,c.registry,object,slot)&&
        names.Object(object,label)&&label.View()==expected&&r.Read(object+cfgprobe::ObjectClass,type)&&
        names.Object(type,kind)&&kind.View()==u"WidgetBlueprintGeneratedClass";
}
template<class Reader>void CaptureClass(Reader& r,const active::Context& c,std::uintptr_t object,
    std::u16string_view expected,std::u16string_view graphName){
    if(!active::Current(r,c,c.thread)||!NamedClass(r,c,object,expected))return;
    constexpr std::array<std::u16string_view,7> handlers={u"",u"Construct",u"OnClick_Accept",u"OnPlayTransitionOut",u"OnClick_Back",u"OfflineBots_AcceptJason",u"GetPreviousMenu"};
    for(unsigned i=0;i<handlers.size();++i){auto function=std::make_unique<capture::PickerFunctionEvidence>();
        capture::FunctionDiagnostic detail;const auto name=i?handlers[i]:graphName;
        cfgreflection::Observation<Reader> reflection(r,c.module);cfgreflection::Field field;
        if(reflection.Find(object,name,field,false)&&field.owner==object&&capture::KnownFunction(r,c.module,field,name,*function,&detail))entrydiagnostic::Function(*function);
        else{std::printf("PickerDiagnosticFunctionUnavailable=");entrydiagnostic::Label(name);
            std::printf(" Stage=%s Fields=%u ScriptBytes=%d\n",detail.stage,detail.fields,detail.scriptBytes);}
    }
}
template<class Reader>std::uintptr_t NamedOperand(Reader& r,const active::Context& c,const capture::GraphSnapshot& g,std::u16string_view name){
    for(unsigned at=0;at+9<=static_cast<unsigned>(g.script.count);++at)if(g.bytes[at]==pickersourcedisk::ObjectConst){
        const auto candidate=pickersource::Pointer(g,at+1);
        if(NamedClass(r,c,candidate,name)){
            std::printf("PickerDiagnosticNamedOperand=0x%X Object=0x%llX Name=",at,static_cast<unsigned long long>(candidate));
            entrydiagnostic::Label(name);std::putchar('\n');return candidate;}
    }
    return 0;
}
template<class Reader>bool Graph(Reader& r,const active::Context& c,std::uintptr_t object,std::u16string_view name,capture::GraphSnapshot& graph){
    cfgreflection::Observation<Reader> reflection(r,c.module);cfgreflection::Field field;capture::FunctionDiagnostic diagnostic;
    if(!reflection.Find(object,name,field,false)||field.owner!=object||!capture::Graph(r,c.module,field,graph,diagnostic))return false;
    entrydiagnostic::Function(graph);return true;
}
template<class Reader>void CaptureSource(Reader& r,const active::Context& c){
    std::puts("PickerSourceDiagnostic=READ_ONLY_NO_CALL_AUTHORITY");
    parentcapture::Record root;
    if(!active::Current(r,c,c.thread)||!parentcapture::Observe(r,c,0,root)||!NamedClass(r,c,root.menuClass,u"MainMenuWidget_C"))return;
    auto main=std::make_unique<capture::GraphSnapshot>();
    if(!Graph(r,c,root.menuClass,u"ExecuteUbergraph_MainMenuWidget",*main))return;
    const auto customize=NamedOperand(r,c,*main,u"CustomizeMenuWidget_C");if(!customize)return;
    auto custom=std::make_unique<capture::GraphSnapshot>();
    if(!Graph(r,c,customize,u"ExecuteUbergraph_CustomizeMenuWidget",*custom))return;
    NamedOperand(r,c,*custom,u"Counselor_Menu_C");NamedOperand(r,c,*custom,u"Jason_Select_Widget_C");
    std::puts("PickerSourceDiagnosticComplete=yes; observations_only_not_runtime_approval");
}
}
