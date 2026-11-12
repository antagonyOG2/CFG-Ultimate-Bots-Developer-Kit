#pragma once
#include "../Core/SetupMenuGameplayHandoff.hpp"
#include "../RuntimeProbe/NativeCounselorBirthObservation.hpp"
namespace cfgmenu::arrival {
struct Trace {
    std::uintptr_t expected=0,actual=0;
    birth::Trace birth{};
    unsigned relationship=0; // 0=unproved,1=preserved,2=exact,3=derived,4=mismatch,5=accepted native asset path
};
// Staged one-shot NEW-world association, never a HUD maintenance task. Invoke
// only when the host has an independently prepared current gameplay Plan.
// Submission/map/Jason/catalogue identity still come from the armed route;
// human class and birth proof ALWAYS come from current native ownership, not
// caller-populated booleans. No possess/input/HUD/class modification occurs.
template<class Reader> bool Consume(Reader& reader,GameplayHandoff& handoff,const NewWorldEvidence& route,
    const cfgobserver::Plan& fresh,std::uint32_t callerThread,bool exactImageVerified,
    cfgbundle::Configuration& output,unsigned* consumed=nullptr,Trace* diagnostic=nullptr){
    output={};output.submission=0;if(consumed)*consumed=0;
    if(diagnostic)*diagnostic={};Trace trace;
    if(handoff.State()!=HandoffState::Armed)return false;
    MenuCatalogueEvidence source;Selection selected;std::uint64_t submission=0;
    if(!handoff.InspectArmed(source,selected,submission))return false;trace.expected=selected.counselorClass;
    auto candidate=route;candidate.nativeCounselorBirthVerified=false;candidate.humanCounselor=0;candidate.verifiedCounselorAncestor=0;candidate.verifiedCounselorChoice=0;
    const cfgspawn::Identity current{fresh.world,fresh.pawn,fresh.objects[0].serial,fresh.objects[3].serial};
    birth::Observation owned;
    if(route.identity==current&&birth::Observe(reader,fresh,callerThread,exactImageVerified,owned,consumed,&trace.birth)){
        candidate.humanCounselor=owned.type;candidate.nativeCounselorBirthVerified=true;
        trace.actual=owned.type;
        if(source.counselorPathSelection){std::uintptr_t base=0,ctor=0;
            if(reader.Read(fresh.module+cataloguedisk::SCCounselorCharacterClass,base)&&reader.Read(fresh.module+cataloguedisk::SCCounselorCharacterConstructor,ctor)&&
               cfgprobe::MatchingClassCache(base,ctor)&&classchoice::MatchesPathOrAncestor(reader,fresh.module,owned.type,base,source.selectedCounselor)&&
               cfgobserver::ContextUnchanged(reader,fresh)&&classchoice::MatchesPathOrAncestor(reader,fresh.module,owned.type,base,source.selectedCounselor)){
                candidate.verifiedCounselorChoice=softpath::Token(source.selectedCounselor);trace.relationship=5;
            }else trace.relationship=4;
        }
        else if(selected.preserveNativeCounselor)trace.relationship=1;
        else if(owned.type==selected.counselorClass)trace.relationship=2;
        else if(maps::Subclass(reader,owned.type,selected.counselorClass)&&cfgobserver::ContextUnchanged(reader,fresh)&&
                maps::Subclass(reader,owned.type,selected.counselorClass)){
            candidate.verifiedCounselorAncestor=selected.counselorClass;trace.relationship=3;
        }else trace.relationship=4;
    }
    if(diagnostic)*diagnostic=trace;
    // Failure consumes this arrival too: never silently bind a later world,
    // adopt an input-less replacement counselor or retry into a stock Jason.
    return handoff.Consume(candidate,output);
}
}
