#pragma once
#include <cstdint>
#include "J24FeatureFlags.hpp"
namespace cfgj24 {
enum class ResourceCheck : unsigned {
    None,PawnSlot,StateSlot,PawnController,ControllerPawn,ClassDefault,DefaultAI,Helper,
    AICache,AIConstructor,ClassCache,ClassConstructor,ClassSlot,FunctionCache,FunctionSlot,FunctionOuter,Dispatch,
    KismetCache=20,KismetConstructor,KismetClassSlot,KismetDefault,KismetDefaultSlot,KismetType,
    ConversionCache,ConversionSlot,SelectedEntry,SelectedTemplate,SelectedClassSlot,SelectedDefaultSlot,
    FunctionCacheRead=40,Factory,Refresh,FunctionPointer,ResolvedCache,ResolvedSlot,ResolvedOuter,Retain,FinalContext=49,
    FactoryIndex=50,CallCap,OwnerThread,Context,FactoryCode,RootScope,NativeException
};
struct Refusal {
    unsigned phase=0,check=0,index=0,slotMask=0;
    std::int32_t capturedSerial=0,freshSerial=0;
    void Phase(unsigned value){if(CFG_J24_SPAWN_RESET&&!check)phase=value;}
    bool Fail(ResourceCheck value,unsigned item=0){if(CFG_J24_SPAWN_RESET&&!check){check=static_cast<unsigned>(value);index=item;}return false;}
    template<class Slot>bool FailSlot(ResourceCheck value,unsigned item,const Slot& before,const Slot& after,bool readable){
        if(CFG_J24_SPAWN_RESET&&!check){
            check=static_cast<unsigned>(value);index=item;capturedSerial=before.serial;freshSerial=after.serial;
            slotMask=(readable?0u:1u)|(before.object==after.object?0u:2u)|(before.index==after.index?0u:4u)|(before.serial==after.serial?0u:8u);
        }
        return false;
    }
};
}
