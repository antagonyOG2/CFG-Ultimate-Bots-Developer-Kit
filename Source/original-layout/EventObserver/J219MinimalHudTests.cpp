#define CFG_J215_HUD_TEST_MAIN ExistingHudGraphFixture
#include "J215PracticeObservationTests.cpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("FAIL check=%u\n",checks);}};
 for(const auto* name:{"SCHealth_Widget_C","SCStamina_Widget_C","CounselorMinimapWidget_C","SCInventory_Widget","SCInventory_Widget_C",
     "SweaterAbilityWidget_C","SC_Noise_Widget_C","ActionMap","MessagePanel","Timer","PoliceCanvas","SCHealth_Widget_C_Child"}){
  WidgetFixture f;constexpr std::uintptr_t ChildType=0x2B00000,StaminaBase=0x2B10000;
  f.Slot(ChildType,21,121);f.Slot(StaminaBase,22,122);f.r.Set(f.Child+cfgprobe::ObjectClass,ChildType);
  const auto label=std::string_view(name);const bool stamina=label=="SCStamina_Widget_C",minimap=label=="CounselorMinimapWidget_C";
  f.r.Set(ChildType+cfgprobe::ClassSuper,stamina?StaminaBase:f.WidgetType);f.r.Set(StaminaBase+cfgprobe::ClassSuper,f.WidgetType);
  f.r.Set(f.Module+cfgpractice::disk::SCStaminaWidgetReflection,StaminaBase);f.r.Set(f.Module+cfgpractice::disk::SCStaminaWidgetConstructor,StaminaBase);
  f.Label(ChildType,name);f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});
  f.r.Set(f.Child+cfgpractice::disk::Visibility,std::uint8_t{4});f.r.Set(f.hud.widget.object+cfgpractice::disk::Minimap,minimap?f.Child:std::uintptr_t{});
  cfgprobe::lifetime::ArrayHeader array;check(cfgprobe::lifetime::ReadArrayHeader(f.r,f.Module+cfgprobe::lifetime::ObjectArrayRva,array));
  struct Host {WidgetFixture& f;cfgprobe::lifetime::ArrayHeader& array;bool minimap=false;unsigned writes=0;
   bool Current(){return cfgpracticeobs::CurrentMinimalTarget(f.r,f.Module,array,f.hud,f.graph,f.Child,1,minimap)!=cfgpracticeobs::MinimalTarget::None;}
   bool Read(std::uint8_t& value){return f.r.Read(f.Child+cfgpractice::disk::Visibility,value);}
   cfgpractice::Write Set(std::uint8_t value){++writes;f.r.Set(f.Child+cfgpractice::disk::Visibility,value);return cfgpractice::Write::Returned;}
  }host{f,array,minimap};cfgpractice::Override<std::uint8_t> own;
  const bool target=label=="SCHealth_Widget_C"||stamina||minimap;check(own.Apply(host,1)==target);std::uint8_t value=255;host.Read(value);
  check(value==(target?1:4)&&host.writes==(target?1u:0u));
  if(target){check(own.Restore(host));host.Read(value);check(value==4&&host.writes==2);
   if(minimap)f.r.Set(f.hud.widget.object+cfgpractice::disk::Minimap,f.Root);
   else f.r.Set(f.SlotObject+cfgpractice::disk::Content,f.Root);
   own={};check(!own.Apply(host,1)&&host.writes==2);}
 }
 std::printf("J219 current health+stamina+minimap collapse/restore, inventory/sweater/noise/objectives/timer/police preserved, stale paths refuse: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
