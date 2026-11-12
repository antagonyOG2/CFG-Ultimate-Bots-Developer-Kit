#pragma once
#include "../Core/CounselorPracticePolicy.hpp"
#include "../RuntimeProbe/CounselorPracticeBindings.generated.hpp"
#include "NativeSoundBlipSuppression.hpp"
#include "SweaterHudObservation.hpp"
namespace cfgpracticeobs {
namespace disk=::cfgpractice::disk;
template<class R>bool Clear(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){
    std::uint32_t flags=0,objectFlags=0;
    return cfgprobe::lifetime::ReadSlot(r,array,object,slot)&&r.Read(array.data+24ULL*static_cast<unsigned>(slot.index)+8,flags)&&
        !(flags&0x30000000u)&&r.Read(object+8,objectFlags)&&!(objectFlags&0x30u);
}
template<class R>bool Typed(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t module,std::uintptr_t object,std::uintptr_t cache){
    std::array<std::uintptr_t,2> pair{};cfgprobe::lifetime::SlotObservation slot;
    return Clear(r,array,object,slot)&&r.Read(module+cache,pair)&&cfgprobe::MatchingClassCache(pair[0],pair[1])&&cfgspawn::ObjectIs(r,object,pair[0]);
}
// Observation only. Current positive offline killer/controller/human links are
// repeated by the existing emitter guard. No UObject allocation or native call.
template<class R>bool Voice(R& r,const cfgsoundblip::Scope& scope,std::uintptr_t component,const cfgnames::Words& line,cfgnames::Text& text){
    cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation slot;std::uintptr_t seen=0;
    cfgnames::Observation<R> names(r,scope.module);
    return cfgprimary::LookupVetoClearForObservation(r,scope.module)&&
        cfgsoundblip::Suppress(r,scope,scope.killer.object,true)&&
        cfgprobe::lifetime::ReadArrayHeader(r,scope.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
        r.Read(scope.killer.object+disk::VoiceComponent,seen)&&seen==component&&
        Typed(r,array,scope.module,component,disk::SCVoiceOverComponent_KillerReflection)&&
        r.Read(component+0xE0,seen)&&seen==scope.killer.object&&r.Read(component,seen)&&seen==scope.module+disk::VoiceVtable&&
        names.Decode(line,text)&&r.Read(scope.killer.object+disk::VoiceComponent,seen)&&seen==component&&
        cfgsoundblip::Suppress(r,scope,scope.killer.object,true)&&cfgprimary::LookupVetoClearForObservation(r,scope.module);
}
struct WidgetNode {
    cfgprobe::lifetime::SlotObservation widget{};
    std::uintptr_t tree=0,slot=0;std::uint32_t parent=UINT32_MAX,index=0,kind=0;
    struct List {std::uintptr_t data=0;int count=0,capacity=0;
        bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
        bool Sane()const{return count>=0&&count<=128&&capacity>=count&&capacity<=256&&(!count||cfgprobe::ObjectPointer(data));}
    } list{};
};
constexpr unsigned NodeCapacity=192;
struct WidgetGraph {
    std::array<WidgetNode,NodeCapacity> nodes{};unsigned count=0,cursor=0;
    cfgprobe::lifetime::SlotObservation hudWidget{};std::uintptr_t world=0;
};
template<class R>cfgpractice::HudNodeDiagnostic Diagnostic(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t object,unsigned node,unsigned reason){
    cfgpractice::HudNodeDiagnostic out;out.object=object;out.node=node;out.reason=reason;
    cfgprobe::lifetime::SlotObservation slot;std::uintptr_t type=0;std::uint8_t visibility=255;
    if(!Clear(r,array,object,slot)||!Typed(r,array,module,object,disk::WidgetReflection))return out;
    if(r.Read(object+disk::Visibility,visibility)&&visibility<=4)out.visibility=visibility;
    cfgnames::Observation<R> names(r,module);cfgnames::Text text;
    const auto copy=[](auto& target,std::u16string_view source){
        const auto count=(std::min)(target.size()-1,source.size());
        for(std::size_t i=0;i<count;++i)target[i]=source[i]>0&&source[i]<128?static_cast<char>(source[i]):'?';
    };
    if(names.Object(object,text))copy(out.name,text.View());
    if(r.Read(object+cfgprobe::ObjectClass,type)&&Clear(r,array,type,slot)&&names.Object(type,text))copy(out.type,text.View());
    return out;
}
template<class R>bool ExactHealthWidget(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t widget){
 cfgprobe::lifetime::SlotObservation slot;std::uintptr_t type=0,again=0;cfgnames::Text name;
 cfgnames::Observation<R> names(r,module);
 return Clear(r,array,widget,slot)&&Typed(r,array,module,widget,disk::WidgetReflection)&&
  r.Read(widget+cfgprobe::ObjectClass,type)&&Clear(r,array,type,slot)&&names.Object(type,name)&&
  name.View()==u"SCHealth_Widget_C"&&r.Read(widget+cfgprobe::ObjectClass,again)&&again==type&&Clear(r,array,widget,slot);
}
// Reprove a complete current strong widget-tree path, including all slot
// headers/cells and nested UserWidget roots. A saved pointer is not authority.
template<class R>bool NodeCurrent(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgsweater::HudEvidence& hud,const WidgetGraph& graph,unsigned index){
    if(index>=graph.count||graph.count>NodeCapacity||hud.widget.object!=graph.hudWidget.object)return false;
    std::array<unsigned,NodeCapacity> seen{};unsigned depth=0;
    for(;;){if(index>=graph.count||depth==seen.size())return false;
        for(unsigned i=0;i<depth;++i)if(seen[i]==index)return false;seen[depth++]=index;
        const auto& node=graph.nodes[index];cfgprobe::lifetime::SlotObservation fresh;std::uintptr_t value=0;
        if(!Clear(r,array,node.widget.object,fresh)||fresh.object!=node.widget.object||fresh.index!=node.widget.index||
           (node.widget.serial>0&&fresh.serial!=node.widget.serial)||!Typed(r,array,module,node.widget.object,disk::WidgetReflection))return false;
        if(node.parent==UINT32_MAX){
            return node.kind==0&&r.Read(hud.widget.object+disk::WidgetTree,value)&&value==node.tree&&
                Typed(r,array,module,node.tree,disk::WidgetTreeReflection)&&r.Read(node.tree+disk::RootWidget,value)&&value==node.widget.object;
        }
        if(node.parent>=index)return false;const auto parent=graph.nodes[node.parent].widget.object;
        if(node.kind==1){if(!Typed(r,array,module,parent,disk::UserWidgetReflection)||
            !r.Read(parent+disk::WidgetTree,value)||value!=node.tree||!Typed(r,array,module,node.tree,disk::WidgetTreeReflection)||
            !r.Read(node.tree+disk::RootWidget,value)||value!=node.widget.object)return false;
        }else if(node.kind==2){WidgetNode::List list;
            if(!Typed(r,array,module,parent,disk::PanelWidgetReflection)||!r.Read(parent+disk::Slots,list)||!(list==node.list)||
                !list.Sane()||node.index>=static_cast<unsigned>(list.count)||!r.Read(list.data+8ULL*node.index,value)||value!=node.slot||
                !Typed(r,array,module,node.slot,disk::PanelSlotReflection)||!r.Read(node.slot+disk::Content,value)||value!=node.widget.object)return false;
        }else return false;
        index=node.parent;
    }
}
enum class MinimalTarget {None,Health,Stamina,Minimap};
template<class R>MinimalTarget CurrentMinimalTarget(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgsweater::HudEvidence& hud,const WidgetGraph& graph,std::uintptr_t widget,unsigned node,bool minimap){
    if(!Typed(r,array,module,widget,disk::WidgetReflection))return MinimalTarget::None;
    if(minimap){std::uintptr_t current=0;
        return r.Read(hud.widget.object+disk::Minimap,current)&&current==widget?MinimalTarget::Minimap:MinimalTarget::None;
    }
    if(node>=graph.count||graph.nodes[node].widget.object!=widget||!NodeCurrent(r,module,array,hud,graph,node))return MinimalTarget::None;
    if(ExactHealthWidget(r,module,array,widget))return MinimalTarget::Health;
    return Typed(r,array,module,widget,disk::SCStaminaWidgetReflection)?MinimalTarget::Stamina:MinimalTarget::None;
}
}
