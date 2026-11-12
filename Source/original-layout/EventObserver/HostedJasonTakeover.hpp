#pragma once
#include "HostedAdoption.hpp"
#include <cstdint>

// H1 hosted Jason takeover: function-for-function transcription of the donor rc1
// OfflineSetup.cpp:10155 TakeOverHostedJason decision/rollback logic, with every
// native effect behind an Ops seam so the SAME transaction is fixture-proven
// (mock world, fault at every step) and then driven by the production adapter.
//
// Donor order, preserved:
//   preflight (read-only) -> [HasFullyTraveled getter] -> class resolution ->
//   spawn killer AI controller (+PlayerState) -> bot flag check -> NumBots delta
//   (1 = native constructor accounted it; 0 = account it ourselves; else rollback) ->
//   roster +1 -> AI active-class/presentation -> AI native-birth readiness ->
//   human active class := counselor -> AI Possess(jason) -> readiness with jason ->
//   Hunt RestartPlayer(human) -> ownership/roster/NumBots postconditions ->
//   publish CurrentKillerState := AI state -> adoption receipt.
// Any failure after the first native effect runs the verified rollback and LOCKS
// the session: an ambiguous native call is never replayed. Local and remote
// selected humans use the same restart/rollback; local counselor is proven AFTER restart.
// H2.47: local-host takeover (the displaced human is the listen host). Off by default: the
// donor rc1 refused this branch after live tests left the host with a Jason HUD/camera, and
// H2.9 reproduced it on 3.5 (MAILBOX #506). It may be enabled only when the production Ops
// can pin every native local view-refresh step (LocalViewReady) and prove the refresh live.
#ifndef CFG_H247_HOST_TAKEOVER
#define CFG_H247_HOST_TAKEOVER 1 // H2.48: every local-view native is byte-pinned (HostedLocalViewMembers.hpp); refresh failure -> verified rollback
#endif
#ifndef CFG_H255_HOST_AI_BIRTH
#define CFG_H255_HOST_AI_BIRTH 1 // H2.55: with the host preborn as a counselor, the AI gets its own controller, the lottery's Jason class, and a native RestartPlayer birth
#endif
#ifndef CFG_H260_PREBORN_DEFER
#define CFG_H260_PREBORN_DEFER 1 // restore the host counselor 5 s into the 16 s intro so remote clients still see the lottery Jason
#endif
#ifndef CFG_H254_HOST_PREBORN
#define CFG_H254_HOST_PREBORN 1 // H2.54: host wins the lottery (intro keeps the Jason), then its ActiveCharacterClass goes back to its counselor before any pawn is born
#endif
#ifndef CFG_H253_HOST_VIEW_REPAIR
#define CFG_H253_HOST_VIEW_REPAIR 1 // H2.53: donor PumpHostedCounselorView's two post-takeover host repairs (+2 s, +20 s; SetInputMode on the last)
#endif
#ifndef CFG_H252_EMPTY_MENU_STACK
#define CFG_H252_EMPTY_MENU_STACK 1 // H2.52: an empty or missing HUD menu stack means "no current menu" (donor GetCurrentMenu null), not a refusal
#endif
#ifndef CFG_H252_RESERVE_BY_OPTION
#define CFG_H252_RESERVE_BY_OPTION 1 // H2.52b: deferred bot fill keeps the takeover slot free whenever the AI Jason option is on (host-as-Jason arms late)
#endif
namespace cfghostedtx {
enum class Outcome : unsigned { None, NotReady, Refused, Adopted, RolledBack, RollbackUnverified };
enum class Reason : unsigned {
    None, NoKiller, HostSelected, LocalCounselorNotReady, RemoteOwnership, BotsRange, RosterFull, TreeUnloaded,
    NotTraveled, ClassUnavailable, Locked, SpawnFailed, BotFlag, BotDelta, RosterAccounting, AiPrepare, AiBirth,
    HumanPrepare, PossessFailed, RestartFailed, Postconditions, PublishFailed, AdoptFailed, NativeException, PathUnavailable, LocalRefresh };
enum class Stage : unsigned { Idle, Preflight, Travel, Classes, Spawn, Account, AiPrepare, AiBirth, HumanPrepare,
    Possess, Restart, Postconditions, Publish, Adopt, Done, LocalView };
enum class Eligibility : unsigned { None, NoKiller, HostSelected, LocalCounselorNotReady, RemoteOwnership, Ready, HostPreborn };

// Read-only facts from the H1 observation frame (takeover35::Frame) plus the native
// loaded-tree evidence. Identity here is sampled, NOT retention; the production
// adapter must root every input before the first allocating call.
struct Snapshot {
    Eligibility eligibility=Eligibility::None;
    std::uintptr_t world=0,mode=0,gameState=0;
    std::uintptr_t jason=0,human=0,humanState=0,localController=0,localPawn=0;
    std::int32_t bots=-1,players=-1;
    bool counselorTreeLoaded=false;
};
struct Birth {                       // result of the native spawn effect
    bool attempted=false;            // allocation/dispatch was entered (possibly ambiguous)
    std::uintptr_t controller=0,state=0;
    bool botFlag=false;              // PlayerState bIsABot (native constructor path)
};
template<class Ops>bool PrepareAiWithBestEffortPresentation(Ops& ops,const Birth& birth){
    if(!ops.PrepareAiClass(birth))return false;
    (void)ops.PresentAiBestEffort(birth); // cosmetics cannot veto a successful AI takeover
    return true;
}
struct Post {                        // ownership facts after the native effects
    std::uintptr_t jasonController=0;        // jason.Controller
    std::uintptr_t jasonPawnState=0;         // jason.PlayerState (APawn::PossessedBy copies the controller state)
    std::uintptr_t aiPawn=0;                 // ai.Pawn
    std::uintptr_t counselor=0;              // human.Pawn
    bool counselorInWorld=false,counselorIsCounselor=false;
    std::uintptr_t localCounselor=0;bool localCounselorProven=false;
    std::uintptr_t counselorController=0;    // counselor.Controller
    std::uintptr_t humanControllerState=0;   // human.PlayerState
    std::uintptr_t currentKiller=0,currentKillerState=0;
    std::int32_t bots=-1,players=-1;
    bool aiRegistered=false;
};
struct Result {
    Outcome outcome=Outcome::None;Stage stage=Stage::Idle;Reason reason=Reason::None;
    bool rollbackVerified=false,locked=false;
    unsigned nativeEffects=0;
    cfghosted::adoption::Receipt receipt{};
    std::uintptr_t counselor=0;
};

inline bool RetryBeforeBirth(const Result& result,bool leaseCurrent){return leaseCurrent&&result.outcome==Outcome::NotReady&&!result.locked&&!result.nativeEffects;}
inline std::uint64_t RetryBackoff(unsigned count){return 1000ULL<<(count<2?0:count<3?1:count<4?2:3);}

// Ops contract (all return false on refusal; native effects set `entered` ambiguity
// through the explicit out-parameters so a failed call is never silently replayed):
//   bool HasFullyTraveled(std::uintptr_t humanState)
//   bool ResolveClasses()                                   // killer soft class == jason class; counselor class loaded
//   bool SpawnAi(Birth&)                                    // native controller + PlayerState (+crowd bypass)
//   bool PathReady(const Birth&)                            // current typed/owned path component, before human mutation
//   bool ReadBots(std::int32_t&)
//   bool AccountBot(std::int32_t before)                    // compare-exchange before -> before+1
//   bool UnaccountBot(std::int32_t current)                 // compare-exchange current -> current-1
//   bool Roster(std::uintptr_t state,std::int32_t& count,bool& contains)
//   bool PrepareAi(const Birth&)                            // active class; presentation copy is best effort
//   bool AiBirthReady(const Birth&,std::uintptr_t jasonOrZero)
//   bool PrepareHuman(std::uintptr_t humanState)            // active class := counselor
//   bool RestoreHumanClass(std::uintptr_t humanState)       // active class := killer
//   bool PossessAi(const Birth&,std::uintptr_t jason,bool& entered)
//   bool PossessHuman(std::uintptr_t human,std::uintptr_t jason,bool& entered)
//   bool Restart(std::uintptr_t mode,std::uintptr_t human,bool& entered)
//   bool ReadPost(const Snapshot&,const Birth&,Post&)
//   bool InWorld(std::uintptr_t actor)
//   bool Destroy(std::uintptr_t actor)
//   bool WriteKillerOwner(std::uintptr_t expected,std::uintptr_t desired)   // guarded exchange of CurrentKillerState
//   bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation&)
//   bool Adopt(const cfghosted::adoption::Receipt&)
//   CFG_H247_HOST_TAKEOVER only (local host as displaced human):
//   bool LocalViewReady()                                   // read-only: every native refresh step is pinned; else the host-picked refusal stands
//   bool LocalViewRefresh(const Snapshot&,std::uintptr_t counselor)  // local controller view/HUD/input bound to the new counselor pawn
template<class Ops> class Takeover {
    Ops& o_;bool locked_=false;
public:
    explicit Takeover(Ops& ops):o_(ops){}
    bool Locked() const noexcept { return locked_; }
    Result Run(const Snapshot& s){
        Result out;out.stage=Stage::Preflight;
        const auto notReady=[&](Reason why){out.outcome=Outcome::NotReady;out.reason=why;return out;};
        if(locked_){out.outcome=Outcome::Refused;out.reason=Reason::Locked;out.locked=true;return out;}
        switch(s.eligibility){
#if CFG_H247_HOST_TAKEOVER
        case Eligibility::HostSelected:if(!o_.LocalViewReady())return notReady(Reason::HostSelected);break; // read-only; refuses BEFORE all native effects
#else
        case Eligibility::HostSelected:return notReady(Reason::HostSelected); // H210: no proven local HUD/view refresh; refuse BEFORE all native effects
#endif
#if CFG_H255_HOST_AI_BIRTH
        case Eligibility::HostPreborn:return RunPreborn(s);
#endif
        case Eligibility::NoKiller:return notReady(Reason::NoKiller);
        case Eligibility::LocalCounselorNotReady:return notReady(Reason::LocalCounselorNotReady);
        case Eligibility::RemoteOwnership:return notReady(Reason::RemoteOwnership);
        case Eligibility::Ready:break;
        default:return notReady(Reason::NoKiller);
        }
        if(!s.jason||!s.human||!s.humanState||!s.localController||!s.localPawn||!s.world||!s.mode||!s.gameState||
           (s.eligibility==Eligibility::HostSelected?(s.jason!=s.localPawn||s.human!=s.localController):(s.jason==s.localPawn||s.human==s.localController)))return notReady(Reason::RemoteOwnership);
        if(s.bots<0||s.bots>7)return notReady(Reason::BotsRange);
        if(s.players<0||s.players>=8)return notReady(Reason::RosterFull);   // donor: beforePlayers >= 8
        if(!s.counselorTreeLoaded)return notReady(Reason::TreeUnloaded);     // donor rejects before spawn
        out.stage=Stage::Travel;
        if(!o_.HasFullyTraveled(s.humanState))return notReady(Reason::NotTraveled);   // pre-birth retry
        out.stage=Stage::Classes;
        if(!o_.ResolveClasses())return notReady(Reason::ClassUnavailable);
        // ---- first allocating native effect: from here on every exit locks the session ----
        locked_=true;out.locked=true;
        Birth birth;bool botAccounted=false,humanChanged=false,adoptionTried=false;
        std::int32_t beforeBots=s.bots;
        std::uintptr_t counselor=0;bool aiPossessEntered=false;
        const auto rollback=[&](Reason why,Stage at){
            out.reason=why;out.stage=at;
            // Verified rollback (donor lambda): restore ownership first, then destroy only
            // actors we own, then repair the roster and NumBots. An ambiguous AI possession
            // keeps the AI (never destroy a controller that may still own the Jason pawn).
            bool restored=true;
            if(humanChanged){
                // The human may already hold a restarted counselor pawn: learn it BEFORE the
                // re-possession displaces it, so the cleanup destroys exactly that pawn.
                Post pre;
                if(!counselor&&o_.ReadPost(s,birth,pre)&&pre.counselor&&pre.counselor!=s.jason&&
                   pre.counselorIsCounselor&&pre.counselorController==s.human)counselor=pre.counselor;
                bool entered=false;
                restored=o_.RestoreHumanClass(s.humanState);
                // Re-possess only if the AI possession was actually entered (otherwise the human never lost the Jason).
                if(aiPossessEntered)restored=o_.PossessHuman(s.human,s.jason,entered)&&restored;
                Post now;
                restored=restored&&o_.ReadPost(s,birth,now)&&now.jasonController==s.human&&now.jasonPawnState==s.humanState&&
                    (!birth.controller||now.aiPawn!=s.jason);
                // The donor re-publishes the original killer owner before re-possessing.
                restored=restored&&(now.currentKillerState==s.humanState||o_.WriteKillerOwner(now.currentKillerState,s.humanState));
            }
            if(restored&&counselor&&counselor!=s.jason)restored=o_.InWorld(counselor)&&o_.Destroy(counselor);
            if(restored&&birth.controller)restored=o_.InWorld(birth.controller)&&o_.Destroy(birth.controller);
            if(!restored&&aiPossessEntered)out.reason=why; // ownership uncertain: retain, never replay
            std::int32_t count=-1;bool contains=false;
            const bool listed=birth.state&&o_.Roster(birth.state,count,contains);
            const bool stillRegistered=listed&&contains;
            const bool rosterRestored=!stillRegistered&&o_.Roster(0,count,contains)&&count==s.players;
            std::int32_t bots=-1;
            const bool botsRead=o_.ReadBots(bots);
            if(restored&&rosterRestored&&botAccounted&&botsRead&&bots==beforeBots+1){
                if(o_.UnaccountBot(bots))bots=beforeBots;
            }
            Post final;
            const bool botsOkay=o_.ReadBots(bots)&&bots==beforeBots;
            const bool killerSame=o_.ReadPost(s,birth,final)&&final.currentKiller==s.jason&&final.currentKillerState==s.humanState;
            out.rollbackVerified=restored&&rosterRestored&&botsOkay&&killerSame;
            out.outcome=out.rollbackVerified?Outcome::RolledBack:Outcome::RollbackUnverified;
            return out;
        };
        out.stage=Stage::Spawn;++out.nativeEffects;
        const bool spawned=o_.SpawnAi(birth);
        if(!spawned||!birth.controller||!birth.state||!o_.InWorld(birth.controller)){
            if(!birth.attempted)return refuseSpawn(out,birth,s);
            return rollback(Reason::SpawnFailed,Stage::Spawn);
        }
        if(!birth.botFlag)return rollback(Reason::BotFlag,Stage::Spawn);
        out.stage=Stage::Account;
        std::int32_t nowBots=-1;
        if(!o_.ReadBots(nowBots))return rollback(Reason::BotDelta,Stage::Account);
        const std::int32_t delta=nowBots-beforeBots;
        if(delta==0){if(!o_.AccountBot(beforeBots))return rollback(Reason::BotDelta,Stage::Account);botAccounted=true;}
        else if(delta==1)botAccounted=true;      // native constructor accounted the bot
        else return rollback(Reason::BotDelta,Stage::Account);
        std::int32_t players=-1;bool contains=false;
        if(!o_.Roster(birth.state,players,contains)||!contains||players!=s.players+1)return rollback(Reason::RosterAccounting,Stage::Account);
        // The later adoption receipt requires the controller's real path component.
        // Refuse before human class/possession/restart mutations, after accounting
        // the owned birth so rollback can prove roster and bot-count restoration.
        if(!o_.PathReady(birth))return rollback(Reason::PathUnavailable,Stage::Spawn);
        out.stage=Stage::AiPrepare;
        if(!o_.PrepareAi(birth))return rollback(Reason::AiPrepare,Stage::AiPrepare);
        out.stage=Stage::AiBirth;
        if(!o_.AiBirthReady(birth,0))return rollback(Reason::AiBirth,Stage::AiBirth);
        // The displaced player's active character becomes a counselor BEFORE the AI
        // unpossesses their Jason (Resurrected handoff order).
        out.stage=Stage::HumanPrepare;humanChanged=true;++out.nativeEffects;
        if(!o_.PrepareHuman(s.humanState))return rollback(Reason::HumanPrepare,Stage::HumanPrepare);
        out.stage=Stage::Possess;++out.nativeEffects;
        const bool possessed=o_.PossessAi(birth,s.jason,aiPossessEntered);
        Post post;
        if(!possessed||!o_.ReadPost(s,birth,post)||post.jasonController!=birth.controller||post.aiPawn!=s.jason||post.jasonPawnState!=birth.state||
           !o_.AiBirthReady(birth,s.jason))return rollback(Reason::PossessFailed,Stage::Possess);
        out.stage=Stage::Restart;++out.nativeEffects;bool restartEntered=false;
        const bool restarted=o_.Restart(s.mode,s.human,restartEntered);
        if(!restarted||!o_.ReadPost(s,birth,post))return rollback(Reason::RestartFailed,Stage::Restart);
        counselor=post.counselor==s.jason?0:post.counselor;
        out.stage=Stage::Postconditions;
        if(!post.localCounselorProven||!post.localCounselor||post.localCounselor==s.jason||
           !counselor||!post.counselorInWorld||!post.counselorIsCounselor||post.counselorController!=s.human||
           (s.eligibility==Eligibility::HostSelected&&post.localCounselor!=counselor)|| // host's own controller must hold the new counselor
           post.humanControllerState!=s.humanState||post.currentKiller!=s.jason||
           (post.currentKillerState!=s.humanState&&post.currentKillerState!=birth.state)||
           post.bots!=beforeBots+1||!post.aiRegistered||post.players!=s.players+1)
            return rollback(Reason::Postconditions,Stage::Postconditions);
#if CFG_H247_HOST_TAKEOVER
        if(s.eligibility==Eligibility::HostSelected){   // before Publish: failure rolls back with the AI still unpublished
            out.stage=Stage::LocalView;++out.nativeEffects;
            if(!o_.LocalViewRefresh(s,counselor))return rollback(Reason::LocalRefresh,Stage::LocalView);
        }
#endif
        out.stage=Stage::Publish;
        // Resurrected leaves the original killer PlayerState in GameState until the human has a
        // real counselor pawn; publish AI ownership only now.
        if(post.currentKillerState!=birth.state&&!o_.WriteKillerOwner(s.humanState,birth.state))
            return rollback(Reason::PublishFailed,Stage::Publish);
        adoptionTried=true;out.stage=Stage::Adopt;
        cfghosted::adoption::Receipt receipt;receipt.world=s.world;receipt.mode=s.mode;
        receipt.controllerState=birth.state;receipt.displacedState=s.humanState;receipt.possessReturned=true;
        receipt.botsBefore=beforeBots;receipt.botsAfter=post.bots;receipt.rosterBefore=s.players;receipt.rosterAfter=post.players;
        receipt.valid=o_.Slot(s.jason,receipt.pawn)&&o_.Slot(birth.controller,receipt.controller);
        if(!receipt.Complete()||!o_.Adopt(receipt)){
            // donor: rollback after a failed hosted behavior adoption; the AI owns the Jason
            // and the owner is already published, so restore through the same verified path.
            (void)adoptionTried;
            humanChanged=true;
            if(!o_.WriteKillerOwner(birth.state,s.humanState)){}
            return rollback(Reason::AdoptFailed,Stage::Adopt);
        }
        out.stage=Stage::Done;out.outcome=Outcome::Adopted;out.reason=Reason::None;out.receipt=receipt;out.counselor=counselor;
        return out;
    }
private:
#if CFG_H255_HOST_AI_BIRTH
    // H2.55: the host is already a counselor and no Jason pawn exists. Same AI controller birth, accounting and adoption as the
    // takeover, but the Jason is born by native RestartPlayer on the AI controller (RestartPlayer consumes ActiveCharacterClass).
    // The human is never touched, so a failure only removes what this transaction created.
    Result RunPreborn(const Snapshot& s){
        Result out;out.stage=Stage::Preflight;
        const auto notReady=[&](Reason why){out.outcome=Outcome::NotReady;out.reason=why;return out;};
        if(s.jason||!s.human||!s.humanState||!s.localController||!s.localPawn||!s.world||!s.mode||!s.gameState||
           s.human!=s.localController)return notReady(Reason::RemoteOwnership);
        if(s.bots<0||s.bots>7)return notReady(Reason::BotsRange);
        if(s.players<0||s.players>=8)return notReady(Reason::RosterFull);
        if(!s.counselorTreeLoaded)return notReady(Reason::TreeUnloaded);
        out.stage=Stage::Travel;
        if(!o_.HasFullyTraveled(s.humanState))return notReady(Reason::NotTraveled);
        out.stage=Stage::Classes;
        if(!o_.ResolveClasses())return notReady(Reason::ClassUnavailable);
        locked_=true;out.locked=true;
        Birth birth;bool botAccounted=false;const std::int32_t beforeBots=s.bots;std::uintptr_t jason=0;
        const auto rollback=[&](Reason why,Stage at){
            out.reason=why;out.stage=at;bool restored=true;
            if(jason)restored=o_.InWorld(jason)&&o_.Destroy(jason);
            if(restored&&birth.controller)restored=o_.InWorld(birth.controller)&&o_.Destroy(birth.controller);
            std::int32_t count=-1;bool contains=false;
            const bool listed=birth.state&&o_.Roster(birth.state,count,contains);
            const bool rosterRestored=!(listed&&contains)&&o_.Roster(0,count,contains)&&count==s.players;
            std::int32_t bots=-1;
            if(restored&&rosterRestored&&botAccounted&&o_.ReadBots(bots)&&bots==beforeBots+1)(void)o_.UnaccountBot(bots);
            const bool botsOkay=o_.ReadBots(bots)&&bots==beforeBots;
            out.rollbackVerified=restored&&rosterRestored&&botsOkay;
            out.outcome=out.rollbackVerified?Outcome::RolledBack:Outcome::RollbackUnverified;
            return out;
        };
        out.stage=Stage::Spawn;++out.nativeEffects;
        const bool spawned=o_.SpawnAi(birth);
        if(!spawned||!birth.controller||!birth.state||!o_.InWorld(birth.controller)){
            if(!birth.attempted)return refuseSpawn(out,birth,s);
            return rollback(Reason::SpawnFailed,Stage::Spawn);
        }
        if(!birth.botFlag)return rollback(Reason::BotFlag,Stage::Spawn);
        out.stage=Stage::Account;
        std::int32_t nowBots=-1;
        if(!o_.ReadBots(nowBots))return rollback(Reason::BotDelta,Stage::Account);
        const std::int32_t delta=nowBots-beforeBots;
        if(delta==0){if(!o_.AccountBot(beforeBots))return rollback(Reason::BotDelta,Stage::Account);botAccounted=true;}
        else if(delta==1)botAccounted=true;
        else return rollback(Reason::BotDelta,Stage::Account);
        std::int32_t players=-1;bool contains=false;
        if(!o_.Roster(birth.state,players,contains)||!contains||players!=s.players+1)return rollback(Reason::RosterAccounting,Stage::Account);
        if(!o_.PathReady(birth))return rollback(Reason::PathUnavailable,Stage::Spawn);
        out.stage=Stage::AiPrepare;
        if(!o_.PrepareAiPreborn(birth))return rollback(Reason::AiPrepare,Stage::AiPrepare);
        out.stage=Stage::AiBirth;
        if(!o_.AiBirthReady(birth,0))return rollback(Reason::AiBirth,Stage::AiBirth);
        out.stage=Stage::Restart;++out.nativeEffects;bool entered=false;
        if(!o_.Restart(s.mode,birth.controller,entered)||!o_.ControllerPawn(birth.controller,jason)||!jason)return rollback(Reason::RestartFailed,Stage::Restart);
        o_.BindJason(jason);
        out.stage=Stage::Postconditions;
        if(!o_.PrebornKillerReady(birth,jason)||!o_.AiPathAfterRestart(birth))return rollback(Reason::Postconditions,Stage::Postconditions);
        out.stage=Stage::Publish;
        if(!o_.PublishPrebornKiller(s.humanState,birth.state,jason))return rollback(Reason::PublishFailed,Stage::Publish);
        std::int32_t bots=-1;
        if(!o_.ReadBots(bots)||bots!=beforeBots+1||!o_.Roster(birth.state,players,contains)||!contains||players!=s.players+1)
            return rollback(Reason::Postconditions,Stage::Postconditions);
        out.stage=Stage::Adopt;
        cfghosted::adoption::Receipt receipt;receipt.world=s.world;receipt.mode=s.mode;
        receipt.controllerState=birth.state;receipt.displacedState=s.humanState;receipt.possessReturned=true;
        receipt.botsBefore=beforeBots;receipt.botsAfter=bots;receipt.rosterBefore=s.players;receipt.rosterAfter=players;
        receipt.valid=o_.Slot(jason,receipt.pawn)&&o_.Slot(birth.controller,receipt.controller);
        if(!receipt.Complete()||!o_.AdoptPreborn(receipt))return rollback(Reason::AdoptFailed,Stage::Adopt);
        out.stage=Stage::Done;out.outcome=Outcome::Adopted;out.reason=Reason::None;out.receipt=receipt;out.counselor=s.localPawn;
        return out;
    }
#endif
    // The spawn effect was never entered (e.g. class unresolved at the last moment): nothing to roll back.
    static Result refuseSpawn(Result& out,const Birth&,const Snapshot&){
        out.outcome=Outcome::Refused;out.reason=Reason::SpawnFailed;out.stage=Stage::Spawn;return out;
    }
};
}
