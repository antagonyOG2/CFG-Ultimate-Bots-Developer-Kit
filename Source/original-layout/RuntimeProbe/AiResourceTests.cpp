#include "AiResourceSnapshot.hpp"
#include "../Core/NativeSpawnParameters.hpp"
#include <map>
#include <cstring>
#include <cstdio>
#include <limits>
#include <string_view>
static int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n",__LINE__,#c); ++failures; } } while(false)
struct Fixture {
    std::map<std::uintptr_t,unsigned char> memory;
    unsigned reads=0,cap=1024;
    std::uintptr_t module=0x140000000,next=0x100000,state=0,pawn=0,defaults=0,bot=0;
    std::uintptr_t botState=0,botController=0,botDefaults=0,pawnClass=0,botClass=0,tree=0;
    std::uintptr_t counselorBase=0,killerBase=0,players=0x710000,spawn=0,mode=0;
    std::array<std::uintptr_t,8> classes{};
    cfgprobe::lifetime::ArrayHeader objects{0x700000,128};
    int nextSlot=0;
    std::uintptr_t trigger=0,mutation=0,newPointer=0;
    unsigned triggerCount=0,triggerAt=2;
    template<class T> void Put(std::uintptr_t address,const T& value) {
        const auto bytes=reinterpret_cast<const unsigned char*>(&value);
        for (std::size_t i=0;i<sizeof(value);++i) memory[address+i]=bytes[i];
    }
    template<class T> bool Read(std::uintptr_t address,T& output) {
        if (reads>=cap) return false;
        if (trigger==address && ++triggerCount==triggerAt) Put(mutation,newPointer);
        ++reads; T candidate{}; auto bytes=reinterpret_cast<unsigned char*>(&candidate);
        for (std::size_t i=0;i<sizeof(candidate);++i) {
            const auto found=memory.find(address+i); if (found==memory.end()) return false;
            bytes[i]=found->second;
        }
        output=candidate; return true;
    }
    std::uintptr_t Object(std::uintptr_t type=0) {
        const auto result=next; next+=0x4000;
        for (std::uintptr_t i=0;i<0x2000;++i) memory[result+i]=0;
        Put(result+0xC,nextSlot); Put(result+0x10,type);
        const auto slot=objects.data+static_cast<std::uintptr_t>(nextSlot++)*24;
        Put(slot,result); Put(slot+8,std::uint32_t{0}); Put(slot+16,std::int32_t{0});
        return result;
    }
    void Serial(std::uintptr_t object,int serial){int index=-1;if(!Read(object+cfgprobe::lifetime::ObjectIndexOffset,index))return;
        const auto slot=objects.data+static_cast<std::uintptr_t>(index)*24;Put(slot+16,serial);}
    Fixture() {
        for (unsigned i=0;i<classes.size();++i) {
            classes[i]=Object(); Put(module+cfgspawn::Classes[i].reflection,classes[i]);
            Put(module+cfgspawn::Classes[i].constructor,classes[i]);
            Serial(classes[i],100+static_cast<int>(i));
        }
        Put(classes[cfgspawn::CounselorAI]+0x30,classes[cfgspawn::BaseAI]);
        Put(classes[cfgspawn::KillerAI]+0x30,classes[cfgspawn::BaseAI]);
        counselorBase=Object(); killerBase=Object();
        pawnClass=Object(); Put(pawnClass+0x30,killerBase);
        botClass=Object(); Put(botClass+0x30,counselorBase);
        pawn=Object(pawnClass); defaults=Object(pawnClass);
        bot=Object(botClass); botDefaults=Object(botClass);
        Put(pawnClass+cfgspawn::ClassDefaultObject,defaults);
        Put(botClass+cfgspawn::ClassDefaultObject,botDefaults);
        Put(defaults+cfgspawn::PawnAIClass,classes[cfgspawn::KillerAI]);
        Put(botDefaults+cfgspawn::PawnAIClass,classes[cfgspawn::CounselorAI]);
        tree=Object(classes[cfgspawn::Tree]);
        Put(botDefaults+cfgspawn::CharacterTree,tree); Put(bot+cfgspawn::CharacterTree,tree);
        mode=Object(); Put(mode+cfgspawn::ModeCounselorTreeOffset,tree);
        const auto helper=Object(classes[cfgspawn::Helper]);
        Put(classes[cfgspawn::Helper]+cfgspawn::ClassDefaultObject,helper);
        spawn=Object(); Put(spawn+0x20,classes[cfgspawn::Helper]);
        Put(module+cfgspawn::SpawnFunctionCache,spawn);
        state=Object(classes[cfgspawn::GameState]);
        botState=Object(classes[cfgspawn::PlayerState]);
        botController=Object(classes[cfgspawn::CounselorAI]);
        const auto brain=Object(classes[cfgspawn::Brain]);
        Put(botController+0x418,brain); Put(botState+0x394,std::uint32_t{4});
        Put(botState+cfgspawn::ActorOwner,botController);
        Put(botController+0x370,bot); Put(botController+0x388,botState);
        Put(bot+0x3A0,botController); Put(bot+0x388,botState);
        Put(players,botState); Put(state+0x380,cfgspawn::PlayerArray{players,1,1});
    }
    bool Capture(cfgspawn::ResourceReport& out,cfgspawn::RosterDiagnostic* roster=nullptr) {
        reads=0;
        return cfgspawn::CaptureResources(*this,module,objects,pawn,2,state,counselorBase,killerBase,out,nullptr,roster);
    }
    bool CaptureHosted(cfgspawn::ResourceReport& out,cfgspawn::ResourceProgress* progress=nullptr){
        reads=0;return cfgspawn::CaptureHostedRuntimeResources(*this,module,objects,out,progress);
    }
    void RealisticRoster(unsigned count) {
        objects.count=512;
        // Shared, nontrivial native ancestry rather than the shallow one-bot
        // fixture that previously missed the real diagnostic budget problem.
        for (const auto root:{classes[cfgspawn::PlayerState],classes[cfgspawn::BaseAI],
            counselorBase,killerBase,classes[cfgspawn::GameState]}) {
            auto current=root;
            for (unsigned depth=0;depth<6;++depth) {
                const auto parent=Object(); Put(current+0x30,parent); current=parent;
            }
        }
        for (unsigned i=1;i<count;++i) {
            const auto npcState=Object(classes[cfgspawn::PlayerState]);
            const auto npcController=Object(classes[cfgspawn::CounselorAI]);
            const auto npcPawn=Object(botClass);
            Put(npcState+0x394,std::uint32_t{4}); Put(npcState+cfgspawn::ActorOwner,npcController);
            Put(npcController+0x370,npcPawn); Put(npcController+0x388,npcState);
            Put(npcPawn+0x3A0,npcController); Put(npcPawn+0x388,npcState);
            Put(npcPawn+cfgspawn::CharacterTree,tree);
            Put(players+static_cast<std::uintptr_t>(i)*8,npcState);
        }
        Put(state+0x380,cfgspawn::PlayerArray{players,static_cast<std::int32_t>(count),static_cast<std::int32_t>(count)});
    }
};
int main() {
    using namespace cfgspawn;
    Fixture fixture; ResourceReport report;
    {
        ResourceProgress progress{};CHECK(fixture.CaptureHosted(report,&progress));
        CHECK(report.classMask==255&&!report.playerArrayObserved&&progress.rosterIndex==-1&&
            std::string_view(progress.stage)=="hosted_classes_complete");
        // Adoption/service class proofs must not be coupled to an unrelated
        // incomplete bot link in the native roster.
        fixture.Put(fixture.botController+0x370,std::uintptr_t{});
        CHECK(!fixture.Capture(report));
        CHECK(fixture.CaptureHosted(report)&&report.classMask==255&&!report.playerArrayObserved);
        fixture.Put(fixture.botController+0x370,fixture.bot);
        fixture.Put(fixture.module+Classes[CounselorAI].constructor,std::uintptr_t{});
        CHECK(!fixture.CaptureHosted(report));
        fixture.Put(fixture.module+Classes[CounselorAI].constructor,fixture.classes[CounselorAI]);
        fixture.Serial(fixture.classes[Brain],0);
        CHECK(fixture.CaptureHosted(report)); // native static caches can be serial-zero, as in the original diagnostic
    }
    fixture.Serial(fixture.classes[Brain],100+static_cast<int>(Brain));
    CHECK(fixture.Capture(report));
    CHECK(report.classMask==255 && report.playerStates==1 && report.botStates==1 && report.counselorBots==1);
    CHECK(report.local.aiKind==2 && !report.local.defaultTreeTyped && !report.local.instanceTreeTyped);
    CHECK(report.counselor.aiKind==1 && report.counselor.defaultTreeTyped && report.counselor.instanceTreeTyped);
    CHECK(report.counselor.brainTyped && report.counselor.controller==fixture.botController);
    CHECK(report.playerArrayObserved==1 && report.counselor.controllerAIKind==1);
    CHECK(report.local.pawnSlot.serial==0); // Observation does NOT invent a serial.
    CHECK(fixture.reads<1024);
    {
        ModeTreeObservation observed{};
        CHECK(ObserveModeCounselorTree(fixture,fixture.objects,fixture.mode,
            fixture.classes[Tree],observed));
        CHECK(observed.tree==fixture.tree && observed.typed && observed.slot.serial==0);
        fixture.Put(fixture.mode+ModeCounselorTreeOffset,std::uintptr_t{0});
        CHECK(ObserveModeCounselorTree(fixture,fixture.objects,fixture.mode,
            fixture.classes[Tree],observed) && !observed.tree && !observed.typed);
        fixture.Put(fixture.spawn+cfgprobe::ObjectClass,fixture.classes[BaseAI]);
        fixture.Put(fixture.mode+ModeCounselorTreeOffset,fixture.spawn);
        CHECK(ObserveModeCounselorTree(fixture,fixture.objects,fixture.mode,
            fixture.classes[Tree],observed) && observed.tree==fixture.spawn && !observed.typed);
        fixture.memory.erase(fixture.tree+cfgprobe::ObjectClass);
        fixture.Put(fixture.mode+ModeCounselorTreeOffset,fixture.tree);
        CHECK(!ObserveModeCounselorTree(fixture,fixture.objects,fixture.mode,
            fixture.classes[Tree],observed));
        fixture.Put(fixture.tree+cfgprobe::ObjectClass,fixture.classes[Tree]);
        Fixture changing;
        changing.trigger=changing.mode+ModeCounselorTreeOffset;
        changing.mutation=changing.mode+ModeCounselorTreeOffset;
        changing.newPointer=0;
        CHECK(!ObserveModeCounselorTree(changing,changing.objects,changing.mode,
            changing.classes[Tree],observed));
    }
    fixture.Put(fixture.module+SpawnFunctionCache,std::uintptr_t{0});
    CHECK(fixture.Capture(report) && !report.spawnFunction);
    fixture.Put(fixture.module+SpawnFunctionCache,fixture.spawn);
    fixture.Put(fixture.botState+0x394,std::uint32_t{0});
    CHECK(fixture.Capture(report) && !report.botStates && !report.counselor.pawn);
    fixture.Put(fixture.botState+0x394,std::uint32_t{4});
    fixture.Put(fixture.bot+0x3A0,std::uintptr_t{0});
    CHECK(!fixture.Capture(report) && !report.local.pawn);
    fixture.Put(fixture.bot+0x3A0,fixture.botController);
    fixture.Put(fixture.defaults+0x10,fixture.botClass);
    CHECK(!fixture.Capture(report));
    fixture.Put(fixture.defaults+0x10,fixture.pawnClass);
    fixture.Put(fixture.state+0x380,PlayerArray{fixture.players,33,33});
    CHECK(!fixture.Capture(report));
    fixture.Put(fixture.state+0x380,PlayerArray{fixture.players,1,1});
    fixture.Put(fixture.players+8,fixture.botState);
    fixture.Put(fixture.state+0x380,PlayerArray{fixture.players,2,2});
    CHECK(!fixture.Capture(report)); // Duplicate state cannot inflate bot count.
    fixture.Put(fixture.state+0x380,PlayerArray{fixture.players,1,1});
    fixture.Put(fixture.spawn+0x20,fixture.classes[BaseAI]);
    CHECK(!fixture.Capture(report));
    fixture.Put(fixture.spawn+0x20,fixture.classes[Helper]);
    fixture.cap=2; CHECK(!fixture.Capture(report)); fixture.cap=1024;
    fixture.Put(fixture.defaults+PawnAIClass,fixture.classes[BaseAI]);
    CHECK(fixture.Capture(report) && report.local.aiKind==3); // Generic != native killer AI.
    fixture.Put(fixture.module+Classes[Helper].constructor,std::uintptr_t{0});
    CHECK(!fixture.Capture(report)); // Populated spawn cannot have unresolved owner.
    for (unsigned change=0;change<3;++change) {
        Fixture live;
        live.trigger=live.state+0x380;
        live.mutation=change==0?live.players:change==1?live.botState+ActorOwner:live.botController+0x418;
        live.newPointer=change==0?live.pawn:0;
        CHECK(!live.Capture(report) && !report.counselor.pawn); // Changes DURING capture.
        CHECK(live.triggerCount==2);
    }
    for (const unsigned count:{7u,32u}) {
        Fixture roster; roster.cap=4096; roster.RealisticRoster(count);
        CHECK(roster.Capture(report) && report.counselorBots==count);
        CHECK(roster.reads<4096);
        if (count==7) CHECK(roster.reads<1024);
        std::printf("Shared ancestry roster %u bots: %u reads (cap4096).\n",count,roster.reads);
        const auto originalReads=roster.reads;cfgspawn::RosterDiagnostic detail;
        CHECK(roster.Capture(report,&detail)&&detail.count==count&&roster.reads==originalReads);
        for(unsigned i=0;i<detail.count;++i)CHECK(detail.rows[i].state&&detail.rows[i].controller&&detail.rows[i].pawn&&detail.rows[i].botFlag==4&&detail.rows[i].kind==1);
        roster.Put(roster.state+0x380,cfgspawn::PlayerArray{roster.players,33,33});
        CHECK(!roster.Capture(report,&detail)&&!detail.count&&!detail.rows[0].state);
    }
    for (unsigned change=0;change<2;++change) {
        Fixture live;
        live.trigger=live.state+0x380;
        live.mutation=change==0?live.botClass+cfgprobe::ClassSuper:live.bot+cfgprobe::ObjectClass;
        live.newPointer=change==0?live.killerBase:live.pawnClass;
        CHECK(!live.Capture(report) && !report.local.pawn); // Cached ancestry/class mutation rejected.
    }
    {
        Fixture cyclic;
        cyclic.Put(cyclic.classes[CounselorAI]+cfgprobe::ClassSuper,cyclic.classes[CounselorAI]);
        CHECK(!cyclic.Capture(report));
    }
    SpawnAIParameters params;
    CHECK(BuildParameters(0x10000,0x20000,0x30000,{10,20,30},{},params));
    CHECK(params.noCollisionFail==0 && !params.result);
    for (const auto byte:params.padding) CHECK(byte==0);
    CHECK(!BuildParameters(0x10001,0x20000,0x30000,{},{},params) && !params.world);
    CHECK(!BuildParameters(0x10000,0x20000,0,{},{},params));
    CHECK(!BuildParameters(0x10000,0x20000,0x30000,{std::numeric_limits<float>::quiet_NaN(),0,0},{},params));
    CHECK(!BuildParameters(0x10000,0x20000,0x30000,{10000001,0,0},{},params));
    std::printf("CFG native AI resource/parameter owned tests %s (%d failures); no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
