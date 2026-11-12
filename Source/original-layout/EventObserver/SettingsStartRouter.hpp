#pragma once
#include "../RuntimeProbe/SettingsStartObservation.hpp"
#include "SetupMenuStartRuntime.hpp"
namespace cfgmenu::startroute {
enum class SlotKind {Wrapper,Graph};
enum class Result {Forwarded,Consumed,Refused};
// Prepared class-local executor routing, not an installer. The eventual host
// must verify slot ownership/current metadata and active widget on the native
// owner thread. Shared ProcessEvent and the script interpreter stay unchanged.
class Router {
    const startbinding::Binding& binding_;SetupStart& start_;bool handling_=false;
    struct Scope {bool& value;explicit Scope(bool& v):value(v){value=true;}~Scope(){value=false;}};
public:
    Router(const startbinding::Binding& binding,SetupStart& start):binding_(binding),start_(start){}
    template<class Reader,class Host,class Forward> Result Dispatch(Reader& r,Host& host,SlotKind kind,
        std::uint32_t thread,std::uintptr_t receiver,std::uintptr_t frame,Forward&& forward){
        // Unrelated objects/threads and ordinary Jason-mode setup are free:
        // no memory reads, discovery, native UI calls or additional forwarding.
        if(receiver!=binding_.context.widget||thread!=binding_.context.thread||!host.Claimed()){
            forward();return Result::Forwarded;
        }
        const auto& slot=kind==SlotKind::Wrapper?binding_.start:binding_.graph;
        std::uintptr_t node=0,object=0,code=0,current=0,locals=0;
        if(!cfgprobe::lifetime::Extent(frame,startdisk::FrameCurrentNative+8)||
            !r.Read(frame+startdisk::FrameNode,node)||node!=slot.field.object||
            !r.Read(frame+startdisk::FrameObject,object)||object!=receiver||
            !r.Read(frame+startdisk::FrameCode,code)||code!=slot.script.data||
            !r.Read(frame+startdisk::FrameCurrentNative,current)||current!=slot.field.object)return Result::Refused;
        if(kind==SlotKind::Graph){std::int32_t entry=0;
            if(!r.Read(frame+startdisk::FrameLocals,locals)||!cfgprobe::lifetime::Extent(locals,4)||!r.Read(locals,entry))return Result::Refused;
            if(entry!=startdisk::StartEntry){forward();return Result::Forwarded;}
        }
        // Do not advance/step a VM frame or call the interpreter ourselves.
        // Owned invalid/early/nested/duplicate Start must NEVER fall through to
        // stock OfflineBots travel (the human becomes Jason on that route).
        if(handling_)return Result::Consumed;
        if(!host.CurrentBinding(binding_,kind))return Result::Refused;
        Scope scope(handling_);
        const auto disposition=start_.Start(host);
        if(disposition==StartDisposition::NativeRoute)return Result::Refused; // Lost claim, no unsafe native fallback.
        return disposition==StartDisposition::Refused?Result::Refused:Result::Consumed;
    }
};
}
