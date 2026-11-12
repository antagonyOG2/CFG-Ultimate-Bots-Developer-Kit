#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define main HelperBirthSparseFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "PendingCounselorBirth.hpp"
#include "../Core/DonorReinforcementQueue.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/ResidentCounselorSelection.hpp"
namespace cfgclosedtrace {inline void Mark(const char*,std::uintptr_t,std::uint64_t) {}}
struct BirthFixture {
    Fixture r_;cfgobserver::Plan plan{},context{};
    cfghelperbirth::PendingCounselorBirth bundleHelperBirth;
    cfgbundle::HelperStartup bundleHelpers;
    cfgbundle::Report bundleReport;
    cfgbundle::Schedule bundleSchedule;
    cfgbundle::RandomCounselors bundleRandomCounselors;
    cfgresidentcounselor::Pass bundleHelperResidentPass;
    cfgspawn::TemplateObservation bundleHelperTemplate;
    struct {cfgspawn::ResourceReport resources;std::uintptr_t gameState=0;} spawnAssets;
    struct {cfgprobe::lifetime::SlotObservation pawn;} bundleJason;
    std::array<cfgprobe::lifetime::SlotObservation,cfgbundle::MaximumBots> bundlePawnSlots{},bundleControllerSlots{};
    std::uintptr_t bundleCounselorClass=0;
    bool bundleHelperSelected=true,bundleHelperReinforcement=true,scope=true,dispatch=true;
    unsigned bundleHelperLocationAttempts=0,bundleHelperStaleRetries=0,calls=0;
    std::uintptr_t nativeResult=0;
    BirthFixture(){r_.cap=10000;
        plan.module=r_.module;plan.world=0x12000000;plan.pawn=r_.pawn;plan.controller=0x50000000;
        plan.objectArray=r_.objects;context=plan;
        r_.Put(plan.module+cfgprobe::lifetime::ObjectArrayRva+0x10,r_.objects.data);
        r_.Put(plan.module+cfgprobe::lifetime::ObjectArrayRva+0x1C,r_.objects.count);
        r_.Put(plan.world+cfgprobe::WorldGameState,r_.state);
        spawnAssets.resources.classes=r_.classes;spawnAssets.resources.helperDefault=1;spawnAssets.resources.spawnFunction=2;
        spawnAssets.gameState=r_.state;bundleCounselorClass=r_.counselorBase;
        bundleHelperTemplate.pawnClass=r_.botClass;nativeResult=r_.bot;bundleJason.pawn.object=r_.pawn;
        for(const auto object:{r_.bot,r_.botController}){std::int32_t i=0;CHECK(r_.Read(object+0xC,i));
            r_.Put(r_.objects.data+24ULL*i+16,std::int32_t{40+i});}
        bundleHelpers.stage=cfgbundle::HelperStage::Spawn;bundleSchedule.requestedBots=1;
        r_.reads=0;r_.cap=1536;
    }
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* raw,bool){
        CHECK(object==1&&function==2);++calls;static_cast<cfgspawn::SpawnAIParameters*>(raw)->result=nativeResult;return dispatch;}
    bool HelperContext(){for(unsigned i=0;i<350;++i){std::uint32_t v=0;if(!r_.Read(r_.pawn,v))return false;}return scope;}
    bool Location(std::uintptr_t,cfgspawn::Vec3& out){out={10,20,90};return true;}
    unsigned Reads()const{return r_.reads;}
#include "HelperBirthRuntimeMembers.hpp"
};
int main(){
    for(unsigned primaryRoute=0;primaryRoute<2;++primaryRoute){BirthFixture f;cfgreinforcement::Counselors queue;
        f.bundleSchedule.active=true;f.bundleSchedule.requestedBots=f.bundleSchedule.spawned=0;
        f.bundleHelpers={};f.bundleReport.spawned=f.bundleReport.bots=0;
        CHECK(queue.Request(0)&&queue.Ready(0,0)); // one admitted F3 press in either primary route
        CHECK(f.bundleHelpers.Reinforcement(0,0,cfgbundle::MaximumBots)&&f.bundleSchedule.AdditionalHelper(1000));
        queue.Started();CHECK(f.bundleSchedule.requestedBots==1&&!queue.pending);
        CHECK(f.bundleHelpers.Projected({0,0,25},{0,0,0}));
        cfgspawn::SpawnAIParameters p;p.location={0,0,90};f.r_.reads=0;f.DispatchHelperBirth(p);
        CHECK(f.bundleReport.spawnAttempts==1&&f.bundleReport.helperDispatched==1&&f.calls==1&&f.bundleHelperBirth.pending);
        f.r_.reads=0;f.FinishHelperBirth();CHECK(f.bundleReport.bots==1&&f.bundleSchedule.spawned==1);
    }
    {BirthFixture f;cfgspawn::SpawnAIParameters p;p.location={2000,2000,90};
        f.r_.reads=1452;f.DispatchHelperBirth(p);
        CHECK(f.calls==1&&f.bundleHelperBirth.pending&&!f.bundleReport.bots&&f.bundleHelpers.Active());
        CHECK(f.r_.reads<=1536);std::printf("Actual dispatch + positive birth journal with captured pre-proof debt:%u/1536 reads.\n",f.r_.reads);
        f.r_.reads=0;f.FinishHelperBirth();
        CHECK(f.bundleReport.bots==1&&f.bundleReport.spawned==1&&f.bundleSchedule.spawned==1);
        CHECK(!f.bundleHelperBirth.pending&&f.bundleHelpers.stage==cfgbundle::HelperStage::Complete&&f.calls==1);
        CHECK(f.r_.reads<1536);std::printf("Actual deferred reciprocal observation:%u/1536 reads; scope helper cost mocked350 twice.\n",f.r_.reads);}
    {BirthFixture f;cfgspawn::SpawnAIParameters p;f.r_.reads=1536-63;f.DispatchHelperBirth(p);
        CHECK(!f.calls&&f.bundleHelpers.stage==cfgbundle::HelperStage::Unavailable);}
    {BirthFixture f;cfgspawn::SpawnAIParameters p;f.nativeResult=0;f.DispatchHelperBirth(p);
        CHECK(f.calls==1&&!f.bundleHelperBirth.pending&&f.bundleHelpers.stage==cfgbundle::HelperStage::Project);}
    {BirthFixture f;cfgspawn::SpawnAIParameters p;f.dispatch=false;f.DispatchHelperBirth(p);
        CHECK(f.calls==1&&!f.bundleHelperBirth.pending&&f.bundleHelpers.stage==cfgbundle::HelperStage::Indeterminate);}
    for(unsigned test=0;test<7;++test){BirthFixture f;cfgspawn::SpawnAIParameters p;f.DispatchHelperBirth(p);CHECK(f.bundleHelperBirth.pending);
        std::int32_t index=0;CHECK(f.r_.Read(f.r_.bot+0xC,index));
        if(test==0)f.r_.Put(f.r_.objects.data+24ULL*index+16,std::int32_t{999}); // recycled address
        if(test==1)f.r_.Put(f.r_.bot+cfgprobe::ObjectClass,f.r_.pawnClass);
        if(test==2)f.r_.Put(f.r_.botController+0x370,f.r_.pawn);
        if(test==3)f.r_.Put(f.r_.botController+0x418,std::uintptr_t{});
        if(test==4)f.scope=false;
        if(test==5)f.bundleHelperBirth.deadline=0;
        if(test==6)++f.plan.world;
        f.r_.reads=0;f.FinishHelperBirth();
        CHECK(!f.bundleReport.bots&&f.bundleHelpers.stage==cfgbundle::HelperStage::Indeterminate&&f.calls==1);}
    {BirthFixture f;cfgspawn::SpawnAIParameters p;std::int32_t index=0;CHECK(f.r_.Read(f.r_.bot+0xC,index));
        f.r_.Put(f.r_.objects.data+24ULL*index+16,std::int32_t{});f.DispatchHelperBirth(p);
        CHECK(!f.bundleHelperBirth.pending&&f.bundleHelpers.stage==cfgbundle::HelperStage::Indeterminate);}
    {BirthFixture f;cfghelperbirth::PendingCounselorBirth birth;
        CHECK(birth.Capture(f.r_,f.plan,f.r_.bot,f.r_.botClass,{0,0,90},100));
        CHECK(birth.Current(f.r_,f.plan,3100)&&!birth.Current(f.r_,f.plan,3101));}
    std::printf("Included helper birth dispatch/finalization:%s (%d failures); no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
