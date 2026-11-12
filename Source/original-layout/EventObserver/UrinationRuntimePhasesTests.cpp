#include "../Core/UrinationRuntimePhases.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
struct Host {
    bool admitted=true,found=true,publicationDue=true;unsigned calls=0,fail=0,loads=0,publishes=0;
    cfgurination::LoadStage state=cfgurination::LoadStage::Empty;
    bool Admitted(){return admitted;}
    bool Next(){return ++calls!=fail;}
    bool Discover(){++calls;return found;}
    bool Configure(){return Next();}bool Install(){return Next();}
    bool PrepareDiscovered(std::uint64_t){state=cfgurination::LoadStage::Prepared;return Next();}
    bool LoaderFunction(bool){return Next();}bool MakePath(){return Next();}
    bool RequestLoad(){++loads;state=cfgurination::LoadStage::Waiting;return Next();}
    void Expire(std::uint64_t){++calls;}
    cfgurination::LoadStage LoadState(){return state;}
    bool PublicationDue(){return publicationDue;}
    bool PublishLoaded(){++publishes;return Next();}bool RetireRequestInputs(){return Next();}
};
}
int main(){using namespace cfgurination;
    {Host h;RuntimePhases p;for(unsigned i=0;i<8;++i)CHECK(p.Step(h,100ULL*i));
        CHECK(p.Stage()==RuntimeStage::Waiting&&h.loads==1&&h.calls==8);
        CHECK(!p.Step(h,750)&&h.calls==8);CHECK(p.Step(h,800)&&p.Stage()==RuntimeStage::Waiting);
        h.state=LoadStage::Ready;CHECK(p.Step(h,900)&&p.Stage()==RuntimeStage::Publish);
        h.publicationDue=false;CHECK(p.Step(h,1000)&&p.Stage()==RuntimeStage::Publish&&!h.publishes);
        h.publicationDue=true;CHECK(p.Step(h,1100)&&p.Step(h,1200)&&p.Stage()==RuntimeStage::Active&&h.publishes==1);
        const auto calls=h.calls;CHECK(!p.Step(h,1300)&&h.calls==calls&&h.loads==1);}
    for(unsigned failure=2;failure<=8;++failure){Host h;h.fail=failure;RuntimePhases p;
        for(unsigned i=0;i<12;++i)p.Step(h,100ULL*i);
        CHECK(p.Stage()==RuntimeStage::Refused&&h.calls==failure&&h.loads<2&&!h.publishes);}
    {Host h;RuntimePhases p;h.admitted=false;CHECK(!p.Step(h,0)&&!h.calls);h.admitted=true;h.found=false;
        CHECK(p.Step(h,0)&&p.Stage()==RuntimeStage::Discovery);h.found=true;CHECK(!p.Step(h,100));CHECK(p.Step(h,1000)&&p.Stage()==RuntimeStage::Configure);}
    {Host h;RuntimePhases p;for(unsigned i=0;i<8;++i)p.Step(h,100ULL*i);h.state=LoadStage::Rejected;
        CHECK(p.Step(h,800)&&p.Stage()==RuntimeStage::Refused&&h.loads==1&&!h.publishes);}
    std::printf("Urination native phase orchestration: %u failures; one operation/slice, admission/throttle, bounded catalogue wait, exact one load/publication, failure/timeout no native replay.\n",failures);
    return failures?1:0;
}
