#include "../RuntimeProbe/CollectorSubmitObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
int main(){unsigned failures=0;
    const auto check=[&](bool value){if(!value)++failures;};
    constexpr std::uintptr_t module=0x100000;
    for(unsigned kind=0;kind<2;++kind){OwnedByteReader reader;
        const auto target=module+(kind?cfgcollectorsubmit::Serial:cfgcollectorsubmit::Parallel);
        if(kind)reader.Set(target,cfgcollectorsubmit::SerialCode);else reader.Set(target,cfgcollectorsubmit::ParallelCode);
        check(cfgcollectorsubmit::Known(reader,module,target));
        // Includes late cluster branches, not just prologue/pending-kill clear.
        const auto extent=kind?cfgcollectorsubmit::SerialCode.size():cfgcollectorsubmit::ParallelCode.size();
        reader.Set(target+extent-2,std::uint8_t{0});
        check(!cfgcollectorsubmit::Known(reader,module,target));
    }
    OwnedByteReader unrelated;check(!cfgcollectorsubmit::Known(unrelated,module,module+0x100));
    check(!cfgcollectorsubmit::Known(unrelated,0,cfgcollectorsubmit::Parallel));
    check(!cfgcollectorsubmit::MutableOwnerRuntimeApproved);
    std::printf("Complete collector Submit code/unknown-target/no-runtime-approval observation: %u failures; owned bytes only\n",failures);
    return failures?1:0;
}
