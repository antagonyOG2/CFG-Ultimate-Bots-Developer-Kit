#pragma once
#include "../RuntimeProbe/NativeFrontendCatalogues.hpp"
namespace cfgmenu::sandboxstart {
enum class WriteResult {Refused,Changed,Indeterminate};
enum class Result {Refused,Forwarded,Consumed,Indeterminate};
enum class State {Idle,Preparing,Forwarded,Refused,Indeterminate};
enum class Target {Sandbox,NativeOfflineBots};
constexpr unsigned ReadLimit=12288;
template<class Reader> class Bounded {
    Reader& reader_;
public:
    unsigned reads=0;explicit Bounded(Reader& r):reader_(r){}
    template<class T> bool Read(std::uintptr_t at,T& out){if(reads==ReadLimit)return false;++reads;return reader_.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads==ReadLimit)return false;++reads;return reader_.ReadBytes(at,out,bytes);}
};
// Staged route, NOT a Windows writer/installer. Runs only on the claimed,
// current settings Start after SetupStart arms its fresh-world submission.
// The actual host must independently prove PendingOfflineMode/Alias schema,
// class-local original Start executor, owning game thread and pinned image.
// Native Start builds both normal/snow URLs from the pending authored alias.
// We do not rebuild weather strings or request/possess a human character.
class Route {
    State state_=State::Idle;
public:
    State Current()const noexcept{return state_;}
    template<class Reader,class Host> Result Start(Reader& reader,Host& host,const active::Context& context,
        bool exactImageVerified,unsigned* consumed=nullptr,bool allowModeSwitch=true,Target target=Target::Sandbox){
        if(consumed)*consumed=0;
        if(state_!=State::Idle)return state_==State::Indeterminate?Result::Indeterminate:Result::Consumed;
        state_=State::Preparing; // BEFORE any callback/reentrant Start.
        Bounded<Reader> r(reader);
        const auto finish=[&](Result value){if(consumed)*consumed=r.reads;return value;};
        const auto refuse=[&](){state_=State::Refused;return finish(Result::Refused);};
        if(!exactImageVerified||!host.Armed(context)||!host.OwnedStartBindings(context))return refuse();
        catalogues::Modes modes;
        if(!catalogues::ObserveModes(r,context,true,modes))return refuse();
        std::uintptr_t original=0;unsigned targets=0,sources=0;catalogues::ModeEntry sandbox;
        if(!r.Read(context.mode+selectiondisk::PendingOfflineMode,original))return refuse();
        for(unsigned i=0;i<modes.count;++i){const auto& entry=modes.entries[i];
            if(entry.type==original&&((entry.name.View()==u"ModeDef_OfflineBots_C"&&entry.alias.View()==u"oflb")||
                (entry.name.View()==u"ModeDef_Sandbox_C"&&entry.alias.View()==u"snbx")))++sources;
            const bool selected=target==Target::Sandbox?
                entry.name.View()==u"ModeDef_Sandbox_C"&&entry.alias.View()==u"snbx":
                entry.name.View()==u"ModeDef_OfflineBots_C"&&entry.alias.View()==u"oflb";
            if(selected){sandbox=entry;++targets;}
        }
        if(sources!=1||targets!=1||!sandbox.supportsSelectedMap)return refuse();
        const auto current=[&](std::uintptr_t pending){
            std::uintptr_t map=0,mode=0,settings=0,type=0,defaults=0;catalogues::StringValue alias;
            return active::Current(r,context,context.thread)&&
                r.Read(context.mode+selectiondisk::PendingOfflineMap,map)&&map==modes.selectedMap&&
                r.Read(context.mode+selectiondisk::PendingOfflineMode,mode)&&mode==pending&&
                r.Read(context.mode+selectiondisk::PendingOfflineSettingsWidget,settings)&&settings==context.type&&
                r.Read(sandbox.type+cfgspawn::ClassDefaultObject,defaults)&&defaults==sandbox.defaults&&
                r.Read(defaults+cfgprobe::ObjectClass,type)&&type==sandbox.type&&
                catalogues::String(r,defaults+selectiondisk::Alias,alias)&&alias==sandbox.alias;
        };
        if(!current(original)||!host.Armed(context)||!host.OwnedStartBindings(context))return refuse();
        // Host compare/write is scoped to this exact instance and old value;
        // never modify definition CDOs, URLs, player state or class defaults.
        const bool needsSwitch=original!=sandbox.type;
        if(needsSwitch&&!allowModeSwitch)return refuse();
        if(needsSwitch){
            const auto changed=host.ChangePendingMode(context,original,sandbox.type);
            if(changed==WriteResult::Refused)return refuse();
            if(changed==WriteResult::Indeterminate){state_=State::Indeterminate;return finish(Result::Indeterminate);}
        }
        if(!current(sandbox.type)||!host.Armed(context)||!host.OwnedStartBindings(context)){
            // A definite PRE-dispatch failure may roll back ONLY while owner
            // and all pending selections remain ours. No cross-world writes.
            if(!needsSwitch)return refuse(); // Existing native Sandbox: nothing was changed.
            if(current(sandbox.type)&&host.ChangePendingMode(context,sandbox.type,original)==WriteResult::Changed)return refuse();
            state_=State::Indeterminate;return finish(Result::Indeterminate);
        }
        state_=State::Forwarded; // Nested/duplicate observations consume.
        if(!host.ForwardOriginalStart(context)){
            // Native Start has no acceptance return. Fault/normal return cannot
            // authorize restoring or retrying after potentially queued travel.
            state_=State::Indeterminate;return finish(Result::Indeterminate);
        }
        return finish(Result::Forwarded); // Submission only, not travel/spawn proof.
    }
    // Recreate a Route with a NEW validated menu session; no reset while a
    // submitted world is arriving, no tick/polling and no post-travel restore.
};
}
