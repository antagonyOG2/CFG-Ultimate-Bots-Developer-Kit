#pragma once
#include "../RuntimeProbe/MenuNativeEvidence.hpp"
#include "SpawnBindings.hpp"
namespace cfghostedlobbystatic {
// A registered root, never a synthetic weak serial. Only the two existing
// native static-library calls used by rows are eligible (Create / string text).
struct Proof {
 unsigned kind=0;std::uintptr_t target=0,function=0,owner=0;
 std::array<cfgprobe::lifetime::SlotObservation,3> slots{};
 bool Contains(std::uintptr_t value)const{return kind&&(value==target||value==function||value==owner);}
};
struct Failure {unsigned reason=0;std::uintptr_t value=0;cfgprobe::lifetime::SlotObservation slot{};
 std::uint32_t internal=~0u,flags=~0u;};
template<class R>bool Slot(R& r,const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t value,
 bool positive,cfgprobe::lifetime::SlotObservation& out,Failure& d,bool defaultObject=false){
 d={};d.value=value;d.reason=1;
 if(!cfgprobe::lifetime::ReadSlot(r,registry,value,d.slot))return false;
 d.reason=2;if(!r.Read(registry.data+24ULL*static_cast<unsigned>(d.slot.index)+8,d.internal)||!r.Read(value+8,d.flags))return false;
 d.reason=3;if((d.internal&0x30000000u)||(defaultObject?!(d.flags&0x10u):(d.flags&0x30u)))return false;
 d.reason=4;if(positive&&!d.slot.HasAssignedSerial())return false;
 cfgprobe::lifetime::SlotObservation again;std::uint32_t internal=0,flags=0;
 d.reason=5;if(!cfgprobe::lifetime::ReadSlot(r,registry,value,again)||!(again==d.slot)||
  !r.Read(registry.data+24ULL*static_cast<unsigned>(again.index)+8,internal)||internal!=d.internal||
  !r.Read(value+8,flags)||flags!=d.flags)return false;
 out=d.slot;d.reason=0;return true;
}
template<class R,std::size_t P,std::size_t C>bool Binding(R& r,std::uintptr_t module,
 const cfgprobe::lifetime::ArrayHeader& registry,unsigned kind,std::uintptr_t target,std::uintptr_t function,
 const cfgmenu::execdisk::Binding& binding,const std::array<cfgmenu::execdisk::Parameter,P>& parameters,
 const std::array<unsigned char,C>& code,Proof& out,Failure& d){
 out={};d={};d.value=function;d.reason=6;cfgmenu::capture::FunctionEvidence f;
 if(!cfgmenu::native::CachedFunction(r,module,binding,parameters,code,f)||f.function.object!=function)return false;
 d.reason=7;std::uintptr_t cdo=0;
 if(!r.Read(f.function.owner+cfgspawn::ClassDefaultObject,cdo)||cdo!=target)return false;
 cfgprobe::Ancestry ancestry;d.value=target;d.reason=8;
 if(!cfgprobe::CaptureAncestry(r,target,ancestry)||!ancestry.Contains(f.function.owner)||!cfgprobe::StableAncestry(r,target,ancestry))return false;
 Proof p;p.kind=kind;p.target=target;p.function=function;p.owner=f.function.owner;
 const std::array<std::uintptr_t,3> values{target,function,p.owner};
 for(unsigned i=0;i<values.size();++i)if(!Slot(r,registry,values[i],false,p.slots[i],d,i==0))return false;
 std::uintptr_t fresh=0;d.value=function;d.reason=9;
 if(!r.Read(module+binding.cache,fresh)||fresh!=function||!r.Read(p.owner+cfgspawn::ClassDefaultObject,fresh)||fresh!=target||
    !cfgprobe::StableAncestry(r,target,ancestry))return false;
 out=p;d.reason=0;return true;
}
template<class R>bool Observe(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& registry,
 unsigned kind,std::uintptr_t target,std::uintptr_t function,Proof& out,Failure& d){
 using namespace cfgmenu::execdisk;
 if(kind==1)return Binding(r,module,registry,kind,target,function,Create,CreateParameters,CreateCode,out,d);
 if(kind==2)return Binding(r,module,registry,kind,target,function,Conv_StringToText,Conv_StringToTextParameters,Conv_StringToTextCode,out,d);
 d={};d.reason=10;d.value=function;return false;
}
template<class R>bool Current(R& r,std::uintptr_t module,const Proof& expected,Failure& d){
 if(!expected.kind)return true;cfgprobe::lifetime::ArrayHeader registry;Proof now;
 d={};d.reason=11;
 if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
 if(!Observe(r,module,registry,expected.kind,expected.target,expected.function,now,d))return false;
 if(now.owner!=expected.owner||now.slots!=expected.slots){d.reason=12;d.value=expected.function;return false;}
 return true;
}
}
