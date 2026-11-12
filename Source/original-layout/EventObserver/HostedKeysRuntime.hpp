#if CFG_H22_HOTKEYS
void HostedH22Disarm()noexcept{
    auto& s=hostedH1;s.disarmed=true;s.manualArm=false;s.armed=false;s.takeoverDue=false;s.f3Pending=0;s.f3Automatic=false;
    if(s.adopted){
        // Disarm the driver now, preserve the current-world restoration ticket
        // until the native terminal phase. No live human is restarted by F9.
        HostedUnpublishAdoption();s.adoptionPending=false;s.pendingReceipt={};
    }
}
void HostedH22Arm()noexcept{
    auto& s=hostedH1;s.disarmed=false;s.manualArm=true;s.armed=true;s.armCheckAt=0;
    // Explicit retry only after a proved pre-birth refusal with a still-valid
    // lease. Never replay a dispatched/ambiguous controller birth or possession.
    if(!s.adopted&&s.takeoverTried&&s.takeoverRetrySafe){s.takeoverTried=false;s.takeoverDue=false;s.takeoverRetrySafe=false;s.stable.Reset();}
}
// Input is polled only after an admitted, returned owner callback. Native
// spawning runs later, exclusively inside the existing retained callback lease.
void ServiceHostedKeys()noexcept{
    auto& s=hostedH1;const auto now=GetTickCount64();if(now<s.keyNext||bundleRootLease||NativeJasonControllerTickInFlight())return;s.keyNext=now+100;
    HostedLobbyPublishScope publication;
    DWORD foreground=0;const auto window=GetForegroundWindow();if(window)GetWindowThreadProcessId(window,&foreground);
    const bool down8=(GetAsyncKeyState(VK_F8)&0x8000)!=0,down9=(GetAsyncKeyState(VK_F9)&0x8000)!=0;
    const bool f8=down8&&!s.f8Down,f9=down9&&!s.f9Down;s.f8Down=down8;s.f9Down=down9;
    if((!f8&&!f9)||foreground!=GetCurrentProcessId())return;
    Reader input;input.cap=8192;std::uintptr_t engine=0,viewport=0,world=0;
    if(!input.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)||!input.Read(engine+cfgprobe::EngineGameViewport,viewport)||
       !input.Read(viewport+cfgprobe::ViewportWorld,world))return;
    const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
    cfghosted::listen35::Context match;
    const bool hostMatch=cfghosted::listen35::Capture(input,admission,GetCurrentThreadId(),match);
    cfghostedlobby::Context lobby;cfghostedlobby::Failure why;
    bool hostLobby=false;if(!hostMatch){input.reads=0;hostLobby=cfghostedlobby::Observe(input,plan.module,plan.threadId,lobby,why)&&lobby.listenHost;}
    input.reads=0;const bool preferenceHost=hostMatch||hostLobby||cfghostedlobby::InputHost(input,plan.module,plan.threadId);
    if(!preferenceHost)return; // only preferences/feedback; effects still need the FULL match lease
    const auto action=cfghostedkeys::Select(preferenceHost,f8,f9);
    if(action==cfghostedkeys::Action::Disarm){
        ++hostedLobbyReport.f9Presses;HostedH22Disarm();
    }else if(action==cfghostedkeys::Action::Arm){HostedH22Arm();++s.f8Presses;
    }
    hostedLobbyReport.f3Queued=s.f3Pending;
}
#endif
