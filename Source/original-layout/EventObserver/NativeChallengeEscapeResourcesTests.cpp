#include "NativeChallengeEscapeResources.hpp"
#include <map>
#include <cstdio>
#include <cstring>
namespace res=cfgchallengeescape::resources;
namespace disk=cfgchallengeescape::disk;
struct EscapeResourceReader {
 std::map<std::uintptr_t,unsigned char> memory;unsigned reads=0;
 std::uintptr_t changeAt=0;unsigned changeOn=0,hits=0;
 template<class T>void Set(std::uintptr_t at,const T& value){SetBytes(at,&value,sizeof(value));}
 void SetBytes(std::uintptr_t at,const void* value,std::size_t length){const auto* p=static_cast<const unsigned char*>(value);
  for(std::size_t i=0;i<length;++i)memory[at+i]=p[i];}
 bool ReadBytes(std::uintptr_t at,void* value,std::size_t length){
  if(++reads>8192)return false;
  if(at==changeAt&&++hits==changeOn)Set(at,std::uint64_t{0x280});
  auto* p=static_cast<unsigned char*>(value);for(std::size_t i=0;i<length;++i){const auto it=memory.find(at+i);
   if(it==memory.end())return false;p[i]=it->second;}return true;
 }
 template<class T>bool Read(std::uintptr_t at,T& value){T candidate{};if(!ReadBytes(at,&candidate,sizeof(candidate)))return false;value=candidate;return true;}
};
struct ResourceFixture {
 static constexpr std::uintptr_t Module=0x100000000,Names=0x200000,Chunk=0x300000,Function=0x400000,
  FunctionKind=0x500000,Owner=0x600000,Target=0x700000,Property=0x800000,Kind=0x900000;
 EscapeResourceReader r;unsigned names=0;const disk::ApiBinding& binding;
 void Label(std::uintptr_t object,std::u16string_view text){const auto id=names++;const std::uintptr_t entry=0xA00000+id*0x1000;
  r.Set(Chunk+8*id,entry);r.Set(entry,std::uint32_t{1});std::array<char16_t,128> words{};
  std::copy(text.begin(),text.end(),words.begin());r.Set(entry+cfgnames::disk::EntryText,words);
  r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(id),0});
  r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(names),1}});
 }
 void Field(std::uintptr_t object,std::uintptr_t owner,std::uintptr_t kind,std::u16string_view label,std::uintptr_t next){
  Label(object,label);r.Set(object+0x20,owner);r.Set(object+cfgprobe::ObjectClass,kind);r.Set(object+cfgreflection::disk::Next,next);
 }
 explicit ResourceFixture(unsigned index):binding(disk::Apis[index]){const auto& b=binding;
  r.Set(Module+cfgnames::disk::Global,Names);r.Set(Names,Chunk);
  r.Set(Module+b.owner,Owner);r.Set(Module+b.ctor,Owner);r.Set(Module+b.cache,Function);
  r.Set(Module+b.factory,std::array<unsigned char,47>{});r.SetBytes(Module+b.factory,b.factoryCode,47);
  r.SetBytes(Module+b.executor,b.code,b.codeBytes);Label(FunctionKind,u"Function");
  Field(Function,Owner,FunctionKind,b.name,0);
  r.Set(Function+0x88,b.flags);r.Set(Function+0x8C,static_cast<std::uint8_t>(b.count));
  r.Set(Function+0x8E,static_cast<std::uint16_t>(b.logical));r.Set(Function+cfgreflection::disk::StorageBytes,static_cast<int>(b.storage));
  r.Set(Function+0x68,std::uintptr_t{});r.Set(Function+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{});
  r.Set(Function+0xB0,Module+b.executor);r.Set(Function+cfgreflection::disk::Children,b.count?Property:0);
  r.Set(Target+cfgprobe::ObjectClass,Owner);r.Set(Owner+cfgprobe::ClassSuper,std::uintptr_t{});
  for(unsigned i=0;i<b.count;++i){const auto& p=b.parameters[i];const auto at=Property+i*0x1000,kind=Kind+i*0x1000;
   Label(kind,p.kind);Field(at,Function,kind,p.name,i+1<b.count?at+0x1000:0);
   r.Set(at+cfgreflection::disk::PropertyDimensions,int{1});r.Set(at+cfgreflection::disk::PropertyElementSize,static_cast<int>(p.bytes));
   r.Set(at+cfgreflection::disk::PropertyOffset,static_cast<int>(p.offset));
   // Actual runtime adds unrelated flags; these are deliberately NOT UHT's full word.
   r.Set(at+cfgreflection::disk::PropertyFlags,(p.flags&0x480)|0x0018001040000200ULL);
   r.Set(at+0x70,std::array<unsigned char,4>{{1,0,1,0xFF}});
  }
 }
};
struct ResourceEngine {
 ResourceFixture& f;unsigned factories=0,calls=0;bool factoryResult=true,invokeResult=true;
 bool Factory(std::uintptr_t at,std::uintptr_t& out){++factories;if(at!=ResourceFixture::Module+f.binding.factory||!factoryResult)return false;
  out=ResourceFixture::Function;f.r.Set(ResourceFixture::Module+f.binding.cache,out);return true;}
 bool Invoke(std::uintptr_t target,std::uintptr_t function,void* packet){++calls;return invokeResult&&target==ResourceFixture::Target&&function==ResourceFixture::Function&&packet;}
};
int main(){unsigned checks=0,failures=0,peak=0;
 const auto check=[&](bool ok,const char* text){++checks;if(!ok){++failures;std::printf("FAIL %s\n",text);}};
 const auto current=[](){return true;};
 for(unsigned index=0;index<disk::Apis.size();++index){const auto api=static_cast<res::Api>(index);
  {ResourceFixture f(index);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
   const auto ready=res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d);
   if(ready!=res::Result::Ready)std::printf("Resource index %u failure %u reads %u\n",index,static_cast<unsigned>(d.failure),f.r.reads);
   check(ready==res::Result::Ready,"all twelve real template metadata accept runtime-added CPF flags");
   std::array<unsigned char,32> packet{};
   check(res::Invoke(f.r,e,f.Module,api,budget,current,f.Target,evidence,packet.data(),32,d)&&e.calls==1&&!e.factories,"all twelve native packets dispatched only after current metadata checks");
   peak=(std::max)(peak,f.r.reads);
  }
  {ResourceFixture f(index);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
   f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});
   check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1&&!e.calls&&budget.factorySpent,"cold UFunction warms one factory without action");
   check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1,"warm callback never becomes same-callback call authority");
   res::CallbackBudget next;
   check(res::Ensure(f.r,e,f.Module,api,next,current,evidence,d)==res::Result::Ready,"new callback repeats full schema after factory");
  }
  for(unsigned fault=0;fault<13;++fault){ResourceFixture f(index);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
   switch(fault){
    case 0:f.r.Set(f.Module+f.binding.ctor,f.Function);break;
    case 1:f.r.Set(f.Function+0x20,f.Function);break;
    case 2:f.Label(f.Function,u"WrongFunction");break;
    case 3:f.r.Set(f.Function+0x88,f.binding.flags^0x400);break;
    case 4:f.r.Set(f.Function+0x8C,static_cast<std::uint8_t>(f.binding.count+1));break;
    case 5:f.r.Set(f.Function+0x8E,static_cast<std::uint16_t>(f.binding.logical+1));break;
    case 6:f.r.Set(f.Function+cfgreflection::disk::StorageBytes,static_cast<int>(f.binding.storage+1));break;
    case 7:f.r.Set(f.Function+0x68,f.Property);break;
    case 8:f.r.Set(f.Function+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{f.Property,1,1});break;
    case 9:f.r.Set(f.Function+0xB0,f.Module+f.binding.factory);break;
    case 10:f.r.Set(f.Function+cfgreflection::disk::Children,f.Function);break;
    case 11:f.r.Set(f.Module+f.binding.cache,f.Property);break;
    case 12:f.Label(f.FunctionKind,u"ObjectProperty");break;
   }
   check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)!=res::Result::Ready&&!e.calls&&!e.factories,"all function/cache/schema corruptions refuse without native calls");
  }
  for(unsigned parameter=0;parameter<disk::Apis[index].count;++parameter){for(unsigned fault=0;fault<10;++fault){ResourceFixture f(index);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
    const auto at=f.Property+parameter*0x1000;const auto& p=f.binding.parameters[parameter];
    switch(fault){
     case 0:f.Label(at,u"WrongParameter");break;
     case 1:f.Label(f.Kind+parameter*0x1000,u"IntProperty");break;
     case 2:f.r.Set(at+0x20,f.Owner);break;
     case 3:f.r.Set(at+cfgreflection::disk::PropertyDimensions,int{2});break;
     case 4:f.r.Set(at+cfgreflection::disk::PropertyElementSize,static_cast<int>(p.bytes+1));break;
     case 5:f.r.Set(at+cfgreflection::disk::PropertyOffset,static_cast<int>(p.offset+1));break;
     case 6:f.r.Set(at+cfgreflection::disk::PropertyFlags,std::uint64_t{});break;
     case 7:f.r.Set(at+cfgreflection::disk::PropertyFlags,(p.flags&0x480)^0x400);break;
     case 8:f.r.Set(at+cfgreflection::disk::Next,at);break;
     case 9:f.r.Set(at+cfgreflection::disk::Next,f.Function);break;
    }
    check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)==res::Result::Refused&&!e.calls&&!e.factories,"every owned parameter role/name/kind/dim/size/offset/link fault refused");
   }
   if(disk::Apis[index].parameters[parameter].kind==u"BoolProperty")for(unsigned byte=0;byte<4;++byte){ResourceFixture f(index);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
    f.r.Set(f.Property+parameter*0x1000+0x70+byte,std::uint8_t{2});
    check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)==res::Result::Refused,"native byte Boolean layout corruptions refused");
   }
  }
  {ResourceFixture f(index);ResourceEngine e{f};res::Diagnostic d;res::Evidence evidence;
   for(unsigned region=0;region<2;++region){const auto at=f.Module+(region?f.binding.executor:f.binding.factory);
    const auto length=region?f.binding.codeBytes:47u;
    for(unsigned byte=0;byte<length;++byte){const auto old=f.r.memory[at+byte];f.r.Set(at+byte,static_cast<unsigned char>(old^1));f.r.reads=0;res::CallbackBudget budget;
     check(res::Ensure(f.r,e,f.Module,api,budget,current,evidence,d)==res::Result::Refused&&!e.factories&&!e.calls,"every byte of full factory/executor pin enforced");f.r.Set(at+byte,old);
    }
   }
  }
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  f.r.Set(f.Module+f.binding.owner,std::uintptr_t{});f.r.Set(f.Module+f.binding.ctor,std::uintptr_t{});f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Deferred&&!e.factories&&budget.blocked,"cold class never invokes guessed native class constructor");
  f.r.Set(f.Module+f.binding.owner,f.Owner);f.r.Set(f.Module+f.binding.ctor,f.Owner);
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Refused&&!e.factories,"owner failure blocks factories through same callback");
 }
 {ResourceFixture f(0);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  const auto& other=disk::Apis[1];f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});f.r.Set(f.Module+other.cache,std::uintptr_t{});
  f.r.SetBytes(f.Module+other.factory,other.factoryCode,47);f.r.SetBytes(f.Module+other.executor,other.code,other.codeBytes);
  check(res::Ensure(f.r,e,f.Module,res::Api::ActorLocation,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1,"first resource spends shared callback factory opportunity");
  check(res::Ensure(f.r,e,f.Module,res::Api::ActorRotation,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1,"different cold API cannot register a second factory in callback");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;unsigned scopes=0;
  f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});const auto changedOwner=[&](){if(++scopes==2)f.r.Set(f.Module+f.binding.ctor,f.Function);return true;};
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,changedOwner,evidence,d)==res::Result::Deferred&&!e.factories&&budget.blocked,"owner drift during current proof prevents factory and blocks callback");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});e.factoryResult=false;
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1&&!d.factoryReturns,"failed registration spends callback opportunity");
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Deferred&&e.factories==1,"failed factory cannot retry within callback");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  const auto refused=[](){return false;};
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,refused,evidence,d)==res::Result::Refused&&!f.r.reads&&!e.calls&&!e.factories,"initial wrong scope causes zero resource reads or actions");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;unsigned scopes=0;
  f.r.Set(f.Module+f.binding.cache,std::uintptr_t{});
  const auto losesScope=[&](){return ++scopes<2;};
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,losesScope,evidence,d)==res::Result::Refused&&!e.factories,"scope repeated at factory edge");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Ready,"dispatch mutation fixture prepared");
  const auto at=f.Property+cfgreflection::disk::PropertyFlags;std::uint64_t flags=0;f.r.Read(at,flags);f.r.Set(at,flags^0x10000000000000ULL);
  res::TeleportPacket packet;
  check(!res::Invoke(f.r,e,f.Module,res::Api::Teleport,budget,current,f.Target,evidence,&packet,sizeof(packet),d)&&!e.calls,"complete sampled CPF flags rechecked at dispatch despite role-only initial matching");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  f.r.changeAt=f.Property+cfgreflection::disk::PropertyFlags;f.r.changeOn=2;
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Refused&&!e.calls,"schema drift between samples refuses");
 }
 {ResourceFixture f(3);ResourceEngine e{f};res::CallbackBudget budget;res::Evidence evidence;res::Diagnostic d;
  check(res::Ensure(f.r,e,f.Module,res::Api::Teleport,budget,current,evidence,d)==res::Result::Ready,"target/scope fixture prepared");res::TeleportPacket packet;
  check(!res::Invoke(f.r,e,f.Module,res::Api::Teleport,budget,current,f.Target,evidence,&packet,24,d)&&!e.calls,"short packet refused");
  f.r.Set(f.Target+cfgprobe::ObjectClass,f.FunctionKind);f.r.Set(f.FunctionKind+cfgprobe::ClassSuper,std::uintptr_t{});
  check(!res::Invoke(f.r,e,f.Module,res::Api::Teleport,budget,current,f.Target,evidence,&packet,sizeof(packet),d)&&!e.calls,"wrong target class refused");
  f.r.Set(f.Target+cfgprobe::ObjectClass,f.Owner);unsigned scopes=0;const auto departed=[&](){return ++scopes<2;};
  check(!res::Invoke(f.r,e,f.Module,res::Api::Teleport,budget,departed,f.Target,evidence,&packet,sizeof(packet),d)&&!e.calls,"scope repeated at invoke edge");
 }
 check(peak<8192,"guard shares existing reader budget without widening");
 std::printf("Challenge escape resource production template: %u checks, %u failures, peak %u/8192 reads; owned synthetic memory only.\n",checks,failures,peak);
 return failures?1:0;
}
