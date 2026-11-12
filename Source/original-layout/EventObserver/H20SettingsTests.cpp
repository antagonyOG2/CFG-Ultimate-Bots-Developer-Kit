#define CFG_J216_LABEL_MAIN PriorLabelTests
#include "J216MenuLabelTests.cpp"
#include "HostedSettingsSpecs.hpp"
#include "../Core/HostedFillPolicy.hpp"
struct FillFixture {
    struct Ticket {bool requested=false;std::uint8_t before=0;};
    bool host=true,requested=true,current=true;std::uint8_t value=0;unsigned writes=0,forwards=0;
    bool Capture(Ticket& t){t={requested,value};return host&&current;}
    bool Exchange(const Ticket&,std::uint8_t before,std::uint8_t after){if(!host||!current||value!=before)return false;value=after;++writes;return true;}
};
int main(){
    unsigned failures=0,checks=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
    using namespace cfgmenu;
    cfghostedoptions::Store store;check(!store.values.Mask()&&!store.values.difficulty&&!store.revision);
    auto specs=hostedsettings::Specs(store.values);LabelHost host;rows::Runtime runtime;
    check(runtime.Begin(19,specs.data(),static_cast<unsigned>(specs.size())));
    for(unsigned step=0;step<80&&runtime.CurrentState()!=rows::State::Ready;++step)runtime.Step(host,step*25);
    check(runtime.CurrentState()==rows::State::Ready);
    for(unsigned row=0;row<specs.size();++row){const auto widget=runtime.At(row).widget;check(specs[row].preserveNativeValue&&specs[row].Valid());
        auto& cell=host.At(widget);check(cell.bound&&cell.shown==specs[row].labels[specs[row].selected]);
        for(unsigned choice=0;choice<specs[row].count;++choice){
            cell.selected=choice;cell.shown=cell.labels[choice];check(cell.shown==specs[row].labels[choice]);
            check(hostedsettings::Save(runtime,host,19,store));
            const std::array<unsigned,4> actual{{store.values.jason,store.values.difficulty,store.values.fill,store.values.jasonCount-1}};check(actual[row]==choice);
        }
    }
    const auto kept=store.values;host.current=false;check(!hostedsettings::Save(runtime,host,19,store)&&store.values==kept);
    check(!store.Save({2,0,0})&&store.values==kept);host.current=true;
    auto reopened=hostedsettings::Specs(store.values);check(reopened[0].selected==1&&reopened[1].selected==3&&reopened[2].selected==1&&reopened[3].selected==1);
    cfghostedoptions::Match match;check(!match.Capture(0x10000,false,store)&&!match.world);
    check(match.Capture(0x10000,true,store)&&match.values==kept&&match.values.Difficulty()==2);
    check(store.Save({})&&match.Capture(0x10000,true,store)&&match.values==kept);
    check(match.Capture(0x20000,true,store)&&!match.values.Mask()&&match.values.Difficulty()==1);
    for(unsigned fault=0;fault<6;++fault){FillFixture f;cfghostedfill::Report r;
        if(fault==1)f.host=false;if(fault==2)f.requested=false;if(fault==3)f.current=false;if(fault==4)f.value=1;
        cfghostedfill::Around(f,[&](){++f.forwards;if(fault==5)f.value=2;},r);
        check(f.forwards==1&&r.entries==1);
        if(fault==0)check(f.value==0&&f.writes==2&&r.applied==1&&r.restored==1);
        else if(fault==4)check(f.value==1&&!f.writes&&r.applied==1&&!r.restored);
        else if(fault==5)check(f.value==2&&f.writes==1&&r.restoreRefused==1);
        else check(!f.writes&&!r.applied&&r.refused==1);
    }
    std::printf("H20 native rows labels/memory-only choices/Back persistence/host immutable snapshot/native fill own-value original-once: %u checks %u failures.\n",checks,failures);
    return failures?1:0;
}
