// Included production birth/registry methods; native allocation/possession are
// mocked, but retained-world, serial/header, helper and CAS proofs are real.
#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "../Core/AdditionalJasonPolicy.hpp"
#include "../Core/AdditionalJasonJournal.hpp"
#include "../Core/HelperSpawnPolicy.hpp"
#include "../Core/VehiclePursuitPolicy.hpp"
#include "../RuntimeProbe/AdditionalLifecycleBindings.generated.hpp"
#include "../RuntimeProbe/AdditionalJasonBindings.generated.hpp"
#include "NativeJasonCalls.hpp"
#include "ObjectiveHudWrite.hpp"
#include "AdditionalRegistryWrite.hpp"
#include "OwnedCounselorTemplate.hpp"
#include <vector>
namespace cfgclosedtrace {template<class A,class B>void Mark(const char*,A,B){}}
struct BirthFixture {
    struct Reader:SpectatorFixture::Reader {
        std::vector<std::uintptr_t> pages;
        bool Real(std::uintptr_t at,std::size_t bytes)const{for(auto page:pages)if(at>=page&&bytes<=0x4000&&at-page<=0x4000-bytes)return true;return false;}
        template<class T>void Put(std::uintptr_t at,const T& value){if(Real(at,sizeof(T)))std::memcpy(reinterpret_cast<void*>(at),&value,sizeof(T));else Fixture::Put(at,value);}
        template<class T>bool Read(std::uintptr_t at,T& value){if(!Real(at,sizeof(T)))return Fixture::Read(at,value);if(reads>=cap)return false;++reads;std::memcpy(&value,reinterpret_cast<void*>(at),sizeof(T));return true;}
        std::uintptr_t RealObject(std::uintptr_t type){
            const auto at=reinterpret_cast<std::uintptr_t>(VirtualAlloc(nullptr,0x4000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
            CHECK(at!=0);pages.push_back(at);Put(at+0xC,nextSlot);Put(at+0x10,type);
            const auto cell=objects.data+24ULL*static_cast<unsigned>(nextSlot++);Put(cell,at);Put(cell+8,std::uint32_t{});Put(cell+16,std::int32_t{1});return at;
        }
        ~Reader(){for(auto page:pages)VirtualFree(reinterpret_cast<void*>(page),0,MEM_RELEASE);}
    }r_;
    cfgobserver::Plan plan{},context{};bool originalActorDispatcher=true;
    cfgjason::NewActorEvidence bundleJason;
    struct Assets {
        cfgspawn::ResourceReport resources{};cfgprimary::Report primary{};
        std::uintptr_t gameState=0,killerBase=0;
        std::uintptr_t conversionFunction=0,kismetClass=0,kismetDefault=0;
        cfgprobe::lifetime::SlotObservation conversionSlot{};
        std::array<std::uintptr_t,4> classes{},functions{};
        std::array<cfgprobe::lifetime::SlotObservation,4> functionSlots{};
        std::array<cfgprobe::lifetime::SlotObservation,7> slots{};
        cfgprobe::lifetime::SlotObservation helperClassSlot{};
    }spawnAssets;
    struct Extra {
        cfgadditional::Policy policy;cfgadditionaljournal::Journal journal;
        cfgjason::NewActorEvidence actor;cfgspawn::TemplateObservation selected,sameClassFallback{};unsigned sameClassFallbackIndex=~0u;bool hostedOwnClass=false;
        cfgbundle::HelperStartup placement;cfgspawn::Vec3 position{};
        std::uintptr_t rawPawn=0,rawController=0,rawPath=0,primaryState=0;bool hostedRandom=false,hostedGateWait=false;unsigned hostedGateRetries=0;
        unsigned selectedIndex=0,selection=0,selectionRefusal=0,selectionConversionFailures=0,selectionTemplateFailures=0;
        unsigned driveRefusals=0,rollback=0;bool allocationAttempted=false,lastTurn=false,genericDestroyAttempted=false;
    }bundleAdditional;
    unsigned calls_=0,allocations=0,projectionCalls=0,projectionFailures=0;
    bool relocate=false,nullSpawn=false,foreignNull=false,nullState=false,nativeReturned=true;
    std::uintptr_t destroyFunction=0,actorBase=0;std::vector<std::uintptr_t> destroyed;
    bool mutatePrimaryState=false,oldHeaderRejected=false;
    unsigned Reads()const{return r_.reads;}
    void Serial(std::uintptr_t object){int index=-1;CHECK(r_.Read(object+0xC,index)&&index>=0&&index<r_.nextSlot);r_.Put(r_.objects.data+24ULL*static_cast<unsigned>(index)+16,std::int32_t{1});}
    void PublishHeader(){r_.objects.count=r_.nextSlot;r_.Put(plan.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,r_.objects.data);r_.Put(plan.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,r_.objects.count);}
    void Fresh(){r_.cap=100000;PublishHeader();CHECK(AdditionalWorld());r_.reads=0;r_.cap=1536;calls_=0;}
    void Relocate(){
        if(!relocate)return;const auto old=r_.objects.data,next=std::uintptr_t{0xC00000};
        for(int i=0;i<r_.nextSlot;++i){std::array<unsigned char,24> bytes{};for(unsigned j=0;j<24;++j){const auto it=r_.memory.find(old+24ULL*static_cast<unsigned>(i)+j);if(it!=r_.memory.end())bytes[j]=it->second;}r_.Put(next+24ULL*static_cast<unsigned>(i),bytes);}
        r_.objects.data=next;PublishHeader();
    }
    bool Context(){return AdditionalWorld()&&cfgprimary::RegistryMatches(r_,context.objectArray,spawnAssets.primary.stateSlot,bundleJason.pawn.object,plan.controller,spawnAssets.resources.classes[cfgspawn::PlayerState]);}
    bool Jason(){return cfgjason::SameSlot(r_,context.objectArray,bundleJason.pawn);}
    bool HelperContext(){return Context();}bool Known(unsigned,std::uintptr_t& pawn){pawn=plan.pawn;return true;}
    bool Location(std::uintptr_t,cfgspawn::Vec3& at){at={0,0,100};return true;}
    template<class B>bool FeatureFunction(const B& binding,std::uintptr_t& function,std::uintptr_t& owner){
        if(binding.cache==cfgadditional::disk::Destroy.cache){function=destroyFunction;owner=actorBase;return true;}
        function=spawnAssets.resources.spawnFunction;owner=spawnAssets.resources.classes[cfgspawn::Helper];return true;}
    bool Invoke(std::uintptr_t,std::uintptr_t,void* value,bool){auto& p=*static_cast<cfgvehicle::Projection*>(value);
        ++projectionCalls;p.projected=p.point;p.projected.z-=25;p.result=projectionCalls<=projectionFailures?0:1;return true;}
    bool AdditionalConstructionReady(){return true;}
    bool AdditionalPawnSpawn(std::uintptr_t type,cfgspawn::Vec3,std::uintptr_t& output){bundleAdditional.allocationAttempted=true; // production sets this at the real dispatch since H2.55
        ++calls_;++allocations;CHECK(type==bundleAdditional.selected.pawnClass);output=0;
        if(nullSpawn){if(foreignNull)r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{});return nativeReturned;}
        output=r_.RealObject(type);r_.Put(output+cfgspawn::PawnAIClass,bundleAdditional.selected.defaultAI);
        r_.Put(output+0x3A0,std::uintptr_t{});
        r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,output);
        PublishHeader();Relocate();cfgprobe::lifetime::SlotObservation stale{};
        oldHeaderRejected=!cfgspawn::ObjectSlot(r_,context.objectArray,output,stale);
        if(mutatePrimaryState)r_.Put(bundleJason.killerController.object+0x388,std::uintptr_t{});
        return nativeReturned;
    }
    const cfgspawn::TemplateObservation& JasonTemplate(){return bundleAdditional.selected;}
    bool AdditionalInvoke(std::uintptr_t object,std::uintptr_t function,void* value,bool actor=true){
        ++calls_;
        if(!actor){++allocations;auto& p=*static_cast<cfgspawn::SpawnAIParameters*>(value);CHECK(p.behaviorTree==0&&p.noCollisionFail==1);
            if(nullSpawn){p.result=0;if(foreignNull){r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,std::uintptr_t{});}return nativeReturned;}
            const auto pawn=r_.RealObject(bundleAdditional.selected.pawnClass);
            const auto controller=r_.Object(bundleAdditional.selected.defaultAI);Serial(controller);
            const auto state=nullState?0:r_.Object(spawnAssets.resources.classes[cfgspawn::PlayerState]);if(state)Serial(state);
            r_.Put(pawn+cfgspawn::PawnAIClass,bundleAdditional.selected.defaultAI);r_.Put(pawn+0x3A0,controller);
            r_.Put(controller+0x370,pawn);r_.Put(controller+0x388,state);
            r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,pawn);r_.Put(spawnAssets.gameState+cfgprimary::CurrentKillerState,state);
            PublishHeader();Relocate();p.result=pawn;
            cfgprobe::lifetime::SlotObservation stale{};oldHeaderRejected=!cfgspawn::ObjectSlot(r_,context.objectArray,pawn,stale);
            if(mutatePrimaryState)r_.Put(bundleJason.killerController.object+0x388,state);
            return nativeReturned;
        }
        if(function==destroyFunction){
            CHECK(object!=plan.controller&&object!=plan.pawn&&object!=bundleJason.pawn.object&&object!=bundleJason.killerController.object);
            CHECK(AdditionalPrimary());destroyed.push_back(object);return nativeReturned;
        }
        CHECK(function==spawnAssets.functions[0]);r_.Put(object+0x370,std::uintptr_t{});r_.Put(bundleAdditional.rawPawn+0x3A0,std::uintptr_t{});return true;
    }
    bool AdditionalNativeSpawn(std::uintptr_t type,cfgspawn::Vec3,std::uintptr_t& output){
        ++calls_;++allocations;output=r_.Object(type);Serial(output);const auto state=r_.Object(spawnAssets.resources.classes[cfgspawn::PlayerState]);Serial(state);
        r_.Put(output+0x370,std::uintptr_t{});r_.Put(output+0x388,state);r_.Put(state+0x88,output);PublishHeader();return true;
    }
    bool AdditionalPossessCall(std::uintptr_t controller,std::uintptr_t pawn,cfgnativejason::Fault&){
        const auto state=r_.Object(spawnAssets.resources.classes[cfgspawn::PlayerState]);Serial(state);
        const auto path=r_.Object(spawnAssets.classes[3]);Serial(path);
        r_.Put(controller+0x388,state);r_.Put(state+0x88,controller);r_.Put(controller+0x370,pawn);r_.Put(pawn+0x3A0,controller);r_.Put(pawn+0x388,state);
        r_.Put(controller+0x410,path);r_.Put(path+0x20,controller);r_.Put(path+cfgjason::disk::ComponentOwner,controller);
        r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,pawn);r_.Put(spawnAssets.gameState+cfgprimary::CurrentKillerState,state);PublishHeader();return true;
    }
    unsigned killerClassCalls=0;bool killerClassResult=true; // H2.83 fixture: the native consume is production-only; the birth must continue whatever it returns
    bool AdditionalKillerClass(std::uintptr_t state){++killerClassCalls;return state&&killerClassResult;}
    bool AdditionalNavigation(){return AdditionalLinked();}bool AdditionalRetireGeneric(){return AdditionalLinked();}
    bool nativeHunting=false,soundPending=false;unsigned soundTurns=0,legacyDrives=0;
    bool NativeAdditionalHuntingActive(){return nativeHunting;}
    bool AdditionalSoundPending(){return soundPending;}
    bool AdditionalBurstService(std::uint64_t){return false;}
    bool AdditionalSoundPhase(){++soundTurns;soundPending=false;return true;}
    unsigned AdditionalSelect(){return 2;}bool AdditionalDrive(std::uint64_t){++legacyDrives;return false;}
#include "AdditionalJasonRegistryMembers.hpp"
#include "AdditionalJasonBirthMembers.hpp"
#include "AdditionalJasonLifecycleMembers.hpp"
    BirthFixture(){
        SpectatorFixture seed;static_cast<SpectatorFixture::Reader&>(r_)=seed.r_;plan=seed.plan;context=seed.context;bundleJason=seed.bundleJason;r_.cap=100000;
        spawnAssets.resources.classes=seed.spawnAssets.resources.classes;spawnAssets.killerBase=seed.spawnAssets.killerBase;
        spawnAssets.classes=seed.spawnAssets.classes;spawnAssets.functions=seed.spawnAssets.functions;spawnAssets.functionSlots=seed.spawnAssets.functionSlots;
        spawnAssets.gameState=r_.RealObject(r_.classes[cfgspawn::GameState]);
        r_.Put(plan.world+cfgprobe::WorldGameState,spawnAssets.gameState);
        r_.Put(spawnAssets.gameState+cfgprimary::CurrentKiller,bundleJason.pawn.object);
        r_.Put(spawnAssets.gameState+cfgprimary::CurrentKillerState,seed.killerState);
        CHECK(cfgspawn::ObjectSlot(r_,r_.objects,spawnAssets.gameState,spawnAssets.primary.stateSlot));
        CHECK(bundleAdditional.journal.Capture({bundleJason.pawn.object,seed.killerState}));
        auto& t=bundleAdditional.selected;t.pawnClass=r_.Object();t.defaults=r_.Object(t.pawnClass);t.defaultAI=r_.classes[cfgspawn::BaseAI];
        r_.Put(t.pawnClass+0x30,spawnAssets.killerBase);r_.Put(t.pawnClass+cfgspawn::ClassDefaultObject,t.defaults);
        r_.Put(t.defaults+cfgspawn::PawnAIClass,t.defaultAI);Serial(t.pawnClass);Serial(t.defaults);
        spawnAssets.primary.catalogue={0x990000,1,1};r_.Put(plan.mode+cfgprimary::KillerClasses,spawnAssets.primary.catalogue);
        r_.Put(0x990000,spawnAssets.primary.entries[0]);
        CHECK(cfgspawn::ObjectSlot(r_,r_.objects,t.pawnClass,t.classSlot));CHECK(cfgspawn::ObjectSlot(r_,r_.objects,t.defaults,t.defaultSlot));
        auto& a=spawnAssets.resources;a.spawnFunction=r_.spawn;
        r_.Put(plan.module+cfgspawn::Classes[cfgspawn::Helper].reflection,a.classes[cfgspawn::Helper]);
        r_.Put(plan.module+cfgspawn::Classes[cfgspawn::Helper].constructor,a.classes[cfgspawn::Helper]);
        r_.Put(plan.module+cfgspawn::SpawnFunctionCache,a.spawnFunction);
        CHECK(r_.Read(r_.classes[cfgspawn::Helper]+cfgspawn::ClassDefaultObject,a.helperDefault));
        CHECK(cfgspawn::ObjectSlot(r_,r_.objects,r_.classes[cfgspawn::Helper],spawnAssets.helperClassSlot));
        CHECK(cfgspawn::ObjectSlot(r_,r_.objects,a.helperDefault,spawnAssets.slots[3]));CHECK(cfgspawn::ObjectSlot(r_,r_.objects,a.spawnFunction,spawnAssets.slots[4]));
        // This direct actor-first birth no longer calls the older helper;
        // fixture only the registry functions actually used by the port.
        const std::uintptr_t table=0xA50000;r_.Put(spawnAssets.gameState,table);
        using namespace cfgadditionallifecycle::disk;
        r_.Put(table+0x6F0,plan.module+Add);r_.Put(table+0x6F8,plan.module+Remove);
        r_.Put(plan.module+Add,AddCode);r_.Put(plan.module+Remove,RemoveCode);r_.Put(plan.module+DispatchEmpty,DispatchEmptyCode);
        r_.Put(plan.module+AddedDelegate,AddedDelegateCode);r_.Put(plan.module+RemovedDelegate,RemovedDelegateCode);
        r_.Put(plan.module+cfgnativejason::disk::KillerControllerReflection,a.classes[cfgspawn::KillerAI]);
        r_.Put(plan.module+cfgnativejason::disk::KillerControllerClassCache,a.classes[cfgspawn::KillerAI]);
        r_.Put(plan.module+cfgnativejason::disk::BasePossess,cfgnativejason::disk::BasePossessCode);
        spawnAssets.classes[0]=a.classes[cfgspawn::BaseAI];spawnAssets.functions[0]=r_.Object();
        r_.Put(spawnAssets.functions[0]+0x20,spawnAssets.classes[0]);
        r_.Put(plan.module+cfgjason::disk::Functions[0].cache,spawnAssets.functions[0]);
        r_.Put(plan.module+cfgjason::disk::Functions[0].factory,cfgjason::disk::Functions[0].prefix);
        cfgspawn::ObjectSlot(r_,r_.objects,spawnAssets.functions[0],spawnAssets.functionSlots[0]);
        actorBase=r_.Object();r_.Put(spawnAssets.killerBase+0x30,actorBase);r_.Put(a.classes[cfgspawn::BaseAI]+0x30,actorBase);
        destroyFunction=r_.Object();r_.Put(destroyFunction+0xB0,plan.module+cfgadditional::disk::DestroyExecutor);
        r_.Put(plan.module+cfgadditional::disk::DestroyExecutor,cfgadditional::disk::DestroyCode);
        bundleAdditional.position={600,0,100};Fresh();
    }
    bool Step(cfgadditional::Stage stage){Fresh();bundleAdditional.policy.stage=stage;const bool result=AdditionalBirth(1000);
        if(!result&&stage==cfgadditional::Stage::Pawn){const auto used=r_.reads;r_.cap=100000;
            std::printf("Pawn refusal reads=%u allocated=%u context=%u jason=%u progress=%u template=%u lifecycle=%u primary=%u\n",used,allocations,Context(),Jason(),AdditionalInProgress(),AdditionalTemplate(),AdditionalLifecycle(),AdditionalPrimary());r_.cap=1536;r_.reads=used;}
        CHECK(r_.reads<=1536&&calls_<=12);return result;}
};
#ifndef CFG_ADDITIONAL_BIRTH_FIXTURE_ONLY
int main(){
    using P=cfgadditional::Stage;
    {BirthFixture f;f.bundleAdditional.journal={};f.bundleAdditional.policy.stage=P::Project;f.projectionFailures=11;
     for(unsigned i=0;i<11;++i){f.Fresh();CHECK(f.AdditionalPhase(100+i*100));CHECK(f.bundleAdditional.policy.stage==P::Project&&!f.allocations&&!f.bundleAdditional.journal.captured);}
     f.Fresh();CHECK(f.AdditionalPhase(1200)&&f.bundleAdditional.policy.stage==P::Pawn&&f.projectionCalls==12);
     CHECK(f.bundleAdditional.position.y==-350&&f.bundleAdditional.position.z==100);}
    {BirthFixture f;f.bundleAdditional.journal={};f.bundleAdditional.policy.stage=P::Project;f.projectionFailures=12;
     for(unsigned i=0;i<12;++i){f.Fresh();CHECK(f.AdditionalPhase(100+i*100));}
     CHECK(f.bundleAdditional.policy.stage==P::Refused&&!f.allocations&&f.projectionCalls==12);}
    for(unsigned variant=0;variant<8;++variant){BirthFixture f;f.relocate=(variant&1)!=0;f.nullState=(variant&2)!=0;f.killerClassResult=(variant&4)==0;
        CHECK(f.Step(P::Pawn));CHECK(f.oldHeaderRejected&&f.allocations==1&&f.bundleAdditional.actor.pawn.HasAssignedSerial());CHECK(f.AdditionalPrimary());
        CHECK(!f.bundleAdditional.actor.initialController.object); // no helper-created generic possession
        CHECK(f.Step(P::Detach));
        CHECK(f.Step(P::ControllerClass));CHECK(f.Step(P::Controller));CHECK(f.allocations==2&&f.bundleAdditional.actor.killerController.HasAssignedSerial());
#if CFG_H283_F1_KILLER_CLASS
        CHECK(f.killerClassCalls==1); // offline F1 PlayerState takes the Jason class once, before Possess; a refused class never blocks the birth
#endif
        CHECK(f.Step(P::Possess));CHECK(f.AdditionalPrimary());f.r_.reads=0;CHECK(f.AdditionalLinked());
    }
    for(unsigned foreign=0;foreign<2;++foreign){BirthFixture f;f.nullSpawn=true;f.foreignNull=foreign!=0;
        CHECK(!f.Step(P::Pawn)&&f.allocations==1&&f.bundleAdditional.allocationAttempted);
        CHECK(!f.bundleAdditional.actor.pawn.HasAssignedSerial());f.r_.reads=0;CHECK(f.AdditionalPrimary()==!foreign);}
    {BirthFixture f;CHECK(f.Step(P::Pawn)&&f.Step(P::Detach)&&f.Step(P::ControllerClass)&&f.Step(P::Controller)&&f.Step(P::Possess));
     f.Fresh();std::uintptr_t state=0;CHECK(f.r_.Read(f.bundleAdditional.rawController+0x388,state));
     std::int32_t index=-1;CHECK(f.r_.Read(state+0xC,index));
     f.r_.Put(f.r_.objects.data+24ULL*static_cast<unsigned>(index)+16,std::int32_t{});f.Fresh();
     CHECK(f.AdditionalLinked()); // strongly owned PlayerState can have no weak serial
     f.r_.Put(state+0x88,f.plan.controller);f.Fresh();CHECK(!f.AdditionalLinked());
     f.r_.Put(state+0x88,f.bundleAdditional.rawController);f.Fresh();CHECK(f.AdditionalLinked());
     f.r_.Put(f.bundleAdditional.rawPawn+0x388,f.bundleAdditional.journal.primary.state);f.Fresh();CHECK(!f.AdditionalLinked());
     f.r_.Put(f.bundleAdditional.rawController+0x388,f.bundleAdditional.journal.primary.state);f.Fresh();
     std::uintptr_t observed=0;CHECK(!f.AdditionalControllerState(f.bundleAdditional.actor.killerController,observed));}
    {BirthFixture f;f.nativeReturned=false;CHECK(!f.Step(P::Pawn));f.r_.reads=0;CHECK(f.AdditionalPrimary()&&f.bundleAdditional.actor.pawn.HasAssignedSerial());}
    {BirthFixture f;f.mutatePrimaryState=true;CHECK(!f.Step(P::Pawn));f.r_.reads=0;
        cfgadditionaljournal::Pair observed{};CHECK(f.AdditionalReadPair(observed));
        CHECK(observed.pawn==f.bundleAdditional.rawPawn&&observed.state==f.bundleAdditional.journal.primary.state);}
    {BirthFixture f;f.nullSpawn=true;f.bundleAdditional.policy.stage=P::Pawn;
        f.Fresh();CHECK(f.AdditionalPhase(100)&&f.allocations==1&&f.bundleAdditional.policy.stage==P::Indeterminate);
        f.Fresh();CHECK(!f.AdditionalPhase(200)&&f.allocations==1&&!f.bundleAdditional.rollback);}
    {alignas(8)std::uintptr_t state=0;const auto at=reinterpret_cast<std::uintptr_t>(&state);
        CHECK(!cfgobjective::CompareWrite(at,0,0x123400)&&state==0); // old script writer rejects a legitimate null state
        CHECK(cfgadditionalwrite::Exchange(at,0,0x123400)&&state==0x123400);
        CHECK(!cfgadditionalwrite::Exchange(at,0,0x567800)&&state==0x123400); // foreign update preserved
        CHECK(!cfgadditionalwrite::Exchange(at+1,state,0));
        CHECK(cfgadditionalwrite::Exchange(at,state,0)&&state==0);}
    {BirthFixture f;f.bundleAdditional.policy.stage=P::Pawn;f.Fresh();CHECK(f.AdditionalPhase(100));
        CHECK(f.bundleAdditional.policy.stage==P::Detach&&f.allocations==1);f.r_.reads=0;CHECK(f.AdditionalPrimary());
        // No generic unpossess is needed. A foreign possession fails closed.
        f.r_.Put(f.bundleAdditional.rawPawn+0x3A0,f.plan.controller);f.Fresh();CHECK(f.AdditionalPhase(200));
        CHECK(f.bundleAdditional.policy.stage==P::Indeterminate&&f.bundleAdditional.rollback==1);
        for(unsigned now=300;now<=700;now+=100){f.Fresh();CHECK(f.AdditionalPhase(now)==(now<=500));CHECK(f.r_.reads<=1536&&f.calls_<=12);}
        CHECK(f.allocations==1&&f.destroyed.size()==1&&f.bundleAdditional.rollback==0);
        CHECK(f.destroyed[0]==f.bundleAdditional.rawPawn);}
    std::printf("Included F1 birth/registry with real private-page CAS,appended/relocated slots/null-state/possession:%d failures; native allocations mocked.\n",failures);
    return failures?1:0;
}
#endif
