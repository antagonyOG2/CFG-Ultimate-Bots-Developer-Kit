#pragma once
#include "CounselorTerminalObservation.hpp"
#include "../Core/NativeEmoteMirrorQueue.hpp"
#include "../RuntimeProbe/EmoteMirrorBindings.generated.hpp"
#include <type_traits>
#include <utility>
namespace cfgemotemirror {
using Slot=cfgprobe::lifetime::SlotObservation;
struct Recipient {Slot pawn{},controller{};};
struct Inputs {
    static constexpr unsigned Capacity=64; // native roster plus reinforcements, donor target capacity
    std::uintptr_t module=0,world=0,controller=0,counselorClass=0,counselorAIClass=0;
    Slot controllerSlot{};
    std::array<Recipient,Capacity> bots{};unsigned count=0;
};
struct Catalogue {
    Slot source{},type{},defaults{},male{},female{};
    std::uintptr_t selected=0,montage=0;
    bool SameAssets(const Catalogue& b)const{return source==b.source&&type==b.type&&defaults==b.defaults&&
        male==b.male&&female==b.female&&selected==b.selected;}
    bool Contains(std::uintptr_t value)const{return value&&(value==male.object||value==female.object);}
};
struct Function {std::uintptr_t object=0,owner=0;Slot slot{},ownerSlot{};};
struct Report {
    unsigned prepared=0,starts=0,queued=0,attempts=0,returns=0,started=0,skipped=0,cancels=0,refused=0;
    unsigned cancelAttempts=0,cancelReturns=0,stopped=0,ownershipLost=0;
    unsigned startedUnchanged=0,startedOtherMontage=0,startedReadRefused=0;
    unsigned functionBindClause=0,functionBindReads=0;
    unsigned failure=0; // 1=context/source,2=function,3=catalogue,4=completion,5=recipient,6=dispatch
    unsigned failSite=0,failClause=0,failBindReads=0; // H2.45 append-only: 1=Before,2=Service of the last failure=2; clause=last Bind refuse clause
};
// Host: Context(), MirrorHuman(Slot&), FeatureFunction(binding,fn,owner),
// Invoke(receiver,fn,packet,actor), public context.objectArray. Same bounded
// reader/call accounting as its gameplay host. All retained zero-serial assets
// require a fresh native selection -> typed class -> CDO -> montage chain.
template<class R,class H> class NativeAccess {
    R& r_;H& h_;Inputs in_;
    template<class T>static auto AtReadLimit(const T& r,int)->decltype(r.reads,r.cap,bool()){return r.reads>=r.cap;}
    template<class T>static bool AtReadLimit(const T&,long){return false;}
    template<class T> bool Same(std::uintptr_t at,const T& value){return cfgescape::Same(r_,at,value);}
    bool CurrentSlot(const Slot& slot){Slot fresh;return cfgspawn::ObjectSlot(r_,h_.context.objectArray,slot.object,fresh)&&fresh==slot;}
    bool Class(std::uintptr_t reflection,std::uintptr_t ctor,std::uintptr_t& value){
        return r_.Read(in_.module+reflection,value)&&value&&Same(in_.module+ctor,value);
    }
    template<class T>static auto InterruptHost(T& h,const Recipient& target,int)->decltype(h.MirrorInterrupt(target)){return h.MirrorInterrupt(target);}
    template<class T>static unsigned InterruptHost(T&,const Recipient&,long){return 1;}
    template<class T>static auto PlayedHost(T& h,const Recipient& target,std::uintptr_t montage,int)->decltype(h.MirrorPlayed(target,montage),void()){h.MirrorPlayed(target,montage);}
    template<class T>static void PlayedHost(T&,const Recipient&,std::uintptr_t,long){}
public:
    unsigned proofStage=0,proofDifference=0,lastBindClause=0,lastBindReads=0;
    NativeAccess(R& r,H& h,const Inputs& in):r_(r),h_(h),in_(in){}
    const Inputs& Input()const{return in_;}
    unsigned BindClause()const{return lastBindClause;}
    unsigned BindReads()const{return lastBindReads;}
    bool ReadExhausted()const{return AtReadLimit(r_,0);}
    bool Guard(){return in_.count<=Inputs::Capacity&&h_.Context()&&h_.context.world==in_.world&&
        in_.controllerSlot.object==in_.controller&&CurrentSlot(in_.controllerSlot);}
    bool Source(Slot& source){
        source={};return Guard()&&h_.MirrorHuman(source)&&source.HasAssignedSerial()&&CurrentSlot(source)&&
            Same(in_.controller+cfgescape::disk::Pawn,source.object)&&
            Same(source.object+cfgescape::disk::Controller,in_.controller);
    }
    bool Montage(std::uintptr_t pawn,std::uintptr_t& out){return r_.Read(pawn+disk::CurrentEmote,out);}
    bool Parameter(std::uintptr_t address,std::uintptr_t& selected){return address&&r_.Read(address,selected)&&selected;}
    bool Bind(unsigned id,Function& out,bool warm){
        out={};lastBindClause=lastBindReads=0;const auto readsBefore=r_.reads;
        const auto refuse=[&](unsigned clause){lastBindClause=clause;lastBindReads=r_.reads-readsBefore;return false;};
        if(id>1)return refuse(1);
        if(!Guard())return refuse(2);
        const auto& binding=id?disk::SERVER_CancelEmote:disk::SERVER_PlayEmote;
        const auto executor=id?disk::SERVER_CancelEmoteExecutor:disk::SERVER_PlayEmoteExecutor;
        std::uintptr_t cached=0;
        if(!r_.Read(in_.module+binding.cache,cached))return refuse(3);
        if(!warm&&!cached)return refuse(4);
        if(!h_.FeatureFunction(binding,out.object,out.owner))return refuse(5);
        if(out.owner!=in_.counselorClass)return refuse(6);
        if(!Same(out.object+0xB0,in_.module+executor))return refuse(7);
        if(!(id?cfgbundle::Code(r_,in_.module,executor,disk::SERVER_CancelEmoteCode):
                cfgbundle::Code(r_,in_.module,executor,disk::SERVER_PlayEmoteCode)))return refuse(8);
        if(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,out.object,out.slot))return refuse(9);
        if(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,out.owner,out.ownerSlot))return refuse(10);
        lastBindReads=r_.reads-readsBefore;
        return true;
    }
    bool FunctionCurrent(unsigned id,const Function& saved){
        Function fresh;return Bind(id,fresh,false)&&fresh.object==saved.object&&fresh.owner==saved.owner&&fresh.slot==saved.slot&&fresh.ownerSlot==saved.ownerSlot;
    }
    bool CatalogueFor(const Slot& source,std::uintptr_t selected,Catalogue& out){
        out={};std::uintptr_t emoteType=0,montageType=0;
        // SERVER_PlayEmote supplies the selected class, including equipped
        // custom emotes absent from DefaultEmotes. That array is not an
        // allowlist. After() additionally requires the human's actual start.
        if(!selected||!source.HasAssignedSerial()||!CurrentSlot(source)||!cfgspawn::ObjectIs(r_,source.object,in_.counselorClass)||
           !Class(disk::SCEmoteDataReflection,disk::SCEmoteDataConstructor,emoteType)||
           !Class(disk::AnimMontageReflection,disk::AnimMontageConstructor,montageType))return false;
        Catalogue value;value.source=source;value.selected=selected;std::uintptr_t defaults=0,male=0,female=0;
        if(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,selected,value.type)||
           !cfgspawn::ClassContains(r_,selected,emoteType)||!r_.Read(selected+cfgspawn::ClassDefaultObject,defaults)||
           !cfgspawn::ObjectSlot(r_,h_.context.objectArray,defaults,value.defaults)||
           !Same(defaults+cfgprobe::ObjectClass,selected)||!cfgspawn::ObjectIs(r_,defaults,emoteType)||
           !r_.Read(defaults+disk::MaleMontage,male)||!r_.Read(defaults+disk::FemaleMontage,female))return false;
        if(male&&(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,male,value.male)||!cfgspawn::ObjectIs(r_,male,montageType)))return false;
        if(female&&(!cfgspawn::ObjectSlot(r_,h_.context.objectArray,female,value.female)||!cfgspawn::ObjectIs(r_,female,montageType)))return false;
        if(!male&&!female)return false;
        if(!Same(in_.module+disk::SCEmoteDataReflection,emoteType)||!Same(in_.module+disk::SCEmoteDataConstructor,emoteType)||
           !Same(in_.module+disk::AnimMontageReflection,montageType)||!Same(in_.module+disk::AnimMontageConstructor,montageType)||
           !cfgspawn::ObjectIs(r_,source.object,in_.counselorClass)||!cfgspawn::ClassContains(r_,selected,emoteType)||
           !Same(defaults+cfgprobe::ObjectClass,selected)||!cfgspawn::ObjectIs(r_,defaults,emoteType)||
           (male&&!cfgspawn::ObjectIs(r_,male,montageType))||(female&&!cfgspawn::ObjectIs(r_,female,montageType))||
           !Same(selected+cfgspawn::ClassDefaultObject,defaults)||!Same(defaults+disk::MaleMontage,male)||
           !Same(defaults+disk::FemaleMontage,female)||!CurrentSlot(source)||!CurrentSlot(value.type)||!CurrentSlot(value.defaults)||
           (male&&!CurrentSlot(value.male))||(female&&!CurrentSlot(value.female)))return false;
        out=value;return true;
    }
    // Queue identity is the admitted class/CDO, not the human's gender montage
    // weak serial. SERVER_PlayEmote remains the recipient gender authority.
    bool IdentityFor(const Catalogue& saved,Catalogue& out){
        out={};proofDifference=0;proofStage=1;std::uintptr_t base=0,defaults=0;Slot type{},cdo{};
        if(!Guard())return false;
        proofStage=2;if(!saved.source.HasAssignedSerial()||!CurrentSlot(saved.source)||
            !cfgspawn::ObjectIs(r_,saved.source.object,in_.counselorClass))return false;
        proofStage=3;if(!Class(disk::SCEmoteDataReflection,disk::SCEmoteDataConstructor,base)||
            !cfgspawn::ObjectSlot(r_,h_.context.objectArray,saved.selected,type)||
            !cfgspawn::ClassContains(r_,saved.selected,base))return false;
        proofStage=4;if(!r_.Read(saved.selected+cfgspawn::ClassDefaultObject,defaults)||
            !cfgspawn::ObjectSlot(r_,h_.context.objectArray,defaults,cdo)||
            !Same(defaults+cfgprobe::ObjectClass,saved.selected)||!cfgspawn::ObjectIs(r_,defaults,base))return false;
        proofStage=5;
        if(type.object!=saved.type.object||type.index!=saved.type.index||(saved.type.serial&&type.serial!=saved.type.serial)){proofDifference=1;return false;}
        if(cdo.object!=saved.defaults.object||cdo.index!=saved.defaults.index||(saved.defaults.serial&&cdo.serial!=saved.defaults.serial)){proofDifference=2;return false;}
        proofStage=6;if(!CurrentSlot(type)||!CurrentSlot(cdo)||!CurrentSlot(saved.source)||
            !Same(saved.selected+cfgspawn::ClassDefaultObject,defaults)||!Same(defaults+cfgprobe::ObjectClass,saved.selected)||
            !Same(in_.module+disk::SCEmoteDataReflection,base)||!Same(in_.module+disk::SCEmoteDataConstructor,base))return false;
        out=saved;out.type=type;out.defaults=cdo;proofStage=0;return true;
    }
    bool RecipientCurrent(const Recipient& target){
        if(!target.pawn.HasAssignedSerial()||!target.controller.HasAssignedSerial()||target.controller.object==in_.controller||
           !CurrentSlot(target.pawn)||!CurrentSlot(target.controller)||
           !cfgspawn::ObjectIs(r_,target.controller.object,in_.counselorAIClass)||
           !Same(target.pawn.object+cfgescape::disk::Controller,target.controller.object)||
           !Same(target.controller.object+cfgescape::disk::Pawn,target.pawn.object))return false;
        cfgescape::State state;
        return cfgescape::Counselor(r_,h_.context.objectArray,in_.module,target.pawn.object,in_.counselorClass,state)&&state==cfgescape::State::Living;
    }
    bool Dispatch(const Recipient& target,const Function& function,std::uintptr_t selected){
        // Original server UFunction keeps its validation and gender selection.
        // Never dispatch multicast directly or pass a class as a montage.
        return h_.Invoke(target.pawn.object,function.object,&selected,true);
    }
    unsigned Interrupt(const Recipient& target){return InterruptHost(h_,target,0);}
    void Played(const Recipient& target,std::uintptr_t montage){PlayedHost(h_,target,montage,0);}
    bool DispatchCancel(const Recipient& target,const Function& function){
        return h_.Invoke(target.pawn.object,function.object,nullptr,true);
    }
};
class Runtime {
    std::array<Function,2> functions_{};Queue queue_;Catalogue active_{};
    std::array<Recipient,Queue::Capacity> recipients_{};
    std::array<std::uintptr_t,Queue::Capacity> startedMontages_{};
    std::array<unsigned,Queue::Capacity> queueIndices_{};
    unsigned recipientCount_=0;bool cancelling_=false;
    struct Ticket {bool valid=false;unsigned id=0;Catalogue catalogue{};Slot source{};std::uintptr_t before=0;};
    Ticket ticket_;std::uintptr_t world_=0;bool mirroring_=false,prepareRefused_=false;
    unsigned pendingIndex_=Queue::Capacity;bool interrupted_=false;
    std::uint64_t pendingUntil_=0,pendingNext_=0;
    std::uint64_t watchNext_=0;bool watchSource_=false;
    bool StartedOwnership()const{for(unsigned i=0;i<recipientCount_;++i)if(startedMontages_[i])return true;return false;}
    void ClearQueued(){
        queue_.Clear();pendingIndex_=Queue::Capacity;
        interrupted_=false;pendingUntil_=pendingNext_=0;
    }
    void QueueOwnedCancellation(std::uint64_t now){
        ClearQueued();unsigned count=0;
        for(unsigned i=0;i<recipientCount_;++i)if(startedMontages_[i])queueIndices_[count++]=i;
        cancelling_=true;queue_.Start(count,now);
    }
    template<class A>static auto Interruptible(int)->decltype(std::declval<A&>().Interrupt(std::declval<const Recipient&>()),std::true_type{}){return {};}
    template<class A>static std::false_type Interruptible(long){return {};}
    template<class A>static auto InterruptStep(A& a,const Recipient& target,int)->decltype(a.Interrupt(target)){return a.Interrupt(target);}
    template<class A>static unsigned InterruptStep(A&,const Recipient&,long){return 1;}
    template<class A>static auto PlayReceipt(A& a,const Recipient& target,std::uintptr_t montage,int)->decltype(a.Played(target,montage),void()){a.Played(target,montage);}
    template<class A>static void PlayReceipt(A&,const Recipient&,std::uintptr_t,long){}
public:
    bool NeedsService(std::uint64_t now)const{return Due(now);} // compatibility only; new C native lane stays disabled
    Report report;
    bool IsPlayFunction(std::uintptr_t function)const{return report.prepared==2&&function==functions_[0].object;}
    bool InterruptPending(const Recipient& target)const{
        return pendingIndex_<recipientCount_&&pendingIndex_<Queue::Capacity&&
            recipients_[pendingIndex_].pawn==target.pawn&&recipients_[pendingIndex_].controller==target.controller;
    }
    bool Pending()const{return pendingIndex_<Queue::Capacity||queue_.Pending();}
    bool Due(std::uint64_t now)const{return pendingIndex_<Queue::Capacity?now>=pendingNext_:
        (queue_.Due(now)||(!queue_.Pending()&&watchSource_&&StartedOwnership()&&now>=watchNext_));}
    bool OwnsMontage(const Recipient& target,std::uintptr_t montage)const{
        if(!active_.Contains(montage))return false;
        for(unsigned i=0;i<recipientCount_;++i)if(startedMontages_[i]&&recipients_[i].pawn==target.pawn&&recipients_[i].controller==target.controller)return true;
        return false;
    }
    bool Candidate(std::uintptr_t object,std::uintptr_t function,bool nested)const{
        if(mirroring_||!object||report.prepared!=2||
           (function!=functions_[0].object&&function!=functions_[1].object))return false;
        if(!nested)return true;
        // Independently requested bot emotes revoke our start ownership even
        // when nested. Our own native dispatch is excluded by mirroring_.
        for(unsigned i=0;i<recipientCount_;++i)if(startedMontages_[i]&&recipients_[i].pawn.object==object)return true;
        return false;
    }
    template<class A> bool Prepare(A& a){
        if(report.prepared==2)return true;
        if(prepareRefused_)return false;
        if(!a.Bind(report.prepared,functions_[report.prepared],true)){prepareRefused_=true;report.failure=2;++report.refused;return false;}
        world_=a.Input().world;++report.prepared;return report.prepared==2;
    }
    bool ObservedPlay(std::uintptr_t object,std::uintptr_t function)const{
        return ticket_.valid&&!ticket_.id&&ticket_.source.object==object&&functions_[0].object==function;
    }
    template<class A> void Before(A& a,std::uintptr_t object,std::uintptr_t function,std::uintptr_t parameters,bool nested){
        if(!Candidate(object,function,nested))return;
        for(unsigned i=0;i<recipientCount_;++i)if(startedMontages_[i]&&recipients_[i].pawn.object==object){
            startedMontages_[i]=0;++report.ownershipLost;return;
        }
        if(nested)return;
        ticket_={};Slot source;
        if(a.Input().world!=world_||!a.Source(source)||source.object!=object)return;
        const unsigned id=function==functions_[1].object?1u:0u;
        if(!a.FunctionCurrent(id,functions_[id])||!a.Montage(object,ticket_.before)){report.failure=2;++report.refused;return;}
        ticket_.id=id;ticket_.source=source;
        if(!id){std::uintptr_t selected=0;
            if(!a.Parameter(parameters,selected)||!a.CatalogueFor(source,selected,ticket_.catalogue)){report.failure=3;++report.refused;return;}
        }
        ticket_.valid=true;
    }
    template<class A> void After(A& a,std::uintptr_t object,std::uintptr_t function,bool nested,bool completed,std::uint64_t now){
        if(nested||!ticket_.valid||object!=ticket_.source.object||function!=functions_[ticket_.id].object)return;
        const auto ticket=ticket_;ticket_={};Slot source;std::uintptr_t montage=0;
        if(!completed||!a.Source(source)||!(source==ticket.source)||!a.FunctionCurrent(ticket.id,functions_[ticket.id])||
           !a.Montage(source.object,montage)){report.failure=4;++report.refused;ClearQueued();return;}
        if(ticket.id){
            if(montage)return; // native cancellation did not complete
            ClearQueued();++report.cancels;Catalogue fresh;
            if(!(source==active_.source)||!a.CatalogueFor(source,active_.selected,fresh)||!fresh.SameAssets(active_))return;
            QueueOwnedCancellation(now);return;
        }
        Catalogue fresh;
        if(!montage||montage==ticket.before||!a.CatalogueFor(source,ticket.catalogue.selected,fresh)||
           !fresh.SameAssets(ticket.catalogue)||!fresh.Contains(montage)){report.failure=4;++report.refused;return;}
        active_=fresh;active_.montage=montage;
        std::copy_n(a.Input().bots.begin(),recipients_.size(),recipients_.begin());startedMontages_={};pendingIndex_=Queue::Capacity;
        recipientCount_=a.Input().count;cancelling_=false;
        if(recipientCount_>Queue::Capacity){ClearQueued();report.failure=5;++report.refused;recipientCount_=0;return;}
        for(unsigned i=0;i<recipientCount_;++i)queueIndices_[i]=i;
        ++report.starts;report.failure=0;
        // The real interruption-capable native path starts promptly and then
        // staggers recipients. Per-recipient pendingUntil remains3s; allow the
        // whole queue enough time for large valid rosters to reach that stage.
        constexpr auto spacing=decltype(Interruptible<A>(0))::value?250u:Queue::SpacingMs;
        watchSource_=decltype(Interruptible<A>(0))::value;watchNext_=now+250;
        if(queue_.Start(a.Input().count,now,spacing))report.queued+=a.Input().count;
    }
    template<class A> void Service(A& a,std::uint64_t now){
        // Finishing the start queue must not lose cancellation when the source
        // later exits its loop without another SERVER_CancelEmote event.
        if(!Pending()&&watchSource_&&StartedOwnership()){
            if(now<watchNext_)return;watchNext_=now+250;
            Slot source;Catalogue fresh;std::uintptr_t montage=0;
            if(a.Input().world!=world_||!a.Guard()||!a.Source(source)||!(source==active_.source)||
               !a.CatalogueFor(source,active_.selected,fresh)||!fresh.SameAssets(active_)||!a.Montage(source.object,montage)){
                startedMontages_={};report.failure=3;++report.refused;return; // identity loss forfeits cancel authority
            }
            if(fresh.Contains(montage)){active_.montage=montage;return;}
            ++report.cancels;QueueOwnedCancellation(now);return;
        }
        unsigned index=0;
        if(pendingIndex_<Queue::Capacity){
            if(now<pendingNext_)return;index=pendingIndex_;
            if(now>=pendingUntil_){pendingIndex_=Queue::Capacity;++report.skipped;return;}
            if(!interrupted_){
                const auto result=InterruptStep(a,recipients_[index],0);pendingNext_=now+25;
                if(result==2){pendingIndex_=Queue::Capacity;++report.skipped;return;}
                if(result==1)interrupted_=true;
                return; // native interruption NEVER shares the full play-dispatch budget
            }
            pendingIndex_=Queue::Capacity;
        }else{
            unsigned queuedIndex=0;if(!queue_.Take(now,queuedIndex))return;index=queueIndices_[queuedIndex];
            if constexpr(decltype(Interruptible<A>(0))::value){
                if(!cancelling_){
                    Slot source;Catalogue fresh;std::uintptr_t montage=0;
                    if(a.Input().world!=world_||!a.Source(source)||!(source==active_.source)||
                       !a.CatalogueFor(source,active_.selected,fresh)||!fresh.SameAssets(active_)||
                       !a.Montage(source.object,montage)){ClearQueued();return;}
                    if(!fresh.Contains(montage)){++report.cancels;QueueOwnedCancellation(now);return;}
                    active_.montage=montage;
                    pendingIndex_=index;pendingUntil_=now+3000;pendingNext_=now+25;interrupted_=false;return;
                }
            }
        }
        const auto ownedMontage=startedMontages_[index];
        if(cancelling_){ // consume ownership before any ambiguous native attempt
            startedMontages_[index]=0;
            if(!ownedMontage){++report.skipped;return;}
        }
        if(a.Input().world!=world_||!a.Guard()){ClearQueued();report.failure=1;++report.refused;return;}
        Slot source;Catalogue fresh;std::uintptr_t montage=0;
        if(!a.Source(source)||!(source==active_.source)||!a.CatalogueFor(source,active_.selected,fresh)||
           !fresh.SameAssets(active_)||!a.Montage(source.object,montage)){
            ClearQueued();report.failure=3;++report.refused;return;
        }
        if(cancelling_?fresh.Contains(montage):!fresh.Contains(montage)){
            if(cancelling_){ClearQueued();report.failure=3;++report.refused;}
            else {++report.cancels;QueueOwnedCancellation(now);}
            return;
        }
        if(!cancelling_)active_.montage=montage;
        const auto target=recipients_[index];
        if(target.pawn.object==source.object||index>=a.Input().count||
           !(target.pawn==a.Input().bots[index].pawn)||!(target.controller==a.Input().bots[index].controller)||
           !a.RecipientCurrent(target)){report.failure=5;++report.skipped;return;}
        const unsigned functionIndex=cancelling_?1u:0u;
        if(!a.FunctionCurrent(functionIndex,functions_[functionIndex])){ClearQueued();report.failure=2;++report.refused;return;}
        std::uintptr_t before=0;if(!a.Montage(target.pawn.object,before)){++report.skipped;return;}
        // A receipt remains owned through this emote's own loop transition.
        // Independently requested recipient emotes revoke it in Before().
        if(cancelling_&&!active_.Contains(before)){++report.skipped;return;}
        mirroring_=true;
        if(cancelling_)++report.cancelAttempts;else ++report.attempts;
        const bool returned=cancelling_?a.DispatchCancel(target,functions_[1]):a.Dispatch(target,functions_[0],active_.selected);
        mirroring_=false;
        if(!returned){if(!cancelling_)PlayReceipt(a,target,0,0);ClearQueued();report.failure=6;++report.refused;return;}
        if(cancelling_)++report.cancelReturns;else ++report.returns;report.failure=0;
        std::uintptr_t after=0;
        // Native playback may allocate/grow object storage. Refresh the owned
        // context before observing its result; a cached pre-call array is not
        // post-call identity proof.
        if(a.Guard()&&a.RecipientCurrent(target)&&a.Montage(target.pawn.object,after)){
            if(cancelling_){if(!after)++report.stopped;}
            else if(after==before)++report.startedUnchanged;
            else if(!active_.Contains(after))++report.startedOtherMontage;
            else {startedMontages_[index]=after;++report.started;}
        }else if(!cancelling_)++report.startedReadRefused;
        if(!cancelling_)PlayReceipt(a,target,startedMontages_[index],0); // verified NEW owned montage only, never an independent one
    }
};
}
