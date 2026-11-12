#define CFG_J215_HUD_TEST_MAIN ExistingHudGraphFixture
#include "J215PracticeObservationTests.cpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check=%u\n",checks);}};
 for(const auto* name:{"SCHealth_Widget_C","SCStamina_Widget_C","SweaterAbilityWidget_C","SC_Noise_Widget_C","ActionMap","MessagePanel","Timer","PoliceCanvas","SCHealth_Widget_C_Child"}){
  WidgetFixture f;f.Slot(f.WidgetType,21,121);f.Label(f.WidgetType,name);f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
  f.r.Set(f.Child+cfgpractice::disk::Visibility,std::uint8_t{4});cfgprobe::lifetime::ArrayHeader array;
  check(cfgprobe::lifetime::ReadArrayHeader(f.r,f.Module+cfgprobe::lifetime::ObjectArrayRva,array));
  struct Host {WidgetFixture& f;cfgprobe::lifetime::ArrayHeader& array;unsigned writes=0;
   bool Current(){return cfgpracticeobs::NodeCurrent(f.r,f.Module,array,f.hud,f.graph,1)&&cfgpracticeobs::ExactHealthWidget(f.r,f.Module,array,f.Child);}
   bool Read(std::uint8_t& value){return f.r.Read(f.Child+cfgpractice::disk::Visibility,value);}
   cfgpractice::Write Set(std::uint8_t value){++writes;f.r.Set(f.Child+cfgpractice::disk::Visibility,value);return cfgpractice::Write::Returned;}
  }host{f,array};cfgpractice::Override<std::uint8_t> own;
  const bool health=std::string_view(name)=="SCHealth_Widget_C";check(own.Apply(host,1)==health);std::uint8_t value=255;host.Read(value);
  check(value==(health?1:4)&&host.writes==(health?1u:0u));
  if(health){check(own.Restore(host));host.Read(value);check(value==4&&host.writes==2);
   f.r.Set(f.SlotObject+cfgpractice::disk::Content,f.Root);own={};check(!own.Apply(host,1)&&host.writes==2);}
 }
 std::printf("J218 exact current SCHealth_Widget_C only collapse/own-value restore; objective/sweater/noise/stamina/foreign path untouched: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
