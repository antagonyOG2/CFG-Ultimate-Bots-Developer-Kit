// Reuse the actual production callback and owned-memory fixture. No production
// preparation export is invoked; no test seam is built into either DLL.
#define CFG_STATS_ADAPTER
#define main ObserverForwardingFixtureMain
#include "ProductionCallbackTests.cpp"
#undef main
int main() {
    if (ObserverForwardingFixtureMain()!=0) return 1;
    // Capture is already disarmed. Stats requests must still work independently.
    Memory memory; CHECK(SetupOwnedContext(memory));
    const auto counselorClass=memory.Reserve(8192),modeClass=memory.Reserve(8192);
    CHECK(counselorClass && modeClass);
    const auto BindClass=[&](std::uintptr_t object,std::uintptr_t native,cfgprobe::NativeClass id) {
        const auto& binding=cfgprobe::NativeClasses[id];
        return memory.Put(object+cfgprobe::ObjectClass,native) && memory.Put(native+cfgprobe::ClassSuper,std::uintptr_t{0}) &&
            memory.Put(plan.module+binding.reflectionCacheRva,native) && memory.Put(plan.module+binding.constructorCacheRva,native);
    };
    CHECK(BindClass(plan.pawn,counselorClass,cfgprobe::Counselor)); CHECK(BindClass(plan.mode,modeClass,cfgprobe::SandboxMode));
    CHECK(memory.Put(plan.module+cfgstats::IndexedLoadRva,std::array<unsigned char,8>{0x0F,0xB6,0x8C,0x28,0xF4,0x13,0,0}));
    CHECK(memory.Put(plan.module+cfgstats::DescriptorRva,std::uint32_t{0}));
    CHECK(memory.Put(plan.module+cfgstats::DescriptorRva+8,plan.module+cfgstats::NameRva));
    CHECK(memory.Put(plan.module+cfgstats::NameRva,std::array<char,6>{'S','t','a','t','s',0}));
    CHECK(memory.Put(plan.module+cfgstats::DescriptorRva+32,std::uint32_t{7}));
    CHECK(memory.Put(plan.module+cfgstats::DescriptorRva+48,std::uint32_t{0x13F4}));
    for (unsigned i=0;i<4;++i) CHECK(memory.Put(plan.objectArray.data+i*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{0}));
    const cfgport::stats::Values native{4,10,2,9,6,3,1}; CHECK(memory.Put(plan.pawn+cfgstats::StatOffset,native));
    statsRuntime.Configure(StatsIdentity());
    cfgstats::Request request; request.bytes=sizeof(request); request.identity=StatsIdentity(); request.command=1;
    CHECK(CfgRequestStats(&request)==1 && CfgRequestStats(&request)==0);
    auto* pawn=reinterpret_cast<void*>(plan.pawn);
    Observe(reinterpret_cast<void*>(plan.mode),reinterpret_cast<void*>(0xABC0),nullptr); CHECK(statsRuntime.Pending());
    observerNested=true; Observe(pawn,reinterpret_cast<void*>(0xABC1),nullptr); observerNested=false; CHECK(statsRuntime.Pending());
    const auto calls=forwarded; Observe(pawn,reinterpret_cast<void*>(0xABC2),nullptr);
    CHECK(forwarded==calls+1 && statsRuntime.report.state==static_cast<unsigned>(cfgstats::State::Applied));
    CHECK(cfgstats::ValidReport(statsRuntime.report) && statsRuntime.report.nativeReads<=256 && !capture.Armed());
    Reader reader; cfgport::stats::Values actual{}; CHECK(reader.Read(plan.pawn+cfgstats::StatOffset,actual) && actual==cfgport::stats::Maximum());
    std::printf("Native owned stats apply read calls=%u\n",statsRuntime.report.nativeReads);
    request.command=2; CHECK(CfgRequestStats(&request)==1); Observe(pawn,reinterpret_cast<void*>(0xABC3),nullptr);
    CHECK(reader.Read(plan.pawn+cfgstats::StatOffset,actual) && actual==native && cfgstats::ValidReport(statsRuntime.report));
    CHECK(statsRuntime.report.state==static_cast<unsigned>(cfgstats::State::Restored));
    // Native-slot rejection disarms requests without touching stats.
    CHECK(memory.Put(plan.objectArray.data+3*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{0x20000000}));
    request.command=1; CHECK(CfgRequestStats(&request)==1); Observe(pawn,reinterpret_cast<void*>(0xABC4),nullptr);
    CHECK(statsRuntime.report.state==static_cast<unsigned>(cfgstats::State::Refused) && statsRuntime.report.writes==0);
    CHECK(reader.Read(plan.pawn+cfgstats::StatOffset,actual) && actual==native && CfgRequestStats(&request)==0);
    std::printf("CFG production stats callback tests: %s (%u failures); no game access\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
