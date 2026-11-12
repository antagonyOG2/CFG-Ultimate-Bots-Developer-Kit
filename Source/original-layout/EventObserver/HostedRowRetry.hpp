#pragma once
#include <cstdint>
namespace cfghostedlobby {
// Retry only before ANY row native dispatch. A returned/ambiguous Create or
// attachment is never replayed, even if rollback later removes the widget.
struct RowRetry {
 unsigned attempts=0;std::uint64_t next=0;
 bool Due(bool failed,bool nativeEntered,unsigned owned,bool poisoned,bool faulted,std::uint64_t now){
  if(!failed||nativeEntered||owned||poisoned||faulted||attempts==3)return false;
  if(!next){next=now+(1000ULL<<attempts);return false;}
  return now>=next;
 }
 bool Poll(bool failed,bool nativeEntered,unsigned owned,bool poisoned,bool faulted,std::uint64_t now){
  if(!Due(failed,nativeEntered,owned,poisoned,faulted,now))return false;
  ++attempts;next=0;return true;
 }
};
}
