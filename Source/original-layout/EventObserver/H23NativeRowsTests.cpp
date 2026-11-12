#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 using namespace cfgmenu;using M=MenuFixture;using F=NativeFixture;
 NativeFixture f;OptionsReader reader{f.fixture.r};ModelEngine engine{f};engine.expectedWorld=F::Controller;
 auto text=std::make_unique<reflection::RetainedMenuText>();rows::NativeAccess<OptionsReader,ModelEngine> access(reader,engine,*text);
 active::Context context;check(active::Observe(reader,M::Base,context),"owned native fixture context");
 f.fixture.r.Set(F::World+cfgprobe::WorldNetDriver,std::uintptr_t{0xA00000});
 f.fixture.Name(M::DonorType,"LobbyOptionComboBoxWidget_C");
 bool hostCurrent=true;const auto current=[](OptionsReader&,void* state){return *static_cast<bool*>(state);};
 check(!access.PrepareHosted(context,91,M::Donor,F::Panel,&hostCurrent,current)&&!engine.calls,"no dedicated retained scope means no preparation/calls");
 rows::NativeRetentionHooks hooks{&hostCurrent,[](void* p,std::uintptr_t){return *static_cast<bool*>(p);},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t,void*){return true;},[](void*,std::uintptr_t,std::uintptr_t,void*){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return true;},[](void*,std::uintptr_t,std::uintptr_t){return true;},
  [](void*){},[](void*){},[](void*){return true;},nullptr};
 rows::HostedLobbyRetentionScope scope(M::Base,hooks,true);
 check(!active::Current(reader,context,42),"offline menu guard still rejects this nonzero NetDriver");
 const bool prepared=access.PrepareHosted(context,91,M::Donor,F::Panel,&hostCurrent,current);if(!prepared)std::printf("prepareStage=%u currentReason=%u engineCalls=%u\n",access.prepareStage,access.currentReason,engine.calls);
 check(prepared&&!engine.calls,"hosted adapter uses its explicit sibling proof, real native class/parent/options schema");
 std::uintptr_t row=0;bool attached=false;
 check(!access.AppendHostedAfter(0,row,attached)&&!engine.calls,"refuse wrong anchor insertion index before creation");
 check(access.AppendHostedAfter(1,row,attached)&&attached&&row==F::NewWidget,"native Create/attach is atomic and appends after exact final native anchor");
 rows::Spec spec;spec.id=RowId::OnlineJason;spec.title=u"Jason Bot";spec.count=2;spec.labels[0]=u"Off";spec.labels[1]=u"On";spec.preserveNativeValue=true;
 check(access.CopyOptions(row,spec),"actual options ABI deep-copy and selected-index readback");
 reflection::RetainedMenuText::Request title;
 check(access.Text(spec.title,title)&&access.SetText(row,u"TitleText",title)&&engine.texts==1,"set only title; native value binding is preserved");
 f.fixture.r.Set(row+0x210,1);unsigned selected=0;
 check(access.Index(row,selected)&&selected==1,"native arrows expose current stored value");
 hostCurrent=false;const auto calls=engine.calls;
 check(!access.CopyOptions(row,spec)&&!access.RemoveOwned(row)&&engine.calls==calls,"host loss causes no old-tree calls or removals");
 hostCurrent=true;check(access.RemoveOwned(row)&&engine.removes==1,"rollback removes only the newly owned row");
 std::printf("H23 actual native lobby-row adapter: %u checks, %u failures; owned engine model, offline NetDriver guard unchanged.\n",checks,failures);return failures?1:0;
}
