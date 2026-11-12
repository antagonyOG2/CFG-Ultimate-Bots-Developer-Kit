#include "../Core/VehicleContinuationPolicy.hpp"
#include <cstdio>
struct Host {
    cfgbundle::Schedule schedule;bool urgent=true,exclusive=false;
    cfgbundle::Schedule& VehicleSchedule(){return schedule;}
    bool UrgentVehicleContinuation(){return urgent&&!exclusive;}
};
struct Counts {unsigned actions=0,preemptions=0,releases=0;};
Counts Replay(bool corrected){
    Host h;h.schedule.Start(0,0);std::uint64_t recoveryNext=0;Counts counts;
    for(std::uint64_t now=100;now<=30000;now+=100){
        const auto lane=h.schedule.Due(now);if(lane==cfgbundle::Lane::None)continue;
        const auto phase=[&](){
            cfgvehiclecontinuation::Turn<Host> continuation(h,corrected?lane:cfgbundle::Lane::None,now);
            if(lane==cfgbundle::Lane::Release){h.schedule.Released();++counts.releases;return;}
            if(now>=recoveryNext){recoveryNext=now+250;if(lane==cfgbundle::Lane::Vehicle)++counts.preemptions;return;}
            if(lane==cfgbundle::Lane::Vehicle){++counts.actions;h.schedule.vehicleAt=now+100;}
        };
        phase(); // actual RAII guard must run on the recovery early-return
        if(now==10000)h.schedule.Pressed(now);
    }
    return counts;
}
int main(){unsigned failures=0;auto check=[&](bool value){if(!value)++failures;};
    const auto old=Replay(false),fixed=Replay(true);
    check(old.preemptions>0&&fixed.actions>old.actions&&fixed.releases==1);
    for(unsigned fault=0;fault<6;++fault){Host h;h.schedule.active=true;h.schedule.vehicleAt=2100;
        if(fault==1)h.urgent=false;if(fault==2)h.exclusive=true;if(fault==3)h.schedule.pressed=true;
        if(fault==4)h.schedule.active=false;
        {cfgvehiclecontinuation::Turn<Host> guard(h,fault==5?cfgbundle::Lane::Hunt:cfgbundle::Lane::Vehicle,1000);}
        check(h.schedule.vehicleAt==(fault?2100:1100));
    }
    {Host h;h.schedule.active=true;h.schedule.vehicleAt=1050;
     {cfgvehiclecontinuation::Turn<Host> guard(h,cfgbundle::Lane::Vehicle,1000);}check(h.schedule.vehicleAt==1050);}
    {Host h;h.schedule.active=true;h.schedule.vehicleAt=2100;
     {cfgvehiclecontinuation::Turn<Host> guard(h,cfgbundle::Lane::Vehicle,1000);h.urgent=false;}check(h.schedule.vehicleAt==2100);}
    std::printf("Actual vehicle continuation RAII/composed250ms recovery replay: old=%u fixed=%u release=%u failures=%u; no game access.\n",
        old.actions,fixed.actions,fixed.releases,failures);return failures?1:0;
}
