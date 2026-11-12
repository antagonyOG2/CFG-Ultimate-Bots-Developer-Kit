#pragma once
#include "MenuScriptObservation.hpp"
#include "SettingsStart.generated.hpp"
namespace cfgmenu::capture {
// Read-only Blueprint graph evidence, NOT a complete local-frame schema or
// permission to invoke the graph. Hundreds of unrelated preview/perk locals
// must not block a bounded two-sample routing-code capture.
using GraphSnapshot=FunctionEvidenceStorage<1,65536>;
template<class Reader> bool Graph(Reader& r,std::uintptr_t module,const cfgreflection::Field& known,
    GraphSnapshot& output,FunctionDiagnostic& diagnostic){
    output={};diagnostic={};auto value=std::make_unique<GraphSnapshot>();auto& a=*value;
    const auto fail=[&](){diagnostic.fields=a.fieldsCount;return false;};
    const auto prefix=std::u16string_view(u"ExecuteUbergraph_");
    if(known.kind.View()!=u"Function"||known.label.View().substr(0,prefix.size())!=prefix)return fail();
    a.function=known;cfgreflection::Observation<Reader> reflection(r,module);cfgprobe::lifetime::ArrayHeader registry;
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgprobe::lifetime::ReadSlot(r,registry,known.object,a.slot)||
       !r.Read(known.object+cfgreflection::disk::FunctionFlags,a.flags)||
       !r.Read(known.object+cfgreflection::arraydisk::FunctionParameterBytes,a.parameterBytes)||a.parameterBytes!=4||
       !r.Read(known.object+cfgreflection::disk::StorageBytes,a.storageBytes)||a.storageBytes<4||a.storageBytes>65536||
       !r.Read(known.object+0xB0,a.nativeExecutor)||a.nativeExecutor!=module+startdisk::ScriptExecutor||
       !r.Read(known.object+0x68,a.destructorLink))return fail();
    diagnostic.stage="entry_parameter";std::uintptr_t head=0;
    if(!r.Read(known.object+cfgreflection::disk::Children,head)||!cfgprobe::ObjectPointer(head)||
       !reflection.ReadField(head,known.object,a.fields[0].field)||
       a.fields[0].field.label.View()!=u"EntryPoint"||a.fields[0].field.kind.View()!=u"IntProperty")return fail();
    auto& entry=a.fields[0];const auto first=entry.field;
    if(!RawProperty(r,first,entry)||entry.dimension!=1||entry.offset!=0||entry.elementBytes!=4||!(entry.flags&0x80))return fail();
    a.fieldsCount=1;diagnostic.stage="script_header";ScriptHeader declared;
    if(!r.Read(known.object+scriptdisk::Script,declared))return fail();diagnostic.scriptBytes=declared.count;
    diagnostic.stage="script_bytes_or_stability";if(!Script(r,known.object,a.script,a.bytes))return fail();
    diagnostic.stage="graph_revalidation";cfgreflection::Field field,parameter;cfgreflection::Property property;
    std::uintptr_t again=0,executor=0,cleanup=0;std::uint32_t flags=0;std::uint16_t parameters=0;std::int32_t storage=0;
    cfgprobe::lifetime::ArrayHeader end;cfgprobe::lifetime::SlotObservation slot;
    if(!reflection.ReadField(known.object,known.owner,field)||field.type!=known.type||field.next!=known.next||!(field.name==known.name)||
       !reflection.ReadField(head,known.object,parameter)||parameter.type!=first.type||parameter.next!=first.next||!(parameter.name==first.name)||
       !RawProperty(r,parameter,property)||property.dimension!=entry.dimension||property.elementBytes!=entry.elementBytes||property.offset!=entry.offset||property.flags!=entry.flags||
       !r.Read(known.object+cfgreflection::disk::Children,again)||again!=head||
       !r.Read(known.object+cfgreflection::disk::FunctionFlags,flags)||flags!=a.flags||
       !r.Read(known.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=a.parameterBytes||
       !r.Read(known.object+cfgreflection::disk::StorageBytes,storage)||storage!=a.storageBytes||
       !r.Read(known.object+0xB0,executor)||executor!=a.nativeExecutor||!r.Read(known.object+0x68,cleanup)||cleanup!=a.destructorLink||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,end)||!(end==registry)||
       !cfgprobe::lifetime::ReadSlot(r,end,known.object,slot)||!(slot==a.slot))return fail();
    output=a;diagnostic.stage="none";diagnostic.fields=1;return true;
}
}
