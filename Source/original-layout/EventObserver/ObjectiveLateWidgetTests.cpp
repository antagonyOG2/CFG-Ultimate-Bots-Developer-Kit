#define CFG_OBJECTIVE_HUD_FIXTURE_ONLY
#include "ObjectiveHudTests.cpp"
int main(){
 unsigned failures=0;auto check=[&](bool value){if(!value)++failures;};unsigned peak=0;
 // Regression: objective class absent throughout BOTH finite surveys, then
 // loaded by a real widget event. Only its exact helpers resume after expiry.
 for(unsigned fault=0;fault<5;++fault){
  ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  f.r.Set(ObjectiveFixture::Registry,std::uintptr_t{0});Step(runtime,f,host,1);Step(runtime,f,host,90001);
  check(runtime.Status().expired&&!runtime.Pending(120000));const unsigned scanned=runtime.Status().scanned;
  f.r.Set(ObjectiveFixture::Registry,MenuFixture::Type);
  constexpr std::uintptr_t widget=0x950000;f.r.Set(widget+16,MenuFixture::Type);
  if(fault==1)host.current=false;
  if(fault==2)f.r.Set(MenuFixture::Start+0x20,MenuFixture::FunctionClass);
  if(fault==3)f.Name(MenuFixture::Type,"Unrelated_Widget_C");
  if(fault==4)f.r.Set(MenuFixture::Base+0x03198110,std::uint32_t{1});
  f.reader.reads=160;f.reader.exhausted=false;
  const bool seeded=runtime.EventOwner(f.reader,MenuFixture::Base,widget,MenuFixture::Start,120000,host);
  check(seeded==(fault==0));check(!host.writes);peak=(std::max)(peak,f.reader.reads);
  if(!fault){
   check(runtime.Pending(120001));Step(runtime,f,host,120001);Step(runtime,f,host,120002);
   check(host.writes==1&&runtime.Status().changedMask==1);
   check(runtime.Status().scanned==scanned&&runtime.Status().expired);
   check(!runtime.Pending(140000));
   f.reader.reads=160;check(!runtime.EventOwner(f.reader,MenuFixture::Base,widget,MenuFixture::Start,140001,host));
  }else check(!runtime.Pending(120001));
 }
 for(unsigned fault=0;fault<6;++fault){
  ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;
  f.Name(MenuFixture::Type,"ObjectivesWidget_C");Step(runtime,f,host,1);
  check(runtime.RootClass()==MenuFixture::Type);
  Step(runtime,f,host,90001);check(runtime.Status().expired&&!runtime.Pending(90002));
  constexpr std::uintptr_t late=0x950000;
  f.r.Set(MenuFixture::Type+0xF8,std::uintptr_t{0x960000});
  f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,3);
  f.r.Set(ObjectiveFixture::Registry+48,std::array<unsigned char,24>{});
  f.r.Set(ObjectiveFixture::Registry+48,late);f.r.Set(ObjectiveFixture::Registry+64,7);
  f.r.Set(late+12,2);f.r.Set(late+16,MenuFixture::Type);
  if(fault==1)host.current=false;
  if(fault==2)f.r.Set(late+16,MenuFixture::FunctionClass);
  if(fault==3)f.r.Set(MenuFixture::Type+0xF8,late);
  if(fault==4)f.r.Set(ObjectiveFixture::Registry+64,0);
  if(fault==5)f.r.Set(ObjectiveFixture::Registry+56,std::uint32_t{0x20000000});
  f.reader.reads=160;unsigned candidate=99;
  const bool admitted=runtime.EventCandidate(f.reader,MenuFixture::Base,late,120000,host,candidate);
  peak=(std::max)(peak,f.reader.reads);check(admitted==(fault==0));check(host.writes==0);
  if(!fault){
   check(candidate==0&&runtime.InstanceCount()==1&&runtime.Instance(0).object==late);
   check(runtime.Status().expired&&!runtime.Pending(120000)); // no restarted survey
   check(!runtime.EventCandidate(f.reader,MenuFixture::Base,late,120001,host,candidate));
   for(unsigned i=1;i<16;++i){f.reader.reads=160;check(runtime.EventCandidate(f.reader,MenuFixture::Base,late,120000+1000ULL*i,host,candidate));}
   f.reader.reads=160;check(!runtime.EventCandidate(f.reader,MenuFixture::Base,late,136000,host,candidate));
   check(runtime.InstanceCount()==1); // repeated events cannot duplicate storage
   runtime.RetireOne(f.reader,MenuFixture::Base,host);
   check(!runtime.EventCandidate(f.reader,MenuFixture::Base,late,200000,host,candidate));
  }else check(runtime.InstanceCount()==0);
 }
 // Actor traffic used to fill the negative cache before UserWidget transport
 // ever arrived. Exhaustion must not classify an unseen objective as negative.
 {ObjectiveFixture f;Host host{f};cfgobjective::Runtime runtime;constexpr std::uintptr_t widget=0x950000;
  for(unsigned i=0;i<128;++i){const auto type=0xA00000ULL+0x1000ULL*i;
   f.Name(type,"Unrelated_Widget_C");f.r.Set(type+16,ObjectiveFixture::Generated);
   f.r.Set(widget+16,type);f.r.Set(MenuFixture::Start+0x20,type);f.reader.reads=160;
   check(!runtime.EventOwner(f.reader,MenuFixture::Base,widget,MenuFixture::Start,120000,host));}
  f.r.Set(widget+16,MenuFixture::Type);f.r.Set(MenuFixture::Start+0x20,MenuFixture::Type);f.reader.reads=160;
  check(!runtime.EventTypeSeen(MenuFixture::Type));
  check(runtime.EventOwner(f.reader,MenuFixture::Base,widget,MenuFixture::Start,120001,host));
  check(runtime.Pending(120002)&&!host.writes);
 }
 std::printf("ObjectiveLateWidgetTests: %u failures; event-only late adoption peak%u/1536 incl160 outer; owner/CDO/serial/flags gates, no survey restart, finite retries\n",failures,peak);
 return failures?1:0;
}
