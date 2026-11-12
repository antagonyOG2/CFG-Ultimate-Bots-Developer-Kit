#pragma once
#include "GameplayBundlePolicy.hpp"
#include "VehicleActionPolicy.hpp"
namespace cfgvehiclecontinuation {
// Due() advances Vehicle by1100ms before the caller runs prerequisite work.
// A bounded recovery/metadata observation may own that entire callback. Keep
// urgent continuation due without stacking its native effects on that budget.
template<class Host>class Turn {
    Host& host_;cfgbundle::Lane lane_;std::uint64_t now_;
public:
    Turn(Host& host,cfgbundle::Lane lane,std::uint64_t now):host_(host),lane_(lane),now_(now){}
    ~Turn(){
        if(lane_!=cfgbundle::Lane::Vehicle||!host_.UrgentVehicleContinuation())return;
        auto& schedule=host_.VehicleSchedule();
        if(!schedule.active||schedule.pressed||now_>UINT64_MAX-100)return;
        const auto deadline=now_+100;
        if(schedule.vehicleAt>deadline)schedule.vehicleAt=deadline;
    }
    Turn(const Turn&)=delete;Turn& operator=(const Turn&)=delete;
};
}
