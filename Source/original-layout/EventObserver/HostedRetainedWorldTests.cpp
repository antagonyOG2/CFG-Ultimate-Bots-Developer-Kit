#include "HostedRetainedWorld.hpp"
#include "HostedAdoption.hpp"
#include "KnifeOwnerObservation.hpp"
#include "PrimarySpawnObservation.hpp"
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <array>
namespace {
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    std::set<std::uintptr_t> log;
    unsigned reads=0,cap=96;
    template<class T> void Put(std::uintptr_t address,const T& value){
        static_assert(sizeof(T)<=16,"Fixture cell bound");
        auto& bytes=memory[address];bytes={};std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& output){
        if(reads>=cap)return false;++reads;log.insert(address);
        const auto found=memory.find(address);if(found==memory.end())return false;
        T value{};std::memcpy(&value,found->second.data(),sizeof(value));output=value;return true;
    }
};
int failures=0,checks=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(false)
namespace d=cfghosted::disk35;
constexpr std::uintptr_t Driver=0x20100000,ModeClass=0x20200000,HuntClass=0x20300000;
struct World {
    cfgobserver::Plan p;Reader r;cfgprimary::HostedBinding binding;
    World(bool hosted){
        using namespace cfgprobe;
        p.bytes=sizeof(p);p.threadId=77;p.module=0x10000000;
        p.engine=0x20000000;p.viewport=0x20010000;p.world=0x20020000;p.mode=0x20030000;
        p.controller=0x20040000;p.pawn=0x20050000;p.player=0x20060000;
        p.gameInstance=0x20070000;p.localPlayerData=0x20080000;
        p.objectArray={0x30000000,64};
        p.dispatch={1,77,0x20090000,p.module+dispatch::NativeActorDispatcherRva};
        const std::array<std::uintptr_t,4> objects{p.world,p.mode,p.controller,p.pawn};
        for(std::size_t i=0;i<4;++i)p.objects[i]={objects[i],static_cast<std::int32_t>(i),static_cast<std::int32_t>(100+i)};
        r.Put(p.module+EngineGlobalCandidate,p.engine);r.Put(p.engine+EngineGameViewport,p.viewport);
        r.Put(p.viewport+ViewportWorld,p.world);r.Put(p.world+WorldGameMode,p.mode);r.Put(p.world+WorldGameInstance,p.gameInstance);
        r.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,p.objectArray.data);
        r.Put(p.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,p.objectArray.count);
        const auto slot=[&](std::uintptr_t object,std::int32_t index,std::int32_t serial){
            r.Put(object+lifetime::ObjectIndexOffset,index);
            r.Put(p.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes,object);
            r.Put(p.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes+lifetime::ItemSerialOffset,serial);};
        for(const auto& s:p.objects)slot(s.object,s.index,s.serial);
        slot(Driver,4,104);
        r.Put(p.module+dispatch::ThreadFlagRva,p.dispatch.flag);r.Put(p.module+dispatch::ThreadIdRva,p.dispatch.threadId);
        r.Put(p.mode+ObjectClass,ModeClass);r.Put(p.module+d::HuntReflection,HuntClass);r.Put(p.module+d::HuntConstructor,HuntClass);
        r.Put(p.controller+d::ActorRole,std::uint8_t(3));
        r.Put(Driver+d::DriverWorld,p.world);r.Put(Driver+d::ServerConnection,std::uintptr_t{});
        r.Put(p.world+WorldNetDriver,hosted?Driver:std::uintptr_t{});
        r.Put(p.pawn,p.dispatch.vtable);r.Put(p.dispatch.vtable+dispatch::EventSlotBytes,p.dispatch.target);
        if(hosted){
            binding.active=true;binding.plan=p;binding.netDriver=Driver;binding.driver={Driver,4,104};
            binding.modeClass=ModeClass;binding.huntClass=HuntClass;
        }
    }
};
}
int main(){
    using namespace cfgprimary;using namespace cfgprobe;
    cfgobserver::Plan out{};
    // ---- Offline plans: byte-identical RetainedOfflineWorld, never with a NetDriver ----
    {World off(false);const HostedBinding none{};
     CHECK(cfgprimary::RetainedOfflineWorld(off.r,off.p,out));
     off.r.reads=0;CHECK(RetainedWorld(off.r,off.p,out,none)&&off.r.reads>0);
     CHECK(none.KindOf(off.p)==PlanKind::Offline);
     auto net=off.r;net.reads=0;net.Put(off.p.world+WorldNetDriver,Driver);
     CHECK(!cfgprimary::RetainedOfflineWorld(net,off.p,out));net.reads=0;
     CHECK(!RetainedWorld(net,off.p,out,none));                    // offline plan + NetDriver: never
     CurrentHostedBinding().Retire();net.reads=0;CHECK(!RetainedWorld(net,off.p,out));
     // An active hosted binding for ANOTHER plan never changes the offline decision.
     World host(true);host.binding.plan.world=0x2FF00000;host.binding.plan.objects[0].object=0x2FF00000;net.reads=0; // a binding for a DIFFERENT hosted world
     CHECK(host.binding.KindOf(off.p)==PlanKind::Offline&&!RetainedWorld(net,off.p,out,host.binding));
     auto clean=off.r;clean.reads=0;CHECK(RetainedWorld(clean,off.p,out,host.binding));}
    // ---- Hosted plans: pass only with their exact NetDriver ----
    {World host(true);auto& r=host.r;auto& p=host.p;auto& b=host.binding;
     CHECK(b.KindOf(p)==PlanKind::Hosted);
     CHECK(!cfgprimary::RetainedOfflineWorld(r,p,out));                       // offline function untouched: refuses a NetDriver
     r.reads=0;r.log.clear();CHECK(RetainedWorld(r,p,out,b)&&r.reads<=40);const auto successReads=r.reads;const auto observed=r.log;
     CHECK(out.world==p.world&&out.pawn==p.pawn&&out.controller==p.controller);
     // Global binding path used at the production call sites.
     CurrentHostedBinding()=b;{auto g=r;g.reads=0;CHECK(RetainedWorld(g,p,out));}
     CurrentHostedBinding().Retire();{auto g=r;g.reads=0;CHECK(!RetainedWorld(g,p,out));}
     // Missing/zeroed NetDriver: never passes, and never falls back to the offline function.
     auto zero=r;zero.reads=0;zero.Put(p.world+WorldNetDriver,std::uintptr_t{});
     CHECK(cfgprimary::RetainedOfflineWorld(zero,p,out));zero.reads=0;CHECK(!RetainedWorld(zero,p,out,b));
     // Every field the hosted proof reads must be required.
     for(const auto address:observed){auto absent=r;absent.reads=0;absent.memory.erase(address);
        CHECK(!RetainedWorld(absent,p,out,b));}
     auto capped=r;capped.reads=0;capped.cap=successReads-1;CHECK(!RetainedWorld(capped,p,out,b));
     struct Mutation{const char* name;std::uintptr_t address;std::uintptr_t value;std::size_t size;};
     auto mutate=[&](auto&& edit){auto m=r;m.reads=0;edit(m);return RetainedWorld(m,p,out,b);};
     CHECK(!mutate([&](Reader& m){m.Put(p.world+WorldNetDriver,std::uintptr_t{0x50000000});}));                 // different driver
     CHECK(!mutate([&](Reader& m){m.Put(Driver+d::DriverWorld,std::uintptr_t{0x50000000});}));                 // driver for another world
     CHECK(!mutate([&](Reader& m){m.Put(Driver+d::ServerConnection,std::uintptr_t{0x50000000});}));            // client, not listen host
     CHECK(!mutate([&](Reader& m){m.Put(p.mode+ObjectClass,std::uintptr_t{0x50000000});}));                    // mode class changed
     CHECK(!mutate([&](Reader& m){m.Put(p.module+d::HuntReflection,std::uintptr_t{0x50000000});}));            // Hunt cache refreshed/changed
     CHECK(!mutate([&](Reader& m){m.Put(p.controller+d::ActorRole,std::uint8_t(1));}));                         // no local authority
     CHECK(!mutate([&](Reader& m){m.Put(p.module+dispatch::ThreadIdRva,std::uint32_t{78});}));                  // wrong thread
     CHECK(!mutate([&](Reader& m){m.Put(p.objectArray.data+4*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});}));  // driver reused
     CHECK(!mutate([&](Reader& m){m.Put(p.objectArray.data+lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});}));    // mode reused
     CHECK(!mutate([&](Reader& m){m.Put(p.viewport+ViewportWorld,std::uintptr_t{0x50000000});}));             // world replaced
     CHECK(!mutate([&](Reader& m){m.Put(p.world+WorldGameMode,std::uintptr_t{0x50000000});}));
     // Binding must exactly match: a changed pawn/world plan is not the hosted plan -> offline path -> refuses (NetDriver present).
     auto other=p;other.pawn=0x20055000;other.objects[3].object=0x20055000;
     CHECK(b.KindOf(other)==PlanKind::Offline&&!RetainedWorld(r,other,out,b));
     auto badBinding=b;badBinding.driver.serial=0;{auto g=r;g.reads=0;CHECK(!RetainedWorld(g,p,out,badBinding));}
     badBinding=b;badBinding.netDriver=0;{auto g=r;g.reads=0;CHECK(!RetainedWorld(g,p,out,badBinding));}
     badBinding=b;badBinding.active=false;{auto g=r;g.reads=0;CHECK(!RetainedWorld(g,p,out,badBinding));}   // inactive => offline => NetDriver refused
    }
    // ---- Adoption receipt replaces ONLY the PawnAIClass check ----
    {World host(true);auto& r=host.r;auto& p=host.p;
     constexpr std::uintptr_t Pawn=0x21000000,Ctl=0x21100000,Path=0x21200000,State=0x21300000;
     constexpr std::uintptr_t KillerBase=0x22000000,PawnClass=0x22100000,AiClass=0x22200000,PathClass=0x22300000,Other=0x22400000,StateClass=0x22500000;
     const auto slot=[&](std::uintptr_t object,std::int32_t index,std::int32_t serial){
        r.Put(object+lifetime::ObjectIndexOffset,index);
        r.Put(p.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes,object);
        r.Put(p.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes+lifetime::ItemSerialOffset,serial);
        r.Put(p.objectArray.data+static_cast<std::uintptr_t>(index)*lifetime::ItemBytes+8,std::uint32_t{0});};
     for(const auto& s:p.objects)r.Put(p.objectArray.data+static_cast<std::uintptr_t>(s.index)*lifetime::ItemBytes+8,std::uint32_t{0});
     slot(Pawn,8,201);slot(Ctl,9,202);slot(Path,10,0);slot(State,11,0);
     r.Put(PawnClass+ClassSuper,KillerBase);r.Put(KillerBase+ClassSuper,std::uintptr_t{});r.Put(AiClass+ClassSuper,std::uintptr_t{});
     r.Put(PathClass+ClassSuper,std::uintptr_t{});r.Put(Other+ClassSuper,std::uintptr_t{});r.Put(StateClass+ClassSuper,std::uintptr_t{});
     r.Put(Pawn+ObjectClass,PawnClass);r.Put(Ctl+ObjectClass,AiClass);r.Put(Path+ObjectClass,PathClass);r.Put(State+ObjectClass,StateClass);
     r.Put(Pawn+cfgspawn::PawnAIClass,Other);                                   // human Jason: AIControllerClass NOT the killer AI
     r.Put(Pawn+0x3A0,Ctl);r.Put(Ctl+0x370,Pawn);r.Put(Ctl+0x388,State);r.Put(Pawn+0x388,State);r.Put(State+0x88,Ctl);
     r.Put(Ctl+0x410,Path);r.Put(Path+0x20,Ctl);r.Put(Path+cfgjason::disk::ComponentOwner,Ctl);
     cfgjason::NewActorEvidence e;e.pawn={Pawn,8,201};e.killerController={Ctl,9,202};e.path={Path,10,0};
     cfghosted::adoption::Receipt receipt;receipt.valid=true;receipt.pawn=e.pawn;receipt.controller=e.killerController;
     receipt.controllerState=State;receipt.displacedState=0x21500000;receipt.world=p.world;receipt.mode=p.mode;
     receipt.possessReturned=true;receipt.botsBefore=0;receipt.botsAfter=1;receipt.rosterBefore=2;receipt.rosterAfter=3;
     r.cap=6144;
     // Reproduce HostedPrepareAdoption's actual pre-stage-5 reader prefix:
     // retained hosted world, counselor cache, the eight hosted runtime class
     // pairs/slots and recheck, four Jason classes, then the resolved template.
     const std::uintptr_t Helper=0x23000000,BaseAI=0x23001000,KillerAIClass=0x23002000,Tree=0x23004000,
         Brain=0x23005000,PlayerStateClass=0x23006000,GameStateClass=0x23007000,HelperDefault=0x23008000;
     const std::array<std::uintptr_t,8> runtimeClasses{Helper,BaseAI,KillerAIClass,PathClass,Tree,Brain,PlayerStateClass,GameStateClass};
     const auto extraSlot=[&](std::uintptr_t object,unsigned index,int serial){r.Put(object+lifetime::ObjectIndexOffset,static_cast<int>(index));
         r.Put(p.objectArray.data+24ULL*index,object);r.Put(p.objectArray.data+24ULL*index+lifetime::ItemSerialOffset,serial);
         r.Put(p.objectArray.data+24ULL*index+8,std::uint32_t{});};
     for(unsigned i=0;i<runtimeClasses.size();++i){const auto& cb=cfgspawn::Classes[i];
         r.Put(p.module+cb.reflection,runtimeClasses[i]);r.Put(p.module+cb.constructor,runtimeClasses[i]);extraSlot(runtimeClasses[i],16+i,500+i);
         r.Put(runtimeClasses[i]+ClassSuper,std::uintptr_t{});}
     extraSlot(HelperDefault,24,524);r.Put(Helper+cfgspawn::ClassDefaultObject,HelperDefault);
     r.Put(HelperDefault+ObjectClass,Helper);
     const std::array<std::uintptr_t,4> jasonClasses{PawnClass,AiClass,PathClass,StateClass};
     for(unsigned i=0;i<jasonClasses.size();++i){const auto& cb=cfgjason::disk::Classes[i];
         r.Put(p.module+cb.reflection,jasonClasses[i]);r.Put(p.module+cb.constructor,jasonClasses[i]);extraSlot(jasonClasses[i],25+i,525+i);}
     extraSlot(KillerBase,29,529);extraSlot(Other,30,530);extraSlot(0x22600000,31,531);
     r.Put(PawnClass+ClassSuper,KillerBase);r.Put(KillerBase+ClassSuper,std::uintptr_t{});
     r.Put(AiClass+ClassSuper,KillerAIClass);r.Put(KillerAIClass+ClassSuper,std::uintptr_t{});
     r.Put(PathClass+ClassSuper,std::uintptr_t{});r.Put(StateClass+ClassSuper,std::uintptr_t{});
     r.Put(Other+ClassSuper,std::uintptr_t{});r.Put(0x22600000+ClassSuper,std::uintptr_t{});
     constexpr std::uintptr_t Defaults=0x22600000;
     extraSlot(Defaults,32,532);r.Put(PawnClass+cfgspawn::ClassDefaultObject,Defaults);r.Put(Defaults+ObjectClass,PawnClass);
     r.Put(Defaults+cfgspawn::PawnAIClass,BaseAI);r.Put(Defaults+cfgspawn::CharacterTree,std::uintptr_t{});
     r.Put(p.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,0x23009000);
     r.Put(0x23009000+ClassSuper,std::uintptr_t{});extraSlot(0x23009000,33,533);
     cfgspawn::ResourceReport resources{};cfgspawn::TemplateObservation resolved{};cfgspawn::ResourceProgress progress{};
     r.reads=0;CHECK(RetainedWorld(r,p,out,host.binding));
     std::uintptr_t counselor=0;CHECK(r.Read(p.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselor)&&counselor);
     CHECK(cfgspawn::CaptureHostedRuntimeResources(r,p.module,p.objectArray,resources,&progress)&&resources.classMask==255);
     for(unsigned i=0;i<cfgjason::disk::Classes.size();++i){std::uintptr_t reflection=0,constructor=0;cfgprobe::lifetime::SlotObservation cacheSlot{};
         CHECK(r.Read(p.module+cfgjason::disk::Classes[i].reflection,reflection)&&r.Read(p.module+cfgjason::disk::Classes[i].constructor,constructor)&&
             reflection&&reflection==constructor&&cfgspawn::ObjectSlot(r,p.objectArray,reflection,cacheSlot));}
     std::uintptr_t observedClass=0;CHECK(r.Read(Pawn+ObjectClass,observedClass)&&
         cfgprimary::ObserveResolvedTemplate(r,p.objectArray,observedClass,KillerBase,BaseAI,resolved));
     std::uintptr_t observedPath=0;cfgprobe::lifetime::SlotObservation pathSlot{};
     CHECK(r.Read(Ctl+cfgjason::disk::PathFollowing,observedPath)&&cfgspawn::ObjectSlot(r,p.objectArray,observedPath,pathSlot));
     const unsigned prefixReads=r.reads;
     const auto actualStage5=cfgjason::LinkedAdoptedKillerProof(r,p.objectArray,e,receipt,p,p.pawn,resolved.pawnClass,
         resolved.defaults,KillerBase,KillerAIClass,PathClass,p.controller);
     CHECK(actualStage5.firstFailure==0&&actualStage5.reads<1536&&r.reads<r.cap);
     // Model a native birth that relocates/grows UObject storage and inserts
     // the new path beyond the saved count without changing old identities.
     auto grown=r;const auto movedData=std::uintptr_t{0x32000000};
     for(std::int32_t i=0;i<p.objectArray.count;++i)for(unsigned field=0;field<cfgprobe::lifetime::ItemBytes;field+=8){
         const auto oldAt=p.objectArray.data+static_cast<std::uintptr_t>(i)*cfgprobe::lifetime::ItemBytes+field;
         const auto found=r.memory.find(oldAt);if(found!=r.memory.end())grown.memory[movedData+static_cast<std::uintptr_t>(i)*cfgprobe::lifetime::ItemBytes+field]=found->second;
     }
     grown.Put(p.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,movedData);
     grown.Put(p.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,std::int32_t{128});
     grown.Put(Path+cfgprobe::lifetime::ObjectIndexOffset,p.objectArray.count);
     grown.Put(movedData+static_cast<std::uintptr_t>(p.objectArray.count)*cfgprobe::lifetime::ItemBytes,Path);
     grown.Put(movedData+static_cast<std::uintptr_t>(p.objectArray.count)*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{0});
     grown.Put(movedData+static_cast<std::uintptr_t>(p.objectArray.count)*cfgprobe::lifetime::ItemBytes+
         cfgprobe::lifetime::ItemSerialOffset,std::int32_t{0});
     cfgobserver::Plan refreshed{};grown.reads=0;
     const bool refreshOkay=cfgprimary::RetainedWorld(grown,p,refreshed,host.binding);
     CHECK(refreshOkay&&refreshed.objectArray.data==movedData&&refreshed.objectArray.count==128&&refreshed.objects==p.objects);
     cfgprobe::lifetime::SlotObservation oldPath{},freshPath{};
     const bool oldRejects=!cfgspawn::ObjectSlot(grown,p.objectArray,Path,oldPath);
     const bool freshAccepts=cfgspawn::ObjectSlot(grown,refreshed.objectArray,Path,freshPath);
     CHECK(oldRejects&&freshAccepts&&freshPath.index==p.objectArray.count&&freshPath.serial==0);
     auto grownEvidence=e;grownEvidence.path=freshPath;
     const auto grownStage5=cfgjason::LinkedAdoptedKillerProof(grown,refreshed.objectArray,grownEvidence,receipt,refreshed,
         refreshed.pawn,resolved.pawnClass,resolved.defaults,KillerBase,KillerAIClass,PathClass,refreshed.controller);
     std::printf("H225 grown registry refresh=%u data=%llX count=%d oldReject=%u freshAccept=%u index=%d serial=%d stage5clause=%u reads=%u\n",
         refreshOkay,static_cast<unsigned long long>(refreshed.objectArray.data),refreshed.objectArray.count,oldRejects,freshAccepts,
         freshPath.index,freshPath.serial,grownStage5.firstFailure,grownStage5.reads);
     CHECK(grownStage5.firstFailure==0&&grownStage5.reads<1536);
     std::printf("H219 stage5 reproduction: stage1-4 production-prefix=%u reads, stage5=%u, cap=%u, clause=%u; realistic class/template/link slots.\n",
         prefixReads,actualStage5.reads,r.cap,actualStage5.firstFailure);
     auto brokenPathType=r;brokenPathType.reads=prefixReads;brokenPathType.Put(Path+ObjectClass,Other);
     CHECK(cfgjason::LinkedAdoptedKillerProof(brokenPathType,p.objectArray,e,receipt,p,p.pawn,resolved.pawnClass,
         resolved.defaults,KillerBase,KillerAIClass,PathClass,p.controller).firstFailure==6);
     auto brokenPathSlot=r;brokenPathSlot.reads=prefixReads;brokenPathSlot.Put(p.objectArray.data+10ULL*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});
     CHECK(cfgjason::LinkedAdoptedKillerProof(brokenPathSlot,p.objectArray,e,receipt,p,p.pawn,resolved.pawnClass,
         resolved.defaults,KillerBase,KillerAIClass,PathClass,p.controller).firstFailure==4);
     r.reads=0;
     const auto& a=p.objectArray;auto probe=[&](Reader& x,const cfghosted::adoption::Receipt& rc,const cfgjason::NewActorEvidence& ev){
        x.reads=0;return cfgjason::LinkedAdoptedKiller(x,a,ev,rc,p,p.pawn,PawnClass,0x22500000,KillerBase,AiClass,PathClass,p.controller);};
     CHECK(receipt.Complete());
     // Stock LinkedKiller refuses the adopted human Jason (PawnAIClass != killer AI); the receipt branch accepts it.
     r.reads=0;CHECK(!cfgjason::LinkedKiller(r,a,e,p,p.pawn,PawnClass,0x22500000,KillerBase,AiClass,PathClass,p.controller));
     CHECK(probe(r,receipt,e));
     // Compose the same hosted retained-world/adoption, Jason, and fresh knife
     // owner-link clauses in one measured reader, including adopted state slot
     // and exact runtime classes. This is the first-owner 1536 cap boundary.
     cfgprobe::lifetime::SlotObservation stateSlot{};std::uintptr_t stateType=0;
     const std::array<std::uintptr_t,3> ownerClasses{PawnClass,AiClass,PathClass};
     CurrentHostedBinding()=host.binding;r.cap=1536;r.reads=0;
     const bool currentBefore=RetainedWorld(r,p,out)&&cfghosted::adoption::Current(r,a,receipt,p);
     const bool adoptedJason=currentBefore&&cfgjason::LinkedAdoptedKiller(r,a,e,receipt,p,p.pawn,PawnClass,0x22500000,
         KillerBase,AiClass,PathClass,p.controller);
     const bool ownerState=adoptedJason&&cfgspawn::ObjectSlot(r,a,State,stateSlot)&&r.Read(State+ObjectClass,stateType)&&stateType==StateClass;
     const bool ownerLinks=ownerState&&cfgknife::FreshOwnerLinks(r,a,e,ownerClasses,stateSlot,StateClass);
     const bool currentAfter=ownerLinks&&RetainedWorld(r,p,out)&&cfghosted::adoption::Current(r,a,receipt,p)&&RetainedWorld(r,p,out);
     if(!(currentBefore&&adoptedJason&&ownerState&&ownerLinks&&currentAfter))
         std::printf("Hosted knife owner member clauses context=%d Jason=%d state=%d links=%d final=%d reads=%u/1536\n",
             currentBefore,adoptedJason,ownerState,ownerLinks,currentAfter,r.reads);
     CHECK(currentBefore&&adoptedJason&&ownerState&&ownerLinks&&currentAfter);
     std::printf("Hosted knife owner composition: context + LinkedAdoptedKiller + state/class slot + FreshOwnerLinks + final reproof = %u/1536 reads.\n",r.reads);
     CHECK(r.reads<1536);
     CurrentHostedBinding().Retire();
     auto writes=r;CHECK(probe(writes,receipt,e));
     // Never "neither": invalid / incomplete / foreign receipts refuse.
     auto bad=receipt;bad.valid=false;CHECK(!probe(r,bad,e));
     bad=receipt;bad.possessReturned=false;CHECK(!probe(r,bad,e));
     bad=receipt;bad.botsAfter=0;CHECK(!probe(r,bad,e));                       // NumBots delta must be exactly 1
     bad=receipt;bad.rosterAfter=2;CHECK(!probe(r,bad,e));
     bad=receipt;bad.controllerState=0x21600000;CHECK(!probe(r,bad,e));       // AI state not owned by the controller
     bad=receipt;bad.world=0x1;CHECK(!probe(r,bad,e));
     bad=receipt;bad.pawn.serial=202;CHECK(!probe(r,bad,e));                   // receipt for a different pawn slot
     auto other=e;other.killerController={0x21700000,12,300};CHECK(!probe(r,receipt,other)); // receipt must match THIS controller
     // Receipt never rescues a broken link.
     for(const auto& mutation:std::array<std::pair<std::uintptr_t,std::uintptr_t>,6>{{{Pawn+0x3A0,0x1},{Ctl+0x370,0x1},{Ctl+0x388,0x1},
         {Pawn+0x388,0x1},{State+0x88,0x1},{Ctl+0x410,0x1}}}){
        auto m=r;m.Put(mutation.first,mutation.second);CHECK(!probe(m,receipt,e));}
     {auto m=r;m.Put(Ctl+ObjectClass,Other);CHECK(!probe(m,receipt,e));}       // controller not the killer AI class
     {auto m=r;m.Put(p.objectArray.data+8*lifetime::ItemBytes+lifetime::ItemSerialOffset,std::int32_t{999});CHECK(!probe(m,receipt,e));}
     // The receipt branch cannot admit the host's own pawn/controller as the Jason.
     bad=receipt;bad.pawn.object=p.pawn;CHECK(!probe(r,bad,e));
    }
    std::printf("H1 hosted retained-world dispatcher/adoption receipt: %d checks, %d failures. Offline plans never pass with a NetDriver; hosted plans never pass without theirs. No native calls.\n",checks,failures);
    return failures?1:0;
}
