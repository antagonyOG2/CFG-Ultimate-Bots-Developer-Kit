#define CFG_SPAWN_ADAPTER
#define CFG_RETAINED_COUNSELOR_SPAWN
#include "OfflineEventObserver.cpp"
#include <cstdio>
namespace {
unsigned forwards=0;
void __fastcall ForwardStub(void*,void*,void*){++forwards;}
}
int main(){
    originalActorDispatcher=&ForwardStub;
    plan.threadId=GetCurrentThreadId();plan.pawn=0x10000;
    const cfgspawn::Identity identity{0x20000,plan.pawn,1,2};
    if(!spawnRuntime.Configure(identity,true,1,0x30000,0x40000,0x50000))return 1;
    cfgspawn::SpawnRequest request;request.identity=identity;
    if(!spawnRuntime.Submit(request))return 1;
    SpawnAccess access;cfgspawn::Vec3 location;
    const bool noDirectCalls=!access.RetainInputs()&&!access.InputsRetained()&&!access.Location(location)&&!access.Calls();
    Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);
    observerNested=true;Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);observerNested=false;
    Observe(reinterpret_cast<void*>(0x20000),nullptr,nullptr);
    plan.threadId=0;Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);
    const bool okay=noDirectCalls&&forwards==4&&spawnRuntime.Pending()&&!spawnRuntime.report.attempts&&
        !gcRegistryReport.calls&&gcRegistryOwner.state.load()==cfggcretain::State::Empty;
    std::printf("Retained counselor production direct/foreign/nested/non-tick refusal: %s; own-process only\n",okay?"PASS":"FAIL");
    return okay?0:1;
}
