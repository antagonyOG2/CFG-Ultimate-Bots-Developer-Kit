#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstring>
#include "../RuntimeProbe/CounselorBirthBindings.generated.hpp"
#include "../RuntimeProbe/SoftClassMarshaling.generated.hpp"
#include "../RuntimeProbe/DispatchObservation.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "NativeStartupRetention.hpp"

namespace cfgmenu::promotion {
struct alignas(8) Packet { std::array<unsigned char,40> bytes{}; };
static_assert(sizeof(Packet)==40 && alignof(Packet)==8);
// Caller supplies a fresh, positively identified owner-callback context, NOT a
// retained frontend plan. selectedClass must already be the native resolution
// of this exact picked packet (not preview/save/catalogue first-entry fallback).
// The host retains actor/field ownership for this synchronous call. Serial
// observations below are replacement detection, not a GC pin.
struct Context {
    std::uintptr_t module=0,engine=0,viewport=0,world=0,gameInstance=0;
    std::uintptr_t localPlayers=0,player=0,mode=0,controller=0,playerState=0;
    std::uintptr_t selectedClass=0,softProperty=0;
    std::uintptr_t modeVtable=0,restartGate=0,getterGate=0;
    DWORD thread=0;
    bool exactImageVerified=false,pickedResolutionVerified=false;
    cfgprobe::lifetime::ArrayHeader registry{};
    std::array<cfgprobe::lifetime::SlotObservation,4> identities{}; // world, mode, controller, state
    Packet picked{};
};
enum class Result { Refused, PromotedNoBirth, Forwarded, SpawnDenied, Indeterminate, CleanupFailed };
enum class Phase { Idle, Preparing, Forwarding, Finished, Faulted };
struct GetterResult { bool handled=false;std::uintptr_t type=0; };
inline std::array<std::uintptr_t,10> RetainedInputs(const Context& c)noexcept{
    // UObject dependencies only: not localPlayers' raw array, vtables, native
    // gates, FString storage or packet weak-cache words.
    return {c.world,c.mode,c.controller,c.playerState,c.selectedClass,c.softProperty,
        c.engine,c.viewport,c.gameInstance,c.player};
}

// These small SEH leaf bridges deliberately have no C++ ownership/destructors.
// Exposed only for focused tests of the actual Windows x64 calling convention.
namespace detail {
template<std::size_t N> bool Code(std::uintptr_t image,std::uintptr_t at,const std::array<unsigned char,N>& expected) noexcept {
    MEMORY_BASIC_INFORMATION m{};SIZE_T done=0;std::array<unsigned char,N> actual{};
    if(!VirtualQuery(reinterpret_cast<const void*>(at),&m,sizeof(m))||m.State!=MEM_COMMIT||m.Type!=MEM_IMAGE||
        reinterpret_cast<std::uintptr_t>(m.AllocationBase)!=image||
        (m.Protect!=PAGE_EXECUTE_READ&&m.Protect!=PAGE_EXECUTE_WRITECOPY))return false;
    return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(at),actual.data(),N,&done)&&done==N&&actual==expected;
}
inline bool Copy(std::uintptr_t fn,Packet* dst,const void* src) noexcept {
    __try {return reinterpret_cast<void*(__fastcall*)(void*,const void*)>(fn)(dst,src)==dst;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool Two(std::uintptr_t fn,std::uintptr_t self,void* argument) noexcept {
    __try {reinterpret_cast<void(__fastcall*)(void*,void*)>(fn)(reinterpret_cast<void*>(self),argument);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
inline bool Get(std::uintptr_t fn,std::uintptr_t self,std::uintptr_t controller,std::uintptr_t& out) noexcept {
    out=0;__try {out=reinterpret_cast<std::uintptr_t(__fastcall*)(void*,void*)>(fn)(reinterpret_cast<void*>(self),reinterpret_cast<void*>(controller));return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
}
class CurrentProcessReader {
public:
    template<class T> bool Read(std::uintptr_t at,T& out) noexcept {
        SIZE_T done=0;return cfgprobe::lifetime::Extent(at,sizeof(T))&&
            ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(at),&out,sizeof(T),&done)&&done==sizeof(T);
    }
};
// No process discovery, remote writes, VirtualProtect, hooks, factories or
// installation. The host must separately provide the two class-local gates.
class WindowsCalls {
    std::uintptr_t module_=0;DWORD thread_=0;bool exact_=false,faulted_=false;
    template<std::size_t N> bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& expected) noexcept {
        return detail::Code(module_,module_+rva,expected);
    }
    bool Guard() noexcept {
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        // Only the separately proved typed lifecycle scope can admit these
        // calls. A HUD retention scope still cannot authorize birth/restart.
        if(!startupretain::Current(module_))return false;
#endif
        if(!exact_||faulted_||GetCurrentThreadId()!=thread_||module_!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
        CurrentProcessReader r;DWORD owner=0;unsigned char flag=0;
        return r.Read(module_+cfgprobe::dispatch::ThreadIdRva,owner)&&owner==thread_&&
            r.Read(module_+cfgprobe::dispatch::ThreadFlagRva,flag)&&flag==1;
    }
    bool Finish(bool value) noexcept {if(!value)faulted_=true;return value;}
public:
    WindowsCalls(std::uintptr_t module,DWORD thread,bool exactImageVerified):module_(module),thread_(thread),exact_(exactImageVerified){}
    DWORD Thread() const noexcept{return GetCurrentThreadId();}
    bool Faulted() const noexcept{return faulted_;}
    bool Ready(const Context& c) noexcept {
        using namespace birthdisk;
        return c.module==module_&&c.thread==thread_&&c.exactImageVerified&&Guard()&&
            Code(CopyConstruct,CopyConstructCode)&&Code(ConsumeActive,ConsumeActiveCode)&&Code(birthdisk::Restart,RestartCode)&&
            Code(Getter,GetterCode)&&Code(ActiveNotify,ActiveNotifyCode)&&Code(SettingsNotify,SettingsNotifyCode)&&
            Code(PathCopy,PathCopyCode)&&Code(CanSpawnActive,CanSpawnActiveCode)&&Code(softdisk::Destroy,softdisk::DestroyCode);
    }
    bool Copy(const Context& c,Packet& p) noexcept {
        if(!Ready(c))return false;
        return Finish(startupretain::Invoke(module_,startupretain::Call::CopyPath,RetainedInputs(c),[&](){
            return detail::Copy(module_+birthdisk::CopyConstruct,&p,reinterpret_cast<const void*>(c.playerState+birthdisk::PickedCounselorClass));}));
    }
    bool Destroy(const Context& c,Packet& p) noexcept {
        if(!Guard()||!Code(softdisk::Destroy,softdisk::DestroyCode))return false;
        // Recheck immutable destructor schema even when actor ownership changed.
        CurrentProcessReader r;std::uintptr_t vt=0;int dim=0,bytes=0;
        if(!r.Read(c.softProperty,vt)||vt!=module_+softdisk::PropertyVtable||
            !r.Read(c.softProperty+0x30,dim)||dim!=1||!r.Read(c.softProperty+0x34,bytes)||bytes!=40)return false;
        return Finish(startupretain::Invoke(module_,startupretain::Call::DestroyPath,RetainedInputs(c),[&](){
            return detail::Two(module_+softdisk::Destroy,c.softProperty,&p);}));
    }
    bool Consume(const Context& c,Packet& p) noexcept {
        if(!Ready(c))return false;
        // Native writes only to an ordinary writable instance. Never change its
        // protection or attempt a protected/CDO write to make a refusal pass.
        MEMORY_BASIC_INFORMATION memory{};const auto at=c.playerState+birthdisk::ActiveCharacterClass;
        if(!VirtualQuery(reinterpret_cast<const void*>(at),&memory,sizeof(memory))||memory.State!=MEM_COMMIT||memory.Type!=MEM_PRIVATE||
            (memory.Protect!=PAGE_READWRITE&&memory.Protect!=PAGE_WRITECOPY)||
            at<reinterpret_cast<std::uintptr_t>(memory.BaseAddress)||
            at-reinterpret_cast<std::uintptr_t>(memory.BaseAddress)>memory.RegionSize||
            memory.RegionSize-(at-reinterpret_cast<std::uintptr_t>(memory.BaseAddress))<sizeof(Packet))return false;
        return Finish(startupretain::Invoke(module_,startupretain::Call::ConsumeActive,RetainedInputs(c),[&](){
            return detail::Two(module_+birthdisk::ConsumeActive,c.playerState,&p);}));
    }
    bool Restart(const Context& c) noexcept {
        if(!Ready(c))return false;
        return Finish(startupretain::Invoke(module_,startupretain::Call::RestartHuman,RetainedInputs(c),[&](){
            return detail::Two(module_+birthdisk::Restart,c.mode,reinterpret_cast<void*>(c.controller));}));
    }
    bool Get(const Context& c,std::uintptr_t& type) noexcept {
        if(!Ready(c))return false;
        std::array<std::uintptr_t,1> output{};
        return Finish(startupretain::Invoke(module_,startupretain::Call::LoadClass,RetainedInputs(c),output,[&](){
            const bool returned=detail::Get(module_+birthdisk::Getter,c.mode,c.controller,type);output[0]=type;return returned;}));
    }
};

template<class Reader> class Bounded {
    Reader& r_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):r_(r){}
    template<class T> bool Read(std::uintptr_t p,T& out){if(reads==512)return false;++reads;return r_.Read(p,out);}
};
template<class Reader> bool Current(Reader& reader,const Context& c,DWORD thread,bool requireActive) {
    using namespace cfgprobe;using namespace birthdisk;
    Bounded<Reader> r(reader);
    if(!c.exactImageVerified||!c.pickedResolutionVerified||!c.thread||thread!=c.thread||!c.registry.Sane())return false;
    const std::array<std::uintptr_t,15> addresses{{c.module,c.engine,c.viewport,c.world,c.gameInstance,c.localPlayers,c.player,
        c.mode,c.controller,c.playerState,c.selectedClass,c.softProperty,c.modeVtable,c.restartGate,c.getterGate}};
    for(auto p:addresses)if(!ObjectPointer(p))return false;
    if(!lifetime::Extent(c.module,cfgprobe::ExecutableImageBytes)||!lifetime::Extent(c.playerState,0x940)||
       !lifetime::Extent(c.controller,ControllerAcknowledgedPawn+8)||!lifetime::Extent(c.modeVtable,RestartSlot+8))return false;
    const auto same=[&](std::uintptr_t at,std::uintptr_t expected){std::uintptr_t v=0;return r.Read(at,v)&&v==expected;};
    std::array<std::uintptr_t,4> objects{{c.world,c.mode,c.controller,c.playerState}};
    for(unsigned i=0;i<objects.size();++i){if(!c.identities[i].HasAssignedSerial()||c.identities[i].object!=objects[i])return false;
        for(unsigned j=0;j<i;++j)if(objects[i]==objects[j]||c.identities[i].index==c.identities[j].index)return false;}
    if(!lifetime::RevalidateBatch(r,c.module+lifetime::ObjectArrayRva,c.registry,c.identities)||
        !same(c.module+EngineGlobalCandidate,c.engine)||!same(c.engine+EngineGameViewport,c.viewport)||!same(c.viewport+ViewportWorld,c.world)||
        !same(c.world+WorldNetDriver,0)||!same(c.world+WorldGameMode,c.mode)||!same(c.world+WorldGameInstance,c.gameInstance)||
        !same(c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)||!same(c.localPlayers,c.player)||
        !same(c.player+PlayerController,c.controller)||!same(c.controller+ControllerPlayer,c.player)||
        !same(c.controller+ControllerPawn,0)||!same(c.controller+ControllerAcknowledgedPawn,0)||
        !same(c.controller+0x388,c.playerState)||!same(c.playerState+0x88,c.controller)||
        !same(c.playerState+SpawnedCharacterClass,0)||!same(c.mode,c.modeVtable)||
        !same(c.modeVtable+RestartSlot,c.restartGate)||!same(c.modeVtable+GetterSlot,c.getterGate)||
        c.getterGate==c.module+Getter||c.restartGate==c.module+Restart)return false;
    int num=0,cap=0;std::uint32_t flags=0;unsigned char role=0;
    if(!r.Read(c.gameInstance+GameInstanceLocalPlayers+8,num)||num!=1||!r.Read(c.gameInstance+GameInstanceLocalPlayers+12,cap)||cap<1||cap>1024||
        !r.Read(c.playerState+0x394,flags)||(flags&6)||!r.Read(c.playerState+0x110,role)||role!=3)return false;
    const auto cache=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t& out){std::uintptr_t second=0;return r.Read(c.module+a,out)&&r.Read(c.module+b,second)&&MatchingClassCache(out,second);};
    std::uintptr_t mode=0,controller=0,state=0,counselor=0,killer=0;
    if(!cache(SCGameMode_SandboxClass,SCGameMode_SandboxConstructor,mode)||!cache(SCPlayerController_SandboxClass,SCPlayerController_SandboxConstructor,controller)||
        !cache(SCPlayerStateClass,SCPlayerStateConstructor,state)||!cache(SCCounselorCharacterClass,SCCounselorCharacterConstructor,counselor)||
        !cache(SCKillerCharacterClass,SCKillerCharacterConstructor,killer)||!same(c.mode+ObjectClass,mode))return false;
    Ancestry a,b;if(!CaptureAncestry(r,c.controller,a)||!a.Contains(controller)||!StableAncestry(r,c.controller,a)||
        !CaptureAncestry(r,c.playerState,b)||!b.Contains(state)||!StableAncestry(r,c.playerState,b))return false;
    std::uintptr_t defaults=0;
    if(!r.Read(mode+0xF8,defaults)||defaults==c.mode||!r.Read(a.classes[0]+0xF8,defaults)||defaults==c.controller||
        !r.Read(b.classes[0]+0xF8,defaults)||defaults==c.playerState)return false;
    // Class ancestry is walked as classes, not by pretending the UClass is a pawn.
    std::array<std::uintptr_t,MaximumAncestry> classes{},parents{};std::size_t count=0;
    auto klass=c.selectedClass;bool isCounselor=false;
    while(klass){if(!ObjectPointer(klass)||count==classes.size()||klass==killer)return false;
        for(std::size_t i=0;i<count;++i)if(classes[i]==klass)return false;
        isCounselor|=klass==counselor;classes[count]=klass;
        if(!r.Read(klass+ClassSuper,parents[count]))return false;klass=parents[count++];}
    if(!isCounselor)return false;
    for(std::size_t i=0;i<count;++i)if(!same(classes[i]+ClassSuper,parents[i]))return false;
    Packet picked,again,active;std::uint64_t path=0;std::int32_t chars=0,capacity=0;
    if(!r.Read(c.playerState+PickedCounselorClass,picked)||picked.bytes!=c.picked.bytes)return false;
    std::memcpy(&path,picked.bytes.data()+16,8);std::memcpy(&chars,picked.bytes.data()+32,4);std::memcpy(&capacity,picked.bytes.data()+36,4);
    if(!path||chars<0||capacity<chars||capacity>2048)return false;
    if(!same(c.softProperty,c.module+softdisk::PropertyVtable)||!same(c.softProperty+0x20,state)||
        !r.Read(c.softProperty+0x44,num)||num!=static_cast<int>(PickedCounselorClass)||
        !r.Read(c.softProperty+0x30,num)||num!=1||!r.Read(c.softProperty+0x34,cap)||cap!=40)return false;
    if(requireActive){if(!r.Read(c.playerState+ActiveCharacterClass,active))return false;
        // Native copy may update weak cache fields. Equality here is path identity;
        // exact resolved class is enforced by the final native getter below.
        if(std::memcmp(active.bytes.data()+16,picked.bytes.data()+16,8)!=0)return false;}
    return r.Read(c.playerState+PickedCounselorClass,again)&&again.bytes==picked.bytes&&
        lifetime::RevalidateBatch(r,c.module+lifetime::ObjectArrayRva,c.registry,c.identities)&&
        same(c.controller+ControllerPawn,0)&&same(c.controller+ControllerAcknowledgedPawn,0);
}

// One-shot, synchronous adapter. It never owns hook installation or retries.
// Keep the adapter alive after Indeterminate/CleanupFailed: its packet is quarantined and no
// destructor guesses whether a failed native call allocated/freed its string.
template<class Reader,class Calls=WindowsCalls> class NativeCounselorPromotion {
    Reader& reader_;Calls& calls_;Context c_{};Packet owned_{};Phase phase_=Phase::Idle;
    bool denied_=false,checkingGetter_=false;unsigned getters_=0;
    Result Fault(){phase_=Phase::Faulted;return Result::Indeterminate;}
    Result Refuse(){phase_=Phase::Finished;return Result::Refused;}
public:
    NativeCounselorPromotion(Reader& reader,Calls& calls):reader_(reader),calls_(calls){}
    NativeCounselorPromotion(const NativeCounselorPromotion&)=delete;
    NativeCounselorPromotion& operator=(const NativeCounselorPromotion&)=delete;
    Phase State() const noexcept{return phase_;}
    GetterResult PawnClass(std::uintptr_t mode,std::uintptr_t controller){
        if(phase_==Phase::Idle||mode!=c_.mode||controller!=c_.controller)return {};
        GetterResult out{true,0};
        // Includes setter notification reentry, late calls and recursive getter.
        if(phase_!=Phase::Forwarding||checkingGetter_||getters_==4){denied_=true;return out;}
        ++getters_;checkingGetter_=true;
        if(Current(reader_,c_,calls_.Thread(),true)&&calls_.Ready(c_)){
            std::uintptr_t type=0;
            if(calls_.Get(c_,type)&&!denied_&&type==c_.selectedClass&&Current(reader_,c_,calls_.Thread(),true))out.type=type;
        }
        checkingGetter_=false;if(!out.type)denied_=true;if(calls_.Faulted())phase_=Phase::Faulted;
        return out;
    }
    Result Restart(const Context& c){
        if(phase_!=Phase::Idle){denied_=true;return Result::Refused;}
        c_=c;phase_=Phase::Preparing; // BEFORE any native call/broadcast.
        if(!Current(reader_,c_,calls_.Thread(),false)||!calls_.Ready(c_))return Refuse();
        if(!calls_.Copy(c_,owned_))return calls_.Faulted()?Fault():Refuse();
        if(!Current(reader_,c_,calls_.Thread(),false)||denied_||!calls_.Ready(c_)){
            if(!calls_.Destroy(c_,owned_)){phase_=Phase::Faulted;return calls_.Faulted()?Result::Indeterminate:Result::CleanupFailed;}
            owned_={};return Refuse();
        }
        // Ownership transfers at call entry; never Destroy after this point,
        // even if the native callee or a broadcast listener raises an exception.
        if(!calls_.Consume(c_,owned_))return Fault();
        owned_={};
        if(denied_||!Current(reader_,c_,calls_.Thread(),true)||!calls_.Ready(c_)){phase_=Phase::Finished;return Result::PromotedNoBirth;}
        phase_=Phase::Forwarding;
        if(!calls_.Restart(c_))return Fault();
        if(phase_==Phase::Faulted||calls_.Faulted())return Fault();
        phase_=Phase::Finished;
        // Forwarded is not arrival/possession success; host verifies that later.
        return denied_||!getters_?Result::SpawnDenied:Result::Forwarded;
    }
};
}
