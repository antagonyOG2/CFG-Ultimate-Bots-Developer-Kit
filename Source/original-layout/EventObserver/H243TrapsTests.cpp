// H2.43 fixtures: (1) police-exit fallback trap targets for maps without
// cars/phone (CFG_H243_TRAPS_NOCAR) and (2) slash while the startup-trap walk
// is running (CFG_H243_TRAP_SLASH).
#include "../Core/StartupTrapRuntime.hpp"
#include "../Core/DonorJasonControllerTick.hpp"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
using namespace cfgtrap;
using namespace cfgtrap::startup;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)

struct ExitFake {
    bool legal=true;unsigned attempted=0;int count=1;Vector position{260,0,85};Anchor anchor{Kind::Exit,{0,0,0},{1,0,0}};
    bool Ready(){return true;} bool Discover(unsigned& n){n=1;return true;}
    bool Count(int& n){n=count;return true;}bool AnchorAt(unsigned,Anchor& a){a=anchor;return true;}
    bool Position(Vector& p){p=position;return true;}
    bool Move(Vector p,unsigned& result){position={p.x,p.y,p.z+85};result=2;return true;}
    bool Stop(){return true;} bool Face(Vector,Vector){return true;}
    bool Probe(CanPlaceParameters& p){p={};p.result=legal?1:0;p.impact={100,0,0};return true;}
    bool Attempt(){++attempted;--count;return true;}
};

struct SlashHost {
    std::string order;
    bool traps=true,turn=true,door=false,pressOK=true,pressReadable=true,moduleReady=true;
    cfgdonorhunt::CombatState combat{};std::uint64_t now=1000;
    cfgdonorhunt::Position jason{0,0,0},counselor{200,0,0};std::uintptr_t target=7;
    unsigned presses=0,releases=0,grabs=0,knives=0,trapPhases=0;
    cfgdonorhunt::CombatState& Combat(){return combat;}
    std::uint64_t Now(){return now;}
    bool ModuleReady(){return moduleReady;}
    bool ReadableActor(std::uintptr_t){return true;}
    bool AttackReleaseReadable(){return true;}
    void ReleaseAttack(){++releases;}
    bool FindNearestSet=true;
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t){return FindNearestSet?target:0;}
    bool GetJasonAIActorLocation(std::uintptr_t a,cfgdonorhunt::Position& p){p=a==target?counselor:jason;return true;}
    bool SelectedInteractableIsSCDoor(){return door;}
    bool AttackPressReadable(){return pressReadable;}
    bool PressAttack(){++presses;seq+="P";return pressOK;}
    bool TryGrab(){++grabs;return true;}
    bool TryStartJasonAIKnifeThrowOnGameThread(std::uintptr_t,float,std::uint64_t){++knives;return false;}
    // controller-tick seams
    bool StartupTrapSetupActive(){return traps;}
    void RunJasonAIStartupTrapSetupOnGameThread(){++trapPhases;}
    bool TrapSlashTurn(){return turn;}
    unsigned faces=0;float faceYaw=-999;std::string seq;
    void FaceJasonAtYaw(float y){++faces;faceYaw=y;seq+="F";}
    bool TickJasonAIDoorBreakOnGameThread(){return false;}
    bool KnifeSequenceActive(){return false;}
    bool RunJasonAIChaseUpdateOnGameThread(){return true;}
    std::uintptr_t Jason(){return 1;}
    void RunJasonAICombatOnGameThread(){}
    bool ReadDriverBlockingStates(std::uintptr_t,std::uint8_t&,std::uint8_t&){return true;}
    bool ReadVerifiedDriverSlot(std::uintptr_t,std::uintptr_t& d){d=0;return true;}
    bool SafeJasonDriverCall(std::uintptr_t,std::uintptr_t){return true;}
    bool DriverTailLogged(){return true;}void SetDriverTailLogged(){}void LogDriverTail(bool){}
    SlashHost(){combat.Jason=1;}
};

int main(){
    // ---- 1. exit fallback gating ----
    CHECK(NoCarFallbackMap(MapFamily::Unknown,true,true));
    CHECK(!NoCarFallbackMap(MapFamily::Unknown,false,true));   // unverified arrival
    CHECK(!NoCarFallbackMap(MapFamily::Unknown,true,false));   // no package proof
    CHECK(!NoCarFallbackMap(MapFamily::GroundMap,true,true));  // ground map keeps cars/phone path
    CHECK(EffectiveFamily(MapFamily::Unknown,true)==MapFamily::GroundMap);
    CHECK(EffectiveFamily(MapFamily::Unknown,false)==MapFamily::Unknown);
    CHECK(MapAllowed(EffectiveFamily(MapFamily::Unknown,true),true));
    CHECK(!MapAllowed(EffectiveFamily(MapFamily::Unknown,false),true));
    CHECK(!MapAllowed(EffectiveFamily(MapFamily::Grendel,false),true));
    // ---- exit geometry ----
    Anchor ex{Kind::Exit,{1000,500,50},{0,1,0}};
    const auto s0=Stand(ex,0),s1=Stand(ex,1);
    CHECK(s0.x==1000&&s0.y==760&&s1.y==800);
    CanPlaceParameters p{};p.result=1;p.impact={1000,600,50};
    CHECK(Legal(ex,p,0));                       // 100 uu toward approach side
    p.impact={1000,400,50};CHECK(!Legal(ex,p,0)); // behind the exit centre
    p.impact={1500,600,50};CHECK(!Legal(ex,p,0)); // lateral miss
    p.impact={1000,600,400};CHECK(!Legal(ex,p,0));// wrong height
    p.impact={1000,600,50};p.result=0;CHECK(!Legal(ex,p,0));
    Anchor car{Kind::Car2,{0,0,0},{1,0,0}};p.result=1;p.impact={100,0,0};
    CHECK(!Legal(car,p,0)&&Legal(Anchor{Kind::Exit,{0,0,0},{1,0,0}},p,0)); // car band unchanged
    // ---- runtime places a trap at an exit ----
    {ExitFake f;Runtime rt;CHECK(rt.Start(1000,MapFamily::GroundMap,true));
     std::uint64_t now=7000;for(unsigned i=0;i<200&&rt.Busy();++i,now+=300)rt.Step(f,now);
     CHECK(f.attempted==1&&rt.report.consumed==1&&rt.report.kinds[0]==static_cast<unsigned>(Kind::Exit));}
    {ExitFake f;f.legal=false;Runtime rt;rt.Start(1000,MapFamily::GroundMap,true);
     std::uint64_t now=7000;for(unsigned i=0;i<400&&rt.Busy();++i,now+=1000)rt.Step(f,now);
     CHECK(f.attempted==0&&!rt.Busy());}        // never an unlegal placement
    {Runtime rt;CHECK(!rt.Start(1000,MapFamily::Unknown,true)&&rt.report.failure==Failure::Map);} // no fallback = unchanged refusal

    // ---- H2.46: all-map phone+cars (shack proven per map by Survey), then exits for leftover traps ----
    // Stand depends only on the anchor transform: translating/rotating the
    // anchor rigidly moves the stand point rigidly (no map constants).
    for(Kind k:{Kind::Phone,Kind::Car2,Kind::Car4})for(unsigned r=0;r<4;++r){
        Anchor a0{k,{0,0,0},{1,0,0}},a1{k,{-52000,31000,777},{0,1,0}};
        const auto t0=Stand(a0,r),t1=Stand(a1,r);
        CHECK(std::fabs(t1.x-(a1.position.x-t0.y))<.01f&&std::fabs(t1.y-(a1.position.y+t0.x))<.01f&&t1.z==a1.position.z);}
    struct MultiFake {
        std::vector<Anchor> anchors;unsigned attempted=0;int count=3;Vector position{-60000,0,0};
        bool Ready(){return true;} bool Discover(unsigned& n){n=static_cast<unsigned>(anchors.size());return true;}
        bool Count(int& n){n=count;return true;}bool AnchorAt(unsigned i,Anchor& a){a=anchors[i];return true;}
        bool Position(Vector& v){v=position;return true;}
        bool Move(Vector p,unsigned& result){position={p.x,p.y,p.z+85};result=2;return true;}
        bool Stop(){return true;} bool Face(Vector,Vector){return true;}
        bool Probe(CanPlaceParameters& p){p={};p.result=1;p.impact=position;return true;}
        bool Attempt(){++attempted;--count;return true;}
    };
    {MultiFake f;f.anchors={{Kind::Phone,{9000,4000,300},{1,0,0}},{Kind::Car2,{12000,-3000,300},{0,1,0}},{Kind::Car4,{-8000,2000,300},{-1,0,0}}};
     Runtime rt;CHECK(rt.Start(1000,EffectiveFamily(MapFamily::Unknown,true),true)&&rt.report.failure==Failure::None);
     std::uint64_t now=7000;for(unsigned i=0;i<2000&&rt.Busy();++i,now+=300)rt.Step(f,now);
     CHECK(rt.report.objectives==3&&rt.report.kinds[0]==static_cast<unsigned>(Kind::Phone)&&
           rt.report.kinds[1]==static_cast<unsigned>(Kind::Car2)&&rt.report.kinds[2]==static_cast<unsigned>(Kind::Car4));
     for(unsigned i=0;i<3;++i)CHECK(rt.report.kinds[i]!=static_cast<unsigned>(Kind::Shack)&&rt.report.kinds[i]!=static_cast<unsigned>(Kind::Exit));}
    // Order phone, cars, then exits for leftover traps only; capacity 4.
    CHECK(ExitSlots(3,3,4)==0&&ExitSlots(4,3,4)==1&&ExitSlots(9,3,4)==1&&ExitSlots(4,0,4)==4&&ExitSlots(2,1,4)==1&&ExitSlots(0,0,4)==0&&ExitSlots(5,4,4)==0);
    {MultiFake f;f.count=4;f.anchors={{Kind::Phone,{9000,4000,300},{1,0,0}},{Kind::Car2,{12000,-3000,300},{0,1,0}},{Kind::Car4,{-8000,2000,300},{-1,0,0}},{Kind::Exit,{-20000,0,300},{0,1,0}}};
     Runtime rt;CHECK(rt.Start(1000,EffectiveFamily(MapFamily::Unknown,true),true));
     std::uint64_t now=7000;for(unsigned i=0;i<3000&&rt.Busy();++i,now+=300)rt.Step(f,now);
     CHECK(rt.report.objectives==4&&rt.report.kinds[3]==static_cast<unsigned>(Kind::Exit)&&rt.report.kinds[0]==static_cast<unsigned>(Kind::Phone));}
    {MultiFake f;f.count=2;f.anchors={{Kind::Car2,{12000,-3000,300},{0,1,0}}}; // missing kinds skipped, no failure
     Runtime rt;CHECK(rt.Start(1000,EffectiveFamily(MapFamily::Unknown,true),true));
     std::uint64_t now=7000;for(unsigned i=0;i<3000&&rt.Busy();++i,now+=300)rt.Step(f,now);
     CHECK(rt.report.objectives==1&&rt.report.failure!=Failure::Source&&rt.report.failure!=Failure::Identity);} // fake probe is not car-legal; discovery of a lone car must not fail
    // ---- H2.48: Grendel escape-objective anchors (flag CFG_H248_GRENDEL_TRAPS) ----
    CHECK(CFG_H248_GRENDEL_TRAPS==1&&ExitLike(Kind::Exit)&&ExitLike(Kind::GrendelEscape)&&!ExitLike(Kind::Phone)&&!ExitLike(Kind::Car2)&&!ExitLike(Kind::Car4)&&!ExitLike(Kind::Shack));
    CHECK(static_cast<unsigned>(Kind::Exit)==4&&static_cast<unsigned>(Kind::GrendelEscape)==5); // existing kind values unchanged
    for(unsigned r=0;r<4;++r){Anchor a{Kind::Exit,{100,200,30},{0,1,0}},b{Kind::GrendelEscape,{100,200,30},{0,1,0}};
        const auto sa=Stand(a,r),sb=Stand(b,r);CHECK(sa.x==sb.x&&sa.y==sb.y&&sa.z==sb.z);}
    {Anchor g{Kind::GrendelEscape,{1000,500,50},{0,1,0}};CanPlaceParameters q{};q.result=1;q.impact={1000,600,50};
     CHECK(Legal(g,q,0));q.impact={1000,400,50};CHECK(!Legal(g,q,0));q.impact={1500,600,50};CHECK(!Legal(g,q,0));q.impact={1000,600,400};CHECK(!Legal(g,q,0));}
    // Order with no shack/phone/cars: Grendel objectives first, police exits for leftover traps.
    {MultiFake f;f.count=3;f.anchors={{Kind::GrendelEscape,{9000,4000,300},{1,0,0}},{Kind::GrendelEscape,{-8000,2000,300},{-1,0,0}},{Kind::Exit,{-20000,0,300},{0,1,0}}};
     Runtime rt;CHECK(rt.Start(1000,EffectiveFamily(MapFamily::Unknown,true),true));
     std::uint64_t now=7000;for(unsigned i=0;i<3000&&rt.Busy();++i,now+=300)rt.Step(f,now);
     CHECK(rt.report.objectives==3&&rt.report.kinds[0]==static_cast<unsigned>(Kind::GrendelEscape)&&rt.report.kinds[1]==static_cast<unsigned>(Kind::GrendelEscape)&&
           rt.report.kinds[2]==static_cast<unsigned>(Kind::Exit)&&f.attempted==3);}
    // ---- 2. slash during trap walk ----
    {SlashHost h;CHECK(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));
     CHECK(h.presses==1&&h.trapPhases==0&&h.grabs==0&&h.knives==0&&h.combat.AttackPressed&&h.combat.AttackReleaseAt==1120);
     // same slice window: cooldown holds, trap phase resumes immediately
     h.now=1050;CHECK(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));
     CHECK(h.presses==1&&h.trapPhases==1);
     // release drains at its deadline, trap resumes, next swing waits the 1050 ms cooldown
     h.now=1130;CHECK(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));
     CHECK(h.releases==1&&!h.combat.AttackPressed&&h.presses==1&&h.trapPhases==2);
     h.now=2100;CHECK(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));
     CHECK(h.presses==2&&h.trapPhases==2);}
    {SlashHost h;h.counselor={261,0,0};cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==0&&h.trapPhases==1);}      // outside 260 uu: keep placing
    {SlashHost h;h.counselor={260,0,0};cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==1);}                       // boundary inclusive, like donor
    {SlashHost h;h.counselor={100,0,0};h.combat.CombatAttemptCounter=0;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.grabs==0&&h.presses==1);}           // donor combat would grab at counter%3==0; trap slash never does
    {SlashHost h;h.turn=false;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==0&&h.trapPhases==1);}      // placement stages (turn=false) never slash
    {SlashHost h;h.door=true;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==0&&h.trapPhases==1);}
    {SlashHost h;h.FindNearestSet=false;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==0&&h.trapPhases==1);}
    {SlashHost h;h.pressOK=false;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==1&&!h.combat.AttackPressed&&h.trapPhases==1);} // failed press does not own the slice
    {SlashHost h;h.combat.GrabKillReadyAt=5000;cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h);
     CHECK(h.presses==0&&h.trapPhases==1);}
    std::printf("H243 traps tests PASS (%u checks)\n",checks);
    return 0;
}
