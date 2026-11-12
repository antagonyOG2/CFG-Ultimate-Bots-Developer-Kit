#pragma once
#include "NativeMenuCallbackSlots.hpp"
#include "NativeCounselorRoutePolicy.hpp"
#include "NativeOfflineEntryReturnPolicy.hpp"
#include "NativePreparedCounselorConstruct.hpp"
#include "../RuntimeProbe/OfflineEntryRouteObservation.hpp"
namespace cfgmenu::pickercoordinator {
constexpr unsigned CounselorCells=5,EntryCells=3,Cells=CounselorCells+EntryCells;
constexpr unsigned CallbackDepthLimit=16,GuardReadLimit=16384,AdmissionReadLimit=16384,ConstructReadLimit=16384;
constexpr unsigned GuardCallsPerCallback=1,OwnerGetterCallsPerAdmission=1,OriginalCallsPerCallback=1;
constexpr std::uint64_t PublicationIntervalMs=25;
enum class Kind {Other,EntryClick,EntryGraph,EntryTransition,Construct,Accept,Back,Transition};
enum class CounselorHudArrival {Unobserved,Verified,AwaitGuardedInput};
struct Call {unsigned cell=Cells;std::uintptr_t widget=0,frame=0,result=0;};
// Entry boundary uses only immutable prepublication facts. A foreign native
// thread forwards without touching coordinator state or owner-thread reports.
template<class Forward> bool ForwardForeign(std::uint32_t current,std::uint32_t owner,std::uintptr_t original,const Call& call,Forward&& forward)noexcept{
    if(current==owner)return false;forward(original,call);return true;
}
struct Invocation {Kind kind=Kind::Other;active::Context actual{};counselortransition::Packet parameter{};};
struct Expected {pickerprepare::Executors counselor{};entryroute::Executors entry{};};
struct Configuration {
    active::Context source{};std::uintptr_t counselor=0,map=0,settings=0,sandbox=0;
    callbackslots::Plan plan{};Expected original{};
};
// These are exact typed executor expectations, not permission to ignore a slot.
// The first five cells are counselor Graph/Construct/Accept/Transition/Back;
// only then are entry Click/Graph/Transition published. Every cell retains its
// original interpreter. The host checks the cell addresses against metadata.
inline bool Shape(const Configuration& c)noexcept{
    if(!callbackslots::Shape(c.plan)||c.plan.count!=Cells)return false;
    const auto original=c.source.module+startdisk::ScriptExecutor;
    for(unsigned i=0;i<Cells;++i)if(c.plan.cells[i].original!=original)return false;
    for(unsigned i=0;i<pickerprepare::Roles;++i)if(c.original.counselor.values[i]!=(i<CounselorCells?original:0))return false;
    for(auto value:c.original.entry.values)if(value!=original)return false;
    return true;
}
inline Expected Executors(const Configuration& c,unsigned installed)noexcept{
    auto out=c.original;if(installed>Cells)return {};
    for(unsigned i=0;i<installed;++i){const auto value=c.plan.cells[i].replacement;
        if(i<CounselorCells)out.counselor.values[i]=value;else out.entry.values[i-CounselorCells]=value;}
    return out;
}
inline bool Compatible(unsigned cell,Kind kind)noexcept{
    if(cell==0)return kind==Kind::Other||kind==Kind::Construct||kind==Kind::Accept||kind==Kind::Back;
    if(cell==1)return kind==Kind::Construct;if(cell==2)return kind==Kind::Accept;
    if(cell==3)return kind==Kind::Transition;if(cell==4)return kind==Kind::Back;
    if(cell==5)return kind==Kind::EntryClick;if(cell==6)return kind==Kind::EntryGraph||kind==Kind::Other;
    return cell==7&&kind==Kind::EntryTransition;
}
// Native host contract (all callback operations noexcept, bounded, no scans):
// Resident/Current/Exchange are NativeMenuCallbackSlots' exact publication host.
// Guard(call,expected,receiver,constructForwarding,out) observes the ACTUAL active context,
// exact original wrapper/graph, real FFrame and positive identities. Entry uses
// entryroute::Borrowed (graph DB only); counselor graph classification uses the
// captured Construct/Accept/Back entries, NEVER a requested/synthetic context.
// It verifies Current/OwnedReceiverCurrent, retained source class references,
// and ConsumerCurrent + Borrowed for a transition. It must prove the destination
// reference still equals Configuration's counselor/map; an enum or name alone
// is not authority. Each guard uses one shared <=GuardReadLimit reader budget.
// AdmitConstruct calls FirstOwnedConstruct ONCE (one protected actual-instance
// GetOwningPlayer), with <=AdmissionReadLimit reads, and returns that Receiver.
// Construct validates CounselorState and uses NativePreparedCastHost plus the
// supplied ScopedConstruct and forward closure (<=ConstructReadLimit reads).
// Forward(original,call) is the resident host's protected original executor;
// ExchangeParameter is conditional exchange of ONLY the real borrowed 8B input;
// it cannot dispatch native callbacks or otherwise re-enter this coordinator.
// No host may publish, synthesize a frame, allocate serials, call a picker save,
// or reinterpret a failed guard as authority. This coordinator is orchestration,
// not an alternate native ownership/metadata proof.
class Coordinator {
    Configuration config_{};callbackslots::Transaction slots_{};counselorroute::Transaction route_{};
    entryreturn::Policy return_{};
    pickerprepare::Receiver receiver_{};counselorpicker::ScopedConstruct construct_{};
    counselortransition::State parameter_{};unsigned depth_=0,entryDepth_=0,acceptDepth_=0;
    bool bound_=false,admissionAttempted_=false,ticked_=false;std::uint64_t lastTick_=0;
    CounselorHudArrival counselorHudArrival_=CounselorHudArrival::Unobserved;
    bool IsReceiver(const active::Context& c)const noexcept{
        return receiver_.widget&&receiver_.widget==c.widget&&receiver_.type==c.type&&
            receiver_.identity.HasAssignedSerial()&&receiver_.identity==c.identities[2]&&
            receiver_.owningController.HasAssignedSerial()&&receiver_.owningController==c.identities[1];
    }
public:
    const pickerprepare::Receiver& Receiver()const noexcept{return receiver_;}
    CounselorHudArrival CounselorHudState()const noexcept{return counselorHudArrival_;}
    counselorpicker::ConstructState ConstructState()const noexcept{return construct_.State();}
    counselorroute::State RouteState()const noexcept{return route_.CurrentState();}
    callbackslots::State PublicationState()const noexcept{return slots_.CurrentState();}
    unsigned Installed()const noexcept{return slots_.Installed();}
    const callbackslots::Plan& PublishedPlan()const noexcept{return slots_.BoundPlan();}
    // New post-match native frontend epoch. The host revalidates the ACTUAL
    // offline entry, all eight installed cells and unchanged class/graph edges.
    // Retain the publication ledger/original executors; reset only per-instance
    // route/admission/borrow state. Never republish or synthesize Construct.
    template<class Host> bool Rearm(Host& host,const active::Context& actual)noexcept{
        if(depth_||entryDepth_||acceptDepth_||slots_.CurrentState()!=callbackslots::State::Installed||
           construct_.State()==counselorpicker::ConstructState::Forwarding||construct_.State()==counselorpicker::ConstructState::Indeterminate||
           actual.type!=config_.source.type||actual.module!=config_.source.module||actual.thread!=config_.source.thread||
           !host.EpochCurrent(actual,ExpectedExecutors()))return false;
        counselorroute::Transaction route;entryreturn::Policy returning;
        if(!route.Bind(actual,config_.counselor,config_.map,config_.settings,config_.sandbox)||!returning.Bind(actual,config_.counselor))return false;
        config_.source=actual;route_=route;return_=returning;receiver_={};construct_={};parameter_={};
        admissionAttempted_=false;counselorHudArrival_=CounselorHudArrival::Unobserved;return true;
    }
    Expected ExpectedExecutors()const noexcept{return Executors(config_,slots_.Installed());}
    entryreturn::State ReturnState()const noexcept{return return_.CurrentState();}
    template<class Host> bool ObserveReturn(Host& host,const active::Context& actual,entryreturn::Ticket& ticket)noexcept{
        return slots_.CurrentState()==callbackslots::State::Installed&&route_.CurrentState()==counselorroute::State::EntryReady&&
            return_.Observe(host,actual,ticket);
    }
    template<class Host> bool CompleteReturn(Host& host,const entryreturn::Ticket& ticket)noexcept{
        if(depth_||slots_.CurrentState()!=callbackslots::State::Installed||route_.CurrentState()!=counselorroute::State::EntryReady||
           !return_.CommitRelabeled(host,ticket)||!route_.RebindEntry(ticket.entry))return false;
        config_.source=ticket.entry;receiver_={};admissionAttempted_=false;construct_={};counselorHudArrival_=CounselorHudArrival::Unobserved;return true;
    }
    bool ReturnFromSelection(const active::Context& actual)noexcept{
        if(depth_||entryDepth_||acceptDepth_||slots_.CurrentState()!=callbackslots::State::Installed)return false;
        // Both policies must admit the same actual native return before either
        // ledger changes. A failed source/identity proof cannot half-cancel it.
        auto route=route_;auto returning=return_;
        if(!route.ReturnToEntry(actual)||!returning.BeginSelectionReturn(actual))return false;
        route_=route;return_=returning;return true;
    }
    template<class Host> bool Prepare(Host& host,const Configuration& config){
        if(bound_||!Shape(config))return false;
        if(!route_.Bind(config.source,config.counselor,config.map,config.settings,config.sandbox)||!return_.Bind(config.source,config.counselor))return false;
        config_=config;bound_=true;if(!slots_.Prepare(host,config.plan)){route_.Retire();return false;}return true;
    }
    // Call from the ONE existing HUD phase, not a new timer/polling transport.
    template<class Host> bool PrepareInherited(Host& host,const Configuration& config,const Coordinator& prior){
        if(bound_||!Shape(config)||prior.PublicationState()!=callbackslots::State::Installed||
           prior.depth_||prior.entryDepth_||prior.acceptDepth_||
           prior.construct_.State()==counselorpicker::ConstructState::Forwarding||
           prior.construct_.State()==counselorpicker::ConstructState::Indeterminate)return false;
        if(!route_.Bind(config.source,config.counselor,config.map,config.settings,config.sandbox)||!return_.Bind(config.source,config.counselor))return false;
        config_=config;bound_=true;
        if(!slots_.PrepareInherited(host,config.plan,prior.PublishedPlan(),CounselorCells)){route_.Retire();return false;}
        return true;
    }
    // One cell maximum per eligible phase, including reverse conditional restore.
    template<class Host> bool Tick(Host& host,std::uint64_t now){
        if(!bound_||(ticked_&&(now<lastTick_||now-lastTick_<PublicationIntervalMs)))return false;
        ticked_=true;lastTick_=now;const bool changed=slots_.Step(host);
        const auto s=slots_.CurrentState();if(s==callbackslots::State::Indeterminate||s==callbackslots::State::Refused)route_.Retire();
        return changed;
    }
    template<class Host> bool Restore(Host& host){Retire();return slots_.Restore(host);}
    void Retire()noexcept{route_.Retire();return_.Retire();}
    // Map/settings arrival is an explicit already-validated host observation;
    // never substitute a manufactured counselor Context during construction.
    template<class Host> bool Arrive(Host& host,const active::Context& actual)noexcept{
        static_assert(noexcept(host.ArrivalCurrent(actual,ExpectedExecutors(),receiver_)));
        if(slots_.CurrentState()!=callbackslots::State::Installed||!host.ArrivalCurrent(actual,ExpectedExecutors(),receiver_))return false;
        if(actual.type==config_.counselor&&!IsReceiver(actual))return false;
        return route_.Arrive(actual);
    }
    // Passive HUD observation is not the user's selection transaction. Native
    // Construct/metadata may still be settling on its first draw. One failed
    // read grants NO authority and leaves AwaitCounselor intact; the existing
    // exact, positive-owner Accept guard can independently prove arrival later.
    // Do not retry expensive metadata on every HUD draw or poison that guard.
    template<class Host> bool ObserveCounselorHud(Host& host,const active::Context& actual)noexcept {
        if(counselorHudArrival_!=CounselorHudArrival::Unobserved||route_.CurrentState()!=counselorroute::State::AwaitCounselor||
            actual.type!=config_.counselor)return false;
        counselorHudArrival_=Arrive(host,actual)?CounselorHudArrival::Verified:CounselorHudArrival::AwaitGuardedInput;
        return counselorHudArrival_==CounselorHudArrival::Verified;
    }
    template<class Host> bool Submit(Host& host,const active::Context& actual)noexcept{
        static_assert(noexcept(host.RouteCurrent(actual,ExpectedExecutors(),receiver_)));
        return slots_.CurrentState()==callbackslots::State::Installed&&host.RouteCurrent(actual,ExpectedExecutors(),receiver_)&&route_.Submit(actual);
    }
    template<class Host> bool Invoke(Host& host,const Call& call)noexcept{
        if(!bound_||call.cell>=Cells)return false; // No known original exists.
        const auto original=config_.plan.cells[call.cell].original;bool called=false,returned=false;
        const auto forward=[&]()noexcept{
            static_assert(noexcept(host.Forward(original,call)));
            if(!called){called=true;returned=host.Forward(original,call);if(!returned)route_.Retire();}return returned;
        };
        if(depth_==CallbackDepthLimit||slots_.CurrentState()!=callbackslots::State::Installed||
           (call.cell>=CounselorCells&&return_.CurrentState()!=entryreturn::State::Ready)){return forward();}
        ++depth_;Invocation invocation;const auto expected=ExpectedExecutors();
        const bool constructing=construct_.State()==counselorpicker::ConstructState::Forwarding;
        static_assert(noexcept(host.Guard(call,expected,receiver_,constructing,invocation)));
        if(!host.Guard(call,expected,receiver_,constructing,invocation)||!Compatible(call.cell,invocation.kind)){forward();--depth_;return returned;}
        const auto& actual=invocation.actual;const auto kind=invocation.kind;
        if(kind==Kind::Construct){
            // FirstConstruct proved the actual callback receiver independently;
            // native Construct may run before the active menu top is replaced.
            if(route_.RevisitCounselor(actual,true)){
                receiver_={};admissionAttempted_=false;construct_={};counselorHudArrival_=CounselorHudArrival::Unobserved;
            }
            if(!receiver_.widget&&!admissionAttempted_&&route_.CurrentState()==counselorroute::State::AwaitCounselor){
                admissionAttempted_=true;pickerprepare::Receiver admitted;
                static_assert(noexcept(host.AdmitConstruct(call,expected,actual,admitted)));
                if(host.AdmitConstruct(call,expected,actual,admitted)&&admitted.widget==call.widget&&admitted.type==config_.counselor&&
                   admitted.identity.object==call.widget&&admitted.identity.HasAssignedSerial()&&
                   admitted.owningController.HasAssignedSerial()&&admitted.owningController==actual.identities[1])receiver_=admitted;
                else route_.Retire();
            }
            if(receiver_.widget==call.widget&&receiver_.widget){
                // Successful later independent constructs may get a fresh scope;
                // a Refused/Indeterminate scope is never silently retried.
                if(construct_.State()==counselorpicker::ConstructState::Returned)construct_={};
                static_assert(noexcept(host.Construct(call,expected,receiver_,construct_,forward)));
                host.Construct(call,expected,receiver_,construct_,forward);
                // RestoredAwaitValidation owns no route authority and never
                // re-arms the constructor scope. Only a later independent
                // source/receiver/frame Guard may advance this native menu.
                if(construct_.State()==counselorpicker::ConstructState::Indeterminate||construct_.State()==counselorpicker::ConstructState::Refused)route_.Retire();
            }
            forward();--depth_;return returned;
        }
        // A cached native picker may become visible without Construct. The
        // actual guarded Accept/Back and admitted positive owner prove revisit.
        if(IsReceiver(actual)&&(kind==Kind::Accept||kind==Kind::Back))route_.RevisitCounselor(actual);
        if(IsReceiver(actual)&&route_.CurrentState()==counselorroute::State::AwaitCounselor)route_.Arrive(actual);
        if(kind==Kind::EntryClick||kind==Kind::EntryGraph){
            // A published route is not a finished view. Do not launch stock
            // Sandbox or an unserviceable picker while labels/order are still
            // initializing or have refused. Other native menu events pass on.
            static_assert(noexcept(host.EntryReady()));
            if(!entryDepth_&&!host.EntryReady()){--depth_;return true;}
            const bool outer=!entryDepth_;if(outer&&route_.RequestEntry(actual)){receiver_={};admissionAttempted_=false;construct_={};counselorHudArrival_=CounselorHudArrival::Unobserved;}
            ++entryDepth_;forward();--entryDepth_;
            if(outer&&route_.CurrentState()==counselorroute::State::EntryRequested)route_.Cancel(actual);
        }else if(kind==Kind::Accept){
            const bool outer=!acceptDepth_;if(outer&&IsReceiver(actual))route_.RequestAccept(actual);
            ++acceptDepth_;forward();--acceptDepth_;
            // Captured Accept calls PlayTransitionOut synchronously. A click
            // that has not exited grants no authority to a later null event.
            if(outer&&route_.CurrentState()==counselorroute::State::AcceptRequested)route_.CancelAccept(actual);
        }
        else if(kind==Kind::Back){if(IsReceiver(actual)&&route_.Cancel(actual)&&
                !return_.Begin(actual,receiver_.identity,receiver_.owningController))Retire();forward();}
        else if(kind==Kind::EntryTransition||kind==Kind::Transition){
            const bool entry=kind==Kind::EntryTransition;const auto step=entry?counselorroute::Step::Entry:counselorroute::Step::AcceptedCounselor;
            counselorroute::Token token;const auto previous=entry?config_.sandbox:0;
            const bool authorized=(entry?entryDepth_!=0:acceptDepth_!=0&&IsReceiver(actual))&&invocation.parameter.value==previous&&route_.Borrow(actual,step,previous,token);
            counselortransition::Lease lease;lease.generation=lease.acceptedGeneration=token.generation;lease.targetClass=token.destination;
            bool consumed=false;
            const auto exchange=[&](std::uintptr_t at,std::uintptr_t before,std::uintptr_t after)noexcept{
                static_assert(noexcept(host.ExchangeParameter(at,before,after)));
                if(!host.ExchangeParameter(at,before,after))return false;
                if(!consumed){consumed=route_.Consume(actual,token);if(!consumed){
                    route_.Retire();host.ExchangeParameter(at,after,before);return false;}}
                return true;
            };
            const auto result=counselortransition::ForwardBorrowed(parameter_,invocation.parameter,lease,authorized,exchange,[&]()noexcept{forward();},previous);
            if(result==counselortransition::Result::RestoreConflict)route_.Retire();
        }else forward();
        --depth_;return returned;
    }
};
}
