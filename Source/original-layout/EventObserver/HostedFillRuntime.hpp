// Included after H1 state, on the existing serialized observer owner thread.
#if CFG_H20_FILL
using HostedFillOriginal=void(__fastcall*)(void*);
HostedFillOriginal hostedFillOriginal=nullptr;
std::uintptr_t hostedFillTarget=0;bool hostedFillReady=false,hostedFillAttempted=false;
thread_local unsigned hostedFillDepth=0;
cfghostedfill::Report hostedFillReport;
unsigned hostedFillReason=0,hostedFillContextStage=0;
std::int32_t hostedFillHumans=-1,hostedFillExpected=-1,hostedFillBotsBefore=-1,hostedFillBotsAfter=-1;
struct HostedFillHost {
    std::uintptr_t mode=0;
    struct Ticket {cfghosted::listen35::Context context{};std::uint8_t before=0;bool requested=false,reserve=false;};
    bool Capture(Ticket& ticket){
        Reader r;r.cap=8192;std::uintptr_t engine=0,viewport=0,world=0,table=0;
        hostedFillReason=1;hostedFillContextStage=0;
        if(hostedFillDepth!=1||GetCurrentThreadId()!=observerOwnerThread||!plan.module||
           !r.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)||!r.Read(engine+cfgprobe::EngineGameViewport,viewport)||
           !r.Read(viewport+cfgprobe::ViewportWorld,world))return false;
        const cfghosted::native::Admission a{plan.module,engine,viewport,world,observerOwnerThread,true,true};
        cfghosted::native::Trace trace;hostedFillReason=2;
        if(!cfghosted::listen35::Capture(r,a,observerOwnerThread,ticket.context,nullptr,&trace)){hostedFillContextStage=trace.stage;return false;}
        hostedFillReason=3;
        if(ticket.context.session.Mode()!=mode||
           !r.Read(mode,table)||table!=plan.module+cfghosted::birth35::HuntVtable||
           !cfgobserver::Same(r,table+cfghosted::fill35::StartupSlot,hostedFillTarget)||
           cfgownednative::Resolve(plan.module,hostedFillTarget,cfghosted::fill35::EntryCode)!=reinterpret_cast<std::uintptr_t>(hostedFillOriginal)||
           !cfgbundle::Code(r,plan.module,cfghosted::fill35::Startup+17,cfghosted::fill35::BodyCode)||
           !r.Read(mode+cfghosted::fill35::BotFill,ticket.before)||ticket.before>1)return false;
        const auto& store=cfghostedoptions::Current();
        hostedFillReason=4;if(!hostedH1.options.Capture(world,true,store))return false;
        ticket.requested=hostedH1.options.values.fill!=0;
#ifdef CFG_H1_JASON
        ticket.reserve=hostedH1.armed||hostedH1.manualArm||hostedH1.options.values.jason||HostedH1ArmFilePresent();
#if CFG_H22_HOTKEYS
        ticket.reserve=ticket.reserve&&!hostedH1.disarmed;
#endif
#endif
        hostedFillReason=ticket.requested?0u:5u;
#if CFG_H22_HOTKEYS
        ticket.requested=ticket.requested&&!hostedH1.disarmed;if(hostedH1.disarmed)hostedFillReason=6;
#endif
        return true;
    }
    bool Exchange(const Ticket& ticket,std::uint8_t expected,std::uint8_t desired){
        Ticket fresh;if(!Capture(fresh)||(!fresh.requested&&!fresh.reserve&&!ticket.reserve)||fresh.before!=expected||
           !cfghosted::native::SameSession(fresh.context.session,ticket.context.session)||fresh.context.local!=ticket.context.local)return false;
        const auto at=mode+cfghosted::fill35::BotFill;MEMORY_BASIC_INFORMATION m{};
        hostedFillReason=7;
        if(VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||m.Protect!=PAGE_READWRITE)return false;
        const bool exchanged=static_cast<std::uint8_t>(_InterlockedCompareExchange8(reinterpret_cast<volatile char*>(at),static_cast<char>(desired),static_cast<char>(expected)))==expected;
        hostedFillReason=exchanged?0u:8u;return exchanged;
    }
    bool Counts(std::int32_t& humans,std::int32_t& bots){
        Ticket t;if(!Capture(t))return false;
        Reader r;r.cap=1800;const auto& c=t.context.session;cfghosted::listen35::LocalPlayers list{};
        std::uintptr_t stateClass=0,controllerClass=0;const auto& pc=cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass];
        if(!cfghosted::native::Class(r,plan.module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateClass)||
           !cfghosted::native::Class(r,plan.module,pc.reflectionCacheRva,pc.constructorCacheRva,controllerClass)||
           !r.Read(c.State()+cfghosted::birth35::PlayerArray,list)||list.count<0||list.count>32||list.capacity<list.count||list.capacity>64||
           (list.count&&!cfgprobe::ObjectPointer(list.data)))return false;
        std::int32_t humanCount=0,botCount=0;
        for(int i=0;i<list.count;++i){std::uintptr_t state=0,controller=0,pawn=0;std::uint32_t flags=0;cfghosted::native::Slot slot{};
            if(!r.Read(list.data+8ULL*i,state)||!cfgspawn::ObjectSlot(r,c.table,state,slot)||!cfgspawn::ObjectIs(r,state,stateClass)||
               !r.Read(state+0x394,flags))return false;
            if(flags&4){++botCount;continue;}
            // Native startup counts PlayerController-derived actors with a pawn.
            // Spectator status affects restart, not that count (3DC9E1 precedes858).
            if(!r.Read(state+0x88,controller))return false;
            if(!controller)continue;
            if(!cfgspawn::ObjectIs(r,controller,controllerClass)||!r.Read(controller+cfgprobe::ControllerPawn,pawn))return false;
            if(pawn)++humanCount;
        }
        cfghosted::listen35::LocalPlayers again{};
        if(!r.Read(c.State()+cfghosted::birth35::PlayerArray,again)||!(again==list))return false;
        humans=humanCount;bots=botCount;return true;
    }
};
#ifndef CFG_H247_RESERVE_SLOT
#define CFG_H247_RESERVE_SLOT 1
#endif
// H2.47 (user-approved): leave room for two AI Jasons (takeover + F1). The stock
// Hunt startup computes bots=min(8-humans,6) with the 8 as a code immediate
// (BodyCode[266..270] = B8 08 00 00 00, pinned by PrepareHostedFill). While the AI
// Jason is armed, that one byte reads 6 for the single forwarded call only, then
// is restored. Births stay the engine's own; no counts are written.
constexpr std::uintptr_t HostedFillMaxPlayersImmediate=17+267;
unsigned hostedFillReservedCalls=0,hostedFillReserveRefused=0;
void __fastcall HostedFillStartup(void* mode){
    const auto original=hostedFillOriginal;if(!original)return;
    ++hostedFillDepth;
    volatile unsigned char* reservePatched=nullptr;DWORD reserveProtect=0;
#if CFG_H247_RESERVE_SLOT
    if(hostedH1.armed&&!hostedH1.disarmed&&hostedFillTarget){
        auto* immediate=reinterpret_cast<unsigned char*>(hostedFillTarget+HostedFillMaxPlayersImmediate);
        if(immediate[-1]==0xB8&&immediate[0]==0x08&&immediate[1]==0&&immediate[2]==0&&immediate[3]==0&&
           VirtualProtect(immediate,1,PAGE_EXECUTE_READWRITE,&reserveProtect)){
            immediate[0]=0x06;FlushInstructionCache(GetCurrentProcess(),immediate,1);
            reservePatched=immediate;++hostedFillReservedCalls;
        }else ++hostedFillReserveRefused;
    }
#endif
    // __finally guarantees depth retirement even if the engine propagates SEH;
    // the original is forwarded exactly once, never replayed on a failed proof.
    __try{
        HostedFillHost host{reinterpret_cast<std::uintptr_t>(mode)};
        hostedFillHumans=hostedFillExpected=hostedFillBotsBefore=hostedFillBotsAfter=-1;
        std::int32_t ignored=-1;
        (void)host.Counts(ignored,hostedFillBotsBefore);
        // H26: replacement helper is not live-proven. Preserve the stock flag;
        // even an armed AI Jason must not remove the game's own counselors.
        cfghostedfill::AroundStock(host,[&](){original(mode);},hostedFillReport);
        const auto reason=hostedFillReason,contextStage=hostedFillContextStage;
        if(host.Counts(hostedFillHumans,hostedFillBotsAfter))hostedFillExpected=cfghostedfill::CounselorTarget(hostedFillHumans,false,hostedH1.adopted);
        hostedFillReason=reason;hostedFillContextStage=contextStage;
    }__finally{
        if(reservePatched){
            reservePatched[0]=0x08;DWORD unused=0;
            VirtualProtect(const_cast<unsigned char*>(reservePatched),1,reserveProtect,&unused);
            FlushInstructionCache(GetCurrentProcess(),const_cast<unsigned char*>(reservePatched),1);
        }
        --hostedFillDepth;
    }
}
bool PrepareHostedFill(std::uintptr_t module){
    if(hostedFillAttempted)return hostedFillReady;hostedFillAttempted=true;
    if(!module||GetCurrentThreadId()!=observerOwnerThread)return false;
    Reader r;r.cap=32;using namespace cfghosted::fill35;
    if(!cfgbundle::Code(r,module,Startup,EntryCode)||!cfgbundle::Code(r,module,Startup+17,BodyCode))return false;
    const auto target=module+Startup;void* original=nullptr;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&HostedFillStartup),&original)!=MH_OK)return false;
    cfgownednative::CurrentProcess process;
    // push rbp(2),push rsi(1),push r14(2): exactly five unrelocated bytes.
    if(!cfgownednative::resident.Prepare(process,module,target,EntryCode,reinterpret_cast<std::uintptr_t>(original),
       reinterpret_cast<std::uintptr_t>(&HostedFillStartup),5)){MH_RemoveHook(reinterpret_cast<void*>(target));return false;}
    hostedFillOriginal=reinterpret_cast<HostedFillOriginal>(original);hostedFillTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK&&cfgownednative::resident.Enable(process,target))return hostedFillReady=true;
    MH_DisableHook(reinterpret_cast<void*>(target));cfgownednative::resident.Disable(target);return false;
}
#endif
