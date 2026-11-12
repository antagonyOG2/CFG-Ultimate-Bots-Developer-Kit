// Actual trace callback compiled only into an owned synthetic test executable.
// Existing fixture main is not executed because it expects generic Tick capture.
#define CFG_EMOTE_TRACE
#define main UnusedGenericFixtureMain
#include "ProductionCallbackTests.cpp"
#undef main
namespace {
std::uintptr_t ownedStartMontage=0;
bool raiseNative=false;
void __fastcall TraceOriginal(void* object,void* function,void* parameters) {
    if (object==reinterpret_cast<void*>(plan.pawn)) {
        const auto fn=reinterpret_cast<std::uintptr_t>(function);
        if (raiseNative && fn==emoteTrace.report.functions[3].slot.object) RaiseException(FixtureException,0,0,nullptr);
        if (fn==emoteTrace.report.functions[1].slot.object || fn==emoteTrace.report.functions[3].slot.object)
            *reinterpret_cast<std::uintptr_t*>(plan.pawn+cfgemote::CurrentEmoteOffset)=ownedStartMontage;
        if (fn==emoteTrace.report.functions[0].slot.object || fn==emoteTrace.report.functions[2].slot.object)
            *reinterpret_cast<std::uintptr_t*>(plan.pawn+cfgemote::CurrentEmoteOffset)=0;
    }
    OwnedOriginal(object,function,parameters);
}
bool CatchNativeException(void* pawn,void* fn,void* params) {
    __try { Observe(pawn,fn,params); }
    __except(GetExceptionCode()==FixtureException?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) { return true; }
    return false;
}
bool SetupTrace(Memory& memory,cfgemote::Trace& trace) {
    CHECK(SetupOwnedContext(memory));
    CHECK(memory.Put(plan.pawn+cfgemote::CurrentEmoteOffset,std::uintptr_t{0}));
    const auto nativeClass=memory.Reserve(8192);
    CHECK(nativeClass!=0);
    plan.objectArray.count=9;
    CHECK(memory.Put(plan.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,plan.objectArray.count));
    for (unsigned i=0;i<4;++i) {
        const auto function=memory.Reserve(8192); CHECK(function!=0);
        CHECK(memory.Put(plan.module+cfgemote::Bindings[i].cache,function));
        CHECK(memory.Put(function+0xC,static_cast<std::int32_t>(i+4)));
        CHECK(memory.Put(function+0x18,std::uint64_t{0x1000+i})); CHECK(memory.Put(function+0x20,nativeClass));
        CHECK(memory.Put(plan.objectArray.data+(i+4)*24,function));
        CHECK(memory.Put(plan.objectArray.data+(i+4)*24+8,std::uint32_t{0}));
        CHECK(memory.Put(plan.objectArray.data+(i+4)*24+16,static_cast<std::int32_t>(i+104)));
    }
    Reader reader; reader.cap=256; CHECK(trace.Prepare(reader,plan,nativeClass));
    CHECK(reader.reads==28 && cfgemote::Valid(trace.report)); return failures==0;
}
struct Access : Reader {
    bool ReadBytes(std::uintptr_t address,void* output,unsigned bytes) {
        if (!bytes || bytes>16 || reads>=cap || !cfgprobe::lifetime::Extent(address,bytes)) return false;
        ++reads; SIZE_T got=0; return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),output,bytes,&got) && got==bytes;
    }
    void Publish(std::uint32_t& value,std::uint32_t next) { InterlockedExchange(reinterpret_cast<volatile LONG*>(&value),static_cast<LONG>(next)); }
    unsigned Reads() const { return reads; }
};
}
int main() {
    Memory memory; CHECK(SetupTrace(memory,emoteTrace)); originalActorDispatcher=&TraceOriginal;
    auto* pawn=reinterpret_cast<void*>(plan.pawn);
    Observe(pawn,reinterpret_cast<void*>(0x4444),nullptr);
    CHECK(forwarded==1 && emoteTrace.report.state==static_cast<unsigned>(cfgemote::State::Capturing) && emoteTrace.report.count==0);
    CHECK(!emoteTrace.Candidate(plan.pawn,plan.threadId+1,false) && !emoteTrace.Candidate(plan.pawn,plan.threadId,true));
    const auto start=emoteTrace.report.generation;
    for (unsigned i=0;i<2000;++i) Observe(pawn,reinterpret_cast<void*>(0x4444),nullptr);
    CHECK(forwarded==2001 && emoteTrace.report.generation==start && emoteTrace.report.readCalls==0);
    // Native SERVER_PlayEmote reference is a class object, observed not retained.
    const auto asset=memory.Reserve(8192); CHECK(asset!=0);
    ownedStartMontage=asset;
    CHECK(memory.Put(asset+0xC,std::int32_t{8})); CHECK(memory.Put(asset+0x18,std::uint64_t{0x8888}));
    CHECK(memory.Put(asset+cfgprobe::ObjectClass,plan.mode)); CHECK(memory.Put(asset+0x20,plan.world));
    CHECK(memory.Put(plan.objectArray.data+8*24,asset)); CHECK(memory.Put(plan.objectArray.data+8*24+16,std::int32_t{108}));
    CHECK(memory.Put(plan.objectArray.data+8*24+8,std::uint32_t{0}));
    const auto function=emoteTrace.report.functions[3].slot.object;
    Observe(pawn,reinterpret_cast<void*>(function),const_cast<std::uintptr_t*>(&asset));
    CHECK(emoteTrace.report.count==1 && emoteTrace.report.events[0].binding==3 && emoteTrace.report.events[0].reference.object==asset);
    CHECK(emoteTrace.report.events[0].readCalls==50 && emoteTrace.report.events[0].afterReads==40 && cfgemote::Valid(emoteTrace.report) && !observerNested);
    CHECK(emoteTrace.report.events[0].outcome==static_cast<unsigned>(cfgemote::Outcome::MontageStarted));
    CHECK(emoteTrace.report.events[0].montageBefore==0 && emoteTrace.report.events[0].montageAfter==asset);
    // No spacing rule: consecutive server/cancel events may share one timestamp.
    Observe(pawn,reinterpret_cast<void*>(emoteTrace.report.functions[2].slot.object),nullptr);
    CHECK(emoteTrace.report.count==2 && cfgemote::Valid(emoteTrace.report));
    CHECK(emoteTrace.report.events[1].outcome==static_cast<unsigned>(cfgemote::Outcome::MontageCleared) && emoteTrace.report.events[1].afterReads==35);
    const std::array<std::uintptr_t,2> montageParameters{asset,1};
    Observe(pawn,reinterpret_cast<void*>(emoteTrace.report.functions[1].slot.object),const_cast<std::uintptr_t*>(montageParameters.data()));
    CHECK(emoteTrace.report.count==3 && emoteTrace.report.events[2].binding==1 && emoteTrace.report.events[2].parameterBytes==16);
    CHECK(emoteTrace.report.events[2].parameters[8]==1 && emoteTrace.report.events[2].reference.object==asset);
    Observe(pawn,reinterpret_cast<void*>(emoteTrace.report.functions[0].slot.object),nullptr);
    CHECK(emoteTrace.report.count==4 && emoteTrace.report.events[3].binding==0 && cfgemote::Valid(emoteTrace.report));
    // Exception propagation and TLS restoration preserve original behavior.
    CHECK(CatchOriginalException(pawn) && !observerNested);
    const auto events=emoteTrace.report.count;
    Observe(pawn,reinterpret_cast<void*>(NestedToken),nullptr);
    CHECK(!observerNested && emoteTrace.report.count==events);
    Observe(reinterpret_cast<void*>(plan.mode),reinterpret_cast<void*>(function),const_cast<std::uintptr_t*>(&asset));
    CHECK(emoteTrace.report.count==events);
    // Full context before selected reads; changed world stops without parameter access.
    CHECK(memory.Put(plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{0x10000}));
    Observe(pawn,reinterpret_cast<void*>(function),reinterpret_cast<void*>(1));
    CHECK(!emoteTrace.Armed() && emoteTrace.report.stop==static_cast<unsigned>(cfgemote::Stop::Context));
    CHECK(emoteTrace.report.count==events && cfgemote::SameReport(emoteTrace.report,emoteTrace.report));
    const auto stopped=emoteTrace.report.generation;
    Observe(pawn,reinterpret_cast<void*>(function),reinterpret_cast<void*>(1)); CHECK(emoteTrace.report.generation==stopped);
    // Independent traces use owned memory, deterministic time and negative cases.
    Memory clockMemory; cfgemote::Trace clockTrace; CHECK(SetupTrace(clockMemory,clockTrace));
    Access zero; clockTrace.Observe(zero,0x4444,0,100); CHECK(zero.reads==0);
    clockTrace.Observe(zero,0x4444,0,30100); CHECK(!clockTrace.Armed() && zero.reads==0 && clockTrace.report.stop==1);
    auto torn=clockTrace.report; ++torn.generation; CHECK(!cfgemote::SameReport(clockTrace.report,torn));
    auto bad=clockTrace.report; bad.count=33; CHECK(!cfgemote::Valid(bad));
    Memory backwardMemory; cfgemote::Trace backward; CHECK(SetupTrace(backwardMemory,backward));
    Access back; backward.Observe(back,0x4444,0,100); backward.Observe(back,0x4444,0,99); CHECK(backward.report.stop==4 && !backward.Armed());
    Memory budgetMemory; cfgemote::Trace budget; CHECK(SetupTrace(budgetMemory,budget));
    Access budgetAccess; for (unsigned i=0;i<4097;++i) budget.Observe(budgetAccess,0x4444,0,100);
    CHECK(!budget.Armed() && budget.report.callbacks==4096 && budget.report.stop==3 && budgetAccess.reads==0);
    const auto Fault=[&](unsigned kind,cfgemote::Stop expected) {
        Memory faultMemory; cfgemote::Trace fault; CHECK(SetupTrace(faultMemory,fault));
        const auto fn=fault.report.functions[3].slot.object;
        std::uintptr_t nullReference=0; const auto params=reinterpret_cast<std::uintptr_t>(&nullReference);
        if (kind==0) CHECK(faultMemory.Put(plan.module+cfgemote::Bindings[3].cache,std::uintptr_t{0}));
        if (kind==1) CHECK(faultMemory.Put(plan.objectArray.data+7*24+16,std::int32_t{999}));
        if (kind==2) CHECK(faultMemory.Put(fn+0x18,std::uint64_t{123}));
        if (kind==3) CHECK(faultMemory.Put(plan.objectArray.data+7*24+8,std::uint32_t{0x20000000}));
        Access faultAccess;
        if (kind==5) faultAccess.cap=35;
        fault.Observe(faultAccess,fn,kind==4?1:params,100);
        CHECK(!fault.Armed() && fault.report.count==0 && fault.report.stop==static_cast<unsigned>(expected));
        CHECK(cfgemote::Valid(fault.report) && fault.report.readCalls<=64);
    };
    for (unsigned i=0;i<6;++i) Fault(i,i==4?cfgemote::Stop::Parameter:cfgemote::Stop::Binding);
    Memory countMemory; cfgemote::Trace count; CHECK(SetupTrace(countMemory,count));
    for (unsigned i=0;i<32;++i) {
        Access eventAccess; const auto cancel=count.report.functions[2].slot.object; count.Observe(eventAccess,cancel,0,100);
        if (i==31) CHECK(count.report.state==2 && count.report.stop==0 && count.report.events[i].outcome==0 && cfgemote::Valid(count.report));
        Access done; count.After(done,cancel,true);
    }
    CHECK(!count.Armed() && count.report.count==32 && count.report.stop==2 && cfgemote::Valid(count.report));
    auto changed=count.report; ++changed.events[0].function; CHECK(!cfgemote::Valid(changed));
    changed=count.report; ++changed.generation; CHECK(!cfgemote::SameReport(count.report,changed));
    // Quiescent preparation: fail closed on missing mandatory cache, changed
    // owner, duplicate cache aliases or unreadable slots; never call a getter.
    const auto PrepFault=[&](unsigned kind) {
        Memory prepMemory; cfgemote::Trace seed; CHECK(SetupTrace(prepMemory,seed));
        const auto fn=seed.report.functions[3].slot.object;
        std::uintptr_t owner=0; Reader getOwner; CHECK(getOwner.Read(fn+0x20,owner));
        Reader noRearm; CHECK(!seed.Prepare(noRearm,plan,owner) && noRearm.reads==0);
        if (kind==0) CHECK(prepMemory.Put(plan.module+cfgemote::Bindings[3].cache,std::uintptr_t{0}));
        if (kind==1) CHECK(prepMemory.Put(fn+0x20,plan.world));
        if (kind==2) CHECK(prepMemory.Put(plan.module+cfgemote::Bindings[2].cache,fn));
        cfgemote::Trace refused; Reader prepRead; prepRead.cap=kind==3?1:256;
        CHECK(!refused.Prepare(prepRead,plan,owner) && !refused.Armed());
        CHECK(!cfgemote::Valid(refused.report));
    };
    for (unsigned i=0;i<4;++i) PrepFault(i);
    // Actual native-call exception must still propagate with TLS restored and
    // a completion marker, without trying any game-field reads in the finally.
    Memory exceptionMemory; emoteTrace=cfgemote::Trace{}; CHECK(SetupTrace(exceptionMemory,emoteTrace));
    pawn=reinterpret_cast<void*>(plan.pawn); std::uintptr_t nullReference=0; raiseNative=true;
    CHECK(CatchNativeException(pawn,reinterpret_cast<void*>(emoteTrace.report.functions[3].slot.object),&nullReference));
    raiseNative=false;
    CHECK(!observerNested && !emoteTrace.Armed() && emoteTrace.report.events[0].outcome==5 && emoteTrace.report.events[0].afterReads==0);
    CHECK(emoteTrace.report.stop==8 && cfgemote::Valid(emoteTrace.report));
    Memory completionMemory; cfgemote::Trace completion; CHECK(SetupTrace(completionMemory,completion));
    const auto play=completion.report.functions[3].slot.object; Access entryAccess;
    completion.Observe(entryAccess,play,reinterpret_cast<std::uintptr_t>(&nullReference),100);
    CHECK(completion.report.events[0].outcome==0);
    Access afterAccess; completion.After(afterAccess,play,true);
    CHECK(completion.report.events[0].outcome==3 && completion.report.events[0].afterReads==35 && cfgemote::Valid(completion.report));
    // Refusal after original completion is never labelled a successful start.
    Access secondEntry; completion.Observe(secondEntry,play,reinterpret_cast<std::uintptr_t>(&nullReference),101);
    CHECK(completionMemory.Put(plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{0x10000}));
    Access refusedAfter; completion.After(refusedAfter,play,true);
    CHECK(completion.report.events[1].outcome==4 && completion.report.stop==8 && !completion.Armed() && cfgemote::Valid(completion.report));
    Memory transitionMemory; cfgemote::Trace transition; CHECK(SetupTrace(transitionMemory,transition));
    const auto substitute=transition.report.functions[0].slot.object;
    CHECK(transitionMemory.Put(plan.pawn+cfgemote::CurrentEmoteOffset,transition.report.functions[1].slot.object));
    const auto cancel=transition.report.functions[2].slot.object; Access transitionEntry;
    transition.Observe(transitionEntry,cancel,0,100);
    CHECK(transitionMemory.Put(plan.pawn+cfgemote::CurrentEmoteOffset,substitute));
    Access transitionAfter; transition.After(transitionAfter,cancel,true);
    CHECK(transition.report.events[0].outcome==6 && cfgemote::Valid(transition.report));
    std::printf("CFG production native-emote trace tests: %s (%u failures); no game access\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
