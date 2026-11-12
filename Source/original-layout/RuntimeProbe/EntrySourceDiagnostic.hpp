#pragma once
#include "OfflineEntrySettingsObservation.hpp"
#include <cstdio>
namespace cfgmenu::entrydiagnostic {
inline void Label(std::u16string_view value){for(const auto c:value)std::putchar(c>=32&&c<127?static_cast<int>(c):'?');}
template<std::size_t Fields,std::size_t Bytes>inline void Function(const capture::FunctionEvidenceStorage<Fields,Bytes>& value){
    std::printf("EntrySourceFunction=");Label(value.function.label.View());
    std::printf(" Object=0x%llX Owner=0x%llX Flags=0x%X Frame=%u Storage=%d ScriptBytes=%d Executor=0x%llX Cleanup=0x%llX\n",
        static_cast<unsigned long long>(value.function.object),static_cast<unsigned long long>(value.function.owner),value.flags,
        value.parameterBytes,value.storageBytes,value.script.count,static_cast<unsigned long long>(value.nativeExecutor),
        static_cast<unsigned long long>(value.destructorLink));
    for(unsigned i=0;i<value.fieldsCount;++i){const auto& p=value.fields[i];
        std::printf("EntrySourceField=0x%llX Name=",static_cast<unsigned long long>(p.field.object));Label(p.field.label.View());
        std::printf(" Kind=");Label(p.field.kind.View());
        std::printf(" Offset=%d Bytes=%d Dimension=%d Flags=0x%llX\n",p.offset,p.elementBytes,p.dimension,static_cast<unsigned long long>(p.flags));
    }
    for(unsigned at=0;at<static_cast<unsigned>(value.script.count);at+=32){
        std::printf("EntrySourceScript+0x%04X=",at);
        for(unsigned i=at;i<at+32&&i<static_cast<unsigned>(value.script.count);++i)std::printf("%02X",value.bytes[i]);
        std::putchar('\n');
    }
}
// Failure evidence ONLY. No caller may use these candidate operands to approve
// a native call, source binding, hook or retry. Narrow current entry + named map
// graph snapshots prevent a future script-shape refusal requiring another round
// merely to collect the missing bytes. No whole-registry scan or engine call.
template<class Reader>void Capture(Reader& reader,const active::Context& context){
    std::puts("EntrySourceDiagnostic=READ_ONLY_NO_CALL_AUTHORITY");
    if(!active::Current(reader,context,context.thread))return;
    auto graph=std::make_unique<capture::DiagnosticFunctionEvidence>();
    if(!capture::Function(reader,context.module,context.type,u"ExecuteUbergraph_OfflinePlayMenuWidget",*graph)){
        std::puts("EntrySourceGraph=UNAVAILABLE");return;
    }
    Function(*graph);
    cfgnames::Observation<Reader> names(reader,context.module);
    for(unsigned at=0;at+9<=static_cast<unsigned>(graph->script.count);++at)if(graph->bytes[at]==0x89){
        std::uintptr_t candidate=0,type=0;std::memcpy(&candidate,graph->bytes.data()+at+1,8);
        cfgprobe::lifetime::SlotObservation slot{};cfgnames::Text name,kind;
        if(!cfgprobe::ObjectPointer(candidate)||!cfgprobe::lifetime::ReadSlot(reader,context.registry,candidate,slot)||
           !reader.Read(candidate+cfgprobe::ObjectClass,type)||!names.Object(type,kind)||
           (kind.View()!=u"WidgetBlueprintGeneratedClass"&&kind.View()!=u"BlueprintGeneratedClass")||
           !names.Object(candidate,name)||name.View()!=u"PickOfflineBotsMapMenuWidget_C")continue;
        std::printf("EntrySourceNamedMapOperand=0x%X Object=0x%llX\n",at,static_cast<unsigned long long>(candidate));
        auto map=std::make_unique<capture::DiagnosticFunctionEvidence>();
        if(capture::Function(reader,context.module,candidate,u"ExecuteUbergraph_PickOfflineBotsMapMenuWidget",*map))Function(*map);
        else std::puts("EntrySourceMapGraph=UNAVAILABLE");
        break;
    }
    std::puts("EntrySourceDiagnosticComplete=yes; observations_only_not_source_or_runtime_approval");
}
}
