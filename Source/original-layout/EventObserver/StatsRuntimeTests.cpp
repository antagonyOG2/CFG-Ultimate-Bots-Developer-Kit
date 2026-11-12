#include "StatsRuntime.hpp"
#include <cstdio>
#include <thread>
namespace {
unsigned failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
constexpr cfgport::stats::Identity identity{100,0x20000,103};
struct Access {
    cfgport::stats::Values values{4,10,2,9,6,3,1};
    unsigned reads=0,writes=0,contexts=0,error=0;
    bool badContext=false,lateContext=false,badRead=false,badRegion=false,partial=false,badReadback=false,changed=false;
    bool ContextMatches() noexcept { ++contexts; ++reads; return !badContext && !(lateContext && contexts==2); }
    bool ReadStats(cfgport::stats::Values& out) noexcept {
        ++reads; if (badRead) return false; out=values;
        if (changed && reads==3) out[0]=3;
        if (badReadback && writes) out[0]=2;
        return true;
    }
    bool Writable() noexcept { return !badRegion; }
    bool WriteStats(const cfgport::stats::Values& in) noexcept { ++writes; if (partial) { values[0]=in[0]; error=299; return false; } values=in; return true; }
    void Publish(std::uint32_t& word,std::uint32_t value) noexcept { word=value; }
    unsigned Reads() const noexcept { return reads; }
    unsigned Error() const noexcept { return error; }
};
cfgstats::Request Request(cfgstats::Command command) { cfgstats::Request r; r.bytes=sizeof(r); r.identity=identity; r.command=static_cast<unsigned>(command); return r; }
}
int main() {
    using cfgstats::Runtime; using cfgstats::Command; using cfgstats::State;
    using cfgstats::ValidReport; using cfgstats::SameReport;
    Runtime runtime; CHECK(!runtime.Submit(Request(Command::Enable))); runtime.Configure(identity);
    Access access; const auto original=access.values;
    CHECK(runtime.Submit(Request(Command::Enable))); CHECK(!runtime.Submit(Request(Command::Restore)));
    runtime.Service(access); CHECK(access.writes==1 && access.values==cfgport::stats::Maximum());
    CHECK(runtime.report.state==static_cast<unsigned>(State::Applied) && runtime.report.active==1 && ValidReport(runtime.report));
    access.reads=access.writes=access.contexts=0;
    CHECK(runtime.Submit(Request(Command::Restore))); runtime.Service(access);
    CHECK(access.writes==1 && access.values==original && runtime.report.state==static_cast<unsigned>(State::Restored));
    CHECK(!runtime.report.active && ValidReport(runtime.report));
    auto torn=runtime.report; ++torn.generation; CHECK(!SameReport(runtime.report,torn));
    torn=runtime.report; torn.after[0]=7; CHECK(!SameReport(runtime.report,torn));
    torn=runtime.report; torn.nativeReads=257; CHECK(!ValidReport(torn));
    for (unsigned test=0;test<8;++test) {
        Runtime candidate; candidate.Configure(identity); Access a;
        if (test==0) a.badContext=true;
        if (test==1) a.lateContext=true;
        if (test==2) a.badRead=true;
        if (test==3) a.badRegion=true;
        if (test==4) a.partial=true;
        if (test==5) a.badReadback=true;
        if (test==6) a.changed=true;
        if (test==7) a.values[0]=0;
        CHECK(candidate.Submit(Request(Command::Enable))); candidate.Service(a);
        CHECK(!candidate.Submit(Request(Command::Enable)) && !candidate.report.active && ValidReport(candidate.report));
        CHECK(a.writes==((test==4 || test==5)?1u:0u));
        CHECK(candidate.report.state==static_cast<unsigned>((test==4 || test==5)?State::Indeterminate:State::Refused));
    }
    Runtime conflict; conflict.Configure(identity); Access c;
    CHECK(conflict.Submit(Request(Command::Enable))); conflict.Service(c); c.values[0]=9; c.reads=c.contexts=c.writes=0;
    CHECK(conflict.Submit(Request(Command::Restore))); conflict.Service(c);
    CHECK(c.writes==0 && c.values[0]==9 && !conflict.Submit(Request(Command::Restore)));
    Runtime maximum; maximum.Configure(identity); Access m; m.values=cfgport::stats::Maximum();
    CHECK(maximum.Submit(Request(Command::Enable))); maximum.Service(m); CHECK(!maximum.report.active && m.writes==0);
    CHECK(maximum.Submit(Request(Command::Restore))); maximum.Service(m); CHECK(m.writes==0 && maximum.report.state==static_cast<unsigned>(State::NoOverride));
    Runtime concurrent; concurrent.Configure(identity); std::atomic<unsigned> accepted{0};
    auto producer=[&] { for (unsigned i=0;i<10000;++i) if (concurrent.Submit(Request(Command::Enable))) ++accepted; };
    std::thread first(producer),second(producer); first.join(); second.join(); CHECK(accepted==1);
    Access queued; concurrent.Service(queued); CHECK(queued.writes==1 && !concurrent.Pending());
    Runtime wrong; wrong.Configure(identity); auto request=Request(Command::Enable); request.identity.objectSerial=999;
    CHECK(!wrong.Submit(request)); request=Request(Command::Enable); request.command=3; CHECK(!wrong.Submit(request));
    Runtime bounded; bounded.Configure(identity); Access b;
    for (unsigned i=0;i<16;++i) { b.reads=b.contexts=b.writes=0; CHECK(bounded.Submit(Request(i%2?Command::Restore:Command::Enable))); bounded.Service(b); }
    CHECK(bounded.report.operations==16 && ValidReport(bounded.report));
    CHECK(!bounded.Submit(Request(Command::Enable)) && bounded.report.operations==16);
    std::printf("CFG stats mailbox/transaction tests: %s (%u failures); no game access\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
