// Real rows::Runtime + native-row host contract, including the authored
// ValueText binding which a direct SetText replaces. No game/native calls.
#define main ExistingRowsMain
#include "NativeSetupRowsTests.cpp"
#undef main
#include "NativeSettingsMenuOptions.hpp"
#include "NativeChallengeSettingsSpecs.hpp"
struct LabelHost:Host {
    struct Cell {unsigned selected=0;bool bound=true;std::array<std::u16string,32> labels{};std::u16string shown;};
    std::array<Cell,cfgmenu::rows::Capacity> cells{};std::u16string pending;
    Cell& At(std::uintptr_t widget){return cells[(widget-0x100000)/8];}
    bool BorrowNativeRow(std::uintptr_t& widget,bool& attached,const cfgmenu::rows::Spec&){return Append(widget,attached);}
    bool CopyOptions(std::uintptr_t widget,const cfgmenu::rows::Spec& spec){
        if(!Host::CopyOptions(widget,spec))return false;auto& c=At(widget);c.selected=spec.selected;
        for(unsigned i=0;i<spec.count;++i)c.labels[i]=spec.labels[i];c.shown=c.labels[c.selected];return true;
    }
    bool Text(std::u16string_view label,cfgmenu::reflection::RetainedMenuText::Request& out){pending=label;return Host::Text(label,out);}
    bool SetText(std::uintptr_t widget,std::u16string_view field,const cfgmenu::reflection::RetainedMenuText::Request& value){
        if(!Host::SetText(widget,field,value))return false;
        if(field==u"ValueText"){auto& c=At(widget);c.bound=false;c.shown=pending;}return true;
    }
    bool Index(std::uintptr_t widget,unsigned& value){value=At(widget).selected;return true;}
    void Input(std::uintptr_t widget,int delta){auto& c=At(widget);c.selected=static_cast<unsigned>((static_cast<int>(c.selected)+delta+2)%2);if(c.bound)c.shown=c.labels[c.selected];}
};
#ifndef CFG_J216_LABEL_MAIN
#define CFG_J216_LABEL_MAIN main
#endif
int CFG_J216_LABEL_MAIN(){
    unsigned failures=0,checks=0;const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    using namespace cfgmenu;
    const std::array<RowId,9> ids{{RowId::Skill,RowId::Count,RowId::Difficulty,RowId::Duration,RowId::Weapon,RowId::Inventory,RowId::PamelaMute,RowId::GrabMute,RowId::MinimalHud}};
    for(bool repaired:{false,true}){
        std::array<rows::Spec,9> specs{};for(unsigned i=0;i<ids.size();++i){specs[i]=settingsoptions::Practice(ids[i],u"Label fixture");if(!repaired)specs[i].preserveNativeValue=false;}
        LabelHost host;rows::Runtime runtime;check(runtime.Begin(19,specs.data(),static_cast<unsigned>(specs.size())));
        for(unsigned step=0;step<100&&runtime.CurrentState()!=rows::State::Ready;++step)runtime.Step(host,step*25);
        check(runtime.CurrentState()==rows::State::Ready);
        for(unsigned i=6;i<ids.size();++i){const auto widget=runtime.At(i).widget;
            check(host.At(widget).shown==u"Off");
            host.Input(widget,1);check(host.At(widget).selected==1);check(host.At(widget).shown==(repaired?u"On":u"Off")); // Change
            host.Input(widget,1);check(host.At(widget).selected==0&&host.At(widget).shown==u"Off"); // Right wraps
            host.Input(widget,-1);check(host.At(widget).selected==1&&host.At(widget).shown==(repaired?u"On":u"Off")); // Left wraps
        }
        std::array<unsigned,rows::Capacity> values{};check(runtime.Choices(host,19,values));
        for(unsigned i=6;i<ids.size();++i)check(values[i]==1); // unchanged value path in both cases
    }
    for(bool repaired:{false,true}){
        auto specs=challengemenu::Specs();if(!repaired)specs[challengemenu::choice::EdgeWalls].preserveNativeValue=false;
        LabelHost host;rows::Runtime runtime;check(runtime.Begin(19,specs.data(),static_cast<unsigned>(specs.size())));
        for(unsigned step=0;step<100&&runtime.CurrentState()!=rows::State::Ready;++step)runtime.Step(host,step*25);
        check(runtime.CurrentState()==rows::State::Ready);
        for(unsigned i=0;i<specs.size();++i){const auto widget=runtime.At(i).widget;check(runtime.At(i).spec.id==specs[i].id);
            host.Input(widget,1);check(host.At(widget).selected==1);
            check(host.At(widget).shown==specs[i].labels[(!repaired&&i==challengemenu::choice::EdgeWalls)?0:1]);
            host.Input(widget,1);check(host.At(widget).selected==0&&host.At(widget).shown==specs[i].labels[0]);
            host.Input(widget,-1);check(host.At(widget).selected==1);
            check(host.At(widget).shown==specs[i].labels[(!repaired&&i==challengemenu::choice::EdgeWalls)?0:1]);
        }
        std::array<unsigned,rows::Capacity> values{};check(runtime.Choices(host,19,values));
        for(unsigned i=0;i<specs.size();++i)check(values[i]==1);
    }
    std::printf("J216 real rows::Runtime both pages/every toggle displayed and stored after Change/left/right, legacy stale-label replay: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
