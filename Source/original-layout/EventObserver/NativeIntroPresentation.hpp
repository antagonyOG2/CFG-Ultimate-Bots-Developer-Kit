#pragma once
#include "NativeEndingSkin.hpp"
#include "../RuntimeProbe/OfflineIntroMetadataBindings.generated.hpp"
namespace cfgending::skin {
// Source witness taken while the human PlayerState is STILL the native intro
// owner and both borrowed class fields name the selected Jason. The current
// profile's PickedKillerClass is not that borrowed field. Retain it as an exact
// unchanged value, never infer that it selected this match's Jason.
class IntroPresentation {
    std::uintptr_t module_=0,world_=0,controller_=0,state_=0,value_=0,type_=0,sourceSkin_=0;
    cfgprobe::lifetime::SlotObservation slot_{};Packet selected_{},profile_{};
    bool ready_=false;
public:
    unsigned stage=0;std::uintptr_t observedSkin=0;
    bool Ready()const{return ready_;}
    template<class R>bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
        std::uintptr_t world,std::uintptr_t controller,const Packet& selected,std::uintptr_t& out)const{
        out=0;Packet profile{};cfgprobe::lifetime::SlotObservation slot{};
        if(!ready_||world!=world_||controller!=controller_||!SamePath(selected,selected_)||!Safe(r,module_)||
           !LiveSlot(r,registry,state_,slot)||!(slot==slot_)||!Same(r,state_+cfgprobe::ObjectClass,type_)||
           !Same(r,state_+0x88,controller_)||!Same(r,controller_+0x388,state_)||!Same(r,state_+0x110,std::uint8_t{3})||
           !r.Read(state_+disk::PickedClass,profile)||profile!=profile_||!Same(r,state_+disk::PickedSkin,sourceSkin_)||
           (!sourceSkin_&&(!Same(r,module_+disk::SCJasonSkinClass,value_)||!Same(r,module_+disk::SCJasonSkinConstructor,value_)))||
           !Concrete(r,module_,registry,value_)||!Same(r,state_+disk::PickedSkin,sourceSkin_)||
           !Same(r,state_+cfgprobe::ObjectClass,type_)||!Same(r,controller_+0x388,state_)||
           !LiveSlot(r,registry,state_,slot)||!(slot==slot_)||!Safe(r,module_))return false;
        out=value_;return true;
    }
    template<class R,class CurrentOwner>bool Capture(R& r,std::uintptr_t module,
        const cfgprobe::lifetime::ArrayHeader& registry,std::uintptr_t world,std::uintptr_t gameState,
        std::uintptr_t controller,std::uintptr_t state,const Packet& selected,CurrentOwner&& owner){
        using namespace cfgmenu::intrometadatadisk;
        if(stage)return false;stage=1;Packet active{},killer{},profile{};std::uintptr_t base=0;
        if(!owner()||!Safe(r,module)||!EmptySubpath(selected))return false;
        stage=2;if(!r.Read(state+ActiveCharacterClass,active)||!SamePath(active,selected)||
           !r.Read(gameState+KillerClass,killer)||!SamePath(killer,selected)||
           !Same(r,gameState+CurrentKillerPlayerState,state))return false;
        stage=3;if(!LiveSlot(r,registry,state,slot_)||!slot_.HasAssignedSerial()||
           !r.Read(module+disk::SCPlayerStateClass,base)||!Same(r,module+disk::SCPlayerStateConstructor,base)||
           !cfgspawn::ObjectIs(r,state,base)||!Same(r,state+0x88,controller)||!Same(r,controller+0x388,state)||
           !r.Read(state+cfgprobe::ObjectClass,type_)||!r.Read(state+disk::PickedClass,profile)||
           !r.Read(state+disk::PickedSkin,observedSkin))return false;
        sourceSkin_=observedSkin;
        // Donor CopyLifecycleKillerPresentation uses SCJasonSkin (native no
        // override) for an EMPTY offline profile. Resolve ONLY the exact
        // named native class cache pair; never load/fabricate an asset or
        // replace a non-null selected skin. Current re-proves this decision.
        stage=4;
        if(!sourceSkin_&&(!r.Read(module+disk::SCJasonSkinClass,observedSkin)||
           !Same(r,module+disk::SCJasonSkinConstructor,observedSkin)))return false;
        if(!Concrete(r,module,registry,observedSkin))return false;
        module_=module;world_=world;controller_=controller;state_=state;value_=observedSkin;selected_=selected;profile_=profile;ready_=true;
        stage=5;std::uintptr_t current=0;Packet again{};
        if(!owner()||!r.Read(state+ActiveCharacterClass,again)||again!=active||
           !r.Read(gameState+KillerClass,again)||again!=killer||!Same(r,gameState+CurrentKillerPlayerState,state)||
           !Current(r,registry,world,controller,selected,current)){ready_=false;return false;}
        stage=0;return true;
    }
};
}
