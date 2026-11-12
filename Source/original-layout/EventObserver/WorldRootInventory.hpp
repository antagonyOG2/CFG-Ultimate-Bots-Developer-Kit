#pragma once
#include "GcRegistryProbe.hpp"
#include "../Core/WorldRootRetirement.hpp"
namespace cfgretain {
// Diagnostic names belong to resident C++ storage, never inferred from an
// engine owner's address. Unknown registrations are not safe to remove.
struct NamedRootOwner {std::uintptr_t address=0;const char* name=nullptr;const char* type=nullptr;unsigned group=0;};
template<class R,class Classify,class Emit>bool InspectNamedRootOwner(R& reader,const NamedRootOwner& named,
 Classify classify,Emit emit,unsigned& oldReferences)noexcept{
 oldReferences=0;cfggcprobe::OwnerObservation observed;cfggcprobe::OwnerObservationFailure failure{};
 if(!named.address||!named.name||!named.type||!cfggcprobe::CaptureOwnerObservation(reader,named.address,observed,failure))return false;
 if(observed.state==cfggcretain::State::Empty||observed.state==cfggcretain::State::Removed){emit(named,0,MapReference::Unrelated);return true;}
 if(observed.state!=cfggcretain::State::Registered||observed.destroyRequests||observed.callbacks!=observed.completed)return false;
 emit(named,0,MapReference::Unrelated);
 for(const auto object:observed.slots)if(object){const auto kind=classify(object);
  if(kind==MapReference::Unproven)return false;
  if(kind==MapReference::OldMap){++oldReferences;emit(named,object,kind);}
 }
 return true;
}
}
