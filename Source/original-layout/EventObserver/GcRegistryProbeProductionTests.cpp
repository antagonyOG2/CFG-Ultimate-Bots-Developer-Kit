#define CFG_GC_REGISTRY_PROBE
#include "OfflineEventObserver.cpp"
#include <cstdio>
namespace {
unsigned forwards=0;
void __fastcall ForwardStub(void*,void*,void*){++forwards;}
}
int main(){
    originalActorDispatcher=&ForwardStub;
    plan.threadId=GetCurrentThreadId();plan.pawn=0x10000;
    // Arbitrary/foreign/nested actor callbacks never qualify as the pinned
    // native ReceiveTick return PC. The real production detour still forwards
    // each original once, with no lock attempts or engine calls.
    Observe(reinterpret_cast<void*>(0x10000),nullptr,nullptr);
    observerNested=true;Observe(reinterpret_cast<void*>(0x10000),nullptr,nullptr);observerNested=false;
    Observe(reinterpret_cast<void*>(0x20000),nullptr,nullptr);
    plan.threadId=0;Observe(reinterpret_cast<void*>(0x10000),nullptr,nullptr);
    const bool okay=forwards==4&&!gcRegistryReport.published&&!gcRegistryReport.calls&&
        gcRegistryOwner.state.load()==cfggcretain::State::Empty;
    std::printf("GC registry production forwarding/foreign/nested/non-tick refusal: %s; no game access\n",okay?"PASS":"FAIL");
    return okay?0:1;
}
