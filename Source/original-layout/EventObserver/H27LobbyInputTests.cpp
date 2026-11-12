#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "HostedLobbyStaticInputs.hpp"
#include "HostedLobbyLayout.hpp"
#include "CallbackRootLease.hpp"
#include "BundleFrameInputObservation.hpp"
#include <windows.h>
// Native metadata/slot proofs and the production provider/lease are real.
// Only completed-callback, current-lobby and collector locks are owned stubs.
namespace {
NativeFixture* memory=nullptr;const void* ownedPacket=nullptr;std::size_t frameBytes=0;
bool lobbyCurrent=true,callbackComplete=true,windowHeld=false;
struct Reader {
 unsigned reads=0,cap=32768;
 template<class T>bool Read(std::uintptr_t at,T& out){return ReadBytes(at,&out,sizeof(out));}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){
  if(++reads>cap||!memory)return false;
  const auto start=reinterpret_cast<std::uintptr_t>(ownedPacket);
  if(ownedPacket&&at>=start&&at-start<=frameBytes&&bytes<=frameBytes-(at-start)){std::memcpy(out,reinterpret_cast<const void*>(at),bytes);return true;}
  OptionsReader source{memory->fixture.r};return source.ReadBytes(at,out,bytes);
 }
};
}
namespace cfgprimary {inline bool LookupVetoClearForObservation(Reader&,std::uintptr_t){return lobbyCurrent;}}
namespace cfgtick {inline bool Observe(Reader&,std::uintptr_t,std::uintptr_t actor,std::uintptr_t fn,std::uintptr_t pc){
 return callbackComplete&&actor==NativeFixture::Mode&&fn==0x810000&&pc==0x820000;}}
namespace cfghostedlobby {inline bool LayoutCurrent(Reader&,const Discovery& d,const RowLayout& l){
 return lobbyCurrent&&l.world==d.context.world&&l.menu==d.context.menu;}}
namespace {
struct RegistryState {cfggcretain::ReferencerList list{0x10000,0,64};std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};} roots;
class GcRegistryAccess {
public:
 struct NativeContextProof {void* context=nullptr;bool(*current)(Reader&,void*)=nullptr;bool(*completed)(Reader&,void*,std::uintptr_t,std::uintptr_t)=nullptr;};
private:
 NativeContextProof proof_;Reader reader_;bool held_=false;
 bool Current(){return proof_.context&&proof_.current&&proof_.current(reader_,proof_.context);}
public:
 explicit GcRegistryAccess(NativeContextProof proof,unsigned limit=32768):proof_(proof){reader_.cap=limit;}
 bool Begin(std::uintptr_t f,std::uintptr_t pc){if(windowHeld||!Current()||!proof_.completed(reader_,proof_.context,f,pc))return false;held_=windowHeld=true;return true;}
 void End(){if(held_)held_=windowHeld=false;}
 bool ProvenSlotWindow(){return held_&&windowHeld&&Current();}
 bool NativeEntryUnlocked(){return !windowHeld&&Current();}
 bool Snapshot(cfggcretain::ReferencerList& list,decltype(roots.entries)& entries){if(!ProvenSlotWindow())return false;list=roots.list;entries=roots.entries;return true;}
 bool Register(std::uintptr_t owner){if(!ProvenSlotWindow())return false;roots.entries[static_cast<unsigned>(roots.list.count++)]=owner;return true;}
};
cfgretain::ResidentRootBank<32> hostedLobbyRootBank;
#include "HostedLobbyRetainedRuntime.hpp"
}
#ifndef CFG_H27_LOBBY_INPUT_FIXTURE_ONLY
int main(){using F=NativeFixture;using M=MenuFixture;using Scope=cfgmenu::rows::HostedLobbyRetentionScope;
 unsigned n=0,bad=0;const auto check=[&](bool okay,const char* why){++n;if(!okay){++bad;std::printf("FAIL %s\n",why);}};
 // Serial-change is last: refusal after rooting must poison instead of repair/replay.
 for(const unsigned fault:std::array<unsigned,14>{0,1,2,3,4,5,6,8,9,10,11,12,13,7}){NativeFixture f;memory=&f;using namespace cfgprobe;
  const auto thread=GetCurrentThreadId();const auto cdo=F::CreateOwner+0x800;
  for(auto value:{F::World,F::Mode,F::HUD,F::Stack,F::Controller,M::Widget,F::Panel,M::Donor,M::DonorType})f.Register(value,100+static_cast<int>(f.slotCount));
  f.Register(cdo,0);f.Register(F::CreateOwner,0); // function already has native serial0 in NativeFixture
  for(unsigned i=0;i<f.slotCount;++i){std::uintptr_t value=0;f.fixture.r.Read(F::Registry+24ULL*i,value);
   f.fixture.r.Set(F::Registry+24ULL*i+8,std::uint32_t{});f.fixture.r.Set(value+8,std::uint32_t{});}
  cfgmenu::reflection::CreateParameters packet;packet.world=F::Controller;packet.player=F::Controller;packet.type=M::DonorType;
  ownedPacket=&packet;frameBytes=sizeof(packet);lobbyCurrent=callbackComplete=true;
  cfghostedlobby::Discovery discovery;auto& c=discovery.context;c.module=M::Base;c.thread=thread;c.world=F::World;c.mode=F::Mode;
  c.gameState=F::World;c.controller=F::Controller;c.hud=F::HUD;c.stack=F::Stack;c.menu=M::Widget;
  cfghostedlobby::RowLayout layout{F::World,M::Widget,M::DonorType,F::Panel,M::Donor,1,1};
  const auto slotOf=[&](std::uintptr_t value){int index=-1;f.fixture.r.Read(value+12,index);return F::Registry+24ULL*static_cast<unsigned>(index);};
  f.fixture.r.Set(cdo+8,std::uint32_t{0x31});f.fixture.r.Set(slotOf(cdo)+8,std::uint32_t{0x40000000});
  if(fault==1)f.fixture.r.Set(slotOf(cdo)+8,std::uint32_t{0x20000000});
  if(fault==2)f.fixture.r.Set(f.createFunction+8,std::uint32_t{0x20});
  if(fault==3)f.fixture.r.Set(slotOf(M::Donor)+16,int{});
  if(fault==4)f.fixture.r.Set(slotOf(F::Panel)+16,int{});
  if(fault==5)f.fixture.r.Set(M::Base+cfgmenu::execdisk::Create.executor,std::uint8_t{});
  const auto stackType=std::uintptr_t{0x1D00000};f.Root(stackType);f.Register(stackType,250);
  f.fixture.r.Set(M::Base+cfgcompat::cfg35::menuStack::OwnerRva,stackType);
  f.fixture.r.Set(M::Base+cfgcompat::cfg35::menuStack::ConstructorRva,stackType);
  f.fixture.r.Set(F::Stack+cfgprobe::ObjectClass,stackType);f.fixture.r.Set(F::Stack+0x20,F::HUD);
  if(fault>=8){f.fixture.r.Set(slotOf(F::Stack)+16,int{});f.fixture.r.Set(F::Stack+8,std::uint32_t{0x40048});}
  if(fault==9)f.fixture.r.Set(F::Stack+0x20,F::World);
  if(fault==10)f.fixture.r.Set(M::Base+cfgcompat::cfg35::menuStack::ConstructorRva,F::World);
  if(fault>=11){
   f.fixture.Name(F::Panel,fault==12?"ForeignPanel":"CustomServerSettings");f.fixture.Name(F::PanelType,"VerticalBox");
   f.fixture.r.Set(M::Base+cfgpractice::disk::PanelWidgetReflection,F::PanelType);
   f.fixture.r.Set(slotOf(F::Panel)+16,int{});f.fixture.r.Set(F::Panel+8,std::uint32_t{0xA80048});
   if(fault==13)f.fixture.r.Set(M::Base+cfgpractice::disk::PanelWidgetReflection,F::World);
  }

  {
   HostedLobbyRootProvider provider(discovery,layout,F::Mode,0x810000,0x820000);Scope scope(M::Base,provider.Hooks(),provider.Admitted());
   check(scope.Entered()&&!cfgmenu::rows::NativeRetentionScope::Occupied(),"own admitted scope, no offline scope");
   if(fault==6)lobbyCurrent=false;
   const bool admitted=Scope::Inputs(M::Base,cdo,f.createFunction,&packet);
   check(admitted==(fault==0||fault==7||fault==8||fault==11),"only exact registered static/owned stack/authored panel admit serial0");
   if(!admitted)check(provider.Stage()!=0&&provider.Failure().reason&&Scope::InputFailure(),"every refusal has scope/provider reason");
   if(fault==3||fault==4)check(provider.Failure().reason==4&&provider.Failure().slot.serial==0,"widget/donor/panel stay positive serial");
   if(fault==0){int serial=-1;f.fixture.r.Read(slotOf(cdo)+16,serial);check(serial==0,"no CDO serial assigned");
    f.fixture.r.Read(slotOf(f.createFunction)+16,serial);check(serial==0,"no UFunction serial assigned");
    check(Scope::EnterNative(M::Base)&&!windowHeld,"native entry outside registry lock");
    packet.result=0;check(Scope::Outputs(M::Base,f.createFunction,&packet),"zero returned object completes exact output reservation");
   }
   if(fault==8){check(provider.Failure().reason==0,"typed HUD-owned serial0 ILL stack admitted");
    check(Scope::EnterNative(M::Base)&&!windowHeld,"strong stack retained before native entry");
    packet.result=0;check(Scope::Outputs(M::Base,f.createFunction,&packet),"strong stack reproof on outputs");}
   if(fault==11){check(Scope::EnterNative(M::Base)&&!windowHeld,"named current CustomServerSettings panel retained");
    packet.result=0;check(Scope::Outputs(M::Base,f.createFunction,&packet),"panel strong-chain reproof on output");}
   if(fault==12||fault==13)check(provider.Failure().reason==4,"foreign name/type cannot borrow panel exception");
   if(fault==9||fault==10)check(provider.Failure().reason&&provider.Owners()[5].value==F::Stack,"all-owner snapshot preserved on foreign stack/type refusal");
   if(fault==7){f.fixture.r.Set(slotOf(f.createFunction)+16,int{999});
    check(!Scope::EnterNative(M::Base)&&provider.Stage()!=0,"later serial change refuses, never fabricates/repairs");}
  }
  check(!windowHeld,"every collector window closed");
 }
 memory=nullptr;ownedPacket=nullptr;
 std::printf("H27 actual lobby provider + registered serial0 native static inputs: %u checks, %u failures; current lobby/callback/collector simulated, no game/native calls.\n",n,bad);
 return bad?1:0;
}
#endif
