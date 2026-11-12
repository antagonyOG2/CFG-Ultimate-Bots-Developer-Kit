#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <cstdint>
#include <cstdio>
namespace {
struct Reader {inline static bool eligible=true,world=true;unsigned cap=0,reads=0;
    bool Read(std::uintptr_t at,std::uintptr_t& value){
        if(reads>=cap)return false;++reads;
        if(at==0x40100){value=0x70000;return true;}
        if(at==0x40108){value=0x80000;return true;}
        return false;
    }
};
struct Context {};
struct {std::uintptr_t world=0x10000,mode=0x20000,module=0x100000,controller=0xDEAD0000;DWORD threadId=GetCurrentThreadId();}plan;
std::uintptr_t originalActorDispatcher=1,primaryRootTickFunction=1;
struct Lease {bool Current(){return true;}void Uncertain(){}}lease,*bundleRootLease=&lease;
bool PrepareBundleCallbackCall(unsigned){return true;}
bool EnterBundleCallbackCall(){return true;}
bool CancelBundleCallbackCall(){return true;}
struct {struct {std::uintptr_t object;}pawn{0x40000},killerController{0x50000},path{0x60000};}bundleJason;
struct {std::uintptr_t gameState=0x30000;std::array<std::uintptr_t,4> functions{};
    struct {std::uintptr_t spawnFunction=0;}resources;}spawnAssets;
namespace cfghunter::disk {constexpr std::uintptr_t Manager=0x100,Held=0x108;}
namespace cfgprimary {
bool LookupSafe(Reader& r,std::uintptr_t){return r.eligible&&++r.reads<=r.cap;}
bool RetainedOfflineWorld(Reader& r,decltype(plan)&,Context&){return ++r.reads<=r.cap&&r.world;}
}
namespace cfgspawn {
struct MoveParameters {std::uintptr_t goal=0,filterClass=0;};
struct SpawnAIParameters {std::uintptr_t world=0,pawnClass=0,behaviorTree=0,result=0;};
}
namespace cfgretainframe {
enum class Phase {Inputs,Outputs};
template<class R>struct RuntimeHost {RuntimeHost(R&,std::uintptr_t){}};
template<class Host>bool Observe(Host&,std::uintptr_t,std::uintptr_t,std::array<std::uintptr_t,32>&,Phase=Phase::Inputs){return false;}
template<class Host>bool MaximumOutputRoots(Host&,std::uintptr_t,unsigned&){return false;}
}
template<std::size_t N>bool CompleteBundleCallbackCall(Reader&,const std::array<std::uintptr_t,N>&,unsigned=N){return true;}
std::array<std::uintptr_t,32> capturedActorRoots{};unsigned calls=0;
template<std::size_t N>bool RetainPrimaryObjects(Reader&,const std::array<std::uintptr_t,N>& input){
    ++calls;capturedActorRoots={};for(unsigned i=0;i<N;++i)capturedActorRoots[i]=input[i];return true;
}
struct ProductionMembers {
    Reader r_;
    Context context;
    bool RetainTypedSoftClassFrame(std::uintptr_t,std::uintptr_t,void*,bool& matched){matched=false;return true;}
    bool CompleteTypedSoftClassFrame(void*,bool& matched){matched=false;return true;}
#include "BundleRetentionRuntimeMembers.hpp"
};
}
int main(){unsigned failures=0;ProductionMembers access;
    access.r_.cap=1536;access.r_.reads=1536;
    const auto check=[&](bool okay){if(!okay)++failures;};
    check(access.RetainBundleActors(0x90000)&&calls==1);
    check(access.bundleActorStage_==0&&access.bundleEnvironmentStage_==0);
    const std::array<std::uintptr_t,9> expected{0x10000,0x20000,0x30000,0x90000,0x40000,0x50000,0x60000,0x70000,0x80000};
    for(unsigned i=0;i<expected.size();++i)check(capturedActorRoots[i]==expected[i]);
    for(auto root:capturedActorRoots)check(root!=plan.controller);
    // Post-call completion/environment does not consume the already exhausted
    // feature AI reader. Independent bounded proof owns its own read allowance.
    check(access.BundleRetentionEnvironment()&&access.FinishBundleNative()&&access.r_.reads==1536);
    access.r_.world=false;check(!access.RetainBundleActors(0x90000)&&calls==1);
    check(access.bundleActorStage_==2);
    access.r_.world=true;access.r_.eligible=false;check(!access.RetainBundleActors(0x90000)&&calls==1);
    check(access.bundleActorStage_==1&&access.bundleEnvironmentStage_==7);
    std::printf("Existing actual actor-retention member: %u failures; receiver/current AI roots, no universal old-human/target dependency; owned model only\n",failures);
    return failures?1:0;
}
