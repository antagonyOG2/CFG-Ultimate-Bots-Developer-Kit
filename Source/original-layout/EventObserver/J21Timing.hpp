#pragma once
cfgj21::EpochBudget bundleJ21Budget;
std::uint64_t bundleJ21CallbackSequence=0;
std::uint64_t bundleJ21LastControllerAt=0;
thread_local bool bundleJ21BudgetActive=false;
thread_local LARGE_INTEGER bundleJ21BudgetStart{};
bool J21HasBudget(std::uint64_t limit){
    std::uint64_t elapsed=0;
    if(bundleJ21BudgetActive){LARGE_INTEGER end{},frequency{};QueryPerformanceCounter(&end);QueryPerformanceFrequency(&frequency);
        if(frequency.QuadPart>0&&end.QuadPart>=bundleJ21BudgetStart.QuadPart)
            elapsed=static_cast<std::uint64_t>((end.QuadPart-bundleJ21BudgetStart.QuadPart)*1000000/frequency.QuadPart);}
    if(bundleJ21Budget.used+elapsed>=limit){++bundleJ21Budget.deferred;return false;}return true;
}
cfgj21::NameMemo<cfgprobe::lifetime::SlotObservation,cfgnames::Text> bundleJ21Memo;
cfgj21::NameMemo<cfgprobe::lifetime::SlotObservation,cfgnames::Text,128> bundleJ21WorldNames;
unsigned bundleJ21WorldNameCursor=0;
template<class R>bool J21ClassName(R& reader,std::uintptr_t type,cfgnames::Text& name){
    if(bundleJ21Memo.Find(plan.world,type,name))return true;
    cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation slot{};
    if(!cfgprobe::lifetime::ReadArrayHeader(reader,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !cfgspawn::ObjectSlot(reader,registry,type,slot))return false;
    if(bundleJ21WorldNames.world!=plan.world){bundleJ21WorldNames.Begin(plan.world);bundleJ21WorldNameCursor=0;}
    if(slot.HasAssignedSerial())for(const auto& entry:bundleJ21WorldNames.entries)
        if(entry.used&&entry.slot==slot){name=entry.name;bundleJ21Memo.Put(slot,name);return true;}
    cfgnames::Observation<R> names(reader,plan.module);
    if(!names.Object(type,name))return false;
    cfgprobe::lifetime::SlotObservation after{};
    if(!cfgspawn::ObjectSlot(reader,registry,type,after)||!(after==slot))return false;
    if(slot.HasAssignedSerial()){
        bundleJ21WorldNames.entries[bundleJ21WorldNameCursor++%128]={slot,name,true};bundleJ21Memo.Put(slot,name);
    }return true; // zero serial: fresh same-snapshot name only, never cached
}
bool J21OwnedAlive(const cfgprobe::lifetime::SlotObservation& pawn){
    if(!pawn.HasAssignedSerial())return false;
    Reader r;r.cap=256;cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation fresh{},controllerFresh{},level{};float health=0;
    std::uintptr_t levelType=0;
    const auto controller=pawn==bundleJason.pawn?bundleJason.killerController:bundleAdditional.actor.killerController;
    if(!controller.HasAssignedSerial())return false;
    return cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
        cfgspawn::ObjectSlot(r,registry,pawn.object,fresh)&&fresh==pawn&&r.Read(pawn.object+cfghunter::disk::Health,health)&&
        std::isfinite(health)&&health>0&&cfgspawn::ObjectSlot(r,registry,controller.object,controllerFresh)&&controllerFresh==controller&&
        cfgkill::ActorWorldCurrent(r,registry,plan.module,pawn.object,plan.world,level,levelType)&&
        cfgspawn::ObjectIs(r,controller.object,spawnAssets.resources.classes[cfgspawn::KillerAI])&&
        cfgobserver::Same(r,pawn.object+cfgescape::disk::Controller,controller.object)&&
        cfgobserver::Same(r,controller.object+cfgescape::disk::Pawn,pawn.object);
}
bool J21AdmittedAlive(const cfgprobe::lifetime::SlotObservation& pawn){
    // Caller just proved this exact native Tick receiver. Only a scheduling
    // health hint; native authority is re-proved after the original/base call.
    Reader r;float health=0;return pawn.HasAssignedSerial()&&r.Read(pawn.object+cfghunter::disk::Health,health)&&std::isfinite(health)&&health>0;
}
struct J21CallbackMemoScope {
    explicit J21CallbackMemoScope(std::uintptr_t world){++bundleJ21CallbackSequence;bundleJ21Memo.Begin(world);}
    ~J21CallbackMemoScope(){bundleJ21Memo.End();}
};
struct J21Timing {
    cfgj21::Stage stage;LARGE_INTEGER begin{};bool charge=false,stopped=false;
    explicit J21Timing(cfgj21::Stage value,bool cost=false):stage(value),charge(cost&&!bundleJ21BudgetActive){
        QueryPerformanceCounter(&begin);if(charge){bundleJ21BudgetStart=begin;bundleJ21BudgetActive=true;}}
    void Stop(){if(stopped)return;stopped=true;LARGE_INTEGER end{},frequency{};QueryPerformanceCounter(&end);QueryPerformanceFrequency(&frequency);
        if(frequency.QuadPart>0&&end.QuadPart>=begin.QuadPart){
            const auto us=static_cast<std::uint64_t>((end.QuadPart-begin.QuadPart)*1000000/frequency.QuadPart);
            bundleReport.j21Costs[stage].Observe(us);if(charge)bundleJ21Budget.Charge(us);
            bundleReport.j21Epoch=bundleJ21Budget;}if(charge)bundleJ21BudgetActive=false;}
    ~J21Timing(){Stop();}
};
// H2.91 timing only: microseconds since an earlier J21Micros() into a count/total/max row.
inline std::uint64_t J21Micros()noexcept{LARGE_INTEGER t{},f{};QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);
    return f.QuadPart>0?static_cast<std::uint64_t>(t.QuadPart*1000000/f.QuadPart):0;}
inline void J21CostAdd(cfgbundle::Report::H291Cost& c,std::uint64_t start)noexcept{
    const auto now=J21Micros();const auto us=now>=start?now-start:0;++c.count;c.us+=us;if(us>c.maxUs)c.maxUs=us;if(us>16667)++c.overFrame;}
