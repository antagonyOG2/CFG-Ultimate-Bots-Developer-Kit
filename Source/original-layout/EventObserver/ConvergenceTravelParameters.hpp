#pragma once
#include "TrapNativeParameters.hpp"
namespace cfgconvergencetravel {
// FrozenJasonBridge::IssueAIMoveToLocationOnGameThread, called with the
// default exactCenterline=false by SweaterConvergenceJason. Match ALL options.
inline cfgtrap::MoveLocationParameters MovePacket(cfgspawn::Vec3 goal){
 cfgtrap::MoveLocationParameters p;p.destination=goal;p.acceptance=100;
 p.strafe=1;p.allowPartial=1;return p;
}
}
