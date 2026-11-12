#pragma once
#include "StartingPlayerObservation.hpp"
#include "../RuntimeProbe/FreshGameplayObservation.hpp"
#include "../Core/SetupMenuGameplayHandoff.hpp"
#include <algorithm>
namespace cfgmenu::prebirth {
enum class State : unsigned {Empty,Applying,Applied,Refused,Indeterminate};
struct Diagnostic {unsigned stage=0,ownerStage=0,events=0,attempts=0,returns=0;std::uintptr_t world=0,playerState=0;};
// The accepted native asset name is process-local. Weak object caches are NOT
// carried across travel; plain packets own no FString/subobject allocation.
inline bool PacketForBirth(const cfgprimary::SoftClassBytes& selected,promotion::Packet& out)noexcept {
    out={};if(!softpath::Plain(selected))return false;
    const std::int32_t unassigned=-1;std::memcpy(out.bytes.data(),&unassigned,4);
    std::copy_n(selected.begin()+16,8,out.bytes.begin()+16);return true;
}
// Runs BEFORE the original, exactly identified HandleStartingNewPlayer callback.
// It sets only the native ActiveCharacterClass via CFG's consuming setter. The
// untouched original event performs first birth through its native restart and
// getter. No private vtable, pawn replacement, possession repair, class scan,
// profile write, fabricated FFrame or post-birth retries.
class AcceptedCounselorPrebirth {
    State state_=State::Empty;Diagnostic diagnostic_{};
public:
    State CurrentState()const noexcept{return state_;}
    const Diagnostic& Report()const noexcept{return diagnostic_;}
    template<class Reader,class Calls> bool Before(Reader& reader,Calls& calls,const starting::Event& event,
        const freshgame::Source& source,const GameplayHandoff& handoff) {
        if(state_!=State::Empty||!event.exactImageVerified||event.module!=source.module||event.thread!=source.thread)return false;
        MenuCatalogueEvidence choice;Selection selection;std::uint64_t submission=0;
        if(!handoff.InspectArmed(choice,selection,submission)||!choice.counselorPathSelection||
           choice.preserveNativeCounselor||choice.frontendWorld!=source.frontendWorld)return false;
        std::uintptr_t expected=0;
        if(!reader.Read(event.module+startingdisk::FunctionCache,expected)||!expected||expected!=event.function)return false;
        ++diagnostic_.events;
        // A bot/unrelated NewPlayer must not consume this one human selection.
        std::uintptr_t requested=0,local=0;
        if(!cfgprobe::ObjectPointer(event.parameters)||!reader.Read(event.parameters,requested)||
           !reader.Read(source.player+cfgprobe::PlayerController,local)||requested!=local)return false;
        state_=State::Applying;diagnostic_.stage=1;
        const auto refuse=[&](){state_=State::Refused;return false;};
        starting::Observation observed;
        if(!starting::Observe(reader,event,observed,&diagnostic_.ownerStage))return refuse();
        const auto& c=observed.owner;diagnostic_.world=c.world;diagnostic_.playerState=c.playerState;diagnostic_.stage=2;
        if(c.world==source.frontendWorld||c.engine!=source.engine||c.viewport!=source.viewport||
           c.player!=source.player||c.gameInstance!=source.gameInstance||calls.Thread()!=event.thread)return refuse();
        // The disk-proved Sandbox first-birth null-pawn branch uses THESE native
        // slots unchanged. Its character-cycling branch requires an existing
        // pawn and is explicitly excluded by Observe/OwnerCurrent.
        std::uintptr_t restart=0,getter=0,stateTable=0,canSpawn=0;
        if(!reader.Read(c.modeVtable+birthdisk::RestartSlot,restart)||restart!=c.module+birthdisk::Restart||
           !reader.Read(c.modeVtable+birthdisk::GetterSlot,getter)||getter!=c.module+birthdisk::Getter||
           !reader.Read(c.playerState,stateTable)||!cfgprobe::ObjectPointer(stateTable)||
           !reader.Read(stateTable+0x858,canSpawn)||canSpawn!=c.module+birthdisk::CanSpawnActive)return refuse();
        const auto nativeSlotsCurrent=[&](){std::uintptr_t table=0,slot=0;
            return reader.Read(c.playerState,table)&&table==stateTable&&reader.Read(table+0x858,slot)&&slot==canSpawn&&
                reader.Read(c.modeVtable+birthdisk::RestartSlot,slot)&&slot==restart&&
                reader.Read(c.modeVtable+birthdisk::GetterSlot,slot)&&slot==getter;};
        diagnostic_.stage=3;promotion::Packet packet;
        cfgnames::Words words{};cfgnames::Text name;cfgnames::Observation<Reader> names(reader,c.module);
        std::memcpy(&words,choice.selectedCounselor.data()+16,8);
        if(!PacketForBirth(choice.selectedCounselor,packet)||!names.Decode(words,name)||
           name.View().substr(0,28)!=u"/Game/Characters/Counselors/"||name.View().find(u'.')==std::u16string_view::npos)return refuse();
        diagnostic_.stage=4;
        MenuCatalogueEvidence again;Selection againSelection;std::uint64_t againSubmission=0;
        if(!handoff.InspectArmed(again,againSelection,againSubmission)||againSubmission!=submission||
           again.frontend!=choice.frontend||again.generation!=choice.generation||!again.counselorPathSelection||
           !softpath::Same(again.selectedCounselor,choice.selectedCounselor)||
           !promotion::gate::OwnerCurrent(reader,c,event.thread,c.modeVtable,true)||!starting::ActorTypes(reader,c)||
           !nativeSlotsCurrent()||!calls.Ready(c))return refuse();
        diagnostic_.stage=5;++diagnostic_.attempts;
        // Canonical plain packet has no borrowed pointers to free. Ownership
        // transfers at native call entry; never destroy it after an exception.
        if(!calls.Consume(c,packet)){state_=State::Indeterminate;return false;}
        ++diagnostic_.returns;diagnostic_.stage=6;
        promotion::Packet active;
        if(!handoff.InspectArmed(again,againSelection,againSubmission)||againSubmission!=submission||
           again.frontend!=choice.frontend||again.generation!=choice.generation||!again.counselorPathSelection||
           !softpath::Same(again.selectedCounselor,choice.selectedCounselor)||
           !promotion::gate::OwnerCurrent(reader,c,event.thread,c.modeVtable,true)||!starting::ActorTypes(reader,c)||!nativeSlotsCurrent()||
           !reader.Read(c.playerState+birthdisk::ActiveCharacterClass,active)||!softpath::Same(active.bytes,choice.selectedCounselor))return refuse();
        state_=State::Applied;diagnostic_.stage=0;return true;
    }
};
}
