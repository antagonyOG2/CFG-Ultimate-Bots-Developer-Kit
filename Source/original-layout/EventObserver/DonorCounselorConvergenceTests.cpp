#include "../Core/DonorCounselorConvergence.hpp"
#include "../Core/J21WorkPolicy.hpp"
#include <cstdio>
#include <array>
namespace {
using namespace cfgdonorconvergence;
struct Host {
    bool sweater=false,hosted=false,authority=true,unmasked=false,committed=false,keys=true;
    Pawn actors[4]{1,2,3,4};int count=2;Pawn local=99;
    bool live[5]{false,true,true,true,true},weapons[5]{},ticks[5]{true,true,true,true,true};
    Position positions[5]{};Position jason{1000,0,0};
    unsigned grants=0,fear=0,combat=0,paths=0,tickOff=0,tickOn=0;
    bool strong=false,postGrab=false;unsigned scanned=0;int objectCount=600;
    Pawn grantedClass=0;bool includeAxe=true,includeMachete=true;
    bool HostedOnlineBridge(){return hosted;}bool IsHostedAuthorityWorld(){return authority;}
    bool RegisteredUnmaskedJasonInWorld(){return unmasked;}
    bool PermanentHumanSweaterLatched(){return sweater;}
    bool KillTeamFinalInteractionCommitted(){return committed;}
    int TargetCount(){return count;}Pawn Target(int i){return actors[i];}Pawn LocalCounselorTarget(){return local;}
    bool IsValidatedLiveCounselorPawn(Pawn p){return p>0&&p<5&&live[p];}
    bool HasPamelaSweater(Pawn){return false;}
    bool IsCounselorInPostGrabTransition(Pawn,std::uint64_t){return postGrab;}
    bool ObjectDerivesFromNameContaining(Pawn,const char* token){return strong&&!std::strcmp(token,"Jock");}
    int CurrentObjectCount(){return objectCount;}
    bool ReadClassCandidate(int index,Pawn& object,std::string& name){++scanned;
        if(index==400&&includeAxe){object=0x1000;name="CounselorTwoHandedAxe_C";return true;}
        if(index==512&&includeMachete){object=0x2000;name="CounselorMachete_C";return true;}return false;}
    bool CurrentUClass(Pawn type){return type==0x1000||type==0x2000;}
    bool GiveStartingItem(Pawn,Pawn type){grantedClass=type;return true;}
    bool IsAIControlledCounselor(Pawn p){return p!=local;}
    int Key(const char*){return keys?1:-1;}
    bool GiveConvergenceMeleeWeapon(Pawn p){++grants;weapons[p]=true;return true;}
    void SuppressConvergenceCounselorFear(Pawn){++fear;}
    Pawn GetPawnControllerSafe(Pawn p){return p+100;}
    Pawn GetCounselorBlackboardCached(Pawn p){return p+100;}
    Pawn GetCounselorCurrentWeapon(Pawn p){return weapons[p]?p+1000:0;}
    Pawn GetBlackboardObject(Pawn,int){return 0;}
    void ArmKillTeamCounselorCombat(Pawn){++combat;}
    Pawn GetJasonDeathTarget(){return 1000;}
    bool GetJasonAIActorLocation(Pawn p,Position& out){if(p==1000){out=jason;return true;}if(p<1||p>4)return false;out=positions[p];return true;}
    bool SetCounselorBrainTickEnabled(Pawn p,bool on){ticks[p]=on;if(on)++tickOn;else ++tickOff;return true;}
    bool IssueAIMoveToLocationOnGameThread(Pawn,const Position&,float radius,const char*){if(radius!=100)std::abort();++paths;return true;}
    void SetBlackboardObject(Pawn,int,Pawn){}void SetBlackboardBool(Pawn,int,bool){}
    std::string JasonAISafeName(Pawn){return "fixture";}void Log(const std::string&){}
};
}
int main(){unsigned checks=0,failures=0;const auto check=[&](bool okay,const char* why){++checks;if(!okay){++failures;std::printf("FAIL %s\n",why);}};
    check(!cfgj21::ConvergenceLaneEnabled(false,false),"offline convergence remains disabled by default");
    check(cfgj21::ConvergenceLaneEnabled(false,true),"hosted convergence enabled by hosted authority");
    check(cfgj21::ConvergenceLaneEnabled(true,false),"explicit offline convergence remains enabled");
    {
        State s;Host h;
        s.PrecacheConvergenceWeaponClassesSlice(h,1000);
        check(h.scanned==512&&s.CounselorConvergenceMeleeClass==0x1000&&!s.CounselorConvergenceMacheteClass,"first512 registry slice finds exact axe only");
        s.PrecacheConvergenceWeaponClassesSlice(h,1249);
        check(h.scanned==512,"class scan250ms pacing");
        s.PrecacheConvergenceWeaponClassesSlice(h,1250);
        check(h.scanned==600&&s.CounselorConvergenceMacheteClass==0x2000,"second slice reaches machete after legacy512 prefix");
        check(s.GiveConvergenceMeleeWeapon(h,1,1500)&&h.grantedClass==0x2000,"standard counselor gets machete");
        h.hosted=true;h.grantedClass=0;
        check(s.GiveConvergenceMeleeWeapon(h,1,1500)&&h.grantedClass==0x1000,"hosted standard counselor gets the uniform axe");
        h.hosted=false;h.grantedClass=0;
        h.strong=true;check(s.GiveConvergenceMeleeWeapon(h,1,1500)&&h.grantedClass==0x1000,"strong donor archetype gets axe");
        h.postGrab=true;h.grantedClass=0;
        check(!s.GiveConvergenceMeleeWeapon(h,1,2000)&&!h.grantedClass,"post-grab transition forbids inventory grant");
        State missing;Host only;only.includeMachete=false;missing.PrecacheConvergenceWeaponClassesSlice(only,1000);
        missing.PrecacheConvergenceWeaponClassesSlice(only,1250);
        check(missing.NextConvergenceWeaponClassScanAt==31250&&missing.ConvergenceWeaponClassScanCursor==0,"missing class end-of-table30s retry");
        check(missing.GiveConvergenceMeleeWeapon(only,1,1500)&&only.grantedClass==0x1000,"missing machete falls back to proven loaded axe");
    }
    {
        State s;Host h;
        s.DriveCounselorSweaterConvergence(h,1000);
        check(!s.CounselorConvergenceActive&&s.NextCounselorConvergenceAt==1211&&s.SweaterPreconvergenceCursor==1,"one preconvergence pawn every211ms");
        h.sweater=true;s.DriveCounselorSweaterConvergence(h,1210);
        check(!s.CounselorConvergenceActive,"no early catch-up");
        s.DriveCounselorSweaterConvergence(h,1211);
        check(s.CounselorConvergenceActive&&h.grants==2&&s.ConvergenceArmedCounselorCount==2,"prime all living AI once");
        check(h.tickOff==2&&s.ConvergenceTravelCounselorCount==2&&h.paths==1,"prime brains without group path burst; one selected path");
        check(s.NextCounselorConvergenceAt==1544,"donor333ms cadence");
        s.DriveCounselorSweaterConvergence(h,1544);
        check(h.grants==2&&h.paths==2,"rotate second armed bot without repeat grants");
        s.DriveCounselorSweaterConvergence(h,1877);
        check(h.paths==2,"accepted stationary route not refreshed before6001ms");
        h.positions[1]={790,0,0};h.positions[2]={790,0,0};
        s.DriveCounselorSweaterConvergence(h,2210);s.DriveCounselorSweaterConvergence(h,2543);
        check(!s.ConvergenceTravelCounselorCount&&h.tickOn==2,"within220cm resumes native melee brains");
        h.positions[1]={750,0,0};h.positions[2]={750,0,0};
        s.DriveCounselorSweaterConvergence(h,2876);
        check(!s.ConvergenceTravelCounselorCount,"250cm outside melee-release threshold but inside travel-start hysteresis");
        h.committed=true;s.DriveCounselorSweaterConvergence(h,3209);
        check(s.NextCounselorConvergenceAt==5212,"final interaction commit donor2003ms pause");
    }
    {
        State s;Host h;h.sweater=true;h.hosted=true;h.authority=false;
        s.DriveCounselorSweaterConvergence(h,1000);
        check(!s.CounselorConvergenceActive&&!h.grants,"hosted authority required");
        h.authority=true;h.sweater=false;h.unmasked=true;h.keys=false;
        s.DriveCounselorSweaterConvergence(h,1000);
        check(s.CounselorConvergenceActive&&!h.grants&&s.NextCounselorConvergenceAt==3003,"hosted unmasked activation; required keys defer not fake weapon success");
    }
    {
        State s;Host h;s.CounselorConvergenceActive=true;h.count=0;
        s.DriveCounselorSweaterConvergence(h,1000);
        check(s.NextCounselorConvergenceAt==5513,"empty roster4513ms");
        h.count=2;h.local=1;h.live[2]=false;s.NextCounselorConvergenceAt=0;
        s.DriveCounselorSweaterConvergence(h,6000);
        check(!h.grants&&!h.paths&&!s.ConvergenceArmedCounselorCount,"human/dead counselors excluded");
    }
    {
        State s;Host h;h.count=1;h.weapons[1]=true;s.CounselorConvergenceActive=true;
        s.RememberConvergenceCounselor(1);s.RememberConvergenceTravelCounselor(1);
        s.ConvergenceTravelHaveLocation[0]=true;s.ConvergenceTravelLastProgressAt[0]=1000;
        s.DriveCounselorSweaterConvergence(h,4500);
        check(h.tickOn==1&&!h.paths&&s.ConvergenceTravelRecoveryUntil[0]==5501,"3500ms no-progress opens1001ms native-brain pulse");
        s.DriveCounselorSweaterConvergence(h,4833);
        check(h.tickOn==1&&!h.paths,"no repeated recovery pulse/MoveTo during recovery");
        s.DriveCounselorSweaterConvergence(h,5501);
        check(h.tickOff==1&&h.paths==1&&!s.ConvergenceTravelRecoveryUntil[0],"after pulse reclaim travel and accept one path");
        h.jason={1500,0,0};s.DriveCounselorSweaterConvergence(h,5834);
        check(h.paths==2,"target moved more400cm refreshes path before6001ms");
    }
    {
        State s;Host h;s.RememberConvergenceTravelCounselor(1);s.RememberConvergenceTravelCounselor(2);
        s.ConvergenceTravelLastLocations[1]={1,2,3};s.ConvergenceTravelLastProgressAt[1]=21;
        s.ConvergenceTravelRecoveryUntil[1]=22;s.ConvergenceTravelHaveLocation[1]=true;
        s.ConvergenceTravelLastGoals[1]={4,5,6};s.ConvergenceTravelLastRouteAt[1]=23;s.ConvergenceTravelHaveGoal[1]=true;
        s.ForgetConvergenceTravelCounselor(1);
        check(s.ConvergenceTravelCounselorCount==1&&s.ConvergenceTravelCounselors[0]==2&&s.ConvergenceTravelLastLocations[0].Z==3&&
            s.ConvergenceTravelLastProgressAt[0]==21&&s.ConvergenceTravelRecoveryUntil[0]==22&&s.ConvergenceTravelHaveLocation[0]&&
            s.ConvergenceTravelLastGoals[0].Z==6&&s.ConvergenceTravelLastRouteAt[0]==23&&s.ConvergenceTravelHaveGoal[0],"swap-remove carries every parallel travel field");
        check(!s.ConvergenceTravelCounselors[1]&&!s.ConvergenceTravelLastProgressAt[1]&&!s.ConvergenceTravelRecoveryUntil[1]&&
            !s.ConvergenceTravelHaveLocation[1]&&!s.ConvergenceTravelLastRouteAt[1]&&!s.ConvergenceTravelHaveGoal[1],"removed tail fully cleared");
        s.ResumeAllConvergenceCounselorBrains(h);
        check(h.tickOn==1&&!s.ConvergenceTravelCounselorCount,"resume all current actors then clear records");
    }
    std::printf("Active donor convergence: %u checks, %u failures; exact cadence/roster prime/stagger/hysteresis/recovery/swap-remove; native boundaries supplied fixtures, production host pending.\n",checks,failures);
    return failures?1:0;
}
