#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "SettingsStartRouter.hpp"
#include "NativeStartupRetention.hpp"
namespace cfgmenu::startroute {
struct OriginalFrame {
    std::uintptr_t receiver=0,frame=0,result=0;
    SlotKind kind=SlotKind::Wrapper;
};
template<class Reader> bool CurrentFrame(Reader& reader,const startbinding::Binding& binding,
    const OriginalFrame& invocation,std::uint32_t thread){
    if(thread!=binding.context.thread||invocation.receiver!=binding.context.widget||
        !cfgprobe::lifetime::Extent(invocation.frame,startdisk::FrameCurrentNative+8))return false;
    const auto& slot=invocation.kind==SlotKind::Wrapper?binding.start:binding.graph;
    std::uintptr_t node=0,receiver=0,code=0,current=0,locals=0;
    if(!reader.Read(invocation.frame+startdisk::FrameNode,node)||node!=slot.field.object||
        !reader.Read(invocation.frame+startdisk::FrameObject,receiver)||receiver!=invocation.receiver||
        !reader.Read(invocation.frame+startdisk::FrameCode,code)||code!=slot.script.data||
        !reader.Read(invocation.frame+startdisk::FrameCurrentNative,current)||current!=slot.field.object)return false;
    if(invocation.kind==SlotKind::Graph){std::int32_t entry=0;
        if(!reader.Read(invocation.frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||
            !reader.Read(locals,entry)||entry!=startdisk::StartEntry)return false;}
    return true;
}
// A single *borrowed live callback frame*. Never retain this across callbacks,
// construct a replacement FFrame, step bytecode manually or reinvoke ProcessEvent
// (which would re-enter the installed class-local slots). The original native
// script executor handles its own frame progression exactly as before.
template<class Reader> class NativeOriginalStart {
    Reader& reader_;const startbinding::Binding& binding_;
    OriginalFrame invocation_{};bool prepared_=false,called_=false;
    bool ForwardNative()noexcept{
        __try {
            reinterpret_cast<void(__fastcall*)(void*,void*,void*)>(binding_.context.module+startdisk::ScriptExecutor)(
                reinterpret_cast<void*>(invocation_.receiver),reinterpret_cast<void*>(invocation_.frame),reinterpret_cast<void*>(invocation_.result));
            return true;
        } __except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
public:
    NativeOriginalStart(Reader& reader,const startbinding::Binding& binding):reader_(reader),binding_(binding){}
    bool Prepare(const OriginalFrame& invocation,bool exactImageVerified){
        if(prepared_||called_||!exactImageVerified||binding_.context.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||
            !CurrentFrame(reader_,binding_,invocation,GetCurrentThreadId()))return false;
        invocation_=invocation;prepared_=true;return true;
    }
    bool Forward()noexcept{
        if(!prepared_||called_||!CurrentFrame(reader_,binding_,invocation_,GetCurrentThreadId()))return false;
        std::array<unsigned char,startdisk::ScriptExecutorCode.size()> code{};
        SIZE_T got=0;
        if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(binding_.context.module+startdisk::ScriptExecutor),
            code.data(),code.size(),&got)||got!=code.size()||code!=startdisk::ScriptExecutorCode)return false;
        called_=true; // Publish BEFORE the native graph can broadcast/reenter.
        return startupretain::Invoke(binding_.context.module,startupretain::Call::OriginalStart,
            std::array<std::uintptr_t,6>{binding_.context.world,binding_.context.hud,invocation_.receiver,
                binding_.start.field.object,binding_.graph.field.object,binding_.entry.field.object},[&](){return ForwardNative();});
    }
    bool Called()const noexcept{return called_;}
};
}
