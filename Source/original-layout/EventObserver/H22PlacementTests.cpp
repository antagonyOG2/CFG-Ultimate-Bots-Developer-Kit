#define CFG_J216_LABEL_MAIN ExistingLabelChecks
#include "J216MenuLabelTests.cpp"
#include "HostedLobbyPlacement.hpp"
struct LobbyRows:LabelHost {
    bool lease=true;unsigned next=7;std::array<unsigned,3> positions{};
    bool ProvedHostLobbyLease(const cfghostedlobby::RowLayout& layout){return lease&&current&&layout.world==0x10000&&layout.menu==0x20000&&layout.rowClass==0x30000&&layout.panel==0x40000&&layout.anchor==0x50000&&layout.generation==19;}
    bool AppendAfter(const cfghostedlobby::RowLayout& layout,unsigned index,std::uintptr_t& row,bool& attached){
        if(!ProvedHostLobbyLease(layout)||index!=next||next>=10)return false;positions[next-7]=index;++next;return Append(row,attached);
    }
};
int main(){unsigned failures=0,checks=0;const auto check=[&](bool ok,const char* why){++checks;if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    cfghostedlobby::RowLayout layout{0x10000,0x20000,0x30000,0x40000,0x50000,7,19};
    cfghostedlobby::Placement placement;LobbyRows host;cfghostedoptions::Store store;
    host.lease=false;check(!placement.Service(host,layout,0,store)&&!host.appended,"no host-lobby lease means no native UI work");host.lease=true;
    for(unsigned step=0;step<20;++step)(void)placement.Service(host,layout,step*25,store);
    check(placement.Placed()==3&&host.appended==3&&host.positions==std::array<unsigned,3>{{7,8,9}},"three proven compatible host rows insert immediately after anchor");
    check(store.values.jason==0&&store.values.fill==0&&store.values.difficulty==2,"default disabled bot/fill, Normal difficulty");
    for(unsigned row=0;row<3;++row)for(unsigned choice=0;choice<(row==1?3u:2u);++choice){
        auto& cell=host.cells[row];cell.selected=choice;cell.shown=cell.labels[choice];
        check(placement.Service(host,layout,1000,store),"native choice persisted");
        const unsigned value=row==0?store.values.jason:row==1?store.values.difficulty-1:store.values.fill;
        check(value==choice&&cell.bound&&cell.shown==cell.labels[choice],"preserveNativeValue retains live arrow text binding");
    }
    const auto kept=store.values;host.lease=false;host.cells[0].selected^=1;check(!placement.Service(host,layout,2000,store)&&store.values==kept,"lost host authority cannot read/store or mutate old rows");
    cfghostedlobby::RowHooks hooks;cfghostedlobby::HookHost actual{hooks,0};check(!actual.ProvedHostLobbyLease(layout),"production bridge with no dedicated provider refuses; generic/offline scope insufficient");
    std::printf("H22 flagged native-row lifecycle/anchor placement/native label binding/host lease refusal: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
