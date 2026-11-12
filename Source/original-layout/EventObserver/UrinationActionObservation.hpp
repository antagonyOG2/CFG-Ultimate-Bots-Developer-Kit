#pragma once
#include "UrinationBindings.generated.hpp"
#include "../Core/UrinationLoadRequest.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
namespace cfgurination {
struct NativeString {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool Empty()const noexcept{return !data&&!count&&!capacity;}
    bool operator==(const NativeString& b)const noexcept{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
struct NativePath {
    cfgnames::Words asset{};NativeString sub{};
    bool operator==(const NativePath& b)const noexcept{return asset==b.asset&&sub==b.sub;}
};
struct NativeLatent {std::int32_t linkage=-1;std::uint32_t uuid=0;cfgnames::Words execution{};std::uintptr_t controller=0;};
struct NativeWeak {
    std::int32_t index=0,serial=0;
    bool operator==(const NativeWeak& b)const noexcept{return index==b.index&&serial==b.serial;}
};
struct NativeHandle {
    std::uintptr_t object=0,control=0;
    bool operator==(const NativeHandle& b)const noexcept{return object==b.object&&control==b.control;}
};
static_assert(sizeof(NativePath)==24&&sizeof(NativeLatent)==24&&sizeof(NativeWeak)==8&&sizeof(NativeHandle)==16);
inline constexpr std::u16string_view ExpectedAsset=u"/Game/Cinematics/SP_Testing/m_normal_stand_piss_Montage.m_normal_stand_piss_Montage";
struct ActionOwner {
    std::uintptr_t module=0;std::uint32_t thread=0;
    NativeWeak controller{}; // actual positive registry index/serial, not pointer truncation
};
// Disk-pinned native action observation only. Host.Current(expectedOwner) must independently
// prove the current offline owner/thread/GC scope and owned installed hook.
// This helper has NO native/registry/publication API and confers no such authority.
// Do not carry a successful observation across calls or past original completion.
template<class R,class Host>class ActionObservation {
    R& r_;Host& host_;ActionOwner owner_;
    bool Scope(const LoadRequest& request){const auto& expected=request.Owner();return owner_.module&&
        owner_.thread==expected.thread&&owner_.controller.index>=0&&owner_.controller.serial>0&&
        owner_.controller.serial==expected.controllerSerial&&host_.Current(expected);}
    bool Path(std::uintptr_t address,NativePath& out){
        out={};NativePath value{},again{};cfgnames::Text text;cfgnames::Observation<R> names(r_,owner_.module);
        if(!r_.Read(address,value)||!value.sub.Empty()||value.asset.number||
            !names.Decode(value.asset,text)||text.View()!=ExpectedAsset||
            !r_.Read(address,again)||!(again==value))return false;
        out=value;return true;
    }
    bool Stored(std::uintptr_t action){
        NativeWeak callback{};std::int32_t linkage=0;cfgnames::Words execution{};
        return r_.Read(action+disk::ActionLinkage,linkage)&&linkage==-1&&
            r_.Read(action+disk::ActionExecution,execution)&&execution==cfgnames::Words{}&&
            r_.Read(action+disk::ActionCallback,callback)&&callback==owner_.controller;
    }
public:
    ActionObservation(R& reader,Host& host,ActionOwner owner):r_(reader),host_(host),owner_(owner){}
    bool Constructor(LoadRequest& request,std::uintptr_t action,std::uintptr_t returned,
        std::uintptr_t path,std::uintptr_t latent,std::uintptr_t caller){
        if(request.Stage()!=LoadStage::Requesting||!cfgprobe::ObjectPointer(action)||returned!=action||
            caller!=owner_.module+disk::ConstructorReturn||!cfgprobe::ObjectPointer(path)||
            !cfgprobe::ObjectPointer(latent)||!Scope(request))return false;
        NativeLatent input{},again{};NativePath source{},copy{};std::uintptr_t table=0,fresh=0;
        if(!r_.Read(latent,input)||input.linkage!=-1||input.uuid!=request.Owner().uuid||
            input.controller!=request.Owner().controller||!(input.execution==cfgnames::Words{})||
            !r_.Read(action,table)||table!=owner_.module+disk::ConstructorVtable||
            !Path(path,source)||!Path(action+disk::ActionPath,copy)||!(source==copy)||!Stored(action)||
            !Scope(request)||!r_.Read(action,fresh)||fresh!=table||!r_.Read(latent,again)||
            again.linkage!=input.linkage||again.uuid!=input.uuid||again.controller!=input.controller||
            !(again.execution==input.execution))return false;
        return request.Capture(action,returned,owner_.thread,input.uuid,input.controller,true);
    }
private:
    bool CompletionFrame(const LoadRequest& request,LoadStage stage,std::uintptr_t action,std::uintptr_t delegate,std::uintptr_t caller){
        // Native ActionComplete tail-jumps to LoadedDelegate: its return PC
        // remains the exact UpdateOperation virtual-call continuation.
        if(request.Stage()!=stage||!action||action!=request.Action()||
            delegate!=action+disk::ActionDelegate||caller!=owner_.module+disk::DelegateReturn||!Scope(request))return false;
        std::uintptr_t table=0,fresh=0;NativePath path{};NativeHandle handle{},again{};
        std::array<std::uint64_t,2> callback{},callbackAfter{};std::uint8_t completed=0,cancelled=1,doneAgain=0,cancelAgain=1;
        if(!r_.Read(action,table)||table!=owner_.module+disk::ActionVtable||!Stored(action)||
            // Native LoadedDelegate returns without dispatch when its FName
            // word is zero. Its unused weak receiver need not have one guessed
            // canonical empty representation; preserve it and resample it.
            !Path(action+disk::ActionPath,path)||!r_.Read(delegate,callback)||callback[1]||
            !r_.Read(action+disk::ActionHandle,handle)||!cfgprobe::ObjectPointer(handle.object)||!cfgprobe::ObjectPointer(handle.control)||
            !r_.Read(handle.object+disk::HandleComplete,completed)||completed!=1||
            !r_.Read(handle.object+disk::HandleCancelled,cancelled)||cancelled||
            !Scope(request)||!r_.Read(action,fresh)||fresh!=table||!Stored(action)||
            !r_.Read(action+disk::ActionHandle,again)||!(again==handle)||
            !r_.Read(handle.object+disk::HandleComplete,doneAgain)||doneAgain!=completed||
            !r_.Read(handle.object+disk::HandleCancelled,cancelAgain)||cancelAgain!=cancelled||
            !r_.Read(delegate,callbackAfter)||callbackAfter!=callback)return false;
        return true; // caller must consume request BEFORE root/forwarding work
    }
public:
    bool Completion(const LoadRequest& request,std::uintptr_t action,std::uintptr_t delegate,std::uintptr_t caller){
        return CompletionFrame(request,LoadStage::Waiting,action,delegate,caller);
    }
    // A callback-local registry proof must remain valid AFTER the one-shot
    // completion is consumed and WHILE that same native frame still owns the
    // handle. This is observation only, never a replacement completed tick.
    bool Publishing(const LoadRequest& request,std::uintptr_t action,std::uintptr_t delegate,std::uintptr_t caller){
        return CompletionFrame(request,LoadStage::Completing,action,delegate,caller);
    }
};
}
