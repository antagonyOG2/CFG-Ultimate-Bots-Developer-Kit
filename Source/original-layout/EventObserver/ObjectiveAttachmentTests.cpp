#define main ObjectiveAttachmentFixtureMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "ObjectiveAttachmentObservation.hpp"
struct AttachmentFixture:MenuFixture {
    static constexpr std::uintptr_t Registry=0xA00000,Panel=0xA10000,PanelType=0xA20000,
        Slot=0xA30000,SlotType=0xA40000,WidgetType=0xA70000;
    cfgprobe::lifetime::ArrayHeader registry{Registry,3};
    AttachmentFixture(){using namespace cfgobjectivepanel::disk;
        r.Set(Type+cfgprobe::ClassSuper,WidgetType);r.Set(WidgetType+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(PanelType+cfgprobe::ClassSuper,WidgetType);r.Set(SlotType+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(Widget+cfgprobe::ObjectClass,Type);r.Set(Panel+cfgprobe::ObjectClass,PanelType);r.Set(Slot+cfgprobe::ObjectClass,SlotType);
        r.Set(Widget+WidgetSlot,Slot);r.Set(Slot+SlotContent,Widget);r.Set(Slot+SlotParent,Panel);
        const std::uintptr_t objects[]{Widget,Slot,Panel};
        for(unsigned i=0;i<3;++i){r.Set(objects[i]+12,static_cast<int>(i));r.Set(Registry+i*24,objects[i]);r.Set(Registry+i*24+16,static_cast<int>(i+1));}
    }
    bool Read(cfgobjectiveattachment::Edge& e){return cfgobjectiveattachment::Parent(r,registry,Widget,WidgetType,SlotType,PanelType,e);}
};
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    for(unsigned fault=0;fault<7;++fault){AttachmentFixture f;cfgobjectiveattachment::Edge e;
        switch(fault){
        case 1:f.r.Set(f.Slot+cfgobjectivepanel::disk::SlotContent,f.Panel);break;
        case 2:f.r.Set(f.Slot+cfgobjectivepanel::disk::SlotParent,f.Widget);break;
        case 3:f.r.Set(f.Registry+24+16,0);break;
        case 4:f.r.Set(f.Slot+cfgprobe::ObjectClass,f.WidgetType);break;
        case 5:f.r.Set(f.Widget+cfgobjectivepanel::disk::WidgetSlot,std::uintptr_t{});break;
        case 6:f.r.changeAt=f.Slot+cfgobjectivepanel::disk::SlotParent;f.r.changeSlot=f.Widget+cfgobjectivepanel::disk::WidgetSlot;f.r.newPointer=f.Panel;break;
        }
        check(f.Read(e)==(fault==0));
        if(!fault)check(e.widget.object==f.Widget&&e.slot.object==f.Slot&&e.parent.object==f.Panel);
        else check(!e.widget.object&&!e.slot.object&&!e.parent.object);
    }
    std::printf("Objective attachment diagnostic cross-links/serial/type/mutation guards: %s (%u failures). No native calls/writes.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
