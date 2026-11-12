#pragma once
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "../RuntimeProbe/MenuStackParent.generated.hpp"
#include "../Core/J21cFeatureFlags.hpp"
#include "../Core/J22FeatureFlags.hpp"
namespace cfgstatussurface {
inline bool StackHeader(const cfgmenu::active::StackArray& value){
    if(value.Sane())return true;
    // Keep active-menu admission strict. Only this presentation resolver can
    // prove an empty stack through the exact current native HUD-owned widget.
    return (CFG_J3_END_SUMMARY||CFG_J22_END_SUMMARY)&&value.count==0&&value.capacity>=0&&value.capacity<=64&&
        (value.capacity?cfgprobe::ObjectPointer(value.data)&&cfgprobe::lifetime::Extent(value.data,
            static_cast<std::uintptr_t>(value.capacity)*cfgmenu::execdisk::RecordBytes):!value.data);
}
struct Binding {
    cfgprobe::lifetime::SlotObservation hud{},stack{},root{},display{};
    std::uintptr_t rootClass=0;cfgreflection::Property property{};
    bool hudRoot=false;
};
template<class H>bool Slot(H& h,const cfgprobe::lifetime::SlotObservation& expected){
    cfgprobe::lifetime::SlotObservation fresh{};return expected.HasAssignedSerial()&&h.Slot(expected.object,fresh)&&fresh==expected;
}
template<class R,class H>bool Current(R& r,H& h,std::uintptr_t module,std::uintptr_t controller,const Binding& b,bool display=true){
    using namespace cfgmenu::execdisk;cfgmenu::active::StackArray records{};
    cfgreflection::Observation<R> reflection(r,module);
    if(!h.Scope()||!Slot(h,b.hud)||!Slot(h,b.stack)||!Slot(h,b.root)||
       !cfgobserver::Same(r,controller+ControllerHUD,b.hud.object)||!cfgobserver::Same(r,b.hud.object+HUDStack,b.stack.object)||
       !cfgobserver::Same(r,b.hud.object+0x368,controller)||!cfgobserver::Same(r,b.stack.object+0x20,b.hud.object)||
       !cfgobserver::Same(r,b.root.object+cfgprobe::ObjectClass,b.rootClass)||
       !r.Read(b.stack.object+StackArray,records)||!StackHeader(records))return false;
    if(b.hudRoot){
        // The native post-match view can leave the stack EMPTY while its
        // CounselorHUDWidget remains strongly owned by SCInGameHUD+HUDWidget.
        // This is the SAME observed HUD, never a global/cached foreign widget.
        cfgnames::Observation<R> names(r,module);cfgnames::Text type;
        if(!(CFG_J3_END_SUMMARY||CFG_J22_END_SUMMARY)||records.count||
           !cfgobserver::Same(r,b.hud.object+cfgsweater::huddisk::HUDWidget,b.root.object)||
           !names.Object(b.rootClass,type)||type.View()!=u"CounselorHUDWidget_C")return false;
    }else if(records.count<1||!cfgobserver::Same(r,b.root.object+0x20,b.stack.object)||
        !cfgobserver::Same(r,records.data+static_cast<std::uintptr_t>(records.count-1)*RecordBytes+RecordWidget,b.root.object))return false;
    return !display||(Slot(h,b.display)&&reflection.CurrentProperty(b.property)&&
       cfgspawn::ClassContains(r,b.rootClass,b.property.field.owner)&&
       b.property.field.kind.View()==u"ObjectProperty"&&b.property.dimension==1&&b.property.elementBytes==8&&
       b.property.offset>=0&&b.property.offset<=0x4000&&
       cfgobserver::Same(r,b.root.object+static_cast<unsigned>(b.property.offset),b.display.object));
}
struct Runtime {
    Binding binding{};std::uintptr_t owner=0,field=0,listType=0;unsigned depth=0,count=0;
    std::array<std::uintptr_t,512> seen{};bool ready=false;
    unsigned lastFailure=0; // diagnostic only: last refusal point of Step
    bool Fail(unsigned stage){lastFailure=stage;return false;}
    void Reset(){const auto failure=lastFailure;*this={};lastFailure=failure;}
    template<class R,class H>bool Step(R& r,H& h,std::uintptr_t module,std::uintptr_t controller){
        using namespace cfgmenu::execdisk;
        if(ready){if(Current(r,h,module,controller,binding))return true;Reset();}
        if(!binding.root.object||!Current(r,h,module,controller,binding,false)){
            Reset();std::uintptr_t hud=0,stack=0,root=0,hudType=0,stackType=0;cfgmenu::active::StackArray records{};
            // PID1692 end screen: Fail(1) x16. Same predicates, same order; each
            // refusal now names its check (21-33) for the next capture.
            if(!h.Scope())return Fail(21);
            if(!h.NativeClass(cfgsweater::huddisk::SCInGameHUDReflection,cfgsweater::huddisk::SCInGameHUDConstructor,hudType))return Fail(22);
            if(!h.NativeClass(cfgmenu::parentdisk::Owner,cfgmenu::parentdisk::Constructor,stackType))return Fail(23);
            if(!h.NativeClass(cfgsummary::disk::ListClass,cfgsummary::disk::ListConstructor,listType))return Fail(24);
            if(!r.Read(controller+ControllerHUD,hud))return Fail(25);
            if(!cfgspawn::ObjectIs(r,hud,hudType))return Fail(26);
            if(!h.Slot(hud,binding.hud))return Fail(27);
            if(!r.Read(hud+HUDStack,stack)||!cfgspawn::ObjectIs(r,stack,stackType))return Fail(28);
            if(!h.Slot(stack,binding.stack))return Fail(29);
            if(!r.Read(stack+StackArray,records)||!StackHeader(records))return Fail(30);
            if(records.count<1){
                if(!(CFG_J3_END_SUMMARY||CFG_J22_END_SUMMARY)||!r.Read(hud+cfgsweater::huddisk::HUDWidget,root)||!root)return Fail(30);
                binding.hudRoot=true;
            }else if(!r.Read(records.data+static_cast<std::uintptr_t>(records.count-1)*RecordBytes+RecordWidget,root))return Fail(31);
            if(!h.Slot(root,binding.root))return Fail(32);
            if(!r.Read(root+cfgprobe::ObjectClass,binding.rootClass))return Fail(33);
            owner=binding.rootClass;if(!r.Read(owner+cfgreflection::disk::Children,field))return Fail(2);
        }
        if(!Current(r,h,module,controller,binding,false))return Fail(3);
        cfgreflection::Observation<R> reflection(r,module);cfgnames::Observation<R> names(r,module);
        // Only the current owned stack root's reflected fields,16 per turn.
        // No UObject census, guessed property offset or global widget adoption.
        for(unsigned n=0;n<16;++n){
            if(r.reads+384>=r.cap)return Fail(4); // retain the cursor before a budget deferral
            if(!field){
                if(binding.display.object){ready=true;return Current(r,h,module,controller,binding);}
                std::uintptr_t next=0;if(!owner||depth>=8)return Fail(5);
                if(!r.Read(owner+cfgprobe::ClassSuper,next))return Fail(6);
                if(!next){ready=binding.display.object!=0;if(!ready)return Fail(7);return Current(r,h,module,controller,binding)||Fail(8);}
                owner=next;++depth;if(!r.Read(owner+cfgreflection::disk::Children,field))return Fail(9);continue;}
            if(count>=seen.size())return Fail(10);for(unsigned i=0;i<count;++i)if(seen[i]==field)return Fail(11);
            cfgreflection::Field f;if(!reflection.ReadField(field,owner,f))return Fail(12);
            seen[count++]=field;field=f.next;
            if(f.kind.View()!=u"ObjectProperty")continue;
            cfgreflection::Property p;std::uintptr_t object=0,type=0;cfgprobe::lifetime::SlotObservation slot{};cfgnames::Text name;
            if(!reflection.ReadProperty(f,p)||p.dimension!=1||p.elementBytes!=8||p.offset<0||p.offset>0x4000)return Fail(13);
            if(!r.Read(binding.root.object+static_cast<unsigned>(p.offset),object))return Fail(14);
            if(!object||!cfgspawn::ObjectIs(r,object,listType))continue;
            if(!h.Slot(object,slot)||!slot.HasAssignedSerial()||!r.Read(object+cfgprobe::ObjectClass,type)||
               !names.Object(type,name)||name.View()!=u"ScoreboardDisplayWidget_C"||!reflection.CurrentProperty(p))return Fail(15);
            if(binding.display.object&&binding.display.object!=object)return Fail(16);
            binding.display=slot;binding.property=p;
        }
        return Fail(17); // 16-field turn used up; discovery continues next callback
    }
};
}
