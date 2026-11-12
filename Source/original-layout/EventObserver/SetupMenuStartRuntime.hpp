#pragma once
#include "NativeSetupRows.hpp"
#include "../Core/SetupMenuGameplayHandoff.hpp"
namespace cfgmenu {
enum class StartDisposition {NativeRoute,ConsumedWaiting,TravelSubmitted,ConsumedDuplicate,Refused};
// Controller between native constructed controls and the existing configured
// AI backend. The native route provider supplies CFG catalogue/selection
// evidence and dispatches SANDBOX travel. A guarded native route may change
// only current PendingOfflineMode to the shipping Sandbox definition before
// forwarding original Start once. NEVER forward unchanged OfflineBots mode;
// that route assigns the human Jason's controller.
class SetupStart {
    Session& session_;GameplayHandoff& handoff_;rows::Runtime& rows_;
    bool claimed_=false,submitted_=false,failed_=false;
    std::uint64_t generation_=0,frontend_=0;
    static bool Apply(Session& session,std::uint64_t generation,RowId id,unsigned value){
        if(id==RowId::Count)return session.SetCount(generation,value+1);
        if(id==RowId::Weather)return false; // CFG weather uses native opaque identity, not stock indexes.
        if(id>=RowId::Knives&&id<RowId::End)return value<=1&&session.SetToggle(generation,id,value!=0);
        return session.SetChoice(generation,id,value);
    }
public:
    SetupStart(Session& session,GameplayHandoff& handoff,rows::Runtime& rows):session_(session),handoff_(handoff),rows_(rows){}
    bool Claim(std::uint64_t frontend,std::uint64_t generation){
        if(claimed_||!frontend||session_.Generation()!=generation||session_.Current()!=Stage::Settings)return false;
        claimed_=true;generation_=generation;frontend_=frontend;return true;
    }
    bool ClaimPreparing(std::uint64_t frontend,std::uint64_t generation){
        // Guard only: no selection, commit or handoff is granted here. The
        // paced frontend has not acquired native choices yet. Related Start
        // remains consumed waiting until real native rows reach Ready.
        if(claimed_||!frontend||session_.Generation()!=generation||session_.Current()!=Stage::Selecting)return false;
        claimed_=true;generation_=generation;frontend_=frontend;return true;
    }
    template<class Host> StartDisposition Start(Host& host){
        if(!claimed_)return StartDisposition::NativeRoute;
        // ProcessEvent and a class-local wrapper may observe the same Start.
        // After the first queued travel, both must CONSUME rather than letting
        // the duplicate invoke stock Jason-mode travel.
        if(submitted_)return StartDisposition::ConsumedDuplicate;
        if(failed_||session_.Generation()!=generation_||!host.Current(generation_))return StartDisposition::Refused;
        if(rows_.CurrentState()!=rows::State::Ready)return StartDisposition::ConsumedWaiting;
        std::array<unsigned,rows::Capacity> values;
        if(!rows_.Choices(host,generation_,values))return StartDisposition::Refused;
        auto candidate=session_; // Atomic: no partly updated draft on any rejected row.
        for(unsigned i=0;i<rows_.Count();++i){const auto id=rows_.At(i).spec.id;
#if CFG_H20_MENU
            // Online preferences belong only to the process-memory store.
            // They never become an offline configuration or launch request.
            if(id>=RowId::OnlineJason&&id<=RowId::OnlineFill)continue;
#endif
            // A one-value status row describes the already implemented AI
            // route; it is NOT a weather choice or a decorative gameplay toggle.
            if(rows_.At(i).spec.informationOnly)continue;
            if(id==RowId::Inventory){
                const auto& spec=rows_.At(i).spec;unsigned base=0;
                if(spec.count==Inventory.size()){
                    for(unsigned label=0;label<spec.count;++label){
                        if(spec.labels[label].size()!=Inventory[label].size())return StartDisposition::Refused;
                        for(unsigned ch=0;ch<spec.labels[label].size();++ch)
                            if(spec.labels[label][ch]!=static_cast<char16_t>(Inventory[label][ch]))return StartDisposition::Refused;
                    }
                    if(!candidate.SetChoice(generation_,id,values[i]))return StartDisposition::Refused;
                    continue;
                }
                if(spec.count!=restrictedinventory::Labels.size())return StartDisposition::Refused;
                for(unsigned label=0;label<spec.count;++label)
                    if(spec.labels[label]!=restrictedinventory::Labels[label])return StartDisposition::Refused;
                if(!restrictedinventory::BaseChoice(spec.labels[values[i]],base)||!candidate.SetChoice(generation_,id,base))return StartDisposition::Refused;
                continue;
            }
            if(id==RowId::Weather){
                std::uint64_t identity=0;if(!host.Weather(values[i],identity)||!candidate.SetWeather(generation_,identity,true))return StartDisposition::Refused;
            }else if(!Apply(candidate,generation_,id,values[i]))return StartDisposition::Refused;
        }
        // Native built-in selectors/rows are read at Start, never guessed from
        // loaded assets. Provider must confirm any settings absent from addons.
        if(!host.NativeChoices(candidate,generation_)||!candidate.Commit(frontend_,generation_))return StartDisposition::Refused;
        MenuCatalogueEvidence evidence;
        if(!host.Catalogue(evidence)||evidence.frontend!=frontend_||evidence.generation!=generation_||
            !handoff_.CanArm(candidate,evidence)||!host.Current(generation_))return StartDisposition::Refused;
        session_=candidate;
        if(!handoff_.Arm(evidence)){failed_=true;return StartDisposition::Refused;}
        submitted_=true; // Publish BEFORE potentially reentrant/native travel.
        if(!host.TravelSandbox(evidence)){
            // An ambiguous travel cannot be retried into a different world.
            failed_=true;handoff_.Cancel();return StartDisposition::Refused;
        }
        return StartDisposition::TravelSubmitted;
    }
    void Reset(){handoff_.Cancel();claimed_=submitted_=failed_=false;generation_=frontend_=0;}
};
}
