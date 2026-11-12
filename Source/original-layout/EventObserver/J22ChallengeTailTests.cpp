#define CFG_HUD_FIXTURE_ONLY
#include "FrontendHudObservationTests.cpp"
#undef CFG_HUD_FIXTURE_ONLY
#include "NativeChallengeMenuTransition.hpp"
#include "CallbackRootLease.hpp"
struct TailRoots {
 HudFixture& f;OptionsReader& r;const cfgmenu::hud::Binding& binding;bool held=false,returned=false;
 cfgretain::ResidentRootBank<2> bank;cfgretain::CallbackRootLease<decltype(bank),64> lease{bank,MenuFixture::Base};
 cfggcretain::ReferencerList list{0x10000,0,64};std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
 bool Current(){return returned?cfgmenu::challengetransition::ReturnedStartCurrent(r,binding,binding.context.hud,binding.event,
  MenuFixture::Base+cfgmenu::transportdisk::ReturnPc,binding.context.thread):cfgmenu::hud::Current(r,binding,binding.context.thread);}
 bool ProvenSlotWindow(){return held&&Current();}bool NativeEntryUnlocked()const{return !held;}
 bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values){out=list;values=entries;return true;}
 bool Register(std::uintptr_t address){entries[static_cast<unsigned>(list.count++)]=address;return true;}
};
int main(){unsigned checks=0,fail=0;const auto check=[&](bool v){++checks;if(!v){++fail;std::printf("FAIL tail%u\n",checks);}};
 auto f=std::make_unique<HudFixture>();OptionsReader r{f->fixture.r};cfgmenu::active::Context context;
 check(cfgmenu::active::Observe(r,MenuFixture::Base,context));cfgmenu::hud::Binding binding;
 check(cfgmenu::hud::Observe(r,context,true,binding));
 f->fixture.r.Set(MenuFixture::Base+cfgprimary::SerializationFlag,std::uint8_t{});
 f->fixture.r.Set(MenuFixture::Base+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
 for(unsigned i=0;i<f->slotCount;++i)f->fixture.r.Set(NativeFixture::Registry+24*i+8,std::uint32_t{});
 TailRoots h{*f,r,binding};h.held=true;
 std::array<std::uintptr_t,4> owners{};for(unsigned i=0;i<4;++i)owners[i]=context.identities[i].object;
 check(h.lease.Retain(h,owners));check(h.lease.PrepareCall(h,0));h.held=false;check(h.lease.EnterNative(h));
 // OWN model of normal Start returning after the menu empties and travel queues.
 f->fixture.r.Set(NativeFixture::Stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{NativeFixture::Records,0,2});
 f->fixture.r.Set(NativeFixture::Viewport+cfgprobe::ViewportWorld,std::uintptr_t{0x7F00000});h.returned=true;
 check(!cfgmenu::hud::Current(r,binding,context.thread));check(h.Current());
 for(unsigned fault=0;fault<4;++fault){
  auto bad=binding;if(fault==0)bad.context.identities[0].serial++;
  if(fault==1)bad.context.thread++;if(fault==2)bad.vtable+=8;if(fault==3)bad.event+=8;
  check(!cfgmenu::challengetransition::ReturnedStartCurrent(r,bad,context.hud,binding.event,MenuFixture::Base+cfgmenu::transportdisk::ReturnPc,context.thread));}
 h.held=true;std::array<std::uintptr_t,0> outputs{};check(h.lease.CompleteCall(h,outputs,0));check(h.lease.Close(h)&&!h.bank.Poisoned());
 check(!h.lease.PrepareCall(h,0));h.held=false;check(!h.lease.EnterNative(h));
 std::printf("J22 returned Start empty-menu/travel cleanup: %u checks,%u failures; actual root lease closes, no poison/new input, stale slots/thread/HUD event refused. Owned memory only.\n",checks,fail);return fail?1:0;}
