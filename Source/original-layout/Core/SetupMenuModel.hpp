#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include "RestrictedInventoryMenu.hpp"
#include "J214FeatureFlags.hpp"
#include "H20FeatureFlags.hpp"
namespace cfgmenu {
// Exact labels/options from frozen OfflineSetup.cpp and ChallengeNativeUi.cpp.
// CFG's cooked widgets supply the font, spacing, arrows, highlight and background.
// Do not transplant base-game cooked widgets or use base-game property offsets.
constexpr std::wstring_view CounselorEntry=L"Offline Bots - Play as a Counselor";
constexpr std::wstring_view JasonEntry=L"Offline Bots - Play as Jason";
constexpr std::wstring_view ChallengesHeader=L"OFFLINE CHALLENGES SETUP";
enum class Mode : unsigned { Counselor=1, Jason=2, Challenges=4 };
enum class RowId : unsigned { Skill,Count,Weather,Difficulty,Duration,Weapon,Inventory,
    Knives,Recharge,Movement,Stealth,Untimed,SkipReplay,PreventEscapes,Boundary,
#if CFG_J36_CHALLENGE_WALLS
    EdgeWalls,
#endif
    PamelaMute,GrabMute,MinimalHud,HardMovement,BotSweater,
#if CFG_H20_MENU
    OnlineJason,OnlineDifficulty,OnlineFill,OnlineJasonCount,
#endif
    End };
constexpr unsigned RowCount=static_cast<unsigned>(RowId::End);
struct Row { RowId id;std::wstring_view title;unsigned modes;bool native; };
constexpr std::array<Row,RowCount> Rows{{
    {RowId::Skill,L"Counselor Bot Skill",3,true},
    {RowId::Count,L"Counselors",3,true},
    {RowId::Weather,L"Weather",3,false},
    {RowId::Difficulty,L"Jason Difficulty",3,false},
    {RowId::Duration,L"Match Length",3,false},
    {RowId::Weapon,L"Starting Weapon",1,false},
    {RowId::Inventory,L"Starting Inventory",1,false},
    {RowId::Knives,L"Unlimited Throwing Knives",6,false},
    {RowId::Recharge,L"Faster Ability Recharge",6,false},
    {RowId::Movement,L"Faster Jason Movement (2x)",6,false},
    {RowId::Stealth,L"Stealth Practice (Low Detection)",4,false},
    {RowId::Untimed,L"No Match Timeout",4,false},
    {RowId::SkipReplay,L"Skip Intro on Replay",4,false},
    {RowId::PreventEscapes,L"Prevent Counselor Escapes",4,false},
    {RowId::Boundary,L"Disable Jason Boundary",4,false}
#if CFG_J36_CHALLENGE_WALLS
    ,{RowId::EdgeWalls,L"Disable Outer Blocking Walls",4,false}
#endif
    ,{RowId::PamelaMute,L"Mute Pamela (keep sweater voice)",1,false}
    ,{RowId::GrabMute,L"Mute Distant Bot Grab Sounds",1,false}
    ,{RowId::MinimalHud,L"Minimal HUD (keep objectives)",1,false}
    ,{RowId::HardMovement,L"Hard AI Jason Movement (2x)",1,false}
    ,{RowId::BotSweater,L"Bot Sweater Starts Convergence",1,false}
#if CFG_H20_MENU
    ,{RowId::OnlineJason,L"Online: Jason Bot",1,false}
    ,{RowId::OnlineDifficulty,L"Online: Jason Bot Difficulty",1,false}
    ,{RowId::OnlineFill,L"Online: Fill Empty Slots with Counselor Bots",1,false}
    ,{RowId::OnlineJasonCount,L"Online: Number of Jasons",1,false}
#endif
}};
constexpr std::array<std::wstring_view,3> Skill{{L"Low",L"Medium",L"High"}};
constexpr std::array<std::wstring_view,3> Difficulty{{L"Easy",L"Normal",L"Hard"}};
constexpr std::array<std::wstring_view,3> BaseWeather{{L"Off",L"Rain",L"Random"}};
constexpr std::array<std::wstring_view,4> Duration{{L"30 minutes",L"60 minutes",L"90 minutes",L"Unlimited"}};
constexpr std::array<std::wstring_view,5> Weapon{{L"None",L"Random",L"Machete",L"Axe",L"Shotgun"}};
constexpr std::array<std::wstring_view,6> Inventory{{L"None",L"Random",L"Pocket Knife",L"First Aid Spray",L"Firecrackers",L"Map"}};
constexpr std::array<std::wstring_view,2> Toggle{{L"Off",L"On"}};
struct Options {
    const std::wstring_view* data=nullptr;unsigned count=0,initial=0;
};
inline Options BaseOptions(RowId id,Mode mode){
    switch(id){
    case RowId::Skill:return {Skill.data(),3,1}; // Native donor may retain a saved choice.
    case RowId::Difficulty:return {Difficulty.data(),3,1};
    case RowId::Weather:return {BaseWeather.data(),3,2}; // CFG adapter uses CFG native weather instead.
    case RowId::Duration:return {Duration.data(),mode==Mode::Counselor?3u:4u,1};
    case RowId::Weapon:return {Weapon.data(),5,0};
    case RowId::Inventory:return {Inventory.data(),6,0};
    default:if(id>=RowId::Knives&&id<RowId::End)return {Toggle.data(),2,0};return {};
    }
}
// Content comes from CFG's validated native pickers/catalogues. Do not replace
// CFG's maps/Jasons/counselors/weather with the stock base-game six-map list.
struct Capabilities {std::array<bool,RowCount> verified{};bool automaticJason=false;};
inline bool Available(const Row& row,Mode mode,const Capabilities& caps){
    if(CFG_J216_FINISHING&&(row.id==RowId::HardMovement||row.id==RowId::BotSweater))return false;
    return (row.modes&static_cast<unsigned>(mode))&&caps.verified[static_cast<unsigned>(row.id)];
}
struct Selection {
    Mode mode=Mode::Counselor;
    std::uintptr_t map=0,jasonClass=0,counselorClass=0;
    unsigned counselorCount=7,skill=1,difficulty=1,duration=1,weapon=0,inventory=0;
    bool durationEnabled=false,skillEnabled=false,inventoryEnabled=false,weaponEnabled=false;
    // Weather is CFG native choice identity/index, not the base weather enum.
    std::uint64_t weatherIdentity=0;
    std::array<bool,RowCount-7> toggles{};
    // Native Sandbox may finish customization selection at pawn birth. This
    // explicit policy is NOT a guessed class or permission to repair possession.
    bool preserveNativeCounselor=false;
};
enum class Stage : unsigned { Dormant,Selecting,Settings,Committed,Started };
class Session {
    Stage stage_=Stage::Dormant;std::uint64_t frontend_=0,submission_=0,generation_=0;
    Capabilities caps_{};Selection draft_{};unsigned maximumBots_=0;
public:
    bool Open(std::uint64_t frontend,Mode mode,const Capabilities& caps,unsigned maximumBots){
        Reset(); // A refused new route must not leave an old Start submission armed.
        if(!frontend||maximumBots<1||maximumBots>32||
            (mode!=Mode::Counselor&&mode!=Mode::Jason&&mode!=Mode::Challenges))return false;
        frontend_=frontend;caps_=caps;maximumBots_=maximumBots;draft_.mode=mode;
        draft_.durationEnabled=Available(Rows[static_cast<unsigned>(RowId::Duration)],mode,caps);
        draft_.skillEnabled=Available(Rows[static_cast<unsigned>(RowId::Skill)],mode,caps);
        draft_.inventoryEnabled=Available(Rows[static_cast<unsigned>(RowId::Inventory)],mode,caps);
        draft_.weaponEnabled=Available(Rows[static_cast<unsigned>(RowId::Weapon)],mode,caps);
        if(draft_.counselorCount>maximumBots_)draft_.counselorCount=maximumBots_;
        stage_=Stage::Selecting;return true;
    }
    bool Pick(std::uint64_t frontend,std::uint64_t generation,std::uintptr_t map,std::uintptr_t jason,std::uintptr_t counselor){
        if(stage_!=Stage::Selecting||frontend!=frontend_||generation!=generation_||!map||!jason||
            (draft_.mode==Mode::Counselor&&!counselor))return false;
        draft_.map=map;draft_.jasonClass=jason;draft_.counselorClass=counselor;stage_=Stage::Settings;return true;
    }
    bool PickNativeCustomization(std::uint64_t frontend,std::uint64_t generation,std::uintptr_t map,std::uintptr_t jason){
        if(stage_!=Stage::Selecting||frontend!=frontend_||generation!=generation_||draft_.mode!=Mode::Counselor||!map||!jason)return false;
        draft_.map=map;draft_.jasonClass=jason;draft_.counselorClass=0;draft_.preserveNativeCounselor=true;
        stage_=Stage::Settings;return true;
    }
    bool Repick(std::uint64_t frontend,std::uint64_t generation,std::uintptr_t map,std::uintptr_t jason,std::uintptr_t counselor,bool preserveNativeCounselor){
        if(stage_!=Stage::Settings||frontend!=frontend_||generation!=generation_||draft_.mode!=Mode::Counselor||!map||!jason||
            (preserveNativeCounselor?counselor!=0:counselor==0))return false;
        draft_.map=map;draft_.jasonClass=jason;draft_.counselorClass=counselor;draft_.preserveNativeCounselor=preserveNativeCounselor;return true;
    }
    bool SetToggle(std::uint64_t generation,RowId id,bool value){
        const auto at=static_cast<unsigned>(id);if(generation!=generation_||stage_!=Stage::Settings||at<7||at>=RowCount||!Available(Rows[at],draft_.mode,caps_))return false;
        draft_.toggles[at-7]=value;return true;
    }
    bool SetCount(std::uint64_t generation,unsigned value){
        if(generation!=generation_||stage_!=Stage::Settings||!Available(Rows[1],draft_.mode,caps_)||!value||value>maximumBots_)return false;
        draft_.counselorCount=value;return true;
    }
    bool SetChoice(std::uint64_t generation,RowId id,unsigned value){
        const auto at=static_cast<unsigned>(id);
        if(generation!=generation_||stage_!=Stage::Settings||at>=RowCount||!Available(Rows[at],draft_.mode,caps_))return false;
        const auto options=BaseOptions(id,draft_.mode);
        if(!options.count||value>=options.count)return false;
        switch(id){
        case RowId::Skill:draft_.skill=value;break;
        case RowId::Difficulty:draft_.difficulty=value;break;
        case RowId::Duration:draft_.duration=value;break;
        case RowId::Weapon:draft_.weapon=value;break;
        case RowId::Inventory:draft_.inventory=value;break;
        default:return false; // Native weather identities use their own checked path.
        }
        return true;
    }
    bool SetWeather(std::uint64_t generation,std::uint64_t identity,bool nativeCatalogueVerified){
        if(generation!=generation_||stage_!=Stage::Settings||!identity||!nativeCatalogueVerified||
            !Available(Rows[2],draft_.mode,caps_))return false;
        draft_.weatherIdentity=identity;return true;
    }
    bool Commit(std::uint64_t frontend,std::uint64_t generation){
        if(stage_!=Stage::Settings||frontend!=frontend_||generation!=generation_||!draft_.map||!draft_.jasonClass||
            !draft_.counselorCount||draft_.counselorCount>maximumBots_||
            (draft_.mode==Mode::Counselor&&!caps_.automaticJason))return false;
        stage_=Stage::Committed;++submission_;return true;
    }
    // Native menu Start consumes exactly once. No extra F8/manual spawn after
    // travel. The eventual CFG adapter must validate native classes/ownership
    // and associate this submission with the new world before actor creation.
    bool ConsumeStart(std::uint64_t generation,Selection& result,std::uint64_t& submission){
        if(generation!=generation_||stage_!=Stage::Committed)return false;result=draft_;submission=submission_;stage_=Stage::Started;return true;
    }
    bool InspectCommitted(std::uint64_t frontend,std::uint64_t generation,Selection& result,std::uint64_t& submission)const{
        if(frontend!=frontend_||generation!=generation_||stage_!=Stage::Committed)return false;
        result=draft_;submission=submission_;return true;
    }
    void Reset(){++generation_;stage_=Stage::Dormant;frontend_=0;caps_={};draft_={};maximumBots_=0;}
    std::uint64_t Generation()const{return generation_;}
    Stage Current()const{return stage_;}
};
// Adapter contract: create the same native combo CLASS as the donor, append to
// the existing VerticalBox, attach before SetOptions, then label using owned
// native FText copying. Never clear the panel or copy raw FText/Slate pointers.
template<class Host> bool AppendRows(Host& host,Mode mode,const Capabilities& caps){
    for(const auto& row:Rows){if(row.native||!Available(row,mode,caps))continue;
        if(!host.CreateNativeDonorClass(row.id)||!host.AttachToExistingPanel(row.id)||
            !host.CopyOptionsAfterConstruction(row.id)||!host.SetOwnedText(row.id,row.title)){
            // Only remove children owned by this attempt; leave native rows intact.
            host.RollbackOwnedRows();return false;
        }
    }
    return true;
}
}
