#define main ExistingOfflineSettingsMain
#include "OfflineSettingsTests.cpp"
#undef main
#include <cassert>
int main(){
    assert(ExistingOfflineSettingsMain()==0);
    {Fixture f;Runtime runtime;assert(runtime.Begin(f.request,0));
     assert(!runtime.StepParts(f.access,0)&&runtime.CurrentState()==State::Applying&&f.host.writes==1);
     f.memory.Set(f.GameState+disk::ElapsedInProgressTime,int{12});
     assert(runtime.StepParts(f.access,500)&&runtime.CurrentState()==State::Applied&&f.host.writes==2);
     int left=0;assert(f.memory.Read(f.GameState+disk::RemainingTime,left)&&left==3588);
     assert(!runtime.StepParts(f.access,1000)&&f.host.writes==2);}
    {Fixture f;f.request.seconds=UINT32_MAX;NativeAccess<Memory,Host> access(f.memory,f.host,f.request,f.context);
     Runtime runtime;assert(runtime.Begin(f.request,0));assert(!runtime.StepParts(access,0));assert(runtime.StepParts(access,500));
     int left=0;assert(f.memory.Read(f.GameState+disk::RemainingTime,left)&&left==-1);}
    for(unsigned fault=0;fault<4;++fault){Fixture f;Runtime runtime;assert(runtime.Begin(f.request,0));assert(!runtime.StepParts(f.access,0));
     if(fault==0)f.host.current=false;
     if(fault==1)f.Phase("WaitingPostMatch");
     if(fault==2)f.host.failWrite=2;
     assert(!runtime.StepParts(f.access,fault==3?30001:500));
     assert(runtime.CurrentState()==State::Indeterminate&&DurationFinished(runtime.CurrentState()));
     const auto writes=f.host.writes;assert(!runtime.StepParts(f.access,31000)&&f.host.writes==writes);}
    {Fixture f;f.host.failWrite=1;Runtime runtime;assert(runtime.Begin(f.request,0));assert(!runtime.StepParts(f.access,0));
     assert(runtime.CurrentState()==State::Refused&&DurationFinished(runtime.CurrentState()));
     assert(!runtime.StepParts(f.access,500)&&f.host.writes==1);}
    {Fixture f;f.Phase("WaitingToStart");Runtime runtime;assert(runtime.Begin(f.request,0));assert(!runtime.StepParts(f.access,0)&&!f.host.writes);
     assert(!runtime.StepParts(f.access,30001)&&runtime.CurrentState()==State::Refused);}
    assert(!DurationFinished(State::Waiting)&&!DurationFinished(State::Applying));
    std::puts("Duration parts PASS: fresh elapsed, one write/callback, refusal/partial terminal, no retry; optional failure remains explicit.");
}
