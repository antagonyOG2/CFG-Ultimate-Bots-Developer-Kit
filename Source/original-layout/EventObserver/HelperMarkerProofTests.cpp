#define main ExistingMarkerFixtureMain
#include "KillerStartObservationTests.cpp"
#undef main
#include "CounselorStartObservation.hpp"
namespace {
Reader* source=nullptr;bool allowed=true;
cfgcounselorstart::Observation bundleCounselorStarts;
struct BoundedReader {
    unsigned reads=0,cap=1536;
    template<class T>bool Read(std::uintptr_t at,T& out){if(!source||reads>=cap)return false;++reads;return source->Read(at,out);}
};
struct ProofAccess {
    using BundleAccess=ProofAccess;
    BoundedReader r_;
    unsigned Reads()const{return r_.reads;}unsigned Calls()const{return 0;}
    bool Current(){std::uintptr_t world=0;for(unsigned i=0;i<350;++i)if(!r_.Read(Fixture::Levels,world))return false;return allowed;}
#include "HelperMarkerProofRuntimeMembers.hpp"
};
}
int main(){unsigned failed=0;const auto check=[&](bool b){if(!b)++failed;};
    Fixture f;source=&f.reader;
    f.reader.Set(f.Module+cfgcounselorstart::ClassCache,f.Type);f.reader.Set(f.Module+cfgcounselorstart::ConstructorCache,f.Type);
    check(bundleCounselorStarts.Begin(f.reader,f.host,f.Module,f.World));
    for(unsigned i=0;i<20&&bundleCounselorStarts.CurrentState()==cfgcounselorstart::State::Scanning;++i)check(bundleCounselorStarts.Step(f.reader,f.host));
    check(bundleCounselorStarts.CurrentState()==cfgcounselorstart::State::Ready);
    cfgcounselorstart::Marker marker;
    // Reproduce the live shared-budget failure with the actual metadata path,
    // including two full local-owner checks per Current (cost350 each).
    ProofAccess stacked;stacked.r_.reads=530;
    check(bundleCounselorStarts.Current(stacked.r_,stacked,0,marker));
    check(!bundleCounselorStarts.Current(stacked.r_,stacked,0,marker)&&stacked.Reads()==1536);
    ProofAccess native;native.r_.reads=530;
    check(native.HelperMarkerCurrent(0,marker)&&marker.actor==f.Actor);
    const auto proofReads=native.helperMarkerProofReads_;
    check(native.HelperMarkerCurrent(0,marker)&&native.Reads()==530&&proofReads<1536&&native.helperMarkerProofReads_==proofReads);
    check(!native.Calls());
    allowed=false;check(!native.HelperMarkerCurrent(0,marker));allowed=true;
    f.reader.Set(f.Table+16,int{43});check(!native.HelperMarkerCurrent(0,marker));f.reader.Set(f.Table+16,int{42});
    f.reader.Set(f.Level+cfgkill::disk::OwningWorld,f.Other);check(!native.HelperMarkerCurrent(0,marker));f.reader.Set(f.Level+cfgkill::disk::OwningWorld,f.World);
    f.reader.Set(f.Actors+160,std::uintptr_t{});check(!native.HelperMarkerCurrent(0,marker));f.reader.Set(f.Actors+160,f.Actor);
    f.reader.Set(f.Module+cfgcounselorstart::ConstructorCache,f.Other);check(!native.HelperMarkerCurrent(0,marker));
    std::printf("Actual marker proof/member:%u failures; legacy shared1536 exhaustion reproduced; fresh bounded proof%u/1536; native reader unchanged530, no native calls.\n",failed,proofReads);
    return failed?1:0;
}
