#pragma once
#include "NativeChallengePageResolver.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include "OwnedPointerCell.hpp"
#include "../RuntimeProbe/SettingsStartObservation.hpp"
#include "../RuntimeProbe/ChallengeMenuCalls.generated.hpp"
#include "../RuntimeProbe/NativeCounselorTransition.hpp"
#include "../RuntimeProbe/MenuStackParentObservation.hpp"
#include <vector>
namespace cfgmenu::challengeroute {
enum class Role:unsigned {LobbyStart,LobbyGraph,PageStart,PageBack,PageGraph,Count};
enum class PendingAction:unsigned {None,Open,Start,Back};
struct Event {
    std::unique_ptr<capture::DiagnosticFunctionEvidence> evidence;
    startbinding::FastCall fast{};int entry=-1;std::uintptr_t thunk=0;bool adopted=false;
};
inline bool Wrapper(const capture::DiagnosticFunctionEvidence& wrapper,const capture::DiagnosticFunctionEvidence& graph,int& entry){
    entry=-1;if(wrapper.parameterBytes||wrapper.storageBytes||wrapper.fieldsCount||wrapper.destructorLink||wrapper.script.count!=22||
       wrapper.bytes[0]!=0x90||wrapper.bytes[13]!=0x5A||wrapper.bytes[18]!=0x97||wrapper.bytes[19]!=0xA5||wrapper.bytes[20]!=0x39||wrapper.bytes[21]!=0xE5)return false;
    std::array<int,3> name{};std::memcpy(name.data(),wrapper.bytes.data()+1,12);
    if(name[0]!=graph.function.name.index||name[1]!=graph.function.name.index||name[2]!=graph.function.name.number)return false;
    std::memcpy(&entry,wrapper.bytes.data()+14,4);return entry>=0&&entry<graph.script.count;
}
inline bool Graph(const capture::DiagnosticFunctionEvidence& graph){
    if(graph.parameterBytes!=4||graph.script.count<1||graph.storageBytes<4||graph.fieldsCount<1)return false;
    unsigned count=0;for(unsigned i=0;i<graph.fieldsCount;++i){const auto& p=graph.fields[i];
        if(p.field.label.View()==u"EntryPoint"&&p.field.kind.View()==u"IntProperty"&&p.elementBytes==4&&p.dimension==1&&p.offset==0&&p.flags==0x0/*CFG3.5 address withheld*/40000294ULL)++count;}
    return count==1;
}
struct PushPacket {std::uintptr_t type=0;std::uint8_t skip=1;std::array<unsigned char,7> pad{};std::uintptr_t result=0;};
struct PopPacket {cfgnames::Words focus{};std::uint8_t force=1,result=0;std::array<unsigned char,6> pad{};};
static_assert(sizeof(PushPacket)==24&&offsetof(PushPacket,result)==16&&sizeof(PopPacket)==16&&offsetof(PopPacket,result)==9);
// Production route. It intercepts only exact owned Blueprint Start aliases;
// every other graph branch and foreign instance forwards its original once.
// It never forwards the obsolete settings-page travel URL.
template<class Reader,class Engine>class Route {
    Reader& r_;Engine& engine_;active::Context source_{};
    std::array<Event,static_cast<unsigned>(Role::Count)> events_{};
    std::array<std::array<Event,3>,16> retiredPages_{};unsigned retiredPagesCount_=0;
    std::uintptr_t pageType_=0,page_=0;cfgprobe::lifetime::SlotObservation pageIdentity_{};
    bool published_=false,busy_=false,continuing_=false,pushAttempted_=false,popAttempted_=false;
    static unsigned PushOutputSub()noexcept{
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const auto stage=rows::NativeRetentionScope::OutputFailure();return stage>=1&&stage<=3?stage+1:1;
#else
        return 1;
#endif
    }
    bool AdoptPush(const PushPacket& packet,bool admitted){
        if(!cfgprobe::ObjectPointer(packet.result)){if(admitted)sub=5;return false;}
        page_=packet.result;pageIdentity_={};choicesReady=false;
        active::Context current;
        // The ordinary admitted push may not have reached a positive stable
        // page observation yet; preserve its existing asynchronous follow-up.
        if(admitted){(void)PageCurrent(current);return true;}
        // A failed post-native admission is never retried. Adopt only if the
        // actual native stack independently proves THIS returned page now.
        if(!CFG_J25_CHALLENGE_PUSH_OUTPUTS)return false;
        if(!PageCurrent(current)){page_=0;pageIdentity_={};return false;}
        ++recoveredPushes;return true;
    }
    struct Pending {PendingAction action=PendingAction::None;Role role=Role::LobbyStart;active::Context context{};std::uint64_t until=0;}pending_;
    std::uintptr_t push_=0,pop_=0;unsigned sourceDepth_=0;
    template<std::size_t N>bool Code(std::uintptr_t at,const std::array<unsigned char,N>& expected){
        std::array<unsigned char,N> actual{};return r_.ReadBytes(at,actual.data(),actual.size())&&actual==expected;
    }
    bool Base(active::Context& current){
        if(GetCurrentThreadId()!=source_.thread||!active::Observe(r_,source_.module,current)||current.world!=source_.world||
           current.mode!=source_.mode||current.engine!=source_.engine||current.viewport!=source_.viewport||current.players!=source_.players||
           current.controller!=source_.controller||current.hud!=source_.hud||current.stack!=source_.stack||current.player!=source_.player||
           current.gameInstance!=source_.gameInstance||!(current.identities[0]==source_.identities[0])||
           !(current.identities[1]==source_.identities[1])||!(current.identities[3]==source_.identities[3]))return false;
        std::uintptr_t modeType=0,native=0,ctor=0;
        return r_.Read(current.mode+cfgprobe::ObjectClass,modeType)&&r_.Read(current.module+execdisk::FrontendClass,native)&&
            r_.Read(current.module+execdisk::FrontendConstructor,ctor)&&cfgprobe::MatchingClassCache(native,ctor)&&maps::Subclass(r_,modeType,native);
    }
    bool Retained(const active::Context& current){
        std::uintptr_t widget=0,type=0;cfgprobe::lifetime::SlotObservation identity;
        if(current.array.count<=static_cast<int>(sourceDepth_)||!r_.Read(current.array.data+execdisk::RecordBytes*sourceDepth_+execdisk::RecordWidget,widget)||
           widget!=source_.widget||!r_.Read(widget+cfgprobe::ObjectClass,type)||type!=source_.type||
           !cfgprobe::lifetime::ReadSlot(r_,current.registry,widget,identity)||!(identity==source_.identities[2]))return false;
        source_.registry=current.registry;return true;
    }
    bool SuspendedSource(const active::Context& current){
        // Shipping CFG Push removes inactive widgets. The independently pinned
        // native stack retains their CLASSES; a null previous widget is normal.
        // It never becomes authority to read/call the old instance.
        pageGuard=2;parentcapture::Record record;
        if(current.array.count!=static_cast<int>(sourceDepth_+2)||
           !parentcapture::NativeCode(r_,source_.module)||
           !parentcapture::Observe(r_,current,static_cast<int>(sourceDepth_),record)||record.menuClass!=source_.type)return false;
        pageGuard=3;
        cfgnames::Observation<Reader> names(r_,source_.module);cfgnames::Text label;
        if(!names.Object(record.menuClass,label)||label.View()!=u"SP_LobbyWidget_C")return false;
        if(record.widget){cfgprobe::lifetime::SlotObservation identity;std::uintptr_t type=0;
            if(record.widget!=source_.widget||!r_.Read(record.widget+cfgprobe::ObjectClass,type)||type!=record.menuClass||
               !cfgprobe::lifetime::ReadSlot(r_,current.registry,record.widget,identity)||!(identity==source_.identities[2]))return false;}
        pageGuard=4;
        return parentcapture::Current(r_,current,record)&&active::Current(r_,current,source_.thread);
    }
    bool ReturnedSource(active::Context& current){
        // Pop may recreate the lobby. Reacquire its ACTUAL positive instance
        // instead of calling a departed widget, while retaining exact class,
        // current world/controller/HUD and source-function publication ledger.
        returnGuard=1;if(!Base(current))return false;
        returnGuard=2;std::uintptr_t recordClass=0,widget=0;
        if(current.array.count!=static_cast<int>(sourceDepth_+1)||current.type!=source_.type||
           !r_.Read(current.array.data+sourceDepth_*execdisk::RecordBytes+parentdisk::RecordClass,recordClass)||recordClass!=source_.type||
           !r_.Read(current.array.data+sourceDepth_*execdisk::RecordBytes+execdisk::RecordWidget,widget)||widget!=current.widget)return false;
        returnGuard=3;
        if(!popAttempted_||!current.identities[2].HasAssignedSerial())return false;
        returnGuard=4;
        if(!EventCurrent(Role::LobbyStart)||!EventCurrent(Role::LobbyGraph)||!active::Current(r_,current,source_.thread))return false;
        source_=current;returnGuard=0;return true;
    }
    bool EventCurrent(Role role){
        eventRole=static_cast<unsigned>(role);eventGuard=1;
        const auto& saved=events_[static_cast<unsigned>(role)];if(!saved.evidence)return false;
        const auto& old=*saved.evidence;cfgreflection::Observation<Reader> reflection(r_,source_.module);cfgreflection::Field field;
        auto fresh=std::make_unique<capture::DiagnosticFunctionEvidence>();startbinding::FastCall fast;
        if(!reflection.ReadField(old.function.object,old.function.owner,field))return false;
        eventGuard=2;if(!capture::KnownFunction(r_,source_.module,field,old.function.label.View(),*fresh))return false;
        eventGuard=3;if(fresh->nativeExecutor!=((published_||saved.adopted)?saved.thunk:source_.module+startdisk::ScriptExecutor)||
           fresh->flags!=old.flags||fresh->parameterBytes!=old.parameterBytes||fresh->storageBytes!=old.storageBytes||fresh->destructorLink!=old.destructorLink||
           !(fresh->script==old.script)||fresh->bytes!=old.bytes||fresh->fieldsCount!=old.fieldsCount||!(fresh->slot==old.slot)||
           fresh->function.type!=old.function.type||!(fresh->function.name==old.function.name)||fresh->function.next!=old.function.next)return false;
        eventGuard=4;if(!startbinding::ReadFastCall(r_,old.function.object,fast)||!(fast==saved.fast))return false;
        eventGuard=5;
        for(unsigned i=0;i<old.fieldsCount;++i)if(!capture::SameProperty(fresh->fields[i],old.fields[i]))return false;
        eventGuard=0;return true;
    }
    bool ObserveEvent(std::uintptr_t type,std::u16string_view name,Role role,std::uintptr_t thunk,Route* inherited){
        auto& event=events_[static_cast<unsigned>(role)];event.evidence=std::make_unique<capture::DiagnosticFunctionEvidence>();
        cfgreflection::Observation<Reader> reflection(r_,source_.module);cfgreflection::Field field;
        if(!reflection.Find(type,name,field,false)||field.owner!=type||!capture::KnownFunction(r_,source_.module,field,name,*event.evidence)||
           !startbinding::ReadFastCall(r_,field.object,event.fast))return false;
        event.thunk=thunk;
        if(event.evidence->nativeExecutor==source_.module+startdisk::ScriptExecutor)return true;
        const auto i=static_cast<unsigned>(role);
        if(event.evidence->nativeExecutor!=thunk||!inherited||!inherited->Published()||inherited->Source().module!=source_.module||
           !inherited->events_[i].evidence||inherited->events_[i].evidence->function.object!=field.object||
           inherited->events_[i].thunk!=thunk||!inherited->EventCurrent(role))return false;
        event.adopted=true;return true;
    }
    bool Native(const challengecalls::Binding& binding,std::uintptr_t& function,bool warm){
        std::uintptr_t owner=0,ctor=0,found=0;
        if(!r_.Read(source_.module+binding.owner,owner)||!r_.Read(source_.module+binding.ctor,ctor)||!cfgprobe::MatchingClassCache(owner,ctor)||
           !r_.Read(source_.module+binding.cache,found))return false;
        if(!found){if(!warm)return false;bool pinned=false;
            if(binding.name==challengecalls::PushMenu.name)pinned=Code(source_.module+binding.factory,challengecalls::PushMenuFactory);
            if(binding.name==challengecalls::PopMenu.name)pinned=Code(source_.module+binding.factory,challengecalls::PopMenuFactory);
            if(!pinned||!engine_.Factory(source_.module+binding.factory,found))return false;}
        cfgreflection::Observation<Reader> reflection(r_,source_.module);cfgreflection::Field field;
        auto f=std::make_unique<capture::FunctionEvidence>();
        if(!reflection.ReadField(found,owner,field)||!capture::KnownFunction(r_,source_.module,field,binding.name,*f)||
           f->nativeExecutor!=source_.module+binding.executor||f->parameterBytes!=binding.logical||f->storageBytes!=static_cast<int>(binding.storage)||
           f->flags!=binding.flags||f->script.count||f->destructorLink||f->fieldsCount!=3)return false;
        if(binding.name==challengecalls::PushMenu.name){if(!Code(source_.module+binding.executor,challengecalls::PushMenuCode)||!counselortransition::NativeSchema(*f,true))return false;}
        else{
            if(!Code(source_.module+binding.executor,challengecalls::PopMenuCode))return false;unsigned mask=0;
            for(unsigned i=0;i<f->fieldsCount;++i){const auto& p=f->fields[i];unsigned bit=0;
                if(p.dimension!=1||p.field.owner!=found)return false;
                if(counselortransition::Property(p,u"WidgetToFocus",u"NameProperty",8,0,0x0018001040000280ULL))bit=1;
                if(counselortransition::Property(p,u"bForce",u"BoolProperty",1,8,0x0018041040000282ULL))bit=2;
                if(counselortransition::Property(p,u"ReturnValue",u"BoolProperty",1,9,0x0018001040000780ULL))bit=4;
                if(!bit||(mask&bit))return false;mask|=bit;
            }if(mask!=7)return false;
        }
        if(function&&function!=found)return false;function=found;return true;
    }
public:
    unsigned state=0,failure=0,opens=0,backs=0,starts=0;bool choicesReady=false;
    unsigned sub=0,recoveredPushes=0;
    unsigned startGuard=0,eventGuard=0,eventRole=0,pageRefreshFailure=0;
    unsigned pageGuard=0,returnGuard=0,nativeBackFallbacks=0;
    Route(Reader& reader,Engine& engine):r_(reader),engine_(engine){}
    const active::Context& Source()const{return source_;}
    std::uintptr_t Page()const{return page_;}
    std::uintptr_t PageType()const{return pageType_;}
    const cfgprobe::lifetime::SlotObservation& PageIdentity()const{return pageIdentity_;}
    bool Published()const{return published_;}
    unsigned PageRefreshes()const{return retiredPagesCount_;}
    bool Busy()const{return busy_||continuing_;}
    bool Rebind(const active::Context& source){
        if(!published_||busy_||continuing_||state==2||state==3||state==5||state==7||source.type!=source_.type)return false;
        const auto old=source_;const auto depth=sourceDepth_;source_=source;sourceDepth_=static_cast<unsigned>(source.array.count-1);
        active::Context current;
        if(!Base(current)||!Retained(current)||current.widget!=source.widget||!EventCurrent(Role::LobbyStart)||!EventCurrent(Role::LobbyGraph)){
            source_=old;sourceDepth_=depth;return false;
        }
        page_=0;pageIdentity_={};choicesReady=false;pushAttempted_=false;popAttempted_=false;failure=0;state=1;return true;
    }
    bool Prepare(const active::Context& source,std::uintptr_t pageType,const std::array<std::uintptr_t,5>& thunks,Route* inherited=nullptr){
        if(published_||state||!pageType||GetCurrentThreadId()!=source.thread)return false;failure=10;source_=source;pageType_=pageType;sourceDepth_=static_cast<unsigned>(source.array.count-1);
        active::Context current;cfgnames::Observation<Reader> names(r_,source.module);cfgnames::Text name,package;
        std::uintptr_t outer=0;
        if(!Base(current)||!Retained(current)||current.widget!=source.widget||!names.Object(source.type,name)||name.View()!=u"SP_LobbyWidget_C"||
           !names.Object(pageType,name)||name.View()!=u"SPChallengesSettingsMenuWidget_C"||!r_.Read(pageType+0x20,outer)||
           !names.Object(outer,package)||package.View()!=u"/Game/UI/Menu/FrontEnd/OfflinePlay/SPChallengesSettingsMenuWidget"||
           !Code(source.module+startdisk::ScriptExecutor,startdisk::ScriptExecutorCode))return false;
        failure=11;if(!ObserveEvent(source.type,u"OnStartPressed",Role::LobbyStart,thunks[0],inherited)||
           !ObserveEvent(source.type,u"ExecuteUbergraph_SP_LobbyWidget",Role::LobbyGraph,thunks[1],inherited)||
           !ObserveEvent(pageType,u"OnClicked_Start",Role::PageStart,thunks[2],inherited)||
           !ObserveEvent(pageType,u"OnClicked_Back",Role::PageBack,thunks[3],inherited)||
           !ObserveEvent(pageType,u"ExecuteUbergraph_SPChallengesSettingsMenuWidget",Role::PageGraph,thunks[4],inherited))return false;
        failure=12;const auto& lobby=*events_[1].evidence;const auto& graph=*events_[4].evidence;
        if(!Graph(lobby)||!Graph(graph)||lobby.script.count!=2268||lobby.storageBytes!=220||
           !Wrapper(*events_[0].evidence,lobby,events_[0].entry)||events_[0].entry!=0x790||
           !Wrapper(*events_[2].evidence,graph,events_[2].entry)||!Wrapper(*events_[3].evidence,graph,events_[3].entry)||
           events_[2].entry==events_[3].entry)return false;
        for(unsigned i:{0u,2u,3u})if(events_[i].fast.graph&&
            (events_[i].fast.graph!=events_[i?4:1].evidence->function.object||events_[i].fast.entry!=events_[i].entry))return false;
        failure=13;if(!engine_.BeginPhase()||!Native(challengecalls::PushMenu,push_,true)||!Native(challengecalls::PopMenu,pop_,true)||!Base(current)||!Retained(current))return false;
        failure=0;state=1;return true;
    }
    bool Publish(HMODULE resident){
        return PublishUsing(resident,[](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){return ExchangeOwnedPointer(at,old,value);});
    }
    template<class Exchange>bool PublishUsing(HMODULE resident,Exchange exchange){
        if(state!=1||published_||!resident)return false;unsigned processed=0;std::array<bool,5> changed{};bool ambiguous=false;
        for(unsigned i=0;i<events_.size();++i){HMODULE module=nullptr;
            if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(events_[i].thunk),&module)||module!=resident||
               !EventCurrent(static_cast<Role>(i)))break;
            if(!events_[i].adopted){const auto result=exchange(events_[i].evidence->function.object+startdisk::Executor,
                   source_.module+startdisk::ScriptExecutor,events_[i].thunk);
                if(result!=sandboxstart::WriteResult::Changed){ambiguous=result==sandboxstart::WriteResult::Indeterminate;break;}changed[i]=true;}
            ++processed;
        }
        if(processed!=events_.size()){
            bool restored=!ambiguous;
            while(processed){const auto i=--processed;const auto& event=events_[i];if(changed[i])restored&=exchange(event.evidence->function.object+startdisk::Executor,event.thunk,source_.module+startdisk::ScriptExecutor)==sandboxstart::WriteResult::Changed;}
            failure=restored?1u:18u;state=8;return false;
        }
        published_=true;return true;
    }
    // Reacquisition must happen BEFORE any old settings-class/function read.
    // Native travel/GC may retire a serial0 metadata class; it is not a lease.
    // Rebuild only freshly observed own-class cells, retaining old records and
    // forwarding their foreign/old-page frames to the pinned original.
    bool RefreshPage(std::uintptr_t type,HMODULE resident){
        return RefreshPageUsing(type,resident,[](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){return ExchangeOwnedPointer(at,old,value);});
    }
    template<class Exchange>bool RefreshPageUsing(std::uintptr_t type,HMODULE resident,Exchange exchange){
        pageRefreshFailure=1;
        if(!published_||state!=1||busy_||continuing_||!type||!resident)return false;
        if(type==pageType_){pageRefreshFailure=0;return true;}
        pageRefreshFailure=2;if(retiredPagesCount_==retiredPages_.size())return false;
        active::Context current;pageRefreshFailure=3;
        if(!Base(current)||!Retained(current)||current.widget!=source_.widget)return false;
        std::array<std::uintptr_t,5> thunks{};
        for(unsigned i=0;i<thunks.size();++i)thunks[i]=events_[i].thunk;
        auto next=std::make_unique<Route>(r_,engine_);pageRefreshFailure=4;
        // PrepareInherited validates the two SOURCE cells against their saved
        // ledger. All three page cells must be original or independently owned.
        if(!next->Prepare(current,type,thunks,this))return false;
        pageRefreshFailure=5;if(!next->PublishUsing(resident,exchange))return false;
        for(unsigned i=0;i<3;++i){retiredPages_[retiredPagesCount_][i]=std::move(events_[i+2]);
            events_[i+2]=std::move(next->events_[i+2]);}
        ++retiredPagesCount_;pageType_=type;page_=0;pageIdentity_={};choicesReady=false;
        pageRefreshFailure=0;return true;
    }
    bool PageCurrent(active::Context& current){
        // Push's result is provisional until the actual native stack owns an
        // assigned-serial page. Never dereference that returned pointer first.
        pageGuard=1;if(!Base(current))return false;
        if(!SuspendedSource(current))return false;
        pageGuard=5;if(!page_||current.widget!=page_||current.type!=pageType_)return false;
        if(state==2){
            pageIdentity_=current.identities[2];state=3;++opens;
        }
        pageGuard=6;if(!(current.identities[2]==pageIdentity_))return false;
        pageGuard=0;return true;
    }
    // Caller commits supported selections BEFORE requesting continuation.
    unsigned pendingQueued=0,pendingCoalesced=0,pendingDropped=0,pendingExpired=0,pendingConsumed=0;
    bool ContinuationInvocation(std::uintptr_t actor,std::uintptr_t function)const noexcept{
        return continuing_&&state==6&&actor==source_.widget&&events_[0].evidence&&function==events_[0].evidence->function.object;
    }
    bool QueueBorrowed(Role role,void* receiver,void* frame,std::uint64_t now){
        const auto object=reinterpret_cast<std::uintptr_t>(receiver),at=reinterpret_cast<std::uintptr_t>(frame);
        if(!published_||GetCurrentThreadId()!=source_.thread||busy_||continuing_||static_cast<unsigned>(role)>=events_.size())return false;
        const auto& event=events_[static_cast<unsigned>(role)];std::uintptr_t node=0,owner=0;
        if(!cfgprobe::lifetime::Extent(at,startdisk::FrameCurrentNative+8)||!r_.Read(at+startdisk::FrameNode,node)||
           node!=event.evidence->function.object||!r_.Read(at+startdisk::FrameObject,owner)||owner!=object)return false;
        bool open=role==Role::LobbyStart,start=role==Role::PageStart,back=role==Role::PageBack;
        if(role==Role::LobbyGraph||role==Role::PageGraph){std::uintptr_t locals=0;int entry=-1;
            if(!r_.Read(at+startdisk::FrameLocals,locals)||!cfgprobe::ObjectPointer(locals)||!r_.Read(locals,entry))return false;
            open=role==Role::LobbyGraph&&entry==events_[0].entry;
            start=role==Role::PageGraph&&entry==events_[2].entry;back=role==Role::PageGraph&&entry==events_[3].entry;}
        if((!open&&!start&&!back)||(open&&object!=source_.widget)||((start||back)&&object!=page_))return false;
        active::Context current;
        if(!EventCurrent(role)||(open?state!=1||!Base(current)||!Retained(current)||current.widget!=source_.widget:!PageCurrent(current)))return false;
        const auto action=open?PendingAction::Open:start?PendingAction::Start:PendingAction::Back;
        if(pending_.action==action&&pending_.context==current&&now<=pending_.until){++pendingCoalesced;return true;}
        if(now>UINT64_MAX-2000){++pendingDropped;return false;}
        if(pending_.action!=PendingAction::None)++pendingDropped;
        pending_={action,role,current,now+2000};++pendingQueued;return true;
    }
    template<class Commit,class Refresh,class Cancel>bool ServicePending(std::uint64_t now,Commit commit,Refresh refresh,Cancel cancel){
        if(pending_.action==PendingAction::None)return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!rows::NativeRetentionScope::Current(source_.module))return false;
#endif
        const auto queued=pending_;pending_={}; // consumed BEFORE any native call
        if(now>queued.until){++pendingExpired;return false;}
        active::Context current;
        if(!active::Observe(r_,source_.module,current)||!(current==queued.context)||!EventCurrent(queued.role)){++pendingDropped;return false;}
        ++pendingConsumed;
        if(queued.action==PendingAction::Open){
            if(state!=1||!Base(current)||!Retained(current)||current.widget!=source_.widget||!engine_.BeginPhase()||
               !Native(challengecalls::PushMenu,push_,false)||!refresh(current)||!Base(current)||!Retained(current)||
               current.widget!=source_.widget||!EventCurrent(Role::PageStart)||!EventCurrent(Role::PageBack)||
               !EventCurrent(Role::PageGraph)||!Native(challengecalls::PushMenu,push_,false)){++pendingDropped;return false;}
            busy_=true;pushAttempted_=true;state=2;PushPacket packet{pageType_};sub=0;
            const bool returned=engine_.Invoke(source_.widget,push_,&packet);busy_=false;
            if(!returned)sub=PushOutputSub();
            if(!AdoptPush(packet,returned)){failure=3;state=8;return false;}
            failure=0;return true;
        }
        if(!PageCurrent(current)){++pendingDropped;return false;}
        busy_=true;bool result=false;
        if(queued.action==PendingAction::Back){cancel();result=Return(false);}
        else if(!choicesReady||!commit(current))failure=5;
        else result=Return(true);
        busy_=false;return result;
    }
    bool Return(bool start){
        popAttempted_=false;
        active::Context current;if(!PageCurrent(current)||!engine_.BeginPhase()||!Native(challengecalls::PopMenu,pop_,false))return false;
        PopPacket packet;state=start?5u:7u;
        popAttempted_=true;sub=0;
        if(!engine_.Invoke(page_,pop_,&packet)){sub=PushOutputSub();failure=6;state=8;return false;}
        if(!packet.result){sub=5;failure=7;state=8;return false;}
        if(start)return Continue();
        return FinishBack();
    }
    bool FinishBack(){
        active::Context current;if(state!=7||!ReturnedSource(current))return false;
        ++backs;page_=0;pageIdentity_={};choicesReady=false;pushAttempted_=false;state=1;return true;
    }
    bool Continue(){
        active::Context current;if(state!=5||continuing_||!ReturnedSource(current))return false;
        continuing_=true;state=6;++starts;alignas(16) std::array<unsigned char,16> packet{};
        const bool result=engine_.Invoke(source_.widget,events_[0].evidence->function.object,packet.data());continuing_=false;
        if(!result){failure=8;state=8;}return result;
    }
    bool ResumeStart(){return state==5&&engine_.BeginPhase()&&Continue();}
    bool ResumeReturn(){return state==7?FinishBack():ResumeStart();}
    template<class Forward,class Commit>void Dispatch(Role role,void* receiver,void* frame,Forward original,Commit commit){
        Dispatch(role,receiver,frame,original,commit,[](const active::Context&){return true;});
    }
    template<class Forward,class Commit,class Refresh>void Dispatch(Role role,void* receiver,void* frame,Forward original,Commit commit,Refresh refresh){
        Dispatch(role,receiver,frame,original,commit,refresh,[](){});
    }
    template<class Forward,class Commit,class Refresh,class Cancel>void Dispatch(Role role,void* receiver,void* frame,Forward original,Commit commit,Refresh refresh,Cancel cancel){
        const auto object=reinterpret_cast<std::uintptr_t>(receiver);
        if(!published_||GetCurrentThreadId()!=source_.thread||busy_||continuing_){original();return;}
        const auto frameAt=reinterpret_cast<std::uintptr_t>(frame);const auto& event=events_[static_cast<unsigned>(role)];
        std::uintptr_t node=0,frameObject=0;
        if(!cfgprobe::lifetime::Extent(frameAt,startdisk::FrameCurrentNative+8)||
           !r_.Read(frameAt+startdisk::FrameNode,node)||node!=event.evidence->function.object||
           !r_.Read(frameAt+startdisk::FrameObject,frameObject)||frameObject!=object){original();return;}
        bool sourceStart=role==Role::LobbyStart,pageStart=role==Role::PageStart,pageBack=role==Role::PageBack;
        if(role==Role::LobbyGraph||role==Role::PageGraph){std::uintptr_t locals=0;int entry=-1;
            if(!r_.Read(reinterpret_cast<std::uintptr_t>(frame)+startdisk::FrameLocals,locals)||!cfgprobe::ObjectPointer(locals)||!r_.Read(locals,entry)){original();return;}
            sourceStart=role==Role::LobbyGraph&&entry==events_[0].entry;
            pageStart=role==Role::PageGraph&&entry==events_[2].entry;pageBack=role==Role::PageGraph&&entry==events_[3].entry;
        }
        if((!sourceStart&&!pageStart&&!pageBack)||(sourceStart&&object!=source_.widget)||((pageStart||pageBack)&&object!=page_)){original();return;}
        active::Context current;
        if(sourceStart){
            if(state!=1){if(!pushAttempted_)original();return;}
            const auto guard=[&](bool okay,unsigned stage){if(!okay){startGuard=stage;failure=2;}return okay;};
            startGuard=0;
            if(!guard(Base(current),1)||!guard(Retained(current),2)||!guard(current.widget==source_.widget,3)||
               !guard(EventCurrent(role),4)||!guard(engine_.BeginPhase(),5)||
               !guard(Native(challengecalls::PushMenu,push_,false),6)){original();return;}
            // Reacquire the cooked class through its verified owned soft
            // reference on THIS Start callback, not a retained serial0 lease.
            if(!guard(refresh(current),7)||!guard(Base(current),8)||!guard(Retained(current),9)||
               !guard(current.widget==source_.widget,10)||!guard(EventCurrent(Role::PageStart),11)||
               !guard(EventCurrent(Role::PageBack),12)||!guard(EventCurrent(Role::PageGraph),13)||
               !guard(Native(challengecalls::PushMenu,push_,false),14)){original();return;}
            failure=0;
            busy_=true;pushAttempted_=true;state=2;PushPacket packet{pageType_};sub=0;
            const bool returned=engine_.Invoke(source_.widget,push_,&packet);busy_=false;
            if(!returned)sub=PushOutputSub();
            if(!AdoptPush(packet,returned)){failure=3;state=8;return;}
            failure=0;return;
        }
        if(pageBack)cancel(); // Only this route's verified borrowed Back frame.
        if(!PageCurrent(current)||!EventCurrent(role)){
            failure=4;
            if(pageBack){
                // A refused MOD admission must not swallow native Back. Clear
                // our page claim before forwarding so its wrapper/graph aliases
                // cannot recursively intercept the same user Back action.
                page_=0;choicesReady=false;state=8;++nativeBackFallbacks;original();
            }
            return;
        }
        busy_=true;
        if(pageBack){
            if(!Return(false)&&!popAttempted_){page_=0;choicesReady=false;state=8;++nativeBackFallbacks;original();}
        }
        else if(!choicesReady||!commit(current)){failure=5;}
        else Return(true);
        busy_=false;
    }
};
}
