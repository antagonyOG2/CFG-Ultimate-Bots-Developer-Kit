#define CFG_H1_JASON
#include <Windows.h>
#include <cstdint>
#include <cstdio>
struct Reader{unsigned cap=1536,reads=0;bool Spend(unsigned n){reads+=n;return reads<=cap;}};
namespace cfgprobe{constexpr unsigned WorldGameState=0xF8;}
namespace cfgobserver{struct Plan{std::uintptr_t module=0x10000,world=0x20000,mode=0x30000,controller=0x40000;unsigned threadId=GetCurrentThreadId();int objectArray=0;};
 bool state=true;bool Same(Reader& r,std::uintptr_t,std::uintptr_t){return r.Spend(5)&&state;}}
cfgobserver::Plan plan;
enum class BundleManagerGateResult:unsigned{Ready,Deferred,Failed};
template<class Observe>BundleManagerGateResult BundleManagerGate(std::uint64_t,bool hosted,Observe&& observe){
    if(observe())return BundleManagerGateResult::Ready;return hosted?BundleManagerGateResult::Deferred:BundleManagerGateResult::Failed;}
unsigned recordedManagerStage=0,recordedManagerReads=0,recordedManagerCap=0;
void RecordBundleManagerFailure(unsigned stage,unsigned reads,unsigned cap){recordedManagerStage=stage;recordedManagerReads=reads;recordedManagerCap=cap;}
bool hosted=true,world=true,adopted=true,manager=true,veto=true,replace=false;
unsigned freshReads=0,managerCalls=0;
bool HostedRoutineActive(){return hosted;}
namespace cfgprimary{bool LookupVetoClearForObservation(Reader& r,std::uintptr_t){return r.Spend(8)&&veto;}
 bool RetainedWorld(Reader& r,const cfgobserver::Plan& p,cfgobserver::Plan& out){out=p;if(replace)++out.controller;return r.Spend(600)&&world;}}
int hostedAdoptionReceipt=1,bundleManagerClass=2,bundleManagerClassSlot=3;
namespace cfghosted::adoption{bool Current(Reader& r,int,int,const cfgobserver::Plan&){return r.Spend(80)&&adopted;}}
namespace cfgbundle{bool Manager(Reader& r,std::uintptr_t,int,std::uintptr_t,int,int,unsigned* failedClause=nullptr){++managerCalls;const bool okay=r.Spend(40)&&manager;freshReads=r.reads;if(failedClause)*failedClause=okay?0u:9u;return okay;}}
struct{std::uintptr_t gameState=0x50000;}spawnAssets;
struct{struct{std::uintptr_t object=0x60000;}pawn;}bundleJason;
struct Fixture{Reader r_;struct{int objectArray=0;}context;
#include "BundleManagerObservationMembers.hpp"
};
int main(){unsigned checks=0,failed=0;auto check=[&](bool p){++checks;if(!p)++failed;};
 Fixture f;f.r_.reads=f.r_.cap;check(f.Manager()&&freshReads<=768&&f.r_.reads==f.r_.cap);
 // Repeated post-trap/carry/car opportunities cannot spend the old reader or
 // make a healthy hosted proof fail solely because that phase was exhausted.
 for(unsigned i=0;i<100;++i)check(f.Manager());
 manager=false;check(!f.Manager()&&recordedManagerStage==209&&recordedManagerReads>0&&recordedManagerCap==768);manager=true;
 world=false;check(!f.Manager());world=true;
 adopted=false;check(!f.Manager());adopted=true;
 cfgobserver::state=false;check(!f.Manager());cfgobserver::state=true;
 replace=true;check(!f.Manager());replace=false;
 veto=false;check(!f.Manager());veto=true;
 ++plan.threadId;check(!f.Manager());--plan.threadId;
 hosted=false;check(!f.Manager());f.r_.reads=0;check(f.Manager()&&f.r_.reads==40);
 std::printf("H218 actual manager member: %u checks, %u failures; fresh hosted budget, invalid identity refusal, offline original reader.\n",checks,failed);return failed?1:0;}
