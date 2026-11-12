#pragma once
#include <cstdint>
namespace cfgmenu::epochservice {
// Donor's missed-event fallback supplies service opportunities only. It does
// not authorize mutation, relax the offline/native-menu proof, or require an
// old HUD/function to survive a rebuilt frontend.
inline bool Candidate(std::uint32_t thread,std::uint32_t owner,unsigned state,bool nested,bool returned){
 return owner&&thread==owner&&(state==7||state==8)&&!nested&&returned;
}
// A stock Challenge can travel while the counselor pipeline stays EntryReady.
// This only admits cold frontend reproving; it is never a gameplay continuation.
inline bool UnclaimedTravel(unsigned state,bool claimed,bool committed,bool startupEmpty,
 bool entryReady,bool installed,bool bundleIdle)noexcept{
 return state==8&&!claimed&&!committed&&startupEmpty&&entryReady&&installed&&bundleIdle;
}
inline bool Due(std::uint64_t now,std::uint64_t& next){
 if(now<next||now>UINT64_MAX-250)return false;next=now+250;return true;
}
// Metadata retirement and native travel are asynchronous. Waiting on them is
// not a failed rebinding transaction and must not exhaust all eight attempts
// before the last objective metadata restoration has returned.
inline bool AdmitAttempt(bool retirementReady,unsigned& attempts){
 if(!retirementReady||attempts>=8)return false;++attempts;return true;
}
}
