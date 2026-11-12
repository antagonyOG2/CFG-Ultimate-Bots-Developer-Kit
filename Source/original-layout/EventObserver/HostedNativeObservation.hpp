#pragma once
#include "../RuntimeProbe/HostedAuthorityBindings.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/DispatchObservation.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
#include "../RuntimeProbe/EscapeBindings.generated.hpp"
#include <cstring>
namespace cfghosted::native {
// Existing disabled 3.4 fixtures keep their historical default. H1 passes the
// separately verified 3.5 binding set explicitly; no offline gate is relaxed.
struct HistoricalBindings {
    static constexpr std::array<std::array<std::uintptr_t,2>,4> Caches(){return {{
        {{disk::WorldReflection,disk::WorldConstructor}},{{disk::NetDriverReflection,disk::NetDriverConstructor}},
        {{disk::HuntReflection,disk::HuntConstructor}},
        {{cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor}}}};}
};
using Slot=cfgprobe::lifetime::SlotObservation;
using Table=cfgprobe::lifetime::ArrayHeader;
constexpr unsigned ReadLimit=1376; // leaves160 reads for a1536-read owner phase
// Supplied by an independent pinned-image/owner-callback admission, NOT F8's
// input worker. This adapter does not establish thread provenance or install a
// hosted hook. Never construct it by weakening the offline observer's gate.
struct Admission {
    std::uintptr_t module=0,engine=0,viewport=0,world=0;
    std::uint32_t ownerThread=0;
    bool imagePinned=false,requested=false;
};
struct Context {
    Admission admission{};
    Table table{};
    std::array<Slot,4> identities{}; // world, NetDriver, mode, GameState
    std::array<std::uintptr_t,4> classes{};
    cfgnames::Words match{};
    std::uintptr_t World()const{return identities[0].object;}
    std::uintptr_t Driver()const{return identities[1].object;}
    std::uintptr_t Mode()const{return identities[2].object;}
    std::uintptr_t State()const{return identities[3].object;}
};
struct Trace {unsigned stage=0;std::uintptr_t object=0,expected=0;};
template<class R,unsigned Limit=ReadLimit> class Budget {
    R& source_;unsigned used_=0;
public:
    explicit Budget(R& r):source_(r){}
    template<class T> bool Read(std::uintptr_t at,T& output){
        if(used_>=Limit)return false;++used_;return source_.Read(at,output);
    }
    unsigned Used()const{return used_;}
};
template<class R,class T> bool Same(R& r,std::uintptr_t at,const T& expected){T value{};return r.Read(at,value)&&value==expected;}
template<class R> bool Class(R& r,std::uintptr_t module,std::uintptr_t a,std::uintptr_t b,std::uintptr_t& out){
    std::uintptr_t first=0,second=0;
    if(!r.Read(module+a,first)||!r.Read(module+b,second)||!cfgprobe::MatchingClassCache(first,second))return false;
    out=first;return true;
}
template<class R> bool Typed(R& r,const Table& table,std::uintptr_t object,std::uintptr_t type,Slot& out){
    cfgprobe::Ancestry ancestry;Slot first{},again{};
    if(!cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(type)||
       !cfgprobe::lifetime::ReadSlot(r,table,object,first)||!first.HasAssignedSerial()||
       !cfgprobe::StableAncestry(r,object,ancestry)||
       !cfgprobe::lifetime::ReadSlot(r,table,object,again)||!(again==first))return false;
    out=first;return true;
}
// H2.50f: the same proof, but an UNASSIGNED weak serial (0) is accepted. At PreMatchIntro the
// host's LocalPlayer/PlayerStates have no serial yet (live: host lottery refusal 131 = Typed of
// the ILLLocalPlayer). Ancestry, stable ancestry and an identical slot (object, index, serial 0)
// on two reads are still required; never allocates a serial. Used only by the host lottery.
inline unsigned typedAllowUnassignedWhy=0; // H2.50g diagnostics: 1 ancestry read,2 type not in ancestry,3 slot read,4 unstable ancestry,5 slot changed
template<class R> bool TypedAllowUnassigned(R& r,const Table& table,std::uintptr_t object,std::uintptr_t type,Slot& out){
    cfgprobe::Ancestry ancestry;Slot first{},again{};typedAllowUnassignedWhy=1;
    if(!cfgprobe::CaptureAncestry(r,object,ancestry))return false;typedAllowUnassignedWhy=2;
    if(!ancestry.Contains(type))return false;typedAllowUnassignedWhy=3;
    if(!cfgprobe::lifetime::ReadSlot(r,table,object,first))return false;typedAllowUnassignedWhy=4;
    if(!cfgprobe::StableAncestry(r,object,ancestry))return false;typedAllowUnassignedWhy=5;
    if(!cfgprobe::lifetime::ReadSlot(r,table,object,again)||!(again==first))return false;typedAllowUnassignedWhy=0;
    out=first;return true;
}
template<class R,class Bindings=HistoricalBindings> bool Capture(R& source,const Admission& a,std::uint32_t currentThread,Context& output,unsigned* reads=nullptr,Trace* trace=nullptr,bool allowPreMatch=false){
    output={};Budget<R> r(source);Context c;c.admission=a;
    struct Finish{Budget<R>& r;unsigned* result;~Finish(){if(result)*result=r.Used();}}finish{r,reads};
    if(!a.imagePinned||!a.requested||!a.ownerThread||currentThread!=a.ownerThread||
       !cfgprobe::ObjectPointer(a.module)||!cfgprobe::ObjectPointer(a.engine)||
       !cfgprobe::ObjectPointer(a.viewport)||!cfgprobe::ObjectPointer(a.world)||
       !cfgprobe::lifetime::Extent(a.module,cfgprobe::ExecutableImageBytes))return false;
    const auto mark=[&](unsigned stage,std::uintptr_t object=0,std::uintptr_t expected=0){if(trace)*trace={stage,object,expected};};
    mark(1);
    constexpr auto caches=Bindings::Caches();
    std::array<std::uintptr_t,4> objects{{a.world,0,0,0}};
    mark(2);
    if(!Same(r,a.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t(1))||
       !Same(r,a.module+cfgprobe::dispatch::ThreadIdRva,a.ownerThread)||
       !cfgprobe::lifetime::ReadArrayHeader(r,a.module+cfgprobe::lifetime::ObjectArrayRva,c.table)||
       !r.Read(a.world+cfgprobe::WorldNetDriver,objects[1])||
       !r.Read(a.world+cfgprobe::WorldGameMode,objects[2])||
       !r.Read(a.world+cfgprobe::WorldGameState,objects[3]))return false;
    for(unsigned i=0;i<4;++i){
        for(unsigned j=0;j<i;++j)if(objects[i]==objects[j])return false;
        mark(10+i,objects[i]);
        if(!Class(r,a.module,caches[i][0],caches[i][1],c.classes[i]))return false;
        mark(20+i,objects[i],c.classes[i]);
        if(!Typed(r,c.table,objects[i],c.classes[i],c.identities[i]))return false;
    }
    mark(30,c.Mode());
    if(!Same(r,c.Mode()+disk::ActorRole,disk::AuthorityRole)||
       !r.Read(c.Mode()+cfgescape::disk::MatchState,c.match))return false;
    cfgnames::Text text;cfgnames::Observation<Budget<R>> names(r,a.module);
    mark(31,c.Mode());
    // H2.50h: the host lottery runs during PreMatchIntro (live: refusal 131 = this stage), so only that
    // opt-in caller also accepts the two pre-match states; every other caller still requires InProgress.
    if(!names.Decode(c.match,text)||!(text.View()==u"InProgress"||(allowPreMatch&&(text.View()==u"PreMatchIntro"||text.View()==u"WaitingPreMatch"))))return false;
    mark(40);
    for(unsigned i=0;i<4;++i){Slot again;
        if(!Same(r,a.module+caches[i][0],c.classes[i])||!Same(r,a.module+caches[i][1],c.classes[i])||
           !Typed(r,c.table,objects[i],c.classes[i],again)||!(again==c.identities[i]))return false;
    }
    if(!Same(r,a.module+cfgprobe::EngineGlobalCandidate,a.engine)||
       !Same(r,a.engine+cfgprobe::EngineGameViewport,a.viewport)||!Same(r,a.viewport+cfgprobe::ViewportWorld,a.world)||
       !Same(r,a.world+cfgprobe::WorldNetDriver,c.Driver())||!Same(r,a.world+cfgprobe::WorldGameMode,c.Mode())||
       !Same(r,a.world+cfgprobe::WorldGameState,c.State())||!Same(r,c.Mode()+disk::ActorRole,disk::AuthorityRole)||
       !Same(r,c.Mode()+cfgescape::disk::MatchState,c.match)||
       !Same(r,a.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t(1))||
       !Same(r,a.module+cfgprobe::dispatch::ThreadIdRva,a.ownerThread)||
       !cfgprobe::lifetime::RevalidateBatch(r,a.module+cfgprobe::lifetime::ObjectArrayRva,c.table,c.identities))return false;
    mark(0);output=c;return true;
}
inline bool SameSession(const Context& a,const Context& b){
    return a.admission.module==b.admission.module&&a.admission.engine==b.admission.engine&&
        a.admission.viewport==b.admission.viewport&&a.admission.ownerThread==b.admission.ownerThread&&
        a.identities==b.identities&&a.classes==b.classes;
}
// Sampled identity is not a GC pin, replication readiness or call permission.
}
