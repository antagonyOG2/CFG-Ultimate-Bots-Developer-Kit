#pragma once
#include <cstdint>
namespace cfgrbemote {
enum class Phase {Idle,WarmHide,WarmShow,ClearOld,Show,Open,Release,ConfirmOwnerWarm,ConfirmPlayWarm,Confirm,CancelCurrent};
struct Input {bool foreground=false,connected=false,down=false;};
struct EdgeCapture {
    Input latest{};bool lastDown=false,pending=false;std::uint64_t until=0;
    unsigned edges=0,blocked=0,expired=0;
    void Observe(Input input,std::uint64_t now,bool allowed){
        latest=input;const bool down=input.foreground&&input.connected&&input.down;
        if(down&&!lastDown){if(allowed){pending=true;until=now+1000;++edges;}else ++blocked;}
        lastDown=down;
        if(!allowed||!input.foreground||!input.connected)pending=false;
        if(pending&&now>=until){pending=false;++expired;}
    }
    Input Take(std::uint64_t now){
        if(pending&&now>=until){pending=false;++expired;}
        auto input=latest;if(pending){input.down=true;pending=false;}return input;
    }
};
// Input bookkeeping performs no native work. A held RB never retriggers.
// One native factory OR one wheel action is permitted in each Step.
template<class Identity> class Policy {
    bool lastDown_=false;Identity owner_{};
    template<class H>static auto Confirmation(H& h,const Identity& owner,int)->decltype(h.WarmConfirm(false),h.Confirm(owner),bool()){(void)h;(void)owner;return true;}
    template<class H>static bool Confirmation(H&,const Identity&,long){return false;}
    template<class H>static auto WarmConfirmation(H& h,bool play,int)->decltype(h.WarmConfirm(play),bool()){return h.WarmConfirm(play);}
    template<class H>static bool WarmConfirmation(H&,bool,long){return false;}
    template<class H>static auto ConfirmSelection(H& h,const Identity& owner,int)->decltype(h.Confirm(owner),bool()){return h.Confirm(owner);}
    template<class H>static bool ConfirmSelection(H&,const Identity&,long){return false;}
    template<class H>static auto CancelCurrent(H& h,const Identity& owner,int)->decltype(h.CancelCurrent(owner),bool()){return h.CancelCurrent(owner);}
    template<class H>static bool CancelCurrent(H&,const Identity&,long){return true;}
public:
    Phase phase=Phase::Idle;unsigned shows=0,hides=0,refused=0,abandoned=0;
    bool selectionObserved=false;
    const Identity& Owner()const{return owner_;}
    void ObserveSelection(){if(Owned())selectionObserved=true;}
    bool Owned()const{return phase==Phase::Open||phase==Phase::Release||(phase>=Phase::ConfirmOwnerWarm&&phase<=Phase::Confirm);}
    void Release(){if(Owned())phase=Phase::Release;else phase=Phase::Idle;}
    void Reset(){phase=Phase::Idle;owner_={};lastDown_=false;selectionObserved=false;}
    void CapturedPress(){if(phase==Phase::Idle)lastDown_=false;}
    template<class Host> bool Step(Host& h,Input input,bool allowed){
        const bool down=input.foreground&&input.connected&&input.down;
        const bool pressed=down&&!lastDown_;lastDown_=down;
        if(!allowed){Release();return false;} // retain owned close until safe
        if(phase==Phase::Open){if(down)return false;
            phase=input.foreground&&input.connected&&!selectionObserved&&Confirmation(h,owner_,0)?Phase::ConfirmOwnerWarm:Phase::Release;}
        // A real press is latched through bounded cold preparation. Releasing
        // RB before all four native slices used to silently discard the press.
        // Lost focus/device still cancels pending work; a normal short tap
        // reaches Show once, then the existing owned Release closes it.
        if(phase!=Phase::Idle&&phase!=Phase::Release&&phase!=Phase::Open&&
           (!input.foreground||!input.connected)){
            if(Owned())phase=Phase::Release;else{phase=Phase::Idle;owner_={};return false;}}
        if(phase==Phase::Idle){
            if(!pressed)return false;
            if(!h.Source(owner_)){++refused;return true;}
            phase=Confirmation(h,owner_,0)?Phase::CancelCurrent:Phase::WarmHide;return true;
        }
        if(!h.Current(owner_)){if(Owned())++abandoned;else ++refused;phase=Phase::Idle;owner_={};return true;}
        bool ok=false;
        switch(phase){
        case Phase::CancelCurrent:ok=CancelCurrent(h,owner_,0);if(ok)phase=Phase::WarmHide;break;
        case Phase::WarmHide:ok=h.Warm(false);if(ok)phase=Phase::WarmShow;break;
        case Phase::WarmShow:ok=h.Warm(true);if(ok)phase=Phase::ClearOld;break;
        case Phase::ClearOld:ok=h.Call(owner_,false);if(ok){++hides;phase=Phase::Show;}break;
        case Phase::Show:
            // Publish cleanup ownership before potentially reentrant dispatch.
            selectionObserved=false;phase=Phase::Open;ok=h.Call(owner_,true);if(ok)++shows;else phase=Phase::Release;break;
        case Phase::ConfirmOwnerWarm:ok=WarmConfirmation(h,false,0);phase=ok?Phase::ConfirmPlayWarm:Phase::Release;break;
        case Phase::ConfirmPlayWarm:ok=WarmConfirmation(h,true,0);phase=ok?Phase::Confirm:Phase::Release;break;
        case Phase::Confirm:
            // Consume before native widget playback can synchronously dispatch
            // SERVER_PlayEmote. A natural selection never gets duplicated.
            phase=Phase::Release;ok=selectionObserved||ConfirmSelection(h,owner_,0);break;
        case Phase::Release:ok=h.Call(owner_,false);++hides;phase=Phase::Idle;owner_={};break;
        default:return false;
        }
        if(!ok){++refused;if(phase!=Phase::Release){phase=Phase::Idle;owner_={};}}
        return true;
    }
};
}
