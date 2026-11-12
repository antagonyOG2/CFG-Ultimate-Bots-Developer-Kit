#include "../Core/HostedOptions.hpp"
#include <cstdio>
int main(){
    unsigned checks=0,failures=0;
    const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    check(cfghostedoptions::WaitingForMatchBadge(true,false,false,false,false));
    check(!cfghostedoptions::WaitingForMatchBadge(false,false,false,false,false));
    check(!cfghostedoptions::WaitingForMatchBadge(true,true,false,false,false));
    check(!cfghostedoptions::WaitingForMatchBadge(true,false,true,false,false));
    check(!cfghostedoptions::WaitingForMatchBadge(true,false,false,true,false));
    check(!cfghostedoptions::WaitingForMatchBadge(true,false,false,false,true));
    std::printf("H225 primary badge preference: %u checks, %u failures; display-only.\n",checks,failures);
    return failures?1:0;
}
