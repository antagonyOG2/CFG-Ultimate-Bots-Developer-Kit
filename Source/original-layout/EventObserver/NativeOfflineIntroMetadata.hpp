#pragma once
#include "NativeOfflineLifecycleCalls.hpp"
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/OfflineIntroMetadataBindings.generated.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/NativeClassPathChoice.hpp"
#include "../Core/EndingMetadataPolicy.hpp"
#include "../Core/NativeSpawnedClassCache.hpp"
#include "../Core/NativeStartupDiagnostic.hpp"
namespace cfgmenu::intrometadata {
using Packet=promotion::Packet;
inline bool Plain(const Packet& p){return cfgending::EmptySubpath(p.bytes);}
inline bool SamePath(const Packet& a,const Packet& b){return cfgending::SamePath(a.bytes,b.bytes);}
inline bool EmptyOrPlain(const Packet& p){
    for(unsigned i=24;i<40;++i)if(p.bytes[i])return false;
    return true;
}
inline bool SameValue(const Packet& a,const Packet& b){
    return EmptyOrPlain(a)&&EmptyOrPlain(b)&&std::memcmp(a.bytes.data()+16,b.bytes.data()+16,8)==0;
}
// The verified CopyValuesInternal leaf ignores its property argument. Restrict
// it to one plain packet: the transitive FString assignment then cannot enter
// the allocator. Keep this check at the actual ABI boundary as well as Fresh.
inline bool AssignPlain(std::uintptr_t fn,Packet* destination,const Packet* source)noexcept{
    __try{
        if(!fn||!destination||!source||destination==source||!EmptyOrPlain(*destination)||!EmptyOrPlain(*source))return false;
        reinterpret_cast<void(__fastcall*)(void*,void*,const void*,int)>(fn)(nullptr,destination,source,1);return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
struct Context {
    NativeMetadataDiagnostic* diagnostic=nullptr;
    promotion::Context owner{};
    std::uintptr_t gameState=0,pawn=0,killerClass=0,counselorSource=0,killerSource=0;
    std::array<cfgprobe::lifetime::SlotObservation,2> identities{}; // state, born human pawn
    Packet counselor{},killer{};
};
enum class State { Idle,Preparing,Borrowed,Restoring,Restored,Refused,Indeterminate };
// Host operations are compare-before-write native calls. No destructor writes:
// a world transition or third-party writer must never trigger stale rollback.
template<class Host> class Borrow {
    Host& host_;Packet beforeActive_{},beforeClass_{};
    std::uintptr_t beforeOwner_=0,human_=0;Packet killer_{};
    State state_=State::Idle;
    unsigned progress_=0;
    bool active_=false,klass_=false,owner_=false,reentered_=false;
public:
    explicit Borrow(Host& host):host_(host){}
    State Status()const{return state_;}
    unsigned Progress()const{return progress_;}
    bool HumanRestartForbidden()const{return state_==State::Preparing||state_==State::Borrowed||state_==State::Restoring||state_==State::Indeterminate;}
    bool Prepare(){
        if(state_!=State::Idle){reentered_=true;return false;}state_=State::Preparing;
        progress_=1;killer_=host_.Killer();human_=host_.Human();
        if(!Plain(killer_)||!host_.Fresh()||!host_.Read(beforeActive_,beforeClass_,beforeOwner_)||
           !Plain(beforeActive_)||!EmptyOrPlain(beforeClass_)||beforeOwner_||!SamePath(beforeActive_,host_.Counselor())){
            state_=State::Refused;return false;
        }
        // Record attempted ownership before broadcasts/native entry. Failures
        // are indeterminate and must not repeat or free guessed allocations.
        progress_=2;klass_=true;
        if(!host_.SetClass(beforeClass_,killer_)||reentered_||!host_.Fresh())return Fault();
        progress_=3;active_=true;
        if(!host_.SetActive(beforeActive_,killer_)||reentered_||!host_.Fresh())return Fault();
        progress_=4;owner_=true;
        if(!host_.Publish(beforeOwner_,human_)||reentered_||!host_.Fresh())return Fault();
        progress_=5;Packet a{},k{};std::uintptr_t o=0;
        if(!host_.Read(a,k,o)||!SamePath(a,killer_)||!SamePath(k,killer_)||o!=human_)return Fault();
        progress_=6;state_=State::Borrowed;return true;
    }
    // Call after AI Jason adoption has restored/published its own metadata, or
    // on a synchronous refusal while the captured world is still owned.
    bool Restore(){
        if(state_==State::Restored)return true;
        if(state_!=State::Borrowed){reentered_=true;return false;}
        state_=State::Restoring;
        if(!host_.Fresh())return Fault();
        Packet a{},k{};std::uintptr_t o=0;
        if(!host_.Read(a,k,o))return Fault();
        // A later AI owner owns GameState metadata. Do not undo its class or
        // pointer, even when it happens to have the same selected class path.
        if(active_&&SamePath(a,killer_)){
            if(!host_.SetActive(a,beforeActive_)||reentered_||!host_.Fresh())return Fault();
        }else if(active_&&!SamePath(a,beforeActive_))return Fault();
        // SetActive broadcasts. Re-read owner AND class after it; a later AI
        // owner can publish the same killer path during that callback. The
        // pre-call owner sample cannot authorize restoring its GameState data.
        if(!host_.Read(a,k,o)||(active_&&!SamePath(a,beforeActive_)))return Fault();
        const bool ownState=o==human_;
        if(ownState&&klass_&&SamePath(k,killer_)){
            if(!host_.SetClass(k,beforeClass_)||reentered_||!host_.Fresh())return Fault();
        }else if(ownState&&klass_&&!SameValue(k,beforeClass_))return Fault();
        if(ownState&&owner_&&!host_.Publish(human_,beforeOwner_))return Fault();
        state_=State::Restored;return true;
    }
private:
    bool Fault(){state_=State::Indeterminate;return false;}
};

// A synchronous metadata lease: one typed hierarchy lookup, followed by exact
// retained-address reads. Never rediscover/adopt a replacement field. Sibling
// links are retained too, so insertion/unlinking cannot silently change lookup
// precedence. Revalidation does not decode names or walk the current hierarchy.
class SchemaLease {
    bool attempted_=false,ready_=false;std::uintptr_t module_=0,object_=0,vtable_=0;
    unsigned offset_=0;bool soft_=false;cfgreflection::Field field_{};cfgnames::Words kindName_{};
    cfgprobe::Ancestry ancestry_{};
    std::array<std::uintptr_t,cfgprobe::MaximumAncestry> heads_{};unsigned owners_=0;
    struct Link{std::uintptr_t object=0,next=0;};std::array<Link,256> links_{};unsigned linksCount_=0;
    bool RetainDependencies(){
        if(!startupretain::Retain(module_,std::array<std::uintptr_t,4>{object_,field_.object,field_.type,field_.owner}))return false;
        for(unsigned first=0;first<owners_;first+=32){std::array<std::uintptr_t,32> values{};
            for(unsigned i=first;i<first+32&&i<owners_;++i)values[i-first]=ancestry_.classes[i];
            if(!startupretain::Retain(module_,values))return false;
        }
        // Retain the complete observed sibling chains, not only named target
        // properties. The owning startup provider must keep these claims over
        // AwaitMatch and retire them only after metadata restoration.
        for(unsigned first=0;first<linksCount_;first+=32){std::array<std::uintptr_t,32> values{};
            for(unsigned i=first;i<first+32&&i<linksCount_;++i)values[i-first]=links_[i].object;
            if(!startupretain::Retain(module_,values))return false;
        }
        return true;
    }
    template<class R,class T> static bool Same(R& r,std::uintptr_t at,const T& expected){T actual{};return r.Read(at,actual)&&actual==expected;}
    template<class R> bool Stable(R& r){
        if(!cfgprobe::StableAncestry(r,object_,ancestry_))return false;
        for(unsigned i=0;i<owners_;++i)if(!Same(r,ancestry_.classes[i]+cfgreflection::disk::Children,heads_[i]))return false;
        for(unsigned i=0;i<linksCount_;++i)if(!Same(r,links_[i].object+cfgreflection::disk::Next,links_[i].next))return false;
        return Same(r,field_.object,vtable_)&&Same(r,field_.object+cfgprobe::ObjectClass,field_.type)&&
            Same(r,field_.object+0x20,field_.owner)&&Same(r,field_.object+cfgnames::disk::ObjectName,field_.name)&&
            Same(r,field_.type+cfgnames::disk::ObjectName,kindName_)&&
            Same(r,field_.object+0x30,int{1})&&Same(r,field_.object+0x34,soft_?40:8)&&
            Same(r,field_.object+0x44,static_cast<int>(offset_));
    }
public:
    template<class R> bool Observe(R& r,std::uintptr_t module,std::uintptr_t object,std::u16string_view name,unsigned offset,bool soft){
        if(attempted_){
            if(!ready_||module!=module_||object!=object_||offset!=offset_||soft!=soft_||name!=field_.label.View())return false;
            if(!Stable(r)){ready_=false;return false;}return true;
        }
        attempted_=true;module_=module;object_=object;offset_=offset;soft_=soft;
        cfgreflection::Observation<R> obs(r,module);
        if(!cfgprobe::CaptureAncestry(r,object,ancestry_)||!obs.Find(ancestry_.classes[0],name,field_)||
           !ancestry_.Contains(field_.owner)||field_.kind.View()!=(soft?u"SoftClassProperty":u"ObjectProperty")||
           !r.Read(field_.object,vtable_)||!cfgprobe::ObjectPointer(vtable_)||
           (soft&&vtable_!=module+softdisk::PropertyVtable)||
           !r.Read(field_.type+cfgnames::disk::ObjectName,kindName_))return false;
        bool found=false;
        for(unsigned i=0;i<ancestry_.count;++i){
            if(!r.Read(ancestry_.classes[i]+cfgreflection::disk::Children,heads_[i]))return false;++owners_;
            auto child=heads_[i];
            while(child){
                if(!cfgprobe::ObjectPointer(child)||linksCount_==links_.size())return false;
                for(unsigned j=0;j<linksCount_;++j)if(links_[j].object==child)return false;
                Link link{child,0};if(!r.Read(child+cfgreflection::disk::Next,link.next))return false;
                links_[linksCount_++]=link;if(child==field_.object)found=true;child=link.next;
            }
            if(ancestry_.classes[i]==field_.owner)break;
        }
        ready_=found&&Stable(r)&&RetainDependencies();return ready_;
    }
};

// Current-process host. Fresh validates class caches, assigned serials,
// offline world ownership, exact possession, selected packets and named fields
// before EVERY mutation. The caller owns synchronous callback admission.
template<class Reader> class WindowsHost {
    Reader& r_;Context c_;offlinelifecycle::Calls lifecycle_;bool fault_=false,published_=false;
    NativeMetadataDiagnostic localDiagnostic_{};
    NativeMetadataDiagnostic& Diagnostic(){return c_.diagnostic?*c_.diagnostic:localDiagnostic_;}
    bool SampleSame(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t& sample,unsigned bit){
        const bool read=r_.Read(at,sample);if(read)Diagnostic().observedMask|=bit;return read&&sample==expected;
    }
    std::array<SchemaLease,3> schemas_{};
    template<class T> bool Same(std::uintptr_t at,const T& expected){T v{};return r_.Read(at,v)&&v==expected;}
    template<std::size_t N> bool Code(std::uintptr_t rva,const std::array<unsigned char,N>& bytes){return promotion::detail::Code(c_.owner.module,c_.owner.module+rva,bytes);}
    bool Writable(std::uintptr_t at,std::size_t n){
        MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(at),&m,sizeof(m))||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||
           (m.Protect!=PAGE_READWRITE&&m.Protect!=PAGE_WRITECOPY))return false;
        const auto base=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
        return at>=base&&at-base<=m.RegionSize&&n<=m.RegionSize-(at-base);
    }
    bool Exact(std::uintptr_t object,std::uintptr_t cache){
        std::uintptr_t a=0,b=0,defaults=0;
        return r_.Read(c_.owner.module+cache,a)&&r_.Read(c_.owner.module+cache+8,b)&&cfgprobe::MatchingClassCache(a,b)&&
            Same(object+cfgprobe::ObjectClass,a)&&r_.Read(a+0xF8,defaults)&&cfgprobe::ObjectPointer(defaults)&&defaults!=object;
    }
    static bool Exchange(std::uintptr_t at,std::uintptr_t old,std::uintptr_t value,NativeMetadataDiagnostic& diagnostic)noexcept{
        __try{diagnostic.publishObserved=reinterpret_cast<std::uintptr_t>(InterlockedCompareExchangePointer(
            reinterpret_cast<void* volatile*>(at),reinterpret_cast<void*>(value),reinterpret_cast<void*>(old)));
            diagnostic.publishStatus=1;return diagnostic.publishObserved==old;}
        __except(EXCEPTION_EXECUTE_HANDLER){diagnostic.publishStatus=2;return false;}
    }
public:
    WindowsHost(Reader& r,const Context& c):r_(r),c_(c),lifecycle_(c.owner){}
    Packet Killer()const{return c_.killer;}Packet Counselor()const{return c_.counselor;}
    std::uintptr_t Human()const{return c_.owner.playerState;}
    bool Fresh(){
        using namespace cfgprobe;namespace d=intrometadatadisk;
        auto& diagnostic=Diagnostic();diagnostic.observedMask=0;diagnostic.hostState=(fault_?1u:0u)|(published_?2u:0u);
        diagnostic.expectedPawnClass=c_.owner.selectedClass;diagnostic.expectedKillerClass=c_.killerClass;
        const auto stage=[&](unsigned value){diagnostic.freshStage=value;};
        lifetime::ArrayHeader currentRegistry{};
        std::uintptr_t spawnedCache=0;
        const auto lookupSafe=[&](){return cfgprimary::LookupSafe(r_,c_.owner.module);};
        stage(1);if(!lookupSafe())return false;
        stage(2);if(!lifetime::ReadArrayHeader(r_,c_.owner.module+lifetime::ObjectArrayRva,currentRegistry)||
           !lifetime::RevalidateBatch(r_,c_.owner.module+lifetime::ObjectArrayRva,currentRegistry,c_.owner.identities))return false;
        stage(3);
        for(const auto& extra:c_.identities)for(const auto& main:c_.owner.identities)
            if(extra.object==main.object||extra.index==main.index)return false;
        stage(4);if(fault_||!Plain(c_.killer)||!Plain(c_.counselor)||!c_.owner.pickedResolutionVerified||!lifecycle_.Current(r_))return false;
        stage(5);if(c_.identities[0].object!=c_.gameState||c_.identities[1].object!=c_.pawn)return false;
        stage(6);if(!c_.identities[0].HasAssignedSerial()||!c_.identities[1].HasAssignedSerial()||
           c_.gameState==c_.pawn||c_.identities[0].index==c_.identities[1].index)return false;
        stage(7);if(!lifetime::RevalidateBatch(r_,c_.owner.module+lifetime::ObjectArrayRva,currentRegistry,c_.identities))return false;
        stage(8);if(!Same(c_.owner.world+WorldGameState,c_.gameState))return false;
        stage(9);if(!Same(c_.owner.mode+0x3C0,c_.gameState))return false;
        stage(10);if(!Exact(c_.gameState,d::SCGameState_OfflineBotsClass))return false;
        stage(11);if(!Exact(c_.owner.controller,d::SCPlayerController_OfflineBotsClass))return false;
        stage(12);if(!Exact(c_.owner.playerState,d::SCPlayerState_OfflineBotsClass))return false;
        stage(13);if(!Same(c_.owner.controller+ControllerPawn,c_.pawn))return false;
        stage(14);if(!Same(c_.owner.controller+ControllerAcknowledgedPawn,c_.pawn))return false;
        stage(15);if(!Same(c_.pawn+0x3A0,c_.owner.controller))return false;
        stage(16);if(!Same(c_.pawn+0x388,c_.owner.playerState))return false;
        stage(17);if(!SampleSame(c_.pawn+ObjectClass,c_.owner.selectedClass,diagnostic.pawnClass,1))return false;
        stage(18);if(!r_.Read(c_.owner.playerState+birthdisk::SpawnedCharacterClass,spawnedCache))return false;
        diagnostic.spawnedClass=spawnedCache;diagnostic.observedMask|=2;
        if(!SpawnedClassCacheAgrees(spawnedCache,c_.owner.selectedClass))return false;
        Packet counselor{},killer{};std::uintptr_t counselorBase=0,killerBase=0,defaults=0;
        std::uint32_t flags=0;unsigned char role=0;
        stage(19);if(!published_&&!SampleSame(c_.gameState+0x420,0,diagnostic.primaryActor,4))return false;
        stage(20);if(!r_.Read(c_.owner.playerState+0x394,flags))return false;
        diagnostic.flags=flags;diagnostic.observedMask|=8;if(flags&6)return false;
        stage(21);if(!r_.Read(c_.owner.playerState+0x110,role))return false;
        diagnostic.role=role;diagnostic.observedMask|=16;if(role!=3)return false;
        stage(22);if(!r_.Read(c_.owner.selectedClass+0xF8,defaults)||!ObjectPointer(defaults)||defaults==c_.pawn)return false;
        stage(23);if(!r_.Read(c_.owner.module+birthdisk::SCCounselorCharacterClass,counselorBase)||!Same(c_.owner.module+birthdisk::SCCounselorCharacterConstructor,counselorBase))return false;
        stage(24);if(!r_.Read(c_.owner.module+birthdisk::SCKillerCharacterClass,killerBase)||!Same(c_.owner.module+birthdisk::SCKillerCharacterConstructor,killerBase))return false;
        stage(25);if(!classchoice::MatchesPath(r_,c_.owner.module,c_.owner.selectedClass,counselorBase,c_.counselor.bytes))return false;
        stage(26);if(!classchoice::MatchesPath(r_,c_.owner.module,c_.killerClass,killerBase,c_.killer.bytes))return false;
        stage(27);if(!r_.Read(c_.counselorSource,counselor)||counselor.bytes!=c_.counselor.bytes)return false;
        stage(28);if(!r_.Read(c_.killerSource,killer)||killer.bytes!=c_.killer.bytes)return false;
        stage(29);if(!schemas_[0].Observe(r_,c_.owner.module,c_.gameState,u"CurrentKillerPlayerState",d::CurrentKillerPlayerState,false))return false;
        stage(30);if(!schemas_[1].Observe(r_,c_.owner.module,c_.gameState,u"KillerClass",d::KillerClass,true))return false;
        stage(31);if(!schemas_[2].Observe(r_,c_.owner.module,c_.owner.playerState,u"ActiveCharacterClass",d::ActiveCharacterClass,true))return false;
        stage(32);if(!Same(c_.owner.playerState+birthdisk::SpawnedCharacterClass,spawnedCache)||!lookupSafe())return false;
        stage(0);return true;
    }
    bool Read(Packet& active,Packet& klass,std::uintptr_t& owner){
        namespace d=intrometadatadisk;
        auto& diagnostic=Diagnostic();diagnostic.operation=1;diagnostic.step=1;diagnostic.packetMask=0;
        if(!Fresh())return false;
        diagnostic.step=2;if(!r_.Read(c_.owner.playerState+d::ActiveCharacterClass,active))return false;
        diagnostic.active=active.bytes;diagnostic.packetMask|=1;
        diagnostic.step=3;if(!r_.Read(c_.gameState+d::KillerClass,klass))return false;
        diagnostic.klass=klass.bytes;diagnostic.packetMask|=2;
        diagnostic.step=4;if(!r_.Read(c_.gameState+d::CurrentKillerPlayerState,owner))return false;
        diagnostic.step=5;return Fresh();
    }
    bool SetClass(const Packet& expected,const Packet& next){
        namespace d=intrometadatadisk;Packet now{};const auto at=c_.gameState+d::KillerClass;
        auto& diagnostic=Diagnostic();diagnostic.operation=2;diagnostic.step=1;
        if(!d::AssignValuesVerified)return false;
        if(!EmptyOrPlain(next)||!Fresh()||!r_.Read(at,now)||!SameValue(now,expected)||!Writable(at,40)||
           !Code(d::AssignValues,d::AssignValuesCode)||!Code(softdisk::PathCopy,softdisk::PathCopyCode)||
           !Code(softdisk::FStringAssign,softdisk::FStringAssignCode))return false;
        diagnostic.step=2;if(!startupretain::Invoke(c_.owner.module,startupretain::Call::AssignIntroClass,
            std::array<std::uintptr_t,4>{c_.gameState,c_.pawn,c_.killerClass,c_.owner.playerState},
            [&](){return AssignPlain(c_.owner.module+d::AssignValues,reinterpret_cast<Packet*>(at),&next);})){fault_=true;return false;}
        diagnostic.step=3;if(!Fresh()||!r_.Read(at,now))return false;
        diagnostic.klass=now.bytes;diagnostic.packetMask|=2;diagnostic.step=4;return SameValue(now,next);
    }
    bool SetActive(const Packet& expected,const Packet& next){
        Packet now{},owned{};const auto at=c_.owner.playerState+birthdisk::ActiveCharacterClass;
        auto& diagnostic=Diagnostic();diagnostic.operation=3;diagnostic.step=1;
        if(!Plain(next)||!Fresh()||!r_.Read(at,now)||!SamePath(now,expected)||!Writable(at,40)||
           !Code(birthdisk::CopyConstruct,birthdisk::CopyConstructCode)||!Code(birthdisk::ConsumeActive,birthdisk::ConsumeActiveCode)||
           !Code(birthdisk::PathCopy,birthdisk::PathCopyCode)||!Code(birthdisk::ActiveNotify,birthdisk::ActiveNotifyCode)||
           !Code(birthdisk::SettingsNotify,birthdisk::SettingsNotifyCode))return false;
        // Deep copy through CFG's constructor. Plain input guarantees no owned
        // allocation if revalidation refuses before consumption.
        const std::array<std::uintptr_t,4> inputs{c_.gameState,c_.pawn,c_.killerClass,c_.owner.playerState};
        diagnostic.step=2;if(!startupretain::Invoke(c_.owner.module,startupretain::Call::CopyPath,inputs,
            [&](){return promotion::detail::Copy(c_.owner.module+birthdisk::CopyConstruct,&owned,&next);})){fault_=true;return false;}
        diagnostic.step=3;if(!Plain(owned)||!Fresh()||!r_.Read(at,now)||!SamePath(now,expected))return false;
        diagnostic.step=4;if(!startupretain::Invoke(c_.owner.module,startupretain::Call::ConsumeActive,inputs,
            [&](){return promotion::detail::Two(c_.owner.module+birthdisk::ConsumeActive,c_.owner.playerState,&owned);})){fault_=true;return false;}
        owned={};diagnostic.step=5;if(!Fresh()||!r_.Read(at,now))return false;
        diagnostic.active=now.bytes;diagnostic.packetMask|=1;diagnostic.step=6;return SamePath(now,next);
    }
    bool Publish(std::uintptr_t expected,std::uintptr_t next){
        const auto at=c_.gameState+intrometadatadisk::CurrentKillerPlayerState;
        auto& diagnostic=Diagnostic();diagnostic.operation=4;diagnostic.step=1;
        diagnostic.publishAddress=at;diagnostic.publishExpected=expected;diagnostic.publishNext=next;diagnostic.publishStatus=0;
        if((next!=0&&next!=c_.owner.playerState)||!Fresh()||!Writable(at,8))return false;
        diagnostic.step=2;if(!Exchange(at,expected,next,diagnostic))return false;
        if(next==c_.owner.playerState)published_=true;
        diagnostic.step=3;return Fresh()&&Same(at,next);
    }
};
}
