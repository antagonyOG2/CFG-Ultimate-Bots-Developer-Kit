#pragma once
#include <cstdint>
#include "ChallengeIntroDispatchPolicy.hpp"
namespace cfgmenu::challengeintro {
// Admission policy only. A completed callback is NOT a GC lease: the native
// provider must separately demonstrate a synchronized registry publication,
// positive owner/object serials, exact callback token and retained call inputs.
enum class ScopeRefusal:unsigned {None,Disabled,NoRequest,WrongCallback,WrongThread,Online,WrongMode,
    WrongWorld,OwnerStale,GCBlocked,MissingCallbackProof,MissingRetention,ScopeOccupied,NativeRefused};
struct ScopeEvidence {
 bool flag=false,posted=false,completed=false,nested=false,gameThread=false,offline=false,
     challengeMode=false,challengeState=false,routeOwner=false,worldCurrent=false,
     levelSettingsCurrent=false,gcClear=false,callbackProof=false,retainedInputs=false,
     exclusiveScope=false;
 ScopeRefusal Refusal()const noexcept{
  if(!flag)return ScopeRefusal::Disabled;
  if(!posted)return ScopeRefusal::NoRequest;
  if(!completed||nested)return ScopeRefusal::WrongCallback;
  if(!gameThread)return ScopeRefusal::WrongThread;
  if(!offline)return ScopeRefusal::Online;
  if(!challengeMode||!challengeState)return ScopeRefusal::WrongMode;
  if(!worldCurrent)return ScopeRefusal::WrongWorld;
  if(!routeOwner||!levelSettingsCurrent)return ScopeRefusal::OwnerStale;
  if(!gcClear)return ScopeRefusal::GCBlocked;
  if(!callbackProof)return ScopeRefusal::MissingCallbackProof;
  if(!retainedInputs)return ScopeRefusal::MissingRetention;
  if(!exclusiveScope)return ScopeRefusal::ScopeOccupied;
  return ScopeRefusal::None;
 }
 bool Admitted()const noexcept{return Refusal()==ScopeRefusal::None;}
};
struct ScopeDiagnostics {unsigned entered=0,refused=0,serviced=0,dropped=0;ScopeRefusal reason=ScopeRefusal::None;
 void Observe(const ScopeEvidence& evidence)noexcept{reason=evidence.Refusal();if(evidence.Admitted())++entered;else ++refused;}
 void Consume(const DispatchReceipt& before,const DispatchReceipt& after)noexcept{
  serviced+=after.returned-before.returned;dropped+=after.refused-before.refused;
 }
};
// This adapter never grants native permission. The host's InvokeRetainedIntro
// must be an independently proved synchronized root provider, not a raw call.
template<class Host>bool Consume(DispatchReceipt& receipt,Host& host,std::uint64_t now,ScopeDiagnostics& out){
 if(!receipt.Pending())return false;
 const auto before=receipt;const bool result=receipt.Service(host,now);
 out.Consume(before,receipt);return result;
}
}
