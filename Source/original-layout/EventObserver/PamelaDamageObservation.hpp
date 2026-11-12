#pragma once
#include "KillNativeResource.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
namespace cfgkill::pamela {
template<class R>bool Event(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,std::uintptr_t jason,std::uintptr_t function){
        // No packet cast: the three native event layouts differ. Prove exact
        // name, native UFunction type, event flags, owner ancestry and the
        // complete override chain to Actor. No getter/factory/native call.
        std::uintptr_t base=0,actor=0,nativeType=0,type=0;cfgprobe::lifetime::SlotObservation nativeSlot{},typeSlot{};
        if(!function||!r.Read(module+cfgkill::disk::ReceiveAnyDamage.cache,base)||
           !cfgspawn::ObjectSlot(r,objects,base,nativeSlot)||
           !r.Read(base+cfgprobe::ObjectClass,nativeType)||
           !cfgspawn::ObjectSlot(r,objects,nativeType,typeSlot)||
           !r.Read(module+cfgkill::disk::ReceiveAnyDamage.ownerReflection,actor)||!actor||
           !cfgobserver::Same(r,module+cfgkill::disk::ReceiveAnyDamage.ownerConstructor,actor))return false;
        std::uintptr_t baseOwner=0;std::uint32_t baseFlags=0;std::uint16_t baseBytes=0;
        if(!r.Read(base+0x20,baseOwner)||baseOwner!=actor||
           !r.Read(base+cfgreflection::disk::FunctionFlags,baseFlags)||baseFlags!=cfgkill::disk::ReceiveAnyDamageFlags||
           !r.Read(base+cfgreflection::arraydisk::FunctionParameterBytes,baseBytes)||baseBytes!=32)return false;
        cfgnames::Observation<R> names(r,module);cfgnames::Text name;
        if(!names.Object(function,name)||(name.View()!=u"ReceiveAnyDamage"&&name.View()!=u"ReceivePointDamage"&&name.View()!=u"ReceiveRadialDamage"))return false;
        const auto eventName=name;
        std::array<std::uintptr_t,32> seen{};std::uint16_t parameterBytes=0;
        for(unsigned i=0;i<seen.size();++i){
            for(unsigned j=0;j<i;++j)if(seen[j]==function)return false;
            seen[i]=function;cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t owner=0,parent=0;
            std::uint32_t flags=0,storage=0,objectFlags=0;std::uint16_t bytes=0;
            if(!cfgspawn::ObjectSlot(r,objects,function,slot)||
               !r.Read(objects.data+static_cast<std::uintptr_t>(slot.index)*cfgprobe::lifetime::ItemBytes+8,objectFlags)||(objectFlags&0x20000000u)||
               !r.Read(function+cfgprobe::ObjectClass,type)||type!=nativeType||
               !r.Read(function+0x20,owner)||!cfgspawn::ObjectIs(r,jason,owner)||
               !r.Read(function+cfgprobe::ClassSuper,parent)||
               !r.Read(function+cfgreflection::disk::FunctionFlags,flags)||(flags&0x08000800u)!=0x08000800u||(flags&0x40u)||
               !r.Read(function+cfgreflection::disk::StorageBytes,storage)||
               !r.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||bytes<4||bytes>512||storage<bytes||storage>65536||
               (i&&bytes!=parameterBytes)||!names.Object(function,name)||name.View()!=eventName.View()||
               !cfgjason::SameSlot(r,objects,slot)||!cfgobserver::Same(r,function+0x20,owner)||
               !cfgobserver::Same(r,function+cfgprobe::ClassSuper,parent)||
               !cfgobserver::Same(r,function+cfgprobe::ObjectClass,type))return false;
            parameterBytes=bytes;
            if(owner==actor&&!parent)return cfgjason::SameSlot(r,objects,nativeSlot)&&
                cfgjason::SameSlot(r,objects,typeSlot);
            if(!parent||owner==actor)return false;function=parent;
        }
        return false;
    }
}
