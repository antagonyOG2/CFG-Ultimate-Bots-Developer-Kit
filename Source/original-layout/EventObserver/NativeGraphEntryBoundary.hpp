#pragma once
#include "SettingsStartRouter.hpp"
namespace cfgmenu::startroute {
// Zero-alias wrappers call the graph interpreter directly. Fast aliases let
// ProcessEvent substitute the graph BEFORE invoking the executor slot. Claim
// exactly the reachable boundary, never both levels of one native invocation.
inline bool HasStartGraphAlias(const startbinding::Binding& binding)noexcept{
    return binding.start.fast.graph==binding.graph.field.object&&
        cfgprobe::ObjectPointer(binding.graph.field.object)&&binding.start.fast.entry==startdisk::StartEntry;
}
inline SlotKind StartBoundary(const startbinding::Binding& binding)noexcept{
    return HasStartGraphAlias(binding)?SlotKind::Graph:SlotKind::Wrapper;
}
template<class Claim,class Forward> Result NativeEntryBoundary(const startbinding::Binding& binding,SlotKind kind,Claim claim,Forward forward){
    if(kind!=StartBoundary(binding)){forward();return Result::Forwarded;}
    return claim();
}
// A counselor route owns BOTH entry aliases while a replaced settings widget
// is awaiting adoption. Do not let the non-claimed alias become stock travel.
template<class Claim,class Forward> Result ClaimedNativeEntryBoundary(const startbinding::Binding& binding,SlotKind kind,
    bool ownedReady,Claim claim,Forward forward){
    if(!ownedReady)return Result::Consumed;
    return NativeEntryBoundary(binding,kind,claim,forward);
}
// The native wrapper only invokes the graph Start entry. Claim the GRAPH,
// not both levels of that call chain: claiming the wrapper would consume its
// legitimate nested graph as a duplicate and silently prevent native travel.
template<class Claim,class Forward> Result GraphEntryBoundary(SlotKind kind,Claim claim,Forward forward){
    if(kind==SlotKind::Wrapper){forward();return Result::Forwarded;}
    return claim();
}
}
