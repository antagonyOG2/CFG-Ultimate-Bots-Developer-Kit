#pragma once
#include "BundleObservations.hpp"
#include "JasonInstanceObservation.hpp"
#include "../Core/StartupTrapRuntime.hpp"
#include "../Core/VehiclePursuitPolicy.hpp"
#include "../RuntimeProbe/StartupTrapBindings.generated.hpp"
#include "ShackTrapSource.hpp"
#include "PoliceExitObservation.hpp"
#include "VehicleRosterSource.hpp"
namespace cfgtrap::startup {
struct Inputs {
    std::uintptr_t module=0,world=0,mode=0,gameState=0,jason=0,controller=0;
    cfgprobe::lifetime::SlotObservation jasonSlot{},controllerSlot{};
    MapFamily family=MapFamily::Unknown;
    bool verifiedMap=false; // from exact current arrival/package proof, never label substring
    bool fallbackMap=false; // H2.43: non-allowlisted verified map; police-exit volumes only
};
struct Source {Kind kind{};std::uintptr_t actor=0,component=0;cfgprobe::lifetime::SlotObservation actorSlot{},componentSlot{};unsigned exitIndex=0;Vector exitForward{};};
struct MorphState {
    bool projected=false,tried=false,nativeAccepted=false,arrivalVerified=false,directFallback=false,stopped=false;
    unsigned projectionCandidate=0;
    unsigned localRetryAttempted=0;
    Anchor anchor{};Vector destination{},actual{};
};
struct Sources {std::array<Source,4> entries{};std::array<MorphState,4> morph{};unsigned count=0,prepared=0,optionalSlices=0;std::uint64_t optionalStarted=0;bool discovered=false,shackReady=false,optionalBegun=false,facePending=false,faceBodyAccepted=false;Vector pendingFace{},pendingFacePosition{},pendingFaceTarget{};cfgshacktrap::Survey shack;};
struct TeleportParameters {Vector destination{},rotation{};std::uint8_t result=0,padding[3]{};};
static_assert(sizeof(TeleportParameters)==28&&offsetof(TeleportParameters,result)==24);
// H is a thin BundleAccess bridge: Context, Jason, FeatureFunction, Invoke,
// Location plus public context.objectArray and TrapJason()/TrapController().
// R is the SAME bounded reader used by H, not an unmetered extra reader.
template<class R,class H> class NativeAccess {
    R& r_;H& h_;Inputs in_;Sources& sources_;bool ready_=false;
    template<class T> static auto Exception(T& h,int)->decltype(h.Exception()){return h.Exception();}
    template<class T> static bool Exception(T&,long){return false;}
    template<class T> static auto MorphDue(T& h,std::uint64_t now,int)->decltype(h.TrapMorphDue(now),h.TrapMorphCompleted(now),bool{}){return h.TrapMorphDue(now);}
    template<class T> static bool MorphDue(T&,std::uint64_t,long){return false;}
    template<class T> static auto MorphUsed(T& h,std::uint64_t now,int)->decltype(h.TrapMorphCompleted(now),void()){h.TrapMorphCompleted(now);}
    template<class T> static void MorphUsed(T&,std::uint64_t,long){}
    // Callback-local validation credit: known pure actor transform getters do
    // not invalidate full Context/Jason proof. Any factory or side-effecting
    // call consumes it BEFORE dispatch. Never retain this adapter across ticks.
    bool Guard(){return !Exception(h_,0)&&(ready_?CurrentLinks():Ready());}
    MapFamily Fam()const{return EffectiveFamily(in_.family,in_.fallbackMap);}
    // H2.43: same proven police-exit volume reads as cfgpoliceexit::Step
    // (ModeVolumes list, EscapeClass, EscapeType==0, sphere pairing, owning
    // world) but available at match start, before police are called.
    bool ExitList(cfgpoliceexit::List& list){return r_.Read(in_.mode+cfgpoliceexit::disk::ModeVolumes,list)&&list.Sane();}
    bool ExitFresh(const Source& s){
        cfgpoliceexit::List list{};cfgpoliceexit::Entry e{};e.volume=s.actorSlot;e.sphere=s.componentSlot;e.sourceIndex=s.exitIndex;
        return ExitLike(s.kind)&&ExitList(list)&&
            cfgpoliceexit::Current(r_,h_.context.objectArray,in_.module,in_.world,in_.mode,list,e,s.kind==Kind::GrendelEscape);
    }
    // Up to three exits, nearest to Jason first, each with a fixed approach
    // direction (exit toward Jason's start) so Stand/Legal are deterministic.
    bool DiscoverExits(unsigned limit=3,bool objective=false){
        cfgpoliceexit::List list{};Vector here{};
        if(!ExitList(list)||list.count<=0||!Position(here))return false;
        struct Found{Source s;float d2;};std::array<Found,16> found{};unsigned n=0;
        for(int i=0;i<list.count&&n<found.size();++i){
            std::uintptr_t actor=0,type=0,sphere=0;std::uint8_t kind=255;cfgpoliceexit::Entry e{};e.sourceIndex=static_cast<unsigned>(i);
            if(!r_.Read(list.data+8ULL*static_cast<unsigned>(i),actor)||!actor||!cfgpoliceexit::Pair(r_,in_.module,cfgpoliceexit::disk::EscapeClassReflection,type)||
               !cfgspawn::ObjectIs(r_,actor,type)||!r_.Read(actor+cfgpoliceexit::disk::EscapeType,kind)||(objective?(kind<1||kind>4):kind!=0))continue;
            if(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,actor,e.volume)||!r_.Read(actor+cfgpoliceexit::disk::EscapeSphere,sphere)||
               !cfgspawn::ObjectSlot(r_,h_.context.objectArray,sphere,e.sphere)||
               !cfgpoliceexit::Current(r_,h_.context.objectArray,in_.module,in_.world,in_.mode,list,e,objective))continue;
            Vector at{};if(!Location(actor,at)||!Point(at))continue;
            const auto dx=here.x-at.x,dy=here.y-at.y;const auto n2=dx*dx+dy*dy;
            if(!(n2>100.f*100.f)||!std::isfinite(n2))continue; // no defined approach side
            const auto inv=1.f/std::sqrt(n2);Vector fwd{dx*inv,dy*inv,0};if(!Direction(fwd))continue;
            bool dup=false;for(unsigned k=0;k<n;++k)dup=dup||found[k].s.actor==actor;if(dup)continue;
            Source s;s.kind=objective?Kind::GrendelEscape:Kind::Exit;s.actor=actor;s.actorSlot=e.volume;s.component=sphere;s.componentSlot=e.sphere;s.exitIndex=e.sourceIndex;s.exitForward=fwd;
            found[n++]={s,n2};
        }
        for(unsigned i=1;i<n;++i)for(unsigned j=i;j>0&&found[j].d2<found[j-1].d2;--j)std::swap(found[j],found[j-1]);
        for(unsigned i=0;i<n&&sources_.count<limit;++i)sources_.entries[sources_.count++]=found[i].s;
        return sources_.count>0;
    }
    bool Location(std::uintptr_t actor,Vector& p){
        if(!Guard()||!h_.Location(actor,p)||!Point(p)){ready_=false;return false;}return true;
    }
    bool CurrentLinks(){
        return MapAllowed(Fam(),in_.verifiedMap)&&h_.context.world==in_.world&&h_.context.mode==in_.mode&&
            h_.TrapJason()==in_.jason&&h_.TrapController()==in_.controller&&
            cfgjason::SameSlot(r_,h_.context.objectArray,in_.jasonSlot)&&in_.jasonSlot.object==in_.jason&&
            cfgjason::SameSlot(r_,h_.context.objectArray,in_.controllerSlot)&&in_.controllerSlot.object==in_.controller&&
            cfgobserver::Same(r_,in_.world+cfgprobe::WorldGameState,in_.gameState);
    }
    bool Class(std::uintptr_t reflection,std::uintptr_t ctor,std::uintptr_t& out){
        std::uintptr_t a=0,b=0;cfgprobe::lifetime::SlotObservation s{};
        return r_.Read(in_.module+reflection,a)&&r_.Read(in_.module+ctor,b)&&(out=cfgprobe::MatchingClassCache(a,b))&&cfgspawn::ObjectSlot(r_,h_.context.objectArray,out,s);
    }
    bool Typed(std::uintptr_t object,std::uintptr_t reflection,std::uintptr_t ctor){std::uintptr_t type=0;return Class(reflection,ctor,type)&&cfgspawn::ObjectIs(r_,object,type);}
    bool ReadSource(Kind kind,std::uintptr_t actor,Source& out){
        out={};out.kind=kind;out.actor=actor;const bool phone=kind==Kind::Phone;
        if(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,actor,out.actorSlot)||!out.actorSlot.HasAssignedSerial()||
           !Typed(actor,phone?disk::SCPhoneJunctionBoxReflection:disk::SCDriveableVehicleReflection,phone?disk::SCPhoneJunctionBoxConstructor:disk::SCDriveableVehicleConstructor)||
           !r_.Read(actor+(phone?disk::InteractComponent:disk::JasonInteractComponent),out.component)||
           !cfgspawn::ObjectSlot(r_,h_.context.objectArray,out.component,out.componentSlot)||
           !Typed(out.component,disk::SCInteractComponentReflection,disk::SCInteractComponentConstructor)||
           !cfgobserver::Same(r_,out.component+0x20,actor)||!cfgobserver::Same(r_,out.component+0xE0,actor))return false;
        if(!phone&&!cfgobserver::Same(r_,actor+cfgvehicle::disk::Driver,std::uintptr_t{}))return false;
        cfgprobe::lifetime::SlotObservation a{},c{};
        return cfgspawn::ObjectSlot(r_,h_.context.objectArray,actor,a)&&a==out.actorSlot&&
            cfgspawn::ObjectSlot(r_,h_.context.objectArray,out.component,c)&&c==out.componentSlot&&
            cfgobserver::Same(r_,actor+(phone?disk::InteractComponent:disk::JasonInteractComponent),out.component);
    }
    template<std::size_t N> bool Call(const cfgvehicle::disk::Binding& binding,std::uintptr_t exec,const std::array<unsigned char,N>& code,std::uintptr_t receiver,void* packet,bool actor=true,bool pureTransform=false){
        std::uintptr_t fn=0,owner=0,cached=0;
        // Runtime::Step has already refreshed the complete owner context. A
        // cold factory is itself a native call, so refresh before it as well.
        if(!Guard()||!r_.Read(in_.module+binding.cache,cached))return false;
        if(!cached)ready_=false; // FeatureFunction may invoke the pinned factory
        if(!h_.FeatureFunction(binding,fn,owner)||!cfgspawn::ObjectIs(r_,receiver,owner)||
           !cfgobserver::Same(r_,fn+0xB0,in_.module+exec)||!cfgbundle::Code(r_,in_.module,exec,code)||!Guard())return false;
        if(!pureTransform)ready_=false;
        if(!h_.Invoke(receiver,fn,packet,actor)){ready_=false;return false;}return true;
    }
    bool Phone(std::uintptr_t& phone){
        // Independently named CFG SCGameState.Phone is a junction-box object,
        // not the interior police handset. No mode array/global census needed.
        return r_.Read(in_.gameState+disk::Phone,phone)&&cfgobserver::Same(r_,in_.gameState+disk::Phone,phone);
    }
    template<std::size_t N> bool Warm(const cfgvehicle::disk::Binding& b,std::uintptr_t exec,const std::array<unsigned char,N>& code){
        std::uintptr_t fn=0,owner=0,cached=0;
        if(!Guard()||!r_.Read(in_.module+b.cache,cached))return false;if(!cached)ready_=false;
        return h_.FeatureFunction(b,fn,owner)&&Guard()&&cfgobserver::Same(r_,fn+0xB0,in_.module+exec)&&cfgbundle::Code(r_,in_.module,exec,code);
    }
public:
    NativeAccess(R& r,H& h,const Inputs& in,Sources& sources):r_(r),h_(h),in_(in),sources_(sources){}
    bool Ready(){
        ready_=false;return ready_=!Exception(h_,0)&&MapAllowed(Fam(),in_.verifiedMap)&&h_.Context()&&h_.Jason()&&CurrentLinks();
    }
    bool ExpirePreparation(){
        if(sources_.prepared<9)return false;
        if(sources_.prepared<12){sources_.shack.Skip(10);sources_.shackReady=false;sources_.prepared=12;}
        return true;
    }
    unsigned PreparationProgress()const{return sources_.prepared;}
    Preparation Prepare(std::uint64_t now=0){
        if(sources_.prepared>=9&&sources_.prepared<12){
            if(!sources_.optionalBegun){sources_.optionalBegun=true;sources_.optionalStarted=now;}
            // Includes actor survey, reflected door lookup and anchor proof.
            // Callback starvation/array churn cannot extend this allowance.
            if(now-sources_.optionalStarted>=8000||sources_.optionalSlices>=32){ExpirePreparation();return Preparation::Ready;}
            ++sources_.optionalSlices;
        }
        bool ok=false;
        // One pinned binding/factory per callback; never batch cold native
        // factories into the placement or double-facing phase.
        switch(sources_.prepared){
        case 0:ok=Warm(disk::CanPlaceTrap,disk::CanPlaceTrapExecutor,disk::CanPlaceTrapCode);break;
        case 1:ok=Warm(disk::AttemptPlaceTrap,disk::AttemptPlaceTrapExecutor,disk::AttemptPlaceTrapCode);break;
        case 2:ok=Warm(disk::StopMovement,disk::StopMovementExecutor,disk::StopMovementCode);break;
        case 3:ok=Warm(disk::MoveToLocation,disk::MoveToLocationExecutor,disk::MoveToLocationCode);break;
        case 4:ok=Warm(disk::K2_SetActorRotation,disk::K2_SetActorRotationExecutor,disk::K2_SetActorRotationCode);break;
        case 5:ok=Warm(disk::SetControlRotation,disk::SetControlRotationExecutor,disk::SetControlRotationCode);break;
        case 6:ok=Warm(disk::GetActorForwardVector,disk::GetActorForwardVectorExecutor,disk::GetActorForwardVectorCode);break;
        case 7:ok=Warm(disk::K2_ProjectPointToNavigation,disk::K2_ProjectPointToNavigationExecutor,disk::K2_ProjectPointToNavigationCode);break;
        case 8:ok=Warm(disk::K2_TeleportTo,disk::K2_TeleportToExecutor,disk::K2_TeleportToCode);break;
        case 9:
            
            if(!Guard())return Preparation::Unavailable;
            sources_.shack.Step(r_,h_.context.objectArray,in_.module,in_.world);
            if(sources_.shack.Done())++sources_.prepared;
            return Preparation::Pending;
        case 10:{
            if(sources_.shack.stage==2){const auto status=sources_.shack.WarmProperty(r_,h_.context.objectArray);
                if(!status)return Preparation::Pending;
                if(status==2)sources_.shack.Skip(9);}
            ++sources_.prepared;return Preparation::Pending;}
        case 11:{
            if(sources_.shack.stage==2){Vector cabin{},door{},forward{};
                sources_.shackReady=Location(sources_.shack.actor,cabin)&&sources_.shack.Position(r_,h_.context.objectArray,cabin,door,forward);
                if(!sources_.shackReady)sources_.shack.Skip(8);}
            ++sources_.prepared;return Preparation::Pending;}
        default:return Preparation::Ready;
        }
        if(!ok)return Preparation::Unavailable;++sources_.prepared;return Preparation::Pending;
    }
    bool Discover(unsigned& count){
        count=0;if(sources_.discovered||!Guard()||!Typed(in_.mode,disk::SCGameModeReflection,disk::SCGameModeConstructor)||
            !Typed(in_.gameState,disk::SCGameStateReflection,disk::SCGameStateConstructor))return false;
        if(in_.fallbackMap&&!CFG_H246_ALL_MAP_TRAPS){
            // H2.43 behaviour: exits only on non-allowlisted verified maps.
            if(!DiscoverExits())return false;
            return FinishDiscover(count);
        }
        std::uintptr_t phone=0,car2=0,car4=0;
        const bool readOk=Phone(phone)&&cfgvehicle::RosterCar(r_,h_.context.objectArray,in_.module,in_.world,in_.mode,in_.gameState,disk::Car2SeatVehicle,car2)&&
           cfgvehicle::RosterCar(r_,h_.context.objectArray,in_.module,in_.world,in_.mode,in_.gameState,disk::Car4SeatVehicle,car4)&&
           !(car2&&car2==car4);
        if(!readOk){if(!in_.fallbackMap)return false;phone=car2=car4=0;} // fallback maps: skip unreadable kinds, never fail the phase
        const std::uintptr_t actors[]{phone,car2,car4};
        // Shack is anchored to the IsJasonShack cabin actor + its reflected door component
        // (Survey proves both on the live world), so it is kept on every map where found.
        if(sources_.shackReady){auto& source=sources_.entries[sources_.count++];source.kind=Kind::Shack;
            source.actor=sources_.shack.actor;source.actorSlot=sources_.shack.actorSlot;
            source.component=sources_.shack.component;source.componentSlot=sources_.shack.componentSlot;}
        for(unsigned i=0;i<3;++i)if(actors[i]){if(!ReadSource(static_cast<Kind>(i),actors[i],sources_.entries[sources_.count]))return false;++sources_.count;}
        if(in_.fallbackMap){
            // Shack/phone/cars (above), then police exits for any traps left over.
            // Missing exits never fail the phase unless nothing at all was found.
            int have=0;const unsigned slots=Count(have)?ExitSlots(have,sources_.count,static_cast<unsigned>(sources_.entries.size())):0;
            // H2.48: with no shack/phone/cars (Grendel) the map's own escape objectives
            // (bridge/boat/pod/car volumes) come first; police exits fill what is left.
            if(slots&&CFG_H248_GRENDEL_TRAPS&&!sources_.count){DiscoverExits(slots,true);
                if(sources_.count<slots)DiscoverExits(slots);}
            else if(slots)DiscoverExits(sources_.count+slots);
            if(!sources_.count)return false;
        }
        return FinishDiscover(count);
    }
    // One native trap for each proved opening objective, including variants
    // whose authored inventory is only3. Never replenish after discovery.
    bool FinishDiscover(unsigned& count){
        int inventory=0;if(!Count(inventory))return false;
        if(inventory<static_cast<int>(sources_.count)&&
           !h_.TrapOpeningInventory(inventory,static_cast<int>(sources_.count)))return false;
        if(!Count(inventory)||inventory<static_cast<int>(sources_.count))return false;
        sources_.discovered=true;count=sources_.count;return true;
    }
    bool OptionalAnchorUnavailable(unsigned i){return i<sources_.count&&sources_.entries[i].kind==Kind::Shack&&CurrentLinks();}
    bool AnchorAt(unsigned i,Anchor& anchor){
        if(!CurrentLinks()||i>=sources_.count)return false;const auto& saved=sources_.entries[i];std::uintptr_t owner=0;
        if(ExitLike(saved.kind)){
            anchor.kind=saved.kind;anchor.forward=saved.exitForward;
            return ExitFresh(saved)&&Location(saved.actor,anchor.position)&&Point(anchor.position)&&Direction(anchor.forward);}
        if(saved.kind==Kind::Shack){Vector cabin{};anchor.kind=Kind::Shack;
            return sources_.shackReady&&Location(saved.actor,cabin)&&sources_.shack.Position(r_,h_.context.objectArray,cabin,anchor.position,anchor.forward);}
        if(saved.kind==Kind::Phone){if(!Phone(owner))return false;}
        else if(!cfgvehicle::RosterCar(r_,h_.context.objectArray,in_.module,in_.world,in_.mode,in_.gameState,
            saved.kind==Kind::Car2?disk::Car2SeatVehicle:disk::Car4SeatVehicle,owner))return false;
        Source fresh{};if(owner!=saved.actor||!ReadSource(saved.kind,owner,fresh)||!(fresh.actorSlot==saved.actorSlot)||
            fresh.component!=saved.component||!(fresh.componentSlot==saved.componentSlot))return false;
        anchor.kind=saved.kind;
        return Location(owner,anchor.position)&&Call(disk::GetActorForwardVector,disk::GetActorForwardVectorExecutor,disk::GetActorForwardVectorCode,owner,&anchor.forward,true,true)&&
            Point(anchor.position)&&Direction(anchor.forward);
    }
    bool Position(Vector& p){return Location(in_.jason,p);}
    bool Count(int& count){return CurrentLinks()&&r_.Read(in_.jason+disk::TrapCount,count)&&count>=0&&count<=32;}
    bool MoveWithRadius(Vector target,unsigned& result,float radius){MoveLocationParameters p;if(!MoveToAnchor(target,radius,Fam(),in_.verifiedMap,p))return false;p.result=255;p.allowPartial=1;sources_.facePending=false;
        if(!Call(disk::MoveToLocation,disk::MoveToLocationExecutor,disk::MoveToLocationCode,in_.controller,&p))return false;result=p.result;return result<=2;}
    bool Move(Vector target,unsigned& result){return MoveWithRadius(target,result,310);}
    bool MovePrecise(Vector target,unsigned& result){return MoveWithRadius(target,result,35);}
    bool Stop(){sources_.facePending=false;return Call(disk::StopMovement,disk::StopMovementExecutor,disk::StopMovementCode,in_.controller,nullptr);}
    bool FacePending()const{return sources_.facePending;}
    bool Face(Vector position,Vector target){RotationParameters p;if(!FaceAnchor(position,target,Fam(),in_.verifiedMap,p))return false;
        // Finish the already-dispatched body/control pair at its exact stored
        // yaw. Capsule settling can change the recomputed angle by >.5 degree
        // every slice; restarting body rotation then never reaches the probe.
        // A materially moved source still restarts the pair; native legality
        // and the objective geometry are freshly repeated before placement.
        const auto px=position.x-sources_.pendingFacePosition.x,py=position.y-sources_.pendingFacePosition.y,pz=position.z-sources_.pendingFacePosition.z;
        const auto tx=target.x-sources_.pendingFaceTarget.x,ty=target.y-sources_.pendingFaceTarget.y,tz=target.z-sources_.pendingFaceTarget.z;
        // At a nearby box even20cm of settling can change yaw by>5deg.
        // Complete the original guarded pair for the same nearby objective;
        // the subsequent phone stability/native placement checks still own
        // acceptance. A moved actor/objective starts a fresh pair.
        if(sources_.facePending&&px*px+py*py+pz*pz<=40.f*40.f&&tx*tx+ty*ty+tz*tz<=5.f*5.f){
            sources_.facePending=false;
            const bool controller=Call(disk::SetControlRotation,disk::SetControlRotationExecutor,disk::SetControlRotationCode,in_.controller,&sources_.pendingFace);
            // Donor accepts actor OR controller rotation. A false native body
            // result must still get the independent controller opportunity.
            // A failed owner/budget proof or exception never becomes success.
            return controller||(sources_.faceBodyAccepted&&Guard());}
        // Body/control are separate freshly guarded callbacks, not two full
        // owner walks after mutation in a1536-read placement phase.
        sources_.facePending=false;
        sources_.faceBodyAccepted=false;
        if(!Call(disk::K2_SetActorRotation,disk::K2_SetActorRotationExecutor,disk::K2_SetActorRotationCode,in_.jason,&p)||p.result>1)return false;
        sources_.faceBodyAccepted=p.result==1;
        sources_.pendingFace=p.rotation;sources_.pendingFacePosition=position;sources_.pendingFaceTarget=target;sources_.facePending=true;return true;}
    bool Probe(CanPlaceParameters& p){p={};return Call(disk::CanPlaceTrap,disk::CanPlaceTrapExecutor,disk::CanPlaceTrapCode,in_.jason,&p,true,true)&&p.result<=1;}
    bool Attempt(){return Call(disk::AttemptPlaceTrap,disk::AttemptPlaceTrapExecutor,disk::AttemptPlaceTrapCode,in_.jason,nullptr);}
    MorphResult RepositionCarRetry(unsigned i,const Anchor& anchor,Vector pawn,unsigned retry){
        if(!retry||retry>3||i>=sources_.count||
           (anchor.kind!=Kind::Car2&&anchor.kind!=Kind::Car4))return MorphResult::NotNeeded;
        auto& m=sources_.morph[i];if(m.localRetryAttempted>=retry)return MorphResult::NotNeeded;
        const auto dx=pawn.x-anchor.position.x,dy=pawn.y-anchor.position.y;
        if(!Point(pawn)||!Point(anchor.position)||!Direction(anchor.forward)||
           dx*dx+dy*dy>450.f*450.f||std::fabs(pawn.z-anchor.position.z)>180.f)return MorphResult::NotNeeded;
        Anchor fresh{};
        if(!AnchorAt(i,fresh)||fresh.kind!=anchor.kind||
           fresh.position.x!=anchor.position.x||fresh.position.y!=anchor.position.y||fresh.position.z!=anchor.position.z||
           fresh.forward.x!=anchor.forward.x||fresh.forward.y!=anchor.forward.y||fresh.forward.z!=anchor.forward.z)return MorphResult::Pending;
        TeleportParameters p;p.destination=Stand(anchor,retry);p.destination.z+=90.f;
        RotationParameters face;
        if(!FaceAnchor(p.destination,anchor.position,Fam(),in_.verifiedMap,face))return MorphResult::Unavailable;
        p.rotation=face.rotation;
        // Same donor car setup retry, not an extra long-range combat Morph.
        // Native collision still decides; each distinct bounded stand is
        // attempted ONCE, including false/exception/uncertain readback.
        // Prepare/guard before consuming permission, never replay a call.
        std::uintptr_t function=0,owner=0,cached=0;
        if(!Guard()||!r_.Read(in_.module+disk::K2_TeleportTo.cache,cached))return MorphResult::Pending;
        if(!cached)ready_=false;
        if(!h_.FeatureFunction(disk::K2_TeleportTo,function,owner)||!cfgspawn::ObjectIs(r_,in_.jason,owner)||
           !cfgobserver::Same(r_,function+0xB0,in_.module+disk::K2_TeleportToExecutor)||
           !cfgbundle::Code(r_,in_.module,disk::K2_TeleportToExecutor,disk::K2_TeleportToCode)||!Guard())return MorphResult::Pending;
        m.localRetryAttempted=retry;ready_=false;
        if(!h_.Invoke(in_.jason,function,&p)||p.result!=1)return MorphResult::Pending;
        Vector actual{};if(!Location(in_.jason,actual))return MorphResult::Pending;
        const auto x=actual.x-p.destination.x,y=actual.y-p.destination.y;
        if(x*x+y*y>40.f*40.f||std::fabs(actual.z-p.destination.z)>120.f)return MorphResult::Pending;
        return MorphResult::Success;
    }
    MorphResult StalledMorph(unsigned i,const Anchor& anchor,Vector pawn,std::uint64_t now){return ExteriorMorph(i,anchor,pawn,now,true);}
    MorphResult ExteriorMorph(unsigned i,const Anchor& anchor,Vector pawn,std::uint64_t now,bool stalled=false){
        if(i>=sources_.count||!Point(pawn)||!Point(anchor.position)||!Direction(anchor.forward))return MorphResult::Unavailable;
        auto& m=sources_.morph[i];if(m.tried)return MorphResult::NotNeeded;
        // Shack staging must work inside the cabin too, as in the donor.
        if(!stalled&&!ExteriorStagingNeeded(anchor,pawn))return MorphResult::NotNeeded;
        if(!MorphDue(h_,now,0))return MorphResult::NotNeeded; // walk while shared clock charges
        Anchor freshAnchor{};
        if(!AnchorAt(i,freshAnchor)||freshAnchor.kind!=anchor.kind||
           freshAnchor.position.x!=anchor.position.x||freshAnchor.position.y!=anchor.position.y||freshAnchor.position.z!=anchor.position.z||
           freshAnchor.forward.x!=anchor.forward.x||freshAnchor.forward.y!=anchor.forward.y||freshAnchor.forward.z!=anchor.forward.z){
            return MorphResult::Unavailable;
        }
        if(!m.projected){
            std::uintptr_t nav=0,system=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!Guard()||!Class(disk::NavigationSystemReflection,disk::NavigationSystemConstructor,nav)||
               !r_.Read(in_.world+disk::WorldNavigationSystem,system)||!cfgspawn::ObjectIs(r_,system,nav)||
               !cfgspawn::ObjectSlot(r_,h_.context.objectArray,system,slot)||
               !cfgobserver::Same(r_,in_.world+disk::WorldNavigationSystem,system))return MorphResult::Pending;
            const float radius=anchor.kind==Kind::Shack?45.f:350.f;
            cfgvehicle::Projection p;p.world=in_.world;p.point=Stand(anchor,m.projectionCandidate);p.point.z=anchor.position.z+25.f;p.extent={radius,radius,110};
            if(!Call(disk::K2_ProjectPointToNavigation,disk::K2_ProjectPointToNavigationExecutor,disk::K2_ProjectPointToNavigationCode,system,&p,false)||
               !cfgjason::OwnedComponentSlot(r_,h_.context.objectArray,slot)||!cfgobserver::Same(r_,in_.world+disk::WorldNavigationSystem,system))return MorphResult::Pending;
            const unsigned candidates=anchor.kind==Kind::Shack?12u:4u;
            if(anchor.kind==Kind::Shack){
                // The donor's doorway fallback was absent here: CFG's native
                // projection rejected every calibrated stand in ABI40. Keep
                // the same authored stand and let K2_TeleportTo resolve its
                // collision, then verify actual arrival and native placement.
                if(!ShackStaging(anchor,m.projectionCandidate,p.result==1,p.projected,m.destination,m.directFallback))return MorphResult::Unavailable;
                m.anchor=anchor;m.projected=true;return MorphResult::Pending;
            }
            // Only a pure projection rejection may try another authored stand.
            // No teleport has occurred and the shared clock is not consumed.
            if(p.result==0&&m.projectionCandidate+1>=candidates){
                // Active donor Features.cpp "Blue-car fix": exhausting nav
                // candidates must NOT silently discard the car. Keep the exact
                // authored FRONT stand; native TeleportTo still collision-tests
                // it in a later guarded callback, then native CanPlaceTrap owns
                // legality. CFG's ground-to-capsule adapter applies here too.
                m.destination=Stand(anchor,m.projectionCandidate);m.destination.z+=90.f;
                m.anchor=anchor;m.projected=true;m.directFallback=true;return MorphResult::Pending;
            }
            if(p.result!=1||!Point(p.projected)||std::fabs(p.projected.z-anchor.position.z)>120.f){
                m.tried=++m.projectionCandidate>=candidates;return MorphResult::Pending;}
            const auto x=p.projected.x-p.point.x,y=p.projected.y-p.point.y;
            if(x*x+y*y>radius*radius){m.tried=++m.projectionCandidate>=candidates;return MorphResult::Pending;}
            // Native projection gives ground.90uu is explicit base policy,
            // not an assertion about an unobserved CFG capsule ABI.
            m.destination=p.projected;m.destination.z+=90.f;m.anchor=anchor;m.projected=true;return MorphResult::Pending;
        }
        // Runtime supplies a newly typed/owned AnchorAt sample every slice.
        const auto ax=anchor.position.x-m.anchor.position.x,ay=anchor.position.y-m.anchor.position.y,az=anchor.position.z-m.anchor.position.z;
        if(anchor.kind!=m.anchor.kind||ax*ax+ay*ay+az*az>1.f||
           std::fabs(anchor.forward.x-m.anchor.forward.x)>.001f||std::fabs(anchor.forward.y-m.anchor.forward.y)>.001f){m.tried=true;return MorphResult::Unavailable;}
        if(anchor.kind==Kind::Shack&&!m.stopped){
            // Donor stops its old path before staging. Separate callback
            // preserves the bounded owner proof and prevents stale locomotion
            // from carrying Jason away from the threshold after teleport.
            if(!Stop())return MorphResult::Pending;
            m.stopped=true;return MorphResult::Pending;
        }
        RotationParameters face;if(!FaceAnchor(m.destination,anchor.position,Fam(),in_.verifiedMap,face)){m.tried=true;return MorphResult::Unavailable;}
        TeleportParameters p;p.destination=m.destination;p.rotation=face.rotation;
        // Publish at-most-once and reserve the shared20s clock before dispatch,
        // including denial/exception. No same-objective native teleport retry.
        if(!MorphDue(h_,now,0))return MorphResult::NotNeeded;
        m.tried=true;MorphUsed(h_,now,0);
        if(!Call(disk::K2_TeleportTo,disk::K2_TeleportToExecutor,disk::K2_TeleportToCode,in_.jason,&p)||p.result!=1)return MorphResult::Pending;
        m.nativeAccepted=true;
        // Native true can include collision adjustment; it is not proof of the
        // requested arrival. Location refreshes complete Context/Jason because
        // teleport consumed callback-local validation credit before dispatch.
        // Failure only yields; tried/cooldown stay consumed and cannot retry.
        if(!Location(in_.jason,m.actual))return MorphResult::Pending;
        const auto tx=m.actual.x-m.destination.x,ty=m.actual.y-m.destination.y,tz=m.actual.z-m.destination.z;
        if(tx*tx+ty*ty>200.f*200.f||std::fabs(tz)>120.f)return MorphResult::Pending;
        m.arrivalVerified=true;
        return MorphResult::Success;
    }
};
}
