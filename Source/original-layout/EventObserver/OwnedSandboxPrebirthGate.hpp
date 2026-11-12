#pragma once
#include "NativeCounselorPromotion.hpp"
#include "../RuntimeProbe/SandboxGateBindings.generated.hpp"

namespace cfgmenu::promotion::gate {
using RestartFunction=void(__fastcall*)(void*,void*);
using GetterFunction=std::uintptr_t(__fastcall*)(void*,void*);
struct Gates {
    RestartFunction restart=nullptr;GetterFunction getter=nullptr;
    std::uintptr_t image=0; // Host DLL containing BOTH compiled, CFG-valid gates.
};
enum class State { Empty, Installed, Retired, Quarantined };
enum class InstallResult { Refused, Installed, Quarantined };
using Table=std::array<std::uintptr_t,gatedisk::Slots>;
static_assert(sizeof(Table)==gatedisk::Bytes && gatedisk::PrefixBytes==0);

// Serial observations are replacement detection, not lifetime pins. Host must
// own these objects, execute on the engine owner thread, prevent concurrent
// virtual dispatch, and retain the gate DLL for the whole installed lifetime.
// No selected-character resolution is needed to install this PRE-birth gate.
template<class Reader> bool OwnerCurrent(Reader& reader,const Context& c,DWORD thread,
                                        std::uintptr_t table,bool prebirth) {
    using namespace cfgprobe;
    Bounded<Reader> r(reader);
    if(!c.exactImageVerified||!thread||thread!=c.thread||!c.registry.Sane()||!ObjectPointer(table)||
       !lifetime::Extent(c.module,cfgprobe::ExecutableImageBytes))return false;
    const std::array<std::uintptr_t,4> actors{{c.world,c.mode,c.controller,c.playerState}};
    for(unsigned i=0;i<actors.size();++i){if(!c.identities[i].HasAssignedSerial()||c.identities[i].object!=actors[i])return false;
        for(unsigned j=0;j<i;++j)if(actors[i]==actors[j]||c.identities[i].index==c.identities[j].index)return false;}
    const auto same=[&](std::uintptr_t at,std::uintptr_t want){std::uintptr_t value=0;return r.Read(at,value)&&value==want;};
    const std::array<std::uintptr_t,10> ptrs{{c.engine,c.viewport,c.world,c.gameInstance,c.localPlayers,c.player,c.mode,c.controller,c.playerState,c.module}};
    for(auto p:ptrs)if(!ObjectPointer(p))return false;
    if(!lifetime::RevalidateBatch(r,c.module+lifetime::ObjectArrayRva,c.registry,c.identities)||
       !same(c.module+EngineGlobalCandidate,c.engine)||!same(c.engine+EngineGameViewport,c.viewport)||
       !same(c.viewport+ViewportWorld,c.world)||!same(c.world+WorldNetDriver,0)||
       !same(c.world+WorldGameMode,c.mode)||!same(c.world+WorldGameInstance,c.gameInstance)||
       !same(c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)||!same(c.localPlayers,c.player)||
       !same(c.player+PlayerController,c.controller)||!same(c.controller+ControllerPlayer,c.player)||
       !same(c.controller+0x388,c.playerState)||!same(c.playerState+0x88,c.controller)||!same(c.mode,table))return false;
    int count=0,cap=0;std::uintptr_t klass=0,second=0,defaults=0;
    if(!r.Read(c.gameInstance+GameInstanceLocalPlayers+8,count)||count!=1||
       !r.Read(c.gameInstance+GameInstanceLocalPlayers+12,cap)||cap<1||cap>1024||
       !r.Read(c.module+birthdisk::SCGameMode_SandboxClass,klass)||
       !r.Read(c.module+birthdisk::SCGameMode_SandboxConstructor,second)||!MatchingClassCache(klass,second)||
       !same(c.mode+ObjectClass,klass)||!r.Read(klass+0xF8,defaults)||!ObjectPointer(defaults)||defaults==c.mode||
       !same(defaults+ObjectClass,klass))return false;
    if(prebirth&&(!same(c.controller+ControllerPawn,0)||!same(c.controller+ControllerAcknowledgedPawn,0)||
                 !same(c.playerState+birthdisk::SpawnedCharacterClass,0)))return false;
    return lifetime::RevalidateBatch(r,c.module+lifetime::ObjectArrayRva,c.registry,c.identities)&&
           same(c.mode,table)&&same(c.world+WorldGameMode,c.mode);
}
template<class Reader> bool ExactTable(Reader& r,std::uintptr_t module,std::uintptr_t at,
                                     std::uintptr_t restart=0,std::uintptr_t getter=0) {
    if(!cfgprobe::lifetime::Extent(at,sizeof(Table)))return false;
    for(std::size_t i=0;i<gatedisk::Slots;++i){
        std::uintptr_t actual=0,expected=module+gatedisk::Entries[i];
        if(i==birthdisk::RestartSlot/8&&restart)expected=restart;
        if(i==birthdisk::GetterSlot/8&&getter)expected=getter;
        if(!r.Read(at+i*8,actual)||actual!=expected)return false;
    }
    return true;
}

class WindowsPublication {
    static bool ImageCode(std::uintptr_t pointer,std::uintptr_t image) noexcept {
        MEMORY_BASIC_INFORMATION m{};
        return pointer&&image&&VirtualQuery(reinterpret_cast<const void*>(pointer),&m,sizeof(m))&&
            m.State==MEM_COMMIT&&m.Type==MEM_IMAGE&&reinterpret_cast<std::uintptr_t>(m.AllocationBase)==image&&
            (m.Protect==PAGE_EXECUTE_READ||m.Protect==PAGE_EXECUTE_WRITECOPY);
    }
public:
    DWORD Thread()const noexcept{return GetCurrentThreadId();}
    bool Ready(const Context& c,const Gates& g) noexcept {
        if(!c.exactImageVerified||c.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||Thread()!=c.thread||
           g.image==c.module||!ImageCode(reinterpret_cast<std::uintptr_t>(g.restart),g.image)||
           !ImageCode(reinterpret_cast<std::uintptr_t>(g.getter),g.image))return false;
        CurrentProcessReader r;DWORD owner=0;unsigned char flag=0;
        return r.Read(c.module+cfgprobe::dispatch::ThreadIdRva,owner)&&owner==c.thread&&
            r.Read(c.module+cfgprobe::dispatch::ThreadFlagRva,flag)&&flag==1&&
            detail::Code(c.module,c.module+gatedisk::SandboxConstructor,gatedisk::SandboxConstructorCode)&&
            detail::Code(c.module,c.module+gatedisk::NextConstructor,gatedisk::NextConstructorCode)&&
            detail::Code(c.module,c.module+gatedisk::LastSlotCaller,gatedisk::LastSlotCallerCode);
    }
    std::uintptr_t Allocate(const Table& table) noexcept {
        auto* p=HeapAlloc(GetProcessHeap(),0,sizeof(table));if(!p)return 0;
        std::memcpy(p,table.data(),sizeof(table));return reinterpret_cast<std::uintptr_t>(p);
    }
    void Free(std::uintptr_t table) noexcept {if(table)HeapFree(GetProcessHeap(),0,reinterpret_cast<void*>(table));}
    // Current process only; writes one naturally aligned, already writable
    // object vptr. Never modifies a shared table or any page protection.
    bool CompareExchange(std::uintptr_t object,std::uintptr_t expected,std::uintptr_t desired,
                         std::uintptr_t& previous) noexcept {
        previous=0;MEMORY_BASIC_INFORMATION m{};
        if(!cfgprobe::ObjectPointer(object)||!VirtualQuery(reinterpret_cast<const void*>(object),&m,sizeof(m))||
           m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||m.Protect!=PAGE_READWRITE||
           object-reinterpret_cast<std::uintptr_t>(m.BaseAddress)>m.RegionSize-sizeof(void*))return false;
        __try {previous=reinterpret_cast<std::uintptr_t>(InterlockedCompareExchangePointer(
            reinterpret_cast<void* volatile*>(object),reinterpret_cast<void*>(desired),reinterpret_cast<void*>(expected)));return true;}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
};

// Installation only: supplied compiled gates route callbacks to the host's
// one-shot NativeCounselorPromotion. No automatic native calls or discovery.
// Do NOT put this object on a temporary stack while installed. On uncertain
// teardown its private allocation is deliberately retained; the host must keep
// its callback registry and code loaded, too. No destructor guesses restoration.
template<class Reader,class Publication=WindowsPublication> class OwnedSandboxPrebirthGate {
    Reader& reader_;Publication& publication_;Context c_{};Gates gates_{};
    State state_=State::Empty;std::uintptr_t clone_=0;
    std::uintptr_t RestartAddress()const{return reinterpret_cast<std::uintptr_t>(gates_.restart);}
    std::uintptr_t GetterAddress()const{return reinterpret_cast<std::uintptr_t>(gates_.getter);}
    bool Current(std::uintptr_t table,bool before){return publication_.Ready(c_,gates_)&&
        OwnerCurrent(reader_,c_,publication_.Thread(),table,before);}
    void Release(){publication_.Free(clone_);clone_=0;state_=State::Retired;}
public:
    OwnedSandboxPrebirthGate(Reader& r,Publication& p):reader_(r),publication_(p){}
    OwnedSandboxPrebirthGate(const OwnedSandboxPrebirthGate&)=delete;
    OwnedSandboxPrebirthGate& operator=(const OwnedSandboxPrebirthGate&)=delete;
    State Status()const noexcept{return state_;}
    std::uintptr_t PrivateTable()const noexcept{return clone_;}
    InstallResult Install(const Context& c,const Gates& gates,bool ownerQuiescent){
        if(state_!=State::Empty)return InstallResult::Refused;
        c_=c;gates_=gates;
        const auto native=c.module+gatedisk::Table;
        if(!ownerQuiescent||!gates.restart||!gates.getter||RestartAddress()==GetterAddress()||
           RestartAddress()==c.module+birthdisk::Restart||GetterAddress()==c.module+birthdisk::Getter||
           !Current(native,true)||!ExactTable(reader_,c.module,native))return InstallResult::Refused;
        Table entries{};for(std::size_t i=0;i<entries.size();++i)entries[i]=c.module+gatedisk::Entries[i];
        entries[birthdisk::RestartSlot/8]=RestartAddress();entries[birthdisk::GetterSlot/8]=GetterAddress();
        clone_=publication_.Allocate(entries);if(!clone_)return InstallResult::Refused;
        if(!Current(native,true)||!ExactTable(reader_,c.module,native)||
           !ExactTable(reader_,c.module,clone_,RestartAddress(),GetterAddress())){Release();return InstallResult::Refused;}
        // State/context exist BEFORE publication; dispatcher must also register
        // this host route before calling Install, while owner dispatch is quiet.
        state_=State::Quarantined;std::uintptr_t previous=0;
        if(!publication_.CompareExchange(c.mode,native,clone_,previous))return InstallResult::Quarantined;
        if(previous!=native){Release();return InstallResult::Refused;}
        if(!Current(clone_,true)||!ExactTable(reader_,c.module,clone_,RestartAddress(),GetterAddress())){
            // Only restore if the SAME current owner still carries our vptr.
            if(Restore(ownerQuiescent))return InstallResult::Refused;
            return InstallResult::Quarantined;
        }
        state_=State::Installed;return InstallResult::Installed;
    }
    bool Restore(bool ownerQuiescent){
        if(!ownerQuiescent||!clone_||(state_!=State::Installed&&state_!=State::Quarantined)||
           !Current(clone_,false)||!ExactTable(reader_,c_.module,c_.module+gatedisk::Table))return false;
        std::uintptr_t previous=0;
        if(!publication_.CompareExchange(c_.mode,clone_,c_.module+gatedisk::Table,previous)||previous!=clone_){
            state_=State::Quarantined;return false;}
        Release();return true;
    }
    // Host merges only the proven installed transport fields into a FRESH
    // complete selection/lifetime context before native promotion.
    bool Bind(Context& fresh){
        if(state_!=State::Installed||fresh.module!=c_.module||fresh.mode!=c_.mode||fresh.thread!=c_.thread||
           !(fresh.identities[1]==c_.identities[1])||!Current(clone_,false)||
           !ExactTable(reader_,c_.module,clone_,RestartAddress(),GetterAddress()))return false;
        fresh.modeVtable=clone_;fresh.restartGate=RestartAddress();fresh.getterGate=GetterAddress();return true;
    }
};
}
