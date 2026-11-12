#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include "GameplayLimits.hpp"
namespace cfgconvergence {
constexpr unsigned MaximumBots=cfglimits::MaximumHelpers,KeyCount=9;
enum Key : unsigned { Weapon,Jason,Fight,Armed,Melee,SeekWeapon,Flee,Hide,Orient };
constexpr std::array<const char16_t*,KeyCount> KeyText{{u"SCWeapon",u"JasonCharacter",u"ShouldFightBack",
    u"ShouldArmedFightBack",u"ShouldMeleeFightBack",u"SeekWeaponWhileFleeing",u"ShouldFleeKiller",
    u"ShouldHide",u"ShouldOrientTowardKiller"}};
enum class Action : unsigned { None, Publish, Clear };
// Donor keeps native melee eligible throughout trance and the death kneel.
// Only a COMMITTED final interaction (not merely an accepted first hit) owns
// all helpers' cleanup. Killer movement remains exclusively blocked separately.
inline bool CombatAllowed(bool finalCommitted,bool completion,bool deathConfirmed){
    return !finalCommitted&&!completion&&!deathConfirmed;
}
// Both features share a scarce paced callback lane. A due500ms sweater poll
// cannot consume every callback when higher-priority lanes leave >500ms gaps.
struct FeatureTurns {
    enum class Work { Other,Sweater,Helpers };
    bool sweaterNext=true,damageNext=true;
    bool Damage(){const bool result=damageNext;damageNext=!damageNext;return result;}
    bool Sweater(){const bool result=sweaterNext;sweaterNext=!sweaterNext;return result;}
    Work Choose(bool sweaterDue,bool helpersActive){
        // Sweater polling must stay eligible AFTER convergence activates.
        // The same chooser is used by the production fresh-budget branches.
        if(sweaterDue&&Sweater())return Work::Sweater;
        return helpersActive?Work::Helpers:Work::Other;
    }
};
// Finish an already-started blackboard transaction in otherwise idle slices.
// Do not speed up the donor's bot-selection/refresh cadence or run a burst of
// groups in one callback. Gameplay, release, camera and emote work win first.
struct PublicationContinuation {
    std::uint64_t next=0;
    bool yieldedCar=false;
    bool Take(std::uint64_t now,bool active,bool idle,bool acceptedCarPath=false){
        if(!active){next=0;yieldedCar=false;return false;}
        if((!idle&&!acceptedCarPath)||now<next||now>UINT64_MAX-100)return false;
        // Only a retained, noninteractive moving-car path may share a turn.
        // Alternate: the deferred vehicle gets the next turn, never a burst
        // or interruption of a hood/seat/Morph/release obligation.
        if(!idle&&acceptedCarPath){if(yieldedCar){yieldedCar=false;return false;}yieldedCar=true;}
        next=now+100;return true;
    }
};
// The donor's armed-only rule is important: forcing fight + seek-weapon on an
// unarmed pawn causes continuous native loot/flee replanning. Never do that.
struct BotState { bool published=false,unsupported=false;std::uint64_t refreshAt=0,retryAt=0; };
struct Policy {
    bool active=false;
    unsigned cursor=0;
    std::uint64_t probeAt=0,publishAt=0;
    std::array<BotState,MaximumBots> bots{};
    bool ProbeDue(std::uint64_t now){
        if(active||now<probeAt)return false;probeAt=now+211;return true;
    }
    void Acquired(){active=true;}
    bool Pick(std::uint64_t now,unsigned count,unsigned& index){
        if(!active||!count||count>MaximumBots||now<publishAt)return false;
        publishAt=now+333;index=cursor%count;cursor=(index+1)%count;return true;
    }
    Action Decide(unsigned index,bool alive,bool armed,std::uint64_t now)const{
        if(index>=MaximumBots||!active||!alive||bots[index].unsupported)return Action::None;
        if(now<bots[index].retryAt)return Action::None;
        if(!armed)return bots[index].published?Action::Clear:Action::None;
        if(bots[index].published&&now<bots[index].refreshAt)return Action::None;
        return Action::Publish;
    }
    void Confirm(unsigned index,Action action,std::uint64_t now){
        if(index>=MaximumBots||action==Action::None)return;
        bots[index].published=action==Action::Publish;bots[index].refreshAt=now+333;bots[index].retryAt=0;
    }
    void Refuse(unsigned index){if(index<MaximumBots)bots[index].unsupported=true;}
    void Retry(unsigned index,std::uint64_t now){if(index<MaximumBots)bots[index].retryAt=now+5000;}
    void Continue(unsigned index,std::uint64_t now){if(index<MaximumBots){cursor=index;publishAt=now+250;}}
};
inline bool Desired(Key key,Action action){
    const bool fighting=action==Action::Publish;
    return key==SeekWeapon||key==Flee?!fighting:key==Fight||key==Armed||key==Melee||key==Orient?fighting:false;
}
struct Name {std::int32_t index=0,number=0;};
struct ObjectPacket {Name key{};std::uintptr_t value=0;};
struct BoolPacket {Name key{};std::uint8_t value=0;std::array<std::uint8_t,7> padding{};};
struct String {std::uintptr_t data=0;std::int32_t count=0,capacity=0;};
struct NamePacket {String input{};Name result{};};
static_assert(sizeof(Name)==8&&sizeof(ObjectPacket)==16&&sizeof(BoolPacket)==16&&sizeof(NamePacket)==24);
static_assert(offsetof(ObjectPacket,value)==8&&offsetof(BoolPacket,value)==8&&offsetof(NamePacket,result)==16);
} // namespace cfgconvergence
