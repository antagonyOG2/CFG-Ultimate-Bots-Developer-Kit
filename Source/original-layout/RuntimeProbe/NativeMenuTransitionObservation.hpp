#pragma once
#include "NativeMapSelectionObservation.hpp"
#include "MenuTransition.generated.hpp"
namespace cfgmenu::nativetransition {
struct Diagnostic {unsigned failure=0;std::uintptr_t cached=0;};
// The pinned factory at2F8470 lazily fills30449A8. A loaded native function
// reachable through its exact owner's reflected fields need not have called
// that factory. Never call the factory or write its cache to initialize it.
// A nonzero cache MUST agree; the real owner/name/ABI/executor/code remain the
// authority. Retained callers supply their already-proved function, avoiding
// repeated property-list discovery while the cache remains cold.
template<class Reader> bool Observe(Reader& r,std::uintptr_t module,std::uintptr_t owner,
    std::uintptr_t& out,std::uintptr_t expected=0,Diagnostic* diagnostic=nullptr){
    out=0;Diagnostic detail;const auto fail=[&](unsigned reason){detail.failure=reason;if(diagnostic)*diagnostic=detail;return false;};
    std::uintptr_t native=0,ctor=0;
    if(!r.Read(module+transitiondisk::Owner,native)||native!=owner||
       !r.Read(module+transitiondisk::Constructor,ctor)||!cfgprobe::MatchingClassCache(native,ctor))return fail(1);
    if(!r.Read(module+transitiondisk::FunctionCache,detail.cached))return fail(2);
    cfgreflection::Observation<Reader> reflection(r,module);cfgreflection::Field field;
    const auto candidate=expected?expected:detail.cached;
    if(candidate){if(!cfgprobe::ObjectPointer(candidate)||!reflection.ReadField(candidate,owner,field))return fail(3);}
    else if(!reflection.Find(owner,u"PlayTransitionOut",field,false))return fail(4);
    if(field.owner!=owner||field.kind.View()!=u"Function"||field.label.View()!=u"PlayTransitionOut"||
       (detail.cached&&detail.cached!=field.object))return fail(5);
    std::uintptr_t executor=0;std::uint32_t flags=0;std::uint16_t frame=0;std::int32_t storage=0;
    std::array<unsigned char,transitiondisk::Code.size()> code{};
    if(!r.Read(field.object+0xB0,executor)||executor!=module+transitiondisk::Executor||
       !r.ReadBytes(executor,code.data(),code.size())||code!=transitiondisk::Code)return fail(6);
    if(!r.Read(field.object+cfgreflection::disk::FunctionFlags,flags)||flags!=0x04020409||
       !r.Read(field.object+cfgreflection::arraydisk::FunctionParameterBytes,frame)||frame!=8||
       !r.Read(field.object+cfgreflection::disk::StorageBytes,storage)||storage!=8)return fail(7);
    cfgreflection::Field again;std::uintptr_t cache=0,type=0,constructor=0;
    if(!reflection.ReadField(field.object,owner,again)||again.type!=field.type||!(again.name==field.name)||again.next!=field.next||
       !r.Read(module+transitiondisk::FunctionCache,cache)||cache!=detail.cached||
       !r.Read(module+transitiondisk::Owner,type)||type!=owner||
       !r.Read(module+transitiondisk::Constructor,constructor)||constructor!=owner)return fail(8);
    if(diagnostic)*diagnostic=detail;out=field.object;return true;
}
}
