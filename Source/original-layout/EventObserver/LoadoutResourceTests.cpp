#include "../Core/LoadoutResourcePolicy.hpp"
#include "../Core/CounselorLoadPolicy.hpp"
#include <cstdio>
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    cfgloadoutresource::Pending p;check(p.Poll(3000));check(!p.Poll(3249));check(p.Poll(3250));
    check(p.Request(3250));check(!p.Request(3500));check(p.Poll(3500));check(!p.failed);
    check(!p.Poll(33250)&&p.failed);check(!p.Poll(34000));check(!p.Request(34000));
    cfgloadoutresource::Pending loaded;check(loaded.Poll(0));loaded.ready=true;check(!loaded.Poll(1000));check(!loaded.Request(1000));
    for(int i=0;i<64;++i)check(cfgloadoutresource::LatentUuid!=0x43464700+i);
    std::printf("CFG loadout async250ms/single30s/deadline/unique latent UUID: %s (%u failures).\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
