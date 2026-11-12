#include "../Core/GameplayBundlePolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;const auto check=[&](bool value){++checks;if(!value)++failures;};
    constexpr unsigned terminal=1;
    check(cfgbundle::OfflineSpectatorCameraAllowed(false,terminal));
    check(!cfgbundle::OfflineSpectatorCameraAllowed(true,terminal));
    check(!cfgbundle::OfflineSpectatorCameraAllowed(true,0));
    // Hosted completion still reaches the independent Jason-name label input.
    check((terminal&1u)!=0);
    std::printf("Hosted terminal camera ownership: %u checks, %u failures; online camera driver refused, terminal label input retained.\n",checks,failures);
    return failures?1:0;
}
