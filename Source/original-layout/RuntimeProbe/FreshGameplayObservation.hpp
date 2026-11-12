#pragma once
#include "NativeCounselorBirthObservation.hpp"
namespace cfgmenu::freshgame {
constexpr unsigned ReadLimit=2048;
struct Source {
    std::uintptr_t module=0,engine=0,viewport=0,player=0,gameInstance=0,frontendWorld=0;
    std::uint32_t thread=0;
    bool nativeOffline=false;
};
struct Observation {cfgobserver::Plan plan{};birth::Observation human{};};
enum class Step : unsigned {None,Input,World,Player,Identity,Dispatcher,ClassDefaults,Birth,Context,Ready};
struct Trace {Step step=Step::None;unsigned defaultIndex=0;birth::Trace human{};};
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T>bool Read(std::uintptr_t at,T& output){if(reads>=ReadLimit)return false;++reads;return reader_.Read(at,output);}
};
// One owner-callback arrival observation, NOT a worker/poll or ownership repair.
// Rebuild the plan from the current engine chain, retaining only persistent
// source identities. Neither the old menu HUD nor its controller is a pawn.
// Success proves native counselor birth consistency, not selected map/Jason
// association; those must be independently established before AI configuration.
template<class Reader>bool Observe(Reader& reader,const Source& source,std::uint32_t callerThread,
    bool exactImageVerified,Observation& output,unsigned* consumed=nullptr,Trace* diagnostic=nullptr){
    output={};if(consumed)*consumed=0;Bounded<Reader> r(reader);
    Trace trace;trace.step=Step::Input;
    const auto finish=[&](bool okay){if(consumed)*consumed=r.reads;if(diagnostic)*diagnostic=trace;return okay;};
    using namespace cfgprobe;
    if(!exactImageVerified||!callerThread||callerThread!=source.thread||!ObjectPointer(source.frontendWorld))return finish(false);
    cfgobserver::Plan p;p.bytes=sizeof(p);p.module=source.module;p.threadId=callerThread;
    trace.step=Step::World;
    if(!ObjectPointer(p.module)||!r.Read(p.module+EngineGlobalCandidate,p.engine)||p.engine!=source.engine||
       !r.Read(p.engine+EngineGameViewport,p.viewport)||p.viewport!=source.viewport||
       !r.Read(p.viewport+ViewportWorld,p.world)||!ObjectPointer(p.world)||p.world==source.frontendWorld||
       !cfgobserver::Same(r,p.world+WorldNetDriver,std::uintptr_t{})||!r.Read(p.world+WorldGameMode,p.mode)||!ObjectPointer(p.mode)||
       !r.Read(p.world+WorldGameInstance,p.gameInstance)||p.gameInstance!=source.gameInstance)return finish(false);
    cfgobserver::LocalPlayers local;
    trace.step=Step::Player;
    if(!r.Read(p.gameInstance+GameInstanceLocalPlayers,local)||local.count!=1||local.capacity<1||local.capacity>1024||
       !ObjectPointer(local.data)||!r.Read(local.data,p.player)||p.player!=source.player||
       !r.Read(p.player+PlayerController,p.controller)||!ObjectPointer(p.controller)||
       !r.Read(p.controller+ControllerPawn,p.pawn)||!ObjectPointer(p.pawn))return finish(false);
    p.localPlayerData=local.data;
    const std::array<std::uintptr_t,4> actors{{p.world,p.mode,p.controller,p.pawn}};
    trace.step=Step::Identity;
    if(!lifetime::ObserveBatch(r,p.module+lifetime::ObjectArrayRva,actors,p.objectArray,p.objects))return finish(false);
    trace.step=Step::Dispatcher;
    if(!dispatch::Observe(r,p.module,p.pawn,p.dispatch)||!cfgobserver::SanePlan(p))return finish(false);
    // Ordinary owned instances only. Being a class-family member alone must not
    // accidentally admit a loaded class default as a gameplay owner.
    trace.step=Step::ClassDefaults;
    for(unsigned i=1;i<actors.size();++i){std::uintptr_t type=0,defaults=0;trace.defaultIndex=i;
        if(!r.Read(actors[i]+ObjectClass,type)||!ObjectPointer(type)||
           !r.Read(type+0xF8,defaults)||!ObjectPointer(defaults)||defaults==actors[i]||
           !cfgobserver::Same(r,defaults+ObjectClass,type))return finish(false);}
    birth::Observation human;
    trace.step=Step::Birth;
    if(!birth::Observe(r,p,callerThread,true,human,nullptr,&trace.human,source.nativeOffline))return finish(false);
    trace.step=Step::Context;
    if(!cfgobserver::ContextUnchanged(r,p)||
       !cfgobserver::Same(r,p.module+EngineGlobalCandidate,source.engine)||
       !cfgobserver::Same(r,p.gameInstance+GameInstanceLocalPlayers,p.localPlayerData))return finish(false);
    output.plan=p;output.human=human;trace.step=Step::Ready;return finish(true);
}
}
