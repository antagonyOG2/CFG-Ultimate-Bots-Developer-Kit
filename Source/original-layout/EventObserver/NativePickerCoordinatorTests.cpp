#include "NativePickerCoordinator.hpp"
#include <cstdio>
using namespace cfgmenu;
using namespace pickercoordinator;
namespace {
// OWNED orchestration model only. It is intentionally not a game Reader and
// cannot pass native reflection/ownership guards. Those have independent suites.
struct Host {
    Coordinator coordinator;Configuration config;active::Context actual;
    std::array<std::uintptr_t,Cells> cells{};std::array<unsigned,64> order{};unsigned writes=0;
    unsigned forwards=0,guards=0,getters=0,constructs=0,exchanges=0,widens=0,restores=0,arrivals=0;
    unsigned arrivalProofs=0,submitProofs=0;
    bool current=true,resident=true,guard=true,admit=true,exchange=true,nativeOkay=true;
    bool chainEntry=false,chainAccept=false,chainBack=false,chainConstruct=false,foreign=false,backDuringAccept=false;
    bool sawConsumed=false,expectedOkay=true,castWidened=false,restoredCheckFails=false;
    bool returnedLabels=false;
    bool entryReady=true;
    bool EntryReady()const noexcept{return entryReady;}
    std::uintptr_t parameter=0,persistent=0,seen=0,acceptParameter=0;
    constexpr static std::uintptr_t Address=0x600000,PickerWidget=0x700000;
    Host(){
        auto& s=config.source;s.module=0x100000;s.thread=5;s.world=0x110000;s.mode=0x120000;s.controller=0x130000;
        s.hud=0x140000;s.stack=0x150000;s.gameInstance=0x160000;s.widget=0x170000;s.type=0x180000;
        s.array={0x230000,2,4};
        s.identities={{{s.world,1,11},{s.controller,2,12},{s.widget,3,13},{s.hud,4,14}}};
        config.counselor=0x190000;config.map=0x200000;config.settings=0x210000;config.sandbox=0x220000;config.plan.count=Cells;
        for(unsigned i=0;i<Cells;++i){config.plan.cells[i]={0x400000+i*8,s.module+startdisk::ScriptExecutor,0x500000+i*0x100};cells[i]=config.plan.cells[i].original;}
        for(unsigned i=0;i<CounselorCells;++i)config.original.counselor.values[i]=s.module+startdisk::ScriptExecutor;
        config.original.entry.values.fill(s.module+startdisk::ScriptExecutor);actual=s;
    }
    bool Resident(const callbackslots::Plan&)noexcept{return resident;}
    bool Current(const callbackslots::Plan& p,unsigned installed)noexcept{
        if(!current)return false;for(unsigned i=0;i<Cells;++i)if(cells[i]!=(i<installed?p.cells[i].replacement:p.cells[i].original))return false;return true;
    }
    sandboxstart::WriteResult Exchange(std::uintptr_t at,std::uintptr_t old,std::uintptr_t value)noexcept{
        for(unsigned i=0;i<Cells;++i)if(config.plan.cells[i].address==at){if(!exchange||cells[i]!=old)return sandboxstart::WriteResult::Refused;
            cells[i]=value;order[writes++]=i;return sandboxstart::WriteResult::Changed;}return sandboxstart::WriteResult::Refused;
    }
    bool Guard(const Call& call,const Expected& expected,const pickerprepare::Receiver& receiver,bool,Invocation& out)noexcept{
        ++guards;for(unsigned i=0;i<Cells;++i)if(cells[i]!=(i<CounselorCells?expected.counselor.values[i]:expected.entry.values[i-CounselorCells]))expectedOkay=false;
        if(!guard||!expectedOkay)return false;out={};out.kind=static_cast<Kind>(call.frame);out.actual=actual;
        if(out.kind==Kind::Transition||out.kind==Kind::EntryTransition)out.parameter={Address,parameter};
        if(call.cell<CounselorCells&&out.kind!=Kind::Construct){
            if(call.widget!=receiver.widget||!(receiver.identity==actual.identities[2])||!(receiver.owningController==actual.identities[1]))return false;
        }return true;
    }
    bool AdmitConstruct(const Call& call,const Expected&,const active::Context& c,pickerprepare::Receiver& out)noexcept{
        ++getters;if(!admit)return false;out={call.widget,config.counselor,{call.widget,8,18},c.identities[1]};return true;
    }
    struct Cast {
        Host& h;bool OwnedConstruct()noexcept{return true;}bool Current(bool widened)noexcept{
            return h.castWidened==widened&&!(h.restoredCheckFails&&!widened&&h.restores);}
        bool NeedsWidening()noexcept{return true;}
        counselorpicker::CastResult Widen()noexcept{h.castWidened=true;++h.widens;return counselorpicker::CastResult::Changed;}
        counselorpicker::CastResult Restore()noexcept{h.castWidened=false;++h.restores;return counselorpicker::CastResult::Changed;}
    };
    template<class Forward> void Construct(const Call&,const Expected&,const pickerprepare::Receiver&,counselorpicker::ScopedConstruct& scope,Forward&& forward)noexcept{
        ++constructs;Cast cast{*this};scope.Run(cast,forward);
    }
    bool RouteCurrent(const active::Context& c,const Expected&,const pickerprepare::Receiver&)noexcept{++arrivals;++submitProofs;return guard&&c==actual;}
    bool ArrivalCurrent(const active::Context& c,const Expected&,const pickerprepare::Receiver&)noexcept{++arrivals;++arrivalProofs;return guard&&c==actual;}
    bool ReturnCurrent(const active::Context&,const active::Context& c)noexcept{return guard&&c==actual&&Current(config.plan,Cells);}
    bool RelabeledCurrent(const active::Context& c)noexcept{return returnedLabels&&c==actual;}
    bool EpochCurrent(const active::Context& c,const Expected&)noexcept{return guard&&c==actual&&Current(config.plan,Cells);}
    bool ExchangeParameter(std::uintptr_t at,std::uintptr_t old,std::uintptr_t value)noexcept{
        ++exchanges;if(!exchange||at!=Address||parameter!=old)return false;parameter=value;return true;
    }
    bool Invoke(unsigned cell,Kind kind,std::uintptr_t widget=0)noexcept{
        return coordinator.Invoke(*this,{cell,widget?widget:actual.widget,static_cast<std::uintptr_t>(kind),0});
    }
    bool Forward(std::uintptr_t original,const Call& call)noexcept{
        ++forwards;if(original!=config.source.module+startdisk::ScriptExecutor)return false;
        const auto kind=static_cast<Kind>(call.frame);
        if(chainEntry&&kind==Kind::EntryClick)Invoke(6,Kind::EntryGraph);
        if(chainEntry&&kind==Kind::EntryGraph){parameter=config.sandbox;Invoke(7,Kind::EntryTransition);}
        if(chainConstruct&&kind==Kind::Construct&&call.cell==1)Invoke(0,Kind::Construct,call.widget);
        if(chainAccept&&kind==Kind::Accept&&call.cell==2)Invoke(0,Kind::Accept);
        if(chainAccept&&kind==Kind::Accept&&call.cell==0){if(backDuringAccept)Invoke(4,Kind::Back);
            else{parameter=acceptParameter;Invoke(3,Kind::Transition);}}
        if(chainBack&&kind==Kind::Back){parameter=0;Invoke(3,Kind::Transition);}
        if(kind==Kind::EntryTransition||kind==Kind::Transition){seen=persistent=parameter;
            sawConsumed=coordinator.RouteState()==(kind==Kind::EntryTransition?counselorroute::State::AwaitCounselor:counselorroute::State::AwaitMap);
            if(foreign)parameter=0x880000;
        }return nativeOkay;
    }
    bool Publish(){if(!coordinator.Prepare(*this,config))return false;for(unsigned i=0;i<Cells;++i)if(!coordinator.Tick(*this,1000+i*100))return false;return true;}
    bool Entry(){chainEntry=true;const bool okay=Invoke(5,Kind::EntryClick);chainEntry=false;return okay;}
    bool ConstructPicker(){chainConstruct=true;const bool okay=Invoke(1,Kind::Construct,PickerWidget);chainConstruct=false;
        actual=config.source;actual.widget=PickerWidget;actual.type=config.counselor;actual.identities[2]={PickerWidget,8,18};return okay;
    }
    bool Ready(){return Publish()&&Entry()&&ConstructPicker()&&coordinator.Arrive(*this,actual);}
};
}
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    {Host old;check(old.Publish(),"surviving counselor prior ledger installed");Host fresh;
        for(unsigned i=0;i<CounselorCells;++i)fresh.cells[i]=fresh.config.plan.cells[i].replacement;
        for(unsigned i=CounselorCells;i<Cells;++i)fresh.config.plan.cells[i].address+=0x100000;
        fresh.config.source.type+=0x100000;fresh.actual=fresh.config.source;
        check(fresh.coordinator.PrepareInherited(fresh,fresh.config,old.coordinator)&&fresh.coordinator.Installed()==5&&fresh.writes==0,
            "new OfflinePlay class adopts exact surviving five cells without republishing them");
        for(unsigned i=0;i<3;++i)check(fresh.coordinator.Tick(fresh,1000+i*25),"25ms callback publishes one new entry cell");
        check(fresh.coordinator.Installed()==Cells&&fresh.writes==3&&fresh.order[0]==5&&fresh.order[2]==7,
            "mixed epoch finishes eight cells with only three new writes");}
    for(unsigned fault=0;fault<4;++fault){Host old;check(old.Publish(),"inherited negative prior fixture installed");Host fresh;
        for(unsigned i=0;i<CounselorCells;++i)fresh.cells[i]=fresh.config.plan.cells[i].replacement;
        if(fault==0)fresh.config.plan.cells[0].address+=8;
        if(fault==1)fresh.config.plan.cells[0].replacement+=8;
        if(fault==2)fresh.cells[2]=0x990000;
        if(fault==3)fresh.current=false;
        check(!fresh.coordinator.PrepareInherited(fresh,fresh.config,old.coordinator)&&!fresh.writes,
            "foreign address/executor/current identity refuses without overwrite");}
    {Host h;check(h.Publish(),"unready entry test publishes exact route");h.entryReady=false;
        const auto state=h.coordinator.RouteState();
        check(h.Entry()&&h.forwards==0&&h.exchanges==0&&h.coordinator.RouteState()==state,
            "unready or refused view consumes counselor click without stock launch or route transition");
        check(h.Invoke(6,Kind::Other)&&h.forwards==1,"unrelated native entry graph still forwards while view unready");
        h.entryReady=true;check(h.Entry()&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,
            "completed view admits next fresh click normally");}
    {Host h;h.restoredCheckFails=true;
        check(h.Publish()&&h.Entry()&&h.ConstructPicker(),"restored-final-check regression starts with owned constructor");
        check(h.coordinator.ConstructState()==counselorpicker::ConstructState::RestoredAwaitValidation&&
            h.coordinator.RouteState()==counselorroute::State::AwaitCounselor&&h.widens==1&&h.restores==1&&!h.castWidened,
            "confirmed restore waits for guarded input instead of retiring native menu");
        h.guard=false;h.chainAccept=true;const auto exchanges=h.exchanges;
        check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor&&h.exchanges==exchanges,
            "restored constructor grants no Accept authority while guard fails");
        h.guard=true;
        check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitMap&&h.persistent==h.config.map,
            "fully independent accepted input advances native route after restored final-check failure");
        check(h.widens==1&&h.restores==1&&h.getters==1,"selection never reruns constructor mutation or owner getter");h.chainAccept=false;
        h.actual.widget=0x710000;h.actual.type=h.config.map;h.actual.identities[2]={h.actual.widget,9,19};
        check(h.coordinator.Arrive(h,h.actual),"restored flow still needs real map arrival proof");
        h.actual.widget=0x720000;h.actual.type=h.config.settings;h.actual.identities[2]={h.actual.widget,10,20};
        check(h.coordinator.Arrive(h,h.actual)&&h.coordinator.Submit(h,h.actual),"restored flow completes same guarded settings and Start admission");
        check(h.arrivalProofs==2&&h.submitProofs==1,"map and settings each use one arrival proof; Start uses separate current proof");}
    {Host h;check(h.Publish()&&h.Entry()&&h.ConstructPicker(),"passive-HUD rejection fixture has proved native Construct and owner");
        h.guard=false;const auto writes=h.writes,exchanges=h.exchanges;
        check(!h.coordinator.ObserveCounselorHud(h,h.actual)&&h.coordinator.CounselorHudState()==CounselorHudArrival::AwaitGuardedInput&&
            h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,"failed passive observation preserves waiting, grants no arrival authority");
        const auto attempts=h.arrivals;h.guard=true;
        for(unsigned i=0;i<30;++i)check(!h.coordinator.ObserveCounselorHud(h,h.actual),"passive metadata is not repeatedly scanned");
        check(h.arrivals==attempts&&h.writes==writes&&h.exchanges==exchanges,"waiting for guarded input does no metadata reattempt or mutation");
        h.guard=false;h.chainAccept=true;check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor&&h.exchanges==exchanges,
            "failed actual Accept proof cannot reuse passive observation");
        h.guard=true;check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitMap&&h.persistent==h.config.map,
            "fresh fully guarded Accept independently advances after transient HUD rejection");h.chainAccept=false;
        h.actual.widget=0x710000;h.actual.type=h.config.map;h.actual.identities[2]={h.actual.widget,9,19};
        check(h.coordinator.Arrive(h,h.actual),"map still needs its independent current proof");
        h.actual.widget=0x720000;h.actual.type=h.config.settings;h.actual.identities[2]={h.actual.widget,10,20};
        check(h.coordinator.Arrive(h,h.actual)&&h.coordinator.Submit(h,h.actual),"settings and Start admission survive passive HUD rejection without bypass");}
    {Host h;check(h.Publish()&&h.Entry()&&h.ConstructPicker()&&h.coordinator.ObserveCounselorHud(h,h.actual)&&
        h.coordinator.CounselorHudState()==CounselorHudArrival::Verified,"proved passive counselor arrival retains normal ready flow");
        ++h.actual.identities[2].serial;h.chainAccept=true;const auto exchanges=h.exchanges;
        check(h.Invoke(2,Kind::Accept)&&h.exchanges==exchanges&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,
            "passive readiness never exempts recycled receiver from Accept guard");}
    {unsigned forwards=0,ownerStateReads=0;const cfgmenu::pickercoordinator::Call call{1,0x20000,0x30000,0x40000};
        const auto forward=[&](std::uintptr_t original,const cfgmenu::pickercoordinator::Call& actual)noexcept{
            ++forwards;return original==0x50000&&actual.cell==call.cell&&actual.widget==call.widget&&actual.frame==call.frame&&actual.result==call.result;};
        if(!cfgmenu::pickercoordinator::ForwardForeign(43,42,0x50000,call,forward))++ownerStateReads;
        check(forwards==1&&!ownerStateReads,"foreign callback forwards before mutable coordinator/report access");
        if(!cfgmenu::pickercoordinator::ForwardForeign(42,42,0x50000,call,forward))++ownerStateReads;
        check(forwards==1&&ownerStateReads==1,"owner callback alone enters mutable runtime");
    }
    {Host h;check(h.coordinator.Prepare(h,h.config),"prepare exact eight-cell plan");
        check(h.coordinator.Tick(h,1000)&&h.writes==1,"one cell per existing HUD phase");
        check(!h.coordinator.Tick(h,1024)&&h.writes==1,"25ms phase bound");
        check(!h.coordinator.Tick(h,999),"clock rollback does not publish");
        for(unsigned i=1;i<Cells;++i)check(h.coordinator.Tick(h,1000+i*100),"remaining one-cell publication");
        for(unsigned i=0;i<Cells;++i)check(h.order[i]==i,"all five counselor cells before any entry cell");
        check(h.coordinator.PublicationState()==callbackslots::State::Installed,"installed after eight exact exchanges");
        const auto ex=h.coordinator.ExpectedExecutors();check(ex.counselor.values[5]==0&&ex.counselor.values[6]==0&&ex.entry.values[2]==h.cells[7],"typed expected arrays never broaden absent handlers");
        check(h.coordinator.Restore(h),"explicit conditional restore");for(unsigned i=0;i<Cells;++i)check(h.coordinator.Tick(h,2000+i*100),"one reverse restore cell");
        for(unsigned i=0;i<Cells;++i)check(h.order[8+i]==7-i,"entry disabled before counselor during reverse restore");}
    {Host h;check(h.Ready(),"entry wrapper graph transition then first owned counselor construct");
        check(h.persistent==h.config.counselor&&h.parameter==h.config.sandbox,"entry persistent copy survives borrowed input restore");
        check(h.getters==1&&h.constructs==2&&h.widens==1&&h.restores==1&&!h.castWidened,"one owner admission and one outer scoped cast across wrapper graph recursion");
        check(h.expectedOkay&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,"actual arrival and exact published expectations");
        const auto before=h.forwards;h.chainAccept=true;check(h.Invoke(2,Kind::Accept),"actual Accept forwarded");h.chainAccept=false;
        check(h.forwards==before+3&&h.persistent==h.config.map&&h.parameter==0,"Accept wrapper graph and transition each forward once; original persists native map");
        check(h.sawConsumed&&h.coordinator.RouteState()==counselorroute::State::AwaitMap,"route token consumed before original sees replacement");
        h.parameter=0;const auto exchanges=h.exchanges;check(h.Invoke(3,Kind::Transition)&&h.exchanges==exchanges&&h.seen==0,"replayed null transition unchanged");
        h.actual.widget=0x710000;h.actual.type=h.config.map;h.actual.identities[2]={h.actual.widget,9,19};check(h.coordinator.Arrive(h,h.actual),"native offline map arrival");
        h.actual.widget=0x720000;h.actual.type=h.config.settings;h.actual.identities[2]={h.actual.widget,10,20};check(h.coordinator.Arrive(h,h.actual)&&h.coordinator.Submit(h,h.actual),"native settings and submit policy");}
    {Host h;check(h.Ready(),"Back fixture ready");h.chainBack=true;const auto n=h.exchanges;check(h.Invoke(4,Kind::Back)&&h.exchanges==n&&h.seen==0,"Back cancels before original null transition");
        check(h.coordinator.RouteState()==counselorroute::State::EntryReady,"Back returns route admission to entry");}
    {Host h;check(h.Ready(),"Back/reopen native sequence ready");h.chainBack=true;check(h.Invoke(4,Kind::Back),"owned Back forwards native pop");h.chainBack=false;
        check(h.coordinator.ReturnState()==entryreturn::State::AwaitEntry,"Back waits for actual native return");
        auto returned=h.config.source;returned.widget=0x750000;returned.identities[2]={returned.widget,13,23};returned.array.data=0x760000;
        h.actual=returned;const auto writes=h.writes,exchanges=h.exchanges;entryreturn::Ticket ticket;
        check(h.coordinator.ObserveReturn(h,returned,ticket),"new positive native entry instance observed with owned callbacks");
        check(h.Entry()&&h.exchanges==exchanges,"entry cannot redirect before fresh relabel completes");
        check(!h.coordinator.CompleteReturn(h,ticket),"unfinished labels cannot enable second entry");h.returnedLabels=true;
        check(h.coordinator.CompleteReturn(h,ticket)&&h.writes==writes,"return commits without any hook republication");
        h.config.source=returned;check(h.Entry()&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,"second actual native click redirects once");
        check(h.ConstructPicker()&&h.coordinator.Arrive(h,h.actual)&&h.getters==2,"second counselor instance is freshly admitted");
        h.chainAccept=true;check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitMap,"second visit preserves real Accept continuation");
        check(h.writes==writes&&!h.coordinator.CompleteReturn(h,ticket),"old return ticket inert and all eight callbacks unchanged");}
    {Host h;check(h.Ready(),"settings Back loop fixture");h.chainAccept=true;check(h.Invoke(2,Kind::Accept),"first counselor accepted");h.chainAccept=false;
        const auto picker=h.actual;
        auto map=h.config.source;map.widget=0x710000;map.type=h.config.map;map.identities[2]={map.widget,9,19};
        auto setup=map;setup.widget=0x720000;setup.type=h.config.settings;setup.identities[2]={setup.widget,10,20};
        h.actual=map;check(h.coordinator.Arrive(h,map),"map arrives first");h.actual=setup;check(h.coordinator.Arrive(h,setup),"settings arrive first");
        for(unsigned i=0;i<4;++i){
            h.actual=map;check(h.coordinator.Arrive(h,map),"Back to map preserves counselor route");
            setup.widget+=0x100;setup.identities[2]={setup.widget,static_cast<int>(20+i),static_cast<int>(30+i)};
            h.actual=setup;check(h.coordinator.Arrive(h,setup),"new settings after changing map or Jason remains counselor");
        }
        // Returning native Construct may run while the map is still top.
        h.actual=map;check(h.ConstructPicker()&&h.coordinator.Arrive(h,h.actual),"Back admits real counselor Construct before active top changes");
        h.chainAccept=true;check(h.Invoke(2,Kind::Accept),"newly constructed revisit accepts");h.chainAccept=false;
        h.actual=map;check(h.coordinator.Arrive(h,map),"map after constructed revisit");h.actual=setup;check(h.coordinator.Arrive(h,setup),"settings after constructed revisit");
        h.actual=picker;h.chainAccept=true;
        check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitMap,"cached counselor Accept without Construct redirects normally");h.chainAccept=false;
        h.actual=map;check(h.coordinator.Arrive(h,map),"map after changed counselor");h.actual=setup;check(h.coordinator.Arrive(h,setup),"settings after changed counselor");
        h.actual=h.config.source;check(h.coordinator.ReturnFromSelection(h.actual),"map/settings can Back directly to Offline Play");
        entryreturn::Ticket ticket;check(h.coordinator.ObserveReturn(h,h.actual,ticket),"entry return still needs native source proof");h.returnedLabels=true;
        check(h.coordinator.CompleteReturn(h,ticket),"entry relabel completes without new hooks");
        check(!h.coordinator.Submit(h,setup),"old settings cannot submit after leaving counselor entry");}
    {Host h;check(h.Publish()&&h.Entry(),"outgoing native entry animation fixture");
        h.actual=h.config.source;
        check(!h.coordinator.ReturnFromSelection(h.actual)&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor&&
            h.coordinator.ReturnState()==entryreturn::State::Ready,
            "old entry still top during native transition cannot be mistaken for a returned customization menu");
        check(h.ConstructPicker()&&h.coordinator.Arrive(h,h.actual),"pending native counselor arrival survives passive old-entry callbacks");}
    for(unsigned pending=0;pending<2;++pending){Host h;
        check(pending?(h.Publish()&&h.Entry()&&h.ConstructPicker()):h.Ready(),"customization return fixture ready or awaiting passive arrival");
        const auto writes=h.writes,exchanges=h.exchanges,getters=h.getters;
        auto returned=h.config.source;returned.widget=0x750000;returned.identities[2]={returned.widget,13,23};returned.array.data=0x760000;
        h.actual=returned;
        check(h.coordinator.ReturnFromSelection(returned)&&h.coordinator.RouteState()==counselorroute::State::EntryReady&&
            h.coordinator.ReturnState()==entryreturn::State::AwaitEntry,"actual Offline Play pop cancels stale customization route without old Back callback");
        entryreturn::Ticket ticket;h.guard=false;
        check(!h.coordinator.ObserveReturn(h,returned,ticket),"return observation still refuses failed native source/callback proof");h.guard=true;
        check(h.coordinator.ObserveReturn(h,returned,ticket),"returned instance needs independent full source and executor proof");
        check(!h.coordinator.CompleteReturn(h,ticket),"old ready labels cannot complete fresh instance return");h.returnedLabels=true;
        check(h.coordinator.CompleteReturn(h,ticket)&&h.writes==writes&&h.exchanges==exchanges&&h.getters==getters,
            "fresh labels commit without hook republication or stale owner getter");
        h.config.source=returned;
        check(h.Entry()&&h.ConstructPicker()&&h.coordinator.Arrive(h,h.actual)&&h.getters==getters+1,
            "next counselor selection freshly admits owner after customization return");
        h.chainAccept=true;check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::AwaitMap,
            "post-customization entry preserves normal real Accept map continuation");}
    for(unsigned fault=0;fault<7;++fault){Host h;check(h.Ready(),"invalid customization return fixture ready");
        auto returned=h.config.source;
        if(fault==0)returned.identities[2].serial=0;
        if(fault==1)++returned.identities[1].serial;
        if(fault==2)returned.stack+=0x100;
        if(fault==3)returned.type=h.config.counselor;
        if(fault==4)++returned.array.count;
        if(fault==5)returned.engine+=0x100;
        if(fault==6)returned.world+=0x100;
        const auto route=h.coordinator.RouteState();const auto returning=h.coordinator.ReturnState();
        check(!h.coordinator.ReturnFromSelection(returned)&&h.coordinator.RouteState()==route&&h.coordinator.ReturnState()==returning,
            "invalid top/serial/infrastructure/depth refuses without half-changing either ledger");}
    for(unsigned terminal=0;terminal<2;++terminal){Host h;check(h.Ready(),"terminal route return fixture ready");
        if(terminal){h.coordinator.Retire();}else{
            h.chainAccept=true;check(h.Invoke(2,Kind::Accept),"terminal return fixture accepts native picker");h.chainAccept=false;
            h.actual.widget=0x710000;h.actual.type=h.config.map;h.actual.identities[2]={h.actual.widget,9,19};
            check(h.coordinator.Arrive(h,h.actual),"terminal return fixture reaches map");
            h.actual.widget=0x720000;h.actual.type=h.config.settings;h.actual.identities[2]={h.actual.widget,10,20};
            check(h.coordinator.Arrive(h,h.actual)&&h.coordinator.Submit(h,h.actual),"terminal return fixture submits actual settings");
        }
        const auto before=h.coordinator.RouteState();h.actual=h.config.source;
        check(!h.coordinator.ReturnFromSelection(h.actual)&&h.coordinator.RouteState()==before,
            "customization fallback cannot revive a retired route or cancel submitted gameplay");}
    {Host h;check(h.Ready(),"cosmetics fixture ready");h.parameter=0x900000;const auto n=h.exchanges;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n&&h.seen==0x900000,"non-null cosmetics untouched without Accept");
        h.chainAccept=true;h.acceptParameter=0x900000;
        check(h.Invoke(2,Kind::Accept)&&h.exchanges==n&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,"non-null cosmetics untouched within Accept; no stale accepted lease");
        h.exchange=false;h.acceptParameter=0;
        check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::CounselorReady&&h.seen==0,"failed exchange does not consume; no-exit Accept clears its lease");
        h.exchange=true;check(h.Invoke(2,Kind::Accept)&&h.seen==h.config.map,"fresh synchronous Accept succeeds only after real exchange");}
    {Host h;check(h.Publish(),"out-of-scope entry fixture installed");h.parameter=h.config.sandbox;
        check(h.Invoke(7,Kind::EntryTransition)&&!h.exchanges&&h.seen==h.config.sandbox,"entry transition outside actual click/graph scope untouched");
        check(h.Invoke(5,Kind::EntryClick)&&h.coordinator.RouteState()==counselorroute::State::EntryReady,"entry click without transition leaves no stale request");}
    {Host h;check(h.Publish(),"direct graph DB fixture installed");h.chainEntry=true;
        check(h.Invoke(6,Kind::EntryGraph)&&h.persistent==h.config.counselor&&h.parameter==h.config.sandbox,"independently proved actual entry graph DB may establish request");
        check(h.sawConsumed&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,"direct graph request consumed before nested transition original");}
    {Host h;check(h.Ready(),"repeat Construct fixture ready");const auto getters=h.getters;
        check(h.ConstructPicker()&&h.getters==getters&&h.widens==2&&h.restores==2,"independent positively owned Construct uses fresh scope without second owner admission");
        check(h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,"Accept without inline transition clears request and retains counselor receiver");
        h.chainBack=true;h.chainAccept=true;h.backDuringAccept=true;const auto n=h.exchanges;
        check(h.Invoke(2,Kind::Accept)&&h.exchanges==n&&h.seen==0,"nested Back revokes accepted lease before null transition");}
    {Host h;check(h.Ready(),"no Accept fixture ready");const auto n=h.exchanges;h.parameter=0;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n&&h.seen==0,"owned counselor null is not sufficient without accepted nonce");}
    {Host h;check(h.coordinator.Prepare(h,h.config)&&h.coordinator.Tick(h,1000),"partial publication fixture");const auto n=h.forwards;
        check(h.Invoke(0,Kind::Construct,Host::PickerWidget)&&h.forwards==n+1&&!h.guards&&!h.getters,"partly published callback forwards unmodified without admission");}
    {Host h;check(h.Ready(),"changed controller fixture ready");h.Invoke(2,Kind::Accept);++h.actual.identities[1].serial;h.parameter=0;const auto n=h.exchanges;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n,"controller slot recycling revokes borrowed transition authority");}
    {Host h;check(h.Ready(),"changed infrastructure fixture ready");h.Invoke(2,Kind::Accept);h.actual.stack+=0x100;h.parameter=0;const auto n=h.exchanges;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n,"route policy independently rejects different actual stack");}
    {Host h;check(h.Publish(),"entry wrong old class fixture installed");h.parameter=0;
        check(h.Invoke(7,Kind::EntryTransition)&&!h.exchanges&&h.seen==0,"entry null never matches authored SandboxMap literal");}
    {Host h;check(h.Ready(),"foreign mutation fixture ready");h.chainAccept=true;h.foreign=true;
        check(h.Invoke(2,Kind::Accept)&&h.persistent==h.config.map&&h.parameter==0x880000,"foreign input mutation is not rollback authority");
        check(h.coordinator.RouteState()==counselorroute::State::Retired,"restore conflict retires route");}
    {Host h;check(h.Ready(),"stale Accept fixture ready");check(h.Invoke(2,Kind::Accept),"no-exit original Accept returns");
        h.parameter=0;const auto n=h.exchanges;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n&&h.seen==0&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,"later unrelated null event cannot reuse previous Accept");}
    {Host h;check(h.Publish()&&h.Entry(),"failed owner fixture entry");h.admit=false;const auto n=h.forwards;
        check(h.ConstructPicker()&&h.getters==1&&h.forwards==n+2&&!h.widens,"failed owner getter forwards original nested callbacks without mutation");
        h.ConstructPicker();check(h.getters==1&&h.coordinator.RouteState()==counselorroute::State::Retired,"failed owner admission is never automatically retried");}
    {Host h;check(h.Ready(),"recycled receiver fixture ready");h.Invoke(2,Kind::Accept);++h.actual.identities[2].serial;h.parameter=0;const auto n=h.exchanges;
        check(h.Invoke(3,Kind::Transition)&&h.exchanges==n&&h.seen==0,"changed positive receiver serial fails guard and forwards unchanged");}
    {Host h;check(h.Ready(),"guard rejection fixture ready");h.guard=false;const auto n=h.forwards;
        check(h.Invoke(2,Kind::Accept)&&h.forwards==n+1&&h.coordinator.RouteState()==counselorroute::State::CounselorReady,"failed guard forwards once without granting Accept lease");}
    {Host h;check(h.Publish(),"incompatible role fixture installed");h.parameter=0;const auto n=h.forwards;
        check(h.Invoke(5,Kind::Transition)&&h.forwards==n+1&&!h.exchanges,"a thunk cannot claim another slot's role");}
    {Host h;check(h.Ready(),"native failure fixture ready");h.nativeOkay=false;check(!h.Invoke(2,Kind::Accept)&&h.coordinator.RouteState()==counselorroute::State::Retired,"protected original failure retires no auto retry");}
    {Host h;check(h.coordinator.Prepare(h,h.config)&&h.coordinator.Tick(h,1000),"rollback fixture one published cell");h.exchange=false;
        check(!h.coordinator.Tick(h,1100)&&h.coordinator.PublicationState()==callbackslots::State::Restoring,"refused publication schedules conditional reverse restore");h.exchange=true;
        check(h.coordinator.Tick(h,1200)&&h.coordinator.PublicationState()==callbackslots::State::Refused,"only owned cell restored after refusal");}
    {Host h;check(h.coordinator.Prepare(h,h.config)&&h.coordinator.Tick(h,1000),"foreign publication fixture");h.cells[0]=0x990000;
        check(!h.coordinator.Tick(h,1100)&&h.coordinator.PublicationState()==callbackslots::State::Indeterminate&&h.cells[0]==0x990000,"foreign executor never overwritten on lost authority");}
    {Host h;check(h.Ready(),"epoch fixture ready");const auto writes=h.writes;
     h.actual=h.config.source;h.actual.world+=0x100000;h.actual.identities[0].object=h.actual.world;++h.actual.identities[0].serial;
     h.actual.widget+=0x100000;h.actual.identities[2].object=h.actual.widget;++h.actual.identities[2].serial;
     h.guard=false;check(!h.coordinator.Rearm(h,h.actual),"unproved epoch cannot reset admission");h.guard=true;
     check(h.coordinator.Rearm(h,h.actual)&&h.writes==writes&&h.coordinator.Installed()==Cells&&
       h.coordinator.RouteState()==counselorroute::State::EntryReady&&!h.coordinator.Receiver().widget,"new epoch retains eight cells, no old receiver");
     h.chainEntry=true;check(h.Invoke(5,Kind::EntryClick)&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,"second epoch native counselor route");
     h.cells[2]=0x990000;check(!h.coordinator.Rearm(h,h.actual)&&h.cells[2]==0x990000,"foreign cell blocks epoch without overwrite");}
    {Host h;check(h.Publish(),"same-world menu instance installed");const auto writes=h.writes;
     for(unsigned reopen=0;reopen<10;++reopen){
      h.actual=h.config.source;h.actual.widget+=0x100000*(reopen+1);h.actual.identities[2].object=h.actual.widget;
      h.actual.identities[2].serial+=static_cast<int>(reopen+1);
      check(h.coordinator.Rearm(h,h.actual)&&h.coordinator.RouteState()==counselorroute::State::EntryReady&&
       h.writes==writes&&h.coordinator.Installed()==Cells,"Back/Main/Offline new instance keeps installed cells");
     }
     auto wrong=h.actual;wrong.type+=0x1000;check(!h.coordinator.Rearm(h,wrong),"foreign menu class refuses re-entry");
     h.guard=false;check(!h.coordinator.Rearm(h,h.actual),"missing full proof refuses re-entry");}
    {Host h;check(h.Ready(),"ten consecutive match/menu epochs begin ready");const auto writes=h.writes;
     for(unsigned cycle=1;cycle<=10;++cycle){
      h.actual=h.config.source;h.actual.world+=static_cast<std::uintptr_t>(cycle)*0x100000;
      h.actual.identities[0].object=h.actual.world;h.actual.identities[0].serial+=static_cast<int>(cycle);
      h.actual.widget+=static_cast<std::uintptr_t>(cycle)*0x100000;h.actual.identities[2].object=h.actual.widget;
      h.actual.identities[2].serial+=static_cast<int>(cycle);
      h.guard=false;check(!h.coordinator.Rearm(h,h.actual),"Nth rearm refuses before current-world proof");h.guard=true;
      check(h.coordinator.Rearm(h,h.actual)&&h.coordinator.RouteState()==counselorroute::State::EntryReady&&
       h.writes==writes&&h.coordinator.Installed()==Cells&&!h.coordinator.Receiver().widget,"Nth epoch retains exact eight slots, clears receiver and rearms route");
      h.chainEntry=true;check(h.Invoke(5,Kind::EntryClick)&&h.coordinator.RouteState()==counselorroute::State::AwaitCounselor,"Nth fresh native entry remains callable");
     }}
    std::printf("Native picker coordinator owned model: %s (%u checks, %u failures; no game access).\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
