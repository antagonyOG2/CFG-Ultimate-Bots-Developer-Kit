#pragma once
#include <cstdint>
// Donor order: OfflineSetup::BirthSelectedCounselorForPreMatch and
// BackendMain::OfflineBots958TraceHook, followed by SpawnCounselorModeJasonAfterMatch.
// This transaction is NOT a native binding and does not enable the menu route.
// Host must prove the exact offline world, owner thread, selected classes and
// native ownership before every operation. Never use it for Sandbox or online.
namespace cfgnativelifecycle {
enum class Stage { Empty, Bots, Human, IntroMetadata, Intro, AwaitMatch, MatchStart,
                   Jason, Transfer, Restore, Ready, Refused };
enum class Result { Waiting, Done, Failed };
struct Request {std::uint64_t session=0;unsigned helpers=0;};
class Transaction {
    Request request_{};Stage stage_=Stage::Empty;unsigned bots_=0;
    bool busy_=false,metadataAttempted_=false,cleanupAttempted_=false;
    struct Scope {bool& b;explicit Scope(bool& value):b(value){b=true;}~Scope(){b=false;}};
    template<class Host> bool Fail(Host& host){
        stage_=Stage::Refused;
        // Host restores only fields still owned by this transaction; it must
        // never overwrite another world's objects or repair possession later.
        if(metadataAttempted_&&!cleanupAttempted_){cleanupAttempted_=true;host.RestoreOwnedIntroMetadata(request_.session);}
        return false;
    }
public:
    Stage State()const{return stage_;}unsigned Bots()const{return bots_;}
    unsigned RequestedBots()const{return request_.helpers;}
    template<class Host> bool Abort(Host& host){return Fail(host);}
    bool Arm(Request r){
        if(stage_!=Stage::Empty||!r.session||r.helpers>31)return false;
        request_=r;stage_=Stage::Bots;return true;
    }
    template<class Host> bool Step(Host& host){
        if(busy_)return false;
        if(stage_==Stage::Ready)return true;
        if(stage_==Stage::Empty||stage_==Stage::Refused)return false;
        Scope scope(busy_);
        if(!host.CurrentNativeOfflineOwner(request_.session))return Fail(host);
        // Callback router runs these bounded stages within the donor's958/818
        // native callbacks. No HUD callback is assumed before human birth.
        Result r=Result::Failed;
        switch(stage_){
        case Stage::Bots:
            if(bots_==request_.helpers){stage_=Stage::Human;return true;}
            r=host.SpawnNativeCounselor(bots_);
            if(r==Result::Done)++bots_;break;
        case Stage::Human:
            r=host.BirthSelectedCounselor();
            if(r==Result::Done){if(!host.HumanCounselorPossessionIntact())return Fail(host);stage_=Stage::IntroMetadata;}break;
        case Stage::IntroMetadata:
            metadataAttempted_=true;r=host.PrepareSelectedJasonIntro();
            if(r==Result::Done){if(!host.HumanCounselorPossessionIntact())return Fail(host);stage_=Stage::Intro;}break;
        case Stage::Intro:
            // Set the next state BEFORE invoking the native parent: it can
            // synchronously invoke another lifecycle callback.
            if(!host.HumanCounselorPossessionIntact())return Fail(host);
            stage_=Stage::AwaitMatch;
            if(!host.CallOfflineParentIntroOnce())return Fail(host);
            return true;
        case Stage::AwaitMatch:
            if(!host.HumanCounselorPossessionIntact())return Fail(host);
            if(host.NativeMatchStartCallback())stage_=Stage::MatchStart;
            return true;
        case Stage::MatchStart:
            // Donor818 calls the common Offline parent, never the OfflineBots
            // override that assigns the human killer role.
            if(!host.HumanCounselorPossessionIntact())return Fail(host);
            stage_=Stage::Jason;
            if(!host.CallOfflineParentMatchStartOnce())return Fail(host);
            return true;
        case Stage::Jason:
            r=host.SpawnIndependentAIJason();
            if(r==Result::Done)stage_=Stage::Transfer;break;
        case Stage::Transfer:
            r=host.TransferKillerOwnershipToAI();
            if(r==Result::Done)stage_=Stage::Restore;break;
        case Stage::Restore:
            r=host.RestoreHumanCounselorMetadata();
            if(r==Result::Done){
                if(!host.HumanCounselorPossessionIntact()||!host.AIKillerOwnershipIntact())return Fail(host);
                metadataAttempted_=false;stage_=Stage::Ready;
            }break;
        default:return Fail(host);
        }
        if(r==Result::Failed)return Fail(host);
        return true;
    }
};
// Mirrors the donor's two specific overrides. The shared630 starting-player
// callback is deliberately not hooked. Providers prewarm all selected assets
// before travel: waiting inside either native callback cannot be resumed later
// by a human pawn that has not yet been born.
enum class CallbackResult { ForwardOriginal, Consumed, Refused };
class CallbackRouter {
    Transaction& transaction_;bool intro_=false,match_=false,busy_=false;
public:
    explicit CallbackRouter(Transaction& t):transaction_(t){}
    template<class Host> CallbackResult Intro(Host& h,bool claimed){
        if(!claimed)return CallbackResult::ForwardOriginal;
        if(busy_||intro_)return CallbackResult::Consumed;
        intro_=true;busy_=true;
        bool ok=transaction_.State()==Stage::Bots;
        const unsigned limit=transaction_.RequestedBots()+5;
        for(unsigned i=0;ok&&i<limit&&transaction_.State()!=Stage::AwaitMatch;++i){
            const auto before=transaction_.State();const auto bots=transaction_.Bots();
            ok=transaction_.Step(h);
            if(ok&&before==transaction_.State()&&bots==transaction_.Bots())ok=false;
        }
        ok=ok&&transaction_.State()==Stage::AwaitMatch;
        if(!ok)transaction_.Abort(h);
        busy_=false;return ok?CallbackResult::Consumed:CallbackResult::Refused;
    }
    template<class Host> CallbackResult MatchStart(Host& h,bool claimed){
        if(!claimed)return CallbackResult::ForwardOriginal;
        if(busy_||match_)return CallbackResult::Consumed;
        match_=true;busy_=true;
        bool ok=intro_&&transaction_.State()==Stage::AwaitMatch;
        for(unsigned i=0;ok&&i<5&&transaction_.State()!=Stage::Ready;++i){
            const auto before=transaction_.State();ok=transaction_.Step(h);
            if(ok&&before==transaction_.State())ok=false;
        }
        ok=ok&&transaction_.State()==Stage::Ready;
        if(!ok)transaction_.Abort(h);
        busy_=false;return ok?CallbackResult::Consumed:CallbackResult::Refused;
    }
};
}
