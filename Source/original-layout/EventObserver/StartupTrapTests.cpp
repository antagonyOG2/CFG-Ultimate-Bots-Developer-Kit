#include "NativeStartupTraps.hpp"
#include "../Core/StartupTrapMapScope.hpp"
#define main CfgExistingTrapFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
using namespace cfgtrap;
using namespace cfgtrap::startup;
struct Fake {
    bool ready=true,legal=true,consume=true,call=true;unsigned attempted=0,calls=0;int count=1;
    Vector position{155,0,85};Anchor anchor{Kind::Phone,{0,0,0},{0,-1,0}};
    bool Ready(){return ready;} bool Discover(unsigned& n){n=1;return true;}
    bool Count(int& n){n=count;return true;}bool AnchorAt(unsigned,Anchor& a){a=anchor;return true;}
    bool Position(Vector& p){p=position;return true;}
    bool Move(Vector p,unsigned& result){position={p.x,p.y,p.z+85};result=2;++calls;return call;}
    bool Stop(){++calls;return call;} bool Face(Vector,Vector){++calls;return call;}
    bool Probe(CanPlaceParameters& p){p={};p.result=legal?1:0;p.impact={100,0,0};++calls;return call;}
    bool Attempt(){++attempted;++calls;if(consume)--count;return call;}
};
struct OptionalShackFake : Fake {
    bool initiallyMissing=true;
    bool Discover(unsigned& n){n=2;return true;}
    bool AnchorAt(unsigned i,Anchor& a){if(i==0&&(initiallyMissing||attempted))return false;a=anchor;return true;}
    bool OptionalAnchorUnavailable(unsigned i){return i==0;}
};
struct ChargingTravelFake : Fake {
    unsigned moved=0,morphed=0;
    ChargingTravelFake(){position={9000,9000,85};}
    MorphResult ExteriorMorph(unsigned,const Anchor&,Vector,std::uint64_t now){
        if(now<25000)return MorphResult::NotNeeded;
        if(!morphed++){position={155,0,85};return MorphResult::Success;}
        return MorphResult::NotNeeded;
    }
    bool Move(Vector,unsigned& result){++moved;result=moved%2;return true;}
};
struct PreparingFake : Fake {
    bool optionalOnly=false,expired=false;unsigned preparations=0;
    Preparation Prepare(std::uint64_t){++preparations;return Preparation::Pending;}
    bool ExpirePreparation(){expired=true;return optionalOnly;}
};
struct StalledTravelFake : Fake {
    unsigned recoveries=0;std::uint64_t firstRecovery=0;
    StalledTravelFake(){position={1200,0,85};}
    bool Move(Vector,unsigned& result){result=2;return true;}
    MorphResult StalledMorph(unsigned,const Anchor&,Vector,std::uint64_t now){
        ++recoveries;firstRecovery=now;position={155,0,85};return MorphResult::Success;
    }
};
struct MovingApproachFake : Fake {
    bool moving=true;unsigned stopped=0,faceWhileMoving=0,probeWhileMoving=0;
    MovingApproachFake(){position={400,0,85};}
    bool Stop(){moving=false;++stopped;return Fake::Stop();}
    bool Face(Vector p,Vector a){if(moving){++faceWhileMoving;position.x+=30;}return Fake::Face(p,a);}
    bool Probe(CanPlaceParameters& p){if(moving)++probeWhileMoving;return Fake::Probe(p);}
};
struct SettlingCarFake : Fake {
    unsigned probes=0;
    SettlingCarFake(){anchor={Kind::Car4,{0,0,0},{1,0,0}};position={300,0,90};}
    bool Probe(CanPlaceParameters& p){++probes;position.y+=30;p={};p.result=1;p.impact={240,0,0};++calls;return true;}
};
struct AlreadyAtCarFake : Fake {
    unsigned moves=0,probes=0,repositions=0;
    AlreadyAtCarFake(){anchor={Kind::Car4,{0,0,0},{1,0,0}};position={300,40.01f,90};legal=false;}
    bool Move(Vector,unsigned& result){++moves;result=1;return true;}
    bool Probe(CanPlaceParameters& p){++probes;p={};p.result=repositions?1:0;p.impact={240,0,0};return true;}
    MorphResult RepositionCarRetry(unsigned,const Anchor& a,Vector,unsigned retry){
        if(!retry||repositions)return MorphResult::NotNeeded;
        ++repositions;const auto stand=Stand(a,retry);position={stand.x,stand.y,90};return MorphResult::Success;
    }
};
struct ProgressPreparingFake : Fake {
    unsigned completed=0;
    unsigned PreparationProgress()const{return completed;}
    Preparation Prepare(std::uint64_t){if(completed<9){++completed;return Preparation::Pending;}return Preparation::Ready;}
    bool ExpirePreparation(){return false;}
};
struct Bridge {
    Fixture& r;cfgobserver::Plan context{};Inputs in{};bool ready=true;unsigned calls=0,guardReads=0,bindingReads=0,locationReads=0;
    unsigned morphReservations=0,teleports=0,projections=0;std::uint64_t morphNext=0;bool projectionOK=true;
    bool haveTeleported=false,readbackFails=false,teleportCallOK=true,controlCallOK=true,controlFault=false,dispatchException=false;std::uint8_t teleportResult=1,rotationResult=1;Vector teleported{},readbackOffset{};
    std::map<std::uintptr_t,std::pair<std::uintptr_t,std::uintptr_t>> functions;
    std::uintptr_t forward=0,probe=0,attempt=0,rotation=0,move=0,project=0,teleport=0,control=0;float moveRadius=0;
    Bridge(Fixture& f):r(f){context.objectArray=f.objects;}
    bool Charge(unsigned n){for(unsigned i=0;i<n;++i){std::uintptr_t dummy=0;if(!r.Read(in.world+cfgprobe::WorldGameState,dummy))return false;}return true;}
    bool Context(){return ready&&Charge(guardReads);}bool Jason(){return ready&&Charge(guardReads);}
    bool Exception(){return dispatchException;}
    bool TrapMorphDue(std::uint64_t now){return now>=morphNext;}
    void TrapMorphCompleted(std::uint64_t now){++morphReservations;morphNext=now+20000;}
    std::uintptr_t TrapJason(){return in.jason;}std::uintptr_t TrapController(){return in.controller;}
    bool TrapOpeningInventory(int expected,int desired){int actual=-1;
        if(!r.Read(in.jason+disk::TrapCount,actual)||actual!=expected||expected<0||desired>4||desired<=expected)return false;
        r.Put(in.jason+disk::TrapCount,desired);return true;}
    bool Location(std::uintptr_t actor,Vector& out){out={0,0,0};
        if(actor==in.jason&&haveTeleported){if(readbackFails)return false;out={teleported.x+readbackOffset.x,teleported.y+readbackOffset.y,teleported.z+readbackOffset.z};}
        return Charge(locationReads);}
    bool FeatureFunction(const cfgvehicle::disk::Binding& b,std::uintptr_t& fn,std::uintptr_t& owner){
        const auto found=functions.find(b.cache);if(found==functions.end()||!Charge(bindingReads))return false;fn=found->second.first;owner=found->second.second;return true;}
    template<std::size_t N> void Function(const cfgvehicle::disk::Binding& b,std::uintptr_t exec,const std::array<unsigned char,N>& code,std::uintptr_t& output){
        std::uintptr_t owner=0;CHECK(r.Read(r.module+b.ownerReflection,owner));output=r.Object();functions[b.cache]={output,owner};
        r.Put(r.module+b.cache,output);r.Put(output+0xB0,r.module+exec);r.Put(r.module+exec,code);
    }
    bool Invoke(std::uintptr_t,std::uintptr_t fn,void* packet,bool=true){++calls;
        if(fn==forward)*static_cast<Vector*>(packet)={1,0,0};
        if(fn==probe){auto& p=*static_cast<CanPlaceParameters*>(packet);p.result=1;p.impact={0,100,0};}
        if(fn==rotation)static_cast<RotationParameters*>(packet)->result=rotationResult;
        if(fn==control){dispatchException=controlFault;return controlCallOK;}
        if(fn==move){auto& p=*static_cast<MoveLocationParameters*>(packet);moveRadius=p.acceptance;p.result=2;}
        if(fn==project){++projections;auto& p=*static_cast<cfgvehicle::Projection*>(packet);CHECK(p.extent.x==350&&p.extent.y==350&&p.extent.z==110);p.projected=p.point;p.projected.z=0;p.result=projectionOK?1:0;}
        if(fn==teleport){++teleports;auto& p=*static_cast<TeleportParameters*>(packet);CHECK(p.destination.z==90);p.result=teleportResult;haveTeleported=true;teleported=p.destination;return teleportCallOK;}
        if(fn==attempt){int count=0;CHECK(r.Read(in.jason+disk::TrapCount,count));r.Put(in.jason+disk::TrapCount,count-1);}
        return true;
    }
};
static std::uintptr_t NativeClass(Fixture& f,std::uintptr_t reflection,std::uintptr_t ctor){
    const auto type=f.Object();f.Put(f.module+reflection,type);f.Put(f.module+ctor,type);return type;
}
static void Positive(Fixture& f,std::uintptr_t object,cfgprobe::lifetime::SlotObservation& out){
    int index=0;CHECK(f.Read(object+0xC,index));f.Put(f.objects.data+static_cast<std::uintptr_t>(index)*24+16,index+1);
    CHECK(cfgspawn::ObjectSlot(f,f.objects,object,out));
}
int main(){
    {Runtime r;AlreadyAtCarFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
     for(std::uint64_t t=5000;t<24000;t+=300)r.Step(host,t);
     CHECK(host.moves>0&&host.repositions==1&&host.probes>=3&&host.attempted==1&&r.report.consumed==1&&!r.report.skipped);}
    {const Anchor shack{Kind::Shack,{100,200,-10},{1,0,0}};Vector destination{};bool direct=false;
     CHECK(ShackStaging(shack,0,false,{},destination,direct)&&direct&&destination.x==190&&destination.y==200&&destination.z==80);
     CHECK(ShackStaging(shack,1,true,{190,230,-12},destination,direct)&&!direct&&destination.z==78);
     CHECK(ShackStaging(shack,0,true,{900,900,0},destination,direct)&&direct&&destination.x==190);
     CHECK(ShackStaging(shack,0,true,{190,200,500},destination,direct)&&direct&&destination.z==80);
     CHECK(!ShackStaging(shack,12,false,{},destination,direct));
     auto invalid=shack;invalid.forward={};CHECK(!ShackStaging(invalid,0,false,{},destination,direct));
     invalid=shack;invalid.kind=Kind::Car2;CHECK(!ShackStaging(invalid,0,false,{},destination,direct));}
    {Runtime r;Fake host;CHECK(r.Start(0,MapFamily::GroundMap,true));CHECK(r.Step(host,5000));
     CHECK(r.PauseForVehicle(6000)&&r.Paused()&&!r.Due(80000));
     CHECK(r.Step(host,80000)&&!host.calls&&!host.attempted&&!r.report.skipped);
     CHECK(r.Resume(80000)&&!r.Paused()&&r.Due(80000));
     for(std::uint64_t t=80000;t<94000;t+=300)r.Step(host,t);
     CHECK(host.attempted==1&&r.report.consumed==1&&!r.report.skipped);
     Runtime pending;CHECK(pending.Start(0,MapFamily::GroundMap,true));pending.report.stage=Stage::Attempt;
     CHECK(!pending.PauseForVehicle(10));pending.report.stage=Stage::Confirm;CHECK(!pending.PauseForVehicle(10));
     pending.report.stage=Stage::Hold;CHECK(!pending.PauseForVehicle(10));}
    {Runtime r;StalledTravelFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
     for(std::uint64_t t=5000;t<25000;t+=300)r.Step(host,t);
     CHECK(host.recoveries==1&&host.firstRecovery>=12300&&host.attempted==1&&r.report.consumed==1&&!r.report.skipped);}
    {Runtime r;ChargingTravelFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        for(std::uint64_t t=5000;t<24000;t+=300)r.Step(host,t);
        CHECK(r.Busy()&&!r.report.retry&&!r.report.skipped&&r.report.travelFailures[0]>4&&!host.attempted);
        for(std::uint64_t t=24000;t<39000;t+=300)r.Step(host,t);
        CHECK(r.report.consumed==1&&host.attempted==1&&!r.report.skipped);}
    {Runtime r;Fake host;CHECK(ObjectiveLimitMs==36000);CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,5000)&&r.report.stage==Stage::Travel);
        CHECK(r.Step(host,5000+ObjectiveLimitMs)&&r.report.skipped==1&&r.report.outcome[0]==2);
        r.Step(host,5000+ObjectiveLimitMs+1200);CHECK(!r.Busy()&&!host.attempted);}
    {Runtime r;ProgressPreparingFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,5000)&&host.completed==1);
        for(unsigned i=1;i<9;++i)CHECK(r.Step(host,5000+30000*i)&&host.completed==i+1&&r.report.stage==Stage::Discover);
        CHECK(r.Step(host,275000)&&r.report.stage==Stage::Travel&&r.report.objectives==1);}
    {Runtime r;PreparingFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,5000)&&r.Busy());CHECK(!r.Step(host,5000+PreparationLimitMs));
        CHECK(!r.Busy()&&r.report.failure==Failure::Timeout&&host.preparations==1&&!host.attempted);}
    {Runtime r;PreparingFake host;host.optionalOnly=true;CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,5000)&&r.Busy());CHECK(r.Step(host,5000+PreparationLimitMs)&&host.expired&&r.report.stage==Stage::Travel);
        for(std::uint64_t t=5000+PreparationLimitMs+300;t<45000;t+=300)r.Step(host,t);
        CHECK(r.report.stage==Stage::Complete&&host.attempted==1&&r.report.consumed==1);}
    {Runtime r;PreparingFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,35000)&&r.Busy()&&host.preparations==1&&!host.expired);
        CHECK(!r.Step(host,35000+PreparationLimitMs)&&host.expired&&r.report.failure==Failure::Timeout);}
    {Anchor shack{Kind::Shack,{0,0,0},{1,0,0}};
        CHECK(ExteriorStagingNeeded(shack,{-100,0,85}));
        CHECK(!ExteriorStagingNeeded(shack,{90,0,85}));
        CHECK(Stand(shack,0).x==90&&Stand(shack,1).x==90&&Stand(shack,1).y==30);
        CHECK(Stand(shack,2).y==-30&&Stand(shack,3).x==110&&Stand(shack,11).x==150);
        Anchor phone{Kind::Phone,{0,0,0},{1,0,0}};
        CHECK(!ExteriorStagingNeeded(phone,{100,0,85})&&ExteriorStagingNeeded(phone,{3000,0,85}));}
    {Runtime r;Fake host;host.position={400,0,85};CHECK(r.Start(0,MapFamily::GroundMap,true));
        CHECK(r.Step(host,5000)&&r.report.stage==Stage::Travel);
        CHECK(r.Step(host,5300)&&r.report.stage==Stage::ApproachStop);
        CHECK(r.Step(host,5600)&&r.report.stage==Stage::ApproachFace);
        CHECK(r.Step(host,5900)&&r.report.stage==Stage::ApproachFace&&host.calls==1);
        CHECK(r.Step(host,6500)&&r.report.stage==Stage::ApproachProbe);
        CHECK(r.Step(host,6800)&&r.report.stage==Stage::Face&&host.calls==3);
        for(std::uint64_t t=7700;t<18000;t+=300)r.Step(host,t);
        CHECK(host.attempted==1&&r.report.consumed==1&&r.report.outcome[0]==1);}
    {Runtime r;MovingApproachFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        for(std::uint64_t t=5000;t<18000;t+=300)r.Step(host,t);
        CHECK(host.stopped==1&&!host.faceWhileMoving&&!host.probeWhileMoving&&host.attempted==1);
        CHECK(r.report.stops[0]==1&&r.report.nativePlacement[0]==1&&r.report.legalPlacement[0]==1&&r.report.kinds[0]==0);}
    {Runtime r;OptionalShackFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
        for(std::uint64_t t=5000;t<25000;t+=300)r.Step(host,t);
        CHECK(r.report.stage==Stage::Complete&&r.report.skipped==1&&r.report.consumed==1&&host.attempted==1);}
    {Runtime r;OptionalShackFake host;host.initiallyMissing=false;host.consume=false;
        CHECK(r.Start(0,MapFamily::GroundMap,true));for(std::uint64_t t=5000;t<25000;t+=300)r.Step(host,t);
        CHECK(r.report.stage==Stage::Indeterminate&&r.report.skipped==0&&host.attempted==1);}
    CHECK(MapFamilyForPackage(u"/Game/Maps/CrystalLake_Small/CrystalLake_Small")==MapFamily::GroundMap);
    CHECK(MapFamilyForPackage(u"/Game/Maps/CrystalLake_Small/Snow/CrystalLake_Small_Snow")==MapFamily::GroundMap);
    CHECK(MapFamilyForArrival(u"/Game/Maps/CrystalLake_Small/Snow/CrystalLake_Small_Snow",1,1)==MapFamily::GroundMap);
    CHECK(MapFamilyForArrival(u"/Game/Maps/CrystalLake_Small/Snow/CrystalLake_Small_Snow",1,2)==MapFamily::Unknown);
    for(const auto path:{u"Crystal Lake Small",u"/Game/Maps/Grendel/Grendel",u"/Game/Maps/GrendelLarge/GrendelLarge",u"/Game/Maps/CrystalLake_Small/CrystalLake_Small_Snow",u"/Game/Maps/CrystalLake_Small/CrystalLake_Small.CrystalLake_Small",u"/game/maps/crystallake_small/crystallake_small"})CHECK(MapFamilyForPackage(path)==MapFamily::Unknown);
    CHECK(MapFamilyForArrival(u"/Game/Maps/CrystalLake_Small/CrystalLake_Small",1,2)==MapFamily::Unknown);
    Runtime runtime;Fake f;
    {Runtime r;SettlingCarFake host;CHECK(r.Start(0,MapFamily::GroundMap,true));
     for(std::uint64_t t=5000;t<18000;t+=300)r.Step(host,t);
     CHECK(host.attempted==1&&host.probes>=3&&r.report.consumed==1);}
    CHECK(runtime.Start(0,MapFamily::GroundMap,true));CHECK(runtime.Step(f,4999)&&runtime.report.stage==Stage::Discover);
    for(std::uint64_t t=5000;t<10000;t+=300)runtime.Step(f,t);
    CHECK(f.attempted==1&&runtime.report.stage==Stage::Hold&&runtime.report.consumed==1&&f.count==0);
    for(std::uint64_t t=10000;t<18000;t+=300)runtime.Step(f,t);
    CHECK(runtime.report.stage==Stage::Complete&&f.attempted==1);
    for(const auto family:{MapFamily::Unknown,MapFamily::Grendel,MapFamily::GrendelLarge}){Runtime r;CHECK(!r.Start(0,family,true));}
    Runtime unverified;CHECK(!unverified.Start(0,MapFamily::GroundMap,false));
    Runtime uncertain;Fake noCount;noCount.consume=false;CHECK(uncertain.Start(0,MapFamily::GroundMap,true));
    for(std::uint64_t t=5000;t<18000;t+=300)uncertain.Step(noCount,t);
    CHECK(uncertain.report.stage==Stage::Indeterminate&&noCount.attempted==1);
    Runtime bad;Fake illegal;illegal.legal=false;CHECK(bad.Start(0,MapFamily::GroundMap,true));
    for(std::uint64_t t=5000;t<50000;t+=300)bad.Step(illegal,t);
    CHECK(illegal.attempted==0&&bad.report.skipped==1);
    Runtime lost;Fake l;CHECK(lost.Start(0,MapFamily::GroundMap,true));l.ready=false;CHECK(!lost.Step(l,5000)&&l.calls==0);
    Anchor phone{Kind::Phone,{0,0,0},{1,0,0}};CanPlaceParameters p;p.result=1;p.impact={0,100,0};CHECK(Legal(phone,p,0));
    p.impact.y=-100;CHECK(!Legal(phone,p,0));p.impact={0,50,0};CHECK(!Legal(phone,p,0));p.impact={0,100,200};CHECK(!Legal(phone,p,0));
    Anchor car{Kind::Car2,{0,0,0},{1,0,0}};p.impact={240,0,0};CHECK(Legal(car,p,0));p.impact={0,240,0};CHECK(!Legal(car,p,0));
    // Compile and execute the real reader-backed adapter's refusal paths.
    Fixture fixture;Bridge bridge(fixture);Sources sources;Inputs inputs;NativeAccess<Fixture,Bridge> native(fixture,bridge,inputs,sources);
    CHECK(!native.Ready());unsigned n=0;CHECK(!native.Discover(n));int count=0;CHECK(!native.Count(count));
    Vector location{};Anchor a;CHECK(!native.AnchorAt(0,a));CHECK(!native.Position(location));
    CHECK(!native.Stop());CHECK(!native.Face({},{1,0,0}));CHECK(!native.Probe(p));CHECK(!native.Attempt());
    unsigned result=0;CHECK(!native.Move({},result));CHECK(bridge.calls==0);
    CHECK(disk::CanPlaceTrap.nativeBytes==25&&disk::CanPlaceTrap.storageBytes==28);
    CHECK(disk::MoveToLocation.nativeBytes==34&&disk::MoveToLocation.storageBytes==40);
    CHECK(disk::K2_SetActorRotation.nativeBytes==14&&disk::K2_SetActorRotation.storageBytes==16);
    Fixture owned;owned.cap=10000;Bridge h(owned);Inputs input;Sources stored;
    const auto modeClass=NativeClass(owned,disk::SCGameModeReflection,disk::SCGameModeConstructor);
    const auto gsClass=NativeClass(owned,disk::SCGameStateReflection,disk::SCGameStateConstructor);
    const auto phoneClass=NativeClass(owned,disk::SCPhoneJunctionBoxReflection,disk::SCPhoneJunctionBoxConstructor);
    const auto carClass=NativeClass(owned,disk::SCDriveableVehicleReflection,disk::SCDriveableVehicleConstructor);
    const auto compClass=NativeClass(owned,disk::SCInteractComponentReflection,disk::SCInteractComponentConstructor);
    const auto actorClass=NativeClass(owned,disk::ActorReflection,disk::ActorConstructor);
    const auto killerClass=NativeClass(owned,disk::SCKillerCharacterReflection,disk::SCKillerCharacterConstructor);
    const auto controllerClass=NativeClass(owned,disk::ControllerReflection,disk::ControllerConstructor);
    const auto aiClass=NativeClass(owned,disk::AIControllerReflection,disk::AIControllerConstructor);
    const auto navClass=NativeClass(owned,disk::NavigationSystemReflection,disk::NavigationSystemConstructor);
    const auto navCDO=owned.Object(navClass);owned.Put(navClass+cfgspawn::ClassDefaultObject,navCDO);
    owned.Put(phoneClass+0x30,actorClass);owned.Put(carClass+0x30,actorClass);owned.Put(killerClass+0x30,actorClass);owned.Put(aiClass+0x30,controllerClass);
    input.module=owned.module;input.world=owned.Object();input.mode=owned.Object(modeClass);input.gameState=owned.Object(gsClass);
    input.jason=owned.Object(killerClass);input.controller=owned.Object(aiClass);input.family=MapFamily::GroundMap;input.verifiedMap=true;
    Positive(owned,input.jason,input.jasonSlot);Positive(owned,input.controller,input.controllerSlot);
    owned.Put(input.world+cfgprobe::WorldGameState,input.gameState);owned.Put(input.jason+disk::TrapCount,2);
    owned.Put(input.world+disk::WorldNavigationSystem,navCDO);
    const auto phoneActor=owned.Object(phoneClass),phoneComp=owned.Object(compClass),carActor=owned.Object(carClass),carComp=owned.Object(compClass);
    cfgprobe::lifetime::SlotObservation temporary;Positive(owned,phoneActor,temporary);Positive(owned,carActor,temporary);
    owned.Put(phoneActor+disk::InteractComponent,phoneComp);owned.Put(phoneComp+0x20,phoneActor);owned.Put(phoneComp+0xE0,phoneActor);
    owned.Put(carActor+disk::JasonInteractComponent,carComp);owned.Put(carComp+0x20,carActor);owned.Put(carComp+0xE0,carActor);
    owned.Put(input.gameState+disk::Phone,phoneActor);
    owned.Put(input.gameState+disk::Car2SeatVehicle,carActor);owned.Put(input.gameState+disk::Car4SeatVehicle,std::uintptr_t{0});
    const auto vehicleLevel=owned.Object();owned.Put(vehicleLevel+cfgkill::disk::OwningWorld,input.world);
    owned.Put(carActor+0x20,vehicleLevel);owned.Put(carActor+cfgvehicle::VehicleType,std::uint8_t{1});
    owned.Put(carActor+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{owned.Object(),2,2});
    const auto roster=owned.Object();owned.Put(roster,carActor);
    owned.Put(input.mode+cfgvehicle::SpawnedVehicles,cfgshacktrap::List{roster,1,1});
    // OfflineBots has an authoritative mode roster while inherited car caches
    // remain empty. Both trap discovery and pursuit use this shared resolver.
    owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
    std::uintptr_t resolved=0;
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&resolved==carActor);
    owned.Put(vehicleLevel+cfgkill::disk::OwningWorld,input.mode);
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&!resolved);
    owned.Put(vehicleLevel+cfgkill::disk::OwningWorld,input.world);
    owned.Put(carActor+cfgvehicle::VehicleType,std::uint8_t{2});
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&!resolved);
    owned.Put(carActor+cfgvehicle::VehicleType,std::uint8_t{1});
    owned.Put(carActor+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,4,4});
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car4,resolved)&&resolved==carActor);
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&!resolved);
    owned.Put(carActor+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,2,2});
    const auto duplicateCar=owned.Object(carClass);Positive(owned,duplicateCar,temporary);
    owned.Put(duplicateCar+0x20,vehicleLevel);owned.Put(duplicateCar+cfgvehicle::VehicleType,std::uint8_t{1});
    owned.Put(duplicateCar+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,2,2});
    owned.Put(roster+8,duplicateCar);owned.Put(input.mode+cfgvehicle::SpawnedVehicles,cfgshacktrap::List{roster,2,2});
    CHECK(!cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved));
    owned.Put(input.mode+cfgvehicle::SpawnedVehicles,cfgshacktrap::List{roster,1,1});
    owned.Put(input.gameState+disk::Car2SeatVehicle,carActor);
    CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&resolved==carActor);
    owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
    h.in=input;h.context.world=input.world;h.context.mode=input.mode;
    { // Expiring an optional shack scan still discovers authoritative phone/car.
        Sources delayed;delayed.prepared=9;delayed.optionalBegun=true;delayed.optionalStarted=1;
        delayed.shack.stage=1;delayed.shack.slices=338;delayed.shack.restarts=21;
        NativeAccess<Fixture,Bridge> optional(owned,h,input,delayed);Runtime bounded;
        CHECK(bounded.Start(0,MapFamily::GroundMap,true));owned.reads=0;
        CHECK(bounded.Step(optional,PreparationLimitMs)&&bounded.report.stage==Stage::Travel&&bounded.report.objectives==2);
        CHECK(delayed.shack.failure==10&&delayed.shack.slices==338&&!delayed.shackReady);
        Sources late;late.prepared=10;late.optionalBegun=true;late.optionalStarted=1000;
        NativeAccess<Fixture,Bridge> timer(owned,h,input,late);
        CHECK(timer.Prepare(9000)==Preparation::Ready&&late.prepared==12&&late.shack.failure==10);
        Sources capped;capped.prepared=9;capped.optionalBegun=true;capped.optionalStarted=9000;capped.optionalSlices=32;
        NativeAccess<Fixture,Bridge> work(owned,h,input,capped);
        CHECK(work.Prepare(9001)==Preparation::Ready&&capped.shack.failure==10);
        Sources required;required.prepared=8;NativeAccess<Fixture,Bridge> incomplete(owned,h,input,required);
        CHECK(!incomplete.ExpirePreparation()&&required.prepared==8);
    }
    { // Two real car entries: compare three full scans to one callback proof.
        owned.Put(duplicateCar+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,4,4});
        owned.Put(input.mode+cfgvehicle::SpawnedVehicles,cfgshacktrap::List{roster,2,2});
        owned.reads=0;
        for(unsigned i=0;i<3;++i)CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved)&&resolved==carActor);
        const unsigned fullReads=owned.reads;owned.reads=0;cfgvehicle::RosterProof proof;
        CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved,&proof)&&resolved==carActor);
        CHECK(cfgvehicle::RosterCurrent(owned,owned.objects,proof));CHECK(cfgvehicle::RosterCurrent(owned,owned.objects,proof));
        const unsigned proofReads=owned.reads;CHECK(proofReads<fullReads&&proofReads<1536);
        std::printf("Vehicle two-car three-check budget: full=%u proof=%u/1536 reads\n",fullReads,proofReads);
        {struct CacheHost {Fixture& r;bool current=true;unsigned writes=0;
            bool Current(){return current;}
            bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t desired){std::uintptr_t before=0;
                if(!r.Read(at,before)||before!=expected)return false;++writes;r.Put(at,desired);return true;}} host{owned};
         auto stale=proof;stale.slot.serial++;CHECK(!cfgvehicle::PublishRosterCache(owned,owned.objects,stale,host)&&!host.writes);
         host.current=false;CHECK(!cfgvehicle::PublishRosterCache(owned,owned.objects,proof,host)&&!host.writes);host.current=true;
         owned.Put(input.gameState+disk::Car2SeatVehicle,duplicateCar);CHECK(!cfgvehicle::PublishRosterCache(owned,owned.objects,proof,host)&&!host.writes);
         owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
         CHECK(cfgvehicle::PublishRosterCache(owned,owned.objects,proof,host)&&host.writes==1);
         CHECK(!cfgvehicle::PublishRosterCache(owned,owned.objects,proof,host)&&host.writes==1);
         cfgvehicle::RosterProof cached;CHECK(cfgvehicle::RosterCar(owned,owned.objects,input.module,input.world,input.mode,input.gameState,cfgvehicle::disk::Car2,resolved,&cached));
         CHECK(cached.cached&&cfgvehicle::PublishRosterCache(owned,owned.objects,cached,host)&&host.writes==1);
         owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
        }
        { // Actual publish adapter, with conservative outer-authority read cost.
          // Acquisition is deliberately NOT charged in this fresh callback.
          struct BudgetHost {Fixture& r;unsigned writes=0;
            bool Current(){if(r.cap-r.reads<512){r.reads=r.cap;return false;}r.reads+=512;return true;}
            bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){
                std::uintptr_t before=0;if(!r.Read(at,before)||before!=expected)return false;
                ++writes;r.Put(at,value);return true;}} host{owned};
          const unsigned savedCap=owned.cap;owned.cap=1536;owned.reads=0;
          CHECK(cfgvehicle::PublishRosterCache(owned,owned.objects,proof,host)&&host.writes==1);
          CHECK(owned.reads<1536);
          std::printf("Fresh car-cache publication including two512-read authority passes=%u/1536\n",owned.reads);
          owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
          owned.cap=savedCap;owned.reads=0;
        }
        owned.Put(roster+8,carActor);CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));owned.Put(roster+8,duplicateCar);
        owned.Put(input.gameState+disk::Car2SeatVehicle,carActor);CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));owned.Put(input.gameState+disk::Car2SeatVehicle,std::uintptr_t{});
        owned.Put(vehicleLevel+cfgkill::disk::OwningWorld,input.mode);CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));owned.Put(vehicleLevel+cfgkill::disk::OwningWorld,input.world);
        owned.Put(carActor+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,4,4});CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));owned.Put(carActor+cfgvehicleinteraction::disk::Seats,cfgshacktrap::List{roster,2,2});
        cfgprobe::lifetime::SlotObservation observed{};CHECK(cfgspawn::ObjectSlot(owned,owned.objects,carActor,observed));
        owned.Put(owned.objects.data+24*observed.index+16,observed.serial+1);CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));owned.Put(owned.objects.data+24*observed.index+16,observed.serial);
        owned.Put(input.mode+cfgvehicle::SpawnedVehicles,cfgshacktrap::List{roster,1,1});CHECK(!cfgvehicle::RosterCurrent(owned,owned.objects,proof));
    }
    h.Function(disk::GetActorForwardVector,disk::GetActorForwardVectorExecutor,disk::GetActorForwardVectorCode,h.forward);
    h.Function(disk::CanPlaceTrap,disk::CanPlaceTrapExecutor,disk::CanPlaceTrapCode,h.probe);
    h.Function(disk::AttemptPlaceTrap,disk::AttemptPlaceTrapExecutor,disk::AttemptPlaceTrapCode,h.attempt);
    h.Function(disk::K2_SetActorRotation,disk::K2_SetActorRotationExecutor,disk::K2_SetActorRotationCode,h.rotation);
    h.Function(disk::MoveToLocation,disk::MoveToLocationExecutor,disk::MoveToLocationCode,h.move);
    std::uintptr_t stop=0;h.Function(disk::StopMovement,disk::StopMovementExecutor,disk::StopMovementCode,stop);
    std::uintptr_t control=0;h.Function(disk::SetControlRotation,disk::SetControlRotationExecutor,disk::SetControlRotationCode,control);
    h.control=control;
    h.Function(disk::K2_ProjectPointToNavigation,disk::K2_ProjectPointToNavigationExecutor,disk::K2_ProjectPointToNavigationCode,h.project);
    h.Function(disk::K2_TeleportTo,disk::K2_TeleportToExecutor,disk::K2_TeleportToCode,h.teleport);
    for(unsigned fault=0;fault<4;++fault){Sources fallback;owned.reads=0;owned.cap=1536;
     h.rotationResult=fault==1||fault==3?1:0;h.controlCallOK=fault==0;h.controlFault=fault==3;h.dispatchException=false;
     NativeAccess<Fixture,Bridge> body(owned,h,input,fallback);
     CHECK(body.Ready()&&body.Face({},{0,100,0})&&body.FacePending());
     owned.reads=0;NativeAccess<Fixture,Bridge> controller(owned,h,input,fallback);
     CHECK(controller.Ready());CHECK(controller.Face({},{0,100,0})==(fault<2));CHECK(!controller.FacePending());
     h.rotationResult=1;h.controlCallOK=true;h.controlFault=h.dispatchException=false;
    }
    owned.reads=0;owned.cap=1536;NativeAccess<Fixture,Bridge> access(owned,h,input,stored);
    owned.Put(input.jason+disk::TrapCount,1);
    CHECK(access.Ready());CHECK(access.Discover(n)&&n==2);CHECK(!access.Discover(n));
    owned.reads=0;CHECK(access.AnchorAt(0,a)&&a.kind==Kind::Phone);CHECK(access.AnchorAt(1,a)&&a.kind==Kind::Car2);
    CHECK(access.Count(count)&&count==2);CHECK(access.Probe(p)&&p.result==1);CHECK(access.Move({100,100,0},result)&&result==2);
    CHECK(h.moveRadius==310);owned.reads=0;CHECK(access.MovePrecise({100,100,0},result)&&h.moveRadius==35&&result==2);
    CHECK(access.Stop());CHECK(access.Face({},{0,100,0}));CHECK(access.Attempt());CHECK(access.Count(count)&&count==1);
    // Strong component serial0 is accepted only with the exact stored owned chain.
    CHECK(stored.entries[0].componentSlot.serial==0);
    owned.Put(phoneComp+0x20,carActor);owned.reads=0;CHECK(!access.AnchorAt(0,a));owned.Put(phoneComp+0x20,phoneActor);
    owned.Put(carActor+cfgvehicle::disk::Driver,input.jason);CHECK(!access.AnchorAt(1,a));owned.Put(carActor+cfgvehicle::disk::Driver,std::uintptr_t{0});
    owned.Put(input.gameState+disk::Phone,carActor);CHECK(!access.AnchorAt(0,a));owned.Put(input.gameState+disk::Phone,phoneActor);
    owned.Put(h.probe+0xB0,std::uintptr_t{0});const auto calls=h.calls;CHECK(!access.Probe(p)&&h.calls==calls);
    owned.cap=owned.reads;CHECK(!access.Count(count));
    owned.Put(h.probe+0xB0,owned.module+disk::CanPlaceTrapExecutor);
    // Production-cost guards: each Context150 + Jason150 real reads, binding50,
    // each Location20, all charged to the SAME1536-read fixture each slice.
    h.guardReads=150;h.bindingReads=50;h.locationReads=20;owned.cap=1536;
    unsigned peak=0;
    for(unsigned phase=0;phase<9;++phase){owned.reads=0;CHECK(h.Context()&&h.Jason());NativeAccess<Fixture,Bridge> warm(owned,h,input,stored);CHECK(warm.Ready());CHECK(warm.Prepare()==Preparation::Pending);if(owned.reads>peak)peak=owned.reads;}
    stored.facePending=false;owned.reads=0;NativeAccess<Fixture,Bridge> facePhase(owned,h,input,stored);
    CHECK(h.Context()&&h.Jason()); // outer BundlePhase guards also count
    CHECK(facePhase.Ready());CHECK(facePhase.Count(count));CHECK(facePhase.AnchorAt(0,a));CHECK(facePhase.Position(location));CHECK(facePhase.Face(location,{0,100,0}));if(owned.reads>peak)peak=owned.reads;
    CHECK(facePhase.FacePending());
    owned.reads=0;NativeAccess<Fixture,Bridge> controlPhase(owned,h,input,stored);
    CHECK(h.Context()&&h.Jason()&&controlPhase.Ready());
    CHECK(controlPhase.Face({location.x+20,location.y,location.z},{0,100,0})&&!controlPhase.FacePending()); // small settling may exceed5deg; finish stored pair
    if(owned.reads>peak)peak=owned.reads;
    owned.reads=0;NativeAccess<Fixture,Bridge> attemptPhase(owned,h,input,stored);
    CHECK(h.Context()&&h.Jason());
    CHECK(attemptPhase.Ready());CHECK(attemptPhase.Count(count));CHECK(attemptPhase.AnchorAt(0,a));CHECK(attemptPhase.Position(location));CHECK(attemptPhase.Probe(p));CHECK(attemptPhase.Count(count));CHECK(attemptPhase.Attempt());if(owned.reads>peak)peak=owned.reads;
    owned.reads=0;NativeAccess<Fixture,Bridge> projectPhase(owned,h,input,stored);
    CHECK(h.Context()&&h.Jason());
    CHECK(projectPhase.Ready());CHECK(projectPhase.AnchorAt(0,a));CHECK(projectPhase.ExteriorMorph(0,a,{3000,0,0},10000)==MorphResult::Pending);CHECK(h.projections==1&&h.teleports==0);if(owned.reads>peak)peak=owned.reads;
    owned.reads=0;NativeAccess<Fixture,Bridge> teleportPhase(owned,h,input,stored);
    CHECK(h.Context()&&h.Jason());
    CHECK(teleportPhase.Ready());CHECK(teleportPhase.Count(count));CHECK(teleportPhase.AnchorAt(0,a));CHECK(teleportPhase.Position(location));CHECK(teleportPhase.ExteriorMorph(0,a,{3000,0,0},10300)==MorphResult::Success);CHECK(h.teleports==1&&h.morphReservations==1&&h.morphNext==30300);
    CHECK(stored.morph[0].nativeAccepted&&stored.morph[0].arrivalVerified&&stored.morph[0].actual.z==90);
    { // A pure-read budget refusal must not consume this objective's one
      // teleport or shared Morph clock. Retry after a fresh bounded callback.
      stored.morph[1]={};h.morphNext=0;owned.reads=1536;
      NativeAccess<Fixture,Bridge> deferred(owned,h,input,stored);
      CHECK(deferred.StalledMorph(1,a,{1200,0,0},11000)==MorphResult::Unavailable);
      CHECK(!stored.morph[1].tried&&!stored.morph[1].projected);
      owned.reads=0;CHECK(deferred.AnchorAt(1,a));
      CHECK(deferred.ExteriorMorph(1,a,{1200,0,0},11200)==MorphResult::NotNeeded);
      CHECK(deferred.StalledMorph(1,a,{1200,0,0},11200)==MorphResult::Pending&&stored.morph[1].projected);
      stored.morph[1]={};h.morphNext=30300;
    }
    CHECK(teleportPhase.ExteriorMorph(0,a,{3000,0,0},40000)==MorphResult::NotNeeded&&h.teleports==1);if(owned.reads>peak)peak=owned.reads;
    owned.reads=0;CHECK(teleportPhase.AnchorAt(1,a));CHECK(teleportPhase.ExteriorMorph(1,a,{3000,0,0},20000)==MorphResult::NotNeeded&&h.projections==2);
    h.projectionOK=false;
    for(unsigned candidate=0;candidate<4;++candidate){owned.reads=0;CHECK(teleportPhase.ExteriorMorph(1,a,{3000,0,0},31000+300*candidate)==MorphResult::Pending);}
    CHECK(stored.morph[1].projectionCandidate==3&&!stored.morph[1].tried&&stored.morph[1].projected&&stored.morph[1].directFallback);
    owned.reads=0;
    CHECK(teleportPhase.ExteriorMorph(1,a,{3000,0,0},33000)==MorphResult::Success&&h.teleports==2);
    CHECK(stored.morph[1].nativeAccepted&&stored.morph[1].arrivalVerified);
    owned.reads=0;stored.morph[1]={};h.projectionOK=true;CHECK(teleportPhase.ExteriorMorph(1,a,{3000,0,0},60000)==MorphResult::Pending);
    auto changedAnchor=a;changedAnchor.position.x=5;CHECK(teleportPhase.ExteriorMorph(1,changedAnchor,{3000,0,0},60300)==MorphResult::Unavailable&&h.teleports==2);
    owned.reads=0;stored.prepared=0;owned.Put(owned.module+disk::CanPlaceTrap.cache,std::uintptr_t{0});
    NativeAccess<Fixture,Bridge> cold(owned,h,input,stored);CHECK(h.Context()&&h.Jason());CHECK(cold.Ready());CHECK(cold.Prepare()==Preparation::Pending);CHECK(owned.reads<1536);if(owned.reads>peak)peak=owned.reads;
    owned.Put(owned.module+disk::CanPlaceTrap.cache,h.probe);
    // A completed-but-displaced arrival, unreadable readback, malformed native
    // result, or ambiguous dispatch cannot trigger another teleport.
    for(unsigned failure=0;failure<5;++failure){
        stored.morph[0]={};h.morphNext=0;h.readbackFails=failure==2;h.teleportResult=failure==3?2:1;h.teleportCallOK=failure!=4;
        h.readbackOffset=failure==0?Vector{201,0,0}:failure==1?Vector{0,0,121}:Vector{};
        owned.reads=0;NativeAccess<Fixture,Bridge> proj(owned,h,input,stored);CHECK(proj.Ready());CHECK(proj.AnchorAt(0,a));
        CHECK(proj.ExteriorMorph(0,a,{3000,0,0},50000)==MorphResult::Pending);
        const auto beforeTeleports=h.teleports;owned.reads=0;NativeAccess<Fixture,Bridge> moved(owned,h,input,stored);CHECK(h.Context()&&h.Jason());CHECK(moved.Ready());CHECK(moved.AnchorAt(0,a));
        CHECK(moved.ExteriorMorph(0,a,{3000,0,0},50300)==MorphResult::Pending&&h.teleports==beforeTeleports+1);
        CHECK(stored.morph[0].tried&&!stored.morph[0].arrivalVerified);
        CHECK(moved.ExteriorMorph(0,a,{3000,0,0},90000)==MorphResult::NotNeeded&&h.teleports==beforeTeleports+1);
        if(owned.reads>peak)peak=owned.reads;
    }
    { // Composed native retry seam: authored front, native false/ambiguity
      // spent once, pure-read refusal unspent, no extra combat Morph clock.
      h.guardReads=150;h.bindingReads=50;h.locationReads=20;owned.cap=1536;owned.reads=0;
      stored.morph[1].localRetryAttempted=0;h.teleportResult=1;h.teleportCallOK=true;h.readbackFails=false;h.readbackOffset={};
      CHECK(h.Context()&&h.Jason());
      NativeAccess<Fixture,Bridge> local(owned,h,input,stored);CHECK(local.AnchorAt(1,a));
      const auto before=h.teleports,reservations=h.morphReservations;
      CHECK(local.RepositionCarRetry(1,a,{315,0,90},1)==MorphResult::Success&&h.teleports==before+1);
      CHECK(h.teleported.x==340&&h.teleported.y==0&&h.teleported.z==90&&stored.morph[1].localRetryAttempted==1);
      CHECK(owned.reads<1536);if(owned.reads>peak)peak=owned.reads;
      owned.reads=0;h.guardReads=0;h.bindingReads=0;h.locationReads=0;
      CHECK(local.RepositionCarRetry(1,a,{340,0,90},1)==MorphResult::NotNeeded&&h.teleports==before+1);
      CHECK(h.morphReservations==reservations); // local geometry retry, not long-range Morph
      owned.reads=0;owned.Put(carActor+cfgvehicle::disk::Driver,input.jason);
      NativeAccess<Fixture,Bridge> occupiedRetry(owned,h,input,stored);
      CHECK(occupiedRetry.RepositionCarRetry(1,a,{340,0,90},2)==MorphResult::Pending&&h.teleports==before+1);
      CHECK(stored.morph[1].localRetryAttempted==1);
      owned.Put(carActor+cfgvehicle::disk::Driver,std::uintptr_t{});
      owned.reads=1536;NativeAccess<Fixture,Bridge> budget(owned,h,input,stored);
      CHECK(budget.RepositionCarRetry(1,a,{340,0,90},2)==MorphResult::Pending&&stored.morph[1].localRetryAttempted==1);
      owned.reads=0;h.teleportResult=0;NativeAccess<Fixture,Bridge> rejected(owned,h,input,stored);
      CHECK(rejected.RepositionCarRetry(1,a,{340,0,90},2)==MorphResult::Pending&&h.teleports==before+2);
      CHECK(rejected.RepositionCarRetry(1,a,{340,0,90},2)==MorphResult::NotNeeded&&h.teleports==before+2);
      CHECK(stored.morph[1].localRetryAttempted==2);
      owned.reads=0;h.teleportResult=1;h.teleportCallOK=false;NativeAccess<Fixture,Bridge> retryUncertain(owned,h,input,stored);
      CHECK(retryUncertain.RepositionCarRetry(1,a,{300,0,90},3)==MorphResult::Pending&&stored.morph[1].localRetryAttempted==3);
      CHECK(retryUncertain.RepositionCarRetry(1,a,{300,0,90},3)==MorphResult::NotNeeded&&h.teleports==before+3);
      CHECK(retryUncertain.RepositionCarRetry(1,a,{9000,0,90},1)==MorphResult::NotNeeded);
    }
    CHECK(peak<1536);std::printf("Startup trap modeled production peak reads=%u/1536\n",peak);
    std::printf("CFG startup traps: %s (%d failures). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
