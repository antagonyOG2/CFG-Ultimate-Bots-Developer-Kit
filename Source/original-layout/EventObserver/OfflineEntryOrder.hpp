#pragma once
#include "NativeMenuRowAccess.hpp"
#include "../RuntimeProbe/OfflineEntryObservation.hpp"
#include "../RuntimeProbe/MenuPanelChildrenObservation.hpp"
#include "../RuntimeProbe/NativeWidgetOwnershipObservation.hpp"
#include "MenuSetupSlice.hpp"
namespace cfgmenu::entryorder {
constexpr unsigned Limit=16;
enum class State:unsigned {Empty,Removing,Adding,Verifying,Ready,RollbackRemoving,RollbackAdding,Failed,Detached,Preserved};
inline bool Usable(unsigned state){return state==static_cast<unsigned>(State::Ready)||state==static_cast<unsigned>(State::Preserved);}
// Donor order: keep Jason, move counselor immediately after it, retain others.
// One native call per verified subphase. A bounded owner callback may group
// up to four subphases. No gameplay timer or Slate writes.
class Transaction {
    std::array<std::uintptr_t,Limit> original_{},desired_{},expected_{};
    unsigned count_=0,remaining_=0,index_=1;State state_=State::Empty;
public:
    State Status()const{return state_;}
    bool Finished()const{return state_==State::Ready||state_==State::Failed||state_==State::Detached;}
    bool Begin(const std::array<std::uintptr_t,Limit>& original,unsigned count,std::uintptr_t jason,std::uintptr_t counselor){
        if(state_!=State::Empty||count<2||count>Limit||original[0]!=jason||jason==counselor)return false;
        unsigned found=0;for(unsigned i=0;i<count;++i){if(!cfgprobe::ObjectPointer(original[i]))return false;
            for(unsigned j=0;j<i;++j)if(original[j]==original[i])return false;if(original[i]==counselor)++found;}
        if(found!=1)return false;original_=expected_=original;count_=remaining_=count;desired_[0]=jason;desired_[1]=counselor;
        unsigned at=2;for(unsigned i=1;i<count;++i)if(original[i]!=counselor)desired_[at++]=original[i];
        state_=desired_==original_?State::Ready:State::Removing;return true;
    }
    template<class Host> bool Step(Host& host){
        if(Finished()||state_==State::Empty)return false;
        if(!host.Current()){state_=State::Detached;return false;}
        std::array<std::uintptr_t,Limit> observed{};unsigned count=0;
        if(!host.Snapshot(observed,count)||count<1||count>count_||observed[0]!=original_[0]){state_=State::Detached;return false;}
        for(unsigned i=0;i<count;++i){bool known=false;for(unsigned j=0;j<count_;++j)known|=observed[i]==original_[j];
            if(!known){state_=State::Detached;return false;}}
        if(count!=remaining_||observed!=expected_){state_=State::RollbackRemoving;expected_=observed;remaining_=count;}
        if(state_==State::Removing||state_==State::RollbackRemoving){
            if(remaining_==1){index_=1;state_=state_==State::Removing?State::Adding:State::RollbackAdding;return true;}
            if(!host.Remove(expected_[remaining_-1])){state_=State::RollbackRemoving;return false;}
            expected_[--remaining_]=0;return true;
        }
        if(state_==State::Adding||state_==State::RollbackAdding){
            const auto& order=state_==State::Adding?desired_:original_;
            if(index_==count_){state_=state_==State::Adding?State::Verifying:State::Failed;return true;}
            if(!host.Add(order[index_])){state_=State::RollbackRemoving;return false;}
            expected_[remaining_++]=order[index_++];return true;
        }
        if(state_==State::Verifying){state_=observed==desired_&&count==count_?State::Ready:State::RollbackRemoving;return true;}
        return false;
    }
};

template<class Reader,class Engine> class Native {
    Reader& r_;Engine& engine_;entry::Binding binding_;active::Context fresh_{};
    std::uintptr_t panel_=0,tree_=0,root_=0;
    struct Link {std::uintptr_t node=0,slot=0,parent=0;};
    std::array<Link,8> links_{};unsigned linksCount_=0;
    panelcapture::Snapshot original_{};std::array<cfgprobe::lifetime::SlotObservation,Limit> children_{};
    std::array<cfgreflection::Property,Limit> childReferences_{};
    std::uintptr_t referenceHead_=0,referenceNext_=0;
    std::array<std::uintptr_t,256> referenceFields_{};unsigned referenceFieldsCount_=0;
    struct MemberBinding {std::uintptr_t type=0;std::u16string_view name{};cfgreflection::Property property{};};
    std::array<MemberBinding,32> members_{};unsigned membersCount_=0;
    Transaction transaction_;unsigned phase_=0,failure_=0,captureStage_=0,captureChild_=0;bool terminal_=false,captured_=false;
    MenuWarmDiagnostic warm_{};
    bool preserved_=false;
    bool OriginalStillPresent(){
        if(!captured_||!Current())return false;panelcapture::Snapshot now;
        if(!PanelSnapshot(now)||now.slots.data!=original_.slots.data||now.slots.count!=original_.slots.count||
           now.slots.capacity!=original_.slots.capacity)return false;
        for(unsigned i=0;i<captureChild_;++i)if(now.children[i].widget!=original_.children[i].widget||
            now.children[i].slot!=original_.children[i].slot)return false;
        return true;
    }
    bool Member(std::uintptr_t object,std::u16string_view name,std::uintptr_t& value){
        std::uintptr_t type=0;cfgreflection::Field field;cfgreflection::Property property;cfgreflection::Observation<Reader> reflection(r_,binding_.context.module);
        if(!r_.Read(object+cfgprobe::ObjectClass,type))return false;
        MemberBinding* cached=nullptr;for(unsigned i=0;i<membersCount_;++i)if(members_[i].type==type&&members_[i].name==name){cached=&members_[i];break;}
        if(!cached){
            if(membersCount_==members_.size())return false;bool found=reflection.Find(type,name,field,false);
            // Resolve inherited framework fields directly from the already
            // proved native class caches, not hundreds of unrelated BP locals.
            if(!found&&(name==u"Slot"||name==u"WidgetTree")){std::uintptr_t native=0,ctor=0;
                const auto at=name==u"Slot"?execdisk::AddChildToVerticalBox.reflection:widgetowner::ClassCache;
                const auto ct=name==u"Slot"?execdisk::AddChildToVerticalBox.constructor:widgetowner::ClassConstructor;
                found=r_.Read(binding_.context.module+at,native)&&r_.Read(binding_.context.module+ct,ctor)&&
                    cfgprobe::MatchingClassCache(native,ctor)&&reflection.Find(native,name,field,true)&&maps::Subclass(r_,type,field.owner);
            }
            if(!found){std::array<std::uintptr_t,32> parents{};unsigned count=0;auto parent=type;
                while(parent&&!found){if(count==parents.size())return false;for(unsigned i=0;i<count;++i)if(parents[i]==parent)return false;
                    parents[count++]=parent;found=reflection.Find(parent,name,field,false);
                    if(!found&&!r_.Read(parent+cfgprobe::ClassSuper,parent))return false;}
            }
            if(!found||field.kind.View()!=u"ObjectProperty"||!capture::RawProperty(r_,field,property)||
                property.dimension!=1||property.elementBytes!=8||property.offset<0x28||property.offset>65528)return false;
            cached=&members_[membersCount_++];*cached={type,name,property};
        }
        return type==cached->type&&maps::Subclass(r_,type,cached->property.field.owner)&&
            reflection.ReadField(cached->property.field.object,cached->property.field.owner,field)&&field.kind.View()==u"ObjectProperty"&&
            capture::RawProperty(r_,field,property)&&pendingmember::Same(property,cached->property)&&
            r_.Read(object+static_cast<unsigned>(property.offset),value)&&cfgprobe::ObjectPointer(value);
    }
    bool PanelSnapshot(panelcapture::Snapshot& snapshot){
        std::uintptr_t type=0;cfgreflection::Field field;cfgreflection::Property property;cfgreflection::Observation<Reader> reflection(r_,binding_.context.module);
        return r_.Read(panel_+cfgprobe::ObjectClass,type)&&reflection.Find(type,u"Slots",field,true)&&field.kind.View()==u"ArrayProperty"&&
            capture::RawProperty(r_,field,property)&&property.dimension==1&&property.elementBytes==16&&property.offset==0xF8&&
            panelcapture::Observe(r_,panel_,0xF8,0x28,0x30,snapshot);
    }
    template<std::size_t P,std::size_t C,std::size_t F> bool Warm(const execdisk::Binding& b,const std::array<execdisk::Parameter,P>& parameters,
        const std::array<unsigned char,C>& code,const std::array<unsigned char,F>& factory){
        std::uintptr_t cached=0,owner=0,ctor=0;
        warm_={};warm_.phase=phase_;warm_.stage=1;if(!Current())return false;
        warm_.stage=2;if(!r_.Read(binding_.context.module+b.reflection,owner))return false;warm_.owner=owner;
        warm_.stage=3;if(!r_.Read(binding_.context.module+b.constructor,ctor))return false;warm_.constructor=ctor;
        warm_.stage=4;if(!cfgprobe::MatchingClassCache(owner,ctor))return false;
        warm_.stage=5;if(!r_.Read(binding_.context.module+b.cache,cached))return false;warm_.cached=cached;
        if(!cached){std::array<unsigned char,F> actual{};warm_.stage=6;
            if(!r_.ReadBytes(binding_.context.module+b.factory,actual.data(),actual.size())||actual!=factory)return false;
            warm_.stage=7;if(!rows::FactoryWithOwner(engine_,binding_.context.module+b.factory,owner,cached))return false;}
        warm_.result=cached;warm_.stage=8;capture::FunctionEvidence function;
        if(!native::CachedFunction(r_,binding_.context.module,b,parameters,code,function,&warm_))return false;
        warm_.stage=9;if(function.function.object!=cached)return false;warm_.stage=0;return true;
    }
public:
    Native(Reader& reader,Engine& engine,const entry::Binding& binding):r_(reader),engine_(engine),binding_(binding){}
    bool Current(){
        if(!entry::Current(r_,binding_,engine_.Thread())||!active::Observe(r_,binding_.context.module,fresh_)||!(fresh_==binding_.context))return false;
        if(!panel_)return true;
        std::uintptr_t tree=0,root=0;if(!Member(binding_.context.widget,u"WidgetTree",tree)||tree!=tree_||!Member(tree,u"RootWidget",root)||root!=root_)return false;
        for(unsigned i=0;i<linksCount_;++i){const auto& link=links_[i];std::uintptr_t slot=0,parent=0,content=0;
            if(!Member(link.node,u"Slot",slot)||slot!=link.slot||!Member(slot,u"Parent",parent)||parent!=link.parent||
               !Member(slot,u"Content",content)||content!=link.node)return false;}
        for(unsigned i=0;i<captureChild_;++i){cfgprobe::lifetime::SlotObservation now;
            const auto& saved=childReferences_[i];cfgreflection::Observation<Reader> reflection(r_,binding_.context.module);
            cfgreflection::Field field;cfgreflection::Property property;std::uintptr_t reference=0;
            if(!cfgprobe::lifetime::ReadSlot(r_,fresh_.registry,children_[i].object,now)||!(now==children_[i]))return false;
            if(!saved.field.object)continue; // Acquisition only; mutation requires every strong reference below.
            if(!reflection.ReadField(saved.field.object,saved.field.owner,field)||!(field.name==saved.field.name)||field.type!=saved.field.type||
               field.kind.View()!=u"ObjectProperty"||!capture::RawProperty(r_,field,property)||!pendingmember::Same(property,saved)||
               !r_.Read(binding_.context.widget+static_cast<unsigned>(saved.offset),reference)||reference!=children_[i].object)return false;}
        return true;
    }
    // Capture used to traverse every field/ancestor/child in one HUD callback.
    // The shipping menu exhausted the shared 32768 cap before any mutation.
    // Separate cold acquisition into bounded native HUD phases; revalidate all
    // captured identities and strong references before the first removal.
    bool Capture(){
        if(!entry::Current(r_,binding_,engine_.Thread())||!active::Observe(r_,binding_.context.module,fresh_)||!(fresh_==binding_.context))return false;
        std::uintptr_t slot=0,parent=0,content=0;
        if(captureStage_==0){
            if(!Member(binding_.sandbox,u"Slot",slot)||!Member(slot,u"Parent",parent)||!Member(slot,u"Content",content)||content!=binding_.sandbox)return false;
            panel_=parent;captureStage_=1;return true;
        }
        if(captureStage_==1){
            if(!Member(binding_.context.widget,u"WidgetTree",tree_)||!Member(tree_,u"RootWidget",root_))return false;
            captureStage_=2;return true;
        }
        if(!Current())return false;
        if(captureStage_==2){const auto node=linksCount_?links_[linksCount_-1].parent:panel_;
            if(node==root_){captureStage_=3;return true;}
            if(linksCount_==links_.size()||!Member(node,u"Slot",slot)||!Member(slot,u"Parent",parent)||
               !Member(slot,u"Content",content)||content!=node)return false;
            for(unsigned i=0;i<linksCount_;++i)if(links_[i].node==node)return false;
            links_[linksCount_++]={node,slot,parent};return true;
        }
        if(captureStage_==3){
            if(!PanelSnapshot(original_)||original_.slots.count<2||original_.slots.count>static_cast<int>(Limit))return false;
            for(unsigned i=0;i<static_cast<unsigned>(original_.slots.count);++i){
                if(!cfgprobe::lifetime::ReadSlot(r_,fresh_.registry,original_.children[i].widget,children_[i])||!children_[i].HasAssignedSerial())return false;
            }
            captureChild_=static_cast<unsigned>(original_.slots.count);
            if(!r_.Read(binding_.context.type+cfgreflection::disk::Children,referenceHead_))return false;referenceNext_=referenceHead_;
            captureStage_=4;return true;
        }
        if(captureStage_==4){
            std::uintptr_t head=0;if(!r_.Read(binding_.context.type+cfgreflection::disk::Children,head)||head!=referenceHead_)return false;
            cfgreflection::Observation<Reader> reflection(r_,binding_.context.module);
            // Generated variable names need not equal designer object names.
            // Walk this one source class once, in batches of eight, and match
            // typed strong references by value rather than N full name scans.
            for(unsigned batch=0;batch<8&&referenceNext_;++batch){
                if(referenceFieldsCount_==referenceFields_.size())return false;
                for(unsigned i=0;i<referenceFieldsCount_;++i)if(referenceFields_[i]==referenceNext_)return false;
                cfgreflection::Field field;if(!reflection.ReadField(referenceNext_,binding_.context.type,field))return false;
                referenceFields_[referenceFieldsCount_++]=referenceNext_;referenceNext_=field.next;
                if(field.kind.View()!=u"ObjectProperty")continue;
                cfgreflection::Property property;std::uintptr_t reference=0;
                if(!capture::RawProperty(r_,field,property)||property.dimension!=1||property.elementBytes!=8||property.offset<0||property.offset>65528||
                   !r_.Read(binding_.context.widget+static_cast<unsigned>(property.offset),reference))return false;
                for(unsigned i=0;i<captureChild_;++i)if(reference==children_[i].object){
                    if(childReferences_[i].field.object)return false;childReferences_[i]=property;}
            }
            if(!referenceNext_){for(unsigned i=0;i<captureChild_;++i)if(!childReferences_[i].field.object)return false;captureStage_=5;}
            return true;
        }
        panelcapture::Snapshot now;
        if(!PanelSnapshot(now)||now.slots.data!=original_.slots.data||now.slots.count!=original_.slots.count||now.slots.capacity!=original_.slots.capacity)return false;
        std::array<std::uintptr_t,Limit> widgets{};
        for(unsigned i=0;i<captureChild_;++i){if(now.children[i].widget!=original_.children[i].widget||now.children[i].slot!=original_.children[i].slot)return false;widgets[i]=now.children[i].widget;}
        captured_=transaction_.Begin(widgets,captureChild_,binding_.jason,binding_.sandbox);return captured_;
    }
    bool Snapshot(std::array<std::uintptr_t,Limit>& widgets,unsigned& count){panelcapture::Snapshot snapshot;
        if(!Current()||!PanelSnapshot(snapshot)||snapshot.slots.count>static_cast<int>(Limit))return false;
        widgets={};count=static_cast<unsigned>(snapshot.slots.count);for(unsigned i=0;i<count;++i)widgets[i]=snapshot.children[i].widget;return true;}
    bool Remove(std::uintptr_t child){
        capture::FunctionEvidence f;cfgprobe::Ancestry ancestry;alignas(8) std::array<unsigned char,16> packet{};std::memcpy(packet.data(),&child,8);
        if(!Current()||!native::CachedFunction(r_,binding_.context.module,execdisk::RemoveChild,execdisk::RemoveChildParameters,execdisk::RemoveChildCode,f)||
           !cfgprobe::CaptureAncestry(r_,panel_,ancestry)||!ancestry.Contains(f.function.owner)||!cfgprobe::StableAncestry(r_,panel_,ancestry))return false;
        return engine_.Invoke(panel_,f.function.object,packet.data())&&packet[8]!=0;
    }
    bool Add(std::uintptr_t child){
        capture::FunctionEvidence f;cfgprobe::Ancestry ancestry;alignas(8) std::array<std::uintptr_t,2> packet{child,0};
        if(!Current()||!native::CachedFunction(r_,binding_.context.module,execdisk::AddChildToVerticalBox,execdisk::AddChildToVerticalBoxParameters,execdisk::AddChildToVerticalBoxCode,f)||
           !cfgprobe::CaptureAncestry(r_,panel_,ancestry)||!ancestry.Contains(f.function.owner)||!cfgprobe::StableAncestry(r_,panel_,ancestry))return false;
        return engine_.Invoke(panel_,f.function.object,packet.data())&&cfgprobe::ObjectPointer(packet[1]);
    }
    bool Service(){
        if(terminal_)return false;if(!engine_.BeginPhase()){failure_=1;terminal_=true;return false;}
        bool okay=false;
        if(phase_==0){okay=Capture();if(!okay){failure_=2;terminal_=true;return false;}
            if(!captured_)return true;
            if(transaction_.Status()==State::Ready){terminal_=true;return true;}}
        else if(phase_==1)okay=Warm(execdisk::RemoveChild,execdisk::RemoveChildParameters,execdisk::RemoveChildCode,execdisk::RemoveChildFactoryCode);
        else if(phase_==2)okay=Warm(execdisk::AddChildToVerticalBox,execdisk::AddChildToVerticalBoxParameters,execdisk::AddChildToVerticalBoxCode,execdisk::AddChildToVerticalBoxFactoryCode);
        else{okay=transaction_.Step(*this);if(transaction_.Finished()){terminal_=true;if(transaction_.Status()!=State::Ready)failure_=4;}}
        if(phase_<3){if(!okay){failure_=3;terminal_=true;
                // Ordering is cosmetic. A refused native warmup must not
                // disable the otherwise valid counselor route. Only preserve
                // an independently revalidated, completely untouched panel.
                // Never use this after Remove/Add or an ownership change.
                preserved_=phase_>0&&OriginalStillPresent();
            }else ++phase_;}return okay;
    }
    bool Finished()const{return terminal_;}
    bool ServiceSlice(){
        // Each Service still validates current ownership before its one native
        // call. Stop on refusal/rollback; do not spend a fresh read budget per
        // subphase. Cold member discovery can still occupy its own callback.
        bool worked=false;
        for(unsigned i=0;i<setupslice::MaximumSteps&&!Finished();++i){
            if(i&&!setupslice::MayContinue(r_))break;
            if(!Service())break;
            worked=true;
        }
        return worked;
    }
    unsigned Status()const{return static_cast<unsigned>(preserved_?State::Preserved:terminal_&&failure_==1&&phase_>=3?State::Detached:transaction_.Status());}
    unsigned Failure()const{return failure_;}
    unsigned CaptureStage()const{return captureStage_;}
    const MenuWarmDiagnostic& WarmDiagnostic()const{return warm_;}
};
}
