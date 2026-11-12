#pragma once
#include "SetupWidgetObservation.hpp"
#include "ScriptLayout.generated.hpp"
#include <memory>
namespace cfgmenu::capture {
struct ScriptHeader {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool operator==(const ScriptHeader& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
template<std::size_t StorageFields,std::size_t ScriptBytes=scriptdisk::MaxBytes> struct FunctionEvidenceStorage {
    cfgreflection::Field function{};cfgprobe::lifetime::SlotObservation slot{};
    ScriptHeader script{};std::uint32_t flags=0;std::uint16_t parameterBytes=0;
    std::int32_t storageBytes=0;std::uintptr_t nativeExecutor=0,destructorLink=0;
    std::array<cfgreflection::Property,StorageFields> fields{};unsigned fieldsCount=0;
    std::array<unsigned char,ScriptBytes> bytes{};
};
using FunctionEvidence=FunctionEvidenceStorage<32>; // Runtime ABI observations stay small.
using DiagnosticFunctionEvidence=FunctionEvidenceStorage<128>; // One-shot Blueprint locals, not call approval.
// One-shot picker evidence only; donor's explicit64KiB Blueprint graph ceiling.
// Never used by the runtime ABI/options/Start observation paths.
using PickerFunctionEvidence=FunctionEvidenceStorage<256,65536>;
struct FunctionDiagnostic {const char* stage="identity";unsigned fields=0;std::int32_t scriptBytes=-1;};
// Diagnostic storage metadata, NOT the UI argument whitelist. Blueprint locals
// may be Struct/Name/Float/Byte/etc and must not hide otherwise bounded script.
template<class Reader> bool RawProperty(Reader& r,const cfgreflection::Field& field,cfgreflection::Property& out){
    cfgreflection::Property a;a.field=field;const auto kind=field.kind.View();
    if(kind.size()<8||kind.substr(kind.size()-8)!=u"Property")return false;
    if(!r.Read(field.object+cfgreflection::disk::PropertyDimensions,a.dimension)||a.dimension<1||a.dimension>32||
       !r.Read(field.object+cfgreflection::disk::PropertyElementSize,a.elementBytes)||a.elementBytes<1||a.elementBytes>65536||
       !r.Read(field.object+cfgreflection::disk::PropertyOffset,a.offset)||a.offset<0||a.offset>1048576||
       !r.Read(field.object+cfgreflection::disk::PropertyFlags,a.flags))return false;
    out=a;return true;
}
template<class Reader,std::size_t Bytes> bool Script(Reader& r,std::uintptr_t function,ScriptHeader& header,std::array<unsigned char,Bytes>& bytes){
    static_assert(Bytes>0&&Bytes<=65536,"Diagnostic graph capacity must remain bounded");
    header={};bytes={};ScriptHeader a,b;
    if(!cfgprobe::ObjectPointer(function)||!r.Read(function+scriptdisk::Script,a)||a.count<0||a.capacity<a.count||a.capacity>1048576||a.count>static_cast<int>(Bytes)||
       (a.count&&!cfgprobe::lifetime::Extent(a.data,static_cast<std::uintptr_t>(a.count))))return false;
    std::array<unsigned char,Bytes> first{},second{};
    if(a.count&&(!r.ReadBytes(a.data,first.data(),static_cast<unsigned>(a.count))||!r.ReadBytes(a.data,second.data(),static_cast<unsigned>(a.count))||first!=second))return false;
    if(!r.Read(function+scriptdisk::Script,b)||!(a==b))return false;
    header=a;bytes=first;return true;
}
template<class Reader,std::size_t StorageFields,std::size_t Bytes> bool KnownFunctionSnapshot(Reader& r,std::uintptr_t module,const cfgreflection::Field& known,
    std::u16string_view name,FunctionEvidenceStorage<StorageFields,Bytes>& output,FunctionEvidenceStorage<StorageFields,Bytes>& a,FunctionDiagnostic* diagnostic){
    output={};FunctionDiagnostic detail;const auto fail=[&](){detail.fields=a.fieldsCount;if(diagnostic)*diagnostic=detail;return false;};
    cfgreflection::Observation<Reader> reflection(r,module);
    a.function=known;
    cfgprobe::lifetime::ArrayHeader registry;
    if(a.function.kind.View()!=u"Function"||a.function.label.View()!=name||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgprobe::lifetime::ReadSlot(r,registry,a.function.object,a.slot)||
       !r.Read(a.function.object+cfgreflection::disk::FunctionFlags,a.flags)||
       !r.Read(a.function.object+cfgreflection::arraydisk::FunctionParameterBytes,a.parameterBytes)||a.parameterBytes>512||
       !r.Read(a.function.object+cfgreflection::disk::StorageBytes,a.storageBytes)||a.storageBytes<a.parameterBytes||a.storageBytes>65536||
       !r.Read(a.function.object+0xB0,a.nativeExecutor)||!r.Read(a.function.object+0x68,a.destructorLink))return fail();
    detail.stage="locals_head";std::uintptr_t head=0;if(!r.Read(a.function.object+cfgreflection::disk::Children,head))return fail();
    for(auto child=head;child;){
        detail.stage="local_field_limit";if(a.fieldsCount==a.fields.size())return fail();
        detail.stage="local_field_cycle";for(unsigned i=0;i<a.fieldsCount;++i)if(a.fields[i].field.object==child)return fail();
        cfgreflection::Field field;auto& property=a.fields[a.fieldsCount++];
        detail.stage="local_field_metadata";if(!reflection.ReadField(child,a.function.object,field)||!RawProperty(r,field,property))return fail();
        child=field.next;
    }
    if(diagnostic){detail.stage="script_header";ScriptHeader script;
        if(!r.Read(a.function.object+scriptdisk::Script,script))return fail();detail.scriptBytes=script.count;}
    detail.stage="script_bytes_or_stability";if(!Script(r,a.function.object,a.script,a.bytes))return fail();
    detail.stage="local_field_revalidation";
    for(unsigned i=0;i<a.fieldsCount;++i){const auto& old=a.fields[i];cfgreflection::Field field;cfgreflection::Property property;
        if(!reflection.ReadField(old.field.object,a.function.object,field)||field.type!=old.field.type||!(field.name==old.field.name)||field.next!=old.field.next||
           !RawProperty(r,field,property)||property.dimension!=old.dimension||property.elementBytes!=old.elementBytes||property.flags!=old.flags||property.offset!=old.offset)return fail();}
    cfgreflection::Field f;cfgprobe::lifetime::ArrayHeader end;cfgprobe::lifetime::SlotObservation slot;
    std::uintptr_t finalHead=0,executor=0,cleanup=0;std::uint32_t flags=0;std::uint16_t size=0;std::int32_t storage=0;
    detail.stage="function_revalidation";
    if(!reflection.ReadField(a.function.object,a.function.owner,f)||f.type!=a.function.type||!(f.name==a.function.name)||
       !r.Read(a.function.object+cfgreflection::disk::Children,finalHead)||finalHead!=head||
       !r.Read(a.function.object+cfgreflection::disk::FunctionFlags,flags)||flags!=a.flags||
       !r.Read(a.function.object+cfgreflection::arraydisk::FunctionParameterBytes,size)||size!=a.parameterBytes||
       !r.Read(a.function.object+cfgreflection::disk::StorageBytes,storage)||storage!=a.storageBytes||
       !r.Read(a.function.object+0xB0,executor)||executor!=a.nativeExecutor||
       !r.Read(a.function.object+0x68,cleanup)||cleanup!=a.destructorLink||
       !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,end)||!(end==registry)||
       !cfgprobe::lifetime::ReadSlot(r,end,a.function.object,slot)||!(slot==a.slot))return fail();
    // Metadata serial0 remains explicitly unassigned; not a lifetime guarantee.
    output=a;detail.stage="none";detail.fields=a.fieldsCount;if(diagnostic)*diagnostic=detail;return true;
}
template<class Reader,std::size_t StorageFields,std::size_t Bytes> bool KnownFunction(Reader& r,std::uintptr_t module,const cfgreflection::Field& known,
    std::u16string_view name,FunctionEvidenceStorage<StorageFields,Bytes>& output,FunctionDiagnostic* diagnostic=nullptr){
    if constexpr(StorageFields>128||Bytes>scriptdisk::MaxBytes){
        auto snapshot=std::make_unique<FunctionEvidenceStorage<StorageFields,Bytes>>();
        return KnownFunctionSnapshot(r,module,known,name,output,*snapshot,diagnostic);
    }else{
        FunctionEvidenceStorage<StorageFields,Bytes> snapshot;
        return KnownFunctionSnapshot(r,module,known,name,output,snapshot,diagnostic);
    }
}
template<class Reader,std::size_t StorageFields,std::size_t Bytes> bool Function(Reader& r,std::uintptr_t module,std::uintptr_t type,std::u16string_view name,FunctionEvidenceStorage<StorageFields,Bytes>& output){
    output={};cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field found;
    return reflection.Find(type,name,found)&&KnownFunction(r,module,found,name,output);
}
}
