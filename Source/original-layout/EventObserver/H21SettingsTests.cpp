#define CFG_J215_HUD_TEST_MAIN ExistingHudChecks
#include "J215PracticeObservationTests.cpp"
#include "HostedLobbyDiscovery.hpp"
struct LobbyFixture:WidgetFixture {
 static constexpr std::uintptr_t Hud=0x2900000,HudType=0x2910000,Stack=0x2920000,Records=0x2930000,
  MenuType=0x2940000,UserType=0x2950000,Driver=0x2960000,DriverType=0x2970000,TextType=0x2980000,StateType=0x2990000;
 void Pair(std::uintptr_t cache,std::uintptr_t type){r.Set(Module+cache,std::array<std::uintptr_t,2>{{type,type}});r.Set(type+cfgprobe::ClassSuper,std::uintptr_t{});}
 LobbyFixture(){namespace d=cfghostedlobby::disk;using namespace cfgprobe;
  Slot(Hud,24,124);Slot(Stack,25,125);Slot(Driver,26,126);Slot(PS,27,127);Slot(Mode,28,128);Slot(Controller,29,129);Slot(hud.widget.object,20,0);
  r.Set(Module+lifetime::ObjectArrayRva+28,int{32});
  Pair(d::ModeReflection,ModeType);Pair(d::HudReflection,HudType);Pair(d::MenuReflection,MenuType);
  Pair(d::PlayerStateReflection,StateType);Pair(d::TextBlockReflection,TextType);Pair(cfghosted::disk35::NetDriverReflection,DriverType);
  Pair(cfgpractice::disk::UserWidgetReflection,UserType);r.Set(MenuType+ClassSuper,UserType);r.Set(UserType+ClassSuper,WidgetType);r.Set(TextType+ClassSuper,WidgetType);
  r.Set(Hud+ObjectClass,HudType);r.Set(Driver+ObjectClass,DriverType);r.Set(PS+ObjectClass,StateType);r.Set(hud.widget.object+ObjectClass,MenuType);
  r.Set(Module+dispatch::ThreadFlagRva,std::uint8_t{1});r.Set(Module+dispatch::ThreadIdRva,std::uint32_t{42});
  r.Set(Module+EngineGlobalCandidate,Engine);r.Set(Engine+EngineGameViewport,Viewport);r.Set(Viewport+ViewportWorld,World);
  r.Set(World+WorldGameMode,Mode);r.Set(World+WorldGameInstance,Instance);
  r.Set(Instance+GameInstanceLocalPlayers,cfghosted::listen35::LocalPlayers{Records+0x100,1,1});r.Set(Records+0x100,Player);
  r.Set(Player+PlayerController,Controller);r.Set(Controller+cfghosted::disk35::ControllerPlayer,Player);
  r.Set(Controller+cfgmenu::execdisk::ControllerHUD,Hud);r.Set(Hud+0x368,Controller);r.Set(Hud+d::HudStack,Stack);r.Set(Stack+0x20,Hud);
  r.Set(Stack+d::StackArray,cfgmenu::active::StackArray{Records,1,1});r.Set(Records+d::RecordWidget,hud.widget.object);
  r.Set(Controller+cfgescape::disk::PlayerState,PS);r.Set(PS+0x88,Controller);r.Set(PS+d::HostByte,std::uint8_t{1});
  r.Set(World+WorldNetDriver,Driver);r.Set(Driver+cfghosted::disk35::DriverWorld,World);r.Set(Driver+cfghosted::disk35::ServerConnection,std::uintptr_t{});
  r.Set(Mode+cfghosted::disk35::ActorRole,std::uint8_t{3});r.Set(Controller+cfghosted::disk35::ActorRole,std::uint8_t{3});
  r.Set(Module+d::GetCurrentMenu,d::GetCurrentMenuCode);r.Set(Module+d::TextToString,d::TextToStringCode);
  Label(MenuType,"PrivateLobby_C");Label(Root,"RightPanel");Label(Child,"WeatherTitle");Label(PanelType,"VerticalBox");Label(TextType,"TextBlock");
  r.Set(Child+ObjectClass,TextType);r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(id),1}});
 }
 void Text(){constexpr std::uintptr_t data=0x2A00000,table=0x2A10000,str=0x2A20000;
  r.Set(Child+cfghostedlobby::disk::AuthoredText,data);r.Set(data,table);r.Set(table+0x10,Module+0x1000);
  r.Set(Module+0x1000,std::array<std::uint8_t,5>{{0x48,0x8D,0x41,0x20,0xC3}});
  struct String{std::uintptr_t data;int count,capacity;};r.Set(data+0x20,String{str,8,8});r.Set(str,std::array<char16_t,8>{{u'W',u'e',u'a',u't',u'h',u'e',u'r',0}});
 }
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool good,const char* label){++checks;if(!good){++failures;std::printf("FAIL %s\n",label);}};
 using namespace cfghostedlobby;
 for(unsigned fault=0;fault<8;++fault){LobbyFixture f;Context c;Failure why;
  if(fault==1)f.r.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,std::uint32_t{99});
  if(fault==2)f.r.Set(f.Records+disk::RecordWidget,f.Child);
  if(fault==3)f.r.Set(f.Hud+0x368,f.Player);
  if(fault==4)f.r.Set(f.Driver+cfghosted::disk35::ServerConnection,f.Player);
  if(fault==5)f.r.Set(f.PS+disk::HostByte,std::uint8_t{});
  if(fault==6)f.r.Set(f.Mode+cfghosted::disk35::ActorRole,std::uint8_t{1});
  if(fault==7)f.r.Set(f.Registry+24*20+16,int{99});
  const bool got=Observe(f.r,f.Module,42,c,why);
  check(got==(fault==0||fault>=4),"only current menu/thread/owner accepted for read-only observation");
  if(got)check(c.listenHost==(fault==0||fault==7),"client/nonhost roles never permit placement");
 }
 for(const char* name:{"PrivateLobby_C","PublicLobby_C","UnclassifiedLobby_C"}){LobbyFixture f;f.Label(f.MenuType,name);f.r.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{static_cast<int>(f.id),1}});f.Text();
  Context c;Failure why;check(Observe(f.r,f.Module,42,c,why)&&c.listenHost,"public and private same listen-host proof");
  Discovery discovery;Report report;auto& s=report.history[0];check(discovery.Begin(f.r,c,s),"current native lobby tree begins");
  for(unsigned n=0;n<4&&!s.complete;++n){check(discovery.Step(f.r,s,report),"bounded strong tree walk");}
  check(s.complete&&s.count==2&&s.cursor==2&&s.rowsPlaced==0&&s.placementRefusal==unsigned(Failure::NoNativeLease),"dump complete, missing lease refuses all writes");
  check(std::string_view(s.nodes[1].authoredText.data())=="Weather","read-only resident FText leaf");
  check(s.lobbyType==(name[0]=='P'?(name[1]=='r'?1u:2u):0u),"type is name hint only, unknown remains unknown");
 }
 {LobbyFixture f;f.r.Set(f.hud.widget.object+cfgprobe::ObjectClass,f.UserType);Context c;Failure why;
  check(Observe(f.r,f.Module,42,c,why)&&c.listenHost,"alternate public UserWidget page is read-only under same typed lobby HUD");
  f.r.Set(f.PS+disk::HostByte,std::uint8_t{});check(Observe(f.r,f.Module,42,c,why)&&!c.listenHost,"non-host diagnostic context");
  Discovery discovery;Snapshot s;check(discovery.Begin(f.r,c,s)&&s.complete&&!s.count&&s.placementRefusal==unsigned(Failure::NoHost),"non-host tree not traversed");
 }
 for(unsigned fault=0;fault<4;++fault){LobbyFixture f;f.Text();Context c;Failure why;check(Observe(f.r,f.Module,42,c,why),"text context");
  if(fault==1)f.r.Set(f.Module+0x1000,std::uint8_t{0xE9});
  if(fault==2)f.r.Set(0x2A00000+0x20+8,int{129});
  if(fault==3)f.r.Set(f.Child+disk::AuthoredText,std::uintptr_t{});
  std::array<char,72> text{};check(AuthoredText(f.r,c,f.Child,text)==(fault==0),"unknown getter, oversized/null text refused without calls");
 }
 Report a,b;check(Accept(a,b),"fixed report header");a.histories=3;check(!Accept(a,b),"report history bound");
 std::printf("H21 lobby current-tree/read-only text, public/private authority, placement refusal: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
