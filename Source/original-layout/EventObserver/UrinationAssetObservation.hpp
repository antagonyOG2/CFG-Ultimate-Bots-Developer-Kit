#pragma once
#include "UrinationBindings.generated.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
namespace cfgurination {
// Caller already holds CURRENT retained class/original asset handles inside a
// proved registry window. This is a fresh metadata proof, not lifetime authority.
struct AssetProof {
    cfgprobe::lifetime::SlotObservation wave{},defaults{},male{},female{},skeleton{},loaded{};
};
template<class R>bool ObserveAssets(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
    const std::array<std::uintptr_t,3>& roots,std::uintptr_t loaded,AssetProof& out,unsigned published=0){
    out={};AssetProof value{};cfgnames::Observation<R> names(r,module);
    if(published>3||(published&&!loaded)||((published&2)&&!roots[2]))return false;
    const auto same=[&](std::uintptr_t address,std::uintptr_t expected){std::uintptr_t actual=0;return r.Read(address,actual)&&actual==expected;};
    const auto named=[&](std::uintptr_t object,std::u16string_view expected){cfgnames::Text text;return names.Object(object,text)&&text.View()==expected;};
    const auto native=[&](std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& type){
        std::uintptr_t a=0,b=0;cfgprobe::lifetime::SlotObservation slot{};
        return r.Read(module+reflection,a)&&r.Read(module+constructor,b)&&(type=cfgprobe::MatchingClassCache(a,b))&&
            cfgspawn::ObjectSlot(r,objects,type,slot);
    };
    std::uintptr_t emoteType=0,montageType=0,skeletonType=0,defaults=0,male=0,female=0,skeleton=0,other=0;
    if(!roots[0]||!roots[1]||!native(disk::SCEmoteDataReflection,disk::SCEmoteDataConstructor,emoteType)||
        !native(disk::AnimMontageReflection,disk::AnimMontageConstructor,montageType)||
        !native(disk::SkeletonReflection,disk::SkeletonConstructor,skeletonType)||
        !cfgspawn::ObjectSlot(r,objects,roots[0],value.wave)||!cfgspawn::ClassContains(r,roots[0],emoteType)||
        !named(roots[0],u"Emote_Wave_C")||!r.Read(roots[0]+cfgspawn::ClassDefaultObject,defaults)||
        !cfgspawn::ObjectSlot(r,objects,defaults,value.defaults)||!same(defaults+cfgprobe::ObjectClass,roots[0])||
        !named(defaults,u"Default__Emote_Wave_C")||!same(defaults+disk::MaleMontage,(published&1)?loaded:roots[1])||
        !(male=roots[1])||
        !cfgspawn::ObjectSlot(r,objects,male,value.male)||!cfgspawn::ObjectIs(r,male,montageType)||
        !named(male,u"AM_m_normal_stand_emote_wave")||!r.Read(male+disk::Skeleton,skeleton)||
        !cfgspawn::ObjectSlot(r,objects,skeleton,value.skeleton)||!cfgspawn::ObjectIs(r,skeleton,skeletonType))return false;
    if(roots[2]){
        if(!same(defaults+disk::FemaleMontage,(published&2)?loaded:roots[2])||!(female=roots[2])||
            !cfgspawn::ObjectSlot(r,objects,female,value.female)||!cfgspawn::ObjectIs(r,female,montageType)||
            !named(female,u"AM_f_normal_stand_emote_wave")||!r.Read(female+disk::Skeleton,other)||other!=skeleton)return false;
    }
    if(loaded){
        if(!cfgspawn::ObjectSlot(r,objects,loaded,value.loaded)||!cfgspawn::ObjectIs(r,loaded,montageType)||
            !named(loaded,u"m_normal_stand_piss_Montage")||!r.Read(loaded+disk::Skeleton,other)||other!=skeleton)return false;
    }
    cfgprobe::lifetime::SlotObservation fresh{};
    for(const auto& slot:{value.wave,value.defaults,value.male,value.female,value.skeleton,value.loaded}){
        if(slot.object&&(!cfgspawn::ObjectSlot(r,objects,slot.object,fresh)||!(fresh==slot)))return false;
    }
    if(!same(roots[0]+cfgspawn::ClassDefaultObject,defaults)||!same(defaults+cfgprobe::ObjectClass,roots[0])||
        !same(defaults+disk::MaleMontage,(published&1)?loaded:male)||!same(male+disk::Skeleton,skeleton)||
        (female&&(!same(defaults+disk::FemaleMontage,(published&2)?loaded:female)||!same(female+disk::Skeleton,skeleton)))||
        (loaded&&!same(loaded+disk::Skeleton,skeleton))||
        !same(module+disk::SCEmoteDataReflection,emoteType)||!same(module+disk::SCEmoteDataConstructor,emoteType)||
        !same(module+disk::AnimMontageReflection,montageType)||!same(module+disk::AnimMontageConstructor,montageType)||
        !same(module+disk::SkeletonReflection,skeletonType)||!same(module+disk::SkeletonConstructor,skeletonType)||
        !cfgspawn::ClassContains(r,roots[0],emoteType)||!cfgspawn::ObjectIs(r,male,montageType)||
        !cfgspawn::ObjectIs(r,skeleton,skeletonType)||(female&&!cfgspawn::ObjectIs(r,female,montageType))||
        (loaded&&!cfgspawn::ObjectIs(r,loaded,montageType)))return false;
    out=value;return true;
}
}
