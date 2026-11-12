#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeChallengeMenuRoute.hpp"
#include "NativeChallengeRowsFixture.hpp"
#include "NativeChallengeSettingsService.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
namespace {
void __fastcall Thunk0(void*,void*,void*){}void __fastcall Thunk1(void*,void*,void*){}void __fastcall Thunk2(void*,void*,void*){}
void __fastcall Thunk3(void*,void*,void*){}void __fastcall Thunk4(void*,void*,void*){}
struct RouteFixture:NativeFixture{
 static constexpr std::uintptr_t PageType=0x1800000,Page=0x1801000,MenuType=0x1802000,Package=0x1803000,
  LobbyStart=0x1810000,LobbyGraph=0x1820000,PageStart=0x1830000,PageBack=0x1840000,PageGraph=0x1850000,
  Push=0x1860000,Pop=0x1870000,Frame=0x1880000,Locals=0x1890000;
 RouteFixture(){using namespace cfgmenu;auto& f=fixture;auto& r=f.r;
  f.Name(MenuFixture::Type,"SP_LobbyWidget_C");f.Name(PageType,"SPChallengesSettingsMenuWidget_C");
  f.Name(Package,"/Game/UI/Menu/FrontEnd/OfflinePlay/SPChallengesSettingsMenuWidget");r.Set(PageType+0x20,Package);
  r.Set(MenuFixture::Type+cfgprobe::ClassSuper,MenuType);r.Set(PageType+cfgprobe::ClassSuper,MenuType);Root(MenuType);
  // GetCurrentMenu is owned by SCHUD, NOT SCMenuWidget. Distinct class
  // objects prevent the old resolver's HUD/widget cache mixup from passing.
  constexpr std::uintptr_t nativeHud=0x1804000;Root(nativeHud);f.Name(nativeHud,"SCHUD");f.Name(MenuType,"SCMenuWidget");
  r.Set(MenuFixture::Base+0x304CFF0,nativeHud);r.Set(MenuFixture::Base+0x304CFF8,nativeHud);
  r.Set(MenuFixture::Base+cfgprobe::dispatch::ThreadIdRva,GetCurrentThreadId());r.Set(Page+cfgprobe::ObjectClass,PageType);Register(Page,104);
  r.Set(MenuFixture::Base+startdisk::ScriptExecutor,startdisk::ScriptExecutorCode);
  r.Set(MenuFixture::Base+parentdisk::PreviousExecutor,parentdisk::PreviousCode);
  r.Set(MenuFixture::Base+parentdisk::PreviousHelper,parentdisk::PreviousLeaf);
  r.Set(Records+parentdisk::RecordClass,MenuFixture::Type);
  GraphFunction(LobbyGraph,MenuFixture::Type,"ExecuteUbergraph_SP_LobbyWidget",2268,220);
  GraphFunction(PageGraph,PageType,"ExecuteUbergraph_SPChallengesSettingsMenuWidget",128,4);
  WrapperFunction(LobbyStart,MenuFixture::Type,"OnStartPressed",LobbyGraph,0x790);
  WrapperFunction(PageStart,PageType,"OnClicked_Start",PageGraph,32);WrapperFunction(PageBack,PageType,"OnClicked_Back",PageGraph,64);
  r.Set(MenuFixture::Type+cfgreflection::disk::Children,LobbyStart);r.Set(LobbyStart+cfgreflection::disk::Next,LobbyGraph);
  r.Set(PageType+cfgreflection::disk::Children,PageStart);r.Set(PageStart+cfgreflection::disk::Next,PageBack);r.Set(PageBack+cfgreflection::disk::Next,PageGraph);
  NativeFunction(challengecalls::PushMenu,Push,challengecalls::PushMenuCode);
  NativeFunction(challengecalls::PopMenu,Pop,challengecalls::PopMenuCode);
  // Read-only wrapper/frame borrowed from exactly the installed function.
  SetFrame(LobbyStart);
 }
 void BaseFunction(std::uintptr_t fn,std::uintptr_t owner,const char* name,int bytes,int storage,unsigned parameters){
  using namespace cfgmenu;auto& f=fixture;auto& r=f.r;f.Field(fn,owner,MenuFixture::FunctionClass,name,0);Register(fn);
  r.Set(fn+cfgreflection::disk::StorageBytes,storage);r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(parameters));
  r.Set(fn+cfgreflection::disk::FunctionFlags,std::uint32_t{});r.Set(fn+0x68,std::uintptr_t{});r.Set(fn+0xB0,MenuFixture::Base+startdisk::ScriptExecutor);
  r.Set(fn+startdisk::FastGraph,std::uintptr_t{});r.Set(fn+startdisk::FastEntry,int{});
  r.Set(fn+scriptdisk::Script,capture::ScriptHeader{bytes?fn+0x4000:0,bytes,bytes});
  r.Set(fn+cfgreflection::disk::Children,std::uintptr_t{});
 }
 void GraphFunction(std::uintptr_t fn,std::uintptr_t owner,const char* name,int bytes,int storage){
  BaseFunction(fn,owner,name,bytes,storage,4);std::array<unsigned char,2268> code{};
  for(int i=0;i<bytes;++i)fixture.r.Set(fn+0x4000+static_cast<unsigned>(i),code[static_cast<unsigned>(i)]);
  fixture.Field(fn+0x100,fn,MenuFixture::IntProperty,"EntryPoint",0);fixture.Property(fn+0x100,4,0);
  fixture.r.Set(fn+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294ULL});fixture.r.Set(fn+cfgreflection::disk::Children,fn+0x100);
 }
 void WrapperFunction(std::uintptr_t fn,std::uintptr_t owner,const char* name,std::uintptr_t graph,int entry){
  BaseFunction(fn,owner,name,22,0,0);cfgnames::Words label{};fixture.r.Read(graph+cfgnames::disk::ObjectName,label);
  std::array<unsigned char,22> code{};code[0]=0x90;code[13]=0x5A;code[18]=0x97;code[19]=0xA5;code[20]=0x39;code[21]=0xE5;
  const std::array<int,3> encoded{{label.index,label.index,label.number}};std::memcpy(code.data()+1,encoded.data(),12);std::memcpy(code.data()+14,&entry,4);
  fixture.r.Set(fn+0x4000,code);
 }
 template<std::size_t N>void NativeFunction(const cfgmenu::challengecalls::Binding& binding,std::uintptr_t fn,const std::array<unsigned char,N>& code){
  using namespace cfgmenu;auto& f=fixture;auto& r=f.r;const auto push=fn==Push;
  BaseFunction(fn,MenuType,push?"PushMenu":"PopMenu",0,static_cast<int>(binding.storage),binding.logical);
  r.Set(fn+0xB0,MenuFixture::Base+binding.executor);r.Set(fn+cfgreflection::disk::FunctionFlags,binding.flags);
  r.Set(MenuFixture::Base+binding.cache,fn);r.Set(MenuFixture::Base+binding.owner,MenuType);r.Set(MenuFixture::Base+binding.ctor,MenuType);
  r.Set(MenuFixture::Base+binding.executor,code);r.Set(fn+cfgreflection::disk::Children,fn+0x100);
  for(unsigned i=0;i<3;++i){const auto prop=fn+0x100+0x100*i,kind=fn+0x3000+0x100*i;
   const char* name=i==0?(push?"MenuClass":"WidgetToFocus"):i==1?(push?"bSkipTransitionOut":"bForce"):"ReturnValue";
   const char* type=i==0?(push?"ClassProperty":"NameProperty"):i==1?"BoolProperty":push?"ObjectProperty":"BoolProperty";
   f.Name(kind,type);f.Field(prop,fn,kind,name,i<2?prop+0x100:0);f.Property(prop,i==0?8:i==1?1:push?8:1,i==0?0:i==1?8:push?16:9);
   const auto flags=i==0?(push?0x001C001040000280ULL:0x0018001040000280ULL):i==1?0x0018041040000282ULL:push?0x0018001040080788ULL:0x0018001040000780ULL;
   r.Set(prop+cfgreflection::disk::PropertyFlags,flags);
  }
 }
 void SetFrame(std::uintptr_t fn,int entry=0){fixture.r.Set(Frame+cfgmenu::startdisk::FrameNode,fn);fixture.r.Set(Frame+cfgmenu::startdisk::FrameObject,fn==LobbyStart||fn==LobbyGraph?MenuFixture::Widget:Page);
  fixture.r.Set(Frame+cfgmenu::startdisk::FrameLocals,Locals);fixture.r.Set(Locals,entry);}
};
struct RouteEngine{
 RouteFixture& f;unsigned calls=0,pushes=0,pops=0,starts=0;bool failPush=false,delayPop=false,replaceLobby=false,clearInactive=true,refusedAfterPush=false,nullAfterPush=false,wrongPageAfterPush=false;
 std::uintptr_t expectedPageType=RouteFixture::PageType,currentLobby=MenuFixture::Widget;
 void RestoreLobby(){using namespace cfgmenu;auto& r=f.fixture.r;
  if(replaceLobby){currentLobby=0x2200000;r.Set(currentLobby+cfgprobe::ObjectClass,MenuFixture::Type);f.Register(currentLobby,704);}
  r.Set(NativeFixture::Records+execdisk::RecordWidget,currentLobby);
  r.Set(NativeFixture::Stack+execdisk::StackArray,active::StackArray{NativeFixture::Records,1,2});
 }
 unsigned Thread()const{return GetCurrentThreadId();}bool BeginPhase(){calls=0;return true;}bool Factory(std::uintptr_t,std::uintptr_t&){return false;}
 bool Invoke(std::uintptr_t object,std::uintptr_t fn,void* packet){++calls;auto& r=f.fixture.r;using R=RouteFixture;using namespace cfgmenu;
  if(fn==R::Push){++pushes;auto& p=*static_cast<challengeroute::PushPacket*>(packet);if(failPush||object!=currentLobby||p.type!=expectedPageType||p.skip!=1)return false;
   p.result=nullAfterPush?0:R::Page;if(wrongPageAfterPush)r.Set(R::Page+cfgprobe::ObjectClass,R::PageType+8);
   r.Set(NativeFixture::Stack+execdisk::StackArray,active::StackArray{NativeFixture::Records,2,2});
   r.Set(NativeFixture::Records+execdisk::RecordBytes+parentdisk::RecordClass,p.type);
   r.Set(NativeFixture::Records+execdisk::RecordBytes+execdisk::RecordWidget,R::Page);
   if(clearInactive)r.Set(NativeFixture::Records+execdisk::RecordWidget,std::uintptr_t{});return !refusedAfterPush;}
  if(fn==R::Pop){++pops;auto& p=*static_cast<challengeroute::PopPacket*>(packet);if(object!=R::Page||p.force!=1||!(p.focus==cfgnames::Words{}))return false;
   p.result=1;if(!delayPop)RestoreLobby();return true;}
  if(fn==R::LobbyStart){++starts;return object==currentLobby;}return false;
 }
};
struct CallbackReader{
 OwnedByteReader& memory;unsigned reads=0,cap=32768;
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=cap)return false;++reads;return memory.Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads>=cap)return false;++reads;
  auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<bytes;++i){auto it=memory.bytes.find(at+i);if(it==memory.bytes.end())return false;p[i]=it->second;}return true;}
};
}
#ifndef CFG_CHALLENGE_ROUTE_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* name){++checks;if(!ok){++failures;std::printf("FAIL %s\n",name);}};
 using namespace cfgmenu;using F=RouteFixture;using Role=challengeroute::Role;
 const std::array<std::uintptr_t,5> thunks{{reinterpret_cast<std::uintptr_t>(&Thunk0),reinterpret_cast<std::uintptr_t>(&Thunk1),reinterpret_cast<std::uintptr_t>(&Thunk2),reinterpret_cast<std::uintptr_t>(&Thunk3),reinterpret_cast<std::uintptr_t>(&Thunk4)}};
 HMODULE resident=nullptr;check(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Thunk0),&resident)!=0,"resident thunk module");
 for(unsigned fault=0;fault<8;++fault){auto f=std::make_unique<F>();OptionsReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<OptionsReader,RouteEngine> route(r,engine);
  active::Context source;check(active::Observe(r,MenuFixture::Base,source),"fresh positively owned lobby source");
  check(route.Prepare(source,F::PageType,thunks),"production exact five-event + named native Push/Pop preparation");
  unsigned exchanges=0;check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;f->fixture.r.Set(at,value);++exchanges;return sandboxstart::WriteResult::Changed;}),"owned five-cell publication");
  check(exchanges==5&&route.Published(),"all five cells published before interception");
  if(fault==7){
   constexpr std::uintptr_t newType=0x1900000,newStart=0x1910000,newGraph=0x1920000;
   f->fixture.Name(newType,"SP_LobbyWidget_C");f->fixture.r.Set(newType+cfgprobe::ClassSuper,F::MenuType);
   f->GraphFunction(newGraph,newType,"ExecuteUbergraph_SP_LobbyWidget",2268,220);
   f->WrapperFunction(newStart,newType,"OnStartPressed",newGraph,0x790);
   f->fixture.r.Set(newType+cfgreflection::disk::Children,newStart);f->fixture.r.Set(newStart+cfgreflection::disk::Next,newGraph);
   f->fixture.r.Set(MenuFixture::Widget+cfgprobe::ObjectClass,newType);
   active::Context fresh;check(active::Observe(r,MenuFixture::Base,fresh),"fresh rebuilt native lobby");
   challengeroute::Route<OptionsReader,RouteEngine> next(r,engine);check(next.Prepare(fresh,F::PageType,thunks,&route),"rebuilt lobby revalidates inherited page cells without replacing foreign executors");
   unsigned added=0;check(next.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
    if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;f->fixture.r.Set(at,value);++added;return sandboxstart::WriteResult::Changed;}),"publish fresh lobby while retaining owned page cells");
   check(added==2&&next.Published(),"only two NEW source cells exchanged; three page cells adopted after full revalidation");continue;
  }
  unsigned forwards=0,commits=0;const auto forward=[&](){++forwards;};const auto commit=[&](const active::Context&){++commits;return true;};
  if(fault==1)f->fixture.r.Set(F::Frame+startdisk::FrameNode,F::PageStart);
  if(fault==2)f->fixture.r.Set(F::Frame+startdisk::FrameObject,F::Page);
  if(fault==3)engine.failPush=true;
  route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),forward,commit);
  if(fault==1||fault==2){check(forwards==1&&!engine.pushes&&!engine.starts,"wrong borrowed frame forwards original once without native redirect");continue;}
  if(fault==3){check(engine.pushes==1&&route.state==8&&!forwards&&!engine.starts,"post-Push failure never double launches stock Start");continue;}
  check(engine.pushes==1&&route.state==3&&route.Page()==F::Page&&!forwards&&!engine.starts,"X opens settings instead of intro");
  route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),forward,commit);
  check(engine.pushes==1&&!forwards&&!engine.starts,"duplicate source Start cannot bypass setup");
  if(fault==4){f->SetFrame(F::PageStart);route.Dispatch(Role::PageStart,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,commit);
   check(!commits&&!engine.pops&&!engine.starts,"building page cannot start or commit");continue;}
  route.choicesReady=true;
  if(fault==5){f->SetFrame(F::PageBack);route.Dispatch(Role::PageBack,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,commit);
   check(engine.pops==1&&!commits&&!engine.starts&&route.backs==1&&route.state==1,"Back returns source without launching or committing");continue;}
  if(fault==6)engine.delayPop=true;
  f->SetFrame(F::PageStart);route.Dispatch(Role::PageStart,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,commit);
  if(fault==6){check(route.state==5&&commits==1&&!engine.starts,"asynchronous native Pop defers original Start");
   engine.RestoreLobby();check(route.ResumeStart(),"next real source callback resumes pending Start");}
  check(engine.pops==1&&commits==1&&engine.starts==1&&route.starts==1&&route.state==6&&!forwards,"page Start commits/pops/starts original mission exactly once");
  check(!route.ResumeStart()&&engine.starts==1,"consumed continuation never starts twice");
  if(!fault){active::Context returned;check(active::Observe(r,MenuFixture::Base,returned)&&returned==route.Source(),"same lobby can survive completed native attempt");
   check(route.Rebind(returned)&&route.state==1&&!route.choicesReady&&!route.Page(),"consumed same-instance route rearms with no inherited page readiness");
   f->SetFrame(F::LobbyStart);route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),forward,commit);
   check(engine.pushes==2&&route.state==3&&route.opens==2&&engine.starts==1,"second mission opens its settings instead of bypassing consumed route");
   route.choicesReady=true;f->SetFrame(F::PageStart);route.Dispatch(Role::PageStart,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,commit);
   check(commits==2&&engine.starts==2&&route.starts==2,"second explicit settings submission starts exactly once");
  }
 }
 for(unsigned fault=0;fault<4;++fault){auto f=std::make_unique<F>();CallbackReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<CallbackReader,RouteEngine> route(r,engine);
  active::Context source;check(active::Observe(r,MenuFixture::Base,source),"fresh source for class reacquisition");
  check(route.Prepare(source,F::PageType,thunks),"original settings lease prepared");
  const auto exchange=[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;
   f->fixture.r.Set(at,value);return sandboxstart::WriteResult::Changed;};
  check(route.PublishUsing(resident,exchange),"initial lease published");
  constexpr std::uintptr_t newType=0x2000000,newStart=0x2010000,newBack=0x2020000,newGraph=0x2030000;
  f->fixture.Name(newType,"SPChallengesSettingsMenuWidget_C");f->fixture.r.Set(newType+0x20,F::Package);
  f->fixture.r.Set(newType+cfgprobe::ClassSuper,F::MenuType);
  f->GraphFunction(newGraph,newType,"ExecuteUbergraph_SPChallengesSettingsMenuWidget",128,4);
  f->WrapperFunction(newStart,newType,"OnClicked_Start",newGraph,32);f->WrapperFunction(newBack,newType,"OnClicked_Back",newGraph,64);
  f->fixture.r.Set(newType+cfgreflection::disk::Children,newStart);f->fixture.r.Set(newStart+cfgreflection::disk::Next,newBack);f->fixture.r.Set(newBack+cfgreflection::disk::Next,newGraph);
  // Old metadata is no longer usable. Must NOT dereference it before fresh class acquisition.
  f->fixture.r.Set(F::PageStart+startdisk::Executor,std::uintptr_t{});
  f->fixture.r.Set(F::PageBack+startdisk::Executor,std::uintptr_t{});
  f->fixture.r.Set(F::PageGraph+startdisk::Executor,std::uintptr_t{});
  f->fixture.r.Set(F::Page+cfgprobe::ObjectClass,newType);engine.expectedPageType=newType;
  if(fault==1)f->fixture.Name(newType,"OtherSettingsMenuWidget_C");
  if(fault==2)f->fixture.r.Set(newStart+startdisk::Executor,std::uintptr_t{0x1900000});
  r.reads=0;
  unsigned forwards=0,refreshes=0;
  route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),[&](){++forwards;},
   [](const active::Context&){return true;},[&](const active::Context&){++refreshes;return fault==3?false:route.RefreshPageUsing(newType,resident,exchange);});
  check(refreshes==1,"fresh acquisition reached BEFORE old page metadata check");
  if(fault){check(!engine.pushes&&forwards==1&&route.startGuard==7&&route.failure==2,"foreign class/executor/reacquire failure cannot push settings");continue;}
  check(engine.pushes==1&&forwards==0&&route.state==3&&route.PageType()==newType&&route.PageRefreshes()==1,
   "fresh exact class hooks replace unusable old metadata and X opens settings");
  check(r.reads<32768,"entire fresh class source callback retains shared read ceiling");
  std::printf("Fresh settings class callback reads=%u/32768 (synthetic schema, not physical peak)\n",r.reads);
  std::uintptr_t executor=0;r.Read(newStart+startdisk::Executor,executor);check(executor==thunks[2],"new page Start owns exact resident thunk");
  f->SetFrame(newBack);route.Dispatch(Role::PageBack,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),[&](){++forwards;},
   [](const active::Context&){return true;});check(route.backs==1&&route.state==1&&!engine.starts,"fresh class Back still native cancel");
 }
 // Reproduce physical61: previous widget removed; only the class remains in
 // the stack. Also retire the old widget's slot, so no old-instance read can pass.
 for(unsigned fault=0;fault<9;++fault){auto f=std::make_unique<F>();CallbackReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<CallbackReader,RouteEngine> route(r,engine);
  active::Context source;check(active::Observe(r,MenuFixture::Base,source),"fresh source for nullable previous-widget regression");
  check(route.Prepare(source,F::PageType,thunks),"nullable native lifecycle prepared");
  check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;f->fixture.r.Set(at,value);return sandboxstart::WriteResult::Changed;}),"nullable lifecycle hooks published");
  unsigned forwards=0;const auto forward=[&](){++forwards;};
  route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),forward,[](const active::Context&){return true;});
  check(route.state==3&&route.opens==1,"native null previous widget still admits the positive active settings page");
  const auto old=source.identities[2];f->fixture.r.Set(F::Registry+old.index*cfgprobe::lifetime::ItemBytes,std::uintptr_t{});
  if(fault==1)f->fixture.r.Set(F::Records+parentdisk::RecordClass,F::PageType);
  if(fault==2)f->fixture.r.Set(F::Records+execdisk::RecordWidget,MenuFixture::Widget); // dead saved instance
  if(fault==3)f->fixture.r.Set(F::Registry+route.Source().identities[3].index*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemSerialOffset,int{999});
  if(fault==4)f->fixture.r.Set(F::World+cfgprobe::WorldNetDriver,std::uintptr_t{0x2300000});
  active::Context page;r.reads=0;const bool current=route.PageCurrent(page);
  check(current==(fault==0||fault>=5),"retained class, native context and active page serial still required; dead source not read");
  if(fault>=1&&fault<=4){f->SetFrame(F::PageBack);route.Dispatch(Role::PageBack,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,[](const active::Context&){return true;});
   check(forwards==1&&engine.pops==0&&route.nativeBackFallbacks==1,"refused mod Back forwards original once, never freezes native UI");continue;}
  engine.replaceLobby=true;engine.delayPop=fault==5||fault==6;route.choicesReady=fault==6||fault==7;
  const bool start=fault==6||fault==7;
  f->SetFrame(start?F::PageStart:F::PageBack);r.reads=0;
  route.Dispatch(start?Role::PageStart:Role::PageBack,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),forward,[](const active::Context&){return true;});
  if(engine.delayPop){check(route.state==(start?5u:7u)&&engine.pops==1&&!engine.starts,"async native Pop waits for actual reconstructed lobby");engine.RestoreLobby();check(route.ResumeReturn(),"actual later owner callback resumes native return");}
  check(route.Source().widget==engine.currentLobby&&route.Source().widget!=source.widget&&route.returnGuard==0,
   "return acquires current positive lobby, never calls retired source");
  check(engine.starts==(start?1u:0u)&&route.backs==(start?0u:1u)&&forwards==0,"recreated-lobby Back cancels; Start forwards native original exactly once");
  if(!start){check(route.state==1&&!route.ResumeReturn(),"Back leaves route ready for another explicit X");}
 }
 {auto f=std::make_unique<F>();CallbackReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<CallbackReader,RouteEngine> route(r,engine);
  constexpr std::uintptr_t difficulty=0x23F0000,visibility=0x23F1000,visibilityOwner=0x23F3000,enumKind=0x23F4000;
  auto& memory=f->fixture.r;
  f->ObjectMember(difficulty,F::PageType,"DifficultyComboBox",F::PageStart,0x400);memory.Set(F::PageType+cfgreflection::disk::Children,difficulty);
  memory.Set(F::Page+0x400,MenuFixture::Donor);
  f->Root(visibilityOwner);memory.Set(MenuFixture::DonorType+cfgprobe::ClassSuper,visibilityOwner);
  const auto& binding=challengecalls::SetVisibility;f->BaseFunction(visibility,visibilityOwner,"SetVisibility",0,1,1);
  memory.Set(visibility+startdisk::Executor,MenuFixture::Base+binding.executor);memory.Set(visibility+cfgreflection::disk::FunctionFlags,binding.flags);
  memory.Set(MenuFixture::Base+binding.cache,visibility);memory.Set(MenuFixture::Base+binding.owner,visibilityOwner);memory.Set(MenuFixture::Base+binding.ctor,visibilityOwner);
  memory.Set(MenuFixture::Base+binding.executor,challengecalls::SetVisibilityCode);
  f->fixture.Name(enumKind,"EnumProperty");f->fixture.Field(visibility+0x100,visibility,enumKind,"InVisibility",0);f->fixture.Property(visibility+0x100,1,0);
  memory.Set(visibility+0x100+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0018001040000280ULL});memory.Set(visibility+cfgreflection::disk::Children,visibility+0x100);
  active::Context source;check(active::Observe(r,MenuFixture::Base,source)&&route.Prepare(source,F::PageType,thunks),"composed page-route/UI source prepared");
  check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;memory.Set(at,value);return sandboxstart::WriteResult::Changed;}),"composed route published");
  unsigned forwards=0;r.reads=0;route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),[&](){++forwards;},[](const active::Context&){return true;});
  active::Context page;r.reads=0;check(route.PageCurrent(page)&&route.state==3,"shipping null-widget stack feeds actual row setup");
  ChallengeRowsEngine rowEngine{*f};rowEngine.expectedWorld=F::Page;rowEngine.ownerThread=GetCurrentThreadId();rowEngine.visibilityFunction=visibility;
  auto text=std::make_unique<reflection::RetainedMenuText>();rows::NativeSetup<CallbackReader,ChallengeRowsEngine> view(r,rowEngine,*text);
  std::array<rows::Spec,5> specs{};constexpr std::array<RowId,5> ids{{RowId::Knives,RowId::Recharge,RowId::Movement,RowId::Stealth,RowId::Untimed}};
  for(unsigned i=0;i<5;++i){specs[i].id=ids[i];specs[i].title=u"Challenge option";specs[i].count=2;specs[i].labels[0]=u"Off";specs[i].labels[1]=u"On";specs[i].preserveNativeValue=true;}
  r.reads=0;check(view.OpenForClass(page,1,specs.data(),5,true),"composed native row provider opens current page");
  unsigned peak=r.reads;for(unsigned i=0;i<32;++i){r.reads=0;check(route.PageCurrent(page),"each UI phase retains native suspended class proof");view.Service(25*i);peak=(std::max)(peak,r.reads);}
  check(view.State()==rows::ViewState::Ready&&rowEngine.creates==5&&rowEngine.texts==5,"all five rows become ready after native source eviction");
  r.reads=0;check(view.HideDifficulty()&&rowEngine.visibilityCalls==1,"actual pinned native visibility packet hides old difficulty row");
  peak=(std::max)(peak,r.reads);check(peak<32768,"composed route+row callback keeps shared32768 limit");
  std::printf("Composed evicting-stack route/UI peak=%u/32768; all five rows ready and old difficulty collapsed (owned engine)\n",peak);
 }
 // A returned page may be present even when native post-output admission
 // refused. Adopt only the proved current native page, never replay Push.
 for(unsigned recovered=0;recovered<3;++recovered){auto f=std::make_unique<F>();OptionsReader r{f->fixture.r};RouteEngine engine{*f};
  engine.refusedAfterPush=true;engine.nullAfterPush=recovered==1;engine.wrongPageAfterPush=recovered==2;
  challengeroute::Route<OptionsReader,RouteEngine> route(r,engine);active::Context source;
  check(active::Observe(r,MenuFixture::Base,source)&&route.Prepare(source,F::PageType,thunks),"recovery fixture prepared");
  check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;f->fixture.r.Set(at,value);return sandboxstart::WriteResult::Changed;}),"recovery fixture published");
  unsigned forwards=0;route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),
   [&](){++forwards;},[](const active::Context&){return true;});
  check(recovered==0&&CFG_J25_CHALLENGE_PUSH_OUTPUTS?(route.state==3&&route.Page()==F::Page&&route.recoveredPushes==1&&route.opens==1):
   (route.state==8&&route.recoveredPushes==0),"adopt only positively proven returned page");
  route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),
   [&](){++forwards;},[](const active::Context&){return true;});
  check(engine.pushes==1&&engine.starts==0&&forwards==0,"ambiguous push never replayed or forwarded");
 }
 // Reproduces the post-mission settings scheduling gap: no callback from the
 // retired HUD/event is delivered. Only completed current owner callbacks are
 // supplied, as in FrontendAfterSafe before its OfflinePlay-only epoch gate.
 {auto f=std::make_unique<F>();CallbackReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<CallbackReader,RouteEngine> route(r,engine);
  auto& memory=f->fixture.r;constexpr std::uintptr_t difficulty=0x23F0000;
  f->ObjectMember(difficulty,F::PageType,"DifficultyComboBox",F::PageStart,0x400);memory.Set(F::PageType+cfgreflection::disk::Children,difficulty);
  memory.Set(F::Page+0x400,MenuFixture::Donor);memory.Set(MenuFixture::Donor+0x188,NativeFixture::Title);
  memory.Set(NativeFixture::Title+cfgprobe::ObjectClass,NativeFixture::TextType);
  active::Context source;check(active::Observe(r,MenuFixture::Base,source)&&route.Prepare(source,F::PageType,thunks),"post-mission current route prepared independently of retired HUD/event");
  check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;
   if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;memory.Set(at,value);return sandboxstart::WriteResult::Changed;}),"post-mission route publication retained");
  unsigned forwards=0;r.reads=0;route.Dispatch(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),[&](){++forwards;},[](const active::Context&){return true;});
  ChallengeRowsEngine rowEngine{*f};rowEngine.expectedWorld=F::Page;rowEngine.ownerThread=GetCurrentThreadId();
  auto text=std::make_unique<reflection::RetainedMenuText>();rows::NativeSetup<CallbackReader,ChallengeRowsEngine> view(r,rowEngine,*text);
  challengeservice::Service service;active::Context viewContext;unsigned phases=0,peak=0;
  const auto work=[&](const active::Context& current,std::uint64_t now){++phases;
   if(view.State()==rows::ViewState::Closed){const auto specs=challengemenu::Specs();
    check(view.OpenForClass(current,1,specs.data(),static_cast<unsigned>(specs.size()),true),"current eight-row setup opens without old HUD callback");viewContext=current;
   }else if(!(viewContext==current)){check(view.Rebind(current),"cached same positive tree rebinds current stack");viewContext=current;}
   view.Service(now);route.choicesReady=view.State()==rows::ViewState::Ready;
  };
  const auto after=[&](std::uint64_t now,bool nested=false,bool completed=true,unsigned thread=GetCurrentThreadId()){
   r.reads=0;const bool result=service.After(route,thread,nested,completed,now,work);peak=(std::max)(peak,r.reads);return result;};
  check(!after(0,true)&&!r.reads&&!phases,"nested scheduling performs no reads or row work");
  check(!after(0,false,false)&&!r.reads&&!phases,"unfinished callbacks cannot schedule native phases");
  check(!after(0,false,true,GetCurrentThreadId()+1)&&!r.reads&&!phases,"foreign thread cannot schedule native phases");
  for(unsigned i=0;i<48;++i)check(after(25ULL*i),"completed owner callback services independently of old HUD/event");
  check(view.State()==rows::ViewState::Ready&&rowEngine.creates==7&&rowEngine.texts==8&&route.choicesReady,
   "eight actual native rows reach Ready on rebuilt frontend scheduling");
  check(challengeservice::OwnsView(route,viewContext),"report accepts only active route page view");
  auto stale=viewContext;stale.world+=0x1000;check(!challengeservice::OwnsView(route,stale),"old-world cached Ready is not current rows");
  stale=viewContext;stale.identities[2].serial+=1;check(!challengeservice::OwnsView(route,stale),"recycled widget cached Ready is not current rows");
  stale=viewContext;stale.type+=0x1000;check(!challengeservice::OwnsView(route,stale),"changed class cached Ready is not current rows");
  const auto calls=rowEngine.calls,phasesBefore=phases;
  check(!after(1176)&&!r.reads&&phases==phasesBefore,"independent HUD and owner scheduling share one25ms pace");
  check(after(1200)&&rowEngine.calls==calls&&rowEngine.creates==7,"ready page validation never duplicates native rows");
  // A relocated native stack retains the positive page and its rows. Free the
  // old fixture storage to catch stale-array reads rather than tolerating them.
  constexpr std::uintptr_t newRecords=0x27F0000;
  for(unsigned i=0;i<2*execdisk::RecordBytes;++i){const auto it=memory.bytes.find(NativeFixture::Records+i);
   if(it!=memory.bytes.end())memory.bytes[newRecords+i]=it->second;}
  memory.Set(NativeFixture::Stack+execdisk::StackArray,active::StackArray{newRecords,2,2});
  for(unsigned i=0;i<2*execdisk::RecordBytes;++i)memory.bytes.erase(NativeFixture::Records+i);
  check(after(1225)&&viewContext.array.data==newRecords&&rowEngine.calls==calls&&route.choicesReady,
   "stack relocation rebinds same rows without old storage reads or duplicate calls");
  // PageCurrent remains the mutation authority even though scheduling accepts
  // arbitrary completed owner callbacks. Refusals must clear Start readiness.
  const auto pageSerial=F::Registry+24ULL*static_cast<unsigned>(viewContext.identities[2].index)+16;
  const auto hudSerial=F::Registry+24ULL*static_cast<unsigned>(viewContext.identities[3].index)+16;
  for(unsigned fault=0;fault<7;++fault){
   if(fault==0)memory.Set(F::World+cfgprobe::WorldNetDriver,F::Mode);
   if(fault==1)memory.Set(pageSerial,int{999});
   if(fault==2)memory.Set(newRecords+execdisk::RecordWidget,F::Page); // Wrong suspended record widget.
   if(fault==3)memory.Set(F::Page+cfgprobe::ObjectClass,MenuFixture::Type);
   if(fault==4)memory.Set(hudSerial,int{999});
   if(fault==5)memory.Set(newRecords+parentdisk::RecordClass,F::PageType);
   if(fault==6)r.cap=16;
   route.choicesReady=true;const auto beforePhases=phases;
   check(!after(1250+25ULL*fault)&&!service.current&&!route.choicesReady&&phases==beforePhases&&rowEngine.calls==calls,
    "online/recycled/foreign stack/class/HUD/budget refuses row work and clears Start readiness");
   memory.Set(F::World+cfgprobe::WorldNetDriver,std::uintptr_t{});memory.Set(pageSerial,viewContext.identities[2].serial);
   memory.Set(newRecords+execdisk::RecordWidget,std::uintptr_t{});memory.Set(F::Page+cfgprobe::ObjectClass,F::PageType);
   memory.Set(hudSerial,viewContext.identities[3].serial);memory.Set(newRecords+parentdisk::RecordClass,MenuFixture::Type);r.cap=32768;
  }
  check(service.refusals==7&&after(1450)&&service.current&&route.choicesReady&&rowEngine.calls==calls,"only a fresh complete page proof resumes cached rows after refusal");
  const auto oldState=route.state;route.state=6;
  check(!after(1475)&&!r.reads&&!challengeservice::OwnsView(route,viewContext),"consumed mission route neither services rows nor reports cached Ready");route.state=oldState;
  check(!after(UINT64_MAX)&&!r.reads,"deadline overflow refuses scheduling");
  check(peak<32768,"current route plus seven-row service stays within original shared read cap");
  std::printf("Independent Challenge settings scheduling: services=%u refused=%u peak=%u/32768 creates=%u texts=%u (owned engine)\n",service.services,service.refusals,peak,rowEngine.creates,rowEngine.texts);
 }
 std::printf("Challenge production Start/Back/frame/native-packet routing: %u checks, %u failures. Synthetic engine only.\n",checks,failures);return failures?1:0;
}

#endif
