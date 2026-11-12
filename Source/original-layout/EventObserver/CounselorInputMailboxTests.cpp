#include "../Core/CounselorInputMailbox.hpp"
#include "../Core/DonorReinforcementQueue.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool v){if(!v)++failures;};
    cfginput::CounselorInputMailbox box;cfginput::CounselorInputConsumer reader;cfginput::CounselorInputSnapshot s;
    check(!box.Snapshot(s,0));check(box.Snapshot(s,123)&&!reader.Poll(s,0,true));
    box.Key(true,false,true,false,10);check(box.Snapshot(s,123)&&reader.Poll(s,100,true));
    check(!reader.Poll(s,100,true));box.Key(true,true,true,false,101);
    check(box.Snapshot(s,123)&&!reader.Poll(s,110,true));
    box.Key(false,false,true,false,120);box.Key(true,false,true,false,121);
    check(box.Snapshot(s,123)&&reader.Poll(s,130,true)); // complete tap survives100ms polling
    box.Key(false,false,true,false,140);box.Key(true,false,true,true,141);
    check(box.Snapshot(s,123)&&!reader.Poll(s,150,true)); // tools menu consumes F3, no spawn
    box.FocusLost();box.Key(true,false,false,false,151);
    check(box.Snapshot(s,123)&&!reader.Poll(s,160,true));
    box.FocusLost();box.Key(true,true,true,false,161);check(box.Snapshot(s,123)&&!reader.Poll(s,170,true));
    box.Button(false,180);check(box.Snapshot(s,123)&&!reader.Poll(s,190,true));
    box.Button(true,200);check(box.Snapshot(s,123)&&reader.Poll(s,210,true)&&s.buttonRequests==2);
    box.Button(true,220);check(box.Snapshot(s,123)&&!reader.Poll(s,230,false)&&!reader.Poll(s,240,true));
    box.Button(true,250);check(box.Snapshot(s,123)&&!reader.Poll(s,250+cfginput::CounselorInputConsumer::LifetimeMs+1,true)); // expired intent never replays
    box.Button(true,1300);check(box.Snapshot(s,123)&&!reader.Poll(s,1299,true));
    box.Button(true,1400);check(box.Snapshot(s,123));auto bad=s;bad.version=2;check(!reader.Poll(bad,1401,true));
    bad=s;bad.bytes=0;check(!reader.Poll(bad,1401,true));bad=s;bad.magic=0;check(!reader.Poll(bad,1401,true));
    check(reader.Poll(s,1401,true)&&!reader.Poll(s,1402,true));
    cfginput::CounselorInputConsumer fresh;check(!fresh.Poll(s,1402,true)); // menu/pre-start events are baseline only
    cfgreinforcement::Counselors queue;box.Button(true,1500);check(box.Snapshot(s,123));
    if(reader.Poll(s,1501,true))check(queue.Request(1));
    check(queue.accepted==1&&queue.pending==1&&queue.Ready(1,1));
    if(reader.Poll(s,1502,true))queue.Request(1);
    check(queue.accepted==1&&queue.pending==1);queue.Started();check(!queue.pending);
    box.Button(true,1600);check(box.Snapshot(s,123)&&reader.Poll(s,11600,true)); // paused UI can resume within15s
    check(!reader.Poll(s,11601,true));
    cfgreinforcement::Counselors full;check(!full.Request(cfglimits::MaximumHelpers));
    std::printf("Counselor window/button input mailbox: %u failures; taps/repeats/focus/menu/expiry/ABI/no replay; no game access.\n",failures);
    return failures?1:0;
}
