#pragma once
#include "MenuScriptObservation.hpp"
#include "InventoryBindings.generated.hpp"
#include "../Core/NativeInventoryParameters.hpp"
namespace cfginventory {
// Pure metadata comparison: no game reads, calls, pointer adoption or mutation.
// Positive match does NOT retain the owning inventory or authorize a call.
inline bool NativeFrameMatches(const cfgmenu::capture::FunctionEvidence& f,
    std::uintptr_t module,std::uintptr_t nativeOwner,bool add) noexcept {
    const auto name=add?u"AddItem":u"FindItem";
    const auto executor=add?disk::AddItemExecutor:disk::FindItemExecutor;
    const auto flags=add?disk::AddItemFlags:disk::FindItemFlags;
    const unsigned bytes=add?sizeof(AddParameters):sizeof(FindParameters);
    if(!cfgprobe::ObjectPointer(module)||!cfgprobe::ObjectPointer(nativeOwner)||!cfgprobe::ObjectPointer(f.function.object)||
        f.function.owner!=nativeOwner||f.function.label.View()!=name||f.function.kind.View()!=u"Function"||
        f.flags!=flags||f.parameterBytes!=bytes||f.storageBytes!=static_cast<int>(bytes)||
        f.nativeExecutor!=module+executor||f.destructorLink||f.script.count||f.script.capacity||f.script.data||
        f.fieldsCount!=(add?1u:2u))return false;
    unsigned seen=0;
    for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];
        if(!cfgprobe::ObjectPointer(p.field.object)||p.field.owner!=f.function.object||p.dimension!=1||p.elementBytes!=8)return false;
        unsigned bit=0;std::uint64_t propertyFlags=0;
        if(add&&p.field.label.View()==u"Item"&&p.field.kind.View()==u"ObjectProperty"&&p.offset==0){bit=1;propertyFlags=0x0/*CFG3.5 address withheld*/40000280ULL;}
        if(!add&&p.field.label.View()==u"ItemClass"&&p.field.kind.View()==u"ClassProperty"&&p.offset==0){bit=1;propertyFlags=0x0/*CFG3.5 address withheld*/40000280ULL;}
        if(!add&&p.field.label.View()==u"ReturnValue"&&p.field.kind.View()==u"ObjectProperty"&&p.offset==8){bit=2;propertyFlags=0x0/*CFG3.5 address withheld*/40000780ULL;}
        if(!bit||(seen&bit)||p.flags!=propertyFlags)return false;
        seen|=bit;
    }
    return seen==(add?1u:3u);
}
}
