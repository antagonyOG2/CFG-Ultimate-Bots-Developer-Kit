#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "TrampolinePreparation.hpp"
#include "../RuntimeProbe/ChallengeEscapeRecovery.generated.hpp"
#include "../SourceBaseline/gameplay-backend/port/vendor/minhook/include/MinHook.h"
#include <algorithm>

namespace cfgchallengeescape::hooks {
// Exact native x64 signatures. Opaque pointers preserve the released ABI:
// overlap(self,component,otherActor,otherComponent,bodyIndex,sweep,hitResult),
// character(counselor,exitSpline,time), presentation(volume,counselor),
// completion(mode,CONTROLLER,COUNSELOR), vehicle(vehicle,volume,exitSpline,time).
using OverlapHandler=void(__fastcall*)(void*,void*,void*,void*,std::int32_t,bool,const void*);
using CharacterHandler=void(__fastcall*)(void*,void*,float);
using PresentationHandler=void(__fastcall*)(void*,void*);
using CompletionHandler=void(__fastcall*)(void*,void*,void*);
using VehicleHandler=void(__fastcall*)(void*,void*,void*,float);
struct Originals {
 OverlapHandler overlap=nullptr;CharacterHandler character=nullptr;
 PresentationHandler presentation=nullptr;CompletionHandler completion=nullptr;VehicleHandler vehicle=nullptr;
};
struct Request {
 std::uintptr_t module=0,handlerImage=0;Originals handlers{};
 bool exactImageVerified=false,outsideLoaderLock=false,exclusiveBackend=false;
 DWORD installThread=0;
};
enum class State {Empty,Preparing,Prepared,Enabling,Enabled,DisabledResident,Refused,Quarantined};
struct Body {std::uintptr_t rva;const unsigned char* code;unsigned bytes,copied;};
inline constexpr std::array<Body,5> Bodies{{
 {disk::Overlap,disk::OverlapCode.data(),static_cast<unsigned>(disk::OverlapCode.size()),6},
 {disk::CharacterStart,disk::CharacterStartCode.data(),static_cast<unsigned>(disk::CharacterStartCode.size()),6},
 {disk::Presentation,disk::PresentationCode.data(),static_cast<unsigned>(disk::PresentationCode.size()),5},
 {disk::Completion,disk::CompletionCode.data(),static_cast<unsigned>(disk::CompletionCode.size()),5},
 {disk::VehicleStart,disk::VehicleStartCode.data(),static_cast<unsigned>(disk::VehicleStartCode.size()),5}
}};
inline std::array<void*,5> Addresses(const Originals& o){return {{reinterpret_cast<void*>(o.overlap),reinterpret_cast<void*>(o.character),
 reinterpret_cast<void*>(o.presentation),reinterpret_cast<void*>(o.completion),reinterpret_cast<void*>(o.vehicle)}};}
inline Originals Typed(const std::array<void*,5>& o){return {reinterpret_cast<OverlapHandler>(o[0]),reinterpret_cast<CharacterHandler>(o[1]),
 reinterpret_cast<PresentationHandler>(o[2]),reinterpret_cast<CompletionHandler>(o[3]),reinterpret_cast<VehicleHandler>(o[4])};}
// These pinned prologues contain only copied stack/register instructions before
// MinHook's 5-byte E9. The existing absolute-jump helper verifies both jump-back
// and own handler relay. No hot-patch-above path is accepted; copied extent<=32.
inline bool TrampolineProof(const Body& b,std::uintptr_t target,std::uintptr_t original,std::uintptr_t handler,
 const unsigned char* bytes,std::array<unsigned char,5>& patch){
 patch={};if(!target||!original||!handler||!bytes||b.copied<5||b.copied>32||b.copied>b.bytes||
  !std::equal(bytes,bytes+b.copied,b.code)||!cfgobserver::AbsoluteJump(bytes+b.copied,target+b.copied)||
  !cfgobserver::AbsoluteJump(bytes+b.copied+14,handler))return false;
 const auto relay=original+b.copied+14;
 const auto delta=static_cast<std::int64_t>(relay)-static_cast<std::int64_t>(target+5);
 if(delta<(std::numeric_limits<std::int32_t>::min)()||delta>(std::numeric_limits<std::int32_t>::max)())return false;
 patch[0]=0xE9;const auto rel=static_cast<std::int32_t>(delta);std::memcpy(patch.data()+1,&rel,sizeof(rel));return true;
}
struct InstalledLease {
 std::uintptr_t module=0,target=0;unsigned index=0;bool captured=false;
 std::array<unsigned char,32> installedPrefix{};
 template<class R>bool Capture(R& r,std::uintptr_t image,unsigned body,const std::array<unsigned char,5>& expectedPatch){
  *this={};if(!image||body>=Bodies.size()||expectedPatch[0]!=0xE9)return false;const auto& b=Bodies[body];
  std::array<unsigned char,disk::OverlapCode.size()> actual{};
  if(b.bytes<installedPrefix.size()||!r.ReadBytes(image+b.rva,actual.data(),b.bytes)||
     !std::equal(actual.begin(),actual.begin()+5,expectedPatch.begin())||
     !std::equal(actual.begin()+5,actual.begin()+b.bytes,b.code+5))return false;
  module=image;target=image+b.rva;index=body;std::copy_n(actual.begin(),installedPrefix.size(),installedPrefix.begin());
  captured=true;return Current(r);
 }
 template<class R>bool Current(R& r)const{
  if(!captured||!module||index>=Bodies.size()||target!=module+Bodies[index].rva)return false;
  const auto& b=Bodies[index];std::array<unsigned char,disk::OverlapCode.size()> actual{};
  return r.ReadBytes(target,actual.data(),b.bytes)&&
   std::equal(installedPrefix.begin(),installedPrefix.end(),actual.begin())&&
   std::equal(actual.begin()+installedPrefix.size(),actual.begin()+b.bytes,b.code+installedPrefix.size());
 }
};
template<class R,class Range>bool BudgetedHookCurrent(R& r,std::uintptr_t module,const std::array<void*,5>& originals,
 const Originals& handlers,const std::array<InstalledLease,5>& leases,const std::array<std::array<unsigned char,5>,5>& patches,Range&& range){
 for(unsigned i=0;i<Bodies.size();++i){const auto& b=Bodies[i];
  if(!leases[i].Current(r))return false;
  const auto original=reinterpret_cast<std::uintptr_t>(originals[i]);
  std::array<unsigned char,60> bytes{};std::array<unsigned char,5> patch{};
  if(!range(originals[i],b.copied+28)||!r.ReadBytes(original,bytes.data(),b.copied+28)||
   !TrampolineProof(b,module+b.rva,original,reinterpret_cast<std::uintptr_t>(Addresses(handlers)[i]),bytes.data(),patch)||patch!=patches[i])return false;
 }
 return true;
}
class WindowsBackend {
 Request request_{};std::array<void*,5> originals_{};
 std::array<std::array<unsigned char,5>,5> patches_{};std::array<InstalledLease,5> leases_{};
 static bool ImageCode(std::uintptr_t at,std::uintptr_t image,std::size_t bytes){
  MEMORY_BASIC_INFORMATION m{};if(!at||!image||!bytes||VirtualQuery(reinterpret_cast<void*>(at),&m,sizeof(m))!=sizeof(m)||
   m.State!=MEM_COMMIT||m.Type!=MEM_IMAGE||reinterpret_cast<std::uintptr_t>(m.AllocationBase)!=image||
   (m.Protect!=PAGE_EXECUTE_READ&&m.Protect!=PAGE_EXECUTE_WRITECOPY))return false;
  const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
  return at>=base&&at-base<=m.RegionSize&&bytes<=m.RegionSize-(at-base);
 }
 bool MutationCurrent()const{return request_.exactImageVerified&&request_.outsideLoaderLock&&request_.exclusiveBackend&&
  request_.installThread&&GetCurrentThreadId()==request_.installThread&&
  request_.module==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));}
 unsigned Index(std::uintptr_t target)const{for(unsigned i=0;i<Bodies.size();++i)if(target==request_.module+Bodies[i].rva)return i;return 5;}
 struct Reader {WindowsBackend& backend;
  bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){SIZE_T read=0;
   return ImageCode(at,backend.request_.module,bytes)&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,bytes,&read)&&read==bytes;}
 };
public:
 template<class R>bool BudgetedCurrent(R& r){
  if(request_.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
  return BudgetedHookCurrent(r,request_.module,originals_,request_.handlers,leases_,patches_,
    [](void* address,unsigned bytes){return cfgobserver::ExecutableRange(address,bytes);});
 }
 bool Ready(const Request& r){
  if(!r.exactImageVerified||!r.outsideLoaderLock||!r.exclusiveBackend||!r.installThread||GetCurrentThreadId()!=r.installThread||
   r.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||!r.handlerImage||r.handlerImage==r.module)return false;
  const auto handlers=Addresses(r.handlers);
  for(unsigned i=0;i<handlers.size();++i){if(!ImageCode(reinterpret_cast<std::uintptr_t>(handlers[i]),r.handlerImage,1))return false;
   for(unsigned j=0;j<i;++j)if(handlers[i]==handlers[j])return false;}
  request_=r;return Pristine();
 }
 bool Pin(){HMODULE resident=nullptr;return MutationCurrent()&&GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
  reinterpret_cast<LPCWSTR>(request_.handlers.overlap),&resident)&&reinterpret_cast<std::uintptr_t>(resident)==request_.handlerImage;}
 int Initialize(){return MutationCurrent()?static_cast<int>(MH_Initialize()):MH_UNKNOWN;}
 bool Pristine(){Reader r{*this};std::array<unsigned char,disk::OverlapCode.size()> actual{};
  for(const auto& b:Bodies)if(!r.ReadBytes(request_.module+b.rva,actual.data(),b.bytes)||!std::equal(actual.begin(),actual.begin()+b.bytes,b.code))return false;
  return true;
 }
 int Create(std::uintptr_t target,void* handler,void*& original){original=nullptr;const auto i=Index(target);
  if(!MutationCurrent()||i>=Bodies.size()||handler!=Addresses(request_.handlers)[i])return MH_UNKNOWN;
  const auto status=MH_CreateHook(reinterpret_cast<void*>(target),handler,&original);
  if(status==MH_OK)originals_[i]=original;return static_cast<int>(status);
 }
 bool Trampoline(std::uintptr_t target,void* original){const auto i=Index(target);if(i>=Bodies.size()||originals_[i]!=original)return false;
  const auto& b=Bodies[i];std::array<unsigned char,60> bytes{};SIZE_T read=0;
  return cfgobserver::ExecutableRange(original,b.copied+28)&&ReadProcessMemory(GetCurrentProcess(),original,bytes.data(),b.copied+28,&read)&&read==b.copied+28&&
   TrampolineProof(b,target,reinterpret_cast<std::uintptr_t>(original),reinterpret_cast<std::uintptr_t>(Addresses(request_.handlers)[i]),bytes.data(),patches_[i]);
 }
 int Queue(std::uintptr_t target,bool enabled){if(!MutationCurrent()||Index(target)>=Bodies.size())return MH_UNKNOWN;
  return static_cast<int>(enabled?MH_QueueEnableHook(reinterpret_cast<void*>(target)):MH_QueueDisableHook(reinterpret_cast<void*>(target)));}
 int Apply(){return MutationCurrent()?static_cast<int>(MH_ApplyQueued()):MH_UNKNOWN;}
 int Remove(std::uintptr_t target){return MutationCurrent()&&Index(target)<Bodies.size()?static_cast<int>(MH_RemoveHook(reinterpret_cast<void*>(target))):MH_UNKNOWN;}
 bool CaptureInstalled(){Reader r{*this};for(unsigned i=0;i<Bodies.size();++i)if(!leases_[i].Capture(r,request_.module,i,patches_[i]))return false;
  return InstalledCurrent();}
 bool InstalledCurrent(){
  if(request_.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
  Reader r{*this};for(unsigned i=0;i<Bodies.size();++i)
   if(!leases_[i].Current(r)||!Trampoline(request_.module+Bodies[i].rva,originals_[i]))return false;
  return true;
 }
};
// No installer or hook handlers. Owner prepares, publishes ALL typed originals,
// then explicitly enables. Published trampolines and callback image remain
// resident forever, including failed apply/rollback and lease quarantine.
template<class Backend=WindowsBackend>class Five {
 Backend& backend_;Request request_{};std::array<void*,5> originals_{};std::array<bool,5> created_{};
 State state_=State::Empty;int lastError_=MH_OK;bool published_=false,enableAttempted_=false;
 std::uintptr_t Target(unsigned i)const{return request_.module+Bodies[i].rva;}
 bool RefuseBeforePublication(){bool removed=true;
  for(unsigned n=5;n;--n){const auto i=n-1;if(created_[i]){const int error=backend_.Remove(Target(i));
   if(error!=MH_OK){lastError_=error;removed=false;}}}
  if(removed){created_={};originals_={};state_=State::Refused;}else state_=State::Quarantined;
  return false;
 }
 bool CancelQueues(){bool okay=true;
  // Try ALL five even after a failure; leave an ambiguous transaction isolated.
  for(unsigned i=0;i<5;++i){const int error=backend_.Queue(Target(i),false);if(error!=MH_OK){lastError_=error;okay=false;}}
  if(!okay)state_=State::Quarantined;return okay;
 }
 bool Rollback(bool applied){
  if(!CancelQueues())return false;
  if(applied){const int error=backend_.Apply();if(error!=MH_OK){lastError_=error;state_=State::Quarantined;return false;}}
  if(!backend_.Pristine()){state_=State::Quarantined;return false;}
  state_=State::DisabledResident;return true;
 }
public:
 explicit Five(Backend& b):backend_(b){}
 Five(const Five&)=delete;Five& operator=(const Five&)=delete;
 // No destructor hook removal, MH_Uninitialize or DLL/trampoline free.
 State Status()const{return state_;}int LastError()const{return lastError_;}
 bool Prepared()const{return state_==State::Prepared||state_==State::Enabled||state_==State::DisabledResident;}
 bool Enabled()const{return state_==State::Enabled;}
 bool Quarantined()const{return state_==State::Quarantined;}
 bool Published()const{return published_;}
 Originals OriginalHandlers()const{return Typed(originals_);}
 bool Prepare(const Request& request){
  if(state_!=State::Empty)return false;state_=State::Preparing;request_=request;
  if(!backend_.Ready(request_)||!backend_.Pin()){state_=State::Refused;return false;}
  lastError_=backend_.Initialize();if(lastError_!=MH_OK&&lastError_!=MH_ERROR_ALREADY_INITIALIZED){state_=State::Refused;return false;}
  if(!backend_.Pristine()){state_=State::Refused;return false;}
  const auto handlers=Addresses(request_.handlers);
  for(unsigned i=0;i<5;++i){lastError_=backend_.Create(Target(i),handlers[i],originals_[i]);
   if(lastError_!=MH_OK)return RefuseBeforePublication();created_[i]=true;
   if(!backend_.Trampoline(Target(i),originals_[i]))return RefuseBeforePublication();}
  if(!backend_.Pristine())return RefuseBeforePublication();state_=State::Prepared;return true;
 }
 bool PublishOriginals(Originals& destination){
  if(state_!=State::Prepared||published_)return false;
  destination=Typed(originals_);MemoryBarrier();published_=true;return true;
 }
 bool Enable(){
  if(state_!=State::Prepared||!published_||enableAttempted_)return false;enableAttempted_=true;
  if(!backend_.Pristine()){state_=State::DisabledResident;return false;}state_=State::Enabling;
  for(unsigned i=0;i<5;++i){lastError_=backend_.Queue(Target(i),true);
   if(lastError_!=MH_OK){Rollback(false);return false;}}
  // Exactly one apply on success. Partial apply requires a single five-target
  // disable transaction; originals are never removed after publication.
  lastError_=backend_.Apply();if(lastError_!=MH_OK){Rollback(true);return false;}
  if(!backend_.CaptureInstalled()){Rollback(true);return false;}
  state_=State::Enabled;return true;
 }
 bool SuppressionReady(){
  if(state_!=State::Enabled||!published_)return false;
  if(!backend_.InstalledCurrent()){state_=State::Quarantined;return false;}
  return true;
 }
 template<class R>bool BudgetedSuppressionReady(R& r){
  if(state_!=State::Enabled||!published_)return false;
  if(!backend_.BudgetedCurrent(r)){if(r.Remaining())state_=State::Quarantined;return false;}
  return true;
 }
 bool Disable(){if(state_!=State::Enabled)return false;return Rollback(true);}
};
}
