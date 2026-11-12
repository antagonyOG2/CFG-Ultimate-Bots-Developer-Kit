#pragma once
#include "HostedLobbyObservation.hpp"
#include <memory>
#include <new>
#include <vector>
#include <cstdio>
#include <string_view>
#include "OfflineDiagnosticTrace.hpp"
#include "../Core/J22FeatureFlags.hpp"
#ifndef CFG_H247_LOBBY_REARM
#define CFG_H247_LOBBY_REARM 1 // H2.49: re-enabled alone (inert during a hosted match); summary bots stay 0 until this run is clean
#endif
namespace cfghostedlobby {
// H2.47: the sampler keeps one Discovery per (world,menu,hud). Step() is stricter than that
// key (full Context::Same, per-node Path/Clear), so a graph built while the lobby was still
// constructing can fail Step forever at the same cursor, never reach the team row, and never
// re-Begin (second lobby: Nodes=6 Cursor=4 Failure=Changed, team Node=-1). Rows then never
// reach the lease scope. After Limit consecutive failed samples with no team row yet, ask the
// sampler to Begin again from fresh evidence; the unchanged first-lobby path does the rest.
struct StallGuard {
    static constexpr unsigned Limit=10;
    unsigned failures=0;bool rearm=false;
    void Progressed(){failures=0;}
    // H2.48: matchLive (hosted adoption/driver active) means no lobby world is current: never count or rearm.
    void StepFailed(bool complete,unsigned teamNode,bool matchLive=false){
        if(matchLive){failures=0;rearm=false;return;}
        if(!CFG_H247_LOBBY_REARM||complete||teamNode!=UINT32_MAX)return;
        if(++failures>=Limit){failures=0;rearm=true;}
    }
    bool TakeRearm(bool matchLive=false){if(matchLive){failures=0;rearm=false;return false;}const bool r=rearm;rearm=false;if(r)failures=0;return r;}
};
template<class R>struct LobbyTextReader {
    R& source;
    bool refused=false;
    template<class T>bool Read(std::uintptr_t address,T& value){
        if(source.Read(address,value))return true;
        refused=true;return false;
    }
};
struct Discovery {
    Context context{};cfgpracticeobs::WidgetGraph graph{};
    std::array<bool,2> bindingsDone{};bool measured=false; // H2.61 one-shot read-only layout report
    template<class R>bool Current(R& r){Context fresh;Failure why;
        if(!Observe(r,context.module,context.thread,fresh,why)||!Same(context,fresh))return false;
        context.registry=fresh.registry;return true;
    }
    template<class R>bool Path(R& r,unsigned index){
        cfgsweater::HudEvidence root;root.widget=context.identities[4];
        // This helper validates only a generic current UserWidget tree path.
        // It is NOT an in-game HUD/native-call admission or a retention scope.
        return cfgpracticeobs::NodeCurrent(r,context.module,context.registry,root,graph,index);
    }
    template<class R>bool Class(R& r,std::uintptr_t cache,std::uintptr_t& out){
        std::array<std::uintptr_t,2> pair{};
        return r.Read(context.module+cache,pair)&&(out=cfgprobe::MatchingClassCache(pair[0],pair[1]))!=0;
    }
    template<class R>bool Ancestry(R& r,std::uintptr_t object,cfgprobe::Ancestry& types){
        return cfgprobe::CaptureAncestry(r,object,types)&&cfgprobe::StableAncestry(r,object,types);
    }
    template<class R>void Name(R& r,std::uintptr_t object,std::array<char,40>& name,std::array<char,56>& typeName,std::uintptr_t type){
        cfgnames::Observation<R> names(r,context.module);cfgnames::Text text;
        if(names.Object(object,text))Copy(name,text.View());
        if(names.Object(type,text))Copy(typeName,text.View());
    }
    template<class R>bool Bindings(R& r,unsigned at,Row& row){
        std::uintptr_t userWidget=0;
        if(at>=graph.count||!Class(r,cfgpractice::disk::UserWidgetReflection,userWidget))return false;
        unsigned parent=at;
        while(parent<graph.count){
            cfgprobe::Ancestry types;
            if(!Ancestry(r,graph.nodes[parent].widget.object,types))return false;
            if(types.Contains(userWidget))break;
            parent=graph.nodes[parent].parent;
        }
        if(parent>=graph.count)return true;
        row.node=parent;row.parent=graph.nodes[parent].parent;row.childIndex=graph.nodes[parent].index;
        const auto object=graph.nodes[parent].widget.object;
        std::uintptr_t type=0,field=0;cfgreflection::Observation<R> reflection(r,context.module);
        if(!Path(r,parent)||!r.Read(object+cfgprobe::ObjectClass,type)||!r.Read(type+cfgreflection::disk::Children,field))return false;
        std::array<std::uintptr_t,64> seen{};unsigned count=0;
        while(field&&count<seen.size()){
            for(unsigned i=0;i<count;++i)if(seen[i]==field)return false;
            seen[count++]=field;cfgreflection::Field f;
            if(!reflection.ReadField(field,type,f))return false;field=f.next;
            if(f.kind.View()!=u"ObjectProperty")continue;
            cfgreflection::Property p;std::uintptr_t value=0;Slot slot;
            if(!reflection.ReadProperty(f,p))return false;
            if(p.dimension!=1||p.elementBytes!=8||p.offset<0||p.offset>0x4000)continue;
            if(!r.Read(object+static_cast<unsigned>(p.offset),value))return false;
            if(!value)continue;
            if(!cfgpracticeobs::Clear(r,context.registry,value,slot)||!reflection.CurrentProperty(p)||
               !cfgobserver::Same(r,object+static_cast<unsigned>(p.offset),value)||!Path(r,parent))return false;
            if(row.bindings==row.members.size()){row.truncated=1;break;}
            auto& member=row.members[row.bindings++];member.object=value;member.property=f.object;member.offset=static_cast<unsigned>(p.offset);Copy(member.name,f.label.View());
        }
        if(field)row.truncated=1;
        return true;
    }
    // H2.61 CFG_H261_CHAT_MEASURE: strictly read-only. A separate bounded walk (the existing 128-node walk and row placement are untouched)
    // lists every node whose class/name contains "Chat", the first LobbyOptionComboBoxWidget_C and the CustomServerSettings box with parent
    // node, the slot object's class name and the slot class's reflected property names/offsets. No slot-layout or geometry offset is proven in
    // this repo, so NO value is read: the reflection path for a later value read is the slot class Children chain printed below.
    std::uintptr_t chatSlot=0,chatWidget=0,chatPanel=0; // H2.69: read-only hints, re-proved before any call
    template<class R>void Measure(R& r){
#if CFG_H261_CHAT_MEASURE
        struct Item {std::uintptr_t object=0,slot=0;unsigned parent=UINT32_MAX;};
        std::uintptr_t panelWidget=0,userWidget=0;
        if(!graph.count||!Class(r,cfgpractice::disk::PanelWidgetReflection,panelWidget)||!Class(r,cfgpractice::disk::UserWidgetReflection,userWidget))return;
        std::vector<Item> items;items.reserve(256);items.push_back({graph.nodes[0].widget.object,0,UINT32_MAX});
        constexpr unsigned WalkLimit=1024;unsigned matches=0;bool combo=false,truncated=false;std::vector<unsigned> chain;
        // H2.62: LayoutData (FAnchorData, 40 bytes) ends at bAutoSize, reflected at 96, so it starts at 0x38. Read-only.
        const auto LayoutLine=[](R& rr,std::size_t node,std::uintptr_t slot,const char* type){if(!slot||std::string_view(type)!="CanvasPanelSlot")return;
            std::array<float,10> v{};char out[170];if(!rr.Read(slot+0x38,v))return;
            std::snprintf(out,sizeof(out),"H262ChatLayout n=%u L=%.1f T=%.1f R=%.1f B=%.1f amin=%.2f,%.2f amax=%.2f,%.2f align=%.2f,%.2f",static_cast<unsigned>(node),v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9]);
            cfgclosedtrace::Mark(out,slot,node);};
        std::array<std::uintptr_t,3> slotTypes{};unsigned slotTypeCount=0;char line[170];
        const auto narrow=[](std::u16string_view text,char* out,std::size_t size){std::size_t n=0;
            for(;n+1<size&&n<text.size();++n)out[n]=text[n]>0&&text[n]<128?static_cast<char>(text[n]):'?';out[n]=0;};
        for(std::size_t at=0;at<items.size();++at){
            if(at>=WalkLimit){truncated=true;break;}
            const Item item=items[at];cfgprobe::lifetime::SlotObservation widget;cfgprobe::Ancestry types;
            if(!cfgpracticeobs::Clear(r,context.registry,item.object,widget)||!Ancestry(r,item.object,types))continue;
            std::array<char,40> name{};std::array<char,56> cls{};Name(r,item.object,name,cls,types.classes[0]);
            std::array<char,40> slotName{};std::array<char,56> slotCls{};std::uintptr_t slotType=0;
            if(item.slot&&r.Read(item.slot+cfgprobe::ObjectClass,slotType))Name(r,item.slot,slotName,slotCls,slotType);
            const std::string_view n(name.data()),c(cls.data());
            const bool chat=n.find("Chat")!=n.npos||c.find("Chat")!=c.npos,isCombo=!combo&&c=="LobbyOptionComboBoxWidget_C",
                isSettings=n.find("CustomServerSettings")!=n.npos;
            if((chat||isCombo||isSettings)&&matches<40){
                ++matches;if(isCombo)combo=true;
                std::snprintf(line,sizeof(line),"H261ChatMeasure n=%u par=%u cls=%.32s nm=%.30s slot=%.30s",static_cast<unsigned>(at),item.parent,cls.data(),name.data(),slotCls.data());
                cfgclosedtrace::Mark(line,item.object,item.slot);
                chain.push_back(static_cast<unsigned>(at));
                if(c.rfind("LobbyChatNotificationOverlayWidg",0)==0&&std::string_view(slotCls.data())=="CanvasPanelSlot"&&item.parent<items.size()&&!chatSlot){
                    chatSlot=item.slot;chatWidget=item.object;chatPanel=items[item.parent].object;}
                LayoutLine(r,at,item.slot,slotCls.data());
                if(slotType&&slotTypeCount<slotTypes.size()){bool seen=false;for(unsigned k=0;k<slotTypeCount;++k)seen|=slotTypes[k]==slotType;if(!seen)slotTypes[slotTypeCount++]=slotType;}
            }
            if(types.Contains(panelWidget)){
                cfgpracticeobs::WidgetNode::List list;
                if(!r.Read(item.object+cfgpractice::disk::Slots,list)||!list.Sane())continue;
                if(std::string_view(name.data()).find("CustomServerSettings")!=std::string_view::npos){std::snprintf(line,sizeof(line),"H262ChatRows n=%u children=%u",static_cast<unsigned>(at),static_cast<unsigned>(list.count));cfgclosedtrace::Mark(line,item.object,list.count);}
                for(unsigned i=0;i<static_cast<unsigned>(list.count)&&items.size()<WalkLimit+128;++i){
                    std::uintptr_t slot=0,object=0;
                    if(!r.Read(list.data+8ULL*i,slot)||!cfgpracticeobs::Typed(r,context.registry,context.module,slot,cfgpractice::disk::PanelSlotReflection)||
                       !r.Read(slot+cfgpractice::disk::Content,object)||!object)continue;
                    items.push_back({object,slot,static_cast<unsigned>(at)});
                }
            }else if(types.Contains(userWidget)){
                std::uintptr_t tree=0,object=0;
                if(r.Read(item.object+cfgpractice::disk::WidgetTree,tree)&&tree&&
                   cfgpracticeobs::Typed(r,context.registry,context.module,tree,cfgpractice::disk::WidgetTreeReflection)&&
                   r.Read(tree+cfgpractice::disk::RootWidget,object)&&object)items.push_back({object,0,static_cast<unsigned>(at)});
            }
        }
        std::snprintf(line,sizeof(line),"H261ChatMeasure walked=%u limit=%u truncated=%u matches=%u geometry=none-proven slotlayout=none-proven",
            static_cast<unsigned>(items.size()),WalkLimit,truncated?1u:0u,matches);
        cfgclosedtrace::Mark(line,context.menu,items.size());
        std::vector<char> printed(items.size(),0);unsigned parents=0; // H2.62: which panel owns each chat piece
        for(unsigned start:chain)for(unsigned at=items[start].parent,steps=0;at<items.size()&&steps<10&&parents<48&&!printed[at];at=items[at].parent,++steps){
            printed[at]=1;++parents;std::uintptr_t type=0,slotType=0;if(!r.Read(items[at].object+cfgprobe::ObjectClass,type))break;
            std::array<char,40> name{},slotName{};std::array<char,56> cls{},slotCls{};Name(r,items[at].object,name,cls,type);
            if(items[at].slot&&r.Read(items[at].slot+cfgprobe::ObjectClass,slotType))Name(r,items[at].slot,slotName,slotCls,slotType);
            std::snprintf(line,sizeof(line),"H262ChatParent n=%u par=%u cls=%.32s nm=%.30s slot=%.30s",at,items[at].parent,cls.data(),name.data(),slotCls.data());
            cfgclosedtrace::Mark(line,items[at].object,items[at].slot);LayoutLine(r,at,items[at].slot,slotCls.data());
        }
        cfgreflection::Observation<R> reflection(r,context.module);
        for(unsigned k=0;k<slotTypeCount;++k){
            std::array<char,40> unusedName{};std::array<char,56> typeName{};Name(r,slotTypes[k],unusedName,typeName,slotTypes[k]);
            std::uintptr_t field=0;if(!r.Read(slotTypes[k]+cfgreflection::disk::Children,field))continue;
            std::array<std::uintptr_t,32> seen{};unsigned count=0;
            while(field&&count<seen.size()){
                bool again=false;for(unsigned i=0;i<count;++i)again|=seen[i]==field;if(again)break;seen[count++]=field;const std::uintptr_t current=field;
                cfgreflection::Field f;if(!reflection.ReadField(field,slotTypes[k],f))break;field=f.next;
                char label[32]{},kind[24]{};narrow(f.label.View(),label,sizeof(label));narrow(f.kind.View(),kind,sizeof(kind));
                int offset=-1,bytes=-1;cfgreflection::Property p;
                if(f.kind.View().find(u"Property")!=std::u16string_view::npos&&reflection.ReadProperty(f,p)){offset=p.offset;bytes=p.elementBytes;}
                std::snprintf(line,sizeof(line),"H261ChatMeasure slotprop cls=%.32s prop=%s kind=%s off=%d bytes=%d",typeName.data(),label,kind,offset,bytes);
                cfgclosedtrace::Mark(line,slotTypes[k],count);
                if(std::string_view(label)=="SetPosition"||std::string_view(label)=="GetPosition"){ // H2.62: live executor and code bytes, to pin before any call
                    std::uintptr_t exec=0;std::array<unsigned char,24> code{};char hex[49]{};
                    if(r.Read(current+0xB0,exec)&&exec>context.module&&exec-context.module<cfgprobe::ExecutableImageBytes&&r.Read(exec,code)){
                        for(unsigned b=0;b<code.size();++b)std::snprintf(hex+2*b,3,"%02X",code[b]);
                        std::snprintf(line,sizeof(line),"H262ChatFunc %s rva=%llX code=%s",label,static_cast<unsigned long long>(exec-context.module),hex);
                        cfgclosedtrace::Mark(line,current,exec-context.module);}
                }
            }
        }
#else
        (void)r;
#endif
    }
    template<class R>bool Begin(R& r,const Context& c,Snapshot& output){
        context=c;graph={};bindingsDone={};measured=false;chatSlot=chatWidget=chatPanel=0;output={};output.world=c.world;output.menu=c.menu;output.hud=c.hud;output.serial=c.identities[4].serial;
        output.hostFlag=c.hostFlag?1u:0u;output.listenHost=c.listenHost?1u:0u;output.seen=1;
        cfgnames::Text text;cfgnames::Observation<R> names(r,c.module);
        if(names.Object(c.type,text))Copy(output.menuClass,text.View());
        const std::string_view label(output.menuClass.data());
        // Name hints are diagnostics only; they never grant host/placement.
        if(label.find("Private")!=label.npos)output.lobbyType=1;
        else if(label.find("Public")!=label.npos||label.find("QuickPlay")!=label.npos)output.lobbyType=2;
        output.placementRefusal=static_cast<unsigned>(c.listenHost?Failure::NoNativeLease:Failure::NoHost);
        if(!c.listenHost){output.complete=1;return true;}
        cfgpracticeobs::WidgetNode root;std::uintptr_t object=0;
        if(!r.Read(c.menu+cfgpractice::disk::WidgetTree,root.tree)||!cfgpracticeobs::Typed(r,c.registry,c.module,root.tree,cfgpractice::disk::WidgetTreeReflection)||
           !r.Read(root.tree+cfgpractice::disk::RootWidget,object)||!cfgpracticeobs::Clear(r,c.registry,object,root.widget))return false;
        graph.hudWidget=c.identities[4];graph.world=c.world;graph.nodes[graph.count++]=root;return true;
    }
    template<class R>static auto Exhausted(const R& r,int)->decltype(r.count,r.cap,bool()){return r.count>=r.cap;}
    template<class R>static auto Exhausted(const R& r,long)->decltype(r.reads,r.cap,bool()){return r.reads>=r.cap;}
    template<class R>static bool Exhausted(const R&,...){return false;}
    template<class R>bool Step(R& r,Snapshot& out,Report& report){
        // Publish a whole node only. A failed/budget-limited step must not lose
        // its children, binding discovery, or the cursor for the next callback.
        Discovery next=*this;std::unique_ptr<Snapshot> staged(new(std::nothrow) Snapshot(out));
        if(!staged)return false;
        auto textRefused=report.textRefused;
        if(!next.StepNode(r,*staged,textRefused)||Exhausted(r,0))return false;
        *this=next;out=*staged;report.textRefused=textRefused;return true;
    }
    template<class R>bool StepNode(R& r,Snapshot& out,unsigned& textRefused){
        if(!graph.count)return false;
        if(graph.cursor>=graph.count||graph.cursor>=NodeLimit){out.complete=1;return true;}
        if(!Current(r))return false;const auto index=graph.cursor;const auto parent=graph.nodes[index].widget.object;
        if(!Path(r,index)){out.truncated=1;return false;}
        auto& n=out.nodes[index];n.object=parent;n.serial=graph.nodes[index].widget.serial;n.parent=graph.nodes[index].parent;
        n.index=graph.nodes[index].index;n.kind=graph.nodes[index].kind;n.slot=graph.nodes[index].slot;
        cfgprobe::Ancestry types;
        std::uintptr_t textBlock=0,panelWidget=0,userWidget=0;
        if(!Ancestry(r,parent,types)||!Class(r,disk::TextBlockReflection,textBlock)||
           !Class(r,cfgpractice::disk::PanelWidgetReflection,panelWidget)||
           !Class(r,cfgpractice::disk::UserWidgetReflection,userWidget))return false;
        n.type=types.classes[0];Name(r,parent,n.name,n.className,types.classes[0]);
        if(types.Contains(textBlock)){
            LobbyTextReader<R> textReader{r};
            n.textState=AuthoredText(textReader,context,parent,n.authoredText)?1u:2u;
            // Unsupported FText shapes remain diagnostic. An incomplete read
            // of a supported shape must not discard this possible row label.
            if(textReader.refused)return false;
            if(n.textState==2)++textRefused;
            const std::string_view text(n.authoredText.data());
            if((text=="Team Killing"||text=="Team killing")&&!bindingsDone[0]){
                if(!Bindings(r,index,out.team))return false;
                bindingsDone[0]=true;
            }else if(text=="Weather"&&!bindingsDone[1]){
                if(!Bindings(r,index,out.weather))return false;
                bindingsDone[1]=true;
            }
        }
        const auto add=[&](cfgpracticeobs::WidgetNode node){
            for(unsigned i=0;i<graph.count;++i)if(graph.nodes[i].widget.object==node.widget.object)return;
            if(graph.count>=NodeLimit){out.truncated=1;return;}graph.nodes[graph.count++]=node;
        };
        if(types.Contains(panelWidget)){
            cfgpracticeobs::WidgetNode::List list;
            if(!r.Read(parent+cfgpractice::disk::Slots,list)||!list.Sane())return false;
            for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){
                if(graph.count>=NodeLimit){out.truncated=1;break;}
                cfgpracticeobs::WidgetNode child;child.parent=index;child.index=i;child.kind=2;child.list=list;std::uintptr_t object=0;
                if(!r.Read(list.data+8ULL*i,child.slot)||!cfgpracticeobs::Typed(r,context.registry,context.module,child.slot,cfgpractice::disk::PanelSlotReflection)||
                   !r.Read(child.slot+cfgpractice::disk::Content,object))return false;
                if(!object)continue;
                if(!cfgpracticeobs::Clear(r,context.registry,object,child.widget))return false;
                add(child);
            }
        }else if(types.Contains(userWidget)){
            cfgpracticeobs::WidgetNode child;child.parent=index;child.kind=1;std::uintptr_t object=0;
            if(!r.Read(parent+cfgpractice::disk::WidgetTree,child.tree))return false;
            if(child.tree){
                if(!cfgpracticeobs::Typed(r,context.registry,context.module,child.tree,cfgpractice::disk::WidgetTreeReflection)||
                   !r.Read(child.tree+cfgpractice::disk::RootWidget,object))return false;
                if(object){
                    if(!cfgpracticeobs::Clear(r,context.registry,object,child.widget))return false;
                    add(child);
                }
            }
        }
        graph.cursor=index+1;out.count=graph.count;out.cursor=graph.cursor;out.complete=graph.cursor>=graph.count?1u:0u;return true;
    }
};
}
