#pragma once
#include "OfflineSettingsSelection.hpp"
namespace cfgloadout::restricted {
// This intentionally omits Random/Firecrackers/weapons until their native
// resources are independently available. These are not the old base indexes.
enum class Choice:unsigned {None,PocketKnife,FirstAid,Map};
struct Request {cfgofflinesettings::Request owner{};Choice choice=Choice::None;
    bool Valid()const{return owner.Valid()&&static_cast<unsigned>(choice)<=3;}};
enum class State {Empty,Waiting,Dispatched,Verified,Preserved,Unavailable,Indeterminate};
constexpr unsigned MaximumItems=8,MaximumAttempts=15,ReadLimit=1536;
}
