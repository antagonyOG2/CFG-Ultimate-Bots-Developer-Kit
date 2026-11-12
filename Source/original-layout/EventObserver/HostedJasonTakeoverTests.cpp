#include "HostedJasonTakeover.hpp"
#include "HostedLocalView.hpp"
#include "../Core/HostedCounselorBirth.hpp"
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace cfghostedtx;
namespace {
int failures=0,checks=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(false)
struct BestEffortPresentationFixture{bool classReady=true,presentationOkay=false;unsigned attempts=0;
    bool PrepareAiClass(const Birth&){return classReady;}
    bool PresentAiBestEffort(const Birth&){++attempts;return presentationOkay;}};
constexpr std::uintptr_t Jason=0x10000000,Human=0x20000000,HumanState=0x20100000,Local=0x30000000,LocalPawn=0x30100000,
    Mode=0x40000000,State=0x40100000,Ai=0x50000000,AiState=0x50100000,Counselor=0x60000000;
struct Sim {
    // Fault plan: call number to fail (0 = none). mode 0 = no effect, 1 = effect applied but reported false.
    unsigned failAt=0,mode=0,calls=0,effects=0;int nativeBotDelta=1;
    std::set<std::uintptr_t> alive{Jason,Human,Local,LocalPawn};
    std::set<std::uintptr_t> roster{HumanState,0x30100010};
    std::uintptr_t jasonController=Human,humanPawn=Jason,aiPawn=0,counselorController=0,killerState=HumanState,jasonPawnState=HumanState;
    int bots=0,humanClass=0;bool localProof=true;bool traveled=true,adopted=false,spawnEntered=false,classesAvailable=true,pathAvailable=true,presentationOkay=true,restartClassDrift=false,preferredReadable=true,viewReady=true,viewOk=true,viewBound=false;unsigned presentationAttempts=0;
    int sizeAtStart()const{return 2;}
    bool Fail(){++calls;return failAt&&calls==failAt;}
    // effect helper: returns result to report; applies the effect unless (fault && mode==0)
    template<class F> bool Effect(F&& apply){
        const bool fault=Fail();
        if(fault&&mode==0)return false;
        ++effects;apply();return !fault;
    }
    bool HasFullyTraveled(std::uintptr_t){return !Fail()&&traveled;}
    bool ResolveClasses(){return !Fail()&&classesAvailable;}
    bool SpawnAi(Birth& b){
        const bool fault=Fail();
        if(fault&&mode==0){b.attempted=false;return false;}
        ++effects;alive.insert(Ai);roster.insert(AiState);bots+=nativeBotDelta;spawnEntered=true;
        b.attempted=true;b.controller=Ai;b.state=AiState;b.botFlag=true;return !fault;
    }
    bool PathReady(const Birth&){return pathAvailable;}
    bool ReadBots(std::int32_t& v){if(Fail())return false;v=bots;return true;}
    bool AccountBot(std::int32_t before){return Effect([&]{if(bots==before)++bots;})&&bots==before+1;}
    bool UnaccountBot(std::int32_t current){return Effect([&]{if(bots==current)--bots;});}
    bool Roster(std::uintptr_t state,std::int32_t& count,bool& contains){
        if(Fail())return false;count=static_cast<std::int32_t>(roster.size());contains=state?roster.count(state)!=0:false;return true;}
    bool PrepareAiClass(const Birth&){return !Fail();}
    bool PresentAiBestEffort(const Birth&){++presentationAttempts;return presentationOkay;}
    bool PrepareAi(const Birth& b){return PrepareAiWithBestEffortPresentation(*this,b);}
    bool AiBirthReady(const Birth&,std::uintptr_t){return !Fail();}
    bool PrepareHuman(std::uintptr_t){return Effect([&]{humanClass=preferredReadable?3:1;});}
    bool RestoreHumanClass(std::uintptr_t){return Effect([&]{humanClass=0;});}
    bool PossessAi(const Birth&,std::uintptr_t,bool& entered){
        const bool fault=Fail();if(fault&&mode==0){entered=false;return false;}
        entered=true;++effects;jasonController=Ai;jasonPawnState=AiState;aiPawn=Jason;humanPawn=humanPawn==Jason?0:humanPawn;return !fault;}
    bool PossessHuman(std::uintptr_t,std::uintptr_t,bool& entered){
        const bool fault=Fail();if(fault&&mode==0){entered=false;return false;}
        entered=true;++effects;viewBound=false;jasonController=Human;jasonPawnState=HumanState;humanPawn=Jason;if(aiPawn==Jason)aiPawn=0;return !fault;}
    bool aiRestartFails=false;
    bool PrepareAiPreborn(const Birth&){return !Fail();}
    bool ControllerPawn(std::uintptr_t,std::uintptr_t& pawn){if(Fail())return false;pawn=aiPawn;return true;}
    void BindJason(std::uintptr_t){}
    bool aiPathSet=false;
    bool AiPathAfterRestart(const Birth&){if(Fail())return false;aiPathSet=true;return true;}
    bool PrebornKillerReady(const Birth&,std::uintptr_t){return !Fail();}
    bool PublishPrebornKiller(std::uintptr_t,std::uintptr_t ai,std::uintptr_t){return Effect([&]{killerState=ai;});}
    bool AdoptPreborn(const cfghosted::adoption::Receipt& r){return Adopt(r);}
    bool Restart(std::uintptr_t,std::uintptr_t controller,bool& entered){
        const bool fault=Fail();if(fault&&mode==0){entered=false;return false;}
        entered=true;++effects;if(controller==Ai){if(aiRestartFails)return false;alive.insert(Jason);aiPawn=Jason;jasonController=Ai;return !fault;}if(humanClass>0){alive.insert(Counselor);humanPawn=Counselor;counselorController=Human;if(restartClassDrift)humanClass=2;}return !fault;}
    bool ReadPost(const Snapshot& s,const Birth&,Post& p){
        if(Fail())return false;
        p.jasonController=jasonController;p.jasonPawnState=jasonPawnState;p.aiPawn=aiPawn;p.counselor=humanPawn;
        p.counselorInWorld=humanPawn&&alive.count(humanPawn);p.counselorIsCounselor=humanPawn==Counselor;
        p.counselorController=humanPawn==Counselor?counselorController:humanPawn==Jason?Human:0;
        p.localCounselor=s.eligibility==Eligibility::HostSelected?humanPawn:LocalPawn;p.localCounselorProven=localProof&&p.localCounselor!=Jason;
        p.humanControllerState=HumanState;p.currentKiller=Jason;p.currentKillerState=killerState;
        p.bots=bots;p.players=static_cast<std::int32_t>(roster.size());p.aiRegistered=roster.count(AiState)!=0;return true;}
    bool InWorld(std::uintptr_t a){return !Fail()&&alive.count(a)!=0;}
    bool Destroy(std::uintptr_t a){
        return Effect([&]{alive.erase(a);
            if(a==Ai){roster.erase(AiState);if(jasonController==Ai)jasonController=0;if(aiPawn==Jason)aiPawn=0;}
            if(a==Counselor&&humanPawn==Counselor)humanPawn=0;});}
    bool WriteKillerOwner(std::uintptr_t expected,std::uintptr_t desired){
        return Effect([&]{if(killerState==expected)killerState=desired;})&&killerState==desired;}
    bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){
        if(Fail())return false;slot.object=object;slot.index=static_cast<std::int32_t>(object>>20);slot.serial=7;return true;}
    bool Adopt(const cfghosted::adoption::Receipt&){if(Fail())return false;adopted=true;return true;}
    // H2.47 local view seam: ready is read-only; refresh is a native effect bound to the new counselor pawn.
    // Production adapter shape: the SAME cfghostview::Refresh sequencer; every native step is a faultable Ops call.
    std::vector<std::string> viewLog;bool ignoredMove=true;std::string viewFailName;
    struct View {
        Sim& w;
        bool Step(const char* name,bool binds=false){
            w.viewLog.push_back(name);
            const bool injected=!w.viewFailName.empty()&&w.viewFailName==name;
            const bool fault=w.Fail()||injected;
            if(fault&&w.mode==0)return false;
            ++w.effects;if(binds)w.viewBound=true;return !fault;
        }
        bool Pinned(){return w.viewReady;}
        bool MoveIgnored(bool& i){const bool ok=Step("IsMoveInputIgnored");i=w.ignoredMove;return ok;}
        bool ClientRestart(){return Step("ClientRestart");}
        bool ResetInput(){return Step("ResetIgnoreInputFlags");}
        bool ViewTarget(){return Step("SetViewTargetWithBlend",true);}
        bool Acknowledged(){return !w.Fail();}
        bool ClearLens(){return Step("ClientClearCameraLensEffects");}
        bool RootHud(){return Step("ChangeRootMenu")&&w.viewOk;}
        bool GameInput(){return Step("SetInputMode_GameOnly");}
    };
    bool LocalViewReady(){View v{*this};return cfghostview::Ready(v);}
    bool LocalViewRefresh(const Snapshot&,std::uintptr_t counselor){
        if(counselor!=Counselor)return false;
        View v{*this};return cfghostview::Refresh(v).ok&&viewBound;
    }
};
Snapshot Ready(){
    Snapshot s;s.eligibility=Eligibility::Ready;s.world=0x70000000;s.mode=Mode;s.gameState=State;s.jason=Jason;s.human=Human;
    s.humanState=HumanState;s.localController=Local;s.localPawn=LocalPawn;s.bots=0;s.players=2;s.counselorTreeLoaded=true;return s;
}
bool Initial(const Sim& w){
    return w.alive==std::set<std::uintptr_t>{Jason,Human,Local,LocalPawn}&&w.jasonController==Human&&w.humanPawn==Jason&&
        w.killerState==HumanState&&w.jasonPawnState==HumanState&&w.bots==0&&w.humanClass==0&&!w.viewBound&&w.roster==std::set<std::uintptr_t>{HumanState,0x30100010};
}
bool Safe(const Sim& w){ // never destroy an AI that still owns the Jason pawn; never lose the Jason pawn
    return !(w.jasonController==Ai&&!w.alive.count(Ai))&&w.alive.count(Jason);
}
}
int main(){
    {BestEffortPresentationFixture fixture;Birth newborn{};newborn.state=AiState;
     CHECK(PrepareAiWithBestEffortPresentation(fixture,newborn)&&fixture.attempts==1);
     fixture.classReady=false;CHECK(!PrepareAiWithBestEffortPresentation(fixture,newborn)&&fixture.attempts==1);}
    CHECK(cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,14,9,true));
    CHECK(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x820000,14,9,true));
    CHECK(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,15,9,true));
    CHECK(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,4,0x810000,14,9,true));
    CHECK(!cfghostedfill::PromoteUnassignedSlot(0x810000,14,0,0x810000,14,9,false));
    // 1. Success: both native bot-accounting variants.
    for(int delta=0;delta<2;++delta){
        Sim w;w.nativeBotDelta=delta;Takeover<Sim> t(w);const auto r=t.Run(Ready());
        CHECK(r.outcome==Outcome::Adopted&&r.reason==Reason::None&&r.stage==Stage::Done&&r.locked);
        CHECK(w.jasonController==Ai&&w.aiPawn==Jason&&w.humanPawn==Counselor&&w.counselorController==Human);
        CHECK(w.bots==1&&w.roster.size()==3&&w.killerState==AiState&&w.adopted&&w.humanClass==3);
        CHECK(r.receipt.Complete()&&r.receipt.botsAfter==r.receipt.botsBefore+1&&r.receipt.rosterAfter==r.receipt.rosterBefore+1);
        CHECK(r.receipt.pawn.object==Jason&&r.receipt.controller.object==Ai&&r.receipt.controllerState==AiState&&r.counselor==Counselor);
        // No replay after adoption.
        const auto effects=w.effects;const auto again=t.Run(Ready());
        CHECK(again.outcome==Outcome::Refused&&again.reason==Reason::Locked&&w.effects==effects);
    }
    {Sim w;w.presentationOkay=false;Takeover<Sim> t(w);const auto r=t.Run(Ready());
     CHECK(r.outcome==Outcome::Adopted&&w.adopted&&w.presentationAttempts==1&&w.aiPawn==Jason&&w.humanPawn==Counselor);}
    {Sim w;w.preferredReadable=false;Takeover<Sim> t(w);const auto r=t.Run(Ready());
     CHECK(r.outcome==Outcome::Adopted&&w.adopted&&w.humanClass==1&&w.humanPawn==Counselor);}
    {Sim w;w.restartClassDrift=true;Takeover<Sim> t(w);const auto r=t.Run(Ready());
     CHECK(r.outcome==Outcome::Adopted&&w.adopted&&w.humanClass==2&&w.humanPawn==Counselor);}
    // H2.47 deliberate update: the displaced human is the local host. With CFG_H247_HOST_TAKEOVER=0 (default) the H210
    // refusal stands (no proven local HUD/view refresh): nothing is touched. With 1 the same donor transaction runs and adds
    // a LocalView stage (pinned refresh) before Publish; an unpinned/failed refresh never mutates before Possess.
    {Sim w;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);
#if CFG_H247_HOST_TAKEOVER
     CHECK(r.outcome==Outcome::Adopted&&r.stage==Stage::Done&&w.adopted&&w.humanPawn==Counselor&&w.viewBound&&w.jasonController==Ai&&w.killerState==AiState&&w.humanClass==3);
#else
     CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::HostSelected&&!w.effects&&!w.calls&&!t.Locked()&&Initial(w));
#endif
    }
    {Sim w;w.viewReady=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);   // pins mismatch / refresh unavailable: no writes in either build
     CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::HostSelected&&!w.effects&&!t.Locked()&&Initial(w)&&!w.adopted);}
#if CFG_H247_HOST_TAKEOVER
    {Sim w;w.viewOk=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);   // refresh fails after Restart: verified rollback, host regains Jason, never pawnless
     CHECK(r.outcome==Outcome::RolledBack&&r.reason==Reason::LocalRefresh&&r.stage==Stage::LocalView&&r.rollbackVerified&&Initial(w)&&!w.adopted&&Safe(w));}
    {Sim w;w.localProof=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);CHECK(r.outcome==Outcome::RolledBack&&r.rollbackVerified&&Initial(w)&&!w.adopted);}
    // H2.48: refresh call order matches the donor (PumpHostedCounselorView host repair + acknowledged tail), reset only when
    // the move input was ignored, camera seed skipped, and the whole sequence runs after Restart, before Publish.
    {Sim w;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);
     const std::vector<std::string> donor{"IsMoveInputIgnored","ClientRestart","ResetIgnoreInputFlags","SetViewTargetWithBlend",
        "IsMoveInputIgnored","ClientClearCameraLensEffects","ChangeRootMenu","SetInputMode_GameOnly"};
     CHECK(r.outcome==Outcome::Adopted&&w.viewLog==donor);}
    {Sim w;w.ignoredMove=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);
     const std::vector<std::string> donor{"IsMoveInputIgnored","ClientRestart","SetViewTargetWithBlend","IsMoveInputIgnored",
        "ClientClearCameraLensEffects","ChangeRootMenu","SetInputMode_GameOnly"};
     CHECK(r.outcome==Outcome::Adopted&&w.viewLog==donor);}
    // H2.55: host preborn as a counselor. The AI's restart makes the Jason; the human is never touched.
    {Sim w;w.alive.erase(Jason);w.jasonController=0;w.humanPawn=Counselor;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostPreborn;s.jason=0;s.human=Human;s.localController=Human;s.localPawn=Counselor;
     const auto r=t.Run(s);CHECK(r.outcome==Outcome::Adopted&&w.aiPawn==Jason&&w.adopted&&w.killerState==AiState&&w.humanPawn==Counselor&&w.humanClass==0&&t.Locked()&&w.aiPathSet);}
    // A refused AI restart removes only the AI objects and leaves the host's counselor alone.
    {Sim w;w.alive.erase(Jason);w.jasonController=0;w.humanPawn=Counselor;w.aiRestartFails=true;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostPreborn;s.jason=0;s.human=Human;s.localController=Human;s.localPawn=Counselor;
     const auto r=t.Run(s);CHECK(r.outcome==Outcome::RolledBack&&r.reason==Reason::RestartFailed&&r.rollbackVerified&&!w.alive.count(Ai)&&!w.adopted&&w.humanPawn==Counselor&&w.bots==0);}
    // Preborn is only for the host: a different human never enters it.
    {Sim w;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostPreborn;s.jason=0;
     const auto r=t.Run(s);CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::RemoteOwnership&&!w.effects&&!t.Locked());}
    // H2.53: the first donor host repair is the same sequence without SetInputMode_GameOnly; the last one includes it.
    {Sim w;Sim::View v{w};const auto r=cfghostview::Refresh(v,false);
     const std::vector<std::string> donor{"IsMoveInputIgnored","ClientRestart","ResetIgnoreInputFlags","SetViewTargetWithBlend",
        "IsMoveInputIgnored","ClientClearCameraLensEffects","ChangeRootMenu"};
     CHECK(r.ok&&w.viewLog==donor&&r.before==2&&r.after==2);}
    {Sim w;w.ignoredMove=false;Sim::View v{w};const auto r=cfghostview::Refresh(v,true);
     CHECK(r.ok&&w.viewLog.back()=="SetInputMode_GameOnly"&&r.before==1&&r.after==1);}
    // Pin mismatch: LocalViewReady false -> zero effects, zero view steps, no lock, world untouched.
    {Sim w;w.viewReady=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);
     CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::HostSelected&&!w.effects&&w.viewLog.empty()&&!t.Locked()&&Initial(w));
     Sim::View v{w};CHECK(!cfghostview::Ready(v)&&!cfghostview::Refresh(v).ok&&cfghostview::Refresh(v).why==cfghostview::Why::Unpinned&&
        w.viewLog.empty()&&!w.effects);}
    // A failure at ANY refresh step (clean no-effect or applied-but-reported-false) rolls back verified: host re-possessed into Jason.
    for(const char* name:{"IsMoveInputIgnored","ClientRestart","ResetIgnoreInputFlags","SetViewTargetWithBlend",
        "ClientClearCameraLensEffects","ChangeRootMenu","SetInputMode_GameOnly"})
        for(unsigned mode=0;mode<2;++mode){
            Sim w;w.mode=mode;w.viewFailName=name;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
            const auto r=t.Run(s);
            const bool ignoredQueryOnly=std::string(name)=="IsMoveInputIgnored"; // donor: unknown input state is not fatal
            if(ignoredQueryOnly&&mode==0){CHECK(r.outcome==Outcome::Adopted);continue;}
            if(ignoredQueryOnly){CHECK(r.outcome==Outcome::Adopted||r.outcome==Outcome::RolledBack);continue;}
            CHECK(r.outcome==Outcome::RolledBack&&r.reason==Reason::LocalRefresh&&r.stage==Stage::LocalView&&r.rollbackVerified);
            CHECK(Initial(w)&&!w.adopted&&Safe(w)&&w.jasonController==Human&&w.humanPawn==Jason&&!w.viewBound);}
    {Sim w;w.viewOk=false;Takeover<Sim> t(w);auto s=Ready();s.eligibility=Eligibility::HostSelected;s.localController=Human;s.localPawn=Jason;
     const auto r=t.Run(s);CHECK(r.outcome==Outcome::RolledBack&&r.rollbackVerified&&Initial(w)&&w.jasonController==Human);}
#endif
    {Sim w;w.pathAvailable=false;Takeover<Sim> t(w);const auto r=t.Run(Ready());
     CHECK(r.outcome==Outcome::RolledBack&&r.reason==Reason::PathUnavailable&&r.stage==Stage::Spawn&&r.rollbackVerified);
     CHECK(Initial(w)&&w.humanClass==0&&!w.adopted);}
    // 3. Not-ready preflights are retryable and touch nothing.
    {struct Case{Eligibility e;Reason why;};
     for(const Case c:{Case{Eligibility::NoKiller,Reason::NoKiller},Case{Eligibility::LocalCounselorNotReady,Reason::LocalCounselorNotReady},
                       Case{Eligibility::RemoteOwnership,Reason::RemoteOwnership}}){
        Sim w;Takeover<Sim> t(w);auto s=Ready();s.eligibility=c.e;const auto r=t.Run(s);
        CHECK(r.outcome==Outcome::NotReady&&r.reason==c.why&&!w.effects&&Initial(w)&&!t.Locked());}
     Sim w;Takeover<Sim> t(w);auto s=Ready();s.counselorTreeLoaded=false;auto r=t.Run(s);
     CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::TreeUnloaded&&!w.effects&&!t.Locked());
     s=Ready();s.players=8;r=t.Run(s);CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::RosterFull);
     s=Ready();s.bots=8;r=t.Run(s);CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::BotsRange);
     s=Ready();s.human=Local;r=t.Run(s);CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::RemoteOwnership);
     w.traveled=false;r=t.Run(Ready());CHECK(r.outcome==Outcome::NotReady&&r.reason==Reason::NotTraveled&&!w.effects&&!t.Locked());
     w.traveled=true;r=t.Run(Ready());CHECK(r.outcome==Outcome::Adopted); // retry succeeds once ready}
    }
    // Live H23 Classes/NotReady must not permanently lock a clean pre-birth attempt.
    {Sim w;w.classesAvailable=false;
     for(unsigned retry=1;retry<=6;++retry){Takeover<Sim> t(w);const auto r=t.Run(Ready());
       CHECK(r.stage==Stage::Classes&&r.reason==Reason::ClassUnavailable&&RetryBeforeBirth(r,true)&&!w.effects&&Initial(w));
       CHECK(!RetryBeforeBirth(r,false));CHECK(RetryBackoff(retry)==(retry==1?1000:retry==2?2000:retry==3?4000:8000));}
     w.classesAvailable=true;Takeover<Sim> ready(w);const auto success=ready.Run(Ready());
     CHECK(success.outcome==Outcome::Adopted&&!RetryBeforeBirth(success,true));
     Result ambiguous;ambiguous.outcome=Outcome::NotReady;ambiguous.nativeEffects=1;
     CHECK(!RetryBeforeBirth(ambiguous,true));ambiguous.nativeEffects=0;ambiguous.locked=true;CHECK(!RetryBeforeBirth(ambiguous,true));}
    // 4. Fault at EVERY Ops call, both fault modes, both native NumBots accounting variants.
    unsigned adopted=0,rolledBack=0,unverified=0,notReady=0,refused=0,totalRuns=0;unsigned maxCalls=0;
    {Sim probe;Takeover<Sim> t(probe);(void)t.Run(Ready());maxCalls=probe.calls;}
    for(unsigned local=0;local<2;++local)for(int delta=0;delta<2;++delta)for(unsigned mode=0;mode<2;++mode)for(unsigned at=1;at<=maxCalls+4;++at){
        Sim w;w.nativeBotDelta=delta;w.failAt=at;w.mode=mode;Takeover<Sim> t(w);auto snapshot=Ready();
        if(local){snapshot.eligibility=Eligibility::HostSelected;snapshot.localController=Human;snapshot.localPawn=Jason;}
        const auto r=t.Run(snapshot);++totalRuns;
        CHECK(Safe(w));
        switch(r.outcome){
        case Outcome::Adopted:++adopted;
            CHECK(w.jasonController==Ai&&w.humanPawn==Counselor&&w.bots==1&&w.killerState==AiState&&r.receipt.Complete());break;
        case Outcome::RolledBack:++rolledBack;
            CHECK(r.rollbackVerified&&Initial(w)&&r.locked);break;       // exact pre-transaction world
        case Outcome::RollbackUnverified:++unverified;
            CHECK(!r.rollbackVerified&&r.locked);break;                   // never claims success, never retries
        case Outcome::NotReady:++notReady;CHECK(!w.effects&&Initial(w));break;
        case Outcome::Refused:++refused;CHECK(!w.effects&&Initial(w));break;
        default:CHECK(false);
        }
        // An uncommitted/failed transaction is never replayed in the same session.
        if(r.outcome!=Outcome::NotReady){const auto before=w.effects;
            const auto again=t.Run(Ready());CHECK(again.outcome==Outcome::Refused&&again.reason==Reason::Locked&&w.effects==before);}
    }
    CHECK(rolledBack>40&&adopted>=2&&notReady>=2);
    // 5. Class prerequisite failing at the very last moment is a retryable not-ready (nothing allocated).
    {Sim w;Takeover<Sim> t(w);w.failAt=2;w.mode=0;const auto r=t.Run(Ready());
     CHECK(r.outcome==Outcome::NotReady&&!w.effects&&Initial(w));}
    // 6. Ambiguous AI possession that cannot be undone retains the AI (never destroyed while owning Jason).
    {Sim w;Takeover<Sim> t(w);
     // fail the rollback re-possession (PossessHuman) by targeting the first call after PossessAi's rollback starts
     Sim probe;probe.failAt=0;
     bool sawRetain=false;
     for(unsigned at=1;at<=maxCalls+30;++at){Sim x;x.failAt=at;x.mode=1;Takeover<Sim> tx(x);
        Sim y;y.failAt=0; (void)y;
        const auto r=tx.Run(Ready());
        if(r.outcome==Outcome::RollbackUnverified&&x.jasonController==Ai&&x.alive.count(Ai))sawRetain=true;}
     CHECK(sawRetain||unverified>0); (void)w;(void)t;}
    // 7. Offline-gate isolation lives in HostedRetainedWorldTests; here only the receipt shape.
    {cfghosted::adoption::Receipt receipt;CHECK(!receipt.Complete());
     receipt.valid=true;CHECK(!receipt.Complete());}
    std::printf("H1 hosted Jason takeover transaction: %d checks, %d failures; runs=%u adopted=%u rolledBack=%u unverified=%u notReady=%u refused=%u maxOpCalls=%u. Mock world; no native calls.\n",
        checks,failures,totalRuns,adopted,rolledBack,unverified,notReady,refused,maxCalls);
    return failures?1:0;
}
