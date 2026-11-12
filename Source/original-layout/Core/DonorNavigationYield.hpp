#pragma once
namespace cfgdonorhunt {
// Stock donor calls are synchronous. Only the3.5 native-budget adapter exposes
// a pending continuation. Ordinary donor hosts retain their original behavior.
template<class Host>auto DonorNavigationPending(Host& host,int)->decltype(host.NavigationPending(),bool{}){
    return host.NavigationPending();
}
template<class Host>bool DonorNavigationPending(Host&,long){return false;}
}
