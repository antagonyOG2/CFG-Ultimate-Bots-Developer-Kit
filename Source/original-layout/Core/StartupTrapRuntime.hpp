#pragma once
#include "../EventObserver/TrapNativeParameters.hpp"
#include <array>
// H2.43: maps with no cars/phone (e.g. Grendel). Opening traps go to the proven
// police-exit escape volumes instead. Compile out with CFG_H243_TRAPS_NOCAR=0.
#ifndef CFG_H243_TRAPS_NOCAR
#define CFG_H243_TRAPS_NOCAR 1
#endif
// H2.46: non-allowlisted verified maps run the SAME phone/car targets and
// Stand geometry as Crystal Lake Small (all derived from actor transforms; no
// map constants). The shack (cabin+door-component anchored) is kept wherever the
// survey finds it (absent on Grendel); police exits then fill traps left
// after phone/cars (all-exits when none exist). Compile out with CFG_H246_ALL_MAP_TRAPS=0.
#ifndef CFG_H246_ALL_MAP_TRAPS
#define CFG_H246_ALL_MAP_TRAPS 1
#endif
// H2.48: Grendel (no shack/phone/cars). Before police exits, anchor traps on the
// map's own SCEscapeVolume objectives (EscapeType Car1/Boat2/EscapePod3/GrendelBridge4,
// same Mode.EscapeVolumes list and offsets as the police exits). Compile out with
// CFG_H248_GRENDEL_TRAPS=0.
#ifndef CFG_H248_GRENDEL_TRAPS
#define CFG_H248_GRENDEL_TRAPS 1
#endif
namespace cfgtrap::startup {
enum class Kind : unsigned { Phone,Car2,Car4,Shack,Exit,GrendelEscape };
inline bool ExitLike(Kind k){return k==Kind::Exit||k==Kind::GrendelEscape;}
struct Anchor { Kind kind{};Vector position{},forward{}; };
enum class MorphResult : unsigned { Pending,Success,Unavailable,NotNeeded };
enum class Preparation : unsigned { Pending,Ready,Unavailable };
template<class A> auto OptionalPreparation(A& a,int)->decltype(a.Prepare()){return a.Prepare();}
template<class A> Preparation OptionalPreparation(A&,long){return Preparation::Ready;}
template<class A> auto TimedPreparation(A& a,std::uint64_t now,int)->decltype(a.Prepare(now)){return a.Prepare(now);}
template<class A> Preparation TimedPreparation(A& a,std::uint64_t,long){return OptionalPreparation(a,0);}
template<class A> auto ExpirePreparation(A& a,int)->decltype(a.ExpirePreparation()){return a.ExpirePreparation();}
template<class A> bool ExpirePreparation(A&,long){return false;}
template<class A> auto PreparationProgress(A& a,int)->decltype(a.PreparationProgress()){return a.PreparationProgress();}
template<class A> unsigned PreparationProgress(A&,long){return 0;}
constexpr std::uint64_t PreparationLimitMs=20000;
// Donor16s traversal plus the existing shared20s Morph wait. Starting a new
// objective immediately after Morph must not expire before that clock is ready.
constexpr std::uint64_t ObjectiveLimitMs=36000;
template<class A> auto ApproachMove(A& a,Vector v,unsigned& result,bool precise,int)->decltype(a.MovePrecise(v,result),bool{}){
    return precise?a.MovePrecise(v,result):a.Move(v,result);
}
template<class A> bool ApproachMove(A& a,Vector v,unsigned& result,bool,long){return a.Move(v,result);}
template<class A> auto OptionalMissingAnchor(A& a,unsigned i,int)->decltype(a.OptionalAnchorUnavailable(i)){return a.OptionalAnchorUnavailable(i);}
template<class A> bool OptionalMissingAnchor(A&,unsigned,long){return false;}
template<class A> auto FacingPending(A& a,int)->decltype(a.FacePending()){return a.FacePending();}
template<class A> bool FacingPending(A&,long){return false;}
template<class A> auto OptionalMorph(A& a,unsigned objective,const Anchor& anchor,Vector pawn,std::uint64_t now,int)
    ->decltype(a.ExteriorMorph(objective,anchor,pawn,now)){return a.ExteriorMorph(objective,anchor,pawn,now);}
template<class A> MorphResult OptionalMorph(A&,unsigned,const Anchor&,Vector,std::uint64_t,long){return MorphResult::NotNeeded;}
template<class A> auto StalledMorph(A& a,unsigned i,const Anchor& anchor,Vector pawn,std::uint64_t now,int)
    ->decltype(a.StalledMorph(i,anchor,pawn,now)){return a.StalledMorph(i,anchor,pawn,now);}
template<class A> MorphResult StalledMorph(A& a,unsigned i,const Anchor& anchor,Vector pawn,std::uint64_t now,long){return OptionalMorph(a,i,anchor,pawn,now,0);}
template<class A> auto PlacementRetry(A& a,unsigned i,const Anchor& anchor,Vector pawn,unsigned retry,int)
    ->decltype(a.RepositionCarRetry(i,anchor,pawn,retry)){return a.RepositionCarRetry(i,anchor,pawn,retry);}
template<class A> MorphResult PlacementRetry(A&,unsigned,const Anchor&,Vector,unsigned,long){return MorphResult::NotNeeded;}
inline bool Direction(Vector v){const auto n=v.x*v.x+v.y*v.y;return Point(v)&&std::isfinite(n)&&n>.98f&&n<1.02f&&std::fabs(v.z)<.1f;}
// Fallback map: exact current arrival proved a world/package that is NOT the
// allowlisted ground map. Only police-exit volumes are used there (never the
// ground-map calibrated phone/car/shack geometry).
inline bool NoCarFallbackMap(MapFamily family,bool verified,bool packagePresent){
    return CFG_H243_TRAPS_NOCAR&&verified&&packagePresent&&family==MapFamily::Unknown;
}
inline MapFamily EffectiveFamily(MapFamily family,bool fallback){return fallback?MapFamily::GroundMap:family;}
// Exits fill only traps left over after phone/cars, within the objective capacity.
inline unsigned ExitSlots(int inventory,unsigned have,unsigned capacity){
    if(inventory<=0||have>=capacity)return 0;const auto left=static_cast<unsigned>(inventory)>have?static_cast<unsigned>(inventory)-have:0u;
    return left<capacity-have?left:capacity-have;
}
inline Vector Stand(const Anchor& a,unsigned retry){
    if(ExitLike(a.kind)){constexpr float distance[]{260,300,220,340};
        return {a.position.x+a.forward.x*distance[retry%4],a.position.y+a.forward.y*distance[retry%4],a.position.z};}
    if(a.kind==Kind::Shack){const float distance=90.f+20.f*static_cast<float>(retry/3<3?retry/3:3);constexpr float side[]{0,30,-30};
        return {a.position.x+a.forward.x*distance-a.forward.y*side[retry%3],
            a.position.y+a.forward.y*distance+a.forward.x*side[retry%3],a.position.z};}
    if(a.kind==Kind::Phone){constexpr float distance[]{155,180,205,230},lateral[]{0,24,-24,48};
        return {a.position.x-a.forward.y*distance[retry%4]+a.forward.x*lateral[retry%4],
            a.position.y+a.forward.x*distance[retry%4]+a.forward.y*lateral[retry%4],a.position.z};}
    constexpr float two[]{315,340,300,355},four[]{300,315,340,340};
    const auto d=a.kind==Kind::Car2?two[retry%4]:four[retry%4];
    return {a.position.x+a.forward.x*d,a.position.y+a.forward.y*d,a.position.z};
}
inline bool ExteriorStagingNeeded(const Anchor& a,Vector pawn){
    const auto target=a.kind==Kind::Shack?Stand(a,0):a.position;
    const float dx=pawn.x-target.x,dy=pawn.y-target.y;
    return a.kind==Kind::Shack?(dx*dx+dy*dy>40.f*40.f||std::fabs(pawn.z-target.z)>180.f):dx*dx+dy*dy>2500.f*2500.f;
}
// Donor TryTeleportCandidate retains the authored shack stand when nav
// projection cannot produce a nearby point. Native teleport collision and
// CanPlaceTrap still decide acceptance; this does not mark a trap completed.
inline bool ShackStaging(const Anchor& a,unsigned candidate,bool accepted,Vector projected,Vector& destination,bool& direct){
    destination={};direct=false;
    if(a.kind!=Kind::Shack||candidate>=12||!Point(a.position)||!Direction(a.forward))return false;
    const auto stand=Stand(a,candidate);
    const auto dx=projected.x-stand.x,dy=projected.y-stand.y;
    if(accepted&&Point(projected)&&dx*dx+dy*dy<=45.f*45.f&&std::fabs(projected.z-a.position.z)<=120.f){
        destination=projected;destination.z+=90.f;
    }else {
        // CFG's typed threshold is ground/door-component height, whereas a
        // teleport destination is the actor's capsule centre. The successful
        // projected path already adds90cm; the direct fallback must apply the
        // same coordinate adapter or it embeds the capsule in the terrain.
        // Preserve authored X/Y and native collision/CanPlaceTrap authority.
        destination=stand;destination.z+=90.f;direct=true;
    }
    return Point(destination);
}
inline bool Legal(const Anchor& a,const CanPlaceParameters& p,unsigned retry){
    if(!PlacementObserved(p)||!Point(a.position)||!Direction(a.forward))return false;
    const auto dx=p.impact.x-a.position.x,dy=p.impact.y-a.position.y;
    // Native CanPlaceTrap owns collision/ground legality; this extra test owns
    // objective proximity. Base placement bands are policy, not CFG ABI facts.
    if(std::fabs(p.impact.z-a.position.z)>(ExitLike(a.kind)?300.f:140.f))return false;
    const auto front=dx*a.forward.x+dy*a.forward.y,side=-dx*a.forward.y+dy*a.forward.x;
    if(a.kind==Kind::Shack)return std::fabs(front)<=100.f&&std::fabs(side)<=85.f;
    // Exit volume centre is a trigger centre, not an object face. Policy band
    // only (native CanPlaceTrap owns collision/ground legality): the trap must
    // land on the approach side, within the volume's vicinity.
    if(ExitLike(a.kind))return front>=40.f&&front<=420.f&&std::fabs(side)<=220.f;
    if(a.kind==Kind::Phone)return dx*dx+dy*dy>=85.f*85.f&&dx*dx+dy*dy<=125.f*125.f&&std::fabs(front)<=85.f&&side>0;
    const auto maxFront=a.kind==Kind::Car2?290.f:(retry>=2?340.f:315.f);
    return front>=135.f&&front<=maxFront&&std::fabs(side)<=(a.kind==Kind::Car2?105.f:115.f);
}
enum class Stage : unsigned { Dormant,Discover,Travel,Stop,Face,Probe,Attempt,Confirm,Hold,Complete,Refused,Indeterminate,ApproachFace,ApproachProbe,ApproachStop };
enum class Failure : unsigned { None,Map,Identity,Source,Call,Count,Geometry,Timeout,Unconfirmed };
struct Report {Stage stage=Stage::Dormant;Failure failure=Failure::None;unsigned objective=0,objectives=0,retry=0,attempts=0,consumed=0,skipped=0,probes=0;int before=-1,lastCount=-1;
    std::array<unsigned,4> outcome{},lastStage{},objectiveProbes{};unsigned currentKind=0;Vector lastImpact{};
    std::array<Vector,4> lastPawn{},lastStand{};
    std::array<unsigned,4> travelFailures{},lastMove{},lastMorph{},kinds{},nativePlacement{},legalPlacement{},stops{};
    std::array<Vector,4> impacts{};};
// Host adapter below implements all methods with native bindings. One Step is
// one owner-thread slice. While Busy, the integrator must not issue competing
// hunt, Morph, attack, door or vehicle commands; owed input release goes first.
class Runtime {
    std::uint64_t next_=0,deadline_=0,attemptAt_=0,holdAt_=0,stableAt_=0,preparationDeadline_=0;
    unsigned stable_=0,retryLimit_=4,preparationProgress_=0;Vector stationary_{};
    bool approachProbed_=false,preparationServiced_=false,travelObserved_=false;
    Vector travelPosition_{};std::uint64_t travelProgressAt_=0;
    bool paused_=false;std::uint64_t pausedAt_=0;
    void Next(std::uint64_t now){++report.objective;report.retry=0;stable_=0;approachProbed_=false;travelObserved_=false;deadline_=now+ObjectiveLimitMs;next_=now+1200;report.stage=Stage::Travel;}
    void Retry(std::uint64_t now){stable_=0;approachProbed_=false;if(++report.retry>=retryLimit_){++report.skipped;report.outcome[report.objective]=3;Next(now);}else{report.stage=Stage::Travel;next_=now+1500;}}
    bool Fail(Failure f,bool uncertain=false){report.failure=f;report.stage=uncertain?Stage::Indeterminate:Stage::Refused;return false;}
public:
    bool Due(std::uint64_t now)const{return Busy()&&!paused_&&now>=next_;}
    bool Paused()const{return paused_;}
    bool CanSuspend()const{return Busy()&&report.stage!=Stage::Attempt&&report.stage!=Stage::Confirm&&report.stage!=Stage::Hold;}
    bool PauseForVehicle(std::uint64_t now){if(!CanSuspend())return false;if(!paused_){paused_=true;pausedAt_=now;}return true;}
    bool Resume(std::uint64_t now){
        if(!paused_)return true;
        if(now<pausedAt_||now>UINT64_MAX-ObjectiveLimitMs)return Fail(Failure::Timeout);
        const auto delay=now-pausedAt_;
        if(deadline_&&deadline_>UINT64_MAX-delay)return Fail(Failure::Timeout);
        if(preparationDeadline_&&preparationDeadline_>UINT64_MAX-delay)return Fail(Failure::Timeout);
        if(deadline_)deadline_+=delay;if(preparationDeadline_)preparationDeadline_+=delay;
        if(report.stage!=Stage::Discover)report.stage=Stage::Travel;
        paused_=false;approachProbed_=false;travelObserved_=false;stable_=0;next_=now;return true;
    }
    Report report{};
    bool Busy()const{return (report.stage>=Stage::Discover&&report.stage<=Stage::Hold)||report.stage==Stage::ApproachFace||report.stage==Stage::ApproachProbe||report.stage==Stage::ApproachStop;}
    bool Start(std::uint64_t now,MapFamily family,bool verified){
        if(report.stage!=Stage::Dormant||!MapAllowed(family,verified)||now>UINT64_MAX-200000)return Fail(Failure::Map);
        report.stage=Stage::Discover;next_=now+5000;preparationDeadline_=now+PreparationLimitMs;return true;
    }
    template<class A> bool Step(A& a,std::uint64_t now){
        if(!Busy())return false;
        if(paused_)return true; // no native command while occupied-car priority owns motion
        if(!a.Ready())return Fail(Failure::Identity,report.stage==Stage::Confirm||report.stage==Stage::Hold);
        // Settings/helper startup may defer this lane beyond Start()+20s.
        // Bound actual preparation from its first serviced callback, never
        // expire before even one mandatory binding has been warmed.
        if(report.stage==Stage::Discover&&!preparationServiced_&&now>=next_){preparationServiced_=true;preparationDeadline_=now+PreparationLimitMs;}
        if(report.stage==Stage::Discover){
            const auto progress=PreparationProgress(a,0);
            if(progress>12||progress<preparationProgress_)return Fail(Failure::Identity);
            // A completed required binding is genuine finite progress, unlike
            // retrying the same pending survey. Do not time out between cold
            // binding phases because an intro/pause delayed the next callback.
            if(progress>preparationProgress_){
                if(now>UINT64_MAX-PreparationLimitMs)return Fail(Failure::Timeout);
                preparationProgress_=progress;preparationDeadline_=now+PreparationLimitMs;
            }
        }
        const bool preparationExpired=report.stage==Stage::Discover&&preparationServiced_&&now>=preparationDeadline_;
        if(now<next_&&!preparationExpired)return true;
        next_=now+263;
        if(report.stage==Stage::Discover){
            // Required bindings cannot be bypassed. An optional source survey
            // may be abandoned without discarding already available phone/cars.
            if(preparationExpired&&!ExpirePreparation(a,0))return Fail(Failure::Timeout);
            const auto prepared=preparationExpired?Preparation::Ready:TimedPreparation(a,now,0);
            if(prepared==Preparation::Pending)return true;
            if(prepared!=Preparation::Ready)return Fail(Failure::Call);
            if(!a.Discover(report.objectives)||report.objectives>4)return Fail(Failure::Source);
            deadline_=now+ObjectiveLimitMs;report.stage=Stage::Travel;return true;
        }
        int count=-1;if(!a.Count(count)||count<0||count>32)return Fail(Failure::Count,report.stage==Stage::Confirm||report.stage==Stage::Hold);
        report.lastCount=count;
        // Observe pending consumption BEFORE the zero-inventory exit. Never
        // claim a count decrease proves a visible trap actor was created.
        if(report.stage==Stage::Confirm){
            if(count==report.before-1){++report.consumed;report.outcome[report.objective]=1;report.failure=Failure::None;holdAt_=now;report.stage=Stage::Hold;return true;}
            if(count!=report.before)return Fail(Failure::Count,true);
            if(now-attemptAt_>=5000)return Fail(Failure::Unconfirmed,true); // no duplicate after uncertain animation
            return true;
        }
        if(report.stage==Stage::Hold){
            if(count!=report.before-1)return Fail(Failure::Count,true);
            if(now-holdAt_>=5000)Next(now);
            return true;
        }
        if(!count||report.objective>=report.objectives){report.stage=Stage::Complete;return false;}
        report.lastStage[report.objective]=static_cast<unsigned>(report.stage);
        if(now>=deadline_){++report.skipped;report.outcome[report.objective]=2;Next(now);return true;}
        Anchor anchor{};Vector pawn{};
        if(!a.AnchorAt(report.objective,anchor)){
            // No placement is pending here: Confirm/Hold drain above. An
            // optional shack source disappearing must not cancel phone/cars.
            if(OptionalMissingAnchor(a,report.objective,0)){++report.skipped;report.outcome[report.objective]=4;Next(now);return true;}
            return Fail(Failure::Source);
        }
        if(!Point(anchor.position)||!Direction(anchor.forward)||!a.Position(pawn)||!Point(pawn))return Fail(Failure::Source);
        report.currentKind=static_cast<unsigned>(anchor.kind);retryLimit_=anchor.kind==Kind::Shack?12u:4u;
        report.kinds[report.objective]=report.currentKind;
        report.lastPawn[report.objective]=pawn;report.lastStand[report.objective]=Stand(anchor,report.retry);
        // The donor stops its approach path before facing/CanPlaceTrap. An
        // accepted MoveTo is asynchronous: facing while it is still active
        // lets path following overwrite rotation between the two native calls.
        if(report.stage==Stage::ApproachStop){
            if(!a.Stop())return Fail(Failure::Call);
            ++report.stops[report.objective];stationary_=pawn;report.stage=Stage::ApproachFace;next_=now+650;return true;
        }
        if(report.stage==Stage::ApproachFace){if(!a.Face(pawn,anchor.position))return Fail(Failure::Call);if(!FacingPending(a,0))report.stage=Stage::ApproachProbe;return true;}
        if(report.stage==Stage::ApproachProbe){CanPlaceParameters p{};++report.probes;++report.objectiveProbes[report.objective];
            if(!a.Probe(p))return Fail(Failure::Call);report.lastImpact=p.impact;
            report.impacts[report.objective]=p.impact;report.nativePlacement[report.objective]=PlacementObserved(p)?1u:0u;
            report.legalPlacement[report.objective]=Legal(anchor,p,report.retry)?1u:0u;
            if(Legal(anchor,p,report.retry)){
                stationary_=pawn;stableAt_=now;stable_=0;report.stage=Stage::Face;next_=now+650;
            }else {approachProbed_=true;report.stage=Stage::Travel;}
            return true;
        }
        if(report.stage==Stage::Travel){
            // The donor actually repositions to each distinct front stand
            // after a rejected placement. A tiny MoveTo can otherwise report
            // AlreadyAtGoal at the old stand and never test the next geometry.
            const auto reposition=PlacementRetry(a,report.objective,anchor,pawn,report.retry,0);
            if(reposition==MorphResult::Pending)return true;
            if(reposition==MorphResult::Success){travelObserved_=false;next_=now+900;return true;}
            const float px=pawn.x-travelPosition_.x,py=pawn.y-travelPosition_.y;
            if(!travelObserved_||px*px+py*py>=80.f*80.f){travelObserved_=true;travelPosition_=pawn;travelProgressAt_=now;}
            const bool stalled=now-travelProgressAt_>=7000;
            // An accepted partial path is not proof of physical progress.
            // Reuse the same guarded, once-per-objective exterior staging;
            // permit its unspent attempt locally only after seven stalled seconds.
            const auto morph=stalled?StalledMorph(a,report.objective,anchor,pawn,now,0):OptionalMorph(a,report.objective,anchor,pawn,now,0);
            report.lastMorph[report.objective]=static_cast<unsigned>(morph);
            if(morph==MorphResult::Pending)return true; // projection and teleport cannot share a slice
            if(morph==MorphResult::Success){travelObserved_=false;next_=now+900;return true;}
            const auto stand=Stand(anchor,report.retry);const auto dx=pawn.x-stand.x,dy=pawn.y-stand.y;
            const auto ax=pawn.x-anchor.position.x,ay=pawn.y-anchor.position.y;
            if(!approachProbed_&&ax*ax+ay*ay<450.f*450.f){report.stage=Stage::ApproachStop;return true;}
            approachProbed_=false;
            if(dx*dx+dy*dy<=40.f*40.f&&std::fabs(pawn.z-anchor.position.z)<180.f){report.stage=Stage::Stop;return true;}
            unsigned result=255;const bool precise=ax*ax+ay*ay<600.f*600.f;
            if(!ApproachMove(a,stand,result,precise,0)||result>2)return Fail(Failure::Call);
            report.lastMove[report.objective]=result;
            // Native AlreadyAtGoal owns reachability, not our floating-point
            // 40cm edge (ABI52 car sample was just outside that edge). Stop and
            // test native legality instead of looping approach/path forever.
            // Keep the bounded objective vicinity and every placement guard.
            if(result==1&&precise){report.stage=Stage::Stop;return true;}
            // Travel failure is not a failed placement. In particular Morph
            // may still be charging: retain this objective through its finite
            // deadline instead of burning all four placement retries in6s.
            if(!result){++report.travelFailures[report.objective];next_=now+1500;}
            else next_=now+(precise?263:2300);
            return true;
        }
        if(report.stage==Stage::Stop){if(!a.Stop())return Fail(Failure::Call);++report.stops[report.objective];stationary_=pawn;stableAt_=now;stable_=0;report.stage=Stage::Face;next_=now+650;return true;}
        if(report.stage==Stage::Face){if(!a.Face(pawn,anchor.position))return Fail(Failure::Call);if(!FacingPending(a,0))report.stage=Stage::Probe;return true;}
        const auto dx=pawn.x-stationary_.x,dy=pawn.y-stationary_.y,dz=pawn.z-stationary_.z;
        // Donor's stable-sample approach is PHONE-specific. Capsule settling
        // and facing at a car must not keep cancelling a legal native point.
        // Cars/shack still repeat current native legality before Attempt.
        if(anchor.kind==Kind::Phone&&dx*dx+dy*dy+dz*dz>20.f*20.f){Retry(now);return true;}
        CanPlaceParameters p{};++report.probes;++report.objectiveProbes[report.objective];if(!a.Probe(p))return Fail(Failure::Call);
        report.lastImpact=p.impact;
        report.impacts[report.objective]=p.impact;report.nativePlacement[report.objective]=PlacementObserved(p)?1u:0u;
        report.legalPlacement[report.objective]=Legal(anchor,p,report.retry)?1u:0u;
        if(!Legal(anchor,p,report.retry)){report.failure=Failure::Geometry;Retry(now);return true;}
        if(report.stage==Stage::Probe){
            if(anchor.kind!=Kind::Phone||(++stable_>=3&&now-stableAt_>=300)){report.stage=Stage::Attempt;next_=now+250;}
            return true;
        }
        if(report.stage!=Stage::Attempt)return Fail(Failure::Call);
        int current=-1;if(!a.Count(current)||current!=count||current<=0)return Fail(Failure::Count);
        // Publish before the side-effecting call. Probe was repeated in this
        // slice; AttemptPlaceTrap also performs its own native legality query.
        report.before=count;attemptAt_=now;report.stage=Stage::Confirm;++report.attempts;
        if(!a.Attempt())return Fail(Failure::Call,true);
        next_=now+700;return true;
    }
};
}
