#define main RestrictedLoadoutRegressionMain
#include "RestrictedLoadoutTests.cpp"
#undef main
#include "ConvergenceAxeRuntime.hpp"
struct FakeAxeAccess {
    unsigned lastStage=0,observes=0,calls=0,verifies=0;bool ready=true,armed=false,granted=true,verified=true;
    cfgconvergence::AxeRuntime* reentry=nullptr;
    bool Observe(cfgloadout::expanded::Evidence&,bool){++observes;return ready;}
    bool AlreadyAxe(const cfgloadout::expanded::Evidence&){return armed;}
    bool Grant(const cfgloadout::expanded::Evidence&){++calls;if(reentry)reentry->Service(*this);return granted;}
    bool Verify(const cfgloadout::expanded::Evidence&){++verifies;return verified;}
};
struct AxeHost:Host {
    cfgofflinesettings::Request owner{};unsigned selected=99;std::uintptr_t hint=0;
    bool paused=false;bool Pause(unsigned)const{return paused;}
    std::uintptr_t LoadedAxeHint(){return hint;}
    void Select(unsigned index){selected=index;}
    bool Owner(cfgofflinesettings::Request& out){out=owner;return selected==0;}
};
// Charge the shared callback for outer world/Jason/held guards (192), deeper
// native ancestry than this fixture (128), and each complete helper ownership
// recheck (256: retained Context + living pawn/controller/slots). This is an
// intentionally conservative shallow-CFG cost model, not a live measurement.
struct AxeBudgetReader {
    Memory& memory;unsigned reads=320,peak=0;
    bool Charge(unsigned count){if(count>1536-reads)return false;reads+=count;return true;}
    void Phase(){peak=(std::max)(peak,reads);reads=320;}
    template<class T> bool Read(std::uintptr_t at,T& value){return Charge(1)&&memory.Read(at,value);}
};
struct AxeBudgetHost {
    AxeBudgetReader& reader;unsigned calls=0;
    bool Current(const cfgofflinesettings::Request&){return reader.Charge(256);}
    bool InvokeGrant(std::uintptr_t,std::uintptr_t,std::uintptr_t){++calls;return true;}
};
int main(){unsigned failures=0,checks=0;const auto check=[&](bool okay,const char* label){++checks;if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    {Memory memory;AxeHost host;host.paused=true;cfgconvergence::AxeRuntime run;run.stage=1;
     check(!run.Step(memory,host,0x140000000,0x100000,1,true,100)&&run.stage==1&&!run.refused&&host.selected==99,"emote hold defers instead of permanently skipping helper grant");}
    {cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;
        run.Service(access);run.Service(access);run.Service(access);run.stage=5;
        check(!run.RequestRearm(1,1000),"never adopt an unserved helper");
        for(unsigned loss=0;loss<4;++loss){const auto now=1000ull+loss*16000ull;
            check(run.RequestRearm(0,now),"verified native weapon loss can queue bounded replacement");run.stage=2;
            run.Service(access);run.Service(access);run.Service(access);
            check(run.stage==5&&run.grants==loss+2,"replacement preserves completed startup prefix");
            check(!run.RequestRearm(0,now+1),"replacement cooldown prevents repeated spawn");}
        check(!run.RequestRearm(0,1000000)&&access.calls==5,"four replacement cap after initial grant");}
    {cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;
        run.Service(access);run.Service(access);run.Service(access);run.stage=5;
        check(run.RequestRearm(0,1000),"verified helper replacement admitted");run.stage=2;access.granted=false;
        run.Service(access);run.Service(access);
        check(!run.RequestRearm(0,50000)&&run.stage==5&&access.calls==2,"ambiguous replacement permanently closes that helper");}
    {cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;access.reentry=&run;
        check(run.Service(access)&&run.stage==3&&!access.calls,"preparation is read-only");
        check(run.Service(access)&&run.stage==4&&access.calls==1&&!access.verifies,"native grant consumed before reentry");
        check(run.Service(access)&&run.stage==1&&run.grants==1&&access.verifies==1,"separate owned verification");
        check(!run.Service(access)&&access.calls==1,"finished helper never repeats");}
    {cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;access.armed=true;
        check(run.Service(access)&&run.alreadyArmed==1&&!access.calls,"existing held Axe preserved");}
    {cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;access.ready=false;
        for(unsigned i=0;i<10;++i)run.Service(access);
        check(access.observes==4&&run.refused==1&&!access.calls,"readiness retries finite and mutation-free");}
    for(unsigned fault=0;fault<2;++fault){cfgconvergence::AxeRuntime run;run.stage=2;FakeAxeAccess access;
        access.granted=fault!=0;access.verified=fault!=1;
        for(unsigned i=0;i<10;++i)run.Service(access);
        check(access.calls==1&&run.refused==1&&!run.grants,"ambiguous grant or ownership failure never replayed");}
    {Fixture f;AxeHost host;host.hint=f.Loaded;cfgconvergence::AxeRuntime run;
        check(run.Step(f.memory,host,f.Module,f.World,1,false,0)&&run.stage==1&&!run.scanned&&!host.calls,
            "known local Axe class hint avoids a global scan without granting");}
    {Fixture f;f.Name(0,"CounselorTwoHandedAxe_C");
        f.memory.Set(f.Item+cfgprobe::ObjectClass,std::uintptr_t{0x390000});f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
        f.memory.Set(0x390000+cfgprobe::ClassSuper,std::uintptr_t{});f.Slot(8,f.Item);
        f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        f.memory.Set(f.Pawn+cfgloadout::restricteddisk::EquippedItem,std::uintptr_t{});
        f.memory.Set(f.Defaults+cfgloadout::restricteddisk::IsLarge,std::uint8_t{1});
        AxeBudgetReader budget{f.memory};AxeBudgetHost budgetHost{budget};
        cfgloadout::expanded::NativeAccess<AxeBudgetReader,AxeBudgetHost> budgetAccess(
            budget,budgetHost,{f.request.owner,cfgloadout::Slot::Weapon,3},f.Module,f.Loaded);
        cfgloadout::expanded::Evidence evidence;
        check(budgetAccess.Observe(evidence,true),"shared 1536-read prepare budget");budget.Phase();
        check(budgetAccess.Grant(evidence)&&budgetHost.calls==1,"shared 1536-read native grant budget");budget.Phase();
        AxeHost host;host.owner=f.request.owner;cfgconvergence::AxeRuntime run;
        unsigned inactiveCalls=0;
        for(unsigned now=0;now<3000;now+=250){run.Step(f.memory,host,f.Module,f.World,1,false,now);inactiveCalls+=host.calls;}
        check(run.stage==1&&run.scanned&&inactiveCalls==0,"cold finite discovery cannot grant before sweater");
        check(!run.Step(f.memory,host,f.Module,f.World,0,true,3000,false)&&run.stage==1,"late helper roster is not prematurely completed");
        for(unsigned now=3250;now<=3750;now+=250)run.Step(f.memory,host,f.Module,f.World,1,true,now);
        check(run.stage==4&&host.calls==1&&run.attempts==1,"real native adapter helper grant phased");
        f.Added();f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        f.memory.Set(f.Small+cfgfeature::disk::InventoryItems,cfgspawn::PlayerArray{});
        f.memory.Set(f.Pawn+cfgloadout::restricteddisk::EquippedItem,f.Created);
        f.memory.Set(f.Created+cfgloadout::restricteddisk::IsLarge,std::uint8_t{1});
        check(budgetAccess.Verify(evidence),"shared 1536-read ownership verification budget");budget.Phase();
        std::printf("Helper Axe production-shape peak=%u/1536, native calls=%u/12\n",budget.peak,budgetHost.calls);
        run.Step(f.memory,host,f.Module,f.World,1,true,4000);
        check(!run.Step(f.memory,host,f.Module,f.World,1,true,4250),"newly armed helper gets a convergence turn");
        run.Step(f.memory,host,f.Module,f.World,1,true,4500);
        check(run.stage==5&&run.grants==1&&!run.refused&&host.calls==1,"native Axe exact ownership verified and terminal");
        run.Step(f.memory,host,f.Module,f.World,2,true,4750);
        check(host.selected==1&&host.calls==1&&run.refused==1,"additional roster suffix never regrants original helper");}
    std::printf("ConvergenceAxeTests: %u checks, %u failures. No game access.\n",checks,failures);return failures?1:0;
}
