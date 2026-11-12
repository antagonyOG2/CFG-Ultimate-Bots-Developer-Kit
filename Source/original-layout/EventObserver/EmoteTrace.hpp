#pragma once
#include "OfflineContextObservation.hpp"
#include <cstring>

namespace cfgemote {
// Exact-build generated function caches corroborated on disk, NOT call targets.
// Native emotes remain native: no Wave replacement, montage load or pawn writes.
struct Binding { const char* name; std::uintptr_t cache; unsigned bytes; };
constexpr std::array<Binding,4> Bindings{{
    {"MULTICAST_CancelEmote",0x03049958,0},
    {"MULTICAST_PlayEmote",0x030499A8,16},
    {"SERVER_CancelEmote",0x03049B58,0},
    {"SERVER_PlayEmote",0x03049BA0,8}
}};
constexpr std::uint32_t Magic=0x43464554,Version=2,Capacity=32,CallbackBudget=4096;
constexpr std::uint64_t WindowMs=30000;
enum class State : std::uint32_t { Empty, Prepared, Capturing, Complete, Refused };
enum class Stop : std::uint32_t { None, Time, Events, Callbacks, Clock, Context, Binding, Parameter, Completion };
enum class Outcome : std::uint32_t { Pending, MontageStarted, MontageCleared, Unchanged, ReadRefused, OriginalException, OtherTransition };
constexpr std::uintptr_t CurrentEmoteOffset=0x1CF0; // Native descriptor/store/clear evidence.
struct Function {
    cfgprobe::lifetime::SlotObservation slot{};
    std::uint64_t name=0;
};
struct Event {
    std::uint32_t sequence=0,binding=0,parameterBytes=0,readCalls=0;
    std::uint64_t elapsedMs=0,function=0,functionName=0;
    std::int32_t functionSerial=0,reserved=0;
    std::array<unsigned char,16> parameters{};
    cfgprobe::lifetime::SlotObservation reference{};
    std::uint64_t referenceName=0,referenceClass=0,referenceOuter=0;
    std::uint64_t montageBefore=0,montageAfter=0,montageName=0;
    std::uint32_t outcome=0,afterReads=0;
};
struct Report {
    std::uint32_t magic=Magic,version=Version,bytes=sizeof(Report),generation=0;
    std::uint32_t state=0,stop=0,count=0,callbacks=0;
    std::uint32_t presentMask=0,threadId=0,readCalls=0,reserved=0;
    std::uint64_t pawn=0,elapsedMs=0;
    std::array<Function,4> functions{};
    std::array<Event,Capacity> events{};
};
static_assert(std::is_trivially_copyable<Report>::value && sizeof(Report)<8192,"Bounded read-only trace");
inline bool Valid(const Report& r) noexcept {
    if (r.magic!=Magic || r.version!=Version || r.bytes!=sizeof(r) || (r.generation&1) ||
        !r.generation || r.state<1 || r.state>3 || r.stop>8 || r.count>Capacity || r.readCalls>64 ||
        r.callbacks>CallbackBudget || (r.presentMask&~15u) || !(r.presentMask&8) || !r.threadId ||
        !cfgprobe::ObjectPointer(r.pawn)) return false;
    if ((r.state==static_cast<unsigned>(State::Prepared) && (r.count || r.callbacks || r.stop || r.elapsedMs)) ||
        (r.state==static_cast<unsigned>(State::Capturing) && r.stop) ||
        (r.state==static_cast<unsigned>(State::Complete) && !r.stop)) return false;
    if (r.state==static_cast<unsigned>(State::Complete) && r.stop==static_cast<unsigned>(Stop::Events) &&
        (r.count!=Capacity || r.events[Capacity-1].outcome==0)) return false;
    for (unsigned i=0;i<4;++i) {
        if (r.presentMask&(1u<<i)) {
            if (!cfgprobe::ObjectPointer(r.functions[i].slot.object) || r.functions[i].slot.index<0 || r.functions[i].slot.serial<0) return false;
            for (unsigned j=0;j<i;++j) if ((r.presentMask&(1u<<j)) && r.functions[i].slot.object==r.functions[j].slot.object) return false;
        } else if (r.functions[i].slot.object) return false;
    }
    for (unsigned i=0;i<r.count;++i) {
        const auto& e=r.events[i];
        if (e.sequence!=i+1 || e.binding>=4 || !(r.presentMask&(1u<<e.binding)) ||
            e.parameterBytes!=Bindings[e.binding].bytes || e.readCalls>64 ||
            e.function!=r.functions[e.binding].slot.object || e.functionName!=r.functions[e.binding].name ||
            e.functionSerial!=r.functions[e.binding].slot.serial || e.elapsedMs>WindowMs ||
            (i && e.elapsedMs<r.events[i-1].elapsedMs) || e.afterReads>64 || e.outcome>6) return false;
        if ((e.outcome==1 && (!e.montageAfter || e.montageBefore==e.montageAfter || (e.binding!=1 && e.binding!=3))) ||
            (e.outcome==2 && (!e.montageBefore || e.montageAfter)) ||
            (e.outcome==3 && e.montageBefore!=e.montageAfter) ||
            (e.outcome==6 && e.montageBefore==e.montageAfter)) return false;
        std::uintptr_t reference=0;
        if (e.parameterBytes) std::memcpy(&reference,e.parameters.data(),sizeof(reference));
        if (reference!=e.reference.object || (reference && (!cfgprobe::ObjectPointer(reference) ||
            e.reference.index<0 || e.reference.serial<0))) return false;
    }
    return true;
}
inline bool SameReport(const Report& a,const Report& b) noexcept {
    return Valid(a) && Valid(b) && std::memcmp(&a,&b,sizeof(a))==0;
}
class Trace {
public:
    // Preparation is quiescent and before installation. Cached identities stay
    // immutable; lazy cache adoption, scans and identity refresh are forbidden.
    template<class Reader> bool Prepare(Reader& r,const cfgobserver::Plan& p,std::uintptr_t counselorClass) {
        if (prepared_ || !cfgobserver::SanePlan(p) || !cfgprobe::ObjectPointer(counselorClass)) return false;
        std::array<Function,4> found{}; std::uint32_t mask=0;
        for (unsigned i=0;i<4;++i) {
            std::uintptr_t function=0,outer=0; std::uint32_t flags=0;
            if (!r.Read(p.module+Bindings[i].cache,function)) return false;
            if (!function) continue;
            if (!cfgprobe::lifetime::ReadSlot(r,p.objectArray,function,found[i].slot) ||
                !r.Read(function+0x20,outer) || outer!=counselorClass || !r.Read(function+0x18,found[i].name) ||
                !r.Read(p.objectArray.data+static_cast<std::uintptr_t>(found[i].slot.index)*24+8,flags) ||
                (flags&0x20000000)) return false;
            for (unsigned j=0;j<i;++j) if (found[j].slot.object==function) return false;
            mask|=1u<<i;
        }
        // User warms a native emote before pausing; its exact server factory
        // must already be populated. A missing cache refuses without guessing.
        if (!(mask&8)) return false;
        plan_=p; counselorClass_=counselorClass; prepared_=armed_=true;
        report.functions=found; report.presentMask=mask; report.pawn=p.pawn; report.threadId=p.threadId;
        report.state=static_cast<unsigned>(State::Prepared); report.generation=2;
        return true;
    }
    bool Candidate(std::uintptr_t object,std::uint32_t thread,bool nested) const noexcept {
        return thread==plan_.threadId && object==plan_.pawn && !nested && armed_;
    }
    bool Owner(std::uintptr_t object,std::uint32_t thread,bool nested) const noexcept {
        return thread==plan_.threadId && object==plan_.pawn && !nested && prepared_;
    }
    template<class Access> void Observe(Access& a,std::uintptr_t function,std::uintptr_t parameters,std::uint64_t now) {
        if (!armed_) return;
        pending_=0;
        if (!started_) { started_=true; first_=last_=now; Publish(a,State::Capturing,Stop::None,0); }
        if (now<last_) { Finish(a,Stop::Clock,now); return; }
        last_=now;
        if (now-first_>=WindowMs) { Finish(a,Stop::Time,now); return; }
        ++callbacks_;
        if (callbacks_>=CallbackBudget) { Finish(a,Stop::Callbacks,now); return; }
        unsigned id=4;
        for (unsigned i=0;i<4;++i) if (report.functions[i].slot.object && report.functions[i].slot.object==function) { id=i; break; }
        if (id==4) return; // Routine calls: zero memory reads, no report write.
        if (!cfgobserver::ContextUnchanged(a,plan_)) { Finish(a,Stop::Context,now); return; }
        // Additional native rejection flags, not a claim of complete GC safety.
        for (const auto& object:plan_.objects) {
            std::uint32_t flags=0;
            if (!a.Read(plan_.objectArray.data+static_cast<std::uintptr_t>(object.index)*24+8,flags) ||
                (flags&0x20000000)) { Finish(a,Stop::Context,now); return; }
        }
        const auto& expected=report.functions[id];
        std::uintptr_t cache=0,outer=0; std::uint64_t name=0; std::uint32_t flags=0;
        cfgprobe::lifetime::SlotObservation slot;
        if (!a.Read(plan_.module+Bindings[id].cache,cache) || cache!=function ||
            !cfgprobe::lifetime::ReadSlot(a,plan_.objectArray,function,slot) || !(slot==expected.slot) ||
            !a.Read(function+0x20,outer) || outer!=counselorClass_ ||
            !a.Read(function+0x18,name) || name!=expected.name ||
            !a.Read(plan_.objectArray.data+static_cast<std::uintptr_t>(slot.index)*24+8,flags) ||
            (flags&0x20000000)) { Finish(a,Stop::Binding,now); return; }
        Event event; event.sequence=report.count+1; event.binding=id; event.parameterBytes=Bindings[id].bytes;
        event.elapsedMs=now-first_; event.function=function; event.functionName=name; event.functionSerial=slot.serial;
        if (event.parameterBytes && !a.ReadBytes(parameters,event.parameters.data(),event.parameterBytes)) { Finish(a,Stop::Parameter,now); return; }
        std::uintptr_t reference=0;
        if (event.parameterBytes) std::memcpy(&reference,event.parameters.data(),sizeof(reference));
        if (reference && (!cfgprobe::lifetime::ReadSlot(a,plan_.objectArray,reference,event.reference) ||
            !a.Read(reference+0x18,event.referenceName) || !a.Read(reference+cfgprobe::ObjectClass,event.referenceClass) ||
            !a.Read(reference+0x20,event.referenceOuter) ||
            !a.Read(plan_.objectArray.data+static_cast<std::uintptr_t>(event.reference.index)*24+8,flags) ||
            (flags&0x20000000))) { Finish(a,Stop::Parameter,now); return; }
        if (!a.Read(plan_.pawn+CurrentEmoteOffset,event.montageBefore)) { Finish(a,Stop::Parameter,now); return; }
        event.readCalls=a.Reads();
        Begin(a); report.events[report.count++]=event; report.callbacks=callbacks_; report.readCalls=event.readCalls;
        report.elapsedMs=event.elapsedMs;
        pending_=report.count;
        if (report.count==Capacity) armed_=false; // Terminal report waits for original's completion.
        End(a);
    }
    template<class Access> void After(Access& a,std::uintptr_t function,bool completed) {
        if (!pending_) return;
        const auto index=pending_-1; pending_=0;
        const auto entry=report.events[index];
        if (entry.function!=function) return;
        std::uintptr_t montage=0; std::uint64_t name=0;
        Outcome outcome=Outcome::OriginalException;
        if (completed) {
            bool same=cfgobserver::ContextUnchanged(a,plan_);
            for (const auto& object:plan_.objects) {
                std::uint32_t flags=0;
                if (same && (!a.Read(plan_.objectArray.data+static_cast<std::uintptr_t>(object.index)*24+8,flags) || (flags&0x20000000))) same=false;
            }
            same=same && a.Read(plan_.pawn+CurrentEmoteOffset,montage);
            if (same && montage) {
                cfgprobe::lifetime::SlotObservation slot; std::uint32_t flags=0;
                same=cfgprobe::lifetime::ReadSlot(a,plan_.objectArray,montage,slot) &&
                    a.Read(montage+0x18,name) &&
                    a.Read(plan_.objectArray.data+static_cast<std::uintptr_t>(slot.index)*24+8,flags) && !(flags&0x20000000);
            }
            outcome=!same?Outcome::ReadRefused:
                montage && montage!=entry.montageBefore && (entry.binding==1 || entry.binding==3)?Outcome::MontageStarted:
                !montage && entry.montageBefore?Outcome::MontageCleared:
                montage==entry.montageBefore?Outcome::Unchanged:Outcome::OtherTransition;
        }
        Begin(a); auto& event=report.events[index]; event.outcome=static_cast<unsigned>(outcome); event.afterReads=a.Reads();
        if (outcome!=Outcome::ReadRefused && completed) { event.montageAfter=montage; event.montageName=name; }
        if (outcome==Outcome::ReadRefused || !completed) { armed_=false; report.stop=static_cast<unsigned>(Stop::Completion); report.state=static_cast<unsigned>(State::Complete); }
        else if (report.count==Capacity) { report.stop=static_cast<unsigned>(Stop::Events); report.state=static_cast<unsigned>(State::Complete); }
        End(a);
    }
    bool Armed() const noexcept { return armed_; }
    Report report{}; // External consumer only via two matching bounded snapshots.
private:
    template<class Access> void Begin(Access& a) { a.Publish(report.generation,report.generation+1); }
    template<class Access> void End(Access& a) { a.Publish(report.generation,report.generation+1); }
    template<class Access> void Publish(Access& a,State state,Stop stop,std::uint64_t elapsed) {
        Begin(a); report.state=static_cast<unsigned>(state); report.stop=static_cast<unsigned>(stop);
        report.callbacks=callbacks_; report.elapsedMs=elapsed; report.readCalls=a.Reads(); End(a);
    }
    template<class Access> void Finish(Access& a,Stop reason,std::uint64_t now) {
        armed_=false; Publish(a,State::Complete,reason,now>=first_?now-first_:0);
    }
    cfgobserver::Plan plan_{};
    std::uintptr_t counselorClass_=0;
    std::uint64_t first_=0,last_=0;
    unsigned callbacks_=0,pending_=0;
    bool prepared_=false,armed_=false,started_=false;
};
} // namespace cfgemote
