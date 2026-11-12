#define CFG_EMOTE_MIRROR_MAIN PriorEmoteFixtureMain
#include "EmoteMirrorTests.cpp"
#undef CFG_EMOTE_MIRROR_MAIN
#include "DonorCounselorEmotes.hpp"
struct DonorAccess:Access {
    DonorRuntime* donor=nullptr;
    bool Dispatch(const Recipient& r,const Function& f,std::uintptr_t selected){
        if(donor)CHECK(!donor->Candidate(r.pawn.object,f.object,false));
        return Access::Dispatch(r,f,selected);
    }
    bool DispatchCancel(const Recipient& r,const Function& f){
        if(donor)CHECK(!donor->Candidate(r.pawn.object,f.object,false));
        return Access::DispatchCancel(r,f);
    }
};
static void StartDonor(DonorRuntime& runtime,DonorAccess& a,std::uint64_t now){
    a.montage[a.human.object]=0;
    runtime.Before(a,a.human.object,0x800000,a.catalogue.selected,false);
    CHECK(runtime.ObservedPlay(a.human.object,0x800000));
    a.montage[a.human.object]=a.catalogue.male.object;
    runtime.After(a,a.human.object,0x800000,false,true,now);
}
#ifndef CFG_DONOR_EMOTE_FIXTURE_ONLY
int main(){const auto prior=PriorEmoteFixtureMain();const unsigned before=failures;
    {
        DonorRuntime r;DonorAccess a;a.donor=&r;a.input.count=64;
        for(unsigned i=0;i<a.input.count;++i)a.input.bots[i]={Identity(0x1000000+0x1000*i),Identity(0x2000000+0x1000*i)};
        CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        CHECK(r.report.queued==64&&r.Pending()&&r.Selection().selected==a.catalogue.selected);
        r.Service(a,1024);CHECK(!a.calls);
        r.Service(a,1025);CHECK(a.calls==1&&r.report.started==1);
        r.Service(a,5000);CHECK(a.calls==2); // delayed frame NEVER drains remaining62
        for(unsigned i=2;i<64;++i)r.Service(a,5000+(i-1)*25);
        CHECK(a.calls==64&&r.report.started==64&&!r.Pending());
        CHECK(!r.InterruptPending(a.input.bots[0]));
        r.Service(a,26000);CHECK(a.cancelCalls==1); // tracked source expires first
        r.Service(a,26049);CHECK(a.cancelCalls==1);
        r.Service(a,26050);CHECK(a.cancelCalls==2); // one bot per50ms
        CHECK(r.playbackExpiries==2);
    }
    {
        DonorRuntime r;DonorAccess a;a.input.count=2;CHECK(!r.Prepare(a)&&r.Prepare(a));
        StartDonor(r,a,1000);StartDonor(r,a,1199);CHECK(r.deduplicated==1&&r.report.starts==1);
        StartDonor(r,a,1200);CHECK(r.report.starts==2);
        a.source=false;r.Service(a,1225);CHECK(a.calls==1); // human death does not lose copied world queue
        a.input.world+=1;r.Service(a,1250);CHECK(!r.Pending()&&a.calls==1&&r.worldCancels==1);
    }
    {
        DonorRuntime r;DonorAccess a;a.input.count=1;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        CHECK(!r.PumpPlayback(a,1249,true)&&!a.cancelCalls);
        CHECK(r.PumpPlayback(a,1299,true)&&a.cancelCalls==1); //50ms pump;250ms local input guard
    }
    {
        DonorRuntime r;DonorAccess a;a.input.count=1;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        a.assets=false;r.Service(a,1025);CHECK(!a.calls&&r.refusals[3]==1&&!r.Pending());
    }
    {
        DonorRuntime r;DonorAccess a;a.input.count=1;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        a.dispatch=false;r.Service(a,1025);CHECK(a.calls==1&&r.refusals[6]==1&&!r.Pending());
        a.dispatch=true;r.Service(a,1050);CHECK(a.calls==1); // unknown entered result never replayed
    }
    {
        DonorRuntime r;DonorAccess a;a.donor=&r;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        CHECK(r.CancelLocalForSelection(a,a.human)&&a.cancelCalls==1&&!r.Pending());
        CHECK(r.CancelLocalForSelection(a,a.human)&&a.cancelCalls==1); // consumed receipt, never cancel twice
    }
    {
        DonorRuntime r;DonorAccess a;a.donor=&r;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);
        a.montage[a.human.object]=0x123456;
        CHECK(r.CancelLocalForSelection(a,a.human)&&!a.cancelCalls); // unrelated native emote stays untouched
    }
    {
        DonorRuntime r;DonorAccess a;a.donor=&r;CHECK(!r.Prepare(a)&&r.Prepare(a));StartDonor(r,a,1000);a.dispatch=false;
        CHECK(!r.CancelLocalForSelection(a,a.human)&&a.cancelCalls==1);
        a.dispatch=true;CHECK(r.CancelLocalForSelection(a,a.human)&&a.cancelCalls==1); // no ambiguous replay
    }
    std::printf("Donor native-selection mirror: %u additional failures;64recipients,25ms/no-burst,200msdedup,world/death handling,250msmovement,25sexpiry/50mscancel,selection cancellation/asset/native refusal. Host supplied fixtures.\n",failures-before);
    return prior||failures?1:0;
}
#endif
