#pragma once
#include <cstdint>
#include <string_view>
namespace cfgloadout {
// Same choices and ordering as the released base menu. No engine calls here.
// The CFG adapter must supply a current owned class, not a captured pointer.
enum class Slot : unsigned {Weapon,Inventory,Complete};
enum class State : unsigned {Empty,Waiting,Verify,Complete,Cancelled};
constexpr std::uint64_t InitialDelayMs=3000,PhaseGapMs=1000;
constexpr unsigned MaximumReadinessAttempts=15;
inline std::u16string_view ClassName(Slot slot,unsigned choice) noexcept {
    if(slot==Slot::Weapon){switch(choice){
        case 2:return u"CounselorMachete_C";case 3:return u"CounselorTwoHandedAxe_C";case 4:return u"Shotgun_C";default:return {};}}
    if(slot==Slot::Inventory){switch(choice){
        case 2:return u"BP_PocketKnife_C";case 3:return u"BP_FirstAid_C";
        case 4:return u"BP_Firecracker_C";case 5:return u"Map_C";default:return {};}}
    return {};
}
struct Selection {unsigned weapon=0,inventory=0;};
inline bool Valid(const Selection& value) noexcept{return value.weapon<5&&value.inventory<6;}
// Resolve Random once at commit, never on a tick or failed grant. The caller
// supplies an existing random value; this model has no RNG/thread ownership.
inline bool Commit(Selection input,std::uint32_t weaponRandom,std::uint32_t inventoryRandom,Selection& out) noexcept {
    out={};if(!Valid(input))return false;
    if(input.weapon==1)input.weapon=2+weaponRandom%3;
    if(input.inventory==1)input.inventory=2+inventoryRandom%4;
    out=input;return true;
}
class Schedule {
    State state_=State::Empty;Slot slot_=Slot::Weapon;Selection selection_{};
    std::uint64_t submission_=0,next_=0;unsigned attempts_=0;bool servicing_=false;
    unsigned Choice()const noexcept{return slot_==Slot::Weapon?selection_.weapon:selection_.inventory;}
    void Advance() noexcept {
        attempts_=0;slot_=slot_==Slot::Weapon?Slot::Inventory:Slot::Complete;
        if(slot_==Slot::Inventory&&!selection_.inventory)slot_=Slot::Complete;
        state_=slot_==Slot::Complete?State::Complete:State::Waiting;
    }
public:
    bool Begin(std::uint64_t submission,Selection committed,std::uint64_t now) noexcept {
        if(state_!=State::Empty||!submission||!Valid(committed)||committed.weapon==1||committed.inventory==1)return false;
        submission_=submission;selection_=committed;next_=now+InitialDelayMs;
        slot_=committed.weapon?Slot::Weapon:committed.inventory?Slot::Inventory:Slot::Complete;
        state_=slot_==Slot::Complete?State::Complete:State::Waiting;return true;
    }
    template<class Host> bool Service(Host& host,std::uint64_t now){
        if(servicing_||(state_!=State::Waiting&&state_!=State::Verify)||now<next_)return false;
        struct Guard {bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}}guard(servicing_);
        // No catch-up bursts. One readiness/dispatch OR verification per gap.
        next_=now+PhaseGapMs;
        if(!host.Current(submission_)){state_=State::Cancelled;return false;}
        if(state_==State::Verify){host.Verify(slot_,Choice());Advance();return true;}
        if(++attempts_>MaximumReadinessAttempts){host.Unavailable(slot_,Choice());Advance();return true;}
        if(!host.Ready(slot_,Choice(),ClassName(slot_,Choice())))return false;
        // Publish BEFORE invocation: no duplicate/reentrant grant, including
        // an exception or an ambiguous native return. Do not retry side effects.
        state_=State::Verify;host.Grant(slot_,Choice());return true;
    }
    State CurrentState()const noexcept{return state_;}
    Slot CurrentSlot()const noexcept{return slot_;}
    unsigned Attempts()const noexcept{return attempts_;}
};
}
