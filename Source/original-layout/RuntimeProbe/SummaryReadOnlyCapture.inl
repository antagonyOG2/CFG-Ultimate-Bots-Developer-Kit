// Included in FrontendMenuCapture's diagnostic-only namespace. Same pinned
// image/path, normal PROCESS_VM_READ,750k read/30s cap, no native calls/writes.
int CaptureSummary(HANDLE process,std::uintptr_t base,Reader& r){
    using namespace cfgprobe::lifetime;
    ArrayHeader registry;Context context;std::uintptr_t mode=0,state=0,type=0;
    cfgnames::Observation<Reader> names(r,base);cfgnames::Text modeName;
    if(!ReadContext(r,base,context)||!ReadArrayHeader(r,base+ObjectArrayRva,registry)||registry.count>262144||
       !r.Read(context.world+cfgprobe::WorldGameMode,mode)||!r.Read(mode+cfgprobe::ObjectClass,type)||!names.Object(type,modeName)||
       (modeName.View()!=u"SCGameMode_OfflineBots"&&modeName.View()!=u"SCGameMode_Sandbox")||
       !r.Read(context.world+cfgprobe::WorldGameState,state))return 5;
    std::puts("SummaryDiagnosticOnly=yes NativeCallsApproved=no NoWritesHooksLoadsOrNativeCalls=yes SnapshotIsNotAtomic=yes");
    std::printf("World=0x%llX Mode=0x%llX State=0x%llX ModeClass=",context.world,mode,state);Name(modeName);std::putchar('\n');
    struct PlayerArrayHeader {std::uintptr_t data=0;std::int32_t count=0,capacity=0;};
    static_assert(sizeof(PlayerArrayHeader)==16);PlayerArrayHeader players{},again{};
    if(!r.Read(state+0x380,players)||players.count<1||players.count>32||players.capacity<players.count||players.capacity>64||
       !cfgprobe::ObjectPointer(players.data))return 6;
    std::printf("NativePlayerArray=0x%llX Count=%d Capacity=%d (count-only;no capacity reads)\n",players.data,players.count,players.capacity);
    std::vector<Candidate> states;
    for(int i=0;i<players.count;++i){std::uintptr_t object=0;Candidate c;cfgnames::Text label;
        std::uint8_t dead=255,escaped=255;std::uint32_t flags=0;
        if(!r.Read(players.data+8ULL*i,object)||!Identity(r,registry,object,c)||!names.Object(c.type,label)||
           !r.Read(object+0x706,dead)||!r.Read(object+0x707,escaped)||!r.Read(object+0x394,flags))return 6;
        states.push_back(c);std::printf("Participant[%d]=0x%llX Serial=%d Type=",i,object,c.slot.serial);Name(label);
        std::printf(" DeadByte=%u EscapedByte=%u StateFlags=0x%X\n",dead,escaped,flags);
    }
    std::vector<Candidate> widgets;cfgmenu::capture::MenuClassCache classes;
    std::array<unsigned char,24*256> block{};
    const auto relevant=[](std::u16string_view label){return label.find(u"Summary")!=label.npos||
        label.find(u"Scoreboard")!=label.npos||label.find(u"MatchResult")!=label.npos||label.find(u"PlayerInfo")!=label.npos;};
    for(int begin=0;begin<registry.count;begin+=256){
        const auto count=(std::min)(registry.count-begin,256);
        if(!r.ReadBytes(registry.data+24ULL*begin,block.data(),static_cast<unsigned>(count)*24))return 6;
        for(int i=0;i<count;++i){std::uintptr_t object=0,klass=0;std::memcpy(&object,block.data()+24*i,8);if(!cfgprobe::ObjectPointer(object))continue;
            if(!r.Read(object+cfgprobe::ObjectClass,klass))return 6;bool wanted=false;
            if(!classes.Lookup(klass,wanted)){cfgnames::Text label;if(!names.Object(klass,label))return 6;wanted=relevant(label.View());classes.Store(klass,wanted);}
            if(!wanted)continue;Candidate c;
            if(!Identity(r,registry,object,c)||c.slot.index!=begin+i)return 6;
            if(c.object==c.defaults)continue;if(widgets.size()==32)return 6;widgets.push_back(c);
        }
    }
    cfgreflection::Observation<Reader> reflection(r,base);std::vector<std::uintptr_t> captured;
    for(const auto& c:widgets){cfgnames::Text label;Candidate current;
        if(!Identity(r,registry,c.object,current)||!Same(c,current)||!names.Object(c.type,label))return 7;
        std::printf("SummaryCandidate=0x%llX Type=0x%llX Outer=0x%llX Serial=%d Class=",c.object,c.type,c.outer,c.slot.serial);Name(label);std::putchar('\n');
        auto owner=c.type;std::array<std::uintptr_t,6> parents{};unsigned depth=0;
        while(owner&&depth<parents.size()){
            for(unsigned j=0;j<depth;++j)if(parents[j]==owner)return 7;parents[depth++]=owner;
            std::uintptr_t field=0;if(!r.Read(owner+cfgreflection::disk::Children,field))return 7;
            std::vector<std::uintptr_t> seen;
            while(field){
                if(seen.size()==256||std::find(seen.begin(),seen.end(),field)!=seen.end())return 7;seen.push_back(field);
                cfgreflection::Field f;if(!reflection.ReadField(field,owner,f))return 7;
                if(f.kind.View()==u"Function"&&relevant(label.View())&&captured.size()<48&&
                   std::find(captured.begin(),captured.end(),field)==captured.end()){
                    // Full schema/bytecode only, never dispatch or warm a cache.
                    auto proof=std::make_unique<cfgmenu::capture::DiagnosticFunctionEvidence>();
                    if(cfgmenu::capture::KnownFunction(r,base,f,f.label.View(),*proof)){Function(*proof);captured.push_back(field);}
                }else if(f.kind.View()!=u"Function"){
                    cfgreflection::Property p;if(reflection.ReadProperty(f,p)){
                        Property("SummaryProperty",p);
                        if(p.dimension==1&&p.offset>=0&&p.offset<0x4000&&(p.elementBytes==8||p.elementBytes==16)){
                            std::array<std::uint64_t,2> value{};
                            if(!r.ReadBytes(c.object+static_cast<unsigned>(p.offset),value.data(),static_cast<unsigned>(p.elementBytes)))return 7;
                            std::printf(" PropertyRawHeader=0x%llX,0x%llX (diagnostic,no dereference authority)\n",value[0],value[1]);
                        }
                    }
                }
                field=f.next;
            }
            if(!r.Read(owner+cfgprobe::ClassSuper,owner))return 7;
        }
    }
    Context final;ArrayHeader last;
    if(!ReadContext(r,base,final)||!(context==final)||!ReadArrayHeader(r,base+ObjectArrayRva,last)||!(last==registry)||
       !r.Read(state+0x380,again)||std::memcmp(&players,&again,sizeof(players))||!ProcessPath(process)||WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return 7;
    for(const auto& c:states){Candidate finalState;if(!Identity(r,registry,c.object,finalState)||!Same(c,finalState))return 7;}
    for(const auto& c:widgets){Candidate finalWidget;if(!Identity(r,registry,c.object,finalWidget)||!Same(c,finalWidget))return 7;}
    std::printf("SummaryCaptureStable=yes Candidates=%zu Functions=%zu Reads=%u;active visibility/row filtering NOT inferred from loaded objects.\n",widgets.size(),captured.size(),r.reads);
    return widgets.empty()?8:0;
}
