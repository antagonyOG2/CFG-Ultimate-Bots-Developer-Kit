#define CFG_SPAWN_ADAPTER
#define CFG_JASON_APPROACH
#define CFG_PRIMARY_JASON
#define CFG_RETAINED_PRIMARY_JASON
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
    if(!spawnRuntime.Configure(identity,true,1,plan.pawn,0))return 1;
    cfgspawn::SpawnRequest request;request.identity=identity;
    if(!spawnRuntime.Submit(request))return 1;
    Reader reader;PrimaryStartHost host(reader);SpawnAccess access;cfgspawn::Vec3 location;
    std::uintptr_t result=0;
    const bool noDirectCalls=!PrimaryRootsCurrent(reader)&&!host.InitializeClass(true,result)&&
        !access.Location(location)&&!access.ContinueNavigation()&&!access.Calls();
    Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);
    observerNested=true;Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);observerNested=false;
    Observe(reinterpret_cast<void*>(0x20000),nullptr,nullptr);
    plan.threadId=0;Observe(reinterpret_cast<void*>(plan.pawn),nullptr,nullptr);
    const bool okay=noDirectCalls&&forwards==4&&spawnRuntime.Pending()&&!spawnRuntime.report.attempts&&
        !gcRegistryReport.calls&&!primaryRoots.Count()&&!primaryRoots.Poisoned();
    std::printf("Retained Jason production direct/foreign/nested/non-tick refusal: %s; own-process only\n",okay?"PASS":"FAIL");
    return okay?0:1;
}
