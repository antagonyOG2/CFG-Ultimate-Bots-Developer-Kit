#pragma once
#include "KnifeRuntime.hpp"
namespace cfgknife {
struct NotifyWindow {
    Identity jason{},driver{},target{};int count=-1;
    bool armed=false,stopped=false,launched=false,multicast=false,consumed=false;
    LaunchGeometry geometry{};bool geometryReady=false,geometryUsed=false;
    bool UseNativeGeometry(std::uintptr_t actualDriver,std::uintptr_t actualJason)noexcept{
        if(!armed||stopped||launched||multicast||consumed||!geometryReady||geometryUsed||
           !driver.object||driver.object!=actualDriver||!jason.object||jason.object!=actualJason)return false;
        geometryUsed=true;return true; // at most the original native multicast, never an extra launch
    }
    bool Missing(const Snapshot& after,std::uintptr_t thrownBy,bool collisionOn)const noexcept{
        return armed&&!consumed&&!stopped&&!launched&&!multicast&&collisionOn&&
            after.jason==jason&&after.driver==driver&&after.target==target&&after.targetLiving&&
            after.count==count-1&&!after.pressed&&thrownBy==jason.object;
    }
};
}
