#include "NativeChallengePageResolver.hpp"
#include <cstdio>
namespace {
struct RefusingReader {unsigned reads=0;template<class T>bool Read(std::uintptr_t,T&){++reads;return false;}
 bool ReadBytes(std::uintptr_t,void*,std::size_t){++reads;return false;}};
struct Engine {unsigned calls=0;bool BeginPhase(){++calls;return true;}
 bool Factory(std::uintptr_t,std::uintptr_t&){++calls;return false;}
 bool Invoke(std::uintptr_t,std::uintptr_t,void*){++calls;return false;}};
}
int main(){using namespace cfgmenu::challengepage;unsigned checks=0,failed=0;
 const auto check=[&](bool value){++checks;if(!value)++failed;};
 const auto reference=SoftReference({123,0});std::int32_t weak=0;cfgnames::Words asset{};
 std::memcpy(&weak,reference.bytes.data(),4);std::memcpy(&asset,reference.bytes.data()+16,8);
 check(weak==-1&&asset==cfgnames::Words{123,0});
 for(unsigned i=4;i<16;++i)check(reference.bytes[i]==0);
 for(unsigned i=24;i<40;++i)check(reference.bytes[i]==0);
 cfgmenu::capture::FunctionEvidence evidence;evidence.function.object=0x20000;
 evidence.function.label.count=17;const char16_t title[]=u"Conv_StringToName";
 std::memcpy(evidence.function.label.data.data(),title,34);
 // Independent CLOSED live58 capture, not copied from a binding constant:
 // function14022401, inString0018000000000280, return0018001040000780.
 evidence.parameterBytes=24;evidence.storageBytes=24;evidence.flags=0x14022401;evidence.fieldsCount=2;
 for(unsigned i=0;i<2;++i){auto& p=evidence.fields[i];p.field.owner=evidence.function.object;p.dimension=1;
  const auto name=i?u"ReturnValue":u"InString";const auto kind=i?u"NameProperty":u"StrProperty";
  p.field.label.count=i?11:8;p.field.kind.count=i?12:11;
  std::memcpy(p.field.label.data.data(),name,p.field.label.count*2);std::memcpy(p.field.kind.data.data(),kind,p.field.kind.count*2);
  p.offset=i?16:0;p.elementBytes=i?8:16;p.flags=i?0x0018001040000780ULL:0x0018000000000280ULL;}
 check(NameSchema(evidence));
 for(unsigned fault=0;fault<11;++fault){auto bad=evidence;
  switch(fault){case 0:bad.parameterBytes=16;break;case 1:bad.storageBytes=32;break;
   case 2:bad.fields[1].offset=8;break;case 3:bad.fields[1].elementBytes=4;break;
   case 4:bad.fields[0].flags|=0x400;break;case 5:bad.fields[1].flags=0x80;break;
   case 6:bad.destructorLink=0x30000;break;case 7:bad.fields[1].field.owner=0x30000;break;
   case 8:bad.flags=0x04022401;break;case 9:bad.flags=0x14022481;break;case 10:bad.fields[0].flags|=0x8000000;break;}
  check(!NameSchema(bad));}
 RefusingReader r;Engine engine;Resolver resolver;cfgmenu::active::Context context{};
 context.thread=GetCurrentThreadId();resolver.Service(r,engine,context);
 check(resolver.stage==5&&resolver.failure==1&&!resolver.attempts&&!engine.calls);
 const auto reads=r.reads;resolver.Service(r,engine,context);check(r.reads==reads&&!engine.calls);
 std::printf("Challenges actual resolver packet/schema/source-refusal: %u checks, %u failures; no game access or native calls.\n",checks,failed);
 return failed?1:0;
}
