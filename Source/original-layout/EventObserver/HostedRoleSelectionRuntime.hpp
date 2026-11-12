// Donor OfflineSetup.cpp:9473-9588 role-selection seam. This changes only the
// native lottery's two PlayerState role-preference inputs; the stock selector
// remains the sole owner of the Jason slot and all later gameplay.
#ifdef CFG_H1_JASON
using HostedRoleSelectionFn=void(__fastcall*)(void*);
HostedRoleSelectionFn hostedRoleSelectionOriginal=nullptr;
bool hostedRoleSelectionHookInstalled=false;
// H2.45: entry, byte pins and field offsets are parameters (cfghostedrole::ActiveLayout): the donor
// layout, or the CFG 3.5 layout when CFG_H245_HOST_JASON. One implementation serves both.
constexpr const cfghostedrole::Layout& HostedRoleLayout=cfghostedrole::ActiveLayout;
constexpr std::uintptr_t HostedRoleSelectionAt=HostedRoleLayout.entry;
constexpr std::uintptr_t HostedRolePreference=HostedRoleLayout.preference,HostedPriorJasonName=HostedRoleLayout.priorName;
bool hostedRolePinsFailed=false; // image bytes cannot change at runtime: never re-read after a mismatch
constexpr std::uint32_t HostedRoleBotMask=4;
struct HostedRoleRoster {
    cfgspawn::PlayerArray header{};
    cfgprobe::lifetime::ArrayHeader table{};
    std::uintptr_t stateType=0;
    std::array<std::uintptr_t,8> state{};
    std::array<cfgprobe::lifetime::SlotObservation,8> slot{};
    std::array<std::uint8_t,8> before{},after{};
    std::array<std::uint8_t,8> human{};
    std::array<std::uint32_t,8> flags{};
    unsigned count=0,hostIndex=~0u,remoteCount=0;
    cfgspawn::PlayerArray prior{};
};
bool HostedRolePreflight(void* mode,HostedRoleRoster& out,unsigned& refusal)noexcept{
    // H2.50b: split refusal 1 (live H2.50: Reason=1 at PreMatchIntro). 15 = not the observer
    // thread, 16 = no mode, 17 = module mismatch. At PreMatchIntro the hosted plan is not built
    // yet (plan.module==0), so only compare the module when a plan exists.
    out={};if(GetCurrentThreadId()!=observerOwnerThread){refusal=15;return false;}
    if(!mode){refusal=16;return false;}
    const auto module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    if(!module||(plan.module&&module!=plan.module)){refusal=17;return false;}
    refusal=1;
    Reader r;r.cap=8192;std::uintptr_t engine=0,viewport=0,world=0,actualMode=0,gameInstance=0;
    // H2.50d: split the former refusal 1 (live H2.50c: still Reason=1 at PreMatchIntro).
    // 18 engine/viewport, 19 viewport world, 20 world game mode != the lottery's mode, 21 game instance,
    // 22 listen-host context capture, 23 Hunt class / host PlayerState.
    if(!r.Read(module+cfgprobe::EngineGlobalCandidate,engine)||!cfgprobe::ObjectPointer(engine)||
       !r.Read(engine+cfgprobe::EngineGameViewport,viewport)||!cfgprobe::ObjectPointer(viewport)){refusal=18;return false;}
    if(!r.Read(viewport+cfgprobe::ViewportWorld,world)||!cfgprobe::ObjectPointer(world)){refusal=19;return false;}
    if(!r.Read(world+cfghosted::disk35::WorldGameMode,actualMode)||actualMode!=reinterpret_cast<std::uintptr_t>(mode)){refusal=20;return false;}
    if(!r.Read(world+cfghosted::disk35::WorldGameInstance,gameInstance)||!cfgprobe::ObjectPointer(gameInstance)){refusal=21;return false;}
    {std::uintptr_t netDriver=0; // hosted worlds only (NetDriver non-null)
     if(!r.Read(world+cfgprobe::WorldNetDriver,netDriver)||!netDriver){refusal=cfghostedrole::RefuseOffline;return false;}}
    const cfghosted::native::Admission admission{module,engine,viewport,world,GetCurrentThreadId(),true,true};
    cfghosted::listen35::Context host;
    {cfghosted::native::Trace trace; // 22 = capture refused (H2.50e: 100+trace.stage names the step); 24 = no session state
     if(!cfghosted::listen35::Capture(r,admission,GetCurrentThreadId(),host,nullptr,&trace,true)){refusal=100u+trace.stage+1000u*cfghosted::native::typedAllowUnassignedWhy;return false;}
     if(host.session.State()==0){refusal=24;return false;}}
    std::uintptr_t huntClass=0,stateClass=0,hostState=0;
    if(!cfghosted::native::Class(r,module,cfghosted::disk35::HuntReflection,cfghosted::disk35::HuntConstructor,huntClass)||
       !cfgspawn::ObjectIs(r,actualMode,huntClass)||
       !cfghosted::native::Class(r,module,cfghosted::birth35::HuntStateReflection,cfghosted::birth35::HuntStateConstructor,stateClass)||
       !r.Read(host.local[2].object+cfghosted::adoption::ControllerState,hostState)||!hostState){refusal=23;return false;}
    out.table=host.session.table;out.stateType=stateClass;
    refusal=2;
    if(!r.Read(host.session.State()+cfghosted::birth35::PlayerArray,out.header)||out.header.count<2||out.header.count>8||
       out.header.capacity<out.header.count||out.header.capacity>128||!cfgprobe::ObjectPointer(out.header.data))return false;
    out.count=static_cast<unsigned>(out.header.count);
    for(unsigned i=0;i<out.count;++i){
        auto& playerState=out.state[i];auto& slot=out.slot[i];auto& flags=out.flags[i];
        if(!r.Read(out.header.data+sizeof(std::uintptr_t)*i,playerState)||!cfgprobe::ObjectPointer(playerState)||
           !cfghosted::native::TypedAllowUnassigned(r,host.session.table,playerState,stateClass,slot)||
           !r.Read(playerState+cfghosted::birth35::BotFlags,flags))return false;
        if(flags&HostedRoleBotMask)continue;
        out.human[i]=1;
        if(!r.Read(playerState+HostedRolePreference,out.before[i])||out.before[i]>cfghostedrole::KillerPreference)return false;
        if(playerState==hostState){if(out.hostIndex!=~0u)return false;out.hostIndex=i;}
        else ++out.remoteCount;
    }
    refusal=3;
    if(out.hostIndex==~0u||out.remoteCount==0)return false;
    if(!cfghostedrole::InvertHostPreference(out.before,out.human,out.count,out.hostIndex,out.after))return false;
    refusal=4; // Preserve donor's guarded prior-Jason exclusion adjustment.
    if(!r.Read(gameInstance+HostedPriorJasonName,out.prior)||out.prior.count<0||out.prior.count>256||
       out.prior.capacity<out.prior.count||out.prior.capacity>512||
       (out.prior.count>0&&(!cfgprobe::ObjectPointer(out.prior.data))))return false;
    if(out.prior.count>0){std::array<wchar_t,256> priorName{};
        if(!r.ReadBytes(out.prior.data,priorName.data(),sizeof(wchar_t)*static_cast<std::size_t>(out.prior.count)))return false;}
    refusal=0;return true;
}
// No unwindable locals: SEH-guarded raw writes through the shared, parameterised WriteRoles.
bool HostedRoleWriteSeh(const std::uintptr_t* state,const std::uint8_t* after,unsigned count,std::uintptr_t gameInstance,unsigned& writes)noexcept{
    writes=0;
    __try{
        writes=cfghostedrole::WriteRoles(HostedRoleLayout,state,after,count,gameInstance,
            [](std::uintptr_t at,std::uint8_t v){*reinterpret_cast<volatile std::uint8_t*>(at)=v;},
            [](std::uintptr_t at,std::int32_t v){*reinterpret_cast<volatile std::int32_t*>(at)=v;});
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
    return true;
}
bool HostedApplyRolePreference(void* mode,unsigned& refusal,std::uint8_t& hostBefore,unsigned& remoteCount)noexcept{
    hostBefore=0;remoteCount=0;
    HostedRoleRoster observed;if(!HostedRolePreflight(mode,observed,refusal))return false;
    hostBefore=observed.before[observed.hostIndex];remoteCount=observed.remoteCount;
    Reader r;r.cap=4096;std::uintptr_t engine=0,viewport=0,world=0,gameInstance=0,actualMode=0;
    cfgspawn::PlayerArray players{},prior{};std::uintptr_t state=0;
    if(!r.Read(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))+cfgprobe::EngineGlobalCandidate,engine)||!r.Read(engine+cfgprobe::EngineGameViewport,viewport)||
       !r.Read(viewport+cfgprobe::ViewportWorld,world)||!r.Read(world+cfghosted::disk35::WorldGameMode,actualMode)||
       actualMode!=reinterpret_cast<std::uintptr_t>(mode)||!r.Read(world+cfghosted::disk35::WorldGameInstance,gameInstance)||
       !r.Read(world+cfghosted::disk35::WorldGameState,state)||!r.Read(state+cfghosted::birth35::PlayerArray,players)||
       !r.Read(gameInstance+HostedPriorJasonName,prior)||
       players.data!=observed.header.data||players.count!=observed.header.count||players.capacity!=observed.header.capacity||
       prior.data!=observed.prior.data||prior.count!=observed.prior.count||prior.capacity!=observed.prior.capacity){refusal=5;return false;}
    for(unsigned i=0;i<observed.count;++i){std::uintptr_t playerState=0;std::uint8_t preference=0;std::uint32_t flags=0;
        cfgprobe::lifetime::SlotObservation current{};
        if(!r.Read(observed.header.data+sizeof(std::uintptr_t)*i,playerState)||playerState!=observed.state[i]||
           !cfghosted::native::TypedAllowUnassigned(r,observed.table,playerState,observed.stateType,current)||!(current==observed.slot[i])||
           !r.Read(playerState+cfghosted::birth35::BotFlags,flags)||flags!=observed.flags[i]){refusal=6;return false;}
        if(playerState==observed.state[observed.hostIndex]||observed.after[i]){
            if(!r.Read(playerState+HostedRolePreference,preference)||preference!=observed.before[i]){refusal=7;return false;}}
    }
    // Single write gate: image pins, hosted world, adoption gate. Nothing is written unless it returns 0.
    if(const unsigned gate=cfghostedrole::WriteDecision(HostedRoleLayout,hostedH1.h245PinMask,true,cfghostedrole::WritesEnabledByBuild())){
        refusal=gate;return false;}
    // All observations are complete before the first write. Only the exact
    // donor preference bytes and prior-name count are adjusted.
    unsigned writes=0;
    if(!HostedRoleWriteSeh(observed.state.data(),observed.after.data(),observed.count,gameInstance,writes)){refusal=8;return false;}
    hostedH1.h245WritesApplied+=writes;
    MemoryBarrier();refusal=0;return true;
}
#if CFG_H254_HOST_PREBORN
// H2.54 (offline's order, online): the Hunt PreMatchIntro handler picks the winner and gives it the Jason class through
// ConsumeActive (0x3FAFD0, called at 0x3DFBAD), and the intro starts from that. Pawns are born later, at InProgress, from
// ActiveCharacterClass. So after the handler returns, the same pinned native puts the host's own counselor back: the host is
// born a counselor and never has a Jason pawn, HUD or input. Game thread, inside the handler's frame (no GC can run), plain
// soft-class packet only (no owned FString), read back afterwards. Never retried. Reported as H245 RefusalCode 3000+reason:
// 0 done, 1 preference, 2 packet not plain, 3 native code pin, 4 field not writable, 5 call faulted, 6 readback mismatch.
void HostedPrebornCounselor(std::uintptr_t mode,std::uintptr_t hostState)noexcept{
    auto& s=hostedH1;namespace d=cfgmenu::birthdisk;
    const auto module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    Reader r;r.cap=2048;cfghostedpacket::Source source{};cfghostedpacket::Diagnostic diagnostic{};
    if(!cfghostedpacket::Preferred(r,module,hostState,source,diagnostic,mode)){s.h245Refusal=3001;return;}
    auto packet=source.packet;
    if(!cfgending::EmptySubpath(packet.bytes)){s.h245Refusal=3002;return;}
    if(!cfgbundle::Code(r,module,d::ConsumeActive,d::ConsumeActiveCode)){s.h245Refusal=3003;return;}
    MEMORY_BASIC_INFORMATION m{};const auto at=hostState+d::ActiveCharacterClass;
    if(VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||
       (m.Protect!=PAGE_READWRITE&&m.Protect!=PAGE_WRITECOPY)||at-reinterpret_cast<std::uintptr_t>(m.BaseAddress)>m.RegionSize||
       m.RegionSize-(at-reinterpret_cast<std::uintptr_t>(m.BaseAddress))<sizeof(packet)){s.h245Refusal=3004;return;}
    const auto expected=packet;
    cfgmenu::promotion::Packet killer{};   // H2.55: 7 = the lottery's Jason class is unreadable, has a subpath, or is already the counselor
    if(!r.Read(at,killer)||!cfgending::EmptySubpath(killer.bytes)||cfgending::SamePath(killer.bytes,expected.bytes)){s.h245Refusal=3007;return;}
    if(!cfgmenu::promotion::detail::Two(module+d::ConsumeActive,hostState,&packet)){s.h245Refusal=3005;return;}
    decltype(packet) after{};
    if(!r.Read(at,after)||!cfgending::SamePath(after.bytes,expected.bytes)){s.h245Refusal=3006;return;}
    hostedPreborn.killer=killer;hostedPreborn.saved=true;
    ++s.h245WritesApplied;s.h245Refusal=3000;
}
#endif
void __fastcall HostedRoleSelectionBridge(void* mode){
    auto& s=hostedH1;
    // H2.48c: s.options is captured only after the hosted context passes, which is after the
    // lottery; use the stored lobby options (the same source the capture copies) instead.
    const bool optionsReady=CFG_H245_HOST_JASON?(cfghostedoptions::Current().values.jason!=0&&cfghostedoptions::Current().values.jasonCount!=0):
        (s.options.world&&s.options.values.jasonCount);
    ++s.h248BridgeCalls;if(!s.h248BridgeFirstCallAt)s.h248BridgeFirstCallAt=GetTickCount64();
    s.h248BridgeLastGate=(s.armed?1u:0u)|(optionsReady?2u:0u)|(s.rolePreferenceApplied?4u:0u);
    // H2.50: live H2.49 reached the bridge (Calls=1) with gate 0x2: options ready but
    // hostedH1.armed is set only after the hosted context passes, i.e. after PreMatchIntro.
    // The stored "Jason Bot On" lobby option is the host's arming choice at this point.
    const bool armedNow=CFG_H245_HOST_JASON?(s.armed||optionsReady):s.armed;
    if(armedNow&&optionsReady&&!s.rolePreferenceApplied){
        ++s.rolePreferenceAttempts;unsigned refusal=0,remoteCount=0;std::uint8_t hostBefore=0;
        if(HostedApplyRolePreference(mode,refusal,hostBefore,remoteCount)){
            s.rolePreferenceApplied=true;s.rolePreferenceRefusal=0;s.rolePreferenceHostAfter=cfghostedrole::KillerPreference;
            s.rolePreferenceHostBefore=hostBefore;s.rolePreferenceRemoteCount=remoteCount;
        }else{++s.rolePreferenceRefused;s.rolePreferenceRefusal=refusal;s.h245Refusal=refusal;}
    }
    if(hostedRoleSelectionOriginal)hostedRoleSelectionOriginal(mode);
    if(s.rolePreferenceApplied){
        HostedRoleRoster current;unsigned refusal=0;Reader r;r.cap=256;std::uintptr_t engine=0,viewport=0,world=0,state=0,winner=0;
        if(HostedRolePreflight(mode,current,refusal)&&r.Read(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))+cfgprobe::EngineGlobalCandidate,engine)&&
           r.Read(engine+cfgprobe::EngineGameViewport,viewport)&&r.Read(viewport+cfgprobe::ViewportWorld,world)&&
           r.Read(world+cfghosted::disk35::WorldGameState,state)&&r.Read(state+cfghosted::birth35::CurrentKillerState,winner))
            {s.rolePreferenceWinnerLocal=winner==current.state[current.hostIndex]?1u:0u;
             s.h245HostWon=s.rolePreferenceWinnerLocal?1u:2u; // 0 not evaluated, 1 host won, 2 lost: stock remote takeover proceeds
#if CFG_H254_HOST_PREBORN
             if(s.rolePreferenceWinnerLocal){
#if CFG_H260_PREBORN_DEFER
                // H2.59b: live, the immediate swap replicated before the intro was chosen on clients, so player 2 saw the wrong Jason. Delay it inside the 16 s intro; pawns are born at InProgress.
                hostedPreborn.pending=true;hostedPreborn.at=GetTickCount64()+5000;hostedPreborn.world=world;hostedPreborn.mode=reinterpret_cast<std::uintptr_t>(mode);hostedPreborn.state=current.state[current.hostIndex];
#else
                HostedPrebornCounselor(reinterpret_cast<std::uintptr_t>(mode),current.state[current.hostIndex]);
#endif
             }
#endif
            }
    }
}
bool InstallHostedRoleSelectionBridge()noexcept{
    if(hostedRoleSelectionHookInstalled)return true;
    const auto module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));if(!module)return false;
    if(hostedRolePinsFailed)return false;
    Reader r;r.cap=256;
    const unsigned pins=cfghostedrole::VerifyPins(HostedRoleLayout,[&](std::uintptr_t rva,unsigned char* out,std::size_t n){
        return r.ReadBytes(module+rva,out,n);});
    hostedH1.h245PinMask=pins;
    if(!cfghostedrole::PinsAccepted(HostedRoleLayout,pins)){ // any mismatch: no hook, no writes
        hostedRolePinsFailed=true;hostedH1.h245Refusal=cfghostedrole::RefusePinMismatch;return false;}
    const auto init=MH_Initialize();if(init!=MH_OK&&init!=MH_ERROR_ALREADY_INITIALIZED)return false;
    const auto entry=module+HostedRoleSelectionAt;
    if(MH_CreateHook(reinterpret_cast<void*>(entry),reinterpret_cast<void*>(&HostedRoleSelectionBridge),
        reinterpret_cast<void**>(&hostedRoleSelectionOriginal))!=MH_OK)return false;
    if(MH_EnableHook(reinterpret_cast<void*>(entry))!=MH_OK){MH_RemoveHook(reinterpret_cast<void*>(entry));hostedRoleSelectionOriginal=nullptr;return false;}
    hostedRoleSelectionHookInstalled=true;hostedH1.h248BridgeInstallAt=GetTickCount64();return true;
}
#endif
