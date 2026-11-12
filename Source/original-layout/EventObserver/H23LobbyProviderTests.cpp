#define CFG_H22_SETTINGS_MAIN PriorLobbyDiscoveryChecks
#include "H22SettingsTests.cpp"
#include "HostedLobbyLayout.hpp"
#include "HostedLobbyStaticInputs.hpp"
#include "HostedInputPremise.hpp"
#include "BundleFrameInputObservation.hpp"
namespace {
cfgretain::ResidentRootBank<32> hostedLobbyRootBank;
#include "HostedLobbyRetainedRuntime.hpp"
}
#ifndef CFG_H23_LOBBY_PROVIDER_MAIN
#define CFG_H23_LOBBY_PROVIDER_MAIN main
#endif
int CFG_H23_LOBBY_PROVIDER_MAIN(){unsigned failures=0,checks=0;const auto check=[&](bool good,const char* why){++checks;if(!good){++failures;std::printf("FAIL %s\n",why);}};
 using namespace cfghostedlobby;
 for(unsigned fault=0;fault<8;++fault){
  LobbyFixture f;f.Slot(f.World,0,100);f.Slot(f.hud.widget.object,20,120);f.Slot(f.Child,17,131);f.Slot(f.Root,16,116);f.Slot(f.UserType,31,132);f.r.Set(f.Controller+0x20,f.World);f.r.Set(f.Child+cfgprobe::ObjectClass,f.UserType);
  for(auto& node:f.graph.nodes){if(node.widget.object==f.Child)node.widget.serial=131;if(node.widget.object==f.Root)node.widget.serial=116;if(node.widget.object==f.hud.widget.object)node.widget.serial=120;}
  const auto thread=GetCurrentThreadId();f.r.Set(f.Module+cfgprobe::dispatch::ThreadIdRva,thread);
  ChallengeReader memory;static_cast<StageReader&>(memory)=f.r;memory.cap=1000000;active=&memory;
  Context c;Failure why;check(Observe(memory,f.Module,thread,c,why)&&c.listenHost,"fixture is a current host lobby");
  Discovery discovery;Snapshot snapshot;check(discovery.Begin(memory,c,snapshot),"begin native path");
  // This owned fixture's one native row is the first child under VerticalBox.
  discovery.graph=f.graph;discovery.context=c;
  RowLayout layout{f.World,c.menu,f.UserType,f.Root,f.Child,1,1};
  check(LayoutCurrent(memory,discovery,layout),"exact current anchor/panel path");
  if(fault==1)memory.Set(f.Root+cfgpractice::disk::Slots,cfgpracticeobs::WidgetNode::List{f.Slots+0x100,2,2});
  if(fault==1){memory.Set(f.Slots+0x100,f.SlotObject);memory.Set(f.Slots+0x108,f.Tree);}
  if(fault==2)memory.Set(f.Driver+cfghosted::disk35::ServerConnection,f.Player);
  if(fault==3)memory.Set(f.Slots,f.Tree);
  if(fault==4)memory.Set(f.SlotObject+cfgpractice::disk::Content,f.Root);
  if(fault==5)memory.Set(f.Records+disk::RecordWidget,f.Child);
  callbackComplete=fault!=6;
  if(fault==7)memory.Set(f.Child+8,std::uint32_t{0x20});
  {
   HostedLobbyRootProvider provider(discovery,layout,f.Mode,0x810000,0x820000);
   cfgmenu::rows::HostedLobbyRetentionScope scope(f.Module,provider.Hooks(),provider.Admitted());
   const bool expected=fault<2;
   check(scope.Entered()==expected,"only current host path and completed callback admit");
   if(expected){
    const bool input=cfgmenu::rows::HostedLobbyRetentionScope::FactoryInput(f.Module,f.Module+0x1000,f.Mode);
    if(!input)std::printf("input fault%u reads%u owners%u poison%d held%d\n",fault,memory.count,hostedLobbyRootBank.Owners(),hostedLobbyRootBank.Poisoned(),windowHeld);
    check(input,"independent registered input roots");
    check(cfgmenu::rows::HostedLobbyRetentionScope::EnterNative(f.Module)&&!windowHeld,"native entry unlocks registry");
    check(cfgmenu::rows::HostedLobbyRetentionScope::FactoryOutput(f.Module,f.Child),"positive widget return retained as exact registered return");
    check(hostedLobbyRootBank.Healthy(),"healthy dedicated bank");
   }
  }
  check(!windowHeld&&hostedLobbyRootBank.Healthy(),"provider teardown closes windows/claims");
 }
 for(unsigned fault=0;fault<4;++fault){LobbyFixture f;
  if(fault==1)f.r.Set(f.Driver+cfghosted::disk35::ServerConnection,f.Player);
  if(fault==2)f.r.Set(f.Controller+cfghosted::disk35::ActorRole,std::uint8_t{2});
  if(fault==3)f.r.Set(f.Controller+cfghosted::disk35::ControllerPlayer,f.Root);
  check(InputHost(f.r,f.Module,42)==(fault==0),"preference premise excludes client/foreign controller");
 }
 active=nullptr;std::printf("H23 actual host-lobby provider + real callback root lease + live-sized anchor relocation: %u checks, %u failures; registry/native platform simulated.\n",checks,failures);
 return failures?1:0;
}
