#include "NativeOfflineLifecycleHooks.hpp"
#include <cstdio>
#include <vector>
namespace h=cfgmenu::offlinehooks;
void __fastcall Intro(void*){}void __fastcall Match(void*){}
struct Backend {
    bool ready=true,pin=true,pristine=true;int createFail=0,queueFail=0,applyFail=0,removeFail=0;
    unsigned creates=0,queues=0,applies=0,removes=0;std::vector<bool> queued;
    bool Ready(const h::Request& r){return ready&&r.outsideLoaderLock&&r.exclusiveBackend&&r.exactImageVerified;}
    bool Pin(){return pin;}bool Pristine(){return pristine;}int Initialize(){return MH_ERROR_ALREADY_INITIALIZED;}
    int Create(std::uintptr_t target,h::Handler handler,h::Handler& original){++creates;
        if(target!=0x100000+(creates==1?cfgmenu::offlinelifecycledisk::OfflineBotsIntro958:cfgmenu::offlinelifecycledisk::OfflineBotsMatchStarted818))return MH_UNKNOWN;
        if(static_cast<int>(creates)==createFail)return MH_ERROR_MEMORY_ALLOC;original=handler;return MH_OK;}
    bool Trampoline(h::Handler p){return p!=nullptr;}
    int Queue(std::uintptr_t,bool value){++queues;queued.push_back(value);return static_cast<int>(queues)==queueFail?MH_ERROR_NOT_CREATED:MH_OK;}
    int Apply(){++applies;return applyFail==-1||static_cast<int>(applies)==applyFail?MH_ERROR_MEMORY_PROTECT:MH_OK;}
    int Remove(std::uintptr_t){++removes;return removeFail?MH_ERROR_MEMORY_PROTECT:MH_OK;}
};
int main(){unsigned failures=0;const auto check=[&](bool p,const char* s){if(!p){++failures;std::printf("FAIL %s\n",s);}};
    h::Request r{0x100000,0x200000,Intro,Match,true,true,true};
    {Backend b;h::Pair<Backend> p(b);check(!p.Enabled()&&!p.Enable(),"no default activation");check(p.Prepare(r)&&p.Prepared()&&!p.Enabled(),"prepare only");
     check(p.OriginalIntro()==Intro&&p.OriginalMatch()==Match&&b.applies==0,"originals available before enable");
     check(p.Enable()&&p.Enabled()&&b.applies==1&&b.queues==2,"one paired queue transaction");check(!p.Enable()&&!p.Prepare(r),"no repeat installation");}
    {Backend b;b.createFail=2;h::Pair<Backend> p(b);check(!p.Prepare(r)&&p.Status()==h::State::Refused&&b.removes==1&&!p.OriginalIntro(),"definite prepublication failure removes disabled first hook");}
    {Backend b;b.createFail=2;b.removeFail=1;h::Pair<Backend> p(b);check(!p.Prepare(r)&&p.Quarantined()&&p.OriginalIntro(),"ambiguous removal retains resident original");}
    {Backend b;b.queueFail=2;h::Pair<Backend> p(b);check(p.Prepare(r)&&!p.Enable()&&p.Status()==h::State::DisabledResident&&b.applies==0&&b.queues==4,"queue failure canceled before any apply");}
    {Backend b;b.applyFail=1;h::Pair<Backend> p(b);check(p.Prepare(r)&&!p.Enable()&&p.Status()==h::State::DisabledResident&&b.applies==2&&b.removes==0&&p.OriginalIntro(),"partial apply rolls back but retains published trampolines");}
    {Backend b;b.applyFail=-1;h::Pair<Backend> p(b);check(p.Prepare(r)&&!p.Enable()&&p.Quarantined()&&b.removes==0,"ambiguous rollback quarantines without unloading");}
    {Backend b;h::Pair<Backend> p(b);check(p.Prepare(r),"pristine fixture");b.pristine=false;check(!p.Enable()&&b.queues==0,"changed full target refuses enable");}
    {Backend b;h::Pair<Backend> p(b);r.outsideLoaderLock=false;check(!p.Prepare(r)&&b.creates==0,"DllMain admission refused");}
    std::printf("Native OfflineBots hook pair tests: %u failures\n",failures);return failures?1:0;
}
