#pragma once
#include "NativeEmoteMirror.hpp"
#include "../Core/J22FeatureFlags.hpp"
namespace cfgemotemirror {
// ACTIVE donor FJB236xx/23703 queue and UEI113/138 playback cadence. CFG3.5
// already owns its emote assets: observe the exact native human selection,
// never patch Wave/montages or start a latent asset loader.
class DonorRuntime {
    struct Ticket {bool valid=false;unsigned id=0;Slot source{};Catalogue assets{};std::uintptr_t before=0;};
    struct Playback {bool live=false;Recipient actor{};Catalogue assets{};std::uintptr_t montage=0,world=0;std::uint64_t at=0;unsigned proofRetries=0;};
    std::array<Function,2> functions_{};Ticket ticket_{};
    std::array<Recipient,Inputs::Capacity> recipients_{};
    std::array<Playback,Inputs::Capacity+1> playback_{}; // source plus all64 possible recipients
    Catalogue selected_{};std::uintptr_t world_=0;
    unsigned count_=0,cursor_=0,playbackCursor_=0;std::uint64_t next_=0,nextPlayback_=0;
    Slot lastSource_{};std::uintptr_t lastSelected_=0;std::uint64_t lastAt_=0;
    bool mirroring_=false,prepareRefused_=false;
    unsigned targetReadYields_=0;
    template<class A>static auto ReadExhausted(A& a,int)->decltype(a.ReadExhausted()){return a.ReadExhausted();}
    template<class A>static bool ReadExhausted(A&,long){return false;}
    template<class A>static auto BindClause(A& a,int)->decltype(a.BindClause()){return a.BindClause();}
    template<class A>static unsigned BindClause(A&,long){return 0;}
    template<class A>static auto BindReads(A& a,int)->decltype(a.BindReads()){return a.BindReads();}
    template<class A>static unsigned BindReads(A&,long){return 0;}
    template<class A>bool YieldReadProof(A& a,std::uint64_t now){
        if(!CFG_J23_EMOTE_READ_YIELD||!ReadExhausted(a,0))return false;
        if(targetReadYields_<3){++targetReadYields_;++readDeferrals;next_=now+50;}
        else {++budgetDropped;++report.skipped;++cursor_;targetReadYields_=0;}
        return true; // pre-dispatch read proof only, never native ambiguity
    }
    template<class A>static auto Identity(A& a,const Catalogue& saved,Catalogue& fresh,int)->decltype(a.IdentityFor(saved,fresh)){
        return a.IdentityFor(saved,fresh);
    }
    template<class A>static bool Identity(A& a,const Catalogue& saved,Catalogue& fresh,long){
        return a.CatalogueFor(saved.source,saved.selected,fresh)&&fresh.source==saved.source&&
            fresh.type==saved.type&&fresh.defaults==saved.defaults&&fresh.selected==saved.selected;
    }
    void ClearQueued(){count_=cursor_=0;next_=0;targetReadYields_=0;}
    void Track(const Recipient& actor,const Catalogue& assets,std::uintptr_t montage,std::uint64_t now){
        if(!montage||!assets.Contains(montage))return;
        Playback* empty=nullptr;
        for(auto& value:playback_){
            if(value.live&&value.actor.pawn==actor.pawn){value={true,actor,assets,montage,world_,now,0};return;}
            if(!value.live&&!empty)empty=&value;
        }
        if(empty)*empty={true,actor,assets,montage,world_,now,0};
    }
public:
    Report report{};
    unsigned playbackExpiries=0,deduplicated=0,worldCancels=0,cancelClears=0;std::uint64_t cancelClearMs=0; // H2.58: queue dropped by a no-montage cancel of the source
    unsigned readDeferrals=0,budgetDropped=0;
    std::array<unsigned,8> refusals{};
    const Catalogue& Selection()const noexcept{return selected_;}
    bool Pending()const noexcept{return cursor_<count_;}
    bool NeedsService(std::uint64_t now)const noexcept{
        if(report.prepared<2)return !prepareRefused_;
        if(Pending()&&now>=next_)return true;
        if(now<nextPlayback_)return false;
        for(const auto& value:playback_)if(value.live)return true;
        return false;
    }
    bool Due(std::uint64_t now)const noexcept{
        if(Pending()&&now>=next_)return true;
        if(now>=nextPlayback_)for(const auto& value:playback_)if(value.live&&now>=value.at&&now-value.at>=25000)return true;
        return false;
    }
    bool IsPlayFunction(std::uintptr_t function)const{return report.prepared==2&&function==functions_[0].object;}
    bool InterruptPending(const Recipient&)const{return false;} // donor performs no multi-turn pre-play brain interruption
    bool OwnsMontage(const Recipient& actor,std::uintptr_t montage)const{
        for(const auto& value:playback_)if(value.live&&value.actor.pawn==actor.pawn&&value.actor.controller==actor.controller&&value.montage==montage)return true;
        return false;
    }
    bool Candidate(std::uintptr_t object,std::uintptr_t function,bool nested)const{
        if(mirroring_||report.prepared!=2||!object||(function!=functions_[0].object&&function!=functions_[1].object))return false;
        if(!nested)return true;
        for(const auto& value:playback_)if(value.live&&value.actor.pawn.object==object)return true;
        return false;
    }
    template<class A>bool Prepare(A& a){
        if(report.prepared==2)return true;if(prepareRefused_)return false;
        if(!a.Bind(report.prepared,functions_[report.prepared],true)){
            report.functionBindClause=BindClause(a,0);report.functionBindReads=BindReads(a,0);
            prepareRefused_=true;report.failure=2;++report.refused;++refusals[2];return false;}
        world_=a.Input().world;++report.prepared;return report.prepared==2;
    }
    bool ObservedPlay(std::uintptr_t object,std::uintptr_t function)const{
        return ticket_.valid&&!ticket_.id&&object==ticket_.source.object&&function==functions_[0].object;
    }
    template<class A>bool CancelLocalForSelection(A& a,const Slot& pawn){
        for(auto& value:playback_)if(value.live&&value.actor.pawn==pawn&&pawn==selected_.source){
            const auto saved=value;Slot current;std::uintptr_t montage=0;
            if(!a.Source(current)||!(current==pawn)||!a.Guard()||!a.Montage(pawn.object,montage))return false;
            if(montage!=saved.montage){value={};++report.ownershipLost;return true;}
            if(!a.FunctionCurrent(1,functions_[1]))return false;
            value={};ClearQueued();mirroring_=true;++report.cancelAttempts;
            const bool returned=a.DispatchCancel(saved.actor,functions_[1]);mirroring_=false;
            if(returned)++report.cancelReturns;else{++report.refused;++refusals[6];}
            return returned; // an ambiguous cancellation is never replayed
        }
        return true; // do not cancel an unrelated native emote
    }
    template<class A>void Before(A& a,std::uintptr_t object,std::uintptr_t function,std::uintptr_t packet,bool nested){
        if(!Candidate(object,function,nested))return;
        for(auto& value:playback_)if(value.live&&value.actor.pawn.object==object){value={};++report.ownershipLost;}
        if(nested)return;ticket_={};Slot source;
        if(a.Input().world!=world_||!a.Source(source)||object!=source.object)return;
        const unsigned id=function==functions_[1].object?1u:0u;
        if(!a.FunctionCurrent(id,functions_[id])||!a.Montage(object,ticket_.before)){report.failure=2;report.failSite=1;report.failClause=BindClause(a,0);report.failBindReads=BindReads(a,0);++report.refused;++refusals[2];return;}
        ticket_.id=id;ticket_.source=source;
        if(!id){std::uintptr_t selected=0;
            if(!a.Parameter(packet,selected)||!a.CatalogueFor(source,selected,ticket_.assets)){report.failure=3;++report.refused;++refusals[3];return;}}
        ticket_.valid=true;
    }
    template<class A>void After(A& a,std::uintptr_t object,std::uintptr_t function,bool nested,bool completed,std::uint64_t now){
        if(nested||!ticket_.valid||object!=ticket_.source.object||function!=functions_[ticket_.id].object)return;
        const auto ticket=ticket_;ticket_={};Slot source;std::uintptr_t montage=0;
        if(!completed||a.Input().world!=world_||!a.Source(source)||!(source==ticket.source)||
           !a.FunctionCurrent(ticket.id,functions_[ticket.id])||!a.Montage(object,montage)){
            report.failure=4;++report.refused;++refusals[4];return;}
        if(ticket.id){++report.cancels;if(!montage){if(Pending()){++cancelClears;cancelClearMs=now>=lastAt_?now-lastAt_:0;}if(!CFG_H259_EMOTE_KEEP_QUEUE)ClearQueued();}return;}
        Catalogue fresh;
        if(!montage||montage==ticket.before||!a.CatalogueFor(source,ticket.assets.selected,fresh)||
           !fresh.SameAssets(ticket.assets)||!fresh.Contains(montage)){report.failure=4;++report.refused;++refusals[4];return;}
        if(lastSource_==source&&lastSelected_==fresh.selected&&now>=lastAt_&&now-lastAt_<200){++deduplicated;return;}
        lastSource_=source;lastSelected_=fresh.selected;lastAt_=now;selected_=fresh;selected_.montage=montage;
        ClearQueued();
        if(a.Input().count>Inputs::Capacity){report.failure=5;++report.refused;++refusals[5];return;}
        for(unsigned i=0;i<a.Input().count;++i){const auto actor=a.Input().bots[i];
            if(actor.pawn.object&&actor.pawn.object!=source.object)recipients_[count_++]=actor;}
        next_=now+25;++report.starts;report.queued+=count_;report.failure=0;
        Track({source,{}},selected_,montage,now);
    }
    template<class A>bool PumpPlayback(A& a,std::uint64_t now,bool cancelLocalForMovement=false){
        if(now<nextPlayback_)return false;nextPlayback_=now+50;
        if(a.Input().world!=world_){ClearQueued();playback_={};playbackCursor_=0;++worldCancels;return false;}
        unsigned inspected=0;
        while(inspected<playback_.size()){
            const unsigned index=playbackCursor_;playbackCursor_=(playbackCursor_+1)%static_cast<unsigned>(playback_.size());++inspected;
            auto& value=playback_[index];
            if(!value.live)continue;
            const bool local=value.actor.pawn==selected_.source;
            const bool expired=now>=value.at&&now-value.at>=25000;
            const bool moved=local&&cancelLocalForMovement&&now>=value.at&&now-value.at>=250;
            if(!expired&&!moved)continue;
            const auto saved=value;
            const auto defer=[&](){
                ++value.proofRetries;const unsigned shift=(std::min)(value.proofRetries,3u);
                nextPlayback_=now+(50u<<shift);return true; // bounded pre-entry proof; exact receipt remains owed
            };
            Slot source;bool current=false;
            if(local){
                if(a.Source(source)){
                    if(!(source==saved.actor.pawn)){value={};++report.ownershipLost;continue;} // positive new local pawn retires the source receipt
                    current=true;
                }
            }else current=a.RecipientCurrent(saved.actor);
            if(!current||!a.Guard()){defer();continue;}
            std::uintptr_t montage=0;if(!a.Montage(saved.actor.pawn.object,montage)){defer();continue;}
            if(montage!=saved.montage){value={};++report.ownershipLost;continue;}
            if(!a.FunctionCurrent(1,functions_[1])){++report.refused;++refusals[2];defer();continue;}
            value={}; // consume only after every fresh proof and immediately before native entry
            mirroring_=true;++report.cancelAttempts;
            const bool returned=a.DispatchCancel(saved.actor,functions_[1]);mirroring_=false;
            if(returned){++report.cancelReturns;if(expired)++playbackExpiries;}else{++report.refused;++refusals[6];}
            return true;
        }
        return false;
    }
    template<class A>void Service(A& a,std::uint64_t now,bool localMovement=false){
        if(a.Input().world!=world_){ClearQueued();playback_={};playbackCursor_=0;++worldCancels;return;}
        if(PumpPlayback(a,now,localMovement))return;
        if(!Pending()||now<next_)return;next_=now+25;
        const auto target=recipients_[cursor_]; // one bot/25ms, never drain a catch-up burst
        if(!a.Guard()||!a.RecipientCurrent(target)){
            if(YieldReadProof(a,now))return;
            ++cursor_;targetReadYields_=0;++report.skipped;++refusals[5];return;}
        Catalogue fresh;
        // Donor queue is world-bound, not tied to the human remaining alive or
        // possessed after the selection. Reprove its original class/CDO/assets
        // from the same source birth; never adopt a respawn/new selection.
        if(!Identity(a,selected_,fresh,0)){
            if(YieldReadProof(a,now))return;
            ClearQueued();report.failure=3;++report.refused;++refusals[3];return;}
        if(!a.FunctionCurrent(0,functions_[0])){
            if(YieldReadProof(a,now))return;
            ClearQueued();report.failure=2;report.failSite=2;report.failClause=BindClause(a,0);report.failBindReads=BindReads(a,0);++report.refused;++refusals[2];return;}
        std::uintptr_t before=0;if(!a.Montage(target.pawn.object,before)){
            if(YieldReadProof(a,now))return;
            ++cursor_;targetReadYields_=0;++report.skipped;return;}
        ++cursor_;targetReadYields_=0; // consume BEFORE the first native effect
        mirroring_=true;++report.attempts;
        const bool returned=a.Dispatch(target,functions_[0],selected_.selected);mirroring_=false;
        if(!returned){ClearQueued();report.failure=6;++report.refused;++refusals[6];return;}
        ++report.returns;std::uintptr_t after=0;
        if(a.Guard()&&a.RecipientCurrent(target)&&a.Montage(target.pawn.object,after)&&after!=before&&selected_.Contains(after)){
            ++report.started;Track(target,selected_,after,now);
        }
        report.failure=0;
    }
};
}
