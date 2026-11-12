#define main ExistingSetupWidgetObservationTests
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "../RuntimeProbe/MenuScriptObservation.hpp"
// Reference is the pre-ABI71 reader, retained only as an independent behavioral
// and I/O-cost oracle. No game process, native function or persistent cache.
template<class R>bool LegacyField(R& r,std::uintptr_t object,std::uintptr_t owner,cfgreflection::Field& out){
 out={};cfgreflection::Field f;f.object=object;cfgnames::Observation<R> names(r,MenuFixture::Base);
 if(!cfgprobe::ObjectPointer(object)||!r.Read(object+0x20,f.owner)||f.owner!=owner||
    !r.Read(object+cfgprobe::ObjectClass,f.type)||!cfgprobe::ObjectPointer(f.type)||
    !r.Read(object+cfgreflection::disk::Next,f.next)||(f.next&&!cfgprobe::ObjectPointer(f.next))||
    !r.Read(object+cfgnames::disk::ObjectName,f.name)||!names.Decode(f.name,f.label)||!names.Object(f.type,f.kind))return false;
 std::uintptr_t type=0,parent=0,next=0;cfgnames::Words name{};
 if(!r.Read(object+cfgprobe::ObjectClass,type)||type!=f.type||!r.Read(object+0x20,parent)||parent!=f.owner||
    !r.Read(object+cfgreflection::disk::Next,next)||next!=f.next||!r.Read(object+cfgnames::disk::ObjectName,name)||!(name==f.name))return false;
 out=f;return true;
}
struct FieldReader {
 OwnedByteReader& memory;unsigned reads=0,headers=0,cap=4096,fault=0;bool changed=false;
 template<class T>bool Read(std::uintptr_t at,T& out){
  if(reads==cap)return false;++reads;
  if(at==MenuFixture::Member+0x10&&sizeof(T)==32)++headers;
  // Change a header member during name decoding, AFTER its initial sample.
  if(!changed&&fault&&at==MenuFixture::Base+cfgnames::disk::Global){changed=true;
   if(fault==1)memory.Set(MenuFixture::Member+0x10,MenuFixture::ArrayProperty);
   if(fault==2)memory.Set(MenuFixture::Member+0x18,cfgnames::Words{0,99});
   if(fault==3)memory.Set(MenuFixture::Member+0x20,MenuFixture::DonorType);
   if(fault==4)memory.Set(MenuFixture::Member+0x28,MenuFixture::Options);
  }
  return memory.Read(at,out);
 }
};
int main(){unsigned checks=0,failures=0;
 const auto check=[&](bool good,const char* name){++checks;if(!good){++failures;std::printf("FAIL %s\n",name);}};
 unsigned beforeReads=0,afterReads=0;
 {MenuFixture f;FieldReader before{f.r},after{f.r};cfgreflection::Field oldField,newField;
  cfgreflection::Observation<FieldReader> reflection(after,MenuFixture::Base);
  check(LegacyField(before,MenuFixture::Member,MenuFixture::Type,oldField)&&
   reflection.ReadField(MenuFixture::Member,MenuFixture::Type,newField),"legacy and production admit the same current exact field");
  check(oldField.object==newField.object&&oldField.owner==newField.owner&&oldField.type==newField.type&&
   oldField.next==newField.next&&oldField.name==newField.name&&oldField.label.View()==newField.label.View()&&oldField.kind.View()==newField.kind.View(),
   "coherent sampling preserves all semantic field evidence");
  beforeReads=before.reads;afterReads=after.reads;
  check(after.headers==2&&afterReads+6==beforeReads,"two fresh32-byte headers replace eight scalar reads, saving six reads per field");
 }
 for(unsigned fault=1;fault<=4;++fault){
  MenuFixture f;FieldReader r{f.r};r.fault=fault;cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;
  check(!reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field)&&!field.object,
   "type/name/owner/next change during observation refuses without partial evidence");
 }
 {MenuFixture f;FieldReader r{f.r};cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;
  check(!reflection.ReadField(MenuFixture::Member,MenuFixture::DonorType,field)&&!field.object,"wrong owner refused before name lookup");
  check(!reflection.ReadField(0x11,MenuFixture::Type,field),"unaligned field pointer refused");
 }
 {MenuFixture f;FieldReader r{f.r};r.cap=1;cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;
  check(!reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field)&&!field.object&&r.reads==1,"original shared read cap remains binding");
 }
 {MenuFixture f;FieldReader r{f.r};f.r.bytes.erase(MenuFixture::Member+0x20);
  cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;
  check(!reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field)&&!field.object,"partial header read cannot publish field evidence");
 }
 {MenuFixture f;FieldReader r{f.r};cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;
  check(reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field),"first current observation succeeds");
  f.r.Set(MenuFixture::Member+0x20,MenuFixture::DonorType);
  check(!reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field)&&!field.object,"next observation reads fresh memory rather than cached authority");
 }
 for(unsigned fault=0;fault<12;++fault){MenuFixture f;FieldReader r{f.r};
  f.r.Set(MenuFixture::Member+cfgreflection::disk::PropertyDimensions,int{1});
  f.r.Set(MenuFixture::Member+cfgreflection::disk::PropertyElementSize,int{8});
  f.r.Set(MenuFixture::Member+cfgreflection::disk::PropertyFlags,std::uint64_t{0x1234567890ULL});
  f.r.Set(MenuFixture::Member+cfgreflection::disk::PropertyOffset,int{0x1A0});
  cfgreflection::Observation<FieldReader> reflection(r,MenuFixture::Base);cfgreflection::Field field;cfgreflection::Property property;
  check(reflection.ReadField(MenuFixture::Member,MenuFixture::Type,field)&&cfgmenu::capture::RawProperty(r,field,property),"known metadata fixture captured independently");
  if(fault==1)f.r.Set(MenuFixture::Member+0x10,MenuFixture::ArrayProperty);
  if(fault==2)f.r.Set(MenuFixture::Member+0x18,cfgnames::Words{0,99});
  if(fault==3)f.r.Set(MenuFixture::Member+0x20,MenuFixture::DonorType);
  if(fault==4)f.r.Set(MenuFixture::Member+0x28,MenuFixture::Options);
  if(fault==5)f.r.Set(MenuFixture::Member+0x30,int{2});
  if(fault==6)f.r.Set(MenuFixture::Member+0x34,int{4});
  if(fault==7)f.r.Set(MenuFixture::Member+0x38,std::uint64_t{0x1234567891ULL});
  if(fault==8)f.r.Set(MenuFixture::Member+0x44,int{0x1A8});
  if(fault==9)f.r.bytes.erase(MenuFixture::Member+0x1C);
  if(fault==10)f.r.bytes.erase(MenuFixture::Member+0x37);
  r.reads=0;if(fault==11)r.cap=2;
  check(reflection.CurrentProperty(property)==(fault==0),"fresh grouped property refuses every changed identity/layout field, partial read and insufficient budget");
  if(!fault){check(r.reads==3,"fresh exact property uses three reads without dropping semantic comparisons");
   f.r.Set(MenuFixture::Member+0x34,int{4});check(!reflection.CurrentProperty(property),"next property check observes mutation without a cached authority lease");}
 }
 std::printf("Reflection field observation: %u checks %u failures; legacy=%u production=%u reads; owned memory only.\n",checks,failures,beforeReads,afterReads);
 return failures?1:0;
}
