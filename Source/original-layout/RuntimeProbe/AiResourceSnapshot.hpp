#pragma once
#include "SpawnBindings.generated.hpp"
#include "ObjectLifetime.hpp"
#include "TypeObservations.hpp"
#include <array>
namespace cfgspawn {
enum ClassId : unsigned { Helper, BaseAI, CounselorAI, KillerAI, Tree, Brain, PlayerState, GameState };
constexpr std::uintptr_t CharacterTree=0x11A0,PawnAIClass=0x380,ActorOwner=0x88;
struct PlayerArray { std::uintptr_t data=0; std::int32_t count=0,capacity=0; };
static_assert(sizeof(PlayerArray)==16,"Native PlayerArray header");
struct TemplateObservation {
    std::uintptr_t pawn=0,pawnClass=0,defaults=0,defaultAI=0,defaultTree=0,instanceTree=0;
    std::uintptr_t controller=0,brain=0;
    std::uint32_t kind=0,aiKind=0,controllerAIKind=0,defaultTreeTyped=0,instanceTreeTyped=0,brainTyped=0;
    cfgprobe::lifetime::SlotObservation pawnSlot{},classSlot{},defaultSlot{};
};
struct ResourceReport {
    std::array<std::uintptr_t,8> classes{};
    std::uint32_t classMask=0,playerArrayObserved=0,playerStates=0,botStates=0,counselorBots=0,killerBots=0;
    std::uintptr_t spawnFunction=0,helperDefault=0;
    cfgprobe::lifetime::SlotObservation spawnSlot{},helperSlot{};
    TemplateObservation local{},counselor{};
};
struct StateObservation {
    std::uintptr_t state=0,controller=0,pawn=0;
    std::uint32_t botFlag=0,kind=0;
    cfgprobe::lifetime::SlotObservation stateSlot{},controllerSlot{},pawnSlot{};
};
struct RosterDiagnostic {std::array<StateObservation,32> rows{};unsigned count=0;};
struct ResourceProgress { const char* stage="not_started"; std::int32_t rosterIndex=-1; };
struct ModeTreeObservation {
    std::uintptr_t tree=0;
    cfgprobe::lifetime::SlotObservation slot{};
    bool typed=false;
};
// Read the mode-owned counselor tree used by the existing spawn path. Missing
// or mistyped values are diagnostic observations; mutations are refused.
// A positive slot serial is not proof of object lifetime or GC eligibility.
template<class Reader> bool ObserveModeCounselorTree(Reader& r,
    const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t mode,
    std::uintptr_t treeClass,ModeTreeObservation& out) {
    out={};
    cfgprobe::lifetime::SlotObservation modeSlot{};
    if (!ObjectSlot(r,array,mode,modeSlot)) return false;
    ModeTreeObservation candidate;
    if (!r.Read(mode+ModeCounselorTreeOffset,candidate.tree)) return false;
    cfgprobe::Ancestry observedAncestry{};
    if (candidate.tree) {
        if (!cfgprobe::ObjectPointer(treeClass) ||
            !cfgprobe::CaptureAncestry(r,candidate.tree,observedAncestry) ||
            !cfgprobe::StableAncestry(r,candidate.tree,observedAncestry) ||
            !ObjectSlot(r,array,candidate.tree,candidate.slot)) return false;
        candidate.typed=observedAncestry.Contains(treeClass);
    }
    cfgprobe::lifetime::SlotObservation fresh{};
    std::uintptr_t treeAgain=0;
    if (!ObjectSlot(r,array,mode,fresh) || !(fresh==modeSlot) ||
        !r.Read(mode+ModeCounselorTreeOffset,treeAgain) || treeAgain!=candidate.tree) return false;
    if (candidate.tree) {
        cfgprobe::Ancestry freshAncestry{};
        if (!ObjectSlot(r,array,candidate.tree,fresh) || !(fresh==candidate.slot) ||
            !cfgprobe::CaptureAncestry(r,candidate.tree,freshAncestry) ||
            !cfgprobe::StableAncestry(r,candidate.tree,freshAncestry) ||
            freshAncestry.count!=observedAncestry.count) return false;
        for (std::size_t i=0;i<freshAncestry.count;++i)
            if (freshAncestry.classes[i]!=observedAncestry.classes[i] ||
                freshAncestry.parents[i]!=observedAncestry.parents[i]) return false;
    }
    out=candidate;
    return true;
}
template<class Reader> bool ClassContains(Reader& r,std::uintptr_t current,std::uintptr_t wanted) {
    if (!cfgprobe::ObjectPointer(wanted)) return false;
    std::array<std::uintptr_t,32> visited{}; unsigned count=0;
    while (current) {
        if (!cfgprobe::ObjectPointer(current) || count==visited.size()) return false;
        for (unsigned i=0;i<count;++i) if (visited[i]==current) return false;
        visited[count++]=current;
        if (current==wanted) return true;
        if (!r.Read(current+cfgprobe::ClassSuper,current)) return false;
    }
    return false;
}
template<class Reader> bool ObjectIs(Reader& r,std::uintptr_t object,std::uintptr_t native) {
    cfgprobe::Ancestry ancestry;
    return cfgprobe::CaptureAncestry(r,object,ancestry) && ancestry.Contains(native) &&
        cfgprobe::StableAncestry(r,object,ancestry);
}
template<class Reader> bool ObjectSlot(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot) {
    std::uint32_t flags=0;
    return cfgprobe::lifetime::ReadSlot(r,array,object,slot) &&
        r.Read(array.data+static_cast<std::uintptr_t>(slot.index)*24+8,flags) && !(flags&0x20000000);
}
template<class Reader> bool ObserveTemplate(Reader& r,TypeObservations<Reader>& types,const cfgprobe::lifetime::ArrayHeader& array,
    const std::array<std::uintptr_t,8>& classes,std::uintptr_t pawn,unsigned kind,
    TemplateObservation& out) {
    out={}; TemplateObservation candidate; candidate.pawn=pawn; candidate.kind=kind;
    if (!ObjectSlot(r,array,pawn,candidate.pawnSlot) ||
        !r.Read(pawn+cfgprobe::ObjectClass,candidate.pawnClass) ||
        !ObjectSlot(r,array,candidate.pawnClass,candidate.classSlot) ||
        !r.Read(candidate.pawnClass+ClassDefaultObject,candidate.defaults) ||
        !ObjectSlot(r,array,candidate.defaults,candidate.defaultSlot)) return false;
    std::uintptr_t defaultClass=0;
    if (!r.Read(candidate.defaults+cfgprobe::ObjectClass,defaultClass) || defaultClass!=candidate.pawnClass ||
        !r.Read(candidate.defaults+PawnAIClass,candidate.defaultAI) ||
        !r.Read(candidate.defaults+CharacterTree,candidate.defaultTree) ||
        !r.Read(pawn+CharacterTree,candidate.instanceTree)) return false;
    if (candidate.defaultAI) {
        if (types.ClassContains(candidate.defaultAI,classes[CounselorAI])) candidate.aiKind=1;
        else if (types.ClassContains(candidate.defaultAI,classes[KillerAI])) candidate.aiKind=2;
        else if (types.ClassContains(candidate.defaultAI,classes[BaseAI])) candidate.aiKind=3;
        else candidate.aiKind=4; // Unknown, not adopted as native AI.
    }
    candidate.defaultTreeTyped=candidate.defaultTree && types.ObjectIs(candidate.defaultTree,classes[Tree]);
    candidate.instanceTreeTyped=candidate.instanceTree && types.ObjectIs(candidate.instanceTree,classes[Tree]);
    cfgprobe::lifetime::SlotObservation fresh;
    std::uintptr_t classAgain=0,defaultAgain=0,aiAgain=0,treeAgain=0,instanceAgain=0;
    if (!ObjectSlot(r,array,pawn,fresh) || !(fresh==candidate.pawnSlot) ||
        !r.Read(pawn+cfgprobe::ObjectClass,classAgain) || classAgain!=candidate.pawnClass ||
        !ObjectSlot(r,array,candidate.pawnClass,fresh) || !(fresh==candidate.classSlot) ||
        !r.Read(candidate.pawnClass+ClassDefaultObject,defaultAgain) || defaultAgain!=candidate.defaults ||
        !ObjectSlot(r,array,candidate.defaults,fresh) || !(fresh==candidate.defaultSlot) ||
        !r.Read(candidate.defaults+PawnAIClass,aiAgain) || aiAgain!=candidate.defaultAI ||
        !r.Read(candidate.defaults+CharacterTree,treeAgain) || treeAgain!=candidate.defaultTree ||
        !r.Read(pawn+CharacterTree,instanceAgain) || instanceAgain!=candidate.instanceTree) return false;
    out=candidate; return true;
}
// One bounded paused diagnostic. No object enumeration, factories, engine calls,
// actor writes, hooks, retained assets, worker, polling or runtime capability.
template<class Reader> bool CaptureResources(Reader& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t pawn,unsigned kind,
    std::uintptr_t gameState,std::uintptr_t counselorBase,std::uintptr_t killerBase,
    ResourceReport& out,ResourceProgress* progress=nullptr,RosterDiagnostic* roster=nullptr) {
    out={};if(roster)*roster={};ResourceReport candidate; TypeObservations<Reader> types(r);
    std::array<StateObservation,32> observedStates{};
    if (progress) *progress={"native_class_caches",-1};
    std::array<std::uintptr_t,8> reflection{},constructor{};
    for (unsigned i=0;i<Classes.size();++i) {
        if (progress) progress->rosterIndex=static_cast<std::int32_t>(i);
        if (!r.Read(module+Classes[i].reflection,reflection[i]) ||
            !r.Read(module+Classes[i].constructor,constructor[i])) return false;
        candidate.classes[i]=cfgprobe::MatchingClassCache(reflection[i],constructor[i]);
        if (candidate.classes[i]) candidate.classMask|=1u<<i;
    }
    if (progress) *progress={"spawn_function_cache",-1};
    if (!r.Read(module+SpawnFunctionCache,candidate.spawnFunction)) return false;
    if (candidate.spawnFunction) {
        std::uintptr_t owner=0;
        if (!candidate.classes[Helper]) {
            if (progress) progress->stage="spawn_helper_class_missing";
            return false;
        }
        if (progress) progress->stage="spawn_function_slot";
        if (!ObjectSlot(r,array,candidate.spawnFunction,candidate.spawnSlot)) return false;
        if (progress) progress->stage="spawn_function_owner";
        if (!r.Read(candidate.spawnFunction+0x20,owner)) return false;
        if (owner!=candidate.classes[Helper]) {
            if (progress) progress->stage="spawn_function_wrong_owner";
            return false;
        }
    }
    if (candidate.classes[Helper]) {
        if (progress) progress->stage="helper_default";
        if (!r.Read(candidate.classes[Helper]+ClassDefaultObject,candidate.helperDefault)) return false;
        if (candidate.helperDefault) {
            std::uintptr_t helperClass=0;
            if (progress) progress->stage="helper_default_slot";
            if (!ObjectSlot(r,array,candidate.helperDefault,candidate.helperSlot) ||
                !r.Read(candidate.helperDefault+cfgprobe::ObjectClass,helperClass) || helperClass!=candidate.classes[Helper]) return false;
        }
    }
    if (progress) progress->stage="local_template";
    if (!ObserveTemplate(r,types,array,candidate.classes,pawn,kind,candidate.local)) return false;
    // Only the native, finite GameState.PlayerArray; never scan GObjects/actors.
    if (progress) progress->stage="native_roster";
    if (candidate.classes[GameState] && types.ObjectIs(gameState,candidate.classes[GameState])) {
        PlayerArray players;
        if (!r.Read(gameState+0x380,players) || players.count<0 || players.count>32 ||
            players.capacity<players.count || players.capacity>128 ||
            (players.count && (!cfgprobe::ObjectPointer(players.data) ||
                !cfgprobe::lifetime::Extent(players.data,static_cast<std::uintptr_t>(players.count)*8)))) return false;
        candidate.playerStates=static_cast<unsigned>(players.count);
        candidate.playerArrayObserved=1;
        cfgprobe::lifetime::SlotObservation gameStateSlot{},brainSlot{};
        if (!ObjectSlot(r,array,gameState,gameStateSlot)) return false;
        for (std::int32_t i=0;i<players.count;++i) {
            if (progress) progress->rosterIndex=i;
            std::uintptr_t state=0,controller=0,botPawn=0,backlink=0;
            std::uint32_t botFlag=0;
            if (!r.Read(players.data+static_cast<std::uintptr_t>(i)*8,state) ||
                !types.ObjectIs(state,candidate.classes[PlayerState]) || !r.Read(state+0x394,botFlag)) return false;
            for (std::int32_t j=0;j<i;++j) if (observedStates[static_cast<unsigned>(j)].state==state) return false;
            auto& observed=observedStates[static_cast<unsigned>(i)];
            observed.state=state; observed.botFlag=botFlag&4;
            if (!ObjectSlot(r,array,state,observed.stateSlot)) return false;
            if (!(botFlag&4)) continue;
            ++candidate.botStates;
            if (!r.Read(state+ActorOwner,controller)) return false;
            observed.controller=controller;
            if (!controller) continue; // Diagnostic absence, not an invented owner.
            if (!types.ObjectIs(controller,candidate.classes[BaseAI]) ||
                !r.Read(controller+0x370,botPawn) || !botPawn ||
                !r.Read(controller+0x388,backlink) || backlink!=state ||
                !r.Read(botPawn+0x3A0,backlink) || backlink!=controller ||
                !r.Read(botPawn+0x388,backlink) || backlink!=state) return false;
            observed.pawn=botPawn;
            if (!ObjectSlot(r,array,controller,observed.controllerSlot) ||
                !ObjectSlot(r,array,botPawn,observed.pawnSlot)) return false;
            if (types.ObjectIs(botPawn,counselorBase)) {
                observed.kind=1;
                ++candidate.counselorBots;
                if (!candidate.counselor.pawn) {
                    if (!ObserveTemplate(r,types,array,candidate.classes,botPawn,1,candidate.counselor) ||
                        !r.Read(controller+0x418,candidate.counselor.brain)) return false;
                    candidate.counselor.controller=controller;
                    std::uintptr_t controllerClass=0;
                    if (!r.Read(controller+cfgprobe::ObjectClass,controllerClass)) return false;
                    candidate.counselor.controllerAIKind=types.ClassContains(controllerClass,candidate.classes[CounselorAI])?1u:
                        types.ClassContains(controllerClass,candidate.classes[KillerAI])?2u:3u;
                    candidate.counselor.brainTyped=candidate.counselor.brain &&
                        types.ObjectIs(candidate.counselor.brain,candidate.classes[Brain]);
                    if (candidate.counselor.brain && !ObjectSlot(r,array,candidate.counselor.brain,brainSlot)) return false;
                }
            } else if (types.ObjectIs(botPawn,killerBase)) { observed.kind=2; ++candidate.killerBots; }
        }
        if (progress) *progress={"roster_recheck",-1};
        PlayerArray again;
        if (!r.Read(gameState+0x380,again) || again.data!=players.data ||
            again.count!=players.count || again.capacity!=players.capacity) return false;
        cfgprobe::lifetime::SlotObservation fresh;
        if (!ObjectSlot(r,array,gameState,fresh) || !(fresh==gameStateSlot)) return false;
        for (std::int32_t i=0;i<players.count;++i) {
            if (progress) progress->rosterIndex=i;
            const auto& observed=observedStates[static_cast<unsigned>(i)];
            std::uintptr_t entry=0,controller=0,botPawn=0,backlink=0; std::uint32_t flag=0;
            if (!r.Read(players.data+static_cast<std::uintptr_t>(i)*8,entry) || entry!=observed.state ||
                !ObjectSlot(r,array,entry,fresh) || !(fresh==observed.stateSlot) ||
                !r.Read(entry+0x394,flag) || (flag&4)!=observed.botFlag) return false;
            if (!observed.botFlag) continue;
            if (!r.Read(entry+ActorOwner,controller) || controller!=observed.controller) return false;
            if (!controller) continue;
            if (!ObjectSlot(r,array,controller,fresh) || !(fresh==observed.controllerSlot) ||
                !types.ObjectIs(controller,candidate.classes[BaseAI]) ||
                !r.Read(controller+0x370,botPawn) || botPawn!=observed.pawn ||
                !ObjectSlot(r,array,botPawn,fresh) || !(fresh==observed.pawnSlot) ||
                !r.Read(controller+0x388,backlink) || backlink!=entry ||
                !r.Read(botPawn+0x3A0,backlink) || backlink!=controller ||
                !r.Read(botPawn+0x388,backlink) || backlink!=entry ||
                (observed.kind==1 && !types.ObjectIs(botPawn,counselorBase)) ||
                (observed.kind==2 && !types.ObjectIs(botPawn,killerBase))) return false;
        }
        if (candidate.counselor.controller) {
            std::uintptr_t brain=0;
            if (!r.Read(candidate.counselor.controller+0x418,brain) || brain!=candidate.counselor.brain ||
                (brain && (!ObjectSlot(r,array,brain,fresh) || !(fresh==brainSlot)))) return false;
        }
    }
    if (progress) *progress={"native_cache_recheck",-1};
    for (unsigned i=0;i<Classes.size();++i) {
        std::uintptr_t a=0,b=0;
        if (!r.Read(module+Classes[i].reflection,a) || a!=reflection[i] ||
            !r.Read(module+Classes[i].constructor,b) || b!=constructor[i]) return false;
    }
    std::uintptr_t functionAgain=0;
    if (!r.Read(module+SpawnFunctionCache,functionAgain) || functionAgain!=candidate.spawnFunction) return false;
    cfgprobe::lifetime::SlotObservation fresh;
    if (candidate.spawnFunction) {
        std::uintptr_t owner=0;
        if (!ObjectSlot(r,array,candidate.spawnFunction,fresh) || !(fresh==candidate.spawnSlot) ||
            !r.Read(candidate.spawnFunction+0x20,owner) || owner!=candidate.classes[Helper]) return false;
    }
    if (candidate.classes[Helper]) {
        std::uintptr_t defaultAgain=0;
        if (!r.Read(candidate.classes[Helper]+ClassDefaultObject,defaultAgain) || defaultAgain!=candidate.helperDefault) return false;
        if (defaultAgain && (!ObjectSlot(r,array,defaultAgain,fresh) || !(fresh==candidate.helperSlot))) return false;
    }
    if (progress) progress->stage="type_recheck";
    if (!types.Revalidate()) return false;
    if (progress) progress->stage="complete";
    if(roster){roster->rows=observedStates;roster->count=candidate.playerStates;}
    out=candidate; return true;
}
// Hosted adoption and the independent hosted counselor service need the same
// resident class bindings as the spawn path, but they do not own or inspect
// the unrelated PlayerArray roster. Keep this collector deliberately limited
// to the eight exact native class cache pairs and their current object slots.
// It never allocates classes, infers missing caches, or scans player entries.
template<class Reader> bool CaptureHostedRuntimeResources(Reader& r,std::uintptr_t module,
    const cfgprobe::lifetime::ArrayHeader& array,ResourceReport& out,ResourceProgress* progress=nullptr) {
    out={};ResourceReport candidate;
    std::array<std::uintptr_t,8> reflection{},constructor{};
    std::array<cfgprobe::lifetime::SlotObservation,8> slots{};
    for(unsigned i=0;i<Classes.size();++i){
        if(progress){progress->stage="hosted_class_caches";progress->rosterIndex=static_cast<std::int32_t>(i);}
        if(!r.Read(module+Classes[i].reflection,reflection[i])||
           !r.Read(module+Classes[i].constructor,constructor[i]))return false;
        candidate.classes[i]=cfgprobe::MatchingClassCache(reflection[i],constructor[i]);
        if(!candidate.classes[i]||!ObjectSlot(r,array,candidate.classes[i],slots[i]))return false;
        candidate.classMask|=1u<<i;
    }
    for(unsigned i=0;i<Classes.size();++i){
        if(progress){progress->stage="hosted_class_recheck";progress->rosterIndex=static_cast<std::int32_t>(i);}
        std::uintptr_t a=0,b=0;cfgprobe::lifetime::SlotObservation fresh;
        if(!r.Read(module+Classes[i].reflection,a)||a!=reflection[i]||
           !r.Read(module+Classes[i].constructor,b)||b!=constructor[i]||
           !ObjectSlot(r,array,candidate.classes[i],fresh)||!(fresh==slots[i]))return false;
    }
    if(progress){progress->stage="hosted_classes_complete";progress->rosterIndex=-1;}
    out=candidate;return candidate.classMask==255;
}
} // namespace cfgspawn
