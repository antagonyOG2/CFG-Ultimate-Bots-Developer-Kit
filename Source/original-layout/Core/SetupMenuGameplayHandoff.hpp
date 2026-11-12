#pragma once
#include "SetupMenuModel.hpp"
#include "GameplayConfiguration.hpp"
#include "SoftClassPathIdentity.hpp"
#include "OfflineSettingsSelection.hpp"
#include "StartingLoadoutPolicy.hpp"
namespace cfgmenu {
// Pure adapter seam, confined to the menu/game-thread owner. Not a runtime
// validator: a CFG provider must produce these observations from pinned native
// metadata, owned catalogue entries and repeated identity observations.
// Tokens are stable provider catalogue identities, never dereferenced here.
struct MenuCatalogueEvidence {
    std::uint64_t frontend=0,generation=0,catalogueGeneration=0;
    std::uintptr_t frontendWorld=0,map=0,jason=0,counselor=0;
    unsigned maximumBots=0;
    cfgprimary::SoftClassBytes selectedJason{};
    bool cfgMetadata=false,ownedSelections=false;
    bool preserveNativeCounselor=false;
    bool counselorPathSelection=false;
    cfgprimary::SoftClassBytes selectedCounselor{};
    std::uint32_t loadoutSeed=0;
    cfgselectedjason::Choice selectedClass{};
};
struct NewWorldEvidence {
    std::uint64_t generation=0,catalogueGeneration=0,submission=0;
    std::uintptr_t map=0,jason=0,humanCounselor=0;
    cfgspawn::Identity identity{};
    unsigned selectedIndex=0xFFFFFFFF;
    cfgprimary::Report resources{};
    bool cfgMetadata=false,offlineSandbox=false,ownedSelections=false;
    // Provider must prove the fresh acknowledged/possessed pawn is the native
    // Sandbox-born counselor via current player-state/class ownership. A class
    // pointer, a delayed forced possession or a save-preview value is not proof.
    bool nativeCounselorBirthVerified=false;
    // Set only by the native arrival provider after twice checking the actual
    // born class's ancestry against the armed counselor choice (native variants).
    std::uintptr_t verifiedCounselorAncestor=0;
    std::uintptr_t verifiedCounselorChoice=0;
    // Explicit native OfflineBots admission, mutually exclusive with Sandbox.
    // Still requires EmptyPrimary: this is a pre-AI handoff, never adoption.
    bool nativeOffline=false;
    cfgselectedjason::Choice selectedClass{};
};
enum class HandoffState { Empty,Armed,Consumed,Refused };
enum class HandoffFailure : unsigned {None,NotArmed,Session,Submission,Metadata,Identity,Content,Human,Resources,SelectedJason,CatalogueEntry,DuplicatePath,Configuration};
class GameplayHandoff {
    Session& session_;
    HandoffState state_=HandoffState::Empty;
    MenuCatalogueEvidence source_{};
    Selection selection_{};
    std::uint64_t submission_=0,lastSubmission_=0;
    HandoffFailure failure_=HandoffFailure::None;
    cfgofflinesettings::Request settings_{};
    std::uintptr_t restartWorld_=0;std::int32_t restartSerial_=0;
public:
    explicit GameplayHandoff(Session& session):session_(session){}
    GameplayHandoff(const GameplayHandoff&)=delete;
    GameplayHandoff& operator=(const GameplayHandoff&)=delete;
    bool CanArm(const Session& session,const MenuCatalogueEvidence& e)const{
        // Never overwrite a pending submission. Replay state remains across Cancel.
        if(state_==HandoffState::Armed)return false;
        if(e.counselorPathSelection&&(e.preserveNativeCounselor||!softpath::Plain(e.selectedCounselor)||e.counselor!=softpath::Token(e.selectedCounselor)))return false;
        const bool explicitClass=e.selectedClass.Valid()&&e.selectedClass.identity.object==e.jason&&softpath::Empty(e.selectedJason);
        if(!e.selectedClass.Empty()&&!explicitClass)return false;
        Selection candidate{};std::uint64_t id=0;
        if(!e.cfgMetadata||!e.ownedSelections||!e.frontend||!e.frontendWorld||!e.catalogueGeneration||
            e.maximumBots>31||(!explicitClass&&!softpath::Plain(e.selectedJason))||
            !session.InspectCommitted(e.frontend,e.generation,candidate,id)||id<=lastSubmission_||
            candidate.mode!=Mode::Counselor||!candidate.map||candidate.map!=e.map||
            !candidate.jasonClass||candidate.jasonClass!=e.jason||
            candidate.preserveNativeCounselor!=e.preserveNativeCounselor||
            (candidate.preserveNativeCounselor?(candidate.counselorClass||e.counselor):
                (!candidate.counselorClass||candidate.counselorClass!=e.counselor))||
            !candidate.counselorCount||candidate.counselorCount-1>e.maximumBots)return false;
        return true;
    }
    bool Arm(const MenuCatalogueEvidence& e){
        if(!CanArm(session_,e))return false;
        Selection candidate{};std::uint64_t id=0;
        if(!session_.ConsumeStart(e.generation,candidate,id))return false;
        cfgloadout::Selection loadout;
        if(!cfgloadout::Commit({candidate.weaponEnabled?candidate.weapon:0u,candidate.inventoryEnabled?candidate.inventory:0u},
            e.loadoutSeed,e.loadoutSeed*1664525u+1013904223u,loadout))return false;
        candidate.weapon=loadout.weapon;candidate.inventory=loadout.inventory;
        source_=e;selection_=candidate;submission_=lastSubmission_=id;restartWorld_=0;restartSerial_=0;state_=HandoffState::Armed;failure_=HandoffFailure::None;return true;
    }
    bool Consume(const NewWorldEvidence& e,cfgbundle::Configuration& output){
        output={};output.submission=0; // Invalid on every failure, never stale success.
        settings_={};
        const auto refused=[&](HandoffFailure value){failure_=value;return false;};
        if(state_!=HandoffState::Armed)return refused(HandoffFailure::NotArmed);
        state_=HandoffState::Refused; // One arrival attempt; mismatch cannot arm a later world.
        if(session_.Generation()!=source_.generation||session_.Current()!=Stage::Started)return refused(HandoffFailure::Session);
        if(e.generation!=source_.generation||e.submission!=submission_||e.catalogueGeneration!=source_.catalogueGeneration)return refused(HandoffFailure::Submission);
        if(!e.cfgMetadata||e.offlineSandbox==e.nativeOffline||!e.ownedSelections)return refused(HandoffFailure::Metadata);
        if(!e.identity.Valid()||e.identity.world==source_.frontendWorld)return refused(HandoffFailure::Identity);
        if(restartWorld_&&(e.identity.world!=restartWorld_||e.identity.worldSerial!=restartSerial_))return refused(HandoffFailure::Identity);
        if(e.map!=selection_.map||e.jason!=selection_.jasonClass||!(e.selectedClass==source_.selectedClass))return refused(HandoffFailure::Content);
        if(!e.humanCounselor||!e.nativeCounselorBirthVerified||(!selection_.preserveNativeCounselor&&
            (source_.counselorPathSelection?e.verifiedCounselorChoice!=selection_.counselorClass:
             e.humanCounselor!=selection_.counselorClass&&e.verifiedCounselorAncestor!=selection_.counselorClass)))return refused(HandoffFailure::Human);
        if(!e.resources.EmptyPrimary()||!cfgprimary::SaneCatalogue(e.resources.catalogue))return refused(HandoffFailure::Resources);
        const bool explicitClass=source_.selectedClass.Valid();
        const bool indexed=e.selectedIndex<static_cast<unsigned>(e.resources.catalogue.count);
        if(!explicitClass&&!softpath::Empty(e.resources.selected)&&!softpath::Same(e.resources.selected,source_.selectedJason))return refused(HandoffFailure::SelectedJason);
        if((!indexed&&(!explicitClass||e.selectedIndex!=cfgselectedjason::NoCatalogueIndex))||
           (indexed&&!explicitClass&&!softpath::Same(e.resources.entries[e.selectedIndex],source_.selectedJason)))return refused(HandoffFailure::CatalogueEntry);
        // Reject duplicate class paths even when their transient caches differ.
        unsigned matches=0;
        if(indexed)for(int i=0;i<e.resources.catalogue.count;++i)
            if(softpath::Same(e.resources.entries[static_cast<unsigned>(i)],e.resources.entries[e.selectedIndex]))++matches;
        if(indexed&&matches!=1)return refused(HandoffFailure::DuplicatePath);
        cfgbundle::Configuration candidate;
        candidate.identity=e.identity;candidate.submission=submission_;
        // Match the released menu: "Counselors" is the TOTAL, including the
        // already-owned human. A total of one intentionally requests no bots.
        candidate.counselorBots=selection_.counselorCount-1;candidate.jasonCatalogueIndex=e.selectedIndex;
        if(indexed)candidate.selectedJason=e.resources.entries[e.selectedIndex];
        else candidate.selectedClass=e.selectedClass;
        candidate.matchSeconds=selection_.durationEnabled?(selection_.duration+1)*1800u:0u;
        candidate.nativeCounselorSkill=selection_.skillEnabled?(selection_.skill==0?1u:selection_.skill==1?0u:2u):3u;
        candidate.difficulty=selection_.difficulty;
        // Only the owned Counselor menu can submit these match-local choices.
        const auto practice=[&](RowId row,cfgpractice::Option option){
            if(selection_.toggles[static_cast<unsigned>(row)-7])candidate.counselorPractice|=option;
        };
        practice(RowId::PamelaMute,cfgpractice::PamelaMute);practice(RowId::GrabMute,cfgpractice::GrabMute);
        practice(RowId::MinimalHud,cfgpractice::MinimalHud);practice(RowId::HardMovement,cfgpractice::HardMovement);
        practice(RowId::BotSweater,cfgpractice::BotSweater);
        // Preserve the existing resource-loading path for small native items.
        // A weapon or firecrackers uses the donor's bounded loaded-class path.
        unsigned inventory=0;
        if(selection_.weapon||!restrictedinventory::WireChoice(selection_.inventory,inventory)){
            candidate.startingWeapon=selection_.weapon;candidate.startingInventory=selection_.inventory;
        }else candidate.restrictedInventory=inventory;
        if(!cfgbundle::MatchesConfiguration(candidate,e.identity,e.resources,source_.maximumBots))return refused(HandoffFailure::Configuration);
        settings_={submission_,e.identity.world,e.identity.pawn,e.identity.worldSerial,e.identity.pawnSerial,
            selection_.durationEnabled?(selection_.duration+1)*1800u:0u,
            selection_.skillEnabled?(selection_.skill==0?1u:selection_.skill==1?0u:2u):3u};
        if(!settings_.Valid()){settings_={};return refused(HandoffFailure::Configuration);}
        output=candidate;state_=HandoffState::Consumed;failure_=HandoffFailure::None;return true;
    }
    // Provider first proves a genuine native intro on a fresh same-map world.
    // Retain only the explicit submitted choices; Consume repeats ALL birth,
    // map, selected-Jason, roster and configuration checks for that world.
    bool RearmForRestart(std::uintptr_t prior,std::int32_t priorSerial,std::uintptr_t world,std::int32_t serial)noexcept{
        if(state_!=HandoffState::Consumed||!settings_.Valid()||settings_.world!=prior||settings_.worldSerial!=priorSerial||
           !world||serial<=0||(world==prior&&serial==priorSerial)||world==source_.frontendWorld||
           session_.Generation()!=source_.generation||session_.Current()!=Stage::Started)return false;
        restartWorld_=world;restartSerial_=serial;settings_={};state_=HandoffState::Armed;failure_=HandoffFailure::None;return true;
    }
    void Cancel()noexcept{state_=HandoffState::Empty;source_={};selection_={};submission_=0;settings_={};restartWorld_=0;restartSerial_=0;failure_=HandoffFailure::None;}
    bool ConsumedSettings(cfgofflinesettings::Request& out)const noexcept{
        out={};if(state_!=HandoffState::Consumed||!settings_.Valid())return false;out=settings_;return true;
    }
    HandoffState State()const noexcept{return state_;}
    HandoffFailure LastFailure()const noexcept{return failure_;}
    bool InspectArmed(MenuCatalogueEvidence& source,Selection& selection,std::uint64_t& submission)const noexcept{
        source={};selection={};submission=0;
        if(!ArmedFor(source_.frontend,source_.generation,source_.frontendWorld))return false;
        source=source_;selection=selection_;submission=submission_;return true;
    }
    bool ArmedFor(std::uint64_t frontend,std::uint64_t generation,std::uintptr_t world)const noexcept{
        return state_==HandoffState::Armed&&source_.frontend==frontend&&source_.generation==generation&&
            source_.frontendWorld==world&&session_.Generation()==generation&&session_.Current()==Stage::Started;
    }
    // Non-configuration choices are not emitted by this adapter; it does not
    // claim to apply weather/loadout/difficulty/ability modifiers.
};
}
