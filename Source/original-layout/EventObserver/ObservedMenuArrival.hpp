#pragma once
#include "NativeMenuArrival.hpp"
#include "PrimarySpawnObservation.hpp"
#include "../RuntimeProbe/FreshGameplayObservation.hpp"
#include "../RuntimeProbe/FreshWorldMapObservation.hpp"
#include "../RuntimeProbe/NativeClassPathChoice.hpp"

namespace cfgmenu::observedarrival {
constexpr unsigned ReadLimit=16384;
enum class Failure {None,Source,Birth,Map,Resources,Jason,Changed,Handoff};
struct Observation {
    cfgobserver::Plan plan{};
    cfgbundle::Configuration configuration{};
    cfgprimary::Report resources{};
    worldmap::Variant mapVariant=worldmap::Variant::None;
    cfgnames::Text actualPackage{}; // typed acknowledged authored-world proof, never a menu label
    bool nativeOffline=false;
};
template<class Reader>class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& reader):reader_(reader){}
    template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=ReadLimit)return false;++reads;return reader_.Read(at,out);}
};
inline bool SameResources(const cfgprimary::Report& a,const cfgprimary::Report& b){
    return a.catalogue.data==b.catalogue.data&&a.catalogue.count==b.catalogue.count&&a.catalogue.capacity==b.catalogue.capacity&&
        a.entries==b.entries&&a.selected==b.selected&&a.killer==b.killer&&a.killerState==b.killerState&&
        a.counselorTree==b.counselorTree&&a.modeSlot==b.modeSlot&&a.stateSlot==b.stateSlot&&a.treeSlot==b.treeSlot;
}
// Concrete read-only arrival acquisition. No caller-supplied map, roster,
// class/birth success flags or catalogue index can become configuration.
// Host invokes once AFTER native birth on the owner thread. This does not
// install startup gates, publish the new Plan, configure AI or request a spawn.
template<class Reader>bool Consume(Reader& reader,GameplayHandoff& handoff,const freshgame::Source& source,
    std::uint32_t callerThread,bool exactImageVerified,Observation& output,unsigned* consumed=nullptr,Failure* failure=nullptr,freshgame::Trace* birthTrace=nullptr,arrival::Trace* handoffTrace=nullptr){
    output={};output.configuration.submission=0;if(consumed)*consumed=0;if(failure)*failure=Failure::Source;
    if(handoffTrace)*handoffTrace={};
    MenuCatalogueEvidence armed;Selection selection;std::uint64_t submission=0;
    if(!handoff.InspectArmed(armed,selection,submission))return false; // Duplicate/retired route: zero reads.
    Bounded<Reader> r(reader);Failure why=Failure::Source;
    const auto finish=[&](bool okay){
        if(consumed)*consumed=r.reads;if(failure)*failure=okay?Failure::None:why;
        if(!okay&&handoff.State()==HandoffState::Armed){cfgbundle::Configuration invalid;handoff.Consume({},invalid);}
        return okay;
    };
    if(source.frontendWorld!=armed.frontendWorld||!exactImageVerified||callerThread!=source.thread)return finish(false);
    freshgame::Observation fresh;why=Failure::Birth;
    if(!freshgame::Observe(r,source,callerThread,true,fresh,nullptr,birthTrace))return finish(false);
    worldmap::ArmedMap selectedMap{source.module,armed.frontendWorld,armed.map,armed.generation,submission};
    worldmap::Observation map;why=Failure::Map;
    if(!worldmap::Observe(r,fresh.plan,selectedMap,callerThread,true,map,nullptr,nullptr,source.nativeOffline))return finish(false);
    why=Failure::Resources;std::uintptr_t state=0;
    const std::array<cfgprobe::NativeClass,3> ids{{source.nativeOffline?cfgprobe::OfflineBotsMode:cfgprobe::SandboxMode,cfgprobe::Counselor,cfgprobe::Killer}};
    std::array<std::uintptr_t,5> native{};
    const auto cache=[&](std::uintptr_t a,std::uintptr_t b,std::uintptr_t& value){
        std::uintptr_t other=0;return r.Read(source.module+a,value)&&r.Read(source.module+b,other)&&cfgprobe::MatchingClassCache(value,other);
    };
    for(unsigned i=0;i<ids.size();++i){const auto& binding=cfgprobe::NativeClasses[ids[i]];
        if(!cache(binding.reflectionCacheRva,binding.constructorCacheRva,native[i]))return finish(false);}
    const auto& playerState=cfgspawn::Classes[cfgspawn::PlayerState];const auto& tree=cfgspawn::Classes[cfgspawn::Tree];
    if(!cache(playerState.reflection,playerState.constructor,native[3])||!cache(tree.reflection,tree.constructor,native[4])||
       !cfgprimary::LookupVetoClearForObservation(r,source.module)||!r.Read(fresh.plan.world+cfgprobe::WorldGameState,state)||!cfgprobe::ObjectPointer(state))return finish(false);
    cfgprimary::Report resources;
    // Native handoff is deliberately before AI creation and after intro metadata
    // restoration. Do not weaken EmptyPrimary to turn this into an adoption API.
    std::uintptr_t capturedNativeState=0;
    const auto nativeStateOkay=[&](){if(!source.nativeOffline)return true;std::uintptr_t klass=0,defaults=0;
        if(!cache(intrometadatadisk::SCGameState_OfflineBotsClass,intrometadatadisk::SCGameState_OfflineBotsClass+8,klass)||
           !cfgobserver::Same(r,state+cfgprobe::ObjectClass,klass)||!cfgobserver::Same(r,fresh.plan.mode+0x3C0,state)||
           (capturedNativeState&&capturedNativeState!=klass)||
           !r.Read(klass+0xF8,defaults)||!cfgprobe::ObjectPointer(defaults)||defaults==state||!cfgobserver::Same(r,defaults+cfgprobe::ObjectClass,klass))return false;
        capturedNativeState=klass;return true;
    };
    if(!nativeStateOkay())return finish(false);
    const auto capture=[&](cfgprimary::Report& value){return cfgprimary::Capture(r,source.module,fresh.plan.objectArray,
        fresh.plan.mode,state,native[0],fresh.plan.pawn,native[1],native[2],native[3],native[4],value);};
    if(!capture(resources)||!resources.EmptyPrimary()||!resources.catalogue.count||!resources.counselorTree||
       !(resources.modeSlot==fresh.plan.objects[1])||!resources.stateSlot.HasAssignedSerial())return finish(false);
    catalogues::CharacterList roster;roster.header={resources.catalogue.data,resources.catalogue.count,resources.catalogue.capacity};
    roster.entries=resources.entries;classchoice::Choice jason;why=Failure::Jason;
    const auto selected=[&](classchoice::Choice& choice){
        const bool explicitClass=armed.selectedClass.Valid();
        if(!armed.selectedClass.Empty()&&(!explicitClass||armed.selectedClass.identity.object!=armed.jason))return false;
        if(explicitClass&&!cfgselectedjason::Current(r,source.module,fresh.plan.objectArray,native[2],armed.selectedClass))return false;
        classchoice::Diagnostic diagnostic;
        const bool found=classchoice::MatchLoaded(r,source.module,armed.jason,native[2],roster,choice,nullptr,&diagnostic);
        if(!found){if(!explicitClass||diagnostic.stage!=7||diagnostic.matches)return false;
            choice={armed.jason,cfgselectedjason::NoCatalogueIndex,{}};
        }else if(!explicitClass&&!softpath::Same(choice.packet,armed.selectedJason))return false;
        // An already-selected native GameState path may not conflict with the
        // exact accepted class, including on the no-catalogue-index route.
        if(explicitClass&&!softpath::Empty(resources.selected)&&
            !classchoice::MatchesPath(r,source.module,armed.jason,native[2],resources.selected))return false;
        return true;
    };
    if(!selected(jason))return finish(false);
    why=Failure::Changed;cfgprimary::Report again;worldmap::Observation mapAgain;classchoice::Choice jasonAgain;
    if(!capture(again)||!SameResources(resources,again)||!cfgobserver::Same(r,fresh.plan.world+cfgprobe::WorldGameState,state)||
       !worldmap::Observe(r,fresh.plan,selectedMap,callerThread,true,mapAgain,nullptr,nullptr,source.nativeOffline)||mapAgain.variant!=map.variant||
       !(mapAgain.definitionIdentity==map.definitionIdentity)||!(mapAgain.defaultsIdentity==map.defaultsIdentity)||
       !(mapAgain.normalPath==map.normalPath)||!(mapAgain.snowPath==map.snowPath)||
       !selected(jasonAgain)||jasonAgain.type!=jason.type||
       jasonAgain.index!=jason.index||jasonAgain.packet!=jason.packet||
       !cfgprimary::LookupVetoClearForObservation(r,source.module)||!cfgobserver::ContextUnchanged(r,fresh.plan)||!nativeStateOkay())return finish(false);
    for(unsigned i=0;i<ids.size();++i){const auto& binding=cfgprobe::NativeClasses[ids[i]];
        if(!cfgobserver::Same(r,source.module+binding.reflectionCacheRva,native[i])||
           !cfgobserver::Same(r,source.module+binding.constructorCacheRva,native[i]))return finish(false);}
    if(!cfgobserver::Same(r,source.module+playerState.reflection,native[3])||!cfgobserver::Same(r,source.module+playerState.constructor,native[3])||
       !cfgobserver::Same(r,source.module+tree.reflection,native[4])||!cfgobserver::Same(r,source.module+tree.constructor,native[4]))return finish(false);
    NewWorldEvidence route;route.generation=armed.generation;route.catalogueGeneration=armed.catalogueGeneration;route.submission=submission;
    route.map=map.definition;route.jason=jason.type;route.selectedIndex=jason.index;route.resources=resources;
    route.selectedClass=armed.selectedClass;
    route.identity={fresh.plan.world,fresh.plan.pawn,fresh.plan.objects[0].serial,fresh.plan.objects[3].serial};
    route.cfgMetadata=route.ownedSelections=true;route.offlineSandbox=!source.nativeOffline;route.nativeOffline=source.nativeOffline;
    cfgbundle::Configuration configuration;why=Failure::Handoff;
    if(source.nativeOffline){
        // NativeMenuArrival's legacy overload deliberately defaults to Sandbox.
        // This explicit native seam repeats birth + accepted selection evidence.
        birth::Observation human;arrival::Trace trace;trace.expected=selection.counselorClass;
        if(!birth::Observe(r,fresh.plan,callerThread,true,human,nullptr,&trace.birth,true))return finish(false);
        route.humanCounselor=human.type;route.nativeCounselorBirthVerified=true;trace.actual=human.type;
        if(armed.counselorPathSelection){
            if(!classchoice::MatchesPathOrAncestor(r,source.module,human.type,native[1],armed.selectedCounselor)||
               !cfgobserver::ContextUnchanged(r,fresh.plan)||!classchoice::MatchesPathOrAncestor(r,source.module,human.type,native[1],armed.selectedCounselor))return finish(false);
            route.verifiedCounselorChoice=softpath::Token(armed.selectedCounselor);trace.relationship=5;
        }else if(selection.preserveNativeCounselor)trace.relationship=1;
        else if(human.type==selection.counselorClass)trace.relationship=2;
        else if(maps::Subclass(r,human.type,selection.counselorClass)&&cfgobserver::ContextUnchanged(r,fresh.plan)&&maps::Subclass(r,human.type,selection.counselorClass)){
            route.verifiedCounselorAncestor=selection.counselorClass;trace.relationship=3;
        }else return finish(false);
        if(handoffTrace)*handoffTrace=trace;
        if(!handoff.Consume(route,configuration))return finish(false);
    }else if(!arrival::Consume(r,handoff,route,fresh.plan,callerThread,true,configuration,nullptr,handoffTrace))return finish(false);
    output.plan=fresh.plan;output.configuration=configuration;output.resources=resources;output.mapVariant=map.variant;
    output.actualPackage=map.actualPackage;output.nativeOffline=source.nativeOffline;return finish(true);
}
}
