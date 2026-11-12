#include "../Core/NativeOfflineLifecycle.hpp"
#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include <cassert>
#include <string>
#include <cstdio>
using namespace cfgnativelifecycle;
struct Host {
    std::string order;bool current=true,human=true,ai=true,match=false;
    unsigned calls=0,failAt=0,restores=0;Transaction* transaction=nullptr;
    bool CurrentNativeOfflineOwner(std::uint64_t session){return current&&session==1;}
    Result Call(char c){order+=c;return ++calls==failAt?Result::Failed:Result::Done;}
    Result SpawnNativeCounselor(unsigned){return Call('B');}
    Result BirthSelectedCounselor(){return Call('H');}
    bool HumanCounselorPossessionIntact(){return human;}
    Result PrepareSelectedJasonIntro(){return Call('M');}
    bool CallOfflineParentIntroOnce(){if(transaction)assert(!transaction->Step(*this));return Call('I')==Result::Done;}
    bool NativeMatchStartCallback(){return match;}
    bool CallOfflineParentMatchStartOnce(){if(transaction)assert(!transaction->Step(*this));return Call('S')==Result::Done;}
    Result SpawnIndependentAIJason(){return Call('J');}
    Result TransferKillerOwnershipToAI(){return Call('T');}
    Result RestoreHumanCounselorMetadata(){return Call('R');}
    bool AIKillerOwnershipIntact(){return ai;}
    void RestoreOwnedIntroMetadata(std::uint64_t){++restores;}
};
int main(){
    {Transaction t;Host h;CallbackRouter router(t);assert(t.Arm({1,4}));
     assert(router.Intro(h,false)==CallbackResult::ForwardOriginal&&!h.calls);
     assert(router.Intro(h,true)==CallbackResult::Consumed&&h.order=="BBBBHMI");
     assert(router.Intro(h,true)==CallbackResult::Consumed&&h.order=="BBBBHMI");
     h.match=true;assert(router.MatchStart(h,true)==CallbackResult::Consumed&&h.order=="BBBBHMISJTR");
     assert(router.MatchStart(h,true)==CallbackResult::Consumed&&h.order=="BBBBHMISJTR");}
    {Transaction t;Host h;CallbackRouter router(t);assert(t.Arm({1,4}));
     assert(router.MatchStart(h,true)==CallbackResult::Refused&&!h.calls);}
    {Transaction t;Host h;h.transaction=&t;assert(t.Arm({1,4}));
     for(unsigned i=0;i<8;++i)assert(t.Step(h));
     assert(h.order=="BBBBHMI"&&t.State()==Stage::AwaitMatch);
     for(unsigned i=0;i<100;++i)assert(t.Step(h));assert(h.order=="BBBBHMI");
     h.match=true;for(unsigned i=0;i<6;++i)assert(t.Step(h));
     assert(t.State()==Stage::Ready&&h.order=="BBBBHMISJTR"&&t.Bots()==4&&!h.restores);
     assert(!t.Arm({2,4}));assert(t.Step(h)&&h.order=="BBBBHMISJTR");}
    for(unsigned failure=1;failure<=8;++failure){Transaction t;Host h;h.failAt=failure;h.match=true;assert(t.Arm({1,1}));
     for(unsigned i=0;i<15&&t.State()!=Stage::Refused;++i)t.Step(h);
     assert(t.State()==Stage::Refused);const auto calls=h.calls;assert(!t.Step(h)&&h.calls==calls);}
    {Transaction t;Host h;assert(t.Arm({1,0}));t.Step(h);h.human=false;assert(!t.Step(h)&&t.State()==Stage::Refused);}
    {Transaction t;Host h;assert(t.Arm({1,0}));h.current=false;assert(!t.Step(h)&&!h.calls);}
    {Transaction t;assert(!t.Arm({0,4})&&!t.Arm({1,32}));}
    std::puts("Native OfflineBots donor ordering/reentry/refusal tests PASS; native adapter not enabled.");
}
