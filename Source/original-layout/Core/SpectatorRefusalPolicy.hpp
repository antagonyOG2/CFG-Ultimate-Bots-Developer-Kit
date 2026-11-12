#pragma once
#include <cstdint>
namespace cfgspectator {
// A successful/paused later callback must not erase the reason a camera
// publication actually refused. Diagnostics only; never camera authority.
struct RefusalReceipt {
    std::uint64_t at=0;
    std::uint32_t count=0,failure=0,decision=0,link=0,reads=0,calls=0,roster=0;
    std::uint32_t binding=0,resource=0,nativeBytes=0;
    void Capture(std::uint64_t now,unsigned refusals,unsigned nativeFailure,unsigned runtimeDecision,
        unsigned linkFailure,unsigned phaseReads,unsigned phaseCalls,unsigned rosterCount,
        unsigned bindingId=0,unsigned resourceFailure=0,unsigned observedBytes=0){
        if(refusals<=count)return;
        at=now;count=refusals;failure=nativeFailure;decision=runtimeDecision;
        link=linkFailure;reads=phaseReads;calls=phaseCalls;roster=rosterCount;
        binding=bindingId;resource=resourceFailure;nativeBytes=observedBytes;
    }
};
}
