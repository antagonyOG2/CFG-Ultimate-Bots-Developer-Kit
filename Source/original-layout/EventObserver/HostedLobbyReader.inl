void PrintHostedLobby(HANDLE process,Reader& reader,std::uintptr_t dll,const LocalObserver& local){
    auto storage=std::make_unique<std::array<cfghostedlobby::Report,2>>();auto& a=(*storage)[0];auto& b=(*storage)[1];
    if(!local.hostedLobbyRva||!ReadableImageSpan(process,dll+local.hostedLobbyRva,sizeof(a),dll)||
       !reader.Read(dll+local.hostedLobbyRva,a)||!reader.Read(dll+local.hostedLobbyRva,b)||
       !cfghostedlobby::Accept(a,b)||std::memcmp(&a,&b,sizeof(a))){
        std::puts("H21Lobby=UNAVAILABLE");return;
    }
    const auto u=[](std::uint64_t value){return static_cast<unsigned long long>(value);};
    std::printf("H21Lobby=CAPTURED Samples=%u Failure=%u Histories=%u PeakReads=%u TextRefused=%u RowsCompiled=%u\n",
        b.samples,b.failure,b.histories,b.peakReads,b.textRefused,CFG_H21_LOBBY_ROWS);
    std::printf("H23LobbyRows Callbacks=%u Admitted=%u Stage=%u Warm=%u RootPoison=%u\n",b.rowCallbacks,b.rowAdmitted,b.rowStage,b.rowWarm,b.rowRootPoison);
    std::printf("H24LobbyAdapter Stage=%u State=%u Owned=%u Calls=%u StaticOwner=0x%llX StaticCDO=0x%llX\n",b.rowAdapterStage,b.rowState,b.rowOwned,b.rowCalls,u(b.rowStaticOwner),u(b.rowStaticDefault));
    std::printf("H24LobbyFirst Failure=%u Adapter=%u Reads=%u PreentryRetries=%u\n",b.rowFirstFailure,b.rowFirstAdapter,b.rowFirstReads,b.rowRetries);
    std::printf("H26LobbyReasons Type=%u Current=%u Invoke=%u RootStage=%u; Adapter103=Type104=Current105=Invoke; Current1=unsealed2=generation3=hostscope4=hostproof5=donortype6=parent; Root1=owners2=frame3=outputs4=reserve\n",b.rowTypeReason,b.rowCurrentReason,b.rowInvokeReason,b.rowRootStage);
    std::printf("H28CounselorCap Reached=%u Players=%d\n",b.f3CapReached,b.f3CapPlayers);
    std::printf("H28LobbyInputs Attempts=%u Failures=%u\n",b.rowInputAttempts,b.rowInputFailures);
    std::printf("H27LobbyInputs Scope=%u Reason=%u Role=%u Value=0x%llX Serial=%d Internal=0x%X ObjectFlags=0x%X; Scope1=missing2=precurrent3=provider4=postcurrent; Role1=owners2=targets3=frame\n",b.rowRootScopeFailure,b.rowRootReason,b.rowRootRole,u(b.rowRootValue),b.rowRootSerial,b.rowRootInternal,b.rowRootFlags);
    for(const auto& d:b.rowOwners)if(d.value)std::printf("H211LobbyOwner Role=%u Index=%u Value=0x%llX Serial=%d Internal=0x%X Flags=0x%X Name=%.*s Class=%.*s\n",d.role,d.index,u(d.value),d.serial,d.internal,d.flags,40,d.name.data(),56,d.className.data());
    std::printf("H26HostedF1 Presses=%u CounselorPacket Reason=%u Source=%u Index=%u Address=0x%llX\n",b.f1Presses,b.packetReason,b.packetKind,b.packetIndex,u(b.packetAddress));
    std::printf("H25LobbyCDO OwnerClass=%.*s ActualClass=%.*s Proof=IsA_plus_retained_slots_flags\n",64,b.rowStaticOwnerName.data(),64,b.rowStaticClassName.data());
    const auto& controller=b.controller;
    std::printf("H22LobbyController Stage=%u Count=%d Capacity=%d Player=0x%llX Controller=0x%llX Class=%.*s Type=0x%llX APlayerController=0x%llX BacklinkOffset=0x%llX Backlink=0x%llX\n",
        controller.stage,controller.count,controller.capacity,u(controller.player),u(controller.controller),64,controller.className.data(),u(controller.type),u(controller.base),u(cfghosted::disk35::ControllerPlayer),u(controller.backlink));
    std::printf("H22HostedHotkeys F9=%u F3=%u Queued=%u Attempts=%u Completed=%u Refused=%u Reason=%u\n",b.f9Presses,b.f3Presses,b.f3Queued,b.f3Attempts,b.f3Completed,b.f3Refused,b.f3Reason);
    std::printf("H22HostedBirth Stage=%u Calls=%u PlayersBefore=%d BotsBefore=%d Controller=0x%llX State=0x%llX Pawn=0x%llX\n",b.f3NativeStage,b.f3NativeCalls,b.f3BeforePlayers,b.f3BeforeBots,u(b.f3Controller),u(b.f3State),u(b.f3Pawn));
    std::printf("H27HostedBirth FirstReason=%u FirstStage=%u FirstCalls=%u TravelReturned=%u TravelReady=%u; Reason11=prior_quarantine_first_failure_preserved\n",b.f3FirstReason,b.f3FirstStage,b.f3FirstCalls,b.f3TravelReturned,b.f3TravelReady);
    for(unsigned h=0;h<b.histories;++h){const auto& s=b.history[h];
        if(s.count>cfghostedlobby::NodeLimit||s.cursor>s.count){std::puts("H21LobbySnapshot=MALFORMED");continue;}
        const char* hint=s.lobbyType==1?"private":s.lobbyType==2?"public":"unknown";
        std::printf("H21LobbySnapshot=%u World=0x%llX Menu=0x%llX Class=%.*s TypeHint=%s TypeSource=class-name-only HostFlag=%u ListenHost=%u Complete=%u Truncated=%u Nodes=%u Cursor=%u RowsPlaced=%u PlacementRefusal=%u\n",
            h,u(s.world),u(s.menu),64,s.menuClass.data(),hint,s.hostFlag,s.listenHost,s.complete,s.truncated,s.count,s.cursor,s.rowsPlaced,s.placementRefusal);
        for(unsigned i=0;i<s.cursor;++i){const auto& n=s.nodes[i];
            std::printf("H21LobbyNode=%u/%u Object=0x%llX Class=%.*s Name=%.*s Parent=%u ChildIndex=%u Slot=0x%llX TextState=%u AuthoredText=%.*s\n",
                h,i,u(n.object),56,n.className.data(),40,n.name.data(),n.parent,n.index,u(n.slot),n.textState,72,n.authoredText.data());
        }
        const auto row=[&](const char* label,const cfghostedlobby::Row& r){
            if(r.bindings>cfghostedlobby::BindingLimit){std::puts("H21LobbyRow=MALFORMED");return;}
            std::printf("H21LobbyRow=%s Snapshot=%u Node=%u Parent=%u ChildIndex=%u Bindings=%u Truncated=%u\n",label,h,r.node,r.parent,r.childIndex,r.bindings,r.truncated);
            for(unsigned i=0;i<r.bindings;++i){const auto& m=r.members[i];
                std::printf("H21LobbyBinding=%s/%u Name=%.*s Offset=0x%X Target=0x%llX Property=0x%llX\n",label,i,40,m.name.data(),m.offset,u(m.object),u(m.property));}
        };row("TeamKilling",s.team);row("Weather",s.weather);
    }
    std::puts("H21LobbyPolicy=read-only; AuthoredText is not a bound-display guarantee; unknown lobby type stays unknown; placement requires its own host-lobby native lease and compatible native row proof. Refusal12=no-native-lease,11=no-host. F8 retained; no offline NetDriver guard changes.");
}
