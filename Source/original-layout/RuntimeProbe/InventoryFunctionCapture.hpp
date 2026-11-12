#pragma once
#include "MenuScriptObservation.hpp"
#include "FeatureSnapshot.hpp"

namespace cfginventory::capture {
inline void Name(const cfgnames::Text& text){for(auto c:text.View())std::putchar(c>=32&&c<127?static_cast<char>(c):'?');}
// One-shot metadata only, on inventories already reached through the verified
// local counselor. No registry scan, creation, asset load or callable binding.
template<class Reader> bool Run(Reader& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t pawn,const cfgfeature::Report& feature){
    cfgnames::Observation<Reader> names(r,module);
    bool stable=true;unsigned samples=0;
    for(unsigned inventoryIndex=0;inventoryIndex<feature.inventories.size();++inventoryIndex){
        const auto inventory=feature.inventories[inventoryIndex];if(!inventory)continue;
        if(inventoryIndex&&inventory==feature.inventories[0])continue;
        std::uintptr_t linked=0,type=0;cfgprobe::lifetime::SlotObservation slot{};
        cfgprobe::Ancestry ancestry{};cfgnames::Text typeName{};
        const auto offset=inventoryIndex?cfgfeature::disk::SmallInventory:cfgfeature::disk::SpecialInventory;
        if(!r.Read(pawn+offset,linked)||linked!=inventory||!r.Read(inventory+cfgprobe::ObjectClass,type)||
            !cfgprobe::CaptureAncestry(r,inventory,ancestry)||!ancestry.Contains(feature.classes[cfgfeature::disk::Inventory])||
            !cfgprobe::lifetime::ReadSlot(r,registry,inventory,slot)||!names.Object(type,typeName)){
            std::printf("InventoryFunctionCapture[%u]=REFUSED_IDENTITY\n",inventoryIndex);stable=false;continue;
        }
        std::printf("InventoryFunctionOwner[%u]=0x%llX Class=0x%llX Serial=%d ClassName=",inventoryIndex,
            static_cast<unsigned long long>(inventory),static_cast<unsigned long long>(type),slot.serial);Name(typeName);std::puts("");
        ++samples;
        // Resource names come from held items, never a global name/object scan
        // or an arbitrary first-loaded class chosen for a future spawn.
        for(unsigned i=0;i<feature.recordCount&&i<feature.records.size();++i){
            const auto& item=feature.records[i];if(item.owner!=inventory||item.related!=inventory)continue;
            std::uintptr_t itemType=0,outer=0;cfgprobe::Ancestry itemAncestry{};
            cfgprobe::lifetime::SlotObservation itemSlot{},endSlot{};cfgnames::Text itemName{},className{};
            if(!r.Read(item.slot.object+cfgfeature::disk::OuterInventory,outer)||outer!=inventory||
                !r.Read(item.slot.object+cfgprobe::ObjectClass,itemType)||!cfgprobe::CaptureAncestry(r,item.slot.object,itemAncestry)||
                !itemAncestry.Contains(feature.classes[cfgfeature::disk::Item])||
                !cfgprobe::lifetime::ReadSlot(r,registry,item.slot.object,itemSlot)||!(itemSlot==item.slot)||
                !names.Object(item.slot.object,itemName)||!names.Object(itemType,className)||
                !cfgprobe::StableAncestry(r,item.slot.object,itemAncestry)||
                !cfgprobe::lifetime::ReadSlot(r,registry,item.slot.object,endSlot)||!(endSlot==itemSlot)){
                std::puts(" HeldItemResource=REFUSED_CHANGED_OR_UNREADABLE");stable=false;continue;
            }
            std::printf(" HeldItemResource=0x%llX Class=0x%llX Serial=%d Name=",static_cast<unsigned long long>(item.slot.object),static_cast<unsigned long long>(itemType),itemSlot.serial);
            Name(itemName);std::printf(" ClassName=");Name(className);std::puts(" CreationApproved=no");
        }
        for(const auto wanted:{u"FindItem",u"SpawnItem",u"AddItem"}){
            cfgmenu::capture::FunctionEvidence f{};
            std::printf(" InventoryFunction=");for(auto c:std::u16string_view(wanted))std::putchar(static_cast<char>(c));
            if(!cfgmenu::capture::Function(r,module,type,wanted,f)){
                std::puts(" Metadata=not_captured_absent_changed_or_budget NativeCallApproved=no");continue;
            }
            std::printf(" Metadata=captured Object=0x%llX Owner=0x%llX Serial=%d Flags=%08X ParameterBytes=%u StorageBytes=%d ScriptBytes=%d NativeExecutor=0x%llX DestructorLink=0x%llX NativeCallApproved=no\n",
                static_cast<unsigned long long>(f.function.object),static_cast<unsigned long long>(f.function.owner),f.slot.serial,f.flags,f.parameterBytes,f.storageBytes,
                f.script.count,static_cast<unsigned long long>(f.nativeExecutor),static_cast<unsigned long long>(f.destructorLink));
            for(unsigned i=0;i<f.fieldsCount;++i){const auto& p=f.fields[i];std::printf("  InventoryParameter=0x%llX Name=",static_cast<unsigned long long>(p.field.object));Name(p.field.label);
                std::printf(" Kind=");Name(p.field.kind);std::printf(" Offset=%d ElementBytes=%d Dimension=%d Flags=%016llX\n",p.offset,p.elementBytes,p.dimension,static_cast<unsigned long long>(p.flags));}
            std::puts("InventoryScriptHexBegin");for(int i=0;i<f.script.count;++i){std::printf("%02X",f.bytes[static_cast<unsigned>(i)]);if(i%32==31)std::putchar('\n');}std::puts("\nInventoryScriptHexEnd");
        }
        cfgprobe::lifetime::SlotObservation end{};std::uintptr_t finalType=0;
        if(!r.Read(pawn+offset,linked)||linked!=inventory||!r.Read(inventory+cfgprobe::ObjectClass,finalType)||finalType!=type||
            !cfgprobe::StableAncestry(r,inventory,ancestry)||!cfgprobe::lifetime::ReadSlot(r,registry,inventory,end)||!(end==slot))stable=false;
    }
    std::printf("InventoryFunctionSamples=%u InventoryFunctionSamplesStable=%s SnapshotAtomic=no NativeCallsApproved=no\n",samples,stable&&samples?"yes":"no");
    return stable&&samples;
}
}
