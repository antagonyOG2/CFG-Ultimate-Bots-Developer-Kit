#include "RootPublicationProbe.hpp"
#include <cstdio>
int main(){unsigned failures=0;
    for(unsigned fault=0;fault<12;++fault){cfgrootprobe::Report r;
        r.stage=static_cast<unsigned>(cfgrootprobe::Stage::Observed);r.module=0x100000;r.owner=0x200000;
        r.world=0x30000;r.human=0x40000;r.registrations=1;r.swaps=1;r.callbacksAtSwap=1;r.submissionsAtSwap=1;
        cfggcprobe::OwnerObservation o;o.state=cfggcretain::State::Registered;
        o.slots[0]=o.expected[0]=r.world;o.slots[1]=o.expected[1]=r.human;o.callbacks=o.completed=2;o.submissions=3;
        unsigned unknown=0;
        if(fault==1)r.stage=static_cast<unsigned>(cfgrootprobe::Stage::Swapped);
        if(fault==2)r.callbacksAtSwap=0;
        if(fault==3)o.callbacks=3;
        if(fault==4)o.submissions=2;
        if(fault==5)o.slots[0]=0;
        if(fault==6)o.cleared=1;
        if(fault==7)o.destroyRequests=1;
        if(fault==8)unknown=1;
        if(fault==9)o.slots[2]=o.expected[2]=0x50000;
        if(fault==10)r.reason=1;
        if(fault==11)r.callbacksAtHumanPublication=1;
        if(cfgrootprobe::Preserved(r,o,unknown)!=(fault==0))++failures;
    }
    std::printf("Synchronized publication report before/after real-callback requirements: %u failures; synthetic observations only\n",failures);
    return failures?1:0;
}
