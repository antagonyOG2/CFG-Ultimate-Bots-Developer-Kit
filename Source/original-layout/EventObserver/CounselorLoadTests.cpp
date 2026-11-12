#include "../Core/CounselorLoadPolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failures=0;auto check=[&](bool value){++checks;if(!value)++failures;};
    cfgprimary::SoftClassBytes source{};source[16]=1;
    cfgcounselorload::Packet p;
    check(cfgcounselorload::Build(0x100000,0x200000,3,source,p));
    check(p.world==0x100000&&p.source==source&&p.latent.target==0x200000);
    check(p.onLoaded.index==-1&&p.onLoaded.serial==0&&p.onLoaded.function==0&&p.latent.linkage==-1&&p.latent.function==0);
    check(p.latent.uuid==0x43464703);
    check(!cfgcounselorload::Build(0,0x200000,3,source,p));
    check(!cfgcounselorload::Build(0x100000,0,3,source,p));
    check(!cfgcounselorload::Build(0x100000,0x200000,64,source,p));
    source[24]=1;check(!cfgcounselorload::Build(0x100000,0x200000,3,source,p)); // borrowed strings refused
    cfgcounselorload::Pending pending;
    check(!pending.Waiting(0));check(pending.Begin(7,100));check(pending.index==7&&pending.Waiting(30099));
    check(!pending.Begin(8,200));check(pending.index==7); // pending asset never advances catalogue
    check(!pending.Waiting(30100));check(pending.active&&!pending.Begin(8,30101)); // timeout never permits a second outstanding request
    pending.Reset();check(!pending.active); // explicit successful-resolution/world reset only
    check(pending.Begin(8,30101));check(pending.index==8);pending.Reset();check(!pending.Begin(64,0));
    std::printf("Counselor async load: %u checks, %u failures.\n",checks,failures);return failures?1:0;
}
