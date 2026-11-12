#pragma once
#include "NativePendingModeAccess.hpp"
#include "NativeOriginalStartFrame.hpp"
#include <type_traits>
namespace cfgmenu::sandboxstart {
// Actual synchronous launch host: joins the current menu's class-local slots,
// armed new-world submission, typed pending-mode writer and original FFrame.
// Do not keep this object/frame beyond the native Start callback. The caller
// retains the menu binding and resident slot thunks for the session lifetime.
template<class Reader,class Writer=NativePendingModeAccess<Reader>,class Forward=startroute::NativeOriginalStart<Reader>>
class NativeHost {
    Reader& reader_;const startbinding::Binding& binding_;const GameplayHandoff& handoff_;
    std::uint64_t generation_=0;std::uintptr_t wrapper_=0,graph_=0;
    Writer writer_;Forward forward_;bool prepared_=false;
    const cfgreflection::Property* authoredMode_=nullptr;
public:
    NativeHost(Reader& reader,const startbinding::Binding& binding,const GameplayHandoff& handoff,
        std::uint64_t generation,std::uintptr_t wrapper,std::uintptr_t graph,const cfgreflection::Property* authoredMode=nullptr):
        reader_(reader),binding_(binding),handoff_(handoff),generation_(generation),wrapper_(wrapper),graph_(graph),
        writer_(reader),forward_(reader,binding),authoredMode_(authoredMode){}
    bool Armed(const active::Context& context)const noexcept{
        return context==binding_.context&&handoff_.ArmedFor(context.widget,generation_,context.world);
    }
    bool OwnedStartBindings(const active::Context& context){
        return prepared_&&context==binding_.context&&
            startbinding::Current(reader_,binding_,context.thread,wrapper_,graph_);
    }
    bool Prepare(const startroute::OriginalFrame& frame,bool exactImageVerified){
        if(prepared_||!exactImageVerified||!Armed(binding_.context)||
            !startbinding::Current(reader_,binding_,binding_.context.thread,wrapper_,graph_))return false;
        bool writerReady=false;
        if(authoredMode_){
            if constexpr(std::is_same_v<Writer,NativePendingModeAccess<Reader>>)
                writerReady=writer_.PrepareWithAuthoredMode(binding_.context,true,*authoredMode_);
            // A model/custom writer cannot silently ignore a supplied source.
        }else writerReady=writer_.Prepare(binding_.context,true);
        if(!writerReady||!forward_.Prepare(frame,true))return false;
        prepared_=true;return true;
    }
    WriteResult ChangePendingMode(const active::Context& context,std::uintptr_t previous,std::uintptr_t replacement){
        if(!Armed(context)||!OwnedStartBindings(context))return WriteResult::Refused;
        return writer_.ChangePendingMode(context,previous,replacement);
    }
    bool ForwardOriginalStart(const active::Context& context){
        // Forwarder's guard is frame/code specific; the host also guards armed
        // session and the exclusively installed reflected executor slots.
        return Armed(context)&&OwnedStartBindings(context)&&forward_.Forward();
    }
};
template<class Reader> Result SubmitNativeStart(Reader& reader,const startbinding::Binding& binding,
    const GameplayHandoff& handoff,std::uint64_t generation,std::uintptr_t wrapper,std::uintptr_t graph,
    const startroute::OriginalFrame& frame,Route& route,bool exactImageVerified,unsigned* reads=nullptr,bool allowModeSwitch=true,
    const cfgreflection::Property* authoredMode=nullptr,Target target=Target::Sandbox){
    NativeHost<Reader> host(reader,binding,handoff,generation,wrapper,graph,authoredMode);
    if(!host.Prepare(frame,exactImageVerified)){if(reads)*reads=0;return Result::Refused;}
    return route.Start(reader,host,binding.context,exactImageVerified,reads,allowModeSwitch,target);
}
}
