#define CFG_H22_SETTINGS_MAIN H22ExistingLobbyChecks
#include "H22SettingsTests.cpp"

struct LobbyCappedReader:StageReader{
    unsigned count=0,cap=32768;
    template<class T>bool Read(std::uintptr_t at,T& out){return count++<cap&&StageReader::Read(at,out);}
};

static int BelowCapRetry(){
    using namespace cfghostedlobby;
    namespace d=cfgpractice::disk;
    LobbyFixture f;f.Text();
    constexpr std::uintptr_t nestedTree=0x2B00000,nestedTitle=0x2B10000;
    f.Slot(nestedTree,32,132);f.Slot(nestedTitle,33,133);
    f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,int{34});
    f.r.Set(f.Child+cfgprobe::ObjectClass,f.UserType);
    f.r.Set(nestedTree+cfgprobe::ObjectClass,f.TreeType);
    f.r.Set(nestedTree+d::RootWidget,nestedTitle);
    f.r.Set(nestedTitle+cfgprobe::ObjectClass,f.TextType);
    f.r.Set(nestedTitle+disk::AuthoredText,std::uintptr_t{0x2A00000});
    Context context;Failure why;Report report;Snapshot snapshot;Discovery discovery;
    if(!Observe(f.r,f.Module,42,context,why)||!discovery.Begin(f.r,context,snapshot))return 10;
    const auto belowCap=[&](){LobbyCappedReader r;r.bytes=f.r.bytes;
        return !discovery.Step(r,snapshot,report)&&r.count<r.cap;};
    f.r.bytes.erase(f.SlotObject+d::Content);
    if(!belowCap()||discovery.graph.count!=1||discovery.graph.cursor!=0)return 11;
    f.r.Set(f.SlotObject+d::Content,f.Child);
    if(!discovery.Step(f.r,snapshot,report)||snapshot.cursor!=1)return 12;
    if(!belowCap()||discovery.graph.count!=2||discovery.graph.cursor!=1)return 13;
    f.r.Set(f.Child+d::WidgetTree,nestedTree);
    if(!discovery.Step(f.r,snapshot,report)||snapshot.cursor!=2)return 14;
    f.r.bytes.erase(nestedTitle+disk::AuthoredText);
    if(!belowCap()||snapshot.cursor!=2||discovery.bindingsDone[1])return 17;
    f.r.Set(nestedTitle+disk::AuthoredText,std::uintptr_t{0x2A00000});
    f.r.bytes.erase(f.UserType+cfgprobe::ClassSuper);
    if(!belowCap()||snapshot.cursor!=2||discovery.bindingsDone[1])return 18;
    f.r.Set(f.UserType+cfgprobe::ClassSuper,f.WidgetType);
    if(!belowCap()||snapshot.cursor!=2||discovery.bindingsDone[1]||snapshot.weather.node!=UINT32_MAX)return 15;
    f.r.Set(f.UserType+cfgreflection::disk::Children,std::uintptr_t{});
    if(!discovery.Step(f.r,snapshot,report)||!snapshot.complete||!discovery.bindingsDone[1]||
       snapshot.weather.node!=1||snapshot.weather.parent!=0)return 16;
    std::puts("H220 below-cap panel content, nested tree, label, ancestry and reflected binding reads retry without losing the row.");
    return 0;
}

// H2.47: a graph that fails Step forever (second lobby: Nodes=6 Cursor=4 Failure=Changed)
// must re-Begin after StallGuard::Limit failed samples, then discover the row normally.
static int StallRearm(){
    using namespace cfghostedlobby;namespace d=cfgpractice::disk;
    LobbyFixture f;f.Text();Context c;Failure why;Report report;Snapshot snapshot;Discovery discovery;StallGuard guard;
    if(!Observe(f.r,f.Module,42,c,why)||!discovery.Begin(f.r,c,snapshot))return 30;
    f.r.bytes.erase(f.SlotObject+d::Content);
    for(unsigned i=1;i<StallGuard::Limit;++i){
        if(discovery.Step(f.r,snapshot,report))return 31;
        guard.StepFailed(snapshot.complete!=0,snapshot.team.node);
        if(guard.rearm||guard.TakeRearm())return 32;
    }
    if(discovery.Step(f.r,snapshot,report))return 33;
    guard.StepFailed(snapshot.complete!=0,snapshot.team.node);
    if(guard.TakeRearm()!=static_cast<bool>(CFG_H247_LOBBY_REARM)||guard.TakeRearm())return 34;
    // A found team row, or a finished snapshot, never rearms.
    for(unsigned i=0;i<StallGuard::Limit*2;++i){guard.StepFailed(false,3);guard.StepFailed(true,UINT32_MAX);}
    if(guard.rearm)return 35;
    // H2.48: while a hosted match is live the guard never counts and never rearms.
    guard={};
    for(unsigned i=0;i<StallGuard::Limit*3;++i)guard.StepFailed(false,UINT32_MAX,true);
    if(guard.rearm||guard.failures)return 40;
    if(!CFG_H247_LOBBY_REARM)return 0;
    for(unsigned i=0;i<StallGuard::Limit;++i)guard.StepFailed(false,UINT32_MAX);
    if(!guard.rearm||guard.TakeRearm(true)||guard.rearm)return 41;
    guard={};
    // Progress resets the count.
    for(unsigned i=1;i<StallGuard::Limit;++i)guard.StepFailed(false,UINT32_MAX);
    guard.Progressed();guard.StepFailed(false,UINT32_MAX);if(guard.rearm)return 36;
    // The re-Begin from fresh evidence completes discovery and finds the row.
    f.r.Set(f.SlotObject+d::Content,f.Child);
    Snapshot fresh;Discovery again;
    if(!Observe(f.r,f.Module,42,c,why)||!again.Begin(f.r,c,fresh))return 37;
    for(unsigned i=0;i<4&&!fresh.complete;++i)if(!again.Step(f.r,fresh,report))return 38;
    if(!fresh.complete){std::printf("cursor=%u count=%u\n",fresh.cursor,fresh.count);return 39;}
    std::puts("H247 lobby rearm: stalled discovery re-Begins after 10 failed samples and finds the row; found/complete/progress never rearm.");
    return 0;
}

int main(){
    if(H22ExistingLobbyChecks()!=0)return 1;
    using namespace cfghostedlobby;
    LobbyFixture f;f.Text();Context c;Failure why;Report report;Snapshot snapshot;
    if(!Observe(f.r,f.Module,42,c,why))return 2;
    Discovery discovery;if(!discovery.Begin(f.r,c,snapshot))return 3;
    const auto root=discovery.graph.nodes[0].tree;
    f.r.Set(root+cfgpractice::disk::RootWidget,std::uintptr_t{});
    if(discovery.Step(f.r,snapshot,report)||discovery.graph.cursor!=0||snapshot.cursor!=0)return 4;
    f.r.Set(root+cfgpractice::disk::RootWidget,discovery.graph.nodes[0].widget.object);
    if(!discovery.Step(f.r,snapshot,report)||discovery.graph.cursor!=1||snapshot.cursor!=1)return 5;
    if(!discovery.Step(f.r,snapshot,report)||!snapshot.complete||snapshot.cursor!=2)return 6;
    // Copy the challenge fixture's byte image, establish the minimum context
    // and path prefix, then exhaust a later field read inside the same node.
    LobbyCappedReader capped;capped.bytes=f.r.bytes;auto budgetDiscovery=discovery;
    Snapshot budgetSnapshot=snapshot;Report budgetReport=report;
    LobbyCappedReader prefix;prefix.bytes=f.r.bytes;
    if(!budgetDiscovery.Current(prefix)||!budgetDiscovery.Path(prefix,0))return 7;
    const auto prefixReads=prefix.count;
    budgetDiscovery.graph.cursor=0;budgetSnapshot.cursor=0;budgetSnapshot.complete=0;
    budgetSnapshot.count=budgetDiscovery.graph.count;
    const auto savedNodes=budgetDiscovery.graph.nodes;const auto savedOut=budgetSnapshot;
    capped.cap=prefixReads+3;
    if(budgetDiscovery.Step(capped,budgetSnapshot,budgetReport)||budgetDiscovery.graph.cursor!=0||
       budgetSnapshot.cursor!=savedOut.cursor||budgetSnapshot.complete!=savedOut.complete||
       budgetSnapshot.count!=savedOut.count||budgetDiscovery.graph.nodes[0].widget.object!=savedNodes[0].widget.object)return 8;
    LobbyCappedReader retry;retry.bytes=f.r.bytes;
    if(!budgetDiscovery.Step(retry,budgetSnapshot,budgetReport)||budgetSnapshot.cursor!=1)return 9;
    std::printf("H220 transactional lobby discovery retry: PASS (path refusal retained node; retry completed)\n");
    std::printf("H220 cumulative mid-node budget refusal retained node state and fresh retry passed: %u prefix reads\n",prefixReads);
    {const int rearm=StallRearm();if(rearm)return rearm;}
    return BelowCapRetry();
}
