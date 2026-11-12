#pragma once
#include <Windows.h>
#include <Xinput.h>
#include "../Core/RBEmoteInputPolicy.hpp"
#include "CounselorTerminalObservation.hpp"
#include "JasonInstanceObservation.hpp"
#include "BundleObservations.hpp"
#include "RBEmoteBindings.generated.hpp"
#include "../RuntimeProbe/Compatibility/CfgRBConfirmation35.generated.hpp"
namespace cfgrbemote {
using Slot=cfgprobe::lifetime::SlotObservation;
struct Owner {std::uintptr_t world=0,controller=0;Slot pawn{},controllerSlot{};};
struct ConfirmationEvidence {Slot hud{},widget{};int index=-1;};
// No DLL loading in gameplay callbacks. Preparation may load Microsoft's
// system provider once, matching the donor fallback without searching the
// game directory. XInput module lookup retries once per
// second until the game's own input DLL is available. Disconnected scans are
// also limited to once per second (at most four devices per scan).
class Gamepad {
    using GetState=DWORD(WINAPI*)(DWORD,XINPUT_STATE*);
    GetState read_=nullptr;DWORD index_=0;std::uint64_t next_=0,reconnect_=0,lookup_=0;
#ifdef CFG35_URINATION_NATIVE_PORT
    ::cfgurination::MovementSample movement_;
#endif
public:
#ifdef CFG35_URINATION_NATIVE_PORT
    bool Movement(std::uint64_t now)const noexcept{return movement_.Recent(now);}
#endif
    unsigned polls=0,foregroundPolls=0,connectedPolls=0,presses=0;
    bool providerReady=false;
    bool PrepareSystemProvider(){
        for(const auto name:{L"xinput1_4.dll",L"xinput1_3.dll",L"xinput9_1_0.dll"}){
            const auto module=GetModuleHandleW(name);
            if(module){read_=reinterpret_cast<GetState>(GetProcAddress(module,"XInputGetState"));if(read_)break;}}
        if(!read_){
            const auto module=LoadLibraryExW(L"xinput1_4.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
            if(module)read_=reinterpret_cast<GetState>(GetProcAddress(module,"XInputGetState"));
        }
        providerReady=read_!=nullptr;return providerReady;
    }
    bool Poll(std::uint64_t now,Input& input,WORD button=XINPUT_GAMEPAD_RIGHT_SHOULDER){
        input={};if(now<next_)return false;next_=now+16;++polls;
#ifdef CFG35_URINATION_NATIVE_PORT
        movement_.Observe(false,false,0,0,now,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
#endif
        DWORD pid=0;const auto window=GetForegroundWindow();if(window)GetWindowThreadProcessId(window,&pid);
        input.foreground=pid==GetCurrentProcessId();if(!input.foreground)return true;++foregroundPolls;
        if(!read_&&now>=lookup_){lookup_=now+1000;
            for(const auto name:{L"xinput1_4.dll",L"xinput1_3.dll",L"xinput9_1_0.dll"}){
                const auto module=GetModuleHandleW(name);if(module){read_=reinterpret_cast<GetState>(GetProcAddress(module,"XInputGetState"));if(read_)break;}}}
        providerReady=read_!=nullptr;if(!read_||now<reconnect_)return true;
        XINPUT_STATE state{};DWORD result=read_(index_,&state);
        if(result!=ERROR_SUCCESS){for(DWORD i=0;i<XUSER_MAX_COUNT;++i){if(i==index_)continue;
            if(read_(i,&state)==ERROR_SUCCESS){index_=i;result=ERROR_SUCCESS;break;}}}
        if(result!=ERROR_SUCCESS){reconnect_=now+1000;return true;}
        reconnect_=0;++connectedPolls;input.connected=true;input.down=(state.Gamepad.wButtons&button)!=0;
#ifdef CFG35_URINATION_NATIVE_PORT
        movement_.Observe(input.foreground,true,state.Gamepad.sThumbLX,state.Gamepad.sThumbLY,now,XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
#endif
        if(input.down)++presses;return true;
    }
};
// Host contract: Context(), MirrorHuman(Slot&), FeatureFunction, Invoke and
// public context. Caller supplies the established owned controller and module;
// allowed must exclude kill ownership, terminal/spectator and shutdown phases.
template<class R,class H> class NativeAccess {
    R& r_;H& h_;std::uintptr_t module_,controller_;Slot controllerSlot_;
    bool Bind(bool show,bool warm,std::uintptr_t& fn,std::uintptr_t& type){
        const auto& b=show?disk::OnRequestShowEmoteSelect:disk::OnHideEmoteSelect;
        std::uintptr_t cached=0;
        if(!r_.Read(module_+b.cache,cached)||(!warm&&!cached)||!h_.FeatureFunction(b,fn,type))return false;
        const auto exec=show?disk::OnRequestShowEmoteSelectExecutor:disk::OnHideEmoteSelectExecutor;
        return cfgobserver::Same(r_,fn+0xB0,module_+exec)&&
            (show?cfgbundle::Code(r_,module_,exec,disk::OnRequestShowEmoteSelectCode):cfgbundle::Code(r_,module_,exec,disk::OnHideEmoteSelectCode));
    }
    bool BindConfirm(bool play,bool warm,std::uintptr_t& fn,std::uintptr_t& type){
        const auto& b=play?confirmdisk::PlayEmote:confirmdisk::GetOwningPlayerPawn;
        std::uintptr_t cached=0;
        if(!r_.Read(module_+b.cache,cached)||(!warm&&!cached)||!h_.FeatureFunction(b,fn,type))return false;
        return cfgobserver::Same(r_,fn+0xB0,module_+(play?confirmdisk::PlayEmoteExecutor:confirmdisk::GetOwningPlayerPawnExecutor))&&
            (play?cfgbundle::Code(r_,module_,confirmdisk::PlayEmoteExecutor,confirmdisk::PlayEmoteCode):
                cfgbundle::Code(r_,module_,confirmdisk::GetOwningPlayerPawnExecutor,confirmdisk::GetOwningPlayerPawnCode));
    }
    bool Widget(const Owner& owner,ConfirmationEvidence& out){
        out={};std::uintptr_t hud=0,widget=0,hudType=0,widgetType=0;
        namespace d=confirmdisk;
        if(!Current(owner)||!r_.Read(module_+d::SCInGameHUDClass,hudType)||!hudType||
            !cfgobserver::Same(r_,module_+d::SCInGameHUDConstructor,hudType)||
            !r_.Read(module_+d::SCCounselorEmoteSelectWidgetClass,widgetType)||!widgetType||
            !cfgobserver::Same(r_,module_+d::SCCounselorEmoteSelectWidgetConstructor,widgetType)||
            !r_.Read(controller_+d::ControllerHUD,hud)||!cfgspawn::ObjectSlot(r_,h_.context.objectArray,hud,out.hud)||
            !cfgspawn::ObjectIs(r_,hud,hudType)||!cfgobserver::Same(r_,hud+d::PlayerOwner,controller_)||
            !r_.Read(hud+d::EmoteWidgetInstance,widget)||!cfgspawn::ObjectSlot(r_,h_.context.objectArray,widget,out.widget)||
            !cfgspawn::ObjectIs(r_,widget,widgetType)||!r_.Read(widget+d::CurrentHighlightSection,out.index)||out.index<0||out.index>=8)return false;
        // Strong controller->HUD->active-widget links permit a native child
        // whose weak serial is not assigned yet. This is observation only;
        // RBInvoke separately retains every input before native entry.
        return cfgjason::OwnedComponentSlot(r_,h_.context.objectArray,out.hud)&&cfgjason::OwnedComponentSlot(r_,h_.context.objectArray,out.widget)&&
            cfgobserver::Same(r_,controller_+d::ControllerHUD,hud)&&cfgobserver::Same(r_,hud+d::PlayerOwner,controller_)&&
            cfgobserver::Same(r_,hud+d::EmoteWidgetInstance,widget)&&Current(owner);
    }
    static bool SameWidget(const ConfirmationEvidence& a,const ConfirmationEvidence& b){return a.hud==b.hud&&a.widget==b.widget&&a.index==b.index;}
    template<class T>static auto Eligible(T& host,const Owner& owner,const ConfirmationEvidence& evidence,int)
        ->decltype(host.RBEligible(owner,evidence),bool()){return host.RBEligible(owner,evidence);}
    template<class T>static bool Eligible(T&,const Owner&,const ConfirmationEvidence&,long){return false;}
    template<class T>static auto InvokeConfirmation(T& host,const Owner& owner,const ConfirmationEvidence& evidence,std::uintptr_t fn,void* packet,int)
        ->decltype(host.RBInvoke(owner,evidence,fn,packet),bool()){return host.RBInvoke(owner,evidence,fn,packet);}
    template<class T>static bool InvokeConfirmation(T&,const Owner&,const ConfirmationEvidence&,std::uintptr_t,void*,long){return false;}
    template<class T>static auto Failed(T& host,int)->decltype(host.RBConfirmationFailed(),void()){host.RBConfirmationFailed();}
    template<class T>static void Failed(T&,long){}
    template<class T>static auto Cancel(T& host,const Owner& owner,int)->decltype(host.RBCancelCurrent(owner),bool()){return host.RBCancelCurrent(owner);}
    template<class T>static bool Cancel(T&,const Owner&,long){return true;}
public:
    NativeAccess(R& r,H& h,std::uintptr_t module,std::uintptr_t controller,Slot controllerSlot):
        r_(r),h_(h),module_(module),controller_(controller),controllerSlot_(controllerSlot){}
    bool Source(Owner& owner){
        owner={};Slot pawn{};
        if(!h_.Context()||controllerSlot_.object!=controller_||!cfgjason::SameSlot(r_,h_.context.objectArray,controllerSlot_)||
           !h_.MirrorHuman(pawn)||!pawn.HasAssignedSerial()||
           !cfgobserver::Same(r_,controller_+cfgescape::disk::Pawn,pawn.object)||
           !cfgobserver::Same(r_,pawn.object+cfgescape::disk::Controller,controller_))return false;
        owner={h_.context.world,controller_,pawn,controllerSlot_};return true;
    }
    bool Current(const Owner& owner){Owner fresh;return Source(fresh)&&fresh.world==owner.world&&fresh.controller==owner.controller&&
        fresh.controllerSlot==owner.controllerSlot&&fresh.pawn==owner.pawn;}
    bool Warm(bool show){std::uintptr_t fn=0,type=0;return Bind(show,true,fn,type);}
    bool WarmConfirm(bool play){std::uintptr_t fn=0,type=0;return BindConfirm(play,true,fn,type);}
    bool CancelCurrent(const Owner& owner){return Current(owner)&&Cancel(h_,owner,0);}
    bool Confirm(const Owner& owner){
        ConfirmationEvidence evidence{},fresh{};std::uintptr_t query=0,queryType=0,play=0,playType=0,pawn=0;
        if(!Widget(owner,evidence)||!BindConfirm(false,false,query,queryType)||!BindConfirm(true,false,play,playType)||
           !cfgspawn::ObjectIs(r_,evidence.widget.object,queryType)||!cfgspawn::ObjectIs(r_,evidence.widget.object,playType)||
           !Eligible(h_,owner,evidence,0)||!Widget(owner,fresh)||!SameWidget(evidence,fresh)||
           !InvokeConfirmation(h_,owner,evidence,query,&pawn,0)||pawn!=owner.pawn.object||
           !Widget(owner,fresh)||!SameWidget(evidence,fresh))return false;
        // The actual native widget resolves the selected profile slot; never
        // hardcode Wave or dispatch a multicast/ability/input flag reset.
        auto selected=evidence.index;
        const bool returned=InvokeConfirmation(h_,owner,evidence,play,&selected,0);
        if(!returned)Failed(h_,0);
        return returned;
    }
    bool Call(const Owner& owner,bool show){
        std::uintptr_t fn=0,type=0;
        return Bind(show,false,fn,type)&&cfgspawn::ObjectIs(r_,owner.pawn.object,type)&&Current(owner)&&
            h_.Invoke(owner.pawn.object,fn,nullptr,true);
    }
};
class Runtime {
    Gamepad gamepad_;
public:
    Policy<Owner> policy;
    EdgeCapture edges;
    bool PrepareSystemProvider(){return gamepad_.PrepareSystemProvider();}
    const Gamepad& InputStatus()const{return gamepad_;}
    void Sample(std::uint64_t now,bool allowed){
        Input input;if(gamepad_.Poll(now,input))edges.Observe(input,now,allowed);
    }
    template<class Access> bool Step(Access& access,std::uint64_t now,bool allowed){
        if(edges.pending&&now<edges.until&&policy.phase==Phase::Idle)policy.CapturedPress();
        // Finish an owned close before consuming a separately captured press.
        if(policy.phase==Phase::Release)return policy.Step(access,edges.latest,allowed);
        return policy.Step(access,edges.Take(now),allowed);
    }
    void RequestRelease(){policy.Release();}
    void ObserveSelection(std::uintptr_t pawn){if(policy.Owner().pawn.object==pawn)policy.ObserveSelection();}
    template<class Access> bool Close(Access& access,bool allowed){
        policy.Release();const bool worked=allowed&&policy.Step(access,Input{},true);
        policy.Reset();edges={};return worked; // terminal cleanup never adopts another pawn
    }
    void Abandon(){policy.Reset();edges={};}
};
}
