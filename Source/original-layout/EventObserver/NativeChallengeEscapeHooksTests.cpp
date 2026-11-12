#include "../Core/ChallengeEscapeStartPolicy.hpp"
#include "NativeChallengeEscapeHooks.hpp"
#include <cstdio>
#include <vector>
namespace h=cfgchallengeescape::hooks;
struct AbiReceipt {unsigned overlap=0,character=0,presentation=0,completion=0,vehicle=0;bool okay=true;} receipt;
void __fastcall Overlap(void* a,void* b,void* c,void* d,std::int32_t index,bool swept,const void* hit){++receipt.overlap;
 receipt.okay=receipt.okay&&a==reinterpret_cast<void*>(1)&&b==reinterpret_cast<void*>(2)&&c==reinterpret_cast<void*>(3)&&
  d==reinterpret_cast<void*>(4)&&index==5&&swept&&hit==reinterpret_cast<void*>(7);}
void __fastcall Character(void* counselor,void* spline,float time){++receipt.character;
 receipt.okay=receipt.okay&&counselor==reinterpret_cast<void*>(1)&&spline==reinterpret_cast<void*>(2)&&time==3.25f;}
void __fastcall Presentation(void* volume,void* counselor){++receipt.presentation;
 receipt.okay=receipt.okay&&volume==reinterpret_cast<void*>(1)&&counselor==reinterpret_cast<void*>(2);}
void __fastcall Completion(void* mode,void* controller,void* counselor){++receipt.completion;
 receipt.okay=receipt.okay&&mode==reinterpret_cast<void*>(1)&&controller==reinterpret_cast<void*>(2)&&counselor==reinterpret_cast<void*>(3);}
void __fastcall Vehicle(void* vehicle,void* volume,void* spline,float time){++receipt.vehicle;
 receipt.okay=receipt.okay&&vehicle==reinterpret_cast<void*>(1)&&volume==reinterpret_cast<void*>(2)&&spline==reinterpret_cast<void*>(3)&&time==4.25f;}
h::Request Request(){return {0x100000,0x200000,{Overlap,Character,Presentation,Completion,Vehicle},true,true,true,42};}
struct Backend {
 bool ready=true,pin=true,capture=true,current=true;int initialize=MH_ERROR_ALREADY_INITIALIZED;
 unsigned creates=0,removes=0,enableQueues=0,disableQueues=0,applies=0,pristineCalls=0,trampolines=0;
 unsigned createFail=0,trampolineFail=0,enableQueueFail=0,disableQueueFail=0,applyFailMask=0,pristineFailAt=0;
 std::array<bool,5> desired{},live{};std::vector<std::pair<std::uintptr_t,bool>> queues;
 std::vector<std::uintptr_t> removed;bool removeFail=false;h::Request request{};
 bool Ready(const h::Request& r){request=r;return ready&&r.module==0x100000&&r.handlerImage==0x200000&&
  r.exactImageVerified&&r.outsideLoaderLock&&r.exclusiveBackend&&r.installThread==42;}
 bool Pin(){return pin;}int Initialize(){return initialize;}
 bool Pristine(){++pristineCalls;if(pristineCalls==pristineFailAt)return false;
  for(bool value:live)if(value)return false;return true;}
 int Create(std::uintptr_t target,void* handler,void*& original){++creates;original=nullptr;
  if(target!=request.module+h::Bodies[creates-1].rva||handler!=h::Addresses(request.handlers)[creates-1])return MH_UNKNOWN;
  if(creates==createFail)return MH_ERROR_MEMORY_ALLOC;original=handler;return MH_OK;}
 bool Trampoline(std::uintptr_t,void* original){++trampolines;return original&&trampolines!=trampolineFail;}
 int Queue(std::uintptr_t target,bool enabled){queues.emplace_back(target,enabled);unsigned index=5;
  for(unsigned i=0;i<5;++i)if(target==request.module+h::Bodies[i].rva)index=i;if(index==5)return MH_UNKNOWN;
  if(enabled){if(++enableQueues==enableQueueFail)return MH_ERROR_NOT_CREATED;}
  else if(++disableQueues==disableQueueFail)return MH_ERROR_NOT_CREATED;
  desired[index]=enabled;return MH_OK;}
 int Apply(){++applies;if(applyFailMask&(1u<<applies)){
   for(unsigned i=0;i<2;++i)live[i]=desired[i];return MH_ERROR_MEMORY_PROTECT;}
  live=desired;return MH_OK;}
 int Remove(std::uintptr_t target){++removes;removed.push_back(target);return removeFail?MH_ERROR_MEMORY_PROTECT:MH_OK;}
 bool CaptureInstalled(){return capture;}
 bool InstalledCurrent(){if(!current)return false;for(bool value:live)if(!value)return false;return true;}
};
struct BodyMemory {
 static constexpr std::uintptr_t Module=0x100000;
 std::array<std::vector<unsigned char>,5> bodies;
 BodyMemory(){for(unsigned i=0;i<5;++i)bodies[i].assign(h::Bodies[i].code,h::Bodies[i].code+h::Bodies[i].bytes);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){for(unsigned i=0;i<5;++i){const auto start=Module+h::Bodies[i].rva;
   if(at>=start&&at-start<bodies[i].size()){const auto offset=static_cast<std::size_t>(at-start);
    if(bytes>bodies[i].size()-offset)return false;std::memcpy(out,bodies[i].data()+offset,bytes);return true;}}
  return false;}
};
void Absolute(unsigned char* out,std::uintptr_t destination){const unsigned char jump[]{0xFF,0x25,0,0,0,0};std::memcpy(out,jump,6);std::memcpy(out+6,&destination,8);}
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
 {Backend b;h::Five<Backend> five(b);h::Originals published{};const auto request=Request();
  check(!five.Enable()&&!five.SuppressionReady()&&!five.PublishOriginals(published),"no default install, publication or suppression");
  check(five.Prepare(request)&&five.Prepared()&&!five.Enabled()&&b.creates==5&&!b.applies,"all five disabled originals prepared");
  check(!five.Enable()&&!b.applies&&!b.enableQueues,"enable requires explicit original publication");
  check(five.PublishOriginals(published)&&five.Published()&&!five.PublishOriginals(published),"publish originals after prepare and before enable exactly once");
  check(h::Addresses(published)==h::Addresses(request.handlers),"all five exact typed original signatures retained");
  published.overlap(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),reinterpret_cast<void*>(3),reinterpret_cast<void*>(4),5,true,reinterpret_cast<void*>(7));
  published.character(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),3.25f);
  published.presentation(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2));
  published.completion(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),reinterpret_cast<void*>(3));
  published.vehicle(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),reinterpret_cast<void*>(3),4.25f);
  check(receipt.okay&&receipt.overlap==1&&receipt.character==1&&receipt.presentation==1&&receipt.completion==1&&receipt.vehicle==1,"native seven/three/two/three/four arguments preserve controller/counselor and float positions");
  check(five.Enable()&&five.Enabled()&&five.SuppressionReady()&&b.applies==1&&b.enableQueues==5,"all five queued in order with exactly one apply");
  for(unsigned i=0;i<5;++i)check(b.queues[i].first==request.module+h::Bodies[i].rva&&b.queues[i].second,"only disk-pinned five targets queued");
  check(!five.Enable()&&!five.Prepare(request),"installation cannot repeat");
  check(five.Disable()&&five.Status()==h::State::DisabledResident&&!five.SuppressionReady()&&b.disableQueues==5&&b.applies==2&&!b.removes,"explicit disable keeps all published trampolines resident");
  check(h::Addresses(published)==h::Addresses(five.OriginalHandlers()),"disable retains published original addresses");
 }
 for(unsigned fault=0;fault<10;++fault){Backend b;auto r=Request();h::Five<Backend> five(b);
  switch(fault){case 0:r.exactImageVerified=false;break;case 1:r.outsideLoaderLock=false;break;case 2:r.exclusiveBackend=false;break;
   case 3:r.module=0x300000;break;case 4:r.handlerImage=0x300000;break;case 5:r.installThread=0;break;
   case 6:b.pin=false;break;case 7:b.initialize=MH_ERROR_MEMORY_ALLOC;break;case 8:b.pristineFailAt=1;break;case 9:b.ready=false;break;}
  check(!five.Prepare(r)&&five.Status()==h::State::Refused&&!b.creates&&!b.applies&&!five.Published(),"all admission failures leave no hooks or originals published");
 }
 for(unsigned failed=1;failed<=5;++failed){
  {Backend b;b.createFail=failed;h::Five<Backend> five(b);
   check(!five.Prepare(Request())&&five.Status()==h::State::Refused&&b.creates==failed&&b.removes==failed-1&&!five.SuppressionReady(),"every create failure removes only earlier disabled hooks before publication");
   check(h::Addresses(five.OriginalHandlers())==std::array<void*,5>{},"definite prepublication cleanup clears originals");}
  {Backend b;b.trampolineFail=failed;h::Five<Backend> five(b);
   check(!five.Prepare(Request())&&five.Status()==h::State::Refused&&b.removes==failed,"every invalid trampoline removes all created disabled hooks");}
  if(failed>1){Backend b;b.createFail=failed;b.removeFail=true;h::Five<Backend> five(b);
   check(!five.Prepare(Request())&&five.Quarantined()&&b.removes==failed-1&&!five.SuppressionReady(),"all prepublication removal attempts made before quarantine");
   check(h::Addresses(five.OriginalHandlers())[0]!=nullptr&&!five.Prepare(Request()),"ambiguous removal retains originals without reinstall or suppression");}
  {Backend b;b.enableQueueFail=failed;h::Five<Backend> five(b);h::Originals originals;
   check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&five.Status()==h::State::DisabledResident&&
    b.enableQueues==failed&&b.disableQueues==5&&!b.applies&&!b.removes,"each enable queue failure cancels all five before any apply");
   check(h::Addresses(originals)==h::Addresses(five.OriginalHandlers())&&!five.SuppressionReady(),"queue failure preserves all published originals");}
  {Backend b;b.enableQueueFail=1;b.disableQueueFail=failed;h::Five<Backend> five(b);h::Originals originals;
   check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&five.Quarantined()&&b.disableQueues==5&&!b.applies&&!b.removes,
    "each cancel queue failure tries remaining targets then quarantines without applying unknown queue");}
  {Backend b;b.applyFailMask=1u<<1;b.disableQueueFail=failed;h::Five<Backend> five(b);h::Originals originals;
   check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&five.Quarantined()&&b.disableQueues==5&&b.applies==1&&!b.removes,
    "every rollback queue failure after partial apply keeps resident originals quarantined");}
 }
 {Backend b;b.pristineFailAt=2;h::Five<Backend> five(b);
  check(!five.Prepare(Request())&&b.removes==5&&five.Status()==h::State::Refused,"full body changed after preparation removes all five before publication");}
 {Backend b;h::Five<Backend> five(b);h::Originals originals;check(five.Prepare(Request())&&five.PublishOriginals(originals),"before-enable body fixture");b.pristineFailAt=3;
  check(!five.Enable()&&five.Status()==h::State::DisabledResident&&!b.enableQueues&&!b.removes,"changed full body refuses enable and retains published originals");}
 for(unsigned mask:{1u<<1,(1u<<1)|(1u<<2)}){Backend b;b.applyFailMask=mask;h::Five<Backend> five(b);h::Originals originals;
  check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&b.enableQueues==5&&b.disableQueues==5&&b.applies==2&&!b.removes,
   "partial apply always tries one five-target disable rollback transaction");
  check(five.Status()==(mask==(1u<<1)?h::State::DisabledResident:h::State::Quarantined)&&!five.SuppressionReady()&&
   h::Addresses(originals)==h::Addresses(five.OriginalHandlers()),"successful rollback disables; failed rollback quarantines; originals always retained");}
 {Backend b;b.applyFailMask=1u<<1;b.pristineFailAt=4;h::Five<Backend> five(b);h::Originals originals;
  check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&five.Quarantined()&&!five.SuppressionReady(),"rollback success cannot hide non-pristine remainder");}
 {Backend b;b.capture=false;h::Five<Backend> five(b);h::Originals originals;
  check(five.Prepare(Request())&&five.PublishOriginals(originals)&&!five.Enable()&&five.Status()==h::State::DisabledResident&&b.applies==2&&b.disableQueues==5&&!b.removes,
   "installed lease capture failure rolls back all five after apply");}
 {Backend b;h::Originals originals;{
   h::Five<Backend> five(b);check(five.Prepare(Request())&&five.PublishOriginals(originals)&&five.Enable(),"installed lease drift fixture");b.current=false;
   check(!five.SuppressionReady()&&five.Quarantined()&&!b.removes,"lease drift immediately removes suppression readiness without freeing active originals");b.current=true;
   check(!five.SuppressionReady()&&!five.Enable()&&!five.Disable()&&five.Quarantined(),"lease quarantine is sticky even after apparent restoration");}
  check(!b.removes&&h::Addresses(originals)==h::Addresses(Request().handlers),"group destructor leaves all published originals safely resident");
 }
 for(unsigned index=0;index<5;++index){const auto& body=h::Bodies[index];const auto target=BodyMemory::Module+body.rva;
  const auto original=target+0x10000;const auto handler=target+0x20000;
  std::array<unsigned char,60> trampoline{};std::copy_n(body.code,body.copied,trampoline.begin());
  Absolute(trampoline.data()+body.copied,target+body.copied);Absolute(trampoline.data()+body.copied+14,handler);
  std::array<unsigned char,5> patch{};
  check(h::TrampolineProof(body,target,original,handler,trampoline.data(),patch)&&body.copied<=32,"five exact copied prologs prove no hotpatchAbove and bounded patch extent");
  for(unsigned byte=0;byte<body.copied+28;++byte){auto bad=trampoline;bad[byte]^=1;std::array<unsigned char,5> rejected{};
   check(!h::TrampolineProof(body,target,original,handler,bad.data(),rejected),"every copied prolog/jumpback/own relay byte is checked");}
  std::array<unsigned char,5> rejected{};
  check(!h::TrampolineProof(body,target,original+0x100000000ULL,handler,trampoline.data(),rejected),"unreachable relay refuses MinHook patch");
  BodyMemory memory;std::copy(patch.begin(),patch.end(),memory.bodies[index].begin());h::InstalledLease lease;
  check(lease.Capture(memory,memory.Module,index,patch)&&lease.Current(memory),"installed lease captures exact patched prefix32 and pristine full-body remainder");
  for(unsigned byte=0;byte<body.bytes;++byte){memory.bodies[index][byte]^=1;
   check(!lease.Current(memory),"every installed body byte covered including patched zero bytes and entire native remainder");memory.bodies[index][byte]^=1;}
  memory.bodies[index][body.bytes-1]^=1;h::InstalledLease bad;
  check(!bad.Capture(memory,memory.Module,index,patch)&&!bad.captured,"capture rejects already-corrupted full native remainder");
  memory.bodies[index][body.bytes-1]^=1;auto wrong=patch;wrong[1]^=1;
  check(!bad.Capture(memory,memory.Module,index,wrong),"capture rejects foreign E9 destination");
  memory.bodies[index][5]^=1;
  check(!bad.Capture(memory,memory.Module,index,patch),"capture rejects changes after actual 5-byte patch inside prefix32");
 }
 // Task005: the paired offline hooks are already installed and never
 // rolled back by a refused escape Five<> transaction.
 {Backend b;b.pristineFailAt=1;h::Five<Backend> escape(b);unsigned paired=1;
  const bool start=cfgj29escape::InstallAfterNative(paired!=0,[&]{h::Originals o;
   return escape.Prepare(Request())&&escape.PublishOriginals(o)&&escape.Enable();});
  check(start&&paired==1&&escape.Status()!=h::State::Enabled,"escape install failure is non-fatal to existing native pair");
 }
 {unsigned failuresBefore=0,canceled=0;
  check(cfgj29escape::AllowStart(false,failuresBefore,[&]{++canceled;})&&failuresBefore==1&&canceled==1,
   "failed escape Arm cancels only escape and still permits Challenges Start");
  check(cfgj29escape::AllowStart(true,failuresBefore,[&]{++canceled;})&&failuresBefore==1&&canceled==1,
   "successful escape Arm does not count or cancel");
 }
 std::printf("Challenge escape five-hook transaction/signature/full-body lease: %u checks, %u failures; owned synthetic memory only.\n",checks,failures);
 return failures?1:0;
}
