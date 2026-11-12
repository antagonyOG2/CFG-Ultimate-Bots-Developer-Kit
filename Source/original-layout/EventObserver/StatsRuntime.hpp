#pragma once
#include "StatsBindings.hpp"
#include <atomic>
namespace cfgstats {
// No timer/polling loop. Only configured native owner callback may Service().
// External helper publishes an enum, never pawn memory or mutable report fields.
class Runtime {
public:
    void Configure(cfgport::stats::Identity identity) noexcept {
        report.identity=identity; ready_.store(identity.Valid(),std::memory_order_release);
    }
    bool Submit(const Request& r) noexcept {
        if (!ready_.load(std::memory_order_acquire)) return false;
        if (r.magic!=Magic || r.version!=Version || r.bytes!=sizeof(Request) ||
            !(r.identity==report.identity) || (r.command!=1 && r.command!=2)) return false;
        unsigned expected=0;
        if (!pending_.compare_exchange_strong(expected,Publishing,std::memory_order_acq_rel)) return false;
        if (!ready_.load(std::memory_order_acquire)) { pending_.store(0,std::memory_order_release); return false; }
        pending_.store(r.command,std::memory_order_release); return true;
    }
    bool Pending() const noexcept { const auto p=pending_.load(std::memory_order_acquire); return p==1 || p==2; }
    template<class Access> void Service(Access& access) noexcept {
        unsigned command=pending_.load(std::memory_order_acquire);
        if ((command!=1 && command!=2) || !pending_.compare_exchange_strong(command,Executing,std::memory_order_acq_rel)) return;
        if (!ready_.load(std::memory_order_acquire)) { pending_.store(0,std::memory_order_release); return; }
        if (report.operations>=16) { ready_.store(false,std::memory_order_release); pending_.store(0,std::memory_order_release); return; }
        access.Publish(report.generation,report.generation+1); // odd: do not consume
        ++report.operations; report.command=command; report.reason=0; report.error=0; report.writes=0;
        report.before={}; report.after={};
        Process(access,static_cast<Command>(command));
        report.nativeReads=access.Reads(); report.error=access.Error(); report.active=transaction_.Active()?1u:0u;
        access.Publish(report.generation,report.operations*2); // even: fields complete
        if (report.operations==16) ready_.store(false,std::memory_order_release);
        pending_.store(0,std::memory_order_release);
    }
    Report report{}; // fixed append-state snapshot via two external RPM reads only
private:
    template<class Access> void Stop(Access&,State state,Reason reason) noexcept {
        transaction_.Invalidate(); report.state=static_cast<unsigned>(state); report.reason=static_cast<unsigned>(reason);
        ready_.store(false,std::memory_order_release); // terminal; no retry/automatic rollback
    }
    template<class Access> void Process(Access& access,Command command) noexcept {
        if (!access.ContextMatches()) { Stop(access,State::Refused,Reason::Context); return; }
        cfgport::stats::Values current{};
        if (!access.ReadStats(current)) { Stop(access,State::Refused,Reason::Read); return; }
        report.before=current;
        if (!cfgport::stats::Valid(current)) { Stop(access,State::Refused,Reason::Values); return; }
        cfgport::stats::Evidence evidence{true,true,true,cfgport::Session::Sandbox};
        const auto proposal=command==Command::Enable?transaction_.PrepareEnable(evidence,report.identity,current):
            transaction_.PrepareDisable(evidence,report.identity,current);
        if (proposal.action==cfgport::stats::Action::None) {
            report.after=current; report.state=static_cast<unsigned>(command==Command::Enable?State::AlreadyMaximum:State::NoOverride); return;
        }
        if (proposal.action!=cfgport::stats::Action::Apply && proposal.action!=cfgport::stats::Action::Restore) {
            Stop(access,State::Refused,Reason::Changed); return;
        }
        cfgport::stats::Values expected{};
        if (!access.ReadStats(expected) || expected!=proposal.expected) { Stop(access,State::Refused,Reason::Changed); return; }
        if (!access.Writable()) { Stop(access,State::Refused,Reason::Region); return; }
        // Last context/slot/flags check before exactly one seven-byte write.
        if (!access.ContextMatches()) { Stop(access,State::Refused,Reason::Context); return; }
        report.writes=1;
        if (!access.WriteStats(proposal.desired)) { Stop(access,State::Indeterminate,Reason::Write); return; }
        cfgport::stats::Values after{};
        if (!access.ReadStats(after)) { Stop(access,State::Indeterminate,Reason::Readback); return; }
        report.after=after;
        if (!transaction_.Confirm(proposal.revision,report.identity,after)) { Stop(access,State::Indeterminate,Reason::Readback); return; }
        report.state=static_cast<unsigned>(command==Command::Enable?State::Applied:State::Restored);
    }
    static constexpr unsigned Executing=3,Publishing=4;
    std::atomic<unsigned> pending_{0}; std::atomic<bool> ready_{false};
    cfgport::stats::Transaction transaction_;
};
}
