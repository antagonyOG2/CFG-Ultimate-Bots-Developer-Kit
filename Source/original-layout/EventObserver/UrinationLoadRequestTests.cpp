#include "../Core/UrinationLoadRequest.hpp"
#include "ResidentRootBank.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
struct Registry {
    bool held=true;cfggcretain::ReferencerList list{0x10000,0,64};
    std::array<std::uintptr_t,cfggcprobe::MaximumOwners> entries{};
    bool ProvenSlotWindow()const noexcept{return held;}
    bool Snapshot(cfggcretain::ReferencerList& out,decltype(entries)& values)noexcept{out=list;values=entries;return true;}
    bool Register(std::uintptr_t owner)noexcept{entries[static_cast<unsigned>(list.count++)]=owner;return true;}
};
}
int main(){using namespace cfgurination;
 const LoadIdentity owner{0x20000,0x30000,7,8,9,0x18E030,1};
 for(unsigned failure=0;failure<6;++failure){LoadRequest r;auto id=owner;if(failure==0)id.generation=0;
  CHECK(r.Prepare(id,100,failure!=1,failure!=2)==(failure>=3));
  if(failure<3)continue;
  CHECK(!r.Enter(false)&&r.Enter(true)&&!r.Enter(true));
  CHECK(!r.Capture(0x40000,0x40001,9,owner.uuid,owner.controller,true));
  CHECK(!r.Capture(0x40000,0x40000,10,owner.uuid,owner.controller,true));
  CHECK(!r.Capture(0x40000,0x40000,9,owner.uuid+1,owner.controller,true));
  CHECK(!r.Capture(0x40000,0x40000,9,owner.uuid,owner.controller+1,true));
  CHECK(!r.Capture(0x40000,0x40000,9,owner.uuid,owner.controller,false));
  CHECK(r.Capture(0x40000,0x40000,9,owner.uuid,owner.controller,true));
  CHECK(!r.Capture(0x50000,0x50000,9,owner.uuid,owner.controller,true));
  CHECK(r.RequestReturned(true,true));
  CHECK(!r.BeginCompletion(0x40001,9,1,200,true,true,true,false));
  CHECK(!r.BeginCompletion(0x40000,10,1,200,true,true,true,false));
  CHECK(!r.BeginCompletion(0x40000,9,2,200,true,true,true,false));
  CHECK(r.Stage()==LoadStage::Waiting);
  if(failure==3){CHECK(r.BeginCompletion(0x40000,9,1,200,true,true,true,false));
   CHECK(!r.BeginCompletion(0x40000,9,1,200,true,true,true,false)&&r.Complete(true)&&r.Stage()==LoadStage::Ready&&!r.Action());}
  if(failure==4){r.ObserveDestroyed(0x40000);CHECK(r.Stage()==LoadStage::Rejected&&!r.Action());}
  if(failure==5){CHECK(r.BeginCompletion(0x40000,9,1,200,true,true,true,false));CHECK(!r.Complete(false)&&r.Stage()==LoadStage::Rejected&&!r.Action());}
 }
 for(unsigned fault=0;fault<7;++fault){LoadRequest r;CHECK(r.Prepare(owner,100,true,true)&&r.Enter(true));
  CHECK(r.Capture(0x40000,0x40000,9,owner.uuid,owner.controller,true)&&r.RequestReturned(true,true));
  if(fault==0)r.Expire(30100,true);else if(fault==1)r.Expire(200,false);
  else CHECK(!r.BeginCompletion(0x40000,9,1,fault==2?30100:200,fault!=3,fault!=4,fault!=5,fault==6));
  CHECK(r.Stage()==LoadStage::Rejected&&!r.Action()&&!r.BeginCompletion(0x40000,9,1,200,true,true,true,false));
 }
 {Registry registry;cfgretain::ResidentRootBank<1> bank;decltype(bank)::Reservation inputs,output;
  std::array<decltype(bank)::Handle,2> inputHandles{};std::array<decltype(bank)::Handle,1> loaded{};
  CHECK(bank.ReserveOutputs(registry,0x100000,2,inputs)&&bank.CommitFreshReturns(registry,inputs,std::array<std::uintptr_t,2>{owner.world,owner.controller},inputHandles));
  CHECK(bank.ReserveOutputs(registry,0x100000,1,output));
  LoadRequest request;CHECK(request.Prepare(owner,100,true,true)&&request.Enter(true));
  CHECK(request.Capture(0x40000,0x40000,9,owner.uuid,owner.controller,true)&&request.RequestReturned(true,true));
  registry.held=false;CHECK(request.BeginCompletion(0x40000,9,1,200,true,true,true,false));
  CHECK(!bank.CommitFreshReturns(registry,output,std::array<std::uintptr_t,1>{0x50000},loaded)&&!loaded[0].generation);
  registry.held=true;CHECK(bank.CommitFreshReturns(registry,output,std::array<std::uintptr_t,1>{0x50000},loaded)&&request.Complete(true));
  std::uintptr_t resolved=0;CHECK(bank.ResolveRequired(registry,loaded[0],resolved)&&resolved==0x50000);
  // Native action can retire now: only opaque rooted output is used later.
  request.ObserveDestroyed(0x40000);CHECK(request.Stage()==LoadStage::Ready);
 }
 std::printf("Urination owned async request and actual resident output bank: %u failures; constructor/thread/UUID/owner/generation/timeout/cancel/reentry/destruction/refusal; no engine authority.\n",failures);
 return failures?1:0;
}
