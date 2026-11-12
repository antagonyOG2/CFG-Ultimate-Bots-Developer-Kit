#pragma once
#include "NativeClassPathChoice.hpp"
#include "AiResourceSnapshot.hpp"
#include "../Core/SelectedJasonClass.hpp"
namespace cfgselectedjason {
// Read-only identity evidence, never a retained root or native-call capability.
// Must be repeated against the current registry at arrival and native entry.
template<class Reader>bool Observe(Reader& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t type,std::uintptr_t killerBase,Choice& out){
    out={};Choice c;std::uintptr_t defaults=0,owner=0,parent=1,again=0,meta=0,nativeClass=0;
    std::uint32_t flags=0,againFlags=0;
    if(!cfgprobe::ObjectPointer(type)||type==killerBase||
        !r.Read(module+cfgcompat::cfg35::worldMap::NativeClassCache,nativeClass)||!cfgprobe::ObjectPointer(nativeClass)||
        !r.Read(type+cfgprobe::ObjectClass,meta)||!cfgmenu::maps::Subclass(r,meta,nativeClass)||
        !cfgmenu::maps::Subclass(r,type,killerBase)||
        !cfgspawn::ObjectSlot(r,registry,type,c.identity)||!c.identity.HasAssignedSerial()||
        !r.Read(type+cfgcompat::cfg35::primaryJason::SelectedClassFlags,flags)||
        (flags&cfgcompat::cfg35::primaryJason::UnspawnableSelectedClassFlags)||
        !r.Read(type+cfgspawn::ClassDefaultObject,defaults)||defaults==type||
        !cfgspawn::ObjectSlot(r,registry,defaults,c.defaultsIdentity)||
        !r.Read(defaults+cfgprobe::ObjectClass,again)||again!=type||!r.Read(type+0x20,owner)||
        !cfgprobe::ObjectPointer(owner)||owner==type||!r.Read(owner+0x20,parent)||parent)return false;
    cfgnames::Observation<Reader> names(r,module);cfgnames::PathText klass,package;
    constexpr std::u16string_view prefix=u"/Game/Characters/Killers/";
    if(!names.Object(type,klass)||!names.Object(owner,package)||klass.View().empty()||
        package.View().substr(0,prefix.size())!=prefix)return false;
    c.length=package.count+1+klass.count;if(c.length>=c.path.size())return false;
    std::copy_n(package.data.begin(),package.count,c.path.begin());c.path[package.count]=u'.';
    std::copy_n(klass.data.begin(),klass.count,c.path.begin()+package.count+1);
    cfgnames::PathText freshClass,freshPackage;cfgprobe::lifetime::SlotObservation slot;
    if(!c.Valid()||!names.Object(type,freshClass)||freshClass.View()!=klass.View()||
        !r.Read(type+0x20,again)||again!=owner||!names.Object(owner,freshPackage)||freshPackage.View()!=package.View()||
        !r.Read(owner+0x20,again)||again||!r.Read(type+cfgspawn::ClassDefaultObject,again)||again!=defaults||
        !r.Read(defaults+cfgprobe::ObjectClass,again)||again!=type||
        !r.Read(type+cfgcompat::cfg35::primaryJason::SelectedClassFlags,againFlags)||againFlags!=flags||
        !cfgspawn::ObjectSlot(r,registry,type,slot)||!(slot==c.identity)||
        !cfgspawn::ObjectSlot(r,registry,defaults,slot)||!(slot==c.defaultsIdentity)||
        !r.Read(type+cfgprobe::ObjectClass,again)||again!=meta||
        !r.Read(module+cfgcompat::cfg35::worldMap::NativeClassCache,again)||again!=nativeClass||
        !cfgmenu::maps::Subclass(r,meta,nativeClass)||!cfgmenu::maps::Subclass(r,type,killerBase))return false;
    out=c;return true;
}
template<class Reader>bool Current(Reader& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t killerBase,const Choice& expected){
    Choice fresh;return expected.Valid()&&Observe(r,module,registry,expected.identity.object,killerBase,fresh)&&fresh==expected;
}
}
